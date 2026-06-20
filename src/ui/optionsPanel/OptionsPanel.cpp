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
    setupMelodicControls();
}

//==============================================================================
// Initialisation UI
//==============================================================================

void OptionsPanel::setupColumns()
{
    std::array<ColumnBox*, 4> columns { &column1, &column2, &column3, &column4 };

    for (auto* column : columns)
        addAndMakeVisible(*column);
}

void OptionsPanel::setupTitles()
{
    OptionsPanelHelpers::setupTitle(*this, title1, "Basic Constraints");
    OptionsPanelHelpers::setupTitle(*this, title2, "Melodic");
    OptionsPanelHelpers::setupTitle(*this, title3, "Harmonic");
    OptionsPanelHelpers::setupTitle(*this, title4, "Other");
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

void OptionsPanel::setupMelodicControls()
{
    //==========================================================================
    // Borrow Mode
    //==========================================================================
    auto* borrowToggle = addParameter<juce::ToggleButton>(
        melodicColumn,
        "Borrow Mode",
        std::make_unique<juce::ToggleButton>()
    );

    if (appController != nullptr)
    {
        borrowToggle->setToggleState(
            appController->getProblem().getSettings().getBorrowMode() == 1,
            juce::dontSendNotification
        );
    }

    borrowToggle->onClick = [this, borrowToggle]()
    {
        if (appController == nullptr || appController->isGenerating())
            return;

        auto& problem = appController->getProblem();

        const int value = borrowToggle->getToggleState() ? 1 : 0;

        problem.getSettings().setBorrowMode(value);
        problem.recalculateCosts();

        std::cout << "\n=== BORROW MODE CHANGED ===\n";
        std::cout << "borrowMode = "
                  << problem.getSettings().getBorrowMode()
                  << "\n";

    };

    //==========================================================================
    // Melodic Leap Control
    //==========================================================================
    auto* leapSlider = addParameter<juce::Slider>(
        melodicColumn,
        "Melodic Leaps",
        std::make_unique<juce::Slider>()
    );

    leapSlider->setSliderStyle(juce::Slider::LinearHorizontal);
    leapSlider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    leapSlider->setRange(0.0, 1.0, 0.01);
    leapSlider->setValue(0.0, juce::dontSendNotification);

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
        std::cout << "largeLeapPenalty = "
                  << problem.getSettings().getLargeLeapPenalty()
                  << "\n";
    };


    //==========================================================================
    // Repetion slider
    //==========================================================================
    auto* repetitionSlider = addParameter<juce::Slider>(
        melodicColumn,
        "Melodic Variety",
        ParameterFactory::slider(0.0, 1.0, 1.0, 0.0)
    );

    repetitionSlider->onValueChange = [this, repetitionSlider]()
    {
        if (appController == nullptr || appController->isGenerating())
            return;

        auto& problem = appController->getProblem();

        const int value = static_cast<int>(repetitionSlider->getValue());

        problem.getSettings().setNoteRepetitionValue(value);
        problem.recalculateCosts();

        std::cout << "\n=== NOTE REPETITION SLIDER CHANGED ===\n";
        std::cout << "noteRepetitionValue = "
                  << problem.getSettings().getNoteRepetitionValue()
                  << "\n";

        std::cout << "General costs = ";
        for (int c : problem.getGeneralCosts())
            std::cout << c << " ";
        std::cout << "\n";
    };
}

//==============================================================================
// Interactions des colonnes
//==============================================================================

void OptionsPanel::setupColumnInteractions()
{
    std::array<ClickableTitle*, 4> titles { &title1, &title2, &title3, &title4 };
    std::array<ColumnBox*, 4> columns { &column1, &column2, &column3, &column4 };

    for (int i = 0; i < 4; ++i)
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
}

//==============================================================================
// Layout
//==============================================================================

void OptionsPanel::resized()
{
    auto fullArea = getLocalBounds().reduced(20);
    auto bottomArea = fullArea.removeFromBottom(80);
    auto contentArea = fullArea.reduced(40, 30);

    constexpr int columnCount = 4;
    constexpr int columnWidth = 240;
    constexpr int columnGap = 14;
    constexpr int titleHeight = 28;
    constexpr int titleGap = 6;

    const int totalWidth = columnCount * columnWidth
                         + (columnCount - 1) * columnGap;

    const int startX = contentArea.getX()
                     + (contentArea.getWidth() - totalWidth) / 2;

    const int titleY = contentArea.getY();
    const int columnY = titleY + titleHeight + titleGap;
    const int columnHeight = juce::jmin(500, contentArea.getHeight() - 30);

    std::array<ClickableTitle*, 4> titles { &title1, &title2, &title3, &title4 };
    std::array<ColumnBox*, 4> columns { &column1, &column2, &column3, &column4 };

    std::array<juce::Rectangle<int>, 4> columnBounds;

    for (int i = 0; i < columnCount; ++i)
    {
        const int x = startX + i * (columnWidth + columnGap);

        titles[i]->setBounds(x, titleY, columnWidth, titleHeight);

        columnBounds[i] = { x, columnY, columnWidth, columnHeight };
        columns[i]->setBounds(columnBounds[i]);
    }

    layoutVoiceColumn(columnBounds[0]);
    melodicColumn.layout(columnBounds[1]);
    harmonicColumn.layout(columnBounds[2]);
    otherColumn.layout(columnBounds[3]);

    layoutButtons(bottomArea);
}

void OptionsPanel::layoutVoiceColumn(juce::Rectangle<int> bounds)
{
    auto inner = bounds.reduced(10);

    constexpr int boxHeight = 60;
    constexpr int gapY = 8;

    std::array<VoiceBox*, 4> boxes { &box1, &box2, &box3, &box4 };

    for (size_t i = 0; i < boxes.size(); ++i)
    {
        boxes[i]->setBounds(inner.removeFromTop(boxHeight));

        if (i + 1 < boxes.size())
            inner.removeFromTop(gapY);
    }
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
}

void OptionsPanel::setLeftPanel(LeftPanel* panel)
{
    leftPanel = panel;
}

LeftPanel* OptionsPanel::getLeftPanel() const
{
    return leftPanel;
}
