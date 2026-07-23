#include "OptionsPanel.h"

#include "../leftPanel/LeftPanel.h"
#include "../../controller/AppController.h"
#include "OptionsPanelHelpers.h"

#include <array>

//==============================================================================
// Construction
//==============================================================================

OptionsPanel::OptionsPanel()
{
    setupColumns();
    setupTitles();
    setupButtons();

    setupColumnInteractions();
    setupSolverPriorities();
    setupMinimizationModePanel();
}

//==============================================================================
// Initialisation UI
//==============================================================================

void OptionsPanel::setupColumns()
{
    addAndMakeVisible(column1);
    addAndMakeVisible(column5);
    addAndMakeVisible(voiceWorkspace);

    voiceWorkspace.onSpeciesChanged = [this](int counterpointIndex, int species)
    {
        if (appController == nullptr)
            return;

        auto& voiceSettings = appController->getVoiceSettings();
        if (counterpointIndex >= (int) voiceSettings.size())
            return;

        appController->updateVoice(counterpointIndex,
                                   species,
                                   voiceSettings[counterpointIndex].type);
    };

    voiceWorkspace.onTypeChanged = [this](int counterpointIndex, int type)
    {
        if (appController == nullptr)
            return;

        auto& voiceSettings = appController->getVoiceSettings();
        if (counterpointIndex >= (int) voiceSettings.size())
            return;

        appController->updateVoice(counterpointIndex,
                                   voiceSettings[counterpointIndex].species,
                                   type);
    };
}

void OptionsPanel::setupTitles()
{
    OptionsPanelHelpers::setupTitle(*this, title5, "");
}

void OptionsPanel::setupButtons()
{
    OptionsPanelHelpers::setupButton(*this, generateButton, "Generate");
    OptionsPanelHelpers::setupButton(*this, clearButton, "Clear");

    generateButton.onClick = [this]()
    {
        if (leftPanel != nullptr)
            leftPanel->triggerGeneration();
    };

    clearButton.onClick = [this]()
    {
        clearGenerationInputs();
    };
}

void OptionsPanel::setupSolverPriorities()
{
    //==========================================================================
    // Solver priorities
    //==========================================================================
    // Liste interactive du vecteur importance.
    // Une ligne sélectionnée peut être montée ou descendue avec les flèches.
    addAndMakeVisible(solverPriorityList);
    addAndMakeVisible(movePriorityUpButton);
    addAndMakeVisible(movePriorityDownButton);

    // Version compacte : elle vit maintenant sous le mode Lexicographic.
    solverPriorityList.setCompactMode(true);
    updateSolverPriorityVisibility();

    solverPriorityList.onPriorityOrderChanged = [this](const std::vector<int>& importanceCosts)
    {
        if (appController == nullptr || appController->isGenerating())
            return;

        auto& problem = appController->getProblem();

        problem.getSettings().setImportanceCosts(importanceCosts);
        problem.recalculateCosts();

        std::cout << "\n=== SOLVER PRIORITIES CHANGED ===\n";
        std::cout << "Importance costs = ";
        for (int cost : problem.getImportanceCosts())
            std::cout << cost << " ";
        std::cout << "\n";
    };

    movePriorityUpButton.onClick = [this]()
    {
        if (appController == nullptr || appController->isGenerating())
            return;

        solverPriorityList.moveSelectedUp();
    };

    movePriorityDownButton.onClick = [this]()
    {
        if (appController == nullptr || appController->isGenerating())
            return;

        solverPriorityList.moveSelectedDown();
    };
}

void OptionsPanel::setupMinimizationModePanel()
{
    //==========================================================================
    // Search / Minimization
    //==========================================================================
    // Panneau purement visuel pour choisir la strategie d'optimisation.
    // La connexion au solveur sera ajoutee dans une etape separee.
    addAndMakeVisible(minimizationModePanel);

    minimizationModePanel.onBabSearchMethodChanged = [this](bool useBab)
    {
        if (appController == nullptr || appController->isGenerating())
            return;

        auto& settings = appController->getProblem().getSettings();

        settings.setSearchMethod(useBab
            ? ConstraintSettings::SearchMethod::bab
            : ConstraintSettings::SearchMethod::dfs);

        std::cout << "\n=== SEARCH METHOD CHANGED ===\n";
        std::cout << "searchMethod = " << (useBab ? "BAB" : "DFS") << "\n";
    };

    minimizationModePanel.onLexicographicModeChanged = [this](bool isLexicographic)
    {
        showLexicographicPriorities = isLexicographic;
        updateSolverPriorityVisibility();
        resized();
    };
}

