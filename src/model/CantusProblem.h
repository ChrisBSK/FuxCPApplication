//
// Créé par Chris BAKASHIKA (2026)
//

/*
//==============================================================================
   CantusProblem.h

   Modèle contenant toutes les données nécessaires
   à la génération d'un contrepoint.
//==============================================================================
*/

#pragma once

#include <vector>
#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include "ConstraintsSettings.h"


class CantusProblem
{
public:
    // =========================
    // Structures de données
    // =========================


    // Une voix de contrepoint générée : sa position relative au CF et son espèce. (utile: côté UI)
    struct Counterpoint
    {
        int type;     // position relative au CF (ex: -1, 1, 2...)
        int species;  // espèce (1 → 5)
    };

    // L'ensemble des voix d'un problème : le Cantus Firmus et les contrepoints générés autour. (utile: côté modèle)
    struct Voices
    {
        std::vector<int> cf;                       // Cantus Firmus
        std::vector<Counterpoint> counterpoints;   // Voix générées
    };

    // =========================
    // Données musicales
    // =========================

    // Construit un problème vide et calcule les vecteurs de coûts par défaut
    CantusProblem();

    // Remplace le Cantus Firmus et les contrepoints par ceux fournis
    void setVoices(const Voices& v);
    // Retourne le Cantus Firmus et les contrepoints du problème
    const Voices& getVoices() const;


    // Retourne uniquement le Cantus Firmus
    const std::vector<int>& getCantusFirmus() const;

    // Retourne uniquement les contrepoints
    const std::vector<Counterpoint>& getCounterpoints() const;

    // Retourne le nombre de contrepoints
    size_t getCounterpointCount() const;

    // Définit le nombre total de voix (CF inclus)
    void setVoiceCount(int count);
    // Retourne le nombre total de voix (CF inclus)
    int getVoiceCount() const;


    // =========================
    // Conversion pour le solver
    // =========================

    // Retourne la liste des espèces de chaque contrepoint (CP uniquement)
    std::vector<int> getSpeciesList() const;
    // Retourne la liste des types de chaque contrepoint (CP uniquement)
    std::vector<int> getVoiceTypes() const;

    // =========================
    // Paramètres du solveur
    // =========================

    // Remplace les paramètres du solveur et recalcule les coûts
    void setSettings(const ConstraintSettings& s);

    // Accès en écriture aux paramètres du solveur
    ConstraintSettings& getSettings();

    // Accès en lecture seule aux paramètres du solveur
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

    /*
    // =========================
       Sauvegarde / Chargement (ValueTree)

       Ces deux méthodes transforment le problème en un seul ValueTree,
       et inversement.

       Ce ValueTree représente TOUT l'état du système
       (Cantus Firmus, voix, réglages du solveur) :

       c'est lui qui est écrit sur le disque par AppController
       pour créer une configuration sauvegardée, puis relu pour la restaurer.
    // =========================
    */

    // Construit un ValueTree représentant l'état complet du problème.
    juce::ValueTree toValueTree() const;

    // Remplace l'état du problème par celui contenu dans ce ValueTree.
    void restoreFromValueTree(const juce::ValueTree& state);

    // =========================
    // Coûts calculés (pour le solveur)
    // =========================

    // Retourne les coûts liés aux mouvements mélodiques.
    const std::vector<int> &getMelodicCosts() const;

    // Retourne les coûts généraux, communs à toutes les voix.
    const std::vector<int> &getGeneralCosts() const;

    // Retourne les coûts spécifiques à l'espèce de chaque contrepoint.
    const std::vector<int> &getSpecificCosts() const;

    // Retourne le vecteur des priorités d'optimisation.
    const std::vector<int> &getImportanceCosts() const;

    // Recalcule tous les vecteurs de coûts à partir des réglages courants.
    void recalculateCosts();

private:
    Voices voices;                 // Données musicales
    ConstraintSettings settings;   // Paramètres solveur
    juce::String title;            // Nom du problème

    int voiceCount = 0;

    std::vector<int> melodicCosts; // coûts liés aux mouvements mélodiques
    std::vector<int> generalCosts; // coûts généraux, communs à toutes les voix
    std::vector<int> specificCosts; // coûts spécifiques à l'espèce de chaque contrepoint
    std::vector<int> importanceCosts; // priorités d'optimisation
};