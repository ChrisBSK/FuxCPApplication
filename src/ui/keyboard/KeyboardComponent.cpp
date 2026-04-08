#include "KeyboardComponent.h"

KeyboardComponent::KeyboardComponent(juce::MidiKeyboardState& state)
    : midiKeyboard(state,
        juce::MidiKeyboardComponent::Orientation::horizontalKeyboard)
{
    addAndMakeVisible(midiKeyboard);

    midiKeyboard.setAvailableRange(36, 84);
    midiKeyboard.setScrollButtonsVisible(true);

    setWantsKeyboardFocus(true);
}

void KeyboardComponent::resized()
{
    midiKeyboard.setBounds(getLocalBounds().reduced(5));
}