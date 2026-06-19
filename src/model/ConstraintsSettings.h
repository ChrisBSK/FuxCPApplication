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

    //==========================================================================
    // UI PARAMETERS
    //==========================================================================
    struct Melodic
    {
        double largeLeapPenalty = 0.0;
    };

    struct General
    {
        /*
        Pénalité appliquée lorsque le contrepoint réutilise
        plusieurs fois la même note dans une fenêtre courte.

        Faible valeur  -> répétitions plus acceptées.
        Grande valeur  -> répétitions fortement pénalisées.
    */
        int noteRepetitionValue = 2;
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

    std::vector<int> buildDefaultSpecificCosts() const;

    void applyRepetitionSlider(std::vector<int> &costs) const;

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

    void setNoteRepetitionValue(int value)
    {
        general.noteRepetitionValue = value;
    }

    int getNoteRepetitionValue() const
    {
        return general.noteRepetitionValue;
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

    //==========================================================================
    // UI MODIFIERS
    //==========================================================================
    void applyLargeLeapPenalty(std::vector<int>& costs) const;
};
