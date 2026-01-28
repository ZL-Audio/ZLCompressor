// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "mag_analyzer_panel.hpp"

namespace zlpanel {
    MagAnalyzerPanel::MagAnalyzerPanel(PluginProcessor& p, zlgui::UIBase& base) :
        p_ref_(p), base_(base),
        meter_display_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PMeterDisplay::kID)),
        background_panel_(p, base),
        peak_panel_(p, base),
        rms_panel_(p, base),
        computer_panel_(p, base),
        separate_panel_(base),
        meter_panel_(p, base),
        updater_(),
        threshold_slider_(base),
        threshold_attachment_(threshold_slider_, p.parameters_,
                              zlp::PThreshold::kID, updater_),
        ratio_slider_(base),
        ratio_attachment_(ratio_slider_, p.parameters_,
                          zlp::PRatio::kID, updater_) {
        addAndMakeVisible(background_panel_);
        addAndMakeVisible(peak_panel_);
        addAndMakeVisible(separate_panel_);
        addAndMakeVisible(rms_panel_);
        addAndMakeVisible(computer_panel_);
        addAndMakeVisible(meter_panel_);
        addChildComponent(threshold_slider_);
        addChildComponent(ratio_slider_);

        setInterceptsMouseClicks(true, false);

        peak_consumer_id_ = transfer_buffer_.getMulticastFIFO().addConsumer();
        meter_consumer_id_ = transfer_buffer_.getMulticastFIFO().addConsumer();
    }

    MagAnalyzerPanel::~MagAnalyzerPanel() = default;

    void MagAnalyzerPanel::resized() {
        updateBounds();
    }

    void MagAnalyzerPanel::run(const juce::Thread& thread) {
        juce::ScopedNoDenormals no_denormals;
        const auto time_stamp = next_stamp_.load(std::memory_order::relaxed);
        auto& sender{p_ref_.getCompressController().getMagAnalyzerSender()};
        sender.getLock().lock();

        if (std::abs(sample_rate_ - sender.getSampleRate()) > 1.0 ||
            max_sum_samples_ != sender.getMaxNumSamples()) {
            sample_rate_ = sender.getSampleRate();
            max_sum_samples_ = sender.getMaxNumSamples();
            transfer_buffer_.prepare(sample_rate_, max_sum_samples_, {2, 2, 2}, 0.5);
        }
        transfer_buffer_.processTransfer(sender.getAbstractFIFO(), sender.getSampleFIFOs());
        sender.getLock().unlock();
        if (thread.threadShouldExit()) {
            return;
        }
        if (sample_rate_ > 20000.0 && max_sum_samples_ > 0) {
            peak_panel_.run(time_stamp, rms_panel_, transfer_buffer_, peak_consumer_id_);
            if (thread.threadShouldExit()) {
                return;
            }
            meter_panel_.run(time_stamp, transfer_buffer_, meter_consumer_id_);
        }
        if (thread.threadShouldExit()) {
            return;
        }
        computer_panel_.run();
    }

    void MagAnalyzerPanel::repaintCallBackSlow() {
        updater_.updateComponents();
        background_panel_.repaintCallBackSlow();

        const auto meter_visible = meter_display_ref_.load(std::memory_order::relaxed) > .5f;
        if (meter_visible != meter_panel_.isVisible()) {
            meter_panel_.setVisible(meter_visible);
            updateBounds();
        }
    }

    void MagAnalyzerPanel::repaintCallBack(const double time_stamp) {
        next_stamp_.store(time_stamp, std::memory_order::relaxed);
        repaint();
        if (time_stamp - rms_previous_stamp_ > 1. && rms_panel_.isVisible()) {
            rms_panel_.repaint();
            rms_previous_stamp_ = time_stamp;
        }
    }

    void MagAnalyzerPanel::mouseDoubleClick(const juce::MouseEvent&) {
        rms_panel_.setToReset();
    }

    void MagAnalyzerPanel::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) {
        if (event.mods.isCommandDown()) {
            ratio_slider_.mouseWheelMove(event, wheel);
        } else {
            threshold_slider_.mouseWheelMove(event, wheel);
        }
    }

    void MagAnalyzerPanel::updateBounds() {
        auto bound = getLocalBounds();
        if (meter_panel_.isVisible()) {
            meter_panel_.setBounds(bound.removeFromRight(static_cast<int>(std::round(base_.getFontSize() * 4.f))));
        }
        background_panel_.setBounds(bound);
        peak_panel_.setBounds(bound);
        rms_panel_.setBounds(bound.withWidth(juce::roundToInt(static_cast<float>(bound.getWidth()) * .15f)));
        const auto r = std::min(bound.getWidth(), bound.getHeight());
        separate_panel_.setBounds(bound.withSize(r, r));
        computer_panel_.setBounds(bound.withSize(r, r));
        threshold_slider_.setBounds(bound);
        ratio_slider_.setBounds(bound);
    }
}
