#include "MainComponent.h"

//==============================================================================
// Constructor
//==============================================================================

MainComponent::MainComponent()
{
    setSize(1900, 1900);

    // =========================
    // UI : ajout des composants
    // =========================
    addAndMakeVisible(header);
    addAndMakeVisible(leftPanel);
    addAndMakeVisible(workArea);
    addAndMakeVisible(optionsPanel);
    addAndMakeVisible(history);
    addAndMakeVisible(keyboard);
    addAndMakeVisible(footer);

    // =========================
    // AUDIO INIT
    // =========================
    deviceManager.initialise(0, 2, nullptr, true);

    player.setSource(&audioPlayer);
    deviceManager.addAudioCallback(&player);

    // =========================
    // KEYBOARD → LEFT PANEL
    // =========================
    keyboard.onNotePressed = [this](int midiNote)
    {
        leftPanel.addNoteFromKeyboard(midiNote);
    };

    // =========================
    // UI SYNC (liaisons entre composants)
    // =========================

    //callback generation
    appController.setLeftPanel(&leftPanel);

    appController.setGenerationService(&leftPanel.getGenerationService());

    //relier les widgets des contraintes d'OptionPanel à AppController
    optionsPanel.setAppController(&appController);

    //sync UI
    leftPanel.setOptionsPanel(&optionsPanel);


    //boutoun Generate
    optionsPanel.setLeftPanel(&leftPanel);


    optionsPanel.setNumVoices(defaultVoiceCount);


    // =========================
    // TOOLTIP
    // =========================
    tooltipWindow = std::make_unique<juce::TooltipWindow>(this, 500);
}

//==============================================================================
// Destructor
//==============================================================================

MainComponent::~MainComponent()
{
    player.setSource(nullptr);
    deviceManager.removeAudioCallback(&player);
}

//==============================================================================
// Paint
//==============================================================================

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
}

//==============================================================================
// UI overlays (séparateurs visuels)
//==============================================================================

void MainComponent::paintOverChildren(juce::Graphics& g)
{
    // ===== séparation verticale =====
    int x = leftPanel.getRight();

    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.drawLine(static_cast<float>(x), 0.0f, static_cast<float>(x), static_cast<float>(keyboard.getBottom()), 1.0f);

    g.setColour(juce::Colours::white.withAlpha(0.1f));
    g.drawLine(static_cast<float>(x) + 1, 0.0f, static_cast<float>(x) + 1, static_cast<float>(keyboard.getBottom()), 1.0f);

    // ===== séparation horizontale (history) =====
    int y = history.getY();

    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.drawLine(static_cast<float>(leftPanel.getX()), static_cast<float>(y),
               static_cast<float>(leftPanel.getRight()), static_cast<float>(y), 1.0f);

    // ===== séparation haut droite =====
    int topY = header.getBottom();

    g.drawLine(static_cast<float>(leftPanel.getRight()), static_cast<float>(topY),
               static_cast<float>(getWidth()), static_cast<float>(topY), 1.0f);
}

//==============================================================================
// Layout
//==============================================================================

void MainComponent::resized()
{
    auto area = getLocalBounds();

    // =========================
    // LEFT PANEL + HISTORY
    // =========================
    auto leftArea = area.removeFromLeft(280);

    history.setBounds(leftArea.removeFromBottom(120));
    leftPanel.setBounds(leftArea);

    // =========================
    // RIGHT SIDE
    // =========================
    auto rightArea = area;

    header.setBounds(rightArea.removeFromTop(60));
    keyboard.setBounds(rightArea.removeFromBottom(100));

    // centre = OptionsPanel
    optionsPanel.setBounds(rightArea);
}

//==============================================================================
// AUDIO CALLBACK
//==============================================================================

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

//==============================================================================
// AUDIO PREP
//==============================================================================

void MainComponent::prepareToPlay(int, double sampleRate)
{
    synth.prepare(sampleRate);
}