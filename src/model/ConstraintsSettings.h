#pragma once

#include <vector>

/*
==============================================================================
    ConstraintSettings.h

    Ce fichier définit les paramètres du solveur de contrepoint.

==============================================================================
*/



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

    std::vector<int> buildImportanceCosts() const;

    int getBorrowMode() const
    {
        return borrowMode;
    }

    void setBorrowMode(int value)
    {
        borrowMode = (value == 0 ? 0 : 1);
    }
};