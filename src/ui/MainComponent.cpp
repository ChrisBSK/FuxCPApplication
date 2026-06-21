#include "MainComponent.h"

//==============================================================================
// Constructor
//==============================================================================

MainComponent::MainComponent()
{
    setSize(1280, 780);

    // =========================
    // UI : ajout des composants
    // =========================
    addAndMakeVisible(header);
    addAndMakeVisible(leftPanel);
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

    // LeftPanel écoute les changements de génération
    leftPanel.connectToGenerationState(appController.getGenerationState());

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
    const int windowWidth = getWidth();
    const int windowHeight = getHeight();

    // Marges et zones calculées à partir de la fenêtre.
    // Cette version privilégie la visibilité complète des composants
    // quand l'utilisateur réduit l'application.
    const int margin = juce::jlimit(6, 16, windowWidth / 120);
    const int leftPanelWidth = juce::jlimit(180, 280, static_cast<int>(windowWidth * 0.22f));
    const int historyHeight = juce::jlimit(75, 120, static_cast<int>(windowHeight * 0.15f));
    const int headerHeight = juce::jlimit(42, 60, static_cast<int>(windowHeight * 0.08f));
    const int keyboardHeight = juce::jlimit(70, 100, static_cast<int>(windowHeight * 0.13f));

    auto area = getLocalBounds().reduced(margin);

    // =========================
    // LEFT PANEL + HISTORY
    // =========================
    auto leftArea = area.removeFromLeft(leftPanelWidth);

    history.setBounds(leftArea.removeFromBottom(historyHeight));
    leftPanel.setBounds(leftArea);

    // =========================
    // RIGHT SIDE
    // =========================
    auto rightArea = area;

    header.setBounds(rightArea.removeFromTop(headerHeight));
    keyboard.setBounds(rightArea.removeFromBottom(keyboardHeight));

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
