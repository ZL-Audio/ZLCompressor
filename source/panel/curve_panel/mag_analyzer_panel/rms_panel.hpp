// Copyright (C) 2026 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../../PluginProcessor.hpp"
#include "../../../gui/gui.hpp"
#include "../../helper/helper.hpp"
#include "../../../dsp/analyzer/analyzer_base/fifo_transfer_buffer.hpp"
#include "../../../dsp/analyzer/mag_analyzer/mag_rms_hist_receiver.hpp"
#include "../../../dsp/lock/spin_lock.hpp"

namespace zlpanel {
    class RMSPanel final : public juce::Component {
    public:
        explicit RMSPanel(PluginProcessor& p, zlgui::UIBase& base);

        void paint(juce::Graphics& g) override;

        void run(double sample_rate, zldsp::container::FIFORange range,
                 zldsp::analyzer::FIFOTransferBuffer<3>& transfer_buffer);

        void resized() override;

        void setToReset() {
            to_reset_.store(true, std::memory_order::relaxed);
        }

    private:
        static constexpr size_t kNumPoints = 100;

        zlgui::UIBase& base_;

        std::atomic<float>& analyzer_min_db_ref_;

        zldsp::analyzer::MagRMSHistReceiver in_receiver_{}, out_receiver_{};

        AtomicBound<float> atomic_bound_;

        double sample_rate_{0.};

        float curve_thickness_{0.f};

        float min_db_idx_{0.f};

        float height_{0.f};

        std::array<float, kNumPoints> in_xs_{}, out_xs_{}, ys_{};
        juce::Path in_path_, out_path_;
        juce::Path next_in_path_, next_out_path_;
        zldsp::lock::SpinLock mutex_;

        std::atomic<bool> to_reset_{false};

        void lookAndFeelChanged() override;
    };
}
