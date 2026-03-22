#include "GenerationService.h"

// JUCE (écriture MIDI)
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_events/juce_events.h>

// FuxCP / Gecode (uniquement ici)
#include "CounterpointUtils.hpp"
#include "CounterpointProblems/CounterpointProblem.hpp"
#include "Utilities.hpp"
#include <gecode/search.hh>

#include <cstdlib>
#include <memory>
#include <stdexcept>

struct GenerationService::Impl
{
    bool initialized = false;
};

namespace
{
    Species mapSpeciesIntToFux(int species)
    {
        switch (species)
        {
            case 1: return FIRST_SPECIES;
            case 2: return SECOND_SPECIES;
            case 3: return THIRD_SPECIES;
            case 4: return FOURTH_SPECIES;
            case 5: return FIFTH_SPECIES;
            default: return FIRST_SPECIES;
        }
    }

    static bool validateMidiNotes(const std::vector<int>& notes, juce::String& err)
    {
        for (int n : notes)
        {
            if (n < 0 || n > 127)
            {
                err = "Solution invalide : note MIDI hors plage [0..127]";
                return false;
            }
        }
        return true;
    }

    bool writeMidiFile(const std::vector<int>& cantusFirmus,
                       const std::vector<std::vector<int>>& counterpointVoices,
                       int species,
                       const juce::File& midiOutFile, juce::String& err)
    {
        // Validation CF (normalement déjà validé en amont)
        if (!validateMidiNotes(cantusFirmus, err))
            return false;

        // Validation voix générées (CRITIQUE pour éviter crash JUCE)
        for (const auto& v : counterpointVoices)
        {
            if (!validateMidiNotes(v, err))
                return false;
        }

        juce::MidiFile midiFile;
        midiFile.setTicksPerQuarterNote(960);

        // Durées : uniquement pour le rendu MIDI (pas pour découper la solution).
        double noteDuration = 4.0;
        if (species == 2) noteDuration = 2.0;
        else if (species == 3) noteDuration = 1.0;
        else if (species == 4) noteDuration = 2.0;
        else if (species == 5) noteDuration = 1.0;

        // Track 1: Cantus Firmus (channel 1)
        juce::MidiMessageSequence cfTrack;
        for (size_t i = 0; i < cantusFirmus.size(); ++i)
        {
            const double start = i * 960.0;
            const double end = (i + 1) * 960.0;
            cfTrack.addEvent(juce::MidiMessage::noteOn(1, cantusFirmus[i], 0.8f), start);
            cfTrack.addEvent(juce::MidiMessage::noteOff(1, cantusFirmus[i]), end);
        }
        midiFile.addTrack(cfTrack);

        // Tracks suivantes: contrepoint (channels 2..)
        int midiChannel = 2;
        for (const auto& voice : counterpointVoices)
        {
            juce::MidiMessageSequence track;
            double currentTime = 0.0;

            for (int note : voice)
            {
                track.addEvent(juce::MidiMessage::noteOn(midiChannel, note, 0.8f), currentTime);
                track.addEvent(juce::MidiMessage::noteOff(midiChannel, note),
                               currentTime + noteDuration * 240.0);
                currentTime += noteDuration * 240.0;
            }

            midiFile.addTrack(track);
            ++midiChannel;
        }

        if (auto outStream = std::unique_ptr<juce::FileOutputStream>(midiOutFile.createOutputStream()))
        {
            midiFile.writeTo(*outStream);
            return true;
        }

        return false;
    }
}


namespace DefaultSettings
{
    const std::vector<int> melodic   = {0, 1, 1, 576, 2, 2, 2, 1};
    const std::vector<int> general   = {4, 1, 1, 2, 2, 2, 8, 1};
    const std::vector<int> specific  = {8, 4, 0, 2, 1, 8, 50};
    const std::vector<int> importance = {8, 7, 5, 2, 9, 3, 14, 12, 6, 11, 4, 10, 1, 13};
}

GenerationService::GenerationService()
    : juce::Thread("FuxCP Solver Thread")
{
    pImpl = std::make_unique<Impl>();
    pImpl->initialized = true;

    ready = true;
    lastError.clear();
    lastGeneratedMidiPath.clear();
    inputValidationError = false;
    generationSuccess.store(false);
}

GenerationService::~GenerationService()
{
    {
        juce::ScopedLock lock(callbackLock);
        onFinishedCallback = nullptr;
    }
    stopThread(-1);
}

bool GenerationService::startGeneration(const std::vector<int>& cantusFirmus,
                                        int species,
                                        int voiceCount,
                                        const juce::String& outputPath)
{
    if (isThreadRunning())
    {
        lastError = "Une generation est deja en cours";
        return false;
    }

    if (!isReady())
    {
        lastError = "Service not ready";
        return false;
    }

    // Reset état (prototype interactif)
    inputValidationError = false;
    generationSuccess.store(false);
    lastError.clear();
    lastGeneratedMidiPath.clear();

    juce::String err;
    if (!validateSpecies(species, err)
        || !validateVoiceCount(voiceCount, err)
        || !validateCantusFirmus(cantusFirmus, err))
    {
        inputValidationError = true;
        lastError = err;
        return false;
    }


    cantusFirmusToGenerate = cantusFirmus;
    speciesToGenerate = species;
    voiceCountToGenerate = voiceCount;
    outputPathToGenerate = outputPath;
    midiOutFileToGenerate = juce::File(outputPathToGenerate);

    startThread();
    return true;
}

