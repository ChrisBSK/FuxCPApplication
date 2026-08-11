#include "MainComponent.h"

//==============================================================================
// SimplePage
//==============================================================================

MainComponent::SimplePage::SimplePage(const juce::String& titleText,
                                      const juce::String& bodyText)
{
    title.setText(titleText, juce::dontSendNotification);
    title.setJustificationType(juce::Justification::centred);
    title.setColour(juce::Label::textColourId, juce::Colours::white);
    title.setFont(juce::Font(18.0f, juce::Font::bold));

    body.setText(bodyText, juce::dontSendNotification);
    body.setJustificationType(juce::Justification::centred);
    body.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.85f));
    body.setFont(juce::Font(13.0f));

    addAndMakeVisible(title);
    addAndMakeVisible(body);
}

void MainComponent::SimplePage::paint(juce::Graphics& g)
{
    auto area = getLocalBounds().reduced(22);

    g.setColour(juce::Colour(0xff3f3f3f));
    g.fillRoundedRectangle(area.toFloat(), 8.0f);

    g.setColour(juce::Colours::white.withAlpha(0.25f));
    g.drawRoundedRectangle(area.toFloat(), 8.0f, 1.0f);
}

void MainComponent::SimplePage::resized()
{
    auto area = getLocalBounds().reduced(36);

    title.setBounds(area.removeFromTop(32));
    area.removeFromTop(8);
    body.setBounds(area.removeFromTop(40));
}

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
    addAndMakeVisible(savedSolutionsPage);
    addAndMakeVisible(solverExplanationPage);
    addAndMakeVisible(aboutPage);


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
    // HEADER → PAGES
    // =========================
    header.onPageChanged = [this](HeaderPanel::Page page)
    {
        switch (page)
        {
            case HeaderPanel::Page::mainScreen:
                showPage(CurrentPage::mainScreen);
                break;

            case HeaderPanel::Page::savedSolutions:
                showPage(CurrentPage::savedSolutions);
                break;

            case HeaderPanel::Page::solver:
                showPage(CurrentPage::solver);
                break;

            case HeaderPanel::Page::about:
                showPage(CurrentPage::about);
                break;
        }
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

    showPage(CurrentPage::mainScreen);
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
    // Clavier plus large et moins haut :
    // les touches restent lisibles sans prendre trop d'espace vertical.
    const int keyboardHeight = juce::jlimit(92, 122, static_cast<int>(windowHeight * 0.17f));

    juce::FlexBox mainColumn;
    mainColumn.flexDirection = juce::FlexBox::Direction::column;
    mainColumn.alignItems = juce::FlexBox::AlignItems::stretch;

    mainColumn.items.add(juce::FlexItem(header).withHeight((float) headerHeight));
    mainColumn.items.add(juce::FlexItem(optionsPanel)
        .withFlex(1.0f)
        .withMargin(juce::FlexItem::Margin(0.0f, (float) margin, 0.0f, (float) margin)));

    mainColumn.performLayout(getLocalBounds());

    auto pageArea = optionsPanel.getBounds();
    savedSolutionsPage.setBounds(pageArea);
    solverExplanationPage.setBounds(pageArea);
    aboutPage.setBounds(pageArea);

    optionsPanel.setLowerReservedHeight(keyboardHeight);

    const auto workspaceBounds = optionsPanel.getWorkspaceBounds()
        .translated(optionsPanel.getX(), optionsPanel.getY());

    const int keyboardWidth = juce::jmin(workspaceBounds.getWidth() + 380,
                                         getWidth() - margin * 2);
    const int keyboardX = workspaceBounds.getCentreX() - keyboardWidth / 2;
    const int keyboardY = optionsPanel.getBottom() - keyboardHeight;

    // Le clavier est centré sur le rectangle arrondi du milieu.
    keyboard.setBounds(keyboardX,
                       keyboardY,
                       keyboardWidth,
                       keyboardHeight);
}

//==============================================================================
// Pages
//==============================================================================

void MainComponent::showPage(CurrentPage page)
{
    currentPage = page;

    const bool showMainScreen = currentPage == CurrentPage::mainScreen;

    optionsPanel.setVisible(showMainScreen);
    keyboard.setVisible(showMainScreen);

    savedSolutionsPage.setVisible(currentPage == CurrentPage::savedSolutions);
    solverExplanationPage.setVisible(currentPage == CurrentPage::solver);
    aboutPage.setVisible(currentPage == CurrentPage::about);
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
