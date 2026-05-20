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
/*truct HardConstraints
{
    // Intervalle maximal autorisé entre deux notes consécutives
    int maxLeap = 5;

    // Autorise ou non la répétition de notes
    bool allowRepetition = true;

    // Direction globale de la ligne mélodique
    // -1 = descendante, 0 = neutre, +1 = ascendante
    int preferredDirection = 0;
};*/


//==============================================================================
// SOFT CONSTRAINTS (coûts / préférences)
//==============================================================================
//
// Ces paramètres n'interdisent rien,
// mais guident le solveur vers de meilleures solutions.
//
/*struct SoftConstraints
{
    // Coûts liés aux règles mélodiques
    std::vector<int> melodic =
    {
        0, 1, 1, 576, 2, 2, 2, 1
    };

    // Coûts globaux
    std::vector<int> general =
    {
        4, 1, 1, 2, 2, 2, 8, 1
    };

    // Coûts spécifiques aux espèces
    std::vector<int> specific =
    {
        8, 4, 0, 2, 1, 8, 50
    };

    // Pondération relative des contraintes
    std::vector<int> importance =
    {
        8, 7, 5, 2, 9, 3, 14, 12, 6, 11, 4, 10, 1, 13
    };
};*/


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
    // UI PARAMETERS
    // =========================

    int leapPenalty = 50;


    int borrowMode = 1;


    // =========================
    // BUILD FUX COSTS
    // =========================

    std::vector<int> buildMelodicCosts() const;

    std::vector<int> buildGeneralCosts() const;

    std::vector<int> buildSpecificCosts() const;

    std::vector<int> buildImportance() const;

    int getBorrowMode() const
    {
        return borrowMode;
    }

    void setBorrowMode(int value)
    {
        borrowMode = (value == 0 ? 0 : 1);
    }
};