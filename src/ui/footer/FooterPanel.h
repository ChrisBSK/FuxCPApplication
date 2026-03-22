#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class FooterPanel : public juce::Component
{
public:
    FooterPanel();
    ~FooterPanel() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FooterPanel);
};