//==============================================================================
// Interactions des colonnes
//==============================================================================

void OptionsPanel::setupColumnInteractions()
{
    title5.onClick = nullptr;
    column5.onClick = nullptr;
}

void OptionsPanel::updateActiveColumn(int index)
{
    (void) index;

    activeColumn = 0;
    column5.isActive = false;
    repaint();
}

void OptionsPanel::updateSolverPriorityVisibility()
{
    solverPriorityList.setVisible(showLexicographicPriorities);
    movePriorityUpButton.setVisible(showLexicographicPriorities);
    movePriorityDownButton.setVisible(showLexicographicPriorities);
}

//==============================================================================
// Affichage
//==============================================================================

void OptionsPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
}

//==============================================================================
// Layout
//==============================================================================

void OptionsPanel::resized()
{
    const int panelWidth = getWidth();
    const int panelHeight = getHeight();

    // Les marges restent proportionnelles à la fenêtre.
    // La répartition des grandes zones est ensuite confiée à FlexBox.
    const int outerMargin = juce::jlimit(2, 6, panelWidth / 220);
    const int horizontalInset = juce::jlimit(4, 10, panelWidth / 130);
    const int verticalInset = juce::jlimit(3, 8, panelHeight / 110);
    const int bottomHeight = juce::jlimit(36, 48, panelHeight / 15);

    auto fullArea = getLocalBounds().reduced(outerMargin);
    auto bottomArea = fullArea.removeFromBottom(bottomHeight);
    auto contentArea = fullArea.reduced(horizontalInset, verticalInset);

    const int columnGap = juce::jlimit(5, 10, panelWidth / 120);
    const int titleHeight = juce::jlimit(18, 24, panelHeight / 42);
    const int titleGap = juce::jlimit(2, 4, panelHeight / 220);

    auto titleRow = contentArea.removeFromTop(titleHeight);
    contentArea.removeFromTop(titleGap);

    // FlexBox garde les proportions visuelles quand la fenêtre change :
    // - colonne d'entrée : largeur bornée
    // - workspace : prend tout l'espace restant
    // - Search : largeur bornée, vide pour l'instant
    juce::FlexBox mainRow;
    mainRow.flexDirection = juce::FlexBox::Direction::row;
    mainRow.alignItems = juce::FlexBox::AlignItems::stretch;
    mainRow.justifyContent = juce::FlexBox::JustifyContent::center;

    if (leftPanel != nullptr)
    {
        mainRow.items.add(juce::FlexItem(*leftPanel)
            .withFlex(0.0f, 1.0f, 220.0f)
            .withMinWidth(175.0f)
            .withMaxWidth(245.0f)
            .withMargin(juce::FlexItem::Margin(0.0f, (float) columnGap, 0.0f, 0.0f)));
    }

    mainRow.items.add(juce::FlexItem(column1)
        .withFlex(1.25f, 1.0f, 640.0f)
        .withMinWidth(390.0f)
        .withMargin(juce::FlexItem::Margin(0.0f, (float) columnGap, 0.0f, 0.0f)));

    mainRow.items.add(juce::FlexItem(column5)
        .withFlex(0.0f, 1.0f, 180.0f)
        .withMinWidth(150.0f)
        .withMaxWidth(210.0f)
        .withMargin(juce::FlexItem::Margin(0.0f)));
    mainRow.performLayout(contentArea);

    workspaceBounds = column1.getBounds();
    column1.setBounds(workspaceBounds);
    voiceWorkspace.setBounds(workspaceBounds.reduced(3));

    const auto solverBounds = column5.getBounds();
    title5.setBounds(solverBounds.getX(), titleRow.getY(), solverBounds.getWidth(), titleHeight);
    layoutMinimizationModePanel(solverBounds);

    layoutButtons(bottomArea);
}

