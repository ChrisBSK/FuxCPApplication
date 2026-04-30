#include "OptionsPanelHelpers.h"

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
}