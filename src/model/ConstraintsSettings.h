#pragma once

#include <vector>

/*
==============================================================================
    ConstraintSettings.h

    Ce fichier définit les paramètres du solveur de contrepoint.

    On distingue clairement deux niveaux :

    1. HARD CONSTRAINTS  (règles strictes)
       → définissent ce qui est autorisé / interdit

    2. SOFT CONSTRAINTS  (coûts / préférences)
       → influencent le choix de la meilleure solution

==============================================================================
*/


//==============================================================================
// HARD CONSTRAINTS (règles structurelles)
//==============================================================================
//
// Ces paramètres modifient directement les règles du contrepoint.
// Ils doivent être respectés pour qu'une solution soit valide.
//
struct HardConstraints
{
    // Intervalle maximal autorisé entre deux notes consécutives
    int maxLeap = 5;

    // Autorise ou non la répétition de notes
    bool allowRepetition = true;

    // Direction globale de la ligne mélodique
    // -1 = descendante, 0 = neutre, +1 = ascendante
    int preferredDirection = 0;
};


//==============================================================================
// SOFT CONSTRAINTS (coûts / préférences)
//==============================================================================
//
// Ces paramètres n'interdisent rien,
// mais guident le solveur vers de meilleures solutions.
//
struct SoftConstraints
{
    // Coûts liés aux règles mélodiques
    std::vector<int> melodic;

    // Coûts globaux
    std::vector<int> general;

    // Coûts spécifiques aux espèces
    std::vector<int> specific;

    // Pondération relative des contraintes
    std::vector<int> importance;
};


//==============================================================================
// GLOBAL SETTINGS
//==============================================================================
//
// Regroupe tous les paramètres du solveur.
//
// → utilisé comme point unique de configuration
//
struct ConstraintSettings
{
    HardConstraints hard;
    SoftConstraints soft;

    // Mode d'emprunt (altérations)
    // 0 = strict
    // 1 = autorisé
    int borrowMode = 0;
};