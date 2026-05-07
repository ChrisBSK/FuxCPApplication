#include "GenerationService.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_events/juce_events.h>

#include "CounterpointUtils.hpp"
#include "CounterpointProblems/CounterpointProblem.hpp"
#include "Utilities.hpp"

#include "../controller/AppController.h"

#include <gecode/search.hh>
#include <memory>

//==============================================================================
// Impl interne
//==============================================================================

struct GenerationService::Impl
{
    bool initialized = false;
};


//==============================================================================
// Fonctions utilitaires internes au fichier
//==============================================================================

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

    std::vector<std::vector<int>> splitVoices(const std::vector<int>& solution,
                                              int totalVoices,
                                              int cfSize)
    // Exemple :
    // solution = {60, 62, 64,  67, 69, 71}
    // totalVoices = 2
    // cfSize = 3
    //
    // Représente :
    // [ CF        | CP1       ]
    // [60 62 64   | 67 69 71  ]
    //
    // Résultat :
    // {
    //   {60, 62, 64},   // CF
    //   {67, 69, 71}    // CP1
    // }
    //
    // → On découpe le vecteur en blocs de taille cfSize
    {
        std::vector<std::vector<int>> result;

        if (cfSize <= 0 || totalVoices <= 0)
            return result;

        if ((int) solution.size() != totalVoices * cfSize)
            return result;

        for (int voice = 0; voice < totalVoices; ++voice)
        {
            int start = voice * cfSize;
            int end   = start + cfSize;

            result.emplace_back(solution.begin() + start,
                                solution.begin() + end);
        }

        return result;
    }



    bool writeMidiFile(const std::vector<int>& cantusFirmus,
                       const std::vector<std::vector<int>>& counterpointVoices,
                       const juce::File& file)
    {
        juce::MidiFile midi;
        midi.setTicksPerQuarterNote(960);

        const int ticksPerNote = 960;

        // =========================
        // Track 1 : Cantus Firmus
        // =========================
        juce::MidiMessageSequence cfTrack;

        for (size_t i = 0; i < cantusFirmus.size(); ++i)
        {
            double start = (double) i * ticksPerNote;
            double end   = (double) (i + 1) * ticksPerNote;

            cfTrack.addEvent(juce::MidiMessage::noteOn(1, cantusFirmus[i], (juce::uint8) 100), start);
            cfTrack.addEvent(juce::MidiMessage::noteOff(1, cantusFirmus[i]), end);
        }

        midi.addTrack(cfTrack);

        // =========================
        // Tracks suivantes : contrepoints
        // =========================
        int channel = 2;

        for (const auto& voice : counterpointVoices)
        {
            juce::MidiMessageSequence track;

            for (size_t i = 0; i < voice.size(); ++i)
            {
                double start = (double) i * ticksPerNote;
                double end   = (double) (i + 1) * ticksPerNote;

                track.addEvent(juce::MidiMessage::noteOn(channel, voice[i], (juce::uint8) 100), start);
                track.addEvent(juce::MidiMessage::noteOff(channel, voice[i]), end);
            }

            midi.addTrack(track);

            if (++channel > 16)
                break;
        }

        if (auto stream = file.createOutputStream())
        {
            midi.writeTo(*stream);
            return true;
        }

        return false;
    }
}


//==============================================================================
// Construction / destruction
//==============================================================================

GenerationService::GenerationService()
    : juce::Thread("FuxCP Solver Thread"),
      pImpl(std::make_unique<Impl>())
{
    pImpl->initialized = true;
    ready = true;
    generationSuccess.store(false);
    lastError.clear();
}

GenerationService::~GenerationService()
{
    stopThread(-1);
}


//==============================================================================
// Lancement thread
//==============================================================================

