#include "HeaderPanel.h"

HeaderPanel::HeaderPanel()
{
}

HeaderPanel::~HeaderPanel()
{
}

void HeaderPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);

    g.setColour(juce::Colours::white);
    g.drawText("",
               getLocalBounds(),
               juce::Justification::centred,
               true);
}

void HeaderPanel::resized()
{
}