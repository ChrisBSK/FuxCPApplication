#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../../../controller/AppController.h"
/**
 * VoiceBox
 *
 * Représente une voix de contrepoint dans l'interface.
 * Permet de sélectionner son espèce et son type.
 */
class VoiceBox : public juce::Component
{
public:
    // Construction de la VoiceBox.
    VoiceBox(const juce::String& name);

    // Rendu et layout.
    void paint(juce::Graphics&) override;
    void resized() override;

    // Gestion de l'état visuel.
    void setActive(bool active);
    bool isActive = false;

    // Contrôles de configuration de la voix.
    juce::ComboBox speciesBox;
    juce::ComboBox typeBox;

    // Liaison avec le modèle.
    AppController* appController = nullptr;
    int voiceIndex = -1;

    // Connecte la voix à l'AppController.
    void connectToController(AppController* controller, int index);

private:
    // Titre affiché en haut de la box.
    juce::Label title;
};