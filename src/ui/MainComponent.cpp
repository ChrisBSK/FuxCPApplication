#include "MainComponent.h"


MainComponent::MainComponent():
    juce::Component(),
    keyboard(keyboardState),
audioPlayer(keyboardState)
{
    setSize(1900, 1900);



    addAndMakeVisible(header);
    addAndMakeVisible(leftPanel);
    addAndMakeVisible(workArea);
    addAndMakeVisible(keyboard);
    addAndMakeVisible(history);
    addAndMakeVisible(footer);

    deviceManager.initialise(0, 2, nullptr, true);

    player.setSource(&audioPlayer);
    deviceManager.addAudioCallback(&player);
    //setAudioChannels(0, 2); // 0 input, 2 output

    keyboard.onNotePressed = [this](int midiNote)
    {
        auto currentText = leftPanel.getCantusText(); // à adapter

        juce::String newText = currentText + " " + juce::String(midiNote);

        leftPanel.setCantusText(newText); // à adapter
    };
}

MainComponent::~MainComponent()
{
    player.setSource(nullptr);
    deviceManager.removeAudioCallback(&player);
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
    auto bottomArea = area.removeFromBottom(80);

    history.setBounds(bottomArea.removeFromLeft(280));
    keyboard.setBounds(bottomArea);

    // Left panel
    leftPanel.setBounds(area.removeFromLeft(280));

    // Work area
    workArea.setBounds(area);


}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    auto* buffer = bufferToFill.buffer;

    buffer->clear();

    juce::MidiBuffer midi;

    keyboardState.processNextMidiBuffer(
        midi,
        bufferToFill.startSample,
        bufferToFill.numSamples,
        true
    );


    synth.render(*buffer, midi);
}

void MainComponent::prepareToPlay(int, double sampleRate)
{
    synth.prepare(sampleRate);
}
