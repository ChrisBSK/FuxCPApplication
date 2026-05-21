#pragma once

#include <juce_core/juce_core.h>
#include <atomic>
#include <memory>

#include "../model/CantusProblem.h"
#include "CounterpointProblems/CounterpointProblem.hpp"

class AppController;

/*
==============================================================================
    GenerationService

    Rôle :
    - Exécute la génération de contrepoint dans un thread séparé
    - Traduit CantusProblem → CounterpointProblem (Fux)
    - Lance le solveur (Gecode)
    - Génère un fichier MIDI

    Flux :
    AppController → GenerationService → Solveur → MIDI → AppController
==============================================================================
*/

class GenerationService : public juce::Thread
{
public:
    GenerationService();
    ~GenerationService() override;

    // =========================
    // Lancement génération
    // =========================

    /**
     * Lance la génération dans un thread
     * - prend une copie du problème
     * - évite les conflits avec le thread UI
     */
    bool startGeneration(const CantusProblem& problem,
                         const juce::String& outputPath,
                         AppController* controller);

    // =========================
    // État du service
    // =========================

    bool isGenerating() const;
    bool isReady() const;

    bool getLastGenerationSuccess() const;
    juce::String getLastGeneratedMidiPath() const;

    juce::String getLastError() const;
    bool isInputValidationError() const;

    void reset();

protected:
    /**
     * Thread principal de génération
     */
    void run() override;

private:
    // =========================
    // Conversion modèle → Fux
    // =========================

    CounterpointProblem* createFuxProblem(const CantusProblem& problem);

    // =========================
    // Pipeline principal
    // =========================

    bool generateMidiFromInputs(const CantusProblem& problem,
                                const juce::String& outputPath);

    // =========================
    // État interne
    // =========================

    bool ready = false;

    std::atomic<bool> generationSuccess { false };

    juce::String lastGeneratedMidiPath;
    juce::String lastError;
    bool inputValidationError = false;

    // =========================
    // Communication avec controller
    // =========================

    AppController* appController = nullptr;
    juce::CriticalSection callbackLock;

    // =========================
    // Données threadées
    // =========================

    CantusProblem problemToGenerate;
    juce::String outputPathToGenerate;

    // =========================
    // Impl interne (si besoin)
    // =========================

    struct Impl;
    std::unique_ptr<Impl> pImpl;

    std::vector<int> melodicStorage;
    std::vector<int> generalStorage;
    std::vector<int> specificStorage;
    std::vector<int> importanceStorage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GenerationService)
};