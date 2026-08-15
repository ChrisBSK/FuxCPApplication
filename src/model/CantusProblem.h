#pragma once

#include <vector>
#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include "ConstraintsSettings.h"

/*
//==============================================================================
   CantusProblem

   Modèle contenant toutes les données nécessaires
   à la génération d'un contrepoint.
//==============================================================================
*/

class CantusProblem
{
public:
    // =========================
    // Structures de données
    // =========================

    //CantusProblem();

    struct Counterpoint
    {
        int type;     // position relative au CF (ex: -1, 1, 2...)
        int species;  // espèce (1 → 5)
    };

    struct Voices
    {
        std::vector<int> cf;                       // Cantus Firmus
        std::vector<Counterpoint> counterpoints;   // Voix générées
    };

    // =========================
    // Données musicales
    // =========================

    CantusProblem();

    void setVoices(const Voices& v);
    const Voices& getVoices() const;

    const std::vector<int>& getCantusFirmus() const;
    const std::vector<Counterpoint>& getCounterpoints() const;

    size_t getCounterpointCount() const;

    void setVoiceCount(int count);
    int getVoiceCount() const;

    // =========================
    // Conversion pour le solver
    // =========================

    std::vector<int> getSpeciesList() const; // CP uniquement
    std::vector<int> getVoiceTypes() const;  // CP uniquement

    // =========================
    // Paramètres du solveur
    // =========================
    void setSettings(const ConstraintSettings& s);

    ConstraintSettings& getSettings();
    const ConstraintSettings& getSettings() const;

    // =========================
    // Métadonnées
    // =========================

    void setTitle(const juce::String& newTitle);
    juce::String getTitle() const;

    // =========================
    // Validation
    // =========================

    bool isEmpty() const;

    // =========================
    // Sauvegarde / Chargement (ValueTree)
    //
    // Ces deux méthodes transforment le problème en un seul ValueTree,
    // et inversement.
    //
    // Ce ValueTree représente TOUT l'état du système
    // (Cantus Firmus, voix, réglages du solveur) : c'est lui qui est
    // écrit sur le disque par AppController pour créer une configuration
    // sauvegardée, puis relu pour la restaurer.
    // =========================

    // Construit un ValueTree représentant l'état complet du problème.
    juce::ValueTree toValueTree() const;

    // Remplace l'état du problème par celui contenu dans ce ValueTree.
    void restoreFromValueTree(const juce::ValueTree& state);

    const std::vector<int> &getMelodicCosts() const;

    const std::vector<int> &getGeneralCosts() const;

    const std::vector<int> &getSpecificCosts() const;

    const std::vector<int> &getImportanceCosts() const;

    void recalculateCosts();

private:
    Voices voices;                 // Données musicales
    ConstraintSettings settings;   // Paramètres solveur
    juce::String title;            // Nom du problème

    int voiceCount = 0;

    std::vector<int> melodicCosts;
    std::vector<int> generalCosts;
    std::vector<int> specificCosts;
    std::vector<int> importanceCosts;
};