#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class HistoryPanel : public juce::Component
{
public:
    HistoryPanel();
    ~HistoryPanel() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HistoryPanel);
};