#include "ConstraintsSettings.h"

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
std::vector<int> ConstraintSettings::buildMelodicCosts() const
{
    auto costs = buildDefaultMelodicCosts();

    return costs;
}

/*
    Coûts mélodiques par défaut de FuxCP.

    Correspondance avec Part.cpp :

    secondCost   = m_costs[0];
    thirdCost    = m_costs[1];
    fourthCost   = m_costs[2];
    tritoneCost  = m_costs[3];
    fifthCost    = m_costs[4];
    sixthCost    = m_costs[5];
    seventhCost  = m_costs[6];
    octaveCost   = m_costs[7];
*/
std::vector<int> ConstraintSettings::buildDefaultMelodicCosts() const
{
    return
    {
        0,   // secondCost   : seconde mélodique
        1,   // thirdCost    : tierce mélodique
        1,   // fourthCost   : quarte mélodique
        576, // tritoneCost  : triton mélodique
        2,   // fifthCost    : quinte mélodique
        2,   // sixthCost    : sixte mélodique
        2,   // seventhCost  : septième mélodique
        1    // octaveCost   : octave mélodique
    };
}

/*
    Slider "Leap".

    Plus la valeur est élevée,
    plus les grands sauts mélodiques sont pénalisés.

    Les intervalles concernés sont :

    - quarte
    - triton
    - quinte
    - sixte
    - septième
    - octave
*/
void ConstraintSettings::applyLeapSlider(std::vector<int>& costs) const
{
    const int value = melodic.leapSliderValue;

    if (value <= 0)
        return;

    costs[fourthCost]  += value;
    costs[tritoneCost] += value;
    costs[fifthCost]   += value;
    costs[sixthCost]   += value;
    costs[seventhCost] += value;
    costs[octaveCost]  += value;
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
        8,  // borrow       : importance des notes empruntées
        7,  // fifth        : importance des quintes harmoniques
        5,  // octave       : importance des octaves harmoniques
        2,  // succ         : importance des mouvements successifs
        9,  // variety      : importance de la variété mélodique
        3,  // triad        : importance des accords complets
        14, // direct       : importance des mouvements directs
        12, // motion       : importance générale des mouvements
        6,  // penult       : importance de la pénultième mesure
        11, // cambiata     : importance des cambiatas
        4,  // triad3       : importance de la tierce de l'accord
        10, // m2           : importance des secondes mélodiques
        1,  // syncopation  : importance des syncopes
        13  // melodic      : importance globale des contraintes mélodiques
    };
}