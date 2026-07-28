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
/*
   =====================================================================
   GenerationService.cpp — lance FuxCP et génère le MIDI

   Ce fichier fait le pont entre l'application JUCE et le solveur FuxCP.

   Rôle général :
     1. recevoir un CantusProblem depuis l'application,
     2. le convertir en données compréhensibles par FuxCP,
     3. lancer la recherche Gecode,
     4. récupérer la meilleure solution,
     5. écrire cette solution dans un fichier MIDI.

    -> C'est le coeur du côté opérationnel

    Sources:
    - Des méthodes dans ce fichier sont inspirées ou entièrement tirées du fichier GenerationService.cpp
    réalisé par Cédric Niyikiza (Disponible sur ce github: https://github.com/cedricniyi/DiatonyDawApplication)

   =====================================================================
*/

//==============================================================================
// Impl interne
//==============================================================================

struct GenerationService::Impl
{
    bool initialized = false;
};


//  ==============================================================================
//  Part1. Fonctions utilitaires internes au fichier
//
//  Les fonctions dans ce namespace ne sont utilisés que par GenerationService
//  Elles évitent de compliquer les méthodes principales
//
//  ==============================================================================
namespace
{
    /*
        Convertit l'espèce choisie dans l'interface vers le type attendu par FuxCP.

        Exemple :
        - 1 devient FIRST_SPECIES
        - 2 devient SECOND_SPECIES
        - 5 devient FIFTH_SPECIES

        Si la valeur reçue est inconnue, on utilise FIRST_SPECIES par défaut.
    */
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

    /*
        Active toutes les contraintes FuxCP.

        FuxCP utilise un tableau global activeConstraints.
        Avant de créer un problème, on remet toutes les contraintes à true
        pour partir d'un état complet et prévisible.
    */
    void activateDefaultFuxConstraints()
    {
        std::fill(activeConstraints.begin(), activeConstraints.end(), true);
    }


    /*
    Découpe la solution brute retournée par FuxCP en voix séparées.

    FuxCP retourne ici uniquement les notes
    des contrepoints générés.

    Le Cantus Firmus est déjà connu séparément
    dans CantusProblem.

    Exemple :
    solution = {
        67, 69, 71,   // notes de CP1
        55, 57, 59    // notes de CP2
    }

    totalCounterpoints = 2
    cfSize = 3

    Résultat :
    {
        {67, 69, 71}, // CP1
        {55, 57, 59}  // CP2
    }

    Principe :
    chaque contrepoint occupe un bloc de cfSize notes.
    On découpe donc le grand vecteur en blocs de taille cfSize.
*/
    std::vector<std::vector<int>> splitVoices(const std::vector<int>& solution,
                                              int totalCounterpoints,
                                              int cfSize)
    {
        std::vector<std::vector<int>> result;

        // Si les dimensions sont invalides, on renvoie une liste vide.
        if (cfSize <= 0 || totalCounterpoints <= 0)
            return result;

        // Si la solution n'a pas la taille attendue, on ne peut pas la découper.
        if ((int) solution.size() != totalCounterpoints * cfSize)
            return result;

        // Chaque contrepoint occupe un bloc continu de cfSize notes.
        for (int voice = 0; voice < totalCounterpoints; ++voice)
        {
            int start = voice * cfSize;
            int end   = start + cfSize;

            // Exemple:
            //  solution = {67, 69, 71, 55, 57, 59};
            //  cfSize = 3;
            //
            result.emplace_back(solution.begin() + start, // pointe sur 67
                                solution.begin() + end); // pointe juste après 71
            // Résultat {67, 69, 71} ajouté
            // ça pour chaque contrepoint
        }

        return result;
    }

    /*
        Écrit la solution dans un fichier MIDI.

        Le fichier MIDI contient :
        - une piste pour le Cantus Firmus
        - une piste par contrepoint généré

        Chaque voix est écrite avec les mêmes durées de notes.
        Le Cantus Firmus est placé sur le canal 1.
        Les contrepoints commencent au canal 2.

        Retourne :
        - true  -> le fichier MIDI a bien été écrit
        - false -> l'écriture du fichier a échoué
    */

