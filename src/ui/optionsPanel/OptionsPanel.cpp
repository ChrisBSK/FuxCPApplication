#include "OptionsPanel.h"
#include "../leftPanel/LeftPanel.h"
#include "../../controller/AppController.h"
#include "OptionsPanelStyle.h"
#include "OptionsPanelHelpers.h"


//==============================================================================
// Constructor
//==============================================================================

OptionsPanel::OptionsPanel()
{
    // =========================
    // Colonnes
    // =========================
    addAndMakeVisible(column1);
    addAndMakeVisible(column2);
    addAndMakeVisible(column3);
    addAndMakeVisible(column4);

    // =========================
    // Titres
    // =========================
    OptionsPanelHelpers::setupTitle(*this, title1, "Basic Constraints");
    OptionsPanelHelpers::setupTitle(*this, title2, "Melodic");
    OptionsPanelHelpers::setupTitle(*this, title3, "Feature 3");
    OptionsPanelHelpers::setupTitle(*this, title4, "Feature 4");

    // =========================
    // Voice boxes
    // =========================
    addAndMakeVisible(box1);
    addAndMakeVisible(box2);
    addAndMakeVisible(box3);
    addAndMakeVisible(box4);


    // =========================
    // Boutons
    // =========================
    addAndMakeVisible(generateButton);
    addAndMakeVisible(cancel);

    generateButton.setButtonText("Generate");
    cancel.setButtonText("Cancel");



    generateButton.onClick = [this]()
    {
        if (leftPanel)
            leftPanel->triggerGeneration();
    };

    // =========================
    // Interactions colonnes
    // =========================
    setupColumnInteractions();

    std::array<ClickableTitle*, 4> titles = { &title1, &title2, &title3, &title4 };
    std::array<ColumnBox*, 4> columns = { &column1, &column2, &column3, &column4 };

    for (int i = 0; i < 4; ++i)
    {
        setupHover(*titles[i], *columns[i], i + 1);
    }

    // =========================
    // Contraintes mélodiques
    // =========================
    setupMelodicControls();

}

void OptionsPanel::setupMelodicControls()
{
    // =========================
    // Label
    // =========================
    addAndMakeVisible(melodicVarietyLabel);
    melodicVarietyLabel.setText("Variety", juce::dontSendNotification);

    // =========================
    // Sliders
    // =========================
    addAndMakeVisible(melodicVarietySlider);

    melodicVarietySlider.setRange(0, 100, 10);
    melodicVarietySlider.setValue(10, juce::dontSendNotification);
    melodicVarietySlider.setSliderStyle(juce::Slider::LinearHorizontal);
    melodicVarietySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);

    // =========================
    // Connexion aux coûts dans les vecteurs d'entrée
    // =========================
    /*melodicVarietySlider.onValueChange = [this]()
    {
        if (appController == nullptr || appController->isGenerating())
            return;

        auto& settings = appController->getProblem().getSettings();

        if (settings.soft.melodic.size() < 8)
            settings.soft.melodic.resize(8, 0);

        settings.soft.melodic[0] = (int) melodicVarietySlider.getValue();

        std::cout << "melodic[0] = " << settings.soft.melodic[0] << std::endl;


    };*/

    melodicVarietySlider.onValueChange = [this]() {
    int value = static_cast<int>(melodicVarietySlider.getValue());

    // Récupère les paramètres actuels
    auto settings = appController->getProblem().getSettings();

    // Met à jour leapPenalty
    settings.leapPenalty = value;

    // Met à jour les paramètres et recalcule les coûts
    appController->updateSettings(settings);
    };
}