void OptionsPanel::layoutMinimizationModePanel(juce::Rectangle<int> columnBounds)
{
    // Le panneau interne laisse respirer le contour de la colonne.
    auto area = columnBounds.reduced(9, 10);

    constexpr int modePanelHeight = 86;
    constexpr int listGap = 7;
    constexpr int arrowColumnWidth = 24;
    constexpr int arrowSize = 22;
    constexpr int arrowGap = 6;

    minimizationModePanel.setBounds(area.removeFromTop(modePanelHeight));

    if (! showLexicographicPriorities)
        return;

    area.removeFromTop(listGap);

    auto arrowArea = area.removeFromRight(arrowColumnWidth);
    auto listArea = area.reduced(0, 2);

    solverPriorityList.setBounds(listArea);

    const int totalArrowHeight = arrowSize * 2 + arrowGap;
    auto arrows = juce::Rectangle<int>(
        arrowArea.getCentreX() - arrowSize / 2,
        arrowArea.getCentreY() - totalArrowHeight / 2,
        arrowSize,
        totalArrowHeight
    );

    movePriorityUpButton.setBounds(arrows.removeFromTop(arrowSize));
    arrows.removeFromTop(arrowGap);
    movePriorityDownButton.setBounds(arrows.removeFromTop(arrowSize));
}

void OptionsPanel::layoutSolverPriorities(juce::Rectangle<int> listBounds)
{
    // La liste occupe toute la colonne.
    solverPriorityList.setBounds(listBounds);
}

void OptionsPanel::layoutButtons(juce::Rectangle<int> bottomArea)
{
    constexpr int buttonWidth = 110;
    constexpr int buttonHeight = 28;
    constexpr int spacing = 12;

    juce::FlexBox buttonRow;
    buttonRow.flexDirection = juce::FlexBox::Direction::row;
    buttonRow.alignItems = juce::FlexBox::AlignItems::center;
    buttonRow.justifyContent = juce::FlexBox::JustifyContent::center;

    buttonRow.items.add(juce::FlexItem(clearButton)
        .withWidth((float) buttonWidth)
        .withHeight((float) buttonHeight)
        .withMargin(juce::FlexItem::Margin(0.0f, (float) spacing / 2.0f, 0.0f, 0.0f)));

    buttonRow.items.add(juce::FlexItem(generateButton)
        .withWidth((float) buttonWidth)
        .withHeight((float) buttonHeight)
        .withMargin(juce::FlexItem::Margin(0.0f, 0.0f, 0.0f, (float) spacing / 2.0f)));

    buttonRow.performLayout(bottomArea);
}

//==============================================================================
// Nettoyage de la page
//==============================================================================

void OptionsPanel::clearGenerationInputs()
{
    if (leftPanel != nullptr)
        leftPanel->clearInputState();

    voiceWorkspace.resetCounterpointSelectors();
    setNumVoices(0);
}

//==============================================================================
// Synchronisation des voix
//==============================================================================

void OptionsPanel::setNumVoices(int numVoices)
{
    const int numCounterpoints = juce::jmax(0, numVoices - 1);
    voiceWorkspace.setActiveCounterpointCount(numCounterpoints);

    if (appController == nullptr)
        return;

    // Le workspace affiche uniquement les contrepoints.
    // On garde seulement les réglages internes par défaut pour que Generate fonctionne encore.
    appController->getVoiceSettings().resize(numCounterpoints);
}

juce::Rectangle<int> OptionsPanel::getWorkspaceBounds() const
{
    return workspaceBounds;
}

//==============================================================================
// Connexions externes
//==============================================================================

void OptionsPanel::setAppController(AppController* app_controller)
{
    appController = app_controller;

    if (appController != nullptr)
    {
        solverPriorityList.setImportanceCosts(
            appController->getProblem().getSettings().getImportanceCosts()
        );

    }
}

void OptionsPanel::setLeftPanel(LeftPanel* panel)
{
    leftPanel = panel;

    if (leftPanel != nullptr)
    {
        addAndMakeVisible(*leftPanel);
        resized();
    }
}

LeftPanel* OptionsPanel::getLeftPanel() const
{
    return leftPanel;
}
