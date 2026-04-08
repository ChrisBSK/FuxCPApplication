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



private:
    CantusProblem problem;
    GenerationService generationService;

    /** Callback après génération (thread → message thread) */
    void handleAsyncUpdate() override;
};