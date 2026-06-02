#include "ConstraintsSettings.h"

/*
//==============================================================================
   ConstraintSettings

   Centralise les paramètres de pondération du solveur.
   Chaque méthode construit un vecteur de coûts utilisé
   lors de la création du problème Fux/Gecode.
//==============================================================================
*/

//==============================================================================
// Construction des coûts mélodiques
// Pondérations liées aux mouvements mélodiques du contrepoint
//==============================================================================

/*
    Construit le vecteur des coûts mélodiques utilisé par le solveur.
*/
std::vector<int> ConstraintSettings::buildMelodicCosts() const {
    std::vector<int> melodic_params = {0, 1, 1, 576, 2, 2, 2, 1};
    /*melodic_params[6] = std::min(leapPenalty / 10, 100);  // Sixte
    melodic_params[7] = std::min(leapPenalty / 5, 100);   // Septième*/
    return melodic_params;
}


//==============================================================================
// Construction des coûts généraux
// Contraintes communes à toutes les espèces
//==============================================================================

/*
    Construit le vecteur des coûts généraux.
*/
std::vector<int> ConstraintSettings::buildGeneralCosts() const
{
    return {4, 1, 1, 2, 2, 2, 8, 1};
}

//==============================================================================
// Construction des coûts spécifiques
// Contraintes propres aux espèces de contrepoint
//==============================================================================

/*
    Construit le vecteur des coûts spécifiques.
*/
std::vector<int> ConstraintSettings::buildSpecificCosts() const
{
    return {8, 4, 0, 2, 1, 8, 50};
}

//==============================================================================
// Priorités d'optimisation
// Ordre lexicographique utilisé par le solveur
//==============================================================================

/*
    Construit le vecteur des niveaux d'importance
    utilisés lors de l'optimisation.
*/
std::vector<int> ConstraintSettings::buildImportanceCosts() const
{
    return {8,7,5,2,9,3,14,12,6,11,4,10,1,13};
}
std::vector<int> ConstraintSettings::buildMelodicCosts() const {
    return {0, 1, 1, 576, 2, 2, 2, 1};

}


std::vector<int> ConstraintSettings::buildGeneralCosts() const
{
    return {4, 1, 1, 2, 2, 2, 8, 1};
}

std::vector<int> ConstraintSettings::buildSpecificCosts() const
{
    return {8, 4, 0, 2, 1, 8, 50};
}

std::vector<int> ConstraintSettings::buildImportanceCosts() const
{
    return {8,7,5,2,9,3,14,12,6,11,4,10,1,13};
}