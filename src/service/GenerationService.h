#pragma once

#include <juce_core/juce_core.h>
#include <atomic>
#include <functional>
#include <memory>
#include <vector>

#include "../model/CantusProblem.h"
#include "CounterpointProblems/CounterpointProblem.hpp"

class AppController;

/**
 * @brief Service de génération MIDI via le solveur Diatony (thread worker).
 *
 * Pattern Adapter : traduit notre modèle Piece vers la librairie Diatony.
 * Seul le .cpp inclut les headers Diatony (couplage faible).
 */
class GenerationService : public juce::Thread
{
public:
    GenerationService();
    ~GenerationService() override;

    bool startGeneration(const CantusProblem& cantusFirmus,
                         const juce::String& outputPath, AppController* controller);


    bool isGenerating() const;
    bool isReady() const;
    juce::String getLastError() const;
    bool isInputValidationError() const;  // true si l'erreur est une validation d'input (warning)
    void reset();

    /** @brief Log console de la pièce (debug). */
    void logGenerationInfo(const CantusProblem& CantusProblem);

    bool getLastGenerationSuccess() const;
    juce::String getLastGeneratedMidiPath() const;

    std::function<void()> onFinished;






protected:
    void run() override;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;

    CounterpointProblem* createFuxProblem(const CantusProblem& problem);

    struct costVectors {
        std::vector<int> melodic;
        std::vector<int> general;
        std::vector<int> specific;
        std::vector<int> importance;
    };

    costVectors getDefaultCosts();


    //const std::vector<int> melodic   = {0, 1, 1, 576, 2, 2, 2, 1};
    //const std::vector<int> general   = {4, 1, 1, 2, 2, 2, 8, 1};
    //const std::vector<int> specific  = {8, 4, 0, 2, 1, 8, 50};
    //const std::vector<int> importance = {8, 7, 5, 2, 9, 3, 14, 12, 6, 11, 4, 10, 1, 13};

    bool generateMidiFromInputs(const CantusProblem& cantusProblem, const juce::String& outputPath);

    mutable juce::String lastError;
    mutable bool inputValidationError = false;  // Distingue warning (validation) vs error (solveur)
    bool ready;


    AppController* appController = nullptr;
    const CantusProblem* cantusProblemToGenerate = nullptr;
    juce::String outputPathToGenerate;

    std::atomic<bool> generationSuccess { false };
    juce::String lastGeneratedMidiPath;
    juce::CriticalSection callbackLock;

    //static bool validateCantusFirmus(const std::vector<int>& cantusFirmus, juce::String& err);
    //static bool validateSpecies(int species, juce::String& err);
    //static bool validateVoiceCount(int voiceCount, juce::String& err);


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GenerationService)
};