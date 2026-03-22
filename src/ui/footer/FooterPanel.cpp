#include "FooterPanel.h"

FooterPanel::FooterPanel()
{
}

FooterPanel::~FooterPanel()
{
}

void FooterPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::purple);

    g.setColour(juce::Colours::white);
    g.drawText("FOOTER",
               getLocalBounds(),
               juce::Justification::centred,
               true);
}

void FooterPanel::resized()
{
}