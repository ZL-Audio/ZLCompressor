// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "BinaryData.h"
#include "logo_panel.hpp"

namespace zlpanel {
    LogoPanel::LogoPanel(PluginProcessor &, zlgui::UIBase &base, multilingual::TooltipHelper &tooltip_helper)
        : base_(base),
          brand_drawable_(juce::Drawable::createFromImageData(BinaryData::zlaudio_svg, BinaryData::zlaudio_svgSize)),
          logo_drawable_(juce::Drawable::createFromImageData(BinaryData::logo_svg, BinaryData::logo_svgSize)) {
        setAlpha(.5f);
        setBufferedToImage(true);

        SettableTooltipClient::setTooltip(tooltip_helper.getToolTipText(multilingual::TooltipLabel::kLogo));
    }

    void LogoPanel::paint(juce::Graphics &g) {
        const auto temp_brand = brand_drawable_->createCopy();
        const auto temp_logo = logo_drawable_->createCopy();
        temp_brand->replaceColour(juce::Colour(0, 0, 0), base_.getTextColor());
        temp_logo->replaceColour(juce::Colour(0, 0, 0), base_.getTextColor());
        temp_logo->replaceColour(
            juce::Colour(static_cast<juce::uint8>(0), static_cast<juce::uint8>(0), static_cast<juce::uint8>(0), .5f),
            base_.getTextColor().withMultipliedAlpha(.5f));

        auto bound = getLocalBounds().toFloat();
        const auto padding = base_.getFontSize() * kPaddingScale * .5f;
        bound.removeFromLeft(padding);
        bound.removeFromTop(padding);
        bound.removeFromBottom(padding);

        const auto brand_scale = bound.getHeight() / static_cast<float>(brand_drawable_->getHeight());
        const auto brand_width = static_cast<float>(brand_drawable_->getWidth()) * brand_scale;

        temp_brand->setTransform(juce::AffineTransform::scale(brand_scale));
        temp_brand->drawAt(g, bound.getX(), bound.getY(), 1.0f);

        const auto logo_scale = bound.getHeight() / static_cast<float>(logo_drawable_->getHeight());
        temp_logo->setTransform(juce::AffineTransform::scale(logo_scale));
        temp_logo->drawAt(g, bound.getX() + brand_width + padding, bound.getY(), 1.0f);
    }

    void LogoPanel::mouseEnter(const juce::MouseEvent &) {
        setAlpha(1.f);
    }

    void LogoPanel::mouseExit(const juce::MouseEvent &) {
        setAlpha(.5f);
    }

    void LogoPanel::mouseDoubleClick(const juce::MouseEvent &) {
        base_.setPanelProperty(zlgui::kUISettingPanel, true);
    }
}
