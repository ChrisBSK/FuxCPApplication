#pragma once


#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>

class KeyboardComponent : public juce::Component, public juce::MidiKeyboardStateListener
{
public:
    KeyboardComponent(juce::MidiKeyboardState& state);

    void resized() override;

    std::function<void(int)> onNotePressed;


private:
    void handleNoteOn(juce::MidiKeyboardState*, int midiChannel,
                      int midiNoteNumber, float velocity) override;

    void handleNoteOff(juce::MidiKeyboardState*, int, int, float) override {}

    juce::MidiKeyboardComponent midiKeyboard;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeyboardComponent)
};