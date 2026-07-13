#include "../OptionsPanel.h"
#include "../../../controller/AppController.h"
#include "../../leftPanel/LeftPanel.h"
#include "../../../model/ConstraintsDefinition.h"
/*
//==============================================================================
   VoiceBox

   Composant représentant une voix de contrepoint.

   Permet de sélectionner l'espèce et le type d'une voix
   puis synchronise ces choix avec l'AppController.
//==============================================================================*/
VoiceBox::VoiceBox(const juce::String& name)
{
    //  titre de la voix
    addAndMakeVisible(title);
    title.setText(name, juce::dontSendNotification);
    title.setJustificationType(juce::Justification::centred);
    title.setColour(juce::Label::textColourId, juce::Colours::white);
    title.setInterceptsMouseClicks(true, false);

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

    // Reçoit aussi les doubles-clics venant des éléments internes.
    addMouseListener(this, true);
}

/*
//==============================================================================
   Dessine l'état visuel de la voix.

   Met en évidence la voix actuellement sélectionnée.
//==============================================================================
*/
void VoiceBox::paint(juce::Graphics& g)
{
    // La sélection est temporaire : elle s'ajoute à l'état actif.
    if (isSelected)
        g.setColour(juce::Colour(0xff4b3b67));

    else if (isActive)
        g.setColour(juce::Colour(0xff2f4f4f));

    else
        g.setColour(juce::Colours::darkgrey.brighter());

    g.fillRoundedRectangle(getLocalBounds().toFloat(), 6.0f);

    if (isSelected)
    {
        g.setColour(juce::Colour(0xff8a79aa));
        g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1.0f),
                               6.0f,
                               2.0f);
    }
}

/*
//==============================================================================
   Positionne les éléments de l'interface.

   Organise le titre, l'espèce et le type dans la colonne.
//==============================================================================
*/
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
// Déclenche l'action associée au double-clic sur cette voix.
void VoiceBox::mouseDoubleClick(const juce::MouseEvent&)
{
    if (onClick != nullptr)
        onClick();
}

//==============================================================================
// Active ou désactive la mise en évidence de la voix.
//==============================================================================
void VoiceBox::setActive(bool active)
{
    if (isActive != active)
    {
        isActive = active;
        repaint();
    }
}

// Active ou désactive la sélection utilisateur de la voix.
void VoiceBox::setSelected(bool selected)
{
    if (isSelected != selected)
    {
        isSelected = selected;
        repaint();
    }
}

/*
//==============================================================================
   Connecte la VoiceBox à l'AppController.

   Synchronise les changements d'espèce et de type
   avec le modèle de l'application.
//==============================================================================
*/
void VoiceBox::connectToController(AppController* controller, int index)
{
    appController = controller;
    voiceIndex = index;

    // Met à jour l'espèce de la voix dans le modèle.
    speciesBox.onChange = [this]()
    {
        if (appController && (voiceIndex >= 0))
        {
            appController->updateVoice(
                voiceIndex,
                speciesBox.getSelectedId(),
                typeBox.getSelectedId() - 4
            );
        }
    };

    // Met à jour le type de la voix dans le modèle.
    typeBox.onChange = [this]()
    {
        if (appController && (voiceIndex >= 0))
        {
            appController->updateVoice(
                voiceIndex,
                speciesBox.getSelectedId(),
                typeBox.getSelectedId() - 4
            );
        }
    };
}
