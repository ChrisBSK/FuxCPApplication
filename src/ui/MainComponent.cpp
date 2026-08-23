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
/*
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
}*/

MainComponent::MainComponent(AppController& controllerToUse, juce::MidiKeyboardState& keyboardStateToUse)
    : appController(controllerToUse), keyboardState(keyboardStateToUse)
{
    setSize(960, 420);

    addAndMakeVisible(header);
    addAndMakeVisible(optionsPanel);
    addAndMakeVisible(keyboard);
    addAndMakeVisible(savedSolutionsPage);
    addAndMakeVisible(solverExplanationPage);
    addAndMakeVisible(aboutPage);


    keyboard.onNotePressed = [this](int midiNote)
    {
        leftPanel.addNoteFromKeyboard(midiNote);
    };

    // =========================
    // HEADER - changement de PAGES
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
    // SYNCHRONISATION UI (liaisons entre composants)
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

    // Onglet Saved configurations : lit la liste sauvegardée et charge la
    // configuration choisie par l'utilisateur.
    savedSolutionsPage.setAppController(&appController);
    savedSolutionsPage.onConfigurationSelected = [this](const juce::File& file)
    {
        loadSavedConfiguration(file);
    };


    /*
        AppController (possédé par PluginProcessor) survit à la fermeture
        de cette fenêtre : rouvrir le plug-in reconstruit un tout nouveau
        MainComponent, mais pas un nouvel AppController.

        S'il contient déjà
        un problème - fenêtre rouverte en cours de session GarageBand -
        on resynchronise l'interface avec ce problème dès la construction.


    */
    if (! appController.getProblem().isEmpty())
    {
        leftPanel.updateCantusDisplay();
        leftPanel.setNumVoicesDisplay(appController.getProblem().getVoiceCount());
        optionsPanel.loadFromModel();
    }
    else
    {
        // Vraie première ouverture : aucun problème à restaurer, on met en
        // place le workspace vide par défaut.
        optionsPanel.setNumVoices(defaultVoiceCount);
    }

    tooltipWindow = std::make_unique<juce::TooltipWindow>(this, 500);
    showPage(CurrentPage::mainScreen);
}


//==============================================================================
// Destructor
//==============================================================================

MainComponent::~MainComponent() = default;

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
    const int keyboardHeight = juce::jlimit(50, 75, static_cast<int>(windowHeight * 0.11f));

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

    //version standalone du clavier, ne va pas sur la version AU (les boutons et keyboard se chevauchent)
    /*const auto workspaceBounds = optionsPanel.getWorkspaceBounds()
    .translated(optionsPanel.getX(), optionsPanel.getY());

    const int keyboardWidth = juce::jmin(workspaceBounds.getWidth() + 380,
                                         getWidth() - margin * 2);
    const int keyboardX = workspaceBounds.getCentreX() - keyboardWidth / 2;
    const int keyboardY = optionsPanel.getBottom() - keyboardHeight;

    // Le clavier est centré sur le rectangle arrondi du milieu.
    keyboard.setBounds(keyboardX,
                       keyboardY,
                       keyboardWidth,
                       keyboardHeight);*/

    const auto leftPanelBounds = optionsPanel.getLeftPanelBounds()
    .translated(optionsPanel.getX(), optionsPanel.getY());

    // L'espace disponible pour le clavier commence après le LeftPanel,
    // garantit qu'il ne peut jamais recouvrir les boutons Generate/Next solution/etc.
    const int keyboardAreaLeft = leftPanelBounds.getRight() + margin;
    const int keyboardAreaRight = getWidth() - margin;
    const int keyboardAreaWidth = juce::jmax(0, keyboardAreaRight - keyboardAreaLeft);

    constexpr int maxKeyboardWidth = 900;

    const int keyboardWidth = juce::jmin(keyboardAreaWidth, maxKeyboardWidth);
    const int keyboardX = keyboardAreaLeft + (keyboardAreaWidth - keyboardWidth) / 2;
    const int keyboardY = optionsPanel.getBottom() - keyboardHeight;

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
    const bool showSavedSolutions = currentPage == CurrentPage::savedSolutions;

    optionsPanel.setVisible(showMainScreen);
    keyboard.setVisible(showMainScreen);

    savedSolutionsPage.setVisible(showSavedSolutions);
    solverExplanationPage.setVisible(currentPage == CurrentPage::solver);
    aboutPage.setVisible(currentPage == CurrentPage::about);

    // La liste peut avoir changé depuis la dernière visite de cet onglet
    // (une nouvelle configuration a pu être sauvegardée entre-temps).
    if (showSavedSolutions)
        savedSolutionsPage.refresh();
}

/*
    Charge la configuration sauvegardée choisie par l'utilisateur et
    rafraîchit toute l'interface pour refléter ce nouvel état.

    L'ordre compte : AppController remplace d'abord le modèle (voir
    CantusProblem::restoreFromValueTree), puis chaque panneau se recale
    dessus (Cantus Firmus, nombre de voix, sliders, shapes, priorités du
    solveur), et on revient enfin à l'écran principal.
*/
void MainComponent::loadSavedConfiguration(const juce::File& file)
{
    if (! appController.loadConfiguration(file))
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            juce::String::fromUTF8("Saved configurations"),
            juce::String::fromUTF8("Impossible de charger cette configuration."));
        return;
    }

    // Plus aucun fichier MIDI généré ne correspond à cette configuration.
    leftPanel.clearGeneratedMidiDisplay();

    // Cantus Firmus affiché à partir du modèle qui vient d'être remplacé.
    leftPanel.updateCantusDisplay();

    // Déclenche la cascade normale (OptionsPanel::setNumVoices, colonnes
    // des contrepoints...), comme si l'utilisateur choisissait ce nombre
    // de voix lui-même.
    leftPanel.setNumVoicesDisplay(appController.getProblem().getVoiceCount());

    // Sliders, shapes, méthode de recherche et de minimisation, priorités.
    optionsPanel.loadFromModel();

    showPage(CurrentPage::mainScreen);
}