bool GenerationService::startGeneration(const CantusProblem& problem,
                                        const juce::String& outputPath,
                                        AppController* controller)
{
    if (isThreadRunning())
    {
        lastError = juce::String::fromUTF8(("Une génération est déjà en cours."));
        return false;
    }

    if (!isReady())
    {
        lastError = juce::String::fromUTF8("Le service n'est pas prêt.");
        return false;
    }

    // Copie du problème pour éviter les accès concurrents avec l'UI.
    problemToGenerate = problem;
    outputPathToGenerate = outputPath;

    {
        juce::ScopedLock lock(callbackLock);
        appController = controller;
    }

    generationSuccess.store(false);
    lastGeneratedMidiPath.clear();
    lastError.clear();

    startThread();
    return true;
}

void GenerationService::run()
{
    bool success = generateMidiFromInputs(problemToGenerate, outputPathToGenerate);
    generationSuccess.store(success);

    AppController* controllerToNotify = nullptr;

    {
        juce::ScopedLock lock(callbackLock);
        controllerToNotify = appController;
    }

    if (controllerToNotify != nullptr)
        controllerToNotify->triggerAsyncUpdate();
}


//==============================================================================
// Pipeline principal de génération
//==============================================================================

bool GenerationService::generateMidiFromInputs(const CantusProblem& problem,
                                               const juce::String& outputPath)
{
    inputValidationError = false;

    // =========================
    // Vérifications de base
    // =========================
    if (!ready)
    {
        lastError = "Le service n'est pas prêt";
        return false;
    }

    if (problem.isEmpty())
    {
        inputValidationError = true;
        lastError = "Le problème est vide.\n\nEntrez un problème complet";
        return false;
    }

    // =========================
    // Récupération des données
    // =========================
    const auto& cf = problem.getCantusFirmus();
    const int cfSize = (int)cf.size();
    const int numVoices = (int)problem.getVoiceCount();
    const int numCounterpoints = numVoices - 1;

    // raw contient UNIQUEMENT les contrepoints
    int expectedSize = numCounterpoints * cfSize;

    // =========================
    // Création du problème Fux
    // =========================
    CounterpointProblem* fuxProblem = createFuxProblem(problem);

    if (fuxProblem == nullptr)
    {
        lastError = "Erreur création problème Fux";
        return false;
    }

    int nb_sol = 0;
    bool success = false;

    bool timeoutExceeded = false;

    // Démarrage du chronomètre (timeout de 3 secondes)
    const int TIMEOUT_SECONDS = 3;
    auto startTime = std::chrono::high_resolution_clock::now();

    try
    {
        // =========================
        // Solveur Branch & Bound
        // =========================
        BAB<CounterpointProblem> engine(fuxProblem);

        // =========================
        // Parcours de toutes les solutions
        // =========================
        while (CounterpointProblem* pb = engine.next())
        {
            auto currentTime = std::chrono::high_resolution_clock::now();

            auto elapsed =
                std::chrono::duration_cast<std::chrono::seconds>(
                    currentTime - startTime).count();

            if (elapsed > TIMEOUT_SECONDS)
            {
                lastError = "Timeout : aucune solution trouvée.";
                break;
            }

           nb_sol++;

            // =========================
            // Récupération solution brute
            // =========================

            int size = pb->getSize();
            int* raw = pb->return_solution();

            //delete pb;

            if (raw == nullptr)
            {
                std::cerr << "Erreur : return_solution() retourne nullptr\n";
                continue;
            }

            // =========================
            // Vérification taille
            // =========================
            if (size != expectedSize)
            {
                /*std::cout << "\n===== SOLUTION INVALIDE =====\n";
                std::cout << "size attendu : " << expectedSize << "\n";
                std::cout << "size reçu    : " << size << "\n";

                std::cout << "raw = ";

                for (int i = 0; i < size; ++i)
                    std::cout << raw[i] << " ";

                std::cout << "\n============================\n";*/

                continue;
            }

            // =========================
            // Conversion raw -> vector
            // =========================
            std::vector<int> solution(raw, raw + size);
            delete[] raw;

            // =========================
            // Découpage voix
            // =========================
            auto voices = splitVoices(solution,
                                      numCounterpoints,
                                      cfSize);

            if (voices.empty())
            {
                std::cout << "voices vide\n";
                continue;
            }



            // =========================
            // Affichage
            // =========================
            std::cout << "\n===== SOLUTION =====\n";

            std::cout << "CF : ";
            for (int note : cf)
                std::cout << note << " ";
            std::cout << "\n";

            for (size_t v = 0; v < voices.size(); ++v)
            {
                std::cout << "CP " << (v + 1) << " : ";

                for (int note : voices[v])
                    std::cout << note << " ";

                std::cout << "\n";
            }

            // =========================
            // MIDI
            // =========================
            juce::File midiFile(outputPath);

            if (writeMidiFile(cf, voices, midiFile))
            {
                lastGeneratedMidiPath = midiFile.getFullPathName();
                lastError.clear();

                success = true;

                std::cout << "MIDI généré\n";
            }
            else
            {
                lastError = "Erreur écriture MIDI";
            }

            break;
        }
    }
    catch (const std::exception& e)
    {
        lastError = juce::String("Erreur solveur : ") + e.what();
        return false;
    }



    if (success)
        return true;

    if (nb_sol == 0)
    {
        lastError =
            "Aucune solution n'existe pour ce problème.";
    }

    return false;
}



