#include "OptionsPanelHelpers.h"

/*
//==============================================================================
// OptionsPanelHelpers
//
// Fonctions utilitaires pour configurer les éléments visuels
// réutilisables de l'OptionsPanel.
//==============================================================================
*/

namespace OptionsPanelHelpers
{
    void setupTitle(juce::Component& parent,
                    juce::Label& label,
                    const juce::String& text)
    {
        parent.addAndMakeVisible(label);

        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, juce::Colours::white);
        label.setFont(juce::Font(16.0f, juce::Font::bold));
    }

    void setupLabel(juce::Component& parent,
                    juce::Label& label,
                    const juce::String& text)
    {
        parent.addAndMakeVisible(label);

        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centredLeft);
        label.setColour(juce::Label::textColourId, juce::Colours::white);
    }

    void setupButton(juce::Component& parent,
                     juce::TextButton& button,
                     const juce::String& text)
    {
        parent.addAndMakeVisible(button);
        button.setButtonText(text);
    }

    void setupHorizontalSlider(juce::Component& parent,
                               juce::Slider& slider,
                               double min,
                               double max,
                               double interval,
                               double defaultValue)
    {
        parent.addAndMakeVisible(slider);

        slider.setRange(min, max, interval);
        slider.setValue(defaultValue, juce::dontSendNotification);
        slider.setSliderStyle(juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    }
}