#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class KeyboardPanel : public juce::Component
{
public:
    KeyboardPanel();
    ~KeyboardPanel() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KeyboardPanel);
};