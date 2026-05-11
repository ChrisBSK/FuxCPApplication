#include "GenerationService.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_events/juce_events.h>

#include "CounterpointUtils.hpp"
#include "CounterpointProblems/CounterpointProblem.hpp"
#include "Utilities.hpp"

#include "../controller/AppController.h"

#include <gecode/search.hh>
#include <sstream>
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

    std::vector<int> extractSolution(const std::string& str)
    // Exemple :
    // str = "Solution array: {60, 62, 64, 67, 69, 71}"
    //
    //
    // - On récupère ce qu’il y a entre { }
    //    → "60, 62, 64, 67, 69, 71"
    //
    // - On découpe par virgules
    //    → "60" "62" "64" "67" "69" "71"
    //
    // - On convertit en int
    //
    // Résultat :
    // {60, 62, 64, 67, 69, 71}
    //
    //  Transforme la string du solver en vecteur de notes
    {
        std::vector<int> result;

        auto pos = str.find("Solution array");

        if (pos == std::string::npos)
            pos = str.find("Solution Array");

        if (pos == std::string::npos)
            return result;

        auto start = str.find("{", pos);
        auto end   = str.find("}", start);

        if (start == std::string::npos || end == std::string::npos)
            return result;

        std::stringstream ss(str.substr(start + 1, end - start - 1));
        std::string token;

        while (std::getline(ss, token, ','))
        {
            try
            {
                result.push_back(std::stoi(token));
            }
            catch (...)
            {
                return {};
            }
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
    juce::ignoreUnused(outputPath);

    inputValidationError = false;
    lastGeneratedMidiPath.clear();
    lastError.clear();

    if (!ready)
    {
        lastError = juce::String::fromUTF8("Le service n'est pas prêt.");
        return false;
    }

    if (problem.isEmpty())
    {
        inputValidationError = true;
        lastError = juce::String::fromUTF8("Le problème est vide.");
        return false;
    }

    try
    {
        CounterpointProblem* fuxProblem = createFuxProblem(problem);

        if (fuxProblem == nullptr)
        {
            lastError = juce::String::fromUTF8("Erreur création problème Fux.");
            return false;
        }

        std::cout << "\n[GEN] Problème Fux créé." << std::endl;

        // on utilise les mêmes méthode que dans main.cpp de FUXCP
        Search::Base<CounterpointProblem>* engine =
            make_solver(fuxProblem, bab_solver);

        if (engine == nullptr)
        {
            lastError = juce::String::fromUTF8("Erreur création solveur FuxCP.");
            return false;
        }

        std::cout << "[GEN] Solveur créé." << std::endl;

        CounterpointProblem* solution =
            get_next_solution_space(engine);

        std::cout << "[GEN] Recherche terminée." << std::endl;

        if (solution == nullptr)
        {
            lastError = juce::String::fromUTF8("Aucune solution trouvée.");
            std::cout << "[GEN] Aucune solution trouvée." << std::endl;
            return false;
        }

        std::cout << "[GEN] Une solution existe." << std::endl;


        lastError.clear();
        return true;
    }
    catch (const std::exception& e)
    {
        lastError =
            juce::String::fromUTF8("Erreur solveur : ")
            + juce::String(e.what());

        return false;
    }
}



//==============================================================================
// Adaptation modèle --> FuxCP
//==============================================================================

CounterpointProblem* GenerationService::createFuxProblem(const CantusProblem& problem)
{
    const auto& cf = problem.getCantusFirmus();
    const auto& counterpoints = problem.getCounterpoints();

    if (cf.empty())
    {
        std::cout << "Erreur : Cantus Firmus vide." << std::endl;
        return nullptr;
    }

    if (cf.size() < 8)
    {
        std::cout << "Erreur : Cantus Firmus trop court pour FuxCP. Taille = "
                  << cf.size()
                  << std::endl;

        return nullptr;
    }

    if (counterpoints.empty())
    {
        std::cout << "Erreur : aucun contrepoint." << std::endl;
        return nullptr;
    }

    const auto& settings = problem.getSettings();

    if (settings.soft.melodic.size() != 8 ||
        settings.soft.general.size() != 8 ||
        settings.soft.specific.size() != 7 ||
        settings.soft.importance.size() != 14)
    {
        std::cout << "Erreur : tailles des paramètres FuxCP invalides." << std::endl;
        std::cout << "melodic = " << settings.soft.melodic.size() << std::endl;
        std::cout << "general = " << settings.soft.general.size() << std::endl;
        std::cout << "specific = " << settings.soft.specific.size() << std::endl;
        std::cout << "importance = " << settings.soft.importance.size() << std::endl;
        return nullptr;
    }

    std::vector<Species> spListFux;
    std::vector<int> vTypeFux;

    spListFux.reserve(counterpoints.size());
    vTypeFux.reserve(counterpoints.size());

    for (const auto& cp : counterpoints)
    {
        if (cp.species < 1 || cp.species > 5)
        {
            std::cout << "Erreur : species invalide = " << cp.species << std::endl;
            return nullptr;
        }

        if (cp.type < -3 || cp.type > 2)
        {
            std::cout << "Erreur : type invalide = " << cp.type << std::endl;
            return nullptr;
        }

        spListFux.push_back(mapSpeciesIntToFux(cp.species));
        vTypeFux.push_back(cp.type);
    }

    std::cout << "\n=== BEFORE create_problem ===" << std::endl;

    std::cout << "voiceCount = "
              << problem.getVoiceCount()
              << std::endl;

    std::cout << "cf size = "
              << cf.size()
              << std::endl;

    std::cout << "cf = ";
    for (int note : cf)
        std::cout << note << " ";
    std::cout << std::endl;

    std::cout << "spListFux size = "
              << spListFux.size()
              << std::endl;

    std::cout << "vTypeFux size = "
              << vTypeFux.size()
              << std::endl;

    for (size_t i = 0; i < spListFux.size(); ++i)
    {
        std::cout << "FUX CP " << i
                  << " species=" << static_cast<int>(spListFux[i])
                  << " type=" << vTypeFux[i]
                  << std::endl;
    }

    std::cout << "melodic size = " << settings.soft.melodic.size() << std::endl;
    std::cout << "general size = " << settings.soft.general.size() << std::endl;
    std::cout << "specific size = " << settings.soft.specific.size() << std::endl;
    std::cout << "importance size = " << settings.soft.importance.size() << std::endl;
    std::cout << "borrowMode = " << settings.borrowMode << std::endl;

    std::cout << "CALL create_problem NOW" << std::endl;

    try
    {
        CounterpointProblem* fuxProblem = create_problem(
        cf,
        spListFux,
        vTypeFux,
        settings.soft.melodic,
        settings.soft.general,
        settings.soft.specific,
        settings.soft.importance,
        settings.borrowMode
    );

        return fuxProblem;
    }
    catch (const std::exception& e)
    {
        std::cout << "Erreur create_problem : "
                  << e.what()
                  << std::endl;

        return nullptr;
    }
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