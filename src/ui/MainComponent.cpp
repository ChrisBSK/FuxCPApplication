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

    leftPanel.setOptionsPanel(&optionsPanel);
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

    int separatorTop = 0;
    int separatorBottom = keyboard.getBottom();

    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.drawLine((float)separatorX,
               (float)separatorTop,
               (float)separatorX,
               (float)separatorBottom,
               1.0f);

    g.setColour(juce::Colours::white.withAlpha(0.1f));
    g.drawLine((float)separatorX + 1,
               (float)separatorTop,
               (float)separatorX + 1,
               (float)separatorBottom,
               1.0f);


    // =============================
    // Ligne horizontale (AU-DESSUS DU HISTORY)
    // =============================
    int bottomY = history.getY();

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
    // Ligne horizontale (HAUT PARTIE DROITE UNIQUEMENT)
    // =============================
    int topY = header.getBottom();

    int topLeft = leftPanel.getRight();
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

    // =============================
    // Partie gauche (LeftPanel + History)
    // =============================
    auto leftArea = area.removeFromLeft(280);

    // History
    auto historyArea = leftArea.removeFromBottom(120);
    history.setBounds(historyArea);

    // Le reste du LeftPanel
    leftPanel.setBounds(leftArea);

    // =============================
    // Partie droite
    // =============================
    auto rightArea = area;

    // Header
    header.setBounds(rightArea.removeFromTop(60));

    // Bas (keyboard)
    auto bottomArea = rightArea.removeFromBottom(100);
    keyboard.setBounds(bottomArea);

    // Centre (OptionsPanel)
    optionsPanel.setBounds(rightArea);
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
