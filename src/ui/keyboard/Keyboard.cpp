#include "Keyboard.h"

KeyboardPanel::KeyboardPanel()
{
}

KeyboardPanel::~KeyboardPanel()
{
}

void KeyboardPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::red);

    g.setColour(juce::Colours::white);
    g.drawText("KEYBOARD",
               getLocalBounds(),
               juce::Justification::centred,
               true);
}

void KeyboardPanel::resized()
{
}