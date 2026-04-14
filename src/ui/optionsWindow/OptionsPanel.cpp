#include "OptionsPanel.h"
#include "../../controller/AppController.h"
#include "../leftPanel/LeftPanel.h"
#include "../../model/ConstraintsDefinition.h"
// =============================
// VoiceBox : UI d'une voix
// =============================
VoiceBox::VoiceBox(const juce::String& name)
{
    //  titre de la voix
    addAndMakeVisible(title);
    title.setText(name, juce::dontSendNotification);
    title.setJustificationType(juce::Justification::centred);
    title.setColour(juce::Label::textColourId, juce::Colours::white);

    //  choix de l'espèce
    addAndMakeVisible(speciesBox);
    for (int i = 1; i <= 5; ++i)
        speciesBox.addItem("Species " + juce::String(i), i);
    speciesBox.setSelectedId(1);

    //  choix du type
    addAndMakeVisible(typeBox);
    int id = 1;
    for (int i = -3; i <= 2; ++i)
        typeBox.addItem("Type " + juce::String(i), id++);
    typeBox.setSelectedId(1);

    // =========================
    // Connexion bouton UI → MODÈLE
    // =========================
    speciesBox.onChange = [this]()
    {
        // update leftpanel -> optionpanel
        if (settings)
            settings->species = speciesBox.getSelectedId();
        //update optionpanel -> leftpanel
        if (auto* parent = findParentComponentOfClass<OptionsPanel>())
        {
            if (auto* lp = parent->getLeftPanel())
                lp->refreshFromModel();
        }
    };

    //Connexion type entre leftpanel et OptionPanel
    typeBox.onChange = [this]()
    {
        // update leftpanel -> optionpanel
        if (settings)
            settings->type = typeBox.getSelectedId() - 4;
        //update optionpanel -> leftpanel
        if (auto* parent = findParentComponentOfClass<OptionsPanel>())
        {
            if (auto* lp = parent->getLeftPanel())
                lp->refreshFromModel();
        }
    };
}

void VoiceBox::paint(juce::Graphics& g)
{
    //  Highlight actif
    if (isActive)
        g.setColour(juce::Colour(0xff2f4f4f));

    else
        g.setColour(juce::Colours::darkgrey.brighter());

    g.fillRoundedRectangle(getLocalBounds().toFloat(), 6.0f);
}

void VoiceBox::resized()
{
    auto area = getLocalBounds().reduced(8);

    title.setBounds(area.removeFromTop(20));
    area.removeFromTop(5);

    auto row = area.removeFromTop(25);
    auto left = row.removeFromLeft(row.getWidth() / 2);

    speciesBox.setBounds(left.reduced(2));
    typeBox.setBounds(row.reduced(2));


}