void GenerationService::run()
{
    bool success = false;

    try
    {
        success = generateMidiFromInputs(cantusFirmusToGenerate,
                                         speciesToGenerate,
                                         voiceCountToGenerate,
                                         midiOutFileToGenerate);
    }
    catch (const std::exception& e)
    {
        lastError = juce::String("Erreur pendant la generation: ") + e.what();
        success = false;
    }
    catch (...)
    {
        lastError = "Erreur inconnue pendant la generation";
        success = false;
    }

    generationSuccess.store(success);

    // Appel callback UI sans capturer `this`
    std::function<void()> cb;
    {
        juce::ScopedLock lock(callbackLock);
        cb = onFinishedCallback;
    }
    if (cb)
        juce::MessageManager::callAsync([cb]() mutable { cb(); });
}



bool GenerationService::generateMidiFromInputs(const std::vector<int>& cantusFirmus,
                                               int species,
                                               int voiceCount,
                                               const juce::File& midiOutFile)
{


    try
    {
        std::srand(currentSeed);

        const int counterpointVoices = voiceCount - 1;

        if (counterpointVoices <= 0)
        {
            lastError = "Nombre de voix invalide";
            return false;
        }

        std::vector<Species> spList = { mapSpeciesIntToFux(species) };
        std::vector<int> v_type(counterpointVoices, 2);

        auto melodic     = DefaultSettings::melodic;
        auto general     = DefaultSettings::general;
        auto specific    = DefaultSettings::specific;
        auto importance  = DefaultSettings::importance;

        int borrowMode = 1;

        std::unique_ptr<CounterpointProblem> problem(
            create_problem(cantusFirmus, spList, v_type,
                           melodic, general, specific, importance, borrowMode)
        );

        if (!problem)
        {
            lastError = "Erreur creation du probleme";
            return false;
        }

        // Timeout du solveur (important)
        Gecode::Search::TimeStop ts(5000); // 5 sec
        Gecode::Search::Options options;
        options.stop = &ts;

        Gecode::BAB<CounterpointProblem> engine(problem.get(), options);

        std::unique_ptr<CounterpointProblem> solution;

        try
        {
            solution.reset(engine.next());
        }
        catch (...)
        {
            lastError = "Erreur solveur";
            return false;
        }

        if (!solution)
        {
            lastError = "Aucune solution n'existe";
            return false;
        }

        int solSize = solution->getSize();

        if (solSize % counterpointVoices != 0 || solSize <= 0 || solSize < counterpointVoices)
        {
            lastError = "Aucune solution n'existe";
            return false;
        }


        size_t notesPerVoice = solSize / counterpointVoices;

        int* raw = nullptr;

        try
        {
            raw = solution->return_solution();
        }
        catch (...)
        {
            lastError = "Erreur lecture solution";
            return false;
        }


        std::vector<std::vector<int>> voices(counterpointVoices);

        for (int v = 0; v < counterpointVoices; ++v)
        {
            size_t base = v * notesPerVoice;

            if (base >= (size_t)solSize)
                continue;

            for (size_t i = 0; i < notesPerVoice; ++i)
            {
                size_t index = base + i;

                if (index >= (size_t)solSize)
                    break;

                voices[v].push_back(raw[index]);
            }
        }

        juce::String midiErr;
        const bool ok = writeMidiFile(cantusFirmus, voices, species, midiOutFile, midiErr);

        if (!ok)
        {
            lastError = "Erreur ecriture MIDI";
            return false;
        }

        lastGeneratedMidiPath = midiOutFile.getFullPathName();

        lastError.clear();
        return true;
    }
    catch (...)
    {
        lastError = "Erreur inconnue";
        return false;
    }
}

bool GenerationService::validateCantusFirmus(const std::vector<int>& cantusFirmus, juce::String& err)
{
    if (cantusFirmus.empty())
    {
        err = "Cantus Firmus vide";
        return false;
    }

    for (int n : cantusFirmus)
    {
        if (n < 0 || n > 127)
        {
            err = "Cantus Firmus invalide : notes MIDI doivent etre dans [0..127]";
            return false;
        }
    }

    err.clear();
    return true;
}

bool GenerationService::validateSpecies(int species, juce::String& err)
{
    if (species < 1 || species > 5)
    {
        err = "Species invalide : doit etre un entier entre 1 et 5";
        return false;
    }
    err.clear();
    return true;
}

bool GenerationService::validateVoiceCount(int voiceCount, juce::String& err)
{
    if (voiceCount < 2 || voiceCount > 4)
    {
        err = "Nombre de voix invalide : doit etre 2, 3 ou 4";
        return false;
    }
    err.clear();
    return true;
}



bool GenerationService::isGenerating() const { return isThreadRunning(); }

bool GenerationService::isReady() const
{
    return ready && pImpl && pImpl->initialized;
}

bool GenerationService::getLastGenerationSuccess() const { return generationSuccess.load(); }

juce::String GenerationService::getLastError() const { return lastError; }

bool GenerationService::isInputValidationError() const { return inputValidationError; }

juce::String GenerationService::getLastGeneratedMidiPath() const { return lastGeneratedMidiPath; }

void GenerationService::reset()
{
    lastError.clear();
    inputValidationError = false;
    generationSuccess.store(false);
    lastGeneratedMidiPath.clear();
    ready = true;
}