#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace OptionsPanelHelpers
{
    void setupTitle(juce::Component& parent,
                    juce::Label& label,
                    const juce::String& text);
}