void OptionsPanel::setupColumnInteractions()
{
    // Click titres
    title1.onClick = [this]() { updateActiveColumn(1); };
    title2.onClick = [this]() { updateActiveColumn(2); };
    title3.onClick = [this]() { updateActiveColumn(3); };
    title4.onClick = [this]() { updateActiveColumn(4); };

    // Hover titres
    title1.onEnter = [this]() { hoveredColumn = 1; column1.isHovered = true; repaint(); };
    title2.onEnter = [this]() { hoveredColumn = 2; column2.isHovered = true; repaint(); };
    title3.onEnter = [this]() { hoveredColumn = 3; column3.isHovered = true; repaint(); };
    title4.onEnter = [this]() { hoveredColumn = 4; column4.isHovered = true; repaint(); };

    title1.onExit = [this]() { hoveredColumn = 0; column1.isHovered = false; repaint(); };
    title2.onExit = [this]() { hoveredColumn = 0; column2.isHovered = false; repaint(); };
    title3.onExit = [this]() { hoveredColumn = 0; column3.isHovered = false; repaint(); };
    title4.onExit = [this]() { hoveredColumn = 0; column4.isHovered = false; repaint(); };

    // Hover colonnes
    column1.onEnter = [this]() { hoveredColumn = 1; column1.isHovered = true; repaint(); };
    column2.onEnter = [this]() { hoveredColumn = 2; column2.isHovered = true; repaint(); };
    column3.onEnter = [this]() { hoveredColumn = 3; column3.isHovered = true; repaint(); };
    column4.onEnter = [this]() { hoveredColumn = 4; column4.isHovered = true; repaint(); };

    column1.onExit = [this]() { column1.isHovered = false; repaint(); };
    column2.onExit = [this]() { column2.isHovered = false; repaint(); };
    column3.onExit = [this]() { column3.isHovered = false; repaint(); };
    column4.onExit = [this]() { column4.isHovered = false; repaint(); };

    // Click colonnes
    column1.onClick = [this]() { updateActiveColumn(1); };
    column2.onClick = [this]() { updateActiveColumn(2); };
    column3.onClick = [this]() { updateActiveColumn(3); };
    column4.onClick = [this]() { updateActiveColumn(4); };
}

void OptionsPanel::setupHover(ClickableTitle& title,
                              ColumnBox& column,
                              int index)
{
    //// Gestion du Hover Titre et colonne
    title.onEnter = [this, &column, index]()
    {
        hoveredColumn = index;
        column.isHovered = true;
        repaint();
    };

    title.onExit = [this, &column]()
    {
        hoveredColumn = 0;
        column.isHovered = false;
        repaint();
    };
}

//==============================================================================
// Paint
//==============================================================================

void OptionsPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);

    auto drawTitle = [&](juce::Label& title, int index)
    {
        auto bounds = title.getBounds().toFloat().reduced(2);

        if (activeColumn == index)
        {
            // colonne active
            g.setColour(juce::Colour(0xff2f4f4f));
            g.fillRoundedRectangle(bounds, 6.0f);
        }
        else if (hoveredColumn == index)
        {
            // hover sur une autre colonne
            g.setColour(juce::Colour(0xff2f4f4f).withAlpha(0.8f));
            g.fillRoundedRectangle(bounds, 6.0f);
        }
    };

    drawTitle(title1, 1);
    drawTitle(title2, 2);
    drawTitle(title3, 3);
    drawTitle(title4, 4);
}
//==============================================================================
// Layout
//==============================================================================

