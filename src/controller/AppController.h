#pragma once

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_data_structures/juce_data_structures.h>

#include "../model/CantusProblem.h"
#include "../service/GenerationService.h"

/*
//==============================================================================
   AppController

   Contrôleur principal de l'application.

   Assure la communication entre l'interface utilisateur,
   le modèle (CantusProblem) et le service de génération.
//==============================================================================
*/

// Forward declaration
class LeftPanel;


class AppController : public juce::AsyncUpdater
{
public:
    AppController();
    //explicit AppController(const juce::String& title);

    // =========================
    // Génération
    // =========================

    /**
     * Lance la génération d'un problème
     * - prend le modèle courant
     * - déclenche le GenerationService (thread)
     */
    void startGeneration(const juce::String& outputPath);


    // =========================
    // Accès modèle
    // =========================

    /**
     * Accès en écriture au problème
     * utilisé par le LeftPanel pour construire le problème
     */
    CantusProblem& getProblem();

    /**
     * Accès en lecture seule
     */
    const CantusProblem& getProblem() const;


    // =========================
    // Synchronisation UI
    // =========================

    /**
     * Structure intermédiaire utilisée UNIQUEMENT pour synchroniser
     * LeftPanel <-> OptionsPanel
     */
    struct VoiceSettings
    {
        int species = 1;
        int type    = 0;
    };

    std::vector<VoiceSettings>& getVoiceSettings();
    const std::vector<VoiceSettings>& getVoiceSettings() const;


    // =========================
    // Connexions UI
    // =========================

    void setLeftPanel(LeftPanel* panel);
    void setGenerationService(GenerationService* service);

    void updateVoice(int index, int species, int type);
    bool isGenerating() const;

    void updateSettings(const ConstraintSettings &newSettings);

    juce::ValueTree& getGenerationState()
    {
        return generationState;
    }

private:
    // =========================
    // Modèle principal
    // =========================
    CantusProblem problem;

    // =========================
    // Synchronisation UI
    // =========================
    std::vector<VoiceSettings> voiceSettings;

    // =========================
    // Services externes
    // =========================
    GenerationService* generationService = nullptr;

    // =========================
    // UI callbacks
    // =========================
    LeftPanel* leftPanel = nullptr;

    /**
     * Callback appelé après la génération (thread → UI)
     */
    void handleAsyncUpdate() override;

    ConstraintSettings currentSettings;

    juce::ValueTree generationState { "GenerationState" };

};