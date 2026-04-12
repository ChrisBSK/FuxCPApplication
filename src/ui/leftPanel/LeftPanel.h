#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

#include "../../service/GenerationService.h"
#include "../optionsWindow/OptionsPanel.h"
/**
 * @brief Panneau principal d’entrée utilisateur (Vue MVC).
 *
 * Rôle :
 * - Permet de saisir le Cantus Firmus (texte MIDI)
 * - Permet de configurer le nombre de voix, espèces et types
 * - Lance la génération via le AppController
 * - Ouvre le panneau d’options avancées (OptionsPanel)
 *
 * Responsabilités :
 * - Gérer l’interface utilisateur (UI)
 * - Synchroniser les choix utilisateur avec le modèle (via AppController)
 *
 * Ne contient PAS :
 * - de logique métier (calcul, génération)
 * - de traitement audio
 */

class AppController;

class LeftPanel : public juce::Component
{
public:
    LeftPanel(AppController& controller);
    ~LeftPanel() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    void showAlert(juce::AlertWindow::AlertIconType icon,
                   const juce::String& title,
                   const juce::String& message);

    juce::String getCantusText() const;

    void setCantusText(const juce::String& newText);

    void setOptionsPanel(OptionsPanel* panel)
    {
        optionsPanel = panel;
    }

private:
    juce::TextEditor text;
    juce::Label label;

    juce::ComboBox voices;
    juce::Label labelVoices;

    juce::TextButton moreOptions;
    juce::TextButton generateButton;

    GenerationService generationService;
    juce::File midiOutFileToGenerate;

    juce::OwnedArray<juce::ComboBox> speciesBoxes;
    juce::OwnedArray<juce::Label> speciesLabels;

    juce::OwnedArray<juce::ComboBox> typeBoxes;

    void updateVoiceSpeciesUI(int numVoices);
    void prepareOutputFile();
    juce::Label speciesHeader;
    juce::Label typeHeader;

    AppController& appController;
    OptionsPanel* optionsPanel = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LeftPanel)
};