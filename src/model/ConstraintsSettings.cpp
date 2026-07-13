#include "ConstraintsSettings.h"

#include "CostModel.hpp"

#include <algorithm>
#include <cstddef>

/*
==============================================================================
    ConstraintSettings.cpp

    Centralise les paramètres de pondération du solveur.

    Principe :
    - chaque méthode publique construit un vecteur complet pour FuxCP
    - chaque vecteur part d'une configuration de base lisible
    - les sliders modifient ensuite uniquement les coûts qui les concernent
==============================================================================
*/

namespace
{
    constexpr int maximumMelodicCost = 576;

    void addMelodicCosts(std::vector<int>& target,
                         const std::vector<int>& source)
    {
        const auto size = std::min(target.size(), source.size());

        for (std::size_t i = 0; i < size; ++i)
            target[i] = std::min(target[i] + source[i], maximumMelodicCost);
    }
}

//==============================================================================
// MELODIC COSTS
//==============================================================================

/*
    Construit le vecteur des coûts mélodiques utilisé par FuxCP.

    Le slider "Melody movement" pilote directement la fonction steps1(s)
    définie par Dorian dans FuxCP :

    - s = 0 favorise les mouvements conjoints.
    - s = 1 favorise les sauts.
*/
std::vector<int> ConstraintSettings::buildMelodicCosts(int cantusFirmusLength) const
{
    (void) cantusFirmusLength;

    auto melodicCosts = steps1(melodic.avoidLargeLeap);

    addMelodicCosts(melodicCosts, steps2(melodic.melodicIntervalColor));

    return melodicCosts;
}

/*
    Coûts mélodiques par défaut de FuxCP/Dorian.
*/
std::vector<int> ConstraintSettings::buildDefaultMelodicCosts(int cantusFirmusLength) const
{
    (void) cantusFirmusLength;

    auto melodicCosts = steps1(0.0);

    addMelodicCosts(melodicCosts, steps2(0.0));

    return melodicCosts;
}




//==============================================================================
// GENERAL COSTS
//==============================================================================

/*
    Correspondance avec Part.cpp :

    borrowCost      = g_costs[0];
    h_fifthCost     = g_costs[1];
    h_octaveCost    = g_costs[2];
    succCost        = g_costs[3];
    varietyCost     = g_costs[4];
    triadCost       = g_costs[5];
    directMoveCost  = g_costs[6];
    penultCost      = g_costs[7];
*/
std::vector<int> ConstraintSettings::buildGeneralCosts() const
{
    auto generalCosts = buildDefaultGeneralCosts();
    const auto harmonicCosts = harmo(harmonic.perfectIntervalBalance);

    generalCosts[harmonicFifthCost] = harmonicCosts[0];
    generalCosts[harmonicOctaveCost] = harmonicCosts[1];

    return generalCosts;
}

/*
    Coûts généraux de base, avant action des sliders.

    Ils correspondent à g_costs dans FuxCP.
*/
std::vector<int> ConstraintSettings::buildDefaultGeneralCosts() const
{

    return
    {
        4,       // borrowCost
        1,       // h_fifthCost
        1,       // h_octaveCost
        2,       // succCost
        2,       // varietyCost
        2,       // triadCost
        8,       // directMoveCost
        1        // penultCost
    };
}




//==============================================================================
// SPECIFIC COSTS
//==============================================================================

/*
    Correspondance avec Part.cpp :

    penultSixthCost = s_costs[0];
    cambiataCost    = s_costs[1];
    mSkipCost       = s_costs[2];
    triad3rdCost    = s_costs[3];
    m2ZeroCost      = s_costs[4];
    syncopationCost = s_costs[5];
    prefSlider      = s_costs[6];
*/
std::vector<int> ConstraintSettings::buildSpecificCosts() const
{
    auto specificCosts = buildDefaultSpecificCosts();

    return specificCosts;
}

/*
    Coûts spécifiques de base, avant action de futurs sliders.

    Ils correspondent à s_costs dans FuxCP.
*/
std::vector<int> ConstraintSettings::buildDefaultSpecificCosts() const
{
    return
    {
        8,  // penultSixthCost
        4,  // cambiataCost
        0,  // mSkipCost
        2,  // triad3rdCost
        1,  // m2ZeroCost
        8,  // syncopationCost
        50  // prefSlider
    };
}


//==============================================================================
// IMPORTANCE COSTS
//==============================================================================

/*
    Niveaux d'importance utilisés lors de l'optimisation.
*/

/*
    Correspondance avec FuxCP :

    importance[0]  = borrow
    importance[1]  = fifth
    importance[2]  = octave
    importance[3]  = succ
    importance[4]  = variety
    importance[5]  = triad
    importance[6]  = direct
    importance[7]  = motion
    importance[8]  = penult
    importance[9]  = cambiata
    importance[10] = triad3
    importance[11] = m2
    importance[12] = syncopation
    importance[13] = melodic
*/
std::vector<int> ConstraintSettings::buildImportanceCosts() const
{
    return importance.costs;
}

/*
    Ordre d'importance des critères pendant l'optimisation.

    Plus le nombre est petit, plus le critère est prioritaire.
*/
std::vector<int> ConstraintSettings::buildDefaultImportanceCosts() const
{
    return Importance{}.costs;
}
