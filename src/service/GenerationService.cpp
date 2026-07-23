#include "GenerationService.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_events/juce_events.h>

#include "CounterpointProblems/CounterpointUtils.hpp"
#include "CounterpointProblems/CounterpointProblem.hpp"
#include "Utilities.hpp"

#include "../controller/AppController.h"

#include <gecode/search.hh>
#include <algorithm>
#include <functional>
#include <memory>

//==============================================================================
// GenerationService
//
// Gère l'exécution du solveur de contrepoint.
//
// Responsable de :
// - lancer la génération
// - exécuter le solveur en arrière-plan
// - récupérer les solutions
// - générer le fichier MIDI correspondant
//==============================================================================

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

    void activateDefaultFuxConstraints()
    {
        std::fill(activeConstraints.begin(), activeConstraints.end(), true);
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

    void printMaxLeap(const std::vector<int>& notes)
    {
        int maxLeap = 0;

        for (size_t i = 1; i < notes.size(); ++i)
        {
            maxLeap = std::max(
                maxLeap,
                std::abs(notes[i] - notes[i - 1]));
        }

        /*std::cout << "Max leap = "
                  << maxLeap
                  << "\n";*/
    }

    // Ajoute source dans target en gardant le plafond utilisé par FuxCP.
    std::vector<int> addMelodicCostVectors(std::vector<int> target,
                                           const std::vector<int>& source)
    {
        const auto size = std::min(target.size(), source.size());

        for (std::size_t i = 0; i < size; ++i)
            target[i] = std::min(target[i] + source[i], 576);

        return target;
    }

    std::vector<double> buildShapeValues(ConstraintSettings::ShapeType type, int size)
    {
        switch (type)
        {
            case ConstraintSettings::ShapeType::fixedZero:       return build_constant_zero_shape(size);
            case ConstraintSettings::ShapeType::fixedOne:        return build_constant_one_shape(size);
            case ConstraintSettings::ShapeType::linear:          return build_linear_shape(size);
            case ConstraintSettings::ShapeType::linearDescending:return build_linear_shape_desc(size);
            case ConstraintSettings::ShapeType::invertedV:       return build_inverted_v_shape(size);
            case ConstraintSettings::ShapeType::v:               return build_v_shape(size);
            case ConstraintSettings::ShapeType::m:               return build_M_shape(size);
            case ConstraintSettings::ShapeType::step:            return build_step_shape(size);
            case ConstraintSettings::ShapeType::stepDescending:  return build_step_desc_shape(size);
        }

        return build_inverted_v_shape(size);
    }

    // Construit une shape complète :
    // - valeur du slider partout
    // - shape choisie seulement entre startMeasure et endMeasure.
    std::vector<double> buildLocalShape(double baseValue,
                                        const ConstraintSettings::ShapeAssignment& assignment,
                                        int measureCount)
    {
        std::vector<double> result(measureCount, baseValue);

        const int start = std::max(0, assignment.startMeasure - 1);
        const int end = std::min(measureCount - 1, assignment.endMeasure - 1);

        if (start > end)
            return result;

        const int localSize = end - start + 1;
        const auto localShape = buildShapeValues(assignment.shape, localSize);

        for (int i = 0; i < localSize; ++i)
            result[start + i] = localShape[static_cast<std::size_t>(i)];

        return result;
    }

    void setCostModelDefaults(CostModel& costModel,
                              const std::vector<int>& melodicCosts,
                              const std::vector<int>& generalCosts,
                              const std::vector<int>& specificCosts)
    {
        costModel.melodicDefaultCosts = melodicCosts;

        costModel.defaultCosts[COST_BORROW] = generalCosts[ConstraintSettings::borrowCost];
        costModel.defaultCosts[COST_FIFTH] = generalCosts[ConstraintSettings::harmonicFifthCost];
        costModel.defaultCosts[COST_OCTAVE] = generalCosts[ConstraintSettings::harmonicOctaveCost];
        costModel.defaultCosts[COST_SUCC] = generalCosts[ConstraintSettings::successiveCost];
        costModel.defaultCosts[COST_VARIETY] = generalCosts[ConstraintSettings::varietyCost];
        costModel.defaultCosts[COST_TRIAD] = generalCosts[ConstraintSettings::triadCost];
        costModel.defaultCosts[COST_DIRECT] = generalCosts[ConstraintSettings::directMotionCost];
        costModel.defaultCosts[COST_PENULT] = generalCosts[ConstraintSettings::penultCost];

        costModel.defaultCosts[COST_CAMBIATA] = specificCosts[ConstraintSettings::cambiataCost];
        costModel.defaultCosts[COST_TRIAD3] = specificCosts[ConstraintSettings::triad3rdCost];
        costModel.defaultCosts[COST_M2] = specificCosts[ConstraintSettings::m2ZeroCost];
        costModel.defaultCosts[COST_SYNCOPATION] = specificCosts[ConstraintSettings::syncopationCost];
    }

    CostGroup makeShapeCostGroup(const ConstraintSettings& settings,
                                 const ConstraintSettings::ShapeAssignment& assignment,
                                 int measureCount,
                                 int counterpointCount)
    {
        CostGroup group;
        group.shapePerVoice.resize(static_cast<std::size_t>(counterpointCount));

        if (assignment.target == ConstraintSettings::ShapeCostTarget::melodyMovement)
        {
            const double intervalColour = settings.getMelodicIntervalColor();

            group.costIndices = { COST_MELODIC };
            group.fn = [intervalColour](double s)
            {
                return addMelodicCostVectors(steps1(s),
                                             steps2(intervalColour));
            };

            group.shapePerVoice[static_cast<std::size_t>(assignment.voiceIndex)] =
                buildLocalShape(settings.getLargeLeapPenalty(), assignment, measureCount);
        }
        else if (assignment.target == ConstraintSettings::ShapeCostTarget::intervalColour)
        {
            const double melodyMovement = settings.getLargeLeapPenalty();

            group.costIndices = { COST_MELODIC };
            group.fn = [melodyMovement](double s)
            {
                return addMelodicCostVectors(steps1(melodyMovement),
                                             steps2(s));
            };

            group.shapePerVoice[static_cast<std::size_t>(assignment.voiceIndex)] =
                buildLocalShape(settings.getMelodicIntervalColor(), assignment, measureCount);
        }
        else
        {
            group.costIndices = { COST_FIFTH, COST_OCTAVE };
            group.fn = harmo;

            group.shapePerVoice[static_cast<std::size_t>(assignment.voiceIndex)] =
                buildLocalShape(settings.getPerfectIntervalBalance(), assignment, measureCount);
        }

        return group;
    }

    CostModel buildCostModelFromSettings(const ConstraintSettings& settings,
                                         const std::vector<int>& melodicCosts,
                                         const std::vector<int>& generalCosts,
                                         const std::vector<int>& specificCosts,
                                         int measureCount,
                                         int counterpointCount)
    {
        CostModel costModel;

        setCostModelDefaults(costModel,
                             melodicCosts,
                             generalCosts,
                             specificCosts);

        for (const auto& assignment : settings.getShapeAssignments())
        {
            if (assignment.voiceIndex < 0 || assignment.voiceIndex >= counterpointCount)
                continue;

            costModel.addGroup(makeShapeCostGroup(settings,
                                                  assignment,
                                                  measureCount,
                                                  counterpointCount));
        }

        return costModel;
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

/*
//==============================================================================
   Démarrage d'une génération

   Vérifie l'état du service, copie le problème courant
   puis lance le thread de génération.
//==============================================================================
*/
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

/*
//==============================================================================
   Exécution du thread

   Lance le solveur en arrière-plan puis notifie
   l'AppController lorsque la génération est terminée.
//==============================================================================
*/
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

    lastError.clear();
    lastGeneratedMidiPath.clear();

    // =========================
    // Vérifications
    // =========================
    if (!ready)
    {
        lastError = "Le service n'est pas prêt";
        return false;
    }

    if (problem.isEmpty())
    {
        inputValidationError = true;

        lastError =
            "Le problème est vide.\n\n"
            "Entrez un problème complet";

        return false;
    }

    // =========================
    // Données
    // =========================
    const auto& cf = problem.getCantusFirmus();
    const int cfSize = (int) cf.size();
    const int numVoices = (int) problem.getVoiceCount();
    const int numCounterpoints = numVoices - 1;
    const int expectedSize = numCounterpoints * cfSize;

    // =========================
    // Création problème
    // =========================
    CounterpointProblem* fuxProblem = createFuxProblem(problem);

    if (fuxProblem == nullptr)
    {
        lastError = "Erreur création problème Fux";
        return false;
    }

    try
    {
        // =========================
        // Timeout Gecode
        // =========================
        Gecode::Search::Options opts;
        Gecode::Search::TimeStop timeout(1000); // Laisse le BAB optimiser les coûts, pas seulement trouver une solution.
        opts.stop = &timeout;
        opts.threads = 1;

        // =========================
        // Solveur
        // =========================
        // La methode vient du modele : BAB par defaut, DFS si l'utilisateur
        // l'a selectionne dans la colonne Search.
        const auto searchMethod = problem.getSettings().getSearchMethod();

        std::unique_ptr<Gecode::Search::Base<CounterpointProblem>> solver;

        if (searchMethod == ConstraintSettings::SearchMethod::bab)
        {
            std::cout << "\n=== SEARCH METHOD SENT TO FUXCP ===\n";
            std::cout << "BAB\n";
            solver = std::make_unique<BAB<CounterpointProblem>>(fuxProblem, opts);
        }
        else
        {
            std::cout << "\n=== SEARCH METHOD SENT TO FUXCP ===\n";
            std::cout << "DFS\n";
            solver = std::make_unique<DFS<CounterpointProblem>>(fuxProblem, opts);
        }

        // =========================
        // Meilleure solution
        // =========================
        CounterpointProblem* best = nullptr;

        while (CounterpointProblem* pb = solver->next()) {

            //on garde la meilleure solution trouvée
            best = pb;
        }

        // =========================
        // Aucune solution
        // =========================
        if (best == nullptr) {
            lastError =
                "Aucune solution trouvée.\n\n"
                "Le problème est peut-être "
                "trop complexe ou aucune "
                "solution n'existe.";

            return false;
        }

        // =========================
        // Solution brute
        // =========================
        int size = best->getSize();
        int *raw = best->return_solution();

        if (raw == nullptr) {


            lastError =
                "Erreur : solution brute invalide.";

            return false;
        }

        // =========================
        // Vérification taille
        // =========================
        if (size != expectedSize) {
            delete[] raw;

            lastError =
                "Erreur : taille de solution invalide.";

            return false;
        }

        // =========================
        // Conversion
        // =========================
        std::vector<int> solution(
                    raw,
                    raw + size);

        delete[] raw;


        // =========================
        // Découpage voix
        // =========================
        auto voices = splitVoices(solution, numCounterpoints, cfSize);

        if (voices.empty())
        {
            lastError =
                "Erreur : découpage des voix invalide.";

            return false;
        }

        // =========================
        // Debug console
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

            printMaxLeap(voices[v]);
        }

        // =========================
        // MIDI
        // =========================
        juce::File midiFile(outputPath);

        if (!writeMidiFile(cf,voices,midiFile))
        {
            lastError =
                "Erreur écriture MIDI";

            return false;
        }

        // =========================
        // Succès
        // =========================
        lastGeneratedMidiPath =
            midiFile.getFullPathName();

        lastError.clear();

        std::cout << "MIDI généré\n";

        return true;
    }
    catch (const std::exception& e)
    {
        lastGeneratedMidiPath.clear();

        lastError =
            juce::String("Erreur solveur : ")
            + e.what();

        return false;
    }
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
    activateDefaultFuxConstraints();

    const auto& settings = problem.getSettings();

    melodicStorage   = settings.buildMelodicCosts(static_cast<int>(cf.size()));
    generalStorage   = settings.buildGeneralCosts();
    specificStorage  = settings.buildSpecificCosts();
    importanceStorage = settings.buildImportanceCosts();
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

    std::cout << "\n=== MELODIC COSTS SENT TO FUXCP ===\n";

    for (int cost : melodicStorage)
        std::cout << cost << " ";

    std::cout << "\nborrowMode = "
              << settings.getBorrowMode()
              << std::endl;

    std::cout << "\n=== GENERAL COSTS SENT TO FUXCP ===\n";
    for (int cost : generalStorage)
        std::cout << cost << " ";
    std::cout << "\n";

    std::cout << "\n=== IMPORTANCE COSTS SENT TO FUXCP ===\n";
    for (int cost : importanceStorage)
        std::cout << cost << " ";
    std::cout << "\n";

    std::cout << "\n";

    // =========================
    //  Création du problème Fux
    // =========================
    if (! settings.getShapeAssignments().empty())
    {
        const CostModel costModel = buildCostModelFromSettings(
            settings,
            melodicStorage,
            generalStorage,
            specificStorage,
            static_cast<int>(cf.size()),
            static_cast<int>(counterpoints.size())
        );

        std::cout << "\n=== COST MODEL SHAPES SENT TO FUXCP ===\n";
        for (const auto& assignment : settings.getShapeAssignments())
        {
            std::cout << "voice=" << assignment.voiceIndex
                      << " start=" << assignment.startMeasure
                      << " end=" << assignment.endMeasure
                      << "\n";
        }

        return create_problem(
            cf,
            spListFux,
            vTypeFux,
            costModel,
            importanceStorage,
            settings.getBorrowMode()
        );
    }

    return create_problem(
        cf,
        spListFux,
        vTypeFux,
        melodicStorage,
        generalStorage,
        specificStorage,
        importanceStorage,
        settings.getBorrowMode()
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
