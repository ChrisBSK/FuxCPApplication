#pragma once

#include <juce_core/juce_core.h>
#include <atomic>
#include <functional>
#include <memory>
#include <vector>

/**
 * Service de génération MIDI via FuxCP (thread worker).
 *
 * Objectif prototype :
 * - 2 issues seulement :
 *    - Solution trouvée -> écrit un MIDI (CF + voix de contrepoint)
 *    - Aucune solution -> message, mais aucun crash
 *
 * Pas de cache pour éviter complexité / bugs.
 * Pas d'include FuxCP ici (seulement dans .cpp).
 */
class GenerationService : public juce::Thread
{
public:
    GenerationService();
    ~GenerationService() override;

    bool startGeneration(const std::vector<int>& cantusFirmus,
                         int species,
                         int voiceCount,
                         const juce::String& outputPath);

    bool isGenerating() const;
    bool isReady() const;

    bool getLastGenerationSuccess() const;
    juce::String getLastError() const;
    bool isInputValidationError() const;

    void setOnFinishedCallback(std::function<void()> callback)
    {
        juce::ScopedLock lock(callbackLock);
        onFinishedCallback = std::move(callback);
    }

    juce::String getLastGeneratedMidiPath() const;
    void reset();

protected:
    void run() override;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;

    bool generateMidiFromInputs(const std::vector<int>& cantusFirmus,
                                int species,
                                int voiceCount,
                                const juce::File& midiOutFile);

    static bool validateCantusFirmus(const std::vector<int>& cantusFirmus, juce::String& err);
    static bool validateSpecies(int species, juce::String& err);
    static bool validateVoiceCount(int voiceCount, juce::String& err);

    // Etat
    bool ready = false;
    juce::String lastError;
    bool inputValidationError = false;
    std::atomic<bool> generationSuccess { false };
    juce::String lastGeneratedMidiPath;

    // Job
    std::vector<int> cantusFirmusToGenerate;
    int speciesToGenerate;
    int voiceCountToGenerate;
    juce::String outputPathToGenerate;
    juce::File midiOutFileToGenerate;

    // Callback UI
    juce::CriticalSection callbackLock;
    std::function<void()> onFinishedCallback;

    // Seed (optionnel)
    int currentSeed = 42;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GenerationService)
};