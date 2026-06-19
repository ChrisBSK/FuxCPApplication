#include "ConstraintsSettings.h"

#include <algorithm>
#include <cmath>

/*
==============================================================================
    ConstraintSettings.cpp

    Centralise les paramètres de pondération du solveur.

    Principe :
    - FuxCP possède des coûts par défaut.
    - Les contrôles de l'interface (sliders, boutons, combobox...)
      viennent modifier ces coûts.
    - Chaque paramètre UI possède sa propre méthode d'application.
==============================================================================
*/

//==============================================================================
// MELODIC COSTS
//==============================================================================

/*
    Construit le vecteur des coûts mélodiques utilisé par FuxCP.

    Étapes :
    1. Construire les coûts par défaut.
    2. Appliquer les paramètres UI.
*/
std::vector<int> ConstraintSettings::buildMelodicCosts(int cantusFirmusLength) const
{
    auto costs = buildDefaultMelodicCosts(cantusFirmusLength);

    applyLargeLeapPenalty(costs);

    return costs;
}

/*
    Coûts mélodiques de base, avant pénalisation par le slider.

    Plus le nombre est petit, plus le solveur accepte facilement l'intervalle.
    Le slider "Melodic Leaps" part de cette base, puis augmente les coûts des
    grands sauts quand largeLeapPenalty se rapproche de 1.
*/
std::vector<int> ConstraintSettings::buildDefaultMelodicCosts(int cantusFirmusLength) const
{
    constexpr int freeMove = 0;
    constexpr int preferredMove = 1;
    constexpr int acceptedLeap = 1;
    constexpr int lessPreferredLeap = 2;
    constexpr int forbiddenPenaltyPerNote = 64;
    constexpr int fallbackCantusFirmusLength = 9;

    const int melodyLength = cantusFirmusLength > 0
        ? cantusFirmusLength
        : fallbackCantusFirmusLength;

    const int almostForbidden = forbiddenPenaltyPerNote * melodyLength;

    return
    {
        freeMove,          // 0-2 demi-tons  : même note ou seconde ( 1-SecondeMineure/2-SecondeMajeure)
        preferredMove,     // 3-4 demi-tons  : tierce (3-TierceMineure/4-TierceMajeure)
        acceptedLeap,      // 5 demi-tons    : quarte (quarte juste)
        almostForbidden,   // 6 demi-tons    : triton (triton)
        acceptedLeap,      // 7 demi-tons    : quinte (juste)
        acceptedLeap,      // 8-9 demi-tons  : sixte (8-SixteMineure/9-SixteMajeure)
        lessPreferredLeap, // 10-11 demi-tons: septième (10-septièmeMineure/11-SetièmeMajeure)
        acceptedLeap       // 12 demi-tons   : octave (12 octave)
    };
}

/*
    Slider "Melodic Leaps".

    0.0 = comportement permissif : les grands sauts coûtent peu.
    1.0 = comportement strict : les grands sauts coûtent cher.

    Les intervalles concernés sont :

    - quarte
    - quinte
    - sixte
    - septième
    - octave

    Le triton garde son coût d'interdit/dernier recours, conformément au mémoire.
*/
void ConstraintSettings::applyLargeLeapPenalty(std::vector<int>& costs) const
{
    constexpr double minimumLeapPenalty = 0.0;
    constexpr double maximumLeapPenalty = 1.0;

    constexpr int lowCost = 1;
    constexpr int mediumCost = 2;
    constexpr int highCost = 4;
    constexpr int lastResortCost = 8;

    //Force la valeur de largeLeapPenalty à rester entre min et max
    const double largeLeapPenalty = std::clamp(
        melodic.largeLeapPenalty,
        minimumLeapPenalty,
        maximumLeapPenalty
    );

    //Formule d'une interpolation linéraire
    // permissiveCost = coût de départ (slider égal à 0)
    // strictCost - permissiveCost = distance entre le coût fort et le coût faible
    //                               défini dans (buildDefaultMelodicCosts)
    // largeLeapPenalty = position du slider
    const auto interpolateCost = [largeLeapPenalty](int permissiveCost, int strictCost)
    {
        const double cost = permissiveCost
            + largeLeapPenalty * static_cast<double>(strictCost - permissiveCost);

        return static_cast<int>(std::lround(cost));
    };

    // Dans la tthéorie de Fux on dit que les saut de fifth et octaves sont tolérés
    // Le reste ne sonne pas forcément bien, du coup je pénalise les autres grands sauts
    // avec lastResortCost
    costs[fourthCost]  = interpolateCost(lowCost, lastResortCost);
    costs[fifthCost]   = interpolateCost(lowCost, highCost);
    costs[sixthCost]   = interpolateCost(lowCost, lastResortCost);
    costs[seventhCost] = interpolateCost(mediumCost, lastResortCost);
    costs[octaveCost]  = interpolateCost(lowCost, highCost);
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
    return
    {
        4,                           // borrowCost
        1,                           // h_fifthCost
        1,                           // h_octaveCost
        2,                           // succCost

        general.noteRepetitionValue, // varietyCost
                                     // Utilisé dans M2_1_varietyCost().
                                     // Pénalise les notes répétées dans une fenêtre courte.

        2,                           // triadCost
        8,                           // directMoveCost
        1                            // penultCost
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
    auto costs = buildDefaultSpecificCosts();

    return costs;
}

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
    return
    {
        8,  // borrow
        7,  // fifth
        5,  // octave
        3,  // succ
        9,  // variety
        4,  // triad
        14, // direct
        12, // motion
        6,  // penult
        11, // cambiata
        5,  // triad3
        10, // m2
        13, // syncopation
        1   // melodic
    };
}
