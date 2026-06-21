#pragma once

#include <vector>

/*
==============================================================================
    ConstraintSettings.h

    Point unique de configuration du solveur.

    Principe :
    - les coûts FuxCP par défaut restent inchangés
    - les sliders modifient ces coûts via des méthodes dédiées
==============================================================================
*/

struct ConstraintSettings
{
    //==========================================================================
    // MELODIC COST INDEXES
    //==========================================================================
    enum MelodicCostIndex
    {
        secondCost = 0,
        thirdCost,
        fourthCost,
        tritoneCost,
        fifthCost,
        sixthCost,
        seventhCost,
        octaveCost
    };

    enum SpecificCostIndex
    {
        penultSixthCost = 0,
        cambiataCost,
        mSkipCost,
        triad3rdCost,
        m2ZeroCost,
        syncopationCost,
        prefSlider
    };

    enum GeneralCostIndex
    {
        borrowCost = 0,
        harmonicFifthCost,
        harmonicOctaveCost,
        successiveCost,
        varietyCost,
        triadCost,
        directMotionCost,
        penultCost
    };

    //==========================================================================
    // UI PARAMETERS
    //==========================================================================
    struct Melodic
    {
        /*
            0.0 = grands sauts plus libres.
            1.0 = grands sauts fortement pénalisés.
        */
        double largeLeapPenalty = 0.0;
    };

    struct General
    {
        /*
            Contrôle binaire de la variété mélodique FuxCP.

            0 = aucune pénalité de variété.
            1 = pénalité forte sur les notes répétées dans la fenêtre de variété
                déjà définie par FuxCP.
        */
        int avoidRepeatedNotes = 0;
    };

    struct Specific
    {
        // TODO
    };

    struct Global
    {
        int borrowMode = 1;
    };

    //==========================================================================
    // SETTINGS INSTANCES
    //==========================================================================
    Melodic melodic;
    General general;
    Specific specific;
    Global global;

    //==========================================================================
    // BUILD FUX COSTS
    //==========================================================================
    std::vector<int> buildMelodicCosts(int cantusFirmusLength) const;
    std::vector<int> buildGeneralCosts() const;
    std::vector<int> buildSpecificCosts() const;
    std::vector<int> buildImportanceCosts() const;

    //==========================================================================
    // UI SETTERS / GETTERS
    //==========================================================================
    void setLargeLeapPenalty(double value)
    {
        melodic.largeLeapPenalty = value;
    }

    double getLargeLeapPenalty() const
    {
        return melodic.largeLeapPenalty;
    }

    void setAvoidRepeatedNotes(int value)
    {
        general.avoidRepeatedNotes = value == 0 ? 0 : 1;
    }

    int getAvoidRepeatedNotes() const
    {
        return general.avoidRepeatedNotes;
    }
    void setBorrowMode(int value)
    {
        global.borrowMode = (value == 0 ? 0 : 1);
    }

    int getBorrowMode() const
    {
        return global.borrowMode;
    }

private:
    //==========================================================================
    // DEFAULT COSTS
    //==========================================================================
    std::vector<int> buildDefaultMelodicCosts(int cantusFirmusLength) const;
    std::vector<int> buildDefaultGeneralCosts() const;
    std::vector<int> buildDefaultSpecificCosts() const;
    std::vector<int> buildDefaultImportanceCosts() const;

    //==========================================================================
    // UI MODIFIERS
    //==========================================================================
    void applyLargeLeapPenalty(std::vector<int>& costs) const;
    void applyNoteRepetitionPenalty(std::vector<int>& costs) const;
};