    // --> Utilisation de ChatGPT pour aider à construire cette méthode
    bool writeMidiFile(const std::vector<int>& cantusFirmus,
                       const std::vector<std::vector<int>>& counterpointVoices,
                       const juce::File& file)
    {
        juce::MidiFile midi;

        /*
        Définit la résolution temporelle du fichier MIDI.

        960 ticks = une noire.

        plus cette valeur est grande, plus le MIDI peut placer les notes
        avec précision dans le temps.
        */
        midi.setTicksPerQuarterNote(960); // Provient de la documentation juce
                                          // MidiFile::setRicksPerQuarterNote

        const int ticksPerNote = 960;

        // =========================
        // Track 1 : Cantus Firmus
        // =========================
        juce::MidiMessageSequence cfTrack;

        /*
            Écrit chaque note du Cantus Firmus dans la piste MIDI.

            Technique utilisée :
            pour chaque note, on crée deux événements MIDI :
            - noteOn  : démarre la note au temps `start`
            - noteOff : arrête la note au temps `end`

            JUCE recommande ce modèle dans sa documentation MidiMessage :
            on crée un message avec MidiMessage::noteOn(...), puis le noteOff
            correspondant avec MidiMessage::noteOff(...).

            La classe MidiMessageSequence permet ensuite d'ajouter ces événements
            avec addEvent(message, time), où `time` indique la position temporelle
            de l'événement dans la piste.

            Sources :
            - JUCE MidiMessage::noteOn / noteOff :
              https://docs.juce.com/develop/classjuce_1_1MidiMessage.html
            - JUCE MidiMessageSequence::addEvent :
              https://docs.juce.com/develop/classjuce_1_1MidiMessageSequence.html
            - Tutoriel JUCE "Create MIDI data" :
              https://juce.com/tutorials/tutorial_midi_message/
        */
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

        // On commence donc les contrepoints au canal MIDI 2
        int channel = 2;

        // On parcourt chaque contrepoint généré.
        // Chaque `voice` représente une voix complète : CP1, CP2, CP3
        for (const auto& voice : counterpointVoices)
        {
            // On crée une piste MIDI séparée pour ce contrepoint.
            juce::MidiMessageSequence track;

            // On parcourt toutes les notes de cette voix.
            for (size_t i = 0; i < voice.size(); ++i)
            {

                // La note i commence après i durées de note.
                // Exemple avec ticksPerNote = 960 :
                // i = 0 -> start = 0
                // i = 1 -> start = 960
                // i = 2 -> start = 1920
                double start = (double) i * ticksPerNote;
                // La note s'arrête juste avant la note suivante.
                // Exemple :
                // i = 0 -> end = 960
                // i = 1 -> end = 1920
                // i = 2 -> end = 2880
                double end   = (double) (i + 1) * ticksPerNote;

                // Démarre la note MIDI au temps `start`.
                // channel : canal MIDI utilisé pour cette voix.
                // voice[i] : hauteur MIDI de la note.
                // 100 : vélocité, donc intensité de la note.
                track.addEvent(juce::MidiMessage::noteOn(channel, voice[i], (juce::uint8) 100), start);
                // Arrête la même note au temps `end`.
                track.addEvent(juce::MidiMessage::noteOff(channel, voice[i]), end);
            }

            // On ajoute la piste complète de ce contrepoint au fichier MIDI.
            midi.addTrack(track);

            // On passe au canal MIDI suivant pour le prochain contrepoint.
            if (++channel > 16) // MIDI possède seulement 16 canaux standards, donc on s'arrête après 16.
                break;
        }

        // On essaie de créer un fichier en écriture à l'emplacement demandé.
        if (auto stream = file.createOutputStream())
        {
            // Si le fichier peut être ouvert, on écrit tout le contenu MIDI dedans.
            midi.writeTo(*stream);
            return true;
        }

        return false;
    }

