//
// Créé par Chris BAKASHIKA (2026)
//

/*
//==============================================================================
   GenerationService.h

   Exécute la génération de contrepoint dans un thread séparé : traduit le
   CantusProblem en CounterpointProblem, lance le solveur, puis génère le
   fichier MIDI de la solution.

   REMARQUE: Ce fichier a été inspiré de l'architecture de la couche Service
   réalisée par Cédric Niyikiza dans son plug-in "DiatonyDawApplication" disponible
   à l'adresse suivant:

            https://github.com/cedricniyi/DiatonyDawApplication.git

//==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>
#include <atomic>
#include <memory>

#include "../model/CantusProblem.h"
#include "CounterpointProblems/CounterpointProblem.hpp"

class AppController;

class GenerationService : public juce::Thread
{
public:
    GenerationService();
    ~GenerationService() override;

    /*
        Durée maximale (en secondes) laissée au solveur pour rechercher une
        solution. Sert à la fois à configurer Gecode::Search::TimeStop dans
        run(), et de référence côté interface pour afficher un compte à
        rebours pendant la génération (voir LeftPanel) : une seule valeur,
        jamais deux nombres à tenir synchronisés à la main.
    */
    static constexpr int searchTimeoutSeconds = 20;

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

    // Demande une autre solution pour le dernier problème généré.
    bool startNextSolution(const juce::String& outputPath,
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
    CantusProblem lastProblemWithSolution;
    juce::String outputPathToGenerate;
    int requestedSolutionIndex = 0;
    bool hasPreviousSolution = false;

    // =========================
    // Impl interne
    // =========================

    struct Impl;
    std::unique_ptr<Impl> pImpl;

    std::vector<int> melodicStorage;
    std::vector<int> generalStorage;
    std::vector<int> specificStorage;
    std::vector<int> importanceStorage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GenerationService)
};
