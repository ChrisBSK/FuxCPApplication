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
    addAndMakeVisible(optionsPanel);

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

void MainComponent::paintOverChildren(juce::Graphics& g)
{
    // =============================
    // Ligne verticale (séparation leftPanel)
    // =============================
    int separatorX = leftPanel.getRight();

    int separatorTop = header.getBottom();
    int separatorBottom = keyboard.getBottom();

    // ligne principale
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.drawLine((float)separatorX,
               (float)separatorTop,
               (float)separatorX,
               (float)separatorBottom,
               1.0f);

    // highlight léger
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    g.drawLine((float)separatorX + 1,
               (float)separatorTop,
               (float)separatorX + 1,
               (float)separatorBottom,
               1.0f);


    // =============================
    // Ligne horizontale (SOUS leftPanel uniquement)
    // =============================
    int bottomY = keyboard.getY();

    int bottomLeft = leftPanel.getX();
    int bottomRight = leftPanel.getRight();

    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.drawLine((float)bottomLeft,
               (float)bottomY,
               (float)bottomRight,
               (float)bottomY,
               1.0f);

    g.setColour(juce::Colours::white.withAlpha(0.1f));
    g.drawLine((float)bottomLeft,
               (float)bottomY + 1,
               (float)bottomRight,
               (float)bottomY + 1,
               1.0f);


    // =============================
    // Ligne horizontale (HAUT - toute la partie droite)
    // =============================
    int topY = header.getBottom();

    int topLeft = 0;
    int topRight = getWidth();

    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.drawLine((float)topLeft,
               (float)topY,
               (float)topRight,
               (float)topY,
               1.5f);

    g.setColour(juce::Colours::white.withAlpha(0.1f));
    g.drawLine((float)topLeft,
               (float)topY + 1,
               (float)topRight,
               (float)topY + 1,
               1.0f);
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

    // OptionPanel area
    optionsPanel.setBounds(area);


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
