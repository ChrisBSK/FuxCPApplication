#include "HistoryPanel.h"

HistoryPanel::HistoryPanel()
{
}

HistoryPanel::~HistoryPanel()
{
}

void HistoryPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::orange);

    g.setColour(juce::Colours::black);
    g.drawText("HISTORY",
               getLocalBounds(),
               juce::Justification::centred,
               true);
}

void HistoryPanel::resized()
{
}