#include "ConstraintsSettings.h"

/*std::vector<int> ConstraintSettings::buildMelodicCosts() const
{
    return {0, 1, 1, 576, 2, 2, 2, 1};
}*/
std::vector<int> ConstraintSettings::buildMelodicCosts() const {
    std::vector<int> melodic_params = {0, 1, 1, 576, 2, 2, 2, 1};
    melodic_params[6] = std::min(leapPenalty / 10, 100);  // Sixte
    melodic_params[7] = std::min(leapPenalty / 5, 100);   // Septième
    return melodic_params;
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