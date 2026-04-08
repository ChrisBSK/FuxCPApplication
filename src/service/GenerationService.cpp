#include "GenerationService.h"

// JUCE (écriture MIDI)
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_events/juce_events.h>

// FuxCP / Gecode (uniquement ici)
#include "CounterpointUtils.hpp"
#include "CounterpointProblems/CounterpointProblem.hpp"
#include "Utilities.hpp"
#include <gecode/search.hh>

#include "../controller/AppController.h"


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

    std::vector<std::vector<int>> splitVoices(
    const std::vector<int>& solution,
    int numVoices,
    int cfSize)
    {
        std::vector<std::vector<int>> voices;

        for (int v = 0; v < numVoices; ++v)
        {
            int start = v * cfSize;
            int end   = start + cfSize;

            voices.emplace_back(
                solution.begin() + start,
                solution.begin() + end
            );
        }

        return voices;
    }

    std::vector<int> extractSolution(const std::string& str)
    {
        std::vector<int> result;

        auto pos = str.find("Solution array");
        if (pos == std::string::npos)
            return result;

        auto start = str.find("{", pos);
        auto end   = str.find("}", start);

        if (start == std::string::npos || end == std::string::npos)
            return result;

        std::string numbers = str.substr(start + 1, end - start - 1);

        std::stringstream ss(numbers);
        std::string token;

        while (std::getline(ss, token, ','))
        {
            result.push_back(std::stoi(token));
        }

        return result;
    }


    bool writeMidiFile(const std::vector<int>& cantusFirmus,
                   const std::vector<std::vector<int>>& voices,
                   const juce::File& file)
    {
        juce::MidiFile midi;
        midi.setTicksPerQuarterNote(960);

        const int ticksPerNote = 960; // noire = 1 temps

        // =========================
        // 🎵 Track 1 : Cantus Firmus
        // =========================
        juce::MidiMessageSequence cfTrack;

        for (size_t i = 0; i < cantusFirmus.size(); ++i)
        {
            double start = i * ticksPerNote;
            double end   = (i + 1) * ticksPerNote;

            cfTrack.addEvent(juce::MidiMessage::noteOn(1, cantusFirmus[i], (juce::uint8)100), start);
            cfTrack.addEvent(juce::MidiMessage::noteOff(1, cantusFirmus[i]), end);
        }

        midi.addTrack(cfTrack);

        // =========================
        // 🎵 Tracks suivantes : voix générées
        // =========================
        int channel = 2;

        for (const auto& voice : voices)
        {
            juce::MidiMessageSequence track;

            for (size_t i = 0; i < voice.size(); ++i)
            {
                double start = i * ticksPerNote;
                double end   = (i + 1) * ticksPerNote;

                int note = voice[i];

                track.addEvent(juce::MidiMessage::noteOn(channel, note, (juce::uint8)100), start);
                track.addEvent(juce::MidiMessage::noteOff(channel, note), end);
            }

            midi.addTrack(track);
            channel++;

            // sécurité : max 16 canaux MIDI
            if (channel > 16)
                break;
        }

        // =========================
        // 💾 Écriture fichier
        // =========================
        if (auto stream = file.createOutputStream())
        {
            midi.writeTo(*stream);
            return true;
        }

        return false;
    }
}



GenerationService::GenerationService()
    : juce::Thread("FuxCP Solver Thread"),
      pImpl(std::make_unique<Impl>()),
      ready(false),
      generationSuccess(false)

{
    pImpl->initialized = true;
    ready = true;
    lastError.clear();

}

GenerationService::~GenerationService()
{
    stopThread(-1); // Attendre que le thread se termine
}

bool GenerationService::startGeneration(const CantusProblem& cantusProblem, const juce::String& outputPath, AppController* controller)
{

    if (isThreadRunning())
    {
        lastError = "Une géneration est déjà en cours";
        return false;
    }

    if (!isReady())
    {
        lastError = "Le service n'est pas prêt";
        return false;
    }

    cantusProblemToGenerate = &cantusProblem;
    outputPathToGenerate = outputPath;

    {
        juce::ScopedLock lock(callbackLock);
        appController = controller;
    }

    generationSuccess.store(false);
    lastError.clear();
    startThread();

    return true;
}

void GenerationService::run()
{

    if (cantusProblemToGenerate == nullptr) {
        generationSuccess.store(false);
        lastError = "Problème invalide (nullptr)";
        return;
    }

    bool success = generateMidiFromInputs(*cantusProblemToGenerate, outputPathToGenerate);
    generationSuccess.store(success);

    AppController* controllerToNotify = nullptr;
    {
        juce::ScopedLock lock(callbackLock);
        controllerToNotify = appController;
    }

    if (controllerToNotify != nullptr)
        controllerToNotify->triggerAsyncUpdate();
}

