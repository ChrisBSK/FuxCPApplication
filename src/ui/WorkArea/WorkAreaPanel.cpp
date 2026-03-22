#include "WorkAreaPanel.h"

WorkAreaPanel::WorkAreaPanel()
{
}

WorkAreaPanel::~WorkAreaPanel()
{
}

void WorkAreaPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::lightgrey);

    g.setColour(juce::Colours::black);
    g.drawText("WORK AREA",
               getLocalBounds(),
               juce::Justification::centred,
               true);
}

void WorkAreaPanel::resized()
{
}