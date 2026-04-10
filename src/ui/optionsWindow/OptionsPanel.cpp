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
    addAndMakeVisible(column1);
    addAndMakeVisible(column2);
    addAndMakeVisible(column3);
    addAndMakeVisible(column4);

    addAndMakeVisible(title1);
    addAndMakeVisible(title2);
    addAndMakeVisible(title3);
    addAndMakeVisible(title4);

    title1.setText("Basic Constraints", juce::dontSendNotification);
    title2.setText("Feature 2", juce::dontSendNotification);
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

    addAndMakeVisible(box1);
    addAndMakeVisible(box2);
    addAndMakeVisible(box3);
    addAndMakeVisible(box4);
}

void OptionsPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
}

void OptionsPanel::resized()
{
    auto area = getLocalBounds().reduced(20);

    const int gap = 10;
    const int titleHeight = 30;
    const int width = (area.getWidth() - 3 * gap) / 4;

    // ===== Column 1 =====
    auto col1Area = area.removeFromLeft(width);
    title1.setBounds(col1Area.removeFromTop(titleHeight));
    column1.setBounds(col1Area);

    auto inner = col1Area.reduced(10);
    const int boxHeight = 70;
    const int gapY = 10;

    box1.setBounds(inner.removeFromTop(boxHeight));
    inner.removeFromTop(gapY);

    box2.setBounds(inner.removeFromTop(boxHeight));
    inner.removeFromTop(gapY);

    box3.setBounds(inner.removeFromTop(boxHeight));
    inner.removeFromTop(gapY);

    box4.setBounds(inner.removeFromTop(boxHeight));

    // ===== Other columns =====
    area.removeFromLeft(gap);

    auto col2Area = area.removeFromLeft(width);
    title2.setBounds(col2Area.removeFromTop(titleHeight));
    column2.setBounds(col2Area);

    area.removeFromLeft(gap);

    auto col3Area = area.removeFromLeft(width);
    title3.setBounds(col3Area.removeFromTop(titleHeight));
    column3.setBounds(col3Area);

    area.removeFromLeft(gap);

    auto col4Area = area.removeFromLeft(width);
    title4.setBounds(col4Area.removeFromTop(titleHeight));
    column4.setBounds(col4Area);
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

