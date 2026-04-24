#pragma once
#include "ConstraintsSettings.h"

struct ConstraintApplier
{
    static CostParameters apply(const ConstraintSettings& settings)
    {
        CostParameters costs;

        // Base par défaut (IMPORTANT)
        costs.melodic   = {0, 1, 1, 1, 2, 2, 2, 1};
        costs.general   = {4, 1, 1, 2, 2, 2, 8, 1};
        costs.specific  = {8, 4, 0, 2, 1, 8, 50};
        costs.importance= {8,7,5,2,9,3,14,12,6,11,4,10,1,13};

        applyMaxLeap(settings.rules, costs);

        return costs;
    }

private:
    static void applyMaxLeap(const RuleParameters& rules, CostParameters& costs)
    {
        int maxLeap = rules.maxLeap;

        for (int i = 0; i < costs.melodic.size(); ++i)
        {
            int intervalSize = i + 2; // approx mapping

            if (intervalSize > maxLeap)
            {
                costs.melodic[i] += (intervalSize - maxLeap) * 5;
            }
        }
    }
};