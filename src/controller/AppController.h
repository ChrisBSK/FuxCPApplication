#pragma once

#include <juce_core/juce_core.h>
#include "../model/CantusProblem.h"
#include "../service/GenerationService.h"
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * @brief Contrôleur principal MVC (version Fux).
 *
 * Rôle :
 * - Possède le modèle CantusProblem
 * - Lance la génération
 * - Gère les callbacks du GenerationService
 *
 * Ne duplique PAS les getters/setters du modèle.
 */
class LeftPanel;
class AppController : public juce::AsyncUpdater, juce::Component
{
public:
    AppController();

    explicit AppController(const juce::String& title);

    // =========================
    // Génération
    // =========================


    void startGeneration(const juce::String& outputPath);

    // =========================
    // Accès modèle
    // =========================

    CantusProblem& getProblem();
    const CantusProblem& getProblem() const;

    std::vector<int> species;
    std::vector<int> types;

    struct VoiceSettings
    {
        int species = 1;
        int type = 0;
    };

    std::vector<VoiceSettings>& getVoiceSettings()
    {
        return voiceSettings;
    }

    void setLeftPanel(LeftPanel* panel)
    {
        leftPanel = panel;
    }

    void setGenerationService(GenerationService* service)
    {
        generationService = service;
    }

    juce::ValueTree& getGenerationState();
    const juce::ValueTree& getGenerationState() const;

private:
    CantusProblem problem;
    GenerationService* generationService = nullptr;

    std::vector<VoiceSettings> voiceSettings;

    LeftPanel* leftPanel = nullptr;

    juce::ValueTree generationState { "GENERATION_STATE" };

    /** Callback après génération (thread → message thread) */
    void handleAsyncUpdate() override;
};