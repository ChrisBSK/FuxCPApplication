#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class OptionsPanel : public juce::Component
{
public:
    OptionsPanel()
    {
        addAndMakeVisible(option1);
        addAndMakeVisible(option2);

        option1.setButtonText("Option 1");
        option2.setButtonText("Option 2");
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::darkgrey.withAlpha(0.9f));
    }

    void resized() override
    {
        option1.setBounds(40, 40, 200, 30);
        option2.setBounds(40, 80, 200, 30);
    }

private:
    juce::ToggleButton option1;
    juce::ToggleButton option2;
};