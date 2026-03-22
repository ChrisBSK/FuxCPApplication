#include "MainComponent.h"


MainComponent::MainComponent(): juce::Component()
{
    setSize(900, 600);



    addAndMakeVisible(header);
    addAndMakeVisible(leftPanel);
    addAndMakeVisible(workArea);
    addAndMakeVisible(keyboard);
    addAndMakeVisible(history);
    addAndMakeVisible(footer);



}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
}

void MainComponent::resized()
{
    auto area = getLocalBounds();

    // Header
    header.setBounds(area.removeFromTop(60));

    // Bottom area (history + keyboard)
    auto bottomArea = area.removeFromBottom(120);

    history.setBounds(bottomArea.removeFromLeft(200));
    keyboard.setBounds(bottomArea);

    // Left panel
    leftPanel.setBounds(area.removeFromLeft(200));

    // Work area
    workArea.setBounds(area);

    optionsPanel.setBounds(getLocalBounds().reduced(100));

}