//==============================================================================
// Adaptation modèle --> FuxCP
//==============================================================================

CounterpointProblem* GenerationService::createFuxProblem(const CantusProblem& problem)
{
    // =========================
    //  Données musicales
    // =========================
    const auto& cf = problem.getCantusFirmus();
    const auto& counterpoints = problem.getCounterpoints();

    /*std::cout << "Nb contrepoints = " << counterpoints.size() << std::endl;

    for (int i = 0; i < counterpoints.size(); ++i)
    {
        std::cout << "CP " << i
                  << " species=" << counterpoints[i].species
                  << " type=" << counterpoints[i].type
                  << std::endl;
    }
    */


    if (cf.empty() || counterpoints.empty())
        return nullptr;

    // =========================
    //  Paramètres du solveur
    // =========================
    const auto& settings = problem.getSettings();

    // =========================
    // Conversion des contrepoints
    // =========================
    // IMPORTANT :
    // Fux reçoit le Cantus Firmus séparément via `cf`.
    // spListFux et vTypeFux contiennent donc uniquement les contrepoints.
    std::vector<Species> spListFux;
    std::vector<int> vTypeFux;

    spListFux.reserve(counterpoints.size());
    vTypeFux.reserve(counterpoints.size());


    for (const auto& cp : counterpoints)
    {
        spListFux.push_back(mapSpeciesIntToFux(cp.species));
        vTypeFux.push_back(cp.type);
    }

    // =========================
    //  Création du problème Fux
    // =========================
    return create_problem(
        cf,
        spListFux,
        vTypeFux,
        settings.soft.melodic,
        settings.soft.general,
        settings.soft.specific,
        settings.soft.importance,
        settings.borrowMode
    );
}


//==============================================================================
// État du service
//==============================================================================

bool GenerationService::isGenerating() const
{
    return isThreadRunning();
}

bool GenerationService::isReady() const
{
    return ready && pImpl && pImpl->initialized;
}

bool GenerationService::getLastGenerationSuccess() const
{
    return generationSuccess.load();
}

juce::String GenerationService::getLastGeneratedMidiPath() const
{
    return lastGeneratedMidiPath;
}

juce::String GenerationService::getLastError() const
{
    return lastError;
}

bool GenerationService::isInputValidationError() const
{
    return inputValidationError;
}

void GenerationService::reset()
{
    lastError.clear();
    lastGeneratedMidiPath.clear();
    inputValidationError = false;
    generationSuccess.store(false);

    if (pImpl)
        pImpl->initialized = true;

    ready = true;
}