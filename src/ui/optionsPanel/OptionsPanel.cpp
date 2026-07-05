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
    setupVoiceBoxes();
    setupButtons();

    setupColumnInteractions();
    setupBasicControls();
    setupMelodicControls();
    setupSolverPriorities();
}

//==============================================================================
// Initialisation UI
//==============================================================================

void OptionsPanel::setupColumns()
{
    std::array<ColumnBox*, 5> columns {
        &column1, &column2, &column3, &column4, &column5
    };

    for (auto* column : columns)
        addAndMakeVisible(*column);
}

void OptionsPanel::setupTitles()
{
    OptionsPanelHelpers::setupTitle(*this, title1, "Basic Constraints");
    OptionsPanelHelpers::setupTitle(*this, title2, "Melodic Lines");
    OptionsPanelHelpers::setupTitle(*this, title3, "Harmonic Relations");
    OptionsPanelHelpers::setupTitle(*this, title4, "Structural Preferences");
    OptionsPanelHelpers::setupTitle(*this, title5, "Solver Priorities");
}

void OptionsPanel::setupVoiceBoxes()
{
    std::array<VoiceBox*, 4> boxes { &box1, &box2, &box3, &box4 };

    for (auto* box : boxes)
        addAndMakeVisible(*box);
}

void OptionsPanel::setupButtons()
{
    OptionsPanelHelpers::setupButton(*this, generateButton, "Generate");
    OptionsPanelHelpers::setupButton(*this, cancel, "Cancel");

    generateButton.onClick = [this]()
    {
        if (leftPanel != nullptr)
            leftPanel->triggerGeneration();
    };
}

void OptionsPanel::setupBasicControls()
{
    //==========================================================================
    // Borrow Mode
    //==========================================================================
    auto* borrowSwitch = addSwitchParameter(
        basicColumn,
        "Borrow Mode",
        false
    );

    if (appController != nullptr)
    {
        borrowSwitch->setOn(
            appController->getProblem().getSettings().getBorrowMode() == 1,
            juce::dontSendNotification
        );
    }

    borrowSwitch->onClick = [this, borrowSwitch]()
    {
        if (appController == nullptr || appController->isGenerating())
            return;

        auto& problem = appController->getProblem();

        const int value = borrowSwitch->isOn() ? 1 : 0;

        problem.getSettings().setBorrowMode(value);
        problem.recalculateCosts();

        std::cout << "\n=== BORROW MODE CHANGED ===\n";
        std::cout << "borrowMode = "
                  << problem.getSettings().getBorrowMode()
                  << "\n";

    };
}

void OptionsPanel::setupMelodicControls()
{
    //==========================================================================
    // Melodic Leap Control
    //==========================================================================
    auto* leapSlider = addSliderParameter(
        melodicColumn,
        "Melody movement",
        0.0,
        1.0,
        0.01,
        0.5,
        "Smooth",
        "Jumpy"
    );

    if (appController != nullptr)
    {
        leapSlider->setValue(
            appController->getProblem().getSettings().getLargeLeapPenalty(),
            juce::dontSendNotification
        );
    }

    leapSlider->onValueChange = [this, leapSlider]()
    {
        if (appController == nullptr || appController->isGenerating())
            return;

        auto& problem = appController->getProblem();

        const double largeLeapPenalty = leapSlider->getValue();

        problem.getSettings().setLargeLeapPenalty(largeLeapPenalty);
        problem.recalculateCosts();

        std::cout << "\n=== MELODIC LEAPS CHANGED ===\n";
        std::cout << "avoidLargeMelodicLeap = "
                  << problem.getSettings().getLargeLeapPenalty()
                  << "\n";
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

//==============================================================================
// Interactions des colonnes
//==============================================================================

void OptionsPanel::setupColumnInteractions()
{
    std::array<ClickableTitle*, 5> titles {
        &title1, &title2, &title3, &title4, &title5
    };

    std::array<ColumnBox*, 5> columns {
        &column1, &column2, &column3, &column4, &column5
    };

    for (int i = 0; i < static_cast<int>(columns.size()); ++i)
    {
        const int columnIndex = i + 1;

        titles[i]->onClick  = [this, columnIndex]() { updateActiveColumn(columnIndex); };
        columns[i]->onClick = [this, columnIndex]() { updateActiveColumn(columnIndex); };

        setupHover(*titles[i], *columns[i], columnIndex);
    }
}

void OptionsPanel::setupHover(ClickableTitle& title,
                              ColumnBox& column,
                              int index)
{
    title.onEnter = column.onEnter = [this, &column, index]()
    {
        hoveredColumn = index;
        column.isHovered = true;
        repaint();
    };

    title.onExit = column.onExit = [this, &column]()
    {
        hoveredColumn = 0;
        column.isHovered = false;
        repaint();
    };
}

void OptionsPanel::updateActiveColumn(int index)
{
    activeColumn = index;

    column1.isActive = index == 1;
    column2.isActive = index == 2;
    column3.isActive = index == 3;
    column4.isActive = index == 4;
    column5.isActive = index == 5;

    repaint();
}

//==============================================================================
// Affichage
//==============================================================================

void OptionsPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);

    auto drawTitleHighlight = [&](juce::Label& title, int index)
    {
        if (activeColumn != index && hoveredColumn != index)
            return;

        auto bounds = title.getBounds().toFloat().reduced(2.0f);

        g.setColour(activeColumn == index
                    ? juce::Colour(0xff2f4f4f)
                    : juce::Colour(0xff2f4f4f).withAlpha(0.8f));

        g.fillRoundedRectangle(bounds, 6.0f);
    };

    drawTitleHighlight(title1, 1);
    drawTitleHighlight(title2, 2);
    drawTitleHighlight(title3, 3);
    drawTitleHighlight(title4, 4);
    drawTitleHighlight(title5, 5);
}

