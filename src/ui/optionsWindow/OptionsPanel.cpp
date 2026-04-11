#include "OptionsPanel.h"
#include "../../controller/AppController.h"
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
        if (settings)
            settings->species = speciesBox.getSelectedId();
    };

    typeBox.onChange = [this]()
    {
        if (settings)
            settings->type = typeBox.getSelectedId() - 4;
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
    melodicMaxLeapLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    melodicMaxLeapLabel.setJustificationType(juce::Justification::centredLeft);

    melodicMaxLeapSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    melodicMaxLeapSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    melodicMaxLeapSlider.setRange(2, 12, 1); // intervalle max
    melodicMaxLeapSlider.setValue(5); // valeur par défaut

    // 2. ===== Step bias ======
    addAndMakeVisible((melodicStepBiasLabel));
    addAndMakeVisible((melodicStepBiasSlider));

    melodicStepBiasLabel.setText("Step bias", juce::dontSendNotification);
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
    melodicRepetitionLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    melodicRepetitionLabel.setJustificationType(juce::Justification::centredLeft);

    melodicRepetitionToggle.setButtonText("Allow");
    melodicRepetitionToggle.setToggleState(false, juce::dontSendNotification);

    // 4. ====== Direction prefered =====
    addAndMakeVisible(melodicDirectionLabel);
    addAndMakeVisible(melodicDirectionBox);

    melodicDirectionLabel.setText("Direction", juce::dontSendNotification);
    melodicDirectionLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    melodicDirectionLabel.setJustificationType(juce::Justification::centredLeft);

    melodicDirectionBox.addItem("Neutral", 1);
    melodicDirectionBox.addItem("Ascending", 2);
    melodicDirectionBox.addItem("Descending", 3);

    melodicDirectionBox.setSelectedId(1);





//Affichage des boutons Generate/Cancel
    generate.setButtonText("Generate");
    cancel.setButtonText("Cancel");
    addAndMakeVisible(generate);
    addAndMakeVisible(cancel);


}

void OptionsPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
}

void OptionsPanel::resized()
{
    auto fullArea = getLocalBounds().reduced(20);

    // zone boutons en bas
    auto bottomArea = fullArea.removeFromBottom(80);

    auto area = fullArea;

    const int gap = 10;
    const int titleHeight = 28;
    const int width = (area.getWidth() - 3 * gap) / 4;

    // =============================
    // Colonne 1 : Basic Constraints
    // =============================
    auto col1Area = area.removeFromLeft(width);
    title1.setBounds(col1Area.removeFromTop(titleHeight));
    column1.setBounds(col1Area);

    auto inner = col1Area.reduced(10);

    const int boxHeight = 60;   // 🔥 plus compact
    const int gapY = 8;

    box1.setBounds(inner.removeFromTop(boxHeight));
    inner.removeFromTop(gapY);

    box2.setBounds(inner.removeFromTop(boxHeight));
    inner.removeFromTop(gapY);

    box3.setBounds(inner.removeFromTop(boxHeight));
    inner.removeFromTop(gapY);

    box4.setBounds(inner.removeFromTop(boxHeight));

    // =============================
    // Colonne 2 (Melodic)
    // =============================
    area.removeFromLeft(gap);

    auto col2Area = area.removeFromLeft(width);
    title2.setBounds(col2Area.removeFromTop(titleHeight));
    column2.setBounds(col2Area);

    auto inner2 = col2Area.reduced(12);

    const int rowHeight = 26;     // hauteur des lignes
    const int spacingY = 10;
    const int labelWidth = 95;    // taille des labels
    const int gapX = 12;          //  ESPACE entre label et contenu

    // ===== 1. Max Leap =====
    auto row1 = inner2.removeFromTop(rowHeight);
    auto left1 = row1.removeFromLeft(labelWidth);
    row1.removeFromLeft(gapX);

    melodicMaxLeapLabel.setBounds(left1);
    melodicMaxLeapSlider.setBounds(row1.reduced(0, 6)); // slider plus fin
    inner2.removeFromTop(spacingY);

    // ===== 2. Step =====
    auto row2 = inner2.removeFromTop(rowHeight);
    auto left2 = row2.removeFromLeft(labelWidth);
    row2.removeFromLeft(gapX);

    melodicStepBiasLabel.setBounds(left2);
    melodicStepBiasSlider.setBounds(row2.reduced(0, 6));
    inner2.removeFromTop(spacingY);

    // ===== 3. Repetion allowed =====
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
    area.removeFromLeft(gap);

    auto col3Area = area.removeFromLeft(width);
    title3.setBounds(col3Area.removeFromTop(titleHeight));
    column3.setBounds(col3Area);

    // =============================
    // Colonne 4
    // =============================
    area.removeFromLeft(gap);

    auto col4Area = area.removeFromLeft(width);
    title4.setBounds(col4Area.removeFromTop(titleHeight));
    column4.setBounds(col4Area);

    // =============================
    // Bottom buttons
    // =============================
    const int buttonWidth = 110;   //
    const int buttonHeight = 28;
    const int spacing = 12;

    int totalWidth = 2 * buttonWidth + spacing;
    int startX = (getWidth() - totalWidth) / 2;
    int y = bottomArea.getY() + (bottomArea.getHeight() - buttonHeight) / 2;

    cancel.setBounds(startX, y, buttonWidth, buttonHeight);
    generate.setBounds(startX + buttonWidth + spacing, y, buttonWidth, buttonHeight);
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

