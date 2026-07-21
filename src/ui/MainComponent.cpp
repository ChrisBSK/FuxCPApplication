#include "MainComponent.h"

//==============================================================================
// Constructor
//==============================================================================

MainComponent::MainComponent()
{
    setSize(1000, 560);

    // =========================
    // UI : ajout des composants
    // =========================
    addAndMakeVisible(header);
    addAndMakeVisible(optionsPanel);
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
    // ===== séparation sous le header =====
    int topY = header.getBottom();

    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.drawLine(0.0f, static_cast<float>(topY),
               static_cast<float>(getWidth()), static_cast<float>(topY), 1.0f);
}

//==============================================================================
// Layout
//==============================================================================

void MainComponent::resized()
{
    const int windowWidth = getWidth();
    const int windowHeight = getHeight();

    // Les hauteurs principales restent bornées,
    // puis FlexBox distribue verticalement le reste.
    const int margin = juce::jlimit(2, 8, windowWidth / 220);
    const int headerHeight = juce::jlimit(28, 42, static_cast<int>(windowHeight * 0.055f));
    const int keyboardHeight = juce::jlimit(62, 88, static_cast<int>(windowHeight * 0.12f));

    juce::FlexBox mainColumn;
    mainColumn.flexDirection = juce::FlexBox::Direction::column;
    mainColumn.alignItems = juce::FlexBox::AlignItems::stretch;

    mainColumn.items.add(juce::FlexItem(header).withHeight((float) headerHeight));
    mainColumn.items.add(juce::FlexItem(optionsPanel)
        .withFlex(1.0f)
        .withMargin(juce::FlexItem::Margin(0.0f, (float) margin, 0.0f, (float) margin)));
    mainColumn.items.add(juce::FlexItem(keyboard)
        .withHeight((float) keyboardHeight)
        .withMargin(juce::FlexItem::Margin(0.0f, (float) margin, 0.0f, (float) margin)));

    mainColumn.performLayout(getLocalBounds());
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
