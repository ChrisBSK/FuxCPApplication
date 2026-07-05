#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "OnOffSwitchButton.h"

/*
//==============================================================================
   ParameterFactory

   Crée des contrôles UI prêts à être ajoutés dans une colonne de paramètres.
//==============================================================================
*/
class ParameterFactory
{
public:
    static std::unique_ptr<juce::Slider> slider(double min,
                                                double max,
                                                double interval,
                                                double defaultValue)
    {
        auto slider = std::make_unique<juce::Slider>();

        slider->setRange(min, max, interval);
        slider->setValue(defaultValue, juce::dontSendNotification);
        slider->setSliderStyle(juce::Slider::LinearHorizontal);
        slider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);

        return slider;
    }

    static std::unique_ptr<juce::ToggleButton> toggle(const juce::String& text)
    {
        auto toggle = std::make_unique<juce::ToggleButton>();
        toggle->setButtonText(text);
        return toggle;
    }

    static std::unique_ptr<OnOffSwitchButton> onOffSwitch(bool defaultValue)
    {
        auto button = std::make_unique<OnOffSwitchButton>();
        button->setOn(defaultValue, juce::dontSendNotification);
        return button;
    }

    static std::unique_ptr<juce::TextButton> button(const juce::String& text)
    {
        auto button = std::make_unique<juce::TextButton>();
        button->setButtonText(text);
        return button;
    }
};
