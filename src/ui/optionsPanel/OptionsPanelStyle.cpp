#include "OptionsPanelStyle.h"

namespace OptionsPanelStyle
{
    void setupTitle(juce::Label& l)
    {
        l.setJustificationType(juce::Justification::centred);
        l.setColour(juce::Label::textColourId, juce::Colours::white);
        l.setFont(juce::Font(16.0f, juce::Font::bold));
    }
}