#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../../controller/AppController.h"

/**
 * @brief Panneau d’options avancées pour la configuration des voix (Vue).
 *
 * Rôle :
 * - Affiche et permet de modifier les paramètres de chaque voix
 *   (espèce et type)
 * - Synchronise les contrôles UI avec le modèle partagé (VoiceSettings)
 * - Visualise les voix actives selon le nombre de voix sélectionné
 *
 * Responsabilités :
 * - Gérer l’affichage et l’organisation des VoiceBox
 * - Connecter les ComboBox au modèle (binding UI ↔ données)
 *
 * Ne contient PAS :
 * - de logique métier (règles musicales, génération)
 * - de traitement audio
 */

// =============================
// VoiceBox
// =============================
class VoiceBox : public juce::Component
{
public:
    VoiceBox(const juce::String& name);

    void paint(juce::Graphics& g) override;
    void resized() override;
    bool isActive = false;
    juce::ComboBox speciesBox;
    juce::ComboBox typeBox;

    AppController::VoiceSettings* settings = nullptr;


private:
    juce::Label title;


};

// =============================
// OptionsPanel
// =============================
class OptionsPanel : public juce::Component
{
public:
    OptionsPanel();

    void paint(juce::Graphics& g) override;
    void resized() override;
    void setNumVoices(int numVoices);

    void setVoiceSettings(std::vector<AppController::VoiceSettings> &settings);

    void updateVoice(int index, int species, int type);


private:
    juce::Component column1, column2, column3, column4;

    juce::Label title1, title2, title3, title4;

    VoiceBox box1 { "Voix 1" };
    VoiceBox box2 { "Voix 2" };
    VoiceBox box3 { "Voix 3" };
    VoiceBox box4 { "Voix 4" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OptionsPanel)
};