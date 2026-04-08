#pragma once


#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>

class KeyboardComponent : public juce::Component
{
public:
    KeyboardComponent(juce::MidiKeyboardState& state);

    void resized() override;

private:
    juce::MidiKeyboardComponent midiKeyboard;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeyboardComponent)
};