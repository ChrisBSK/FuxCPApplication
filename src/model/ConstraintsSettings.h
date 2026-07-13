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
            Paramètre s de steps1(s), défini dans FuxCP par Dorian.

            0.0 = favorise les mouvements conjoints.
            1.0 = favorise les sauts.
        */
        double avoidLargeLeap = 0.0;

        /*
            Paramètre s de steps2(s), défini dans FuxCP par Dorian.

            0.0 = pénalise fortement les dissonances mélodiques.
            1.0 = pénalise fortement les consonances parfaites.
        */
        double melodicIntervalColor = 0.0;


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

    struct Harmonic
    {
        /*
            Paramètre s de harmo(s), défini dans FuxCP par Dorian.

            0.0 = pénalise les octaves.
            1.0 = pénalise les quintes.
        */
        double perfectIntervalBalance = 0.0;
    };

    struct Specific
    {
        // TODO
    };

    struct Global
    {
        int borrowMode = 1;
    };

    struct Importance
    {
        /*
            Chaque valeur est le rang d'une contrainte dans l'optimisation.

            1 = priorité la plus forte.
            14 = priorité la plus faible.
        */
        std::vector<int> costs {
            14,  // borrow
            6,  // fifth
            5,  // octave
            2,  // succ
            9,  // variety
            3,  // triad
            8, // direct
            10, // motion
            12,  // penult
            11, // cambiata
            4,  // triad3
            13, // m2
            1, // syncopation
            7   // melodic
        };
    };

    //==========================================================================
    // SETTINGS INSTANCES
    //==========================================================================
    Melodic melodic;
    General general;
    Harmonic harmonic;
    Specific specific;
    Global global;
    Importance importance;

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
        melodic.avoidLargeLeap = value;
    }

    double getLargeLeapPenalty() const
    {
        return melodic.avoidLargeLeap;
    }

    void setMelodicIntervalColor(double value)
    {
        melodic.melodicIntervalColor = value;
    }

    double getMelodicIntervalColor() const
    {
        return melodic.melodicIntervalColor;
    }

    void setAvoidRepeatedNotes(int value)
    {
        general.avoidRepeatedNotes = value == 0 ? 0 : 1;
    }

    int getAvoidRepeatedNotes() const
    {
        return general.avoidRepeatedNotes;
    }

    void setPerfectIntervalBalance(double value)
    {
        harmonic.perfectIntervalBalance = value;
    }

    double getPerfectIntervalBalance() const
    {
        return harmonic.perfectIntervalBalance;
    }

    void setBorrowMode(int value)
    {
        global.borrowMode = (value == 0 ? 0 : 1);
    }

    int getBorrowMode() const
    {
        return global.borrowMode;
    }

    void setImportanceCosts(const std::vector<int>& values)
    {
        if (values.size() == importance.costs.size())
            importance.costs = values;
    }

    const std::vector<int>& getImportanceCosts() const
    {
        return importance.costs;
    }

private:
    //==========================================================================
    // DEFAULT COSTS
    //==========================================================================
    std::vector<int> buildDefaultMelodicCosts(int cantusFirmusLength) const;
    std::vector<int> buildDefaultGeneralCosts() const;
    std::vector<int> buildDefaultSpecificCosts() const;
    std::vector<int> buildDefaultImportanceCosts() const;

};