bool GenerationService::isGenerating() const { return isThreadRunning(); }
bool GenerationService::getLastGenerationSuccess() const { return generationSuccess.load(); }
juce::String GenerationService::getLastGeneratedMidiPath() const { return lastGeneratedMidiPath; }

static CantusProblem::CostParameters createDefaultCosts()
{
    return {
            {0, 1, 1, 576, 2, 2, 2, 1},                 // melodic
            {4, 1, 1, 2, 2, 2, 8, 1},                   // general
            {8, 4, 0, 2, 1, 8, 50},                     // specific
            {8, 7, 5, 2, 9, 3, 14, 12, 6, 11, 4, 10, 1, 13} // importance
    };
}

bool GenerationService::generateMidiFromInputs(const CantusProblem& problem, const juce::String& outputPath)
{
    inputValidationError = false;

    if (!ready) {
        lastError = "Le service n\'est pas prêt";
        return false;
    }

    if (problem.isEmpty()) {
        inputValidationError = true;
        lastError = "Le problème est vide. \n \n Entrez un problème complet ";
        return false;
    }


    if (problem.getVoiceCount() == -1)
    {
        inputValidationError = true;
        lastError = "No voices defined.";
        return false;
    }



    CounterpointProblem* fuxProblem = createFuxProblem(problem);

    if (fuxProblem == nullptr)
    {
        lastError = "Erreur création problème Fux";
        return false;
    }

    // =========================
    // SOLVEUR (copié FuxTest)
    // =========================
    BAB<CounterpointProblem> engine(fuxProblem);

    int nb_sol = 0;

    try {
        while (CounterpointProblem* pb = engine.next())
        {
            nb_sol++;

            std::string solutionStr = pb->to_string();
            DBG(solutionStr);

            // 🔥 extraction
            auto solution = extractSolution(solutionStr);

            int cfSize = problem.getCantusFirmus().size();
            int numVoices = problem.getVoiceCount();

            // sécurité
            if (solution.size() == numVoices * cfSize)
            {
                auto voices = splitVoices(solution, numVoices, cfSize);

                if (!voices.empty())
                    voices.erase(voices.begin());

                juce::File desktop = juce::File::getSpecialLocation(
                    juce::File::userDesktopDirectory);

                juce::File midiFile = desktop.getChildFile("fux_solution.mid");

                writeMidiFile(problem.getCantusFirmus(), voices, midiFile);
            }
            else
            {
                DBG("Erreur taille solution !");
            }

            delete pb;
            break;
        }
    }
    catch (...)
    {

    }

    delete fuxProblem;
    // =========================
    // RESULTAT (identique FuxTest)
    // =========================

    if (nb_sol > 0)
    {
        lastError.clear();
        return true;
    }
    else
    {
        lastError = "Aucune solution trouvée";
        return false;
    }


}

CounterpointProblem* GenerationService::createFuxProblem(const CantusProblem& problem)
{
    // =========================
    // EXTRACTION DONNÉES MODÈLE
    // =========================

    const auto& cf = problem.getCantusFirmus();
    auto spList = problem.getSpeciesList();
    auto v_type = problem.getVoiceTypes();

    auto costs = problem.getCostParameters();

    // =========================
    // CONVERSION (si nécessaire)
    // =========================

    // Si ton create_problem attend vector<Species> → convertir
    std::vector<Species> spListFux;
    spListFux.reserve(spList.size());

    for (int s : spList)
        spListFux.push_back(mapSpeciesIntToFux(s)); // déjà dans ton cpp 👍

    // =========================
    // CREATION PROBLEME FUX
    // =========================

    CounterpointProblem* pb = create_problem(
        cf,
        spListFux,
        v_type,
        costs.melodic,
        costs.general,
        costs.specific,
        costs.importance,
        problem.getBorrowMode()
    );

    return pb;
}


/*bool GenerationService::validateCantusFirmus(const std::vector<int>& cantusFirmus, juce::String& err)
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
}*/




bool GenerationService::isReady() const { return ready && pImpl && pImpl->initialized; }
juce::String GenerationService::getLastError() const { return lastError; }
bool GenerationService::isInputValidationError() const { return inputValidationError; }


void GenerationService::reset()
{
    lastError.clear();
    if (pImpl)
        pImpl->initialized = true;
    ready = true;
}
