#include "../OptionsPanel.h"
#include "../../../controller/AppController.h"
#include "../../leftPanel/LeftPanel.h"
#include "../../../model/ConstraintsDefinition.h"
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
        /*if (auto* parent = findParentComponentOfClass<OptionsPanel>())
        {
            if (auto* lp = parent->getLeftPanel())
                lp->refreshFromModel();
        }*/
    };

    //Connexion type entre leftpanel et OptionPanel
    typeBox.onChange = [this]()
    {
        // update leftpanel -> optionpanel
        if (settings)
            settings->type = typeBox.getSelectedId() - 4;
        //update optionpanel -> leftpanel
        /*
        if (auto* parent = findParentComponentOfClass<OptionsPanel>())
        {
            if (auto* lp = parent->getLeftPanel())
                lp->refreshFromModel();
        }*/
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