    /*
        Additionne deux vecteurs de coûts mélodiques.

        Dans FuxCP, les fonctions steps1(s) et steps2(s) produisent chacune
        un vecteur de coûts mélodiques dans le même ordre :

        seconde, tierce, quarte, triton, quinte, sixte, septième, octave.

        Cette méthode permet de combiner leurs effets.

        Exemple :
        - steps1(s) règle le mouvement mélodique : conjoint ou sauts
        - steps2(s) règle la couleur des intervalles : dissonances ou parfaits

        --> En les additionnant, on obtient un seul vecteur m_costs qui tient compte
        des deux paramètres.

        Le maximum est limité à 576 car FuxCP utilise cette valeur comme coût
        très élevé, presque équivalent à une interdiction.
    */
    std::vector<int> addMelodicCostVectors(std::vector<int> target,
                                           const std::vector<int>& source)
    {
        const auto size = std::min(target.size(), source.size());

        for (std::size_t i = 0; i < size; ++i)
            // On additionne coût par coût, sans jamais dépasser le plafond FuxCP.
            target[i] = std::min(target[i] + source[i], 576);

        return target;
    }

    /*
        Appelle les shapes définies par Dorian dans FuxCP.
        L'interface choisit seulement le type de shape.
    */
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

    /*
        Étend les valeurs de contrôle d'une shape sur toute la pièce.

        L'interface manipule quelques valeurs simples, par exemple :
        {a, b, c, d}

        FuxCP attend une valeur par position.
        Donc avec size = 8 :
        {a, b, c, d} devient {a, a, b, b, c, c, d, d}
    */
    std::vector<double> buildShapeValuesFromControls(const std::vector<double>& controlValues,
                                                     int size)
    {
        std::vector<double> shape;
        shape.reserve(static_cast<std::size_t>(std::max(0, size)));

        // Taille invalide : aucune shape à construire.
        if (size <= 0)
            return shape;

        // Aucune valeur reçue : on renvoie une shape neutre.
        if (controlValues.empty())
            return std::vector<double>(static_cast<std::size_t>(size), 0.0);

        // Nombre de parties disponibles dans la shape.
        const int partCount = static_cast<int>(controlValues.size()); // Exemple : {a, b, c, d} -> 4 parties.

        for (int measureIndex = 0; measureIndex < size; ++measureIndex)
        {
            // Convertit la mesure courante vers une partie de shape.
            // Exemple : avec 8 mesures et 4 parties, les mesures 0-1 utilisent part1.
            const int partIndex = std::min(partCount - 1,
                                           measureIndex * partCount / size);

            // Ajoute la valeur choisie dans la shape finale.
            shape.push_back(controlValues[static_cast<std::size_t>(partIndex)]);
        }

        return shape;
    }

    /*
        Choisit la shape réellement envoyée à FuxCP.

        Si l'utilisateur a modifié les 4 sliders, on utilise ces valeurs.
        Sinon, on retombe sur la shape de base définie par Dorian.

        Exemple :
        - assignment.shape = invertedV
        - assignment.controlValues = {0.0, 1.0, 1.0, 0.0}

        La méthode utilise les valeurs personnalisées :
        {0.0, 1.0, 1.0, 0.0}

    */
    std::vector<double> buildShapeValuesForAssignment(const ConstraintSettings::ShapeAssignment& assignment,
                                                      int size)
    {
        // L'utilisateur a modifié les 4 parties de la shape dans l'interface.
        if (! assignment.controlValues.empty())
            return buildShapeValuesFromControls(assignment.controlValues, size);

        // Sinon, on utilise la shape standard de Dorian.
        return buildShapeValues(assignment.shape, size);
    }

