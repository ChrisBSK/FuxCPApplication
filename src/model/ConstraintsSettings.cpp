#include "ConstraintsSettings.h"

#include <algorithm>
#include <cmath>

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
    constexpr double minimumSliderValue = 0.0;
    constexpr double maximumSliderValue = 1.0;

    /*
        Garde une valeur de slider dans sa plage normale.
    */
    double clampSlider(double value)
    {
        return std::clamp(value, minimumSliderValue, maximumSliderValue);
    }

    /*
        Calcule une valeur progressive entre deux coûts.

        slider = 0.0 -> coût permissif
        slider = 1.0 -> coût strict
    */
    int interpolateCost(double sliderValue, int permissiveCost, int strictCost)
    {
        const double sliderPosition = clampSlider(sliderValue);

        const double cost = permissiveCost
            + sliderPosition * static_cast<double>(strictCost - permissiveCost);

        return static_cast<int>(std::lround(cost));
    }

    /*
        Calcule un coût très élevé pour le triton.
        Il augmente avec la longueur du Cantus Firmus pour rester dissuasif.
    */
    int buildTritoneCost(int cantusFirmusLength)
    {
        constexpr int penaltyPerNote = 64;
        constexpr int minimumLength = 1;

        return penaltyPerNote * std::max(cantusFirmusLength, minimumLength);
    }
}

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
    auto melodicCosts = buildDefaultMelodicCosts(cantusFirmusLength);

    applyLargeLeapPenalty(melodicCosts);

    return melodicCosts;
}

/*
    Coûts mélodiques de base, avant pénalisation par le slider.

    Plus le nombre est petit, plus le solveur accepte facilement l'intervalle.
    Le slider "Melodic Leaps" part de cette base, puis augmente les coûts des
    grands sauts quand largeLeapPenalty se rapproche de 1.
*/
std::vector<int> ConstraintSettings::buildDefaultMelodicCosts(int cantusFirmusLength) const
{
    constexpr int freeCost = 0;
    constexpr int lowCost = 1;
    constexpr int mediumCost = 2;

    return
    {
        freeCost,                            // Même note ou seconde
        lowCost,                             // Tierce
        lowCost,                             // Quarte
        buildTritoneCost(cantusFirmusLength), // Triton
        lowCost,                             // Quinte
        lowCost,                             // Sixte
        mediumCost,                          // Septième
        lowCost                              // Octave
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
    constexpr int lowCost = 1;
    constexpr int mediumCost = 2;
    constexpr int highCost = 4;
    constexpr int veryHighCost = 8;

    costs[fourthCost]  = interpolateCost(melodic.largeLeapPenalty, lowCost, veryHighCost);
    costs[fifthCost]   = interpolateCost(melodic.largeLeapPenalty, lowCost, highCost);
    costs[sixthCost]   = interpolateCost(melodic.largeLeapPenalty, lowCost, veryHighCost);
    costs[seventhCost] = interpolateCost(melodic.largeLeapPenalty, mediumCost, veryHighCost);
    costs[octaveCost]  = interpolateCost(melodic.largeLeapPenalty, lowCost, highCost);
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

    applyNoteRepetitionPenalty(generalCosts);

    return generalCosts;
}

/*
    Coûts généraux de base, avant action des sliders.

    Ils correspondent à g_costs dans FuxCP.
*/
std::vector<int> ConstraintSettings::buildDefaultGeneralCosts() const
{
    constexpr int lowCost = 1;
    constexpr int mediumCost = 2;
    constexpr int highCost = 4;
    constexpr int veryHighCost = 8;

    return
    {
        highCost,      // borrowCost
        lowCost,       // h_fifthCost
        lowCost,       // h_octaveCost
        mediumCost,    // succCost
        mediumCost,    // varietyCost
        mediumCost,    // triadCost
        veryHighCost,  // directMoveCost
        lowCost        // penultCost
    };
}

/*
    Slider "Melodic Variety".

    Modifie varietyCost, utilisé par FuxCP pour limiter les répétitions
    dans une fenêtre mélodique courte.
*/
void ConstraintSettings::applyNoteRepetitionPenalty(std::vector<int>& costs) const
{
    costs[varietyCost] = general.noteRepetitionValue;
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
    return buildDefaultImportanceCosts();
}

/*
    Ordre d'importance des critères pendant l'optimisation.

    Plus le nombre est petit, plus le critère est prioritaire.
*/
std::vector<int> ConstraintSettings::buildDefaultImportanceCosts() const
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
