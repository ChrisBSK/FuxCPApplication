#pragma once

#include <utility>
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

    enum class ShapeCostTarget
    {
        melodyMovement = 0,
        intervalColour,
        perfectIntervals
    };

    enum class ShapeType
    {
        fixedZero = 0,
        fixedOne,
        linear,
        linearDescending,
        invertedV,
        v,
        m,
        step,
        stepDescending
    };

    enum class SearchMethod
    {
        dfs = 0,
        bab
    };

    struct ShapeAssignment
    {
        // 0 = Contrepoint 1, 1 = Contrepoint 2, etc.
        int voiceIndex = 0;

        // Fonction de coût pilotée par la shape.
        ShapeCostTarget target = ShapeCostTarget::melodyMovement;

        // Forme choisie dans l'interface.
        ShapeType shape = ShapeType::invertedV;

        // Mesures 1-based, comme affiché dans l'interface.
        int startMeasure = 1;
        int endMeasure = 1;
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
        SearchMethod searchMethod = SearchMethod::bab;
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
    std::vector<ShapeAssignment> shapeAssignments;

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

    void setSearchMethod(SearchMethod method)
    {
        global.searchMethod = method;
    }

    SearchMethod getSearchMethod() const
    {
        return global.searchMethod;
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

    void setShapeAssignments(std::vector<ShapeAssignment> values)
    {
        shapeAssignments = std::move(values);
    }

    const std::vector<ShapeAssignment>& getShapeAssignments() const
    {
        return shapeAssignments;
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
