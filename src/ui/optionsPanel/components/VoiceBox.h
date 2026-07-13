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
    void setSelected(bool selected);
    bool isActive = false;
    bool isSelected = false;

    // Contrôles de configuration de la voix.
    juce::ComboBox speciesBox;
    juce::ComboBox typeBox;

    // Liaison avec le modèle.
    AppController* appController = nullptr;
    int voiceIndex = -1;

    // Connecte la voix à l'AppController.
    void connectToController(AppController* controller, int index);

    // Fonction appelée quand l'utilisateur double-clique sur cette voix.
    std::function<void()> onClick;

    // Détecte le double-clic sur la VoiceBox et ses enfants.
    void mouseDoubleClick(const juce::MouseEvent&) override;

private:
    // Titre affiché en haut de la box.
    juce::Label title;
};
