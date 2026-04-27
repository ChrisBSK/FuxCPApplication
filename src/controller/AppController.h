#pragma once

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

#include "../model/CantusProblem.h"
#include "../service/GenerationService.h"

// Forward declaration
class LeftPanel;

/*
==============================================================================
    AppController

    Rôle :
    - Point central entre UI (LeftPanel / OptionsPanel) et logique métier
    - Possède le modèle CantusProblem
    - Lance la génération via GenerationService
    - Reçoit le résultat (callback async)


    Flux :
    UI → AppController → GenerationService → AppController → UI
==============================================================================
*/

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
};