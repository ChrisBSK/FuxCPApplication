#include "KeyboardComponent.h"

KeyboardComponent::KeyboardComponent(juce::MidiKeyboardState& state)
    : midiKeyboard(state,
        juce::MidiKeyboardComponent::Orientation::horizontalKeyboard)
{
    addAndMakeVisible(midiKeyboard);

    midiKeyboard.setAvailableRange(24, 108); //piano standard en général
    midiKeyboard.setKeyWidth(30.0f);

    midiKeyboard.setScrollButtonsVisible(true);

    setWantsKeyboardFocus(true);

    state.addListener(this);



}

// =============================
// Callback des notes jouées
// =============================
void KeyboardComponent::handleNoteOn(juce::MidiKeyboardState*, int,
                                     int midiNoteNumber, float)
{
    if (onNotePressed)
        onNotePressed(midiNoteNumber);
}

void KeyboardComponent::resized()
{
    auto width = getWidth();
    int numKeys = 108 - 24 + 1;

    float keyWidth = width / (float)numKeys;

    midiKeyboard.setKeyWidth(keyWidth);
    auto area = getLocalBounds();

    // largeur réelle du clavier
    float totalWidth = midiKeyboard.getTotalKeyboardWidth();

    // centre horizontalement
    int x = (area.getWidth() - totalWidth) / 2;

    midiKeyboard.setBounds(x, 0, totalWidth, area.getHeight());
}