// =============================
// OptionsPanel : panneau global
// =============================
OptionsPanel::OptionsPanel()
{
    //====== Affichage des colonnes =========
    addAndMakeVisible(column1);
    addAndMakeVisible(column2);
    addAndMakeVisible(column3);
    addAndMakeVisible(column4);


    addAndMakeVisible(title1);
    addAndMakeVisible(title2);
    addAndMakeVisible(title3);
    addAndMakeVisible(title4);

    title1.setText("Basic Constraints", juce::dontSendNotification);
    title2.setText("Melodic", juce::dontSendNotification);
    title3.setText("Feature 3", juce::dontSendNotification);
    title4.setText("Feature 4", juce::dontSendNotification);

    auto setupLabel = [](juce::Label& l)
    {
        l.setJustificationType(juce::Justification::centred);
        l.setColour(juce::Label::textColourId, juce::Colours::white);
        l.setFont(juce::Font(16.0f, juce::Font::bold));
    };

    setupLabel(title1);
    setupLabel(title2);
    setupLabel(title3);
    setupLabel(title4);

    //Affichage des voix colonnne 1
    addAndMakeVisible(box1);
    addAndMakeVisible(box2);
    addAndMakeVisible(box3);
    addAndMakeVisible(box4);

    //===== Melodic Constraints ==========

    // 1. ===== Max Leap ======
    addAndMakeVisible(melodicMaxLeapLabel);
    addAndMakeVisible(melodicMaxLeapSlider);

    melodicMaxLeapLabel.setText("Max Leap", juce::dontSendNotification);
    //affichage du tooltip (text) quand on passe la souris dessus de Max Leap
    auto info = constraintDB["MaxLeap"];
     melodicMaxLeapLabel.setTooltip(
        info.name + "\n\n" +
        info.description + "\n\n" +
        info.theory
    );

    melodicMaxLeapLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    melodicMaxLeapLabel.setJustificationType(juce::Justification::centredLeft);

    melodicMaxLeapSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    melodicMaxLeapSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    melodicMaxLeapSlider.setRange(2, 12, 1); // intervalle max
    melodicMaxLeapSlider.setValue(5); // valeur par défaut

    melodicMaxLeapSlider.onValueChange = [this]()
    {
        int value = (int) melodicMaxLeapSlider.getValue();

        if (appController != nullptr)
        {
            appController->getProblem().getSettings().rules.maxLeap = value;
        }
    };

    // 2. ===== Step bias ======
    addAndMakeVisible((melodicStepBiasLabel));
    addAndMakeVisible((melodicStepBiasSlider));

    melodicStepBiasLabel.setText("Step bias", juce::dontSendNotification);
    //affichage du tooltip (text) quand on passe la souris dessus de Step bias
    auto info2 = constraintDB["StepBias"];
    melodicStepBiasLabel.setTooltip(
       info2.name + "\n\n" +
       info2.description + "\n\n" +
       info2.theory
   );

    melodicStepBiasLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    melodicStepBiasLabel.setJustificationType(juce::Justification::centredLeft);

    melodicStepBiasSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    melodicStepBiasSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    melodicStepBiasSlider.setRange(-1, 1, 1);
    melodicStepBiasSlider.setValue(0);


    // 3. ====== Repetition allowed ======
    addAndMakeVisible(melodicRepetitionLabel);
    addAndMakeVisible(melodicRepetitionToggle);

    melodicRepetitionLabel.setText("Repetition", juce::dontSendNotification);
    //affichage du tooltip (text) quand on passe la souris dessus de Repetition
    auto info3 = constraintDB["Repetition"];
    melodicRepetitionLabel.setTooltip(
       info3.name + "\n\n" +
       info3.description + "\n\n" +
       info3.theory
   );
    melodicRepetitionLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    melodicRepetitionLabel.setJustificationType(juce::Justification::centredLeft);

    melodicRepetitionToggle.setButtonText("Allow");
    melodicRepetitionToggle.setToggleState(false, juce::dontSendNotification);

    // 4. ====== Direction prefered =====
    addAndMakeVisible(melodicDirectionLabel);
    addAndMakeVisible(melodicDirectionBox);

    melodicDirectionLabel.setText("Direction", juce::dontSendNotification);
    auto info4 = constraintDB["Direction"];
    melodicDirectionLabel.setTooltip(
       info4.name + "\n\n" +
       info4.description + "\n\n" +
       info4.theory
   );
    melodicDirectionLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    melodicDirectionLabel.setJustificationType(juce::Justification::centredLeft);

    melodicDirectionBox.addItem("Neutral", 1);
    melodicDirectionBox.addItem("Ascending", 2);
    melodicDirectionBox.addItem("Descending", 3);

    melodicDirectionBox.setSelectedId(1);





    //Affichage des boutons Generate/Cancel
    generateButton.setButtonText("Generate");
    cancel.setButtonText("Cancel");
    addAndMakeVisible(generateButton);
    addAndMakeVisible(cancel);

    // Display colonne active
    title1.onClick = [this]() { updateActiveColumn(1); };
    title2.onClick = [this]() { updateActiveColumn(2); };
    title3.onClick = [this]() { updateActiveColumn(3); };
    title4.onClick = [this]() { updateActiveColumn(4); };

    // Gestion du Hover Titre et colonne
    title1.onEnter = [this]()
    {
        hoveredColumn = 1;
        column1.isHovered = true;
        repaint();
    };

    title2.onEnter = [this]()
    {
        hoveredColumn = 2;
        column2.isHovered = true;
        repaint();
    };

    title3.onEnter = [this]()
    {
        hoveredColumn = 3;
        column3.isHovered = true;
        repaint();
    };

    title4.onEnter = [this]()
    {
        hoveredColumn = 4;
        column4.isHovered = true;
        repaint();
    };

    title1.onExit = [this]()
    {
        hoveredColumn = 0;
        column1.isHovered = false;
        repaint();
    };

    title2.onExit = [this]()
    {
        hoveredColumn = 0;
        column2.isHovered = false;
        repaint();
    };

    title3.onExit = [this]()
    {
        hoveredColumn = 0;
        column3.isHovered = false;
        repaint();
    };

    title4.onExit = [this]()
    {
        hoveredColumn = 0;
        column4.isHovered = false;
        repaint();
    };

    // ===== Hover colonnes =====
    column1.onEnter = [this]() { hoveredColumn = 1; column1.isHovered = true; repaint(); };
    column2.onEnter = [this]() { hoveredColumn = 2; column2.isHovered = true; repaint(); };
    column3.onEnter = [this]() { hoveredColumn = 3; column3.isHovered = true; repaint(); };
    column4.onEnter = [this]() { hoveredColumn = 4; column4.isHovered = true; repaint(); };

    column1.onExit = [this]() { column1.isHovered = false; repaint(); };
    column2.onExit = [this]() { column2.isHovered = false; repaint(); };
    column3.onExit = [this]() { column3.isHovered = false; repaint(); };
    column4.onExit = [this]() { column4.isHovered = false; repaint(); };

    // ===== Click colonnes =====
    column1.onClick = [this]() { updateActiveColumn(1); };
    column2.onClick = [this]() { updateActiveColumn(2); };
    column3.onClick = [this]() { updateActiveColumn(3); };
    column4.onClick = [this]() { updateActiveColumn(4); };

    generateButton.onClick = [this]()
    {
        if (leftPanel != nullptr)
            leftPanel->triggerGeneration();
    };

}

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

    melodicMaxLeapLabel.setBounds(left1);
    melodicMaxLeapSlider.setBounds(row1.reduced(0, 6));
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

// =============================
//  Set active voices
// =============================
void OptionsPanel::setNumVoices(int numVoices)
{
    std::vector<VoiceBox*> boxes = { &box1, &box2, &box3, &box4 };

    for (int i = 0; i < boxes.size(); ++i)
    {
        boxes[i]->isActive = (i < numVoices);
        boxes[i]->repaint();
    }
}

void OptionsPanel::setVoiceSettings(std::vector<AppController::VoiceSettings>& settings)
{
    std::vector<VoiceBox*> boxes = { &box1, &box2, &box3, &box4 };

    for (int i = 0; i < boxes.size(); ++i)
    {
        if (i < settings.size())
        {
            boxes[i]->settings = &settings[i];

            boxes[i]->speciesBox.setSelectedId(settings[i].species);
            boxes[i]->typeBox.setSelectedId(settings[i].type + 4);
        }
    }
}

void OptionsPanel::updateActiveColumn(int columnIndex)
{
    activeColumn = columnIndex;

    column1.isActive = (activeColumn == 1);
    column2.isActive = (activeColumn == 2);
    column3.isActive = (activeColumn == 3);
    column4.isActive = (activeColumn == 4);

    repaint();
}