    /*
        Construit le CostModel complet envoyé à FuxCP.

        C'est ici que les réglages de l'interface deviennent des données
        exploitables par le solveur :
        - coûts par défaut,
        - sliders propres à chaque contrepoint,
        - shapes choisies dans l'interface.
    */
    CostModel buildCostModelFromSettings(const ConstraintSettings& settings,
                                         const std::vector<int>& melodicCosts,
                                         const std::vector<int>& generalCosts,
                                         const std::vector<int>& specificCosts,
                                         int measureCount,
                                         int counterpointCount)
    {
        CostModel costModel;

        // Crée une shape constante : même valeur du slider sur toute la pièce.
        auto makeConstantShape = [measureCount](double value)
        {
            return std::vector<double>(static_cast<std::size_t>(measureCount), value);
        };

        // ============================================================
        // 1. Coûts par défaut
        // ============================================================
        // Ces coûts servent de base à FuxCP avant d'appliquer les sliders
        // et les shapes propres à chaque voix.

        //1.1 Coûts généraux de base
        costModel.melodicDefaultCosts = melodicCosts;

        //1.2 Coûts généraux de base
        costModel.defaultCosts[COST_BORROW] = generalCosts[ConstraintSettings::borrowCost];
        costModel.defaultCosts[COST_FIFTH] = generalCosts[ConstraintSettings::harmonicFifthCost];
        costModel.defaultCosts[COST_OCTAVE] = generalCosts[ConstraintSettings::harmonicOctaveCost];
        costModel.defaultCosts[COST_SUCC] = generalCosts[ConstraintSettings::successiveCost];
        costModel.defaultCosts[COST_VARIETY] = generalCosts[ConstraintSettings::varietyCost];
        costModel.defaultCosts[COST_TRIAD] = generalCosts[ConstraintSettings::triadCost];
        costModel.defaultCosts[COST_DIRECT] = generalCosts[ConstraintSettings::directMotionCost];
        costModel.defaultCosts[COST_PENULT] = generalCosts[ConstraintSettings::penultCost];

        //1.3 Coûts spécifiques de base
        costModel.defaultCosts[COST_CAMBIATA] = specificCosts[ConstraintSettings::cambiataCost];
        costModel.defaultCosts[COST_TRIAD3] = specificCosts[ConstraintSettings::triad3rdCost];
        costModel.defaultCosts[COST_M2] = specificCosts[ConstraintSettings::m2ZeroCost];
        costModel.defaultCosts[COST_SYNCOPATION] = specificCosts[ConstraintSettings::syncopationCost];

        // Vérifie si une shape explicite existe déjà pour une voix et une fonction de coût.
        // Si oui, on ne doit pas aussi ajouter le slider constant pour ce même réglage.
        auto hasExplicitShape = [&settings](int voiceIndex, ConstraintSettings::ShapeCostTarget target)
        {
            for (const auto& assignment : settings.getShapeAssignments())
                if (assignment.voiceIndex == voiceIndex && assignment.target == target)
                    return true;

            return false;
        };

        // ============================================================
        // 2. Sliders par contrepoint
        // ============================================================
        // Chaque slider devient un CostGroup.
        // Comme le slider est fixe sur toute la pièce, on le transforme
        // en shape constante : {value, value, value, ...}.

        CostGroup melodyMovementGroup;
        melodyMovementGroup.costIndices = { COST_MELODIC };
        melodyMovementGroup.fn = steps1;
        melodyMovementGroup.shapePerVoice.resize(static_cast<std::size_t>(counterpointCount));

        CostGroup intervalColourGroup;
        intervalColourGroup.costIndices = { COST_MELODIC };
        intervalColourGroup.fn = steps2;
        intervalColourGroup.shapePerVoice.resize(static_cast<std::size_t>(counterpointCount));

        CostGroup perfectIntervalsGroup;
        perfectIntervalsGroup.costIndices = { COST_FIFTH, COST_OCTAVE };
        perfectIntervalsGroup.fn = harmo;
        perfectIntervalsGroup.shapePerVoice.resize(static_cast<std::size_t>(counterpointCount));

        for (int voiceIndex = 0; voiceIndex < counterpointCount; ++voiceIndex)
        {
            // Récupère les valeurs des sliders propres à ce contrepoint.
            const auto parameters = settings.getCounterpointCostParameters(voiceIndex);
            const auto index = static_cast<std::size_t>(voiceIndex);

            // Si aucune shape ne remplace ce réglage, on utilise la valeur du slider.
            if (! hasExplicitShape(voiceIndex, ConstraintSettings::ShapeCostTarget::melodyMovement))
                melodyMovementGroup.shapePerVoice[index] =
                    makeConstantShape(parameters.melodyMovement);

            if (! hasExplicitShape(voiceIndex, ConstraintSettings::ShapeCostTarget::intervalColour))
                intervalColourGroup.shapePerVoice[index] =
                    makeConstantShape(parameters.intervalColour);

            if (! hasExplicitShape(voiceIndex, ConstraintSettings::ShapeCostTarget::perfectIntervals))
                perfectIntervalsGroup.shapePerVoice[index] =
                    makeConstantShape(parameters.perfectIntervals);
        }

        // On ajoute les trois familles de sliders au modèle envoyé à FuxCP.
        costModel.addGroup(std::move(melodyMovementGroup));
        costModel.addGroup(std::move(intervalColourGroup));
        costModel.addGroup(std::move(perfectIntervalsGroup));

        // ============================================================
        // 3. Shapes explicites
        // ============================================================
        // Une shape explicite remplace le slider constant pour une voix donnée.
        // Exemple : CP1 + Melody movement + Linear.

        for (const auto& assignment : settings.getShapeAssignments())
        {
            // Sécurité
            if (assignment.voiceIndex < 0 || assignment.voiceIndex >= counterpointCount)
                continue;

            // Crée un groupe de coût dédié à cette shape.
            // --> Il décrit quelle fonction de coût est modifiée,
            //     sur quelle voix, et avec quelle shape.
            CostGroup shapeGroup;
            shapeGroup.shapePerVoice.resize(static_cast<std::size_t>(counterpointCount));

            const auto voiceIndex = static_cast<std::size_t>(assignment.voiceIndex);

            // Construit la shape finale envoyée à FuxCP pour cette voix.
            shapeGroup.shapePerVoice[voiceIndex] =
                buildShapeValuesForAssignment(assignment, measureCount);

            // Associe la shape à la bonne fonction de coût.
            switch (assignment.target)
            {
                case ConstraintSettings::ShapeCostTarget::melodyMovement:
                    shapeGroup.costIndices = { COST_MELODIC };
                    shapeGroup.fn = steps1;
                    break;

                case ConstraintSettings::ShapeCostTarget::intervalColour:
                    shapeGroup.costIndices = { COST_MELODIC };
                    shapeGroup.fn = steps2;
                    break;

                case ConstraintSettings::ShapeCostTarget::perfectIntervals:
                    shapeGroup.costIndices = { COST_FIFTH, COST_OCTAVE };
                    shapeGroup.fn = harmo;
                    break;
            }
            // Transfère ce groupe dans le CostModel.
            // Après ça, FuxCP pourra appliquer cette shape à la bonne voix et au bon coût.
            costModel.addGroup(std::move(shapeGroup));
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
        // La méthode vient du modèle : BAB par défaut, DFS choisit ça
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
    const CostModel costModel = buildCostModelFromSettings(
        settings,
        melodicStorage,
        generalStorage,
        specificStorage,
        static_cast<int>(cf.size()),
        static_cast<int>(counterpoints.size())
    );

    std::cout << "\n=== PER-VOICE SLIDERS SENT TO FUXCP ===\n";
    for (int voiceIndex = 0; voiceIndex < static_cast<int>(counterpoints.size()); ++voiceIndex)
    {
        const auto parameters = settings.getCounterpointCostParameters(voiceIndex);

        std::cout << "CP " << (voiceIndex + 1)
                  << " melodyMovement=" << parameters.melodyMovement
                  << " intervalColour=" << parameters.intervalColour
                  << " perfectIntervals=" << parameters.perfectIntervals
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