void OptionsPanel::resized()
{
    auto fullArea = getLocalBounds().reduced(20);

    // zone boutons en bas
    auto bottomArea = fullArea.removeFromBottom(80);

    // =============================
    // zone centrale volontairement plus petite
    // pour que les colonnes soient moins larges et moins hautes
    // =============================
    auto area = fullArea.reduced(40, 30);

    const int gap = 14;
    const int titleHeight = 28;

    // colonnes un peu moins larges que toute la zone dispo
    const int columnWidth = 240;

    // =============================
    // calcul pour centrer les 4 colonnes
    // =============================
    const int totalColumnsWidth = 4 * columnWidth + 3 * gap;
    int startX = area.getX() + (area.getWidth() - totalColumnsWidth) / 2;

    // hauteur volontairement réduite
    const int columnHeight = juce::jmin(500, area.getHeight() - 30);

    // Y de départ commun
    int titleY = area.getY();
    int columnY = titleY + titleHeight + 6;

    // =============================
    // Colonne 1 : Basic Constraints
    // =============================
    juce::Rectangle<int> col1Bounds(startX, columnY, columnWidth, columnHeight);
    title1.setBounds(startX, titleY, columnWidth, titleHeight);
    column1.setBounds(col1Bounds);

    auto inner = col1Bounds.reduced(10);

    const int boxHeight = 60;
    const int gapY = 8;

    box1.setBounds(inner.removeFromTop(boxHeight));
    inner.removeFromTop(gapY);

    box2.setBounds(inner.removeFromTop(boxHeight));
    inner.removeFromTop(gapY);

    box3.setBounds(inner.removeFromTop(boxHeight));
    inner.removeFromTop(gapY);

    box4.setBounds(inner.removeFromTop(boxHeight));

    // =============================
    // Colonne 2 : Melodic
    // =============================
    int col2X = startX + columnWidth + gap;
    juce::Rectangle<int> col2Bounds(col2X, columnY, columnWidth, columnHeight);
    title2.setBounds(col2X, titleY, columnWidth, titleHeight);
    column2.setBounds(col2Bounds);

    auto inner2 = col2Bounds.reduced(12);

    const int rowHeight = 26;
    const int spacingY = 10;
    const int labelWidth = 95;
    const int gapX = 12;

    // ===== 1. Max Leap =====
    auto row1 = inner2.removeFromTop(rowHeight);
    auto left1 = row1.removeFromLeft(labelWidth);
    row1.removeFromLeft(gapX);

    melodicVarietyLabel.setBounds(left1);
    melodicVarietySlider.setBounds(row1.reduced(0, 6));
    inner2.removeFromTop(spacingY);

    // ===== 2. Step =====
    auto row2 = inner2.removeFromTop(rowHeight);
    auto left2 = row2.removeFromLeft(labelWidth);
    row2.removeFromLeft(gapX);

    melodicStepBiasLabel.setBounds(left2);
    melodicStepBiasSlider.setBounds(row2.reduced(0, 6));
    inner2.removeFromTop(spacingY);

    // ===== 3. Repetition allowed =====
    auto row3 = inner2.removeFromTop(rowHeight);
    auto left3 = row3.removeFromLeft(labelWidth);
    row3.removeFromLeft(gapX);

    melodicRepetitionLabel.setBounds(left3);
    melodicRepetitionToggle.setBounds(row3.reduced(0, 3));
    inner2.removeFromTop(spacingY);

    // ===== 4. Direction =====
    auto row4 = inner2.removeFromTop(rowHeight);
    auto left4 = row4.removeFromLeft(labelWidth);
    row4.removeFromLeft(gapX);

    melodicDirectionLabel.setBounds(left4);
    melodicDirectionBox.setBounds(row4.reduced(0, 3));

    // =============================
    // Colonne 3
    // =============================
    int col3X = col2X + columnWidth + gap;
    juce::Rectangle<int> col3Bounds(col3X, columnY, columnWidth, columnHeight);
    title3.setBounds(col3X, titleY, columnWidth, titleHeight);
    column3.setBounds(col3Bounds);

    // =============================
    // Colonne 4
    // =============================
    int col4X = col3X + columnWidth + gap;
    juce::Rectangle<int> col4Bounds(col4X, columnY, columnWidth, columnHeight);
    title4.setBounds(col4X, titleY, columnWidth, titleHeight);
    column4.setBounds(col4Bounds);

    // =============================
    // Bottom buttons
    // on garde leur taille actuelle
    // =============================
    const int buttonWidth = 110;
    const int buttonHeight = 28;
    const int spacing = 12;

    int totalWidth = 2 * buttonWidth + spacing;
    int startButtonsX = (getWidth() - totalWidth) / 2;
    int y = bottomArea.getY() + (bottomArea.getHeight() - buttonHeight) / 2;

    cancel.setBounds(startButtonsX, y, buttonWidth, buttonHeight);
    generateButton.setBounds(startButtonsX + buttonWidth + spacing, y, buttonWidth, buttonHeight);
}


//==============================================================================
// Synchronisation : nombre de voix
//==============================================================================

void OptionsPanel::setNumVoices(int numVoices)
{
    if (appController == nullptr)
        return;

    std::vector<VoiceBox*> boxes = { &box1, &box2, &box3, &box4 };

    // resize modèle
    int numCP = juce::jmax(0, numVoices - 1);
    appController->getVoiceSettings().resize(numCP);

    for (size_t i = 0; i < boxes.size(); ++i)
    {
        boxes[i]->setActive(i < (size_t)numVoices);

        if (i == 0)
        {
            // CF
            boxes[i]->speciesBox.setVisible(false);
            boxes[i]->typeBox.setVisible(false);
        }
        else if (i >= 1 && i < (size_t)numVoices)
        {
            int cpIndex = i - 1;
            //  CP
            boxes[i]->speciesBox.setVisible(true);
            boxes[i]->typeBox.setVisible(true);

            boxes[i]->connectToController(appController, cpIndex);
        }
        boxes[i]->repaint();
    }
}


//==============================================================================
// Colonne active (UI only)
//==============================================================================

void OptionsPanel::updateActiveColumn(int index)
{
    activeColumn = index;

    column1.isActive = (index == 1);
    column2.isActive = (index == 2);
    column3.isActive = (index == 3);
    column4.isActive = (index == 4);

    repaint();
}

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