//==============================================================================
// Layout
//==============================================================================

void OptionsPanel::resized()
{
    const int panelWidth = getWidth();
    const int panelHeight = getHeight();

    // Les dimensions suivent la taille disponible.
    // Les cinq colonnes restent donc visibles quand la fenêtre est réduite.
    const int outerMargin = juce::jlimit(6, 20, panelWidth / 80);
    const int horizontalInset = juce::jlimit(8, 40, panelWidth / 45);
    const int verticalInset = juce::jlimit(8, 30, panelHeight / 35);
    const int bottomHeight = juce::jlimit(52, 80, panelHeight / 9);

    auto fullArea = getLocalBounds().reduced(outerMargin);
    auto bottomArea = fullArea.removeFromBottom(bottomHeight);
    auto contentArea = fullArea.reduced(horizontalInset, verticalInset);

    constexpr int columnCount = 5;
    const int columnGap = juce::jlimit(6, 14, panelWidth / 90);
    const int solverArrowGap = juce::jlimit(5, 8, panelWidth / 150);
    const int solverArrowWidth = juce::jlimit(24, 32, panelWidth / 45);
    const int titleHeight = juce::jlimit(22, 28, panelHeight / 30);
    const int titleGap = juce::jlimit(4, 6, panelHeight / 140);

    const int availableWidth = contentArea.getWidth()
                             - (columnCount - 1) * columnGap
                             - solverArrowGap
                             - solverArrowWidth;

    const int columnWidth = juce::jmax(1, availableWidth / columnCount);
    const int totalWidth = columnCount * columnWidth
                         + (columnCount - 1) * columnGap;

    const int startX = contentArea.getX()
                     + (contentArea.getWidth() - totalWidth) / 2;

    const int titleY = contentArea.getY();
    const int columnY = titleY + titleHeight + titleGap;
    const int columnHeight = juce::jmax(1, contentArea.getBottom() - columnY);

    std::array<ClickableTitle*, 5> titles {
        &title1, &title2, &title3, &title4, &title5
    };

    std::array<ColumnBox*, 5> columns {
        &column1, &column2, &column3, &column4, &column5
    };

    std::array<juce::Rectangle<int>, 5> columnBounds;

    for (int i = 0; i < columnCount; ++i)
    {
        const int x = startX + i * (columnWidth + columnGap);

        titles[i]->setBounds(x, titleY, columnWidth, titleHeight);

        columnBounds[i] = { x, columnY, columnWidth, columnHeight };
        columns[i]->setBounds(columnBounds[i]);
    }

    const juce::Rectangle<int> solverArrowBounds {
        columnBounds[4].getRight() + solverArrowGap,
        columnY,
        solverArrowWidth,
        columnHeight
    };

    layoutVoiceColumn(columnBounds[0]);
    melodicColumn.layout(columnBounds[1]);
    harmonicColumn.layout(columnBounds[2]);
    otherColumn.layout(columnBounds[3]);
    solverColumn.layout(columnBounds[4]);
    layoutSolverPriorities(columnBounds[4], solverArrowBounds);

    layoutButtons(bottomArea);
}

void OptionsPanel::layoutVoiceColumn(juce::Rectangle<int> bounds)
{
    const int inset = juce::jlimit(6, 10, bounds.getWidth() / 24);
    auto inner = bounds.reduced(inset);

    const int gapY = juce::jlimit(4, 8, bounds.getHeight() / 70);
    const int controlsGapY = gapY;
    const int basicControlsHeight = juce::jlimit(54, 60, bounds.getHeight() / 8);
    const int availableForVoices = inner.getHeight()
                                - basicControlsHeight
                                - controlsGapY
                                - 3 * gapY;
    const int boxHeight = juce::jlimit(42, 60, availableForVoices / 4);

    std::array<VoiceBox*, 4> boxes { &box1, &box2, &box3, &box4 };

    for (size_t i = 0; i < boxes.size(); ++i)
    {
        boxes[i]->setBounds(inner.removeFromTop(boxHeight));

        if (i + 1 < boxes.size())
            inner.removeFromTop(gapY);
    }

    inner.removeFromTop(controlsGapY);
    basicColumn.layout(inner, false);
}

void OptionsPanel::layoutSolverPriorities(juce::Rectangle<int> listBounds,
                                          juce::Rectangle<int> arrowBounds)
{
    // La liste occupe toute la colonne.
    // Les flèches vivent dans une bande externe à droite.
    const int gap = juce::jlimit(4, 7, arrowBounds.getWidth() / 4);
    const int buttonWidth = juce::jlimit(22, 30, arrowBounds.getWidth());
    const int buttonHeight = juce::jlimit(22, 30, arrowBounds.getHeight() / 14);

    solverPriorityList.setBounds(listBounds);

    const int totalButtonHeight = 2 * buttonHeight + gap;
    const int buttonY = arrowBounds.getY()
                    + (arrowBounds.getHeight() - totalButtonHeight) / 2;

    movePriorityUpButton.setBounds(arrowBounds.getX(),
                                   buttonY,
                                   buttonWidth,
                                   buttonHeight);

    movePriorityDownButton.setBounds(arrowBounds.getX(),
                                     buttonY + buttonHeight + gap,
                                     buttonWidth,
                                     buttonHeight);
}

void OptionsPanel::layoutButtons(juce::Rectangle<int> bottomArea)
{
    constexpr int buttonWidth = 110;
    constexpr int buttonHeight = 28;
    constexpr int spacing = 12;

    const int totalWidth = 2 * buttonWidth + spacing;
    const int startX = (getWidth() - totalWidth) / 2;
    const int y = bottomArea.getY()
                + (bottomArea.getHeight() - buttonHeight) / 2;

    cancel.setBounds(startX, y, buttonWidth, buttonHeight);

    generateButton.setBounds(startX + buttonWidth + spacing,
                             y,
                             buttonWidth,
                             buttonHeight);
}

//==============================================================================
// Synchronisation des voix
//==============================================================================

void OptionsPanel::setNumVoices(int numVoices)
{
    if (appController == nullptr)
        return;

    const int numCounterpoints = juce::jmax(0, numVoices - 1);
    appController->getVoiceSettings().resize(numCounterpoints);

    std::array<VoiceBox*, 4> boxes { &box1, &box2, &box3, &box4 };

    for (size_t i = 0; i < boxes.size(); ++i)
    {
        auto* box = boxes[i];

        const bool isVisibleVoice = i < static_cast<size_t>(numVoices);
        const bool isCantusFirmus = i == 0;
        const bool isCounterpoint = isVisibleVoice && !isCantusFirmus;

        box->setActive(isVisibleVoice);

        box->speciesBox.setVisible(isCounterpoint);
        box->typeBox.setVisible(isCounterpoint);

        if (isCounterpoint)
            box->connectToController(appController, static_cast<int>(i) - 1);

        box->repaint();
    }
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
}

LeftPanel* OptionsPanel::getLeftPanel() const
{
    return leftPanel;
}
