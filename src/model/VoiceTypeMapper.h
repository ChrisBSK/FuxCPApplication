#pragma once
#include <vector>
#include <cstdlib>

namespace VoiceTypeMapper
{
    enum SimpleType
    {
        Lower = 1,
        Same = 2,
        Higher = 3
    };

    // UI → candidats backend
    inline std::vector<int> getCandidates(int selectedId)
    {
        switch (selectedId)
        {
            case Lower:  return {-1, -2, -3};
            case Same:   return {0};
            case Higher: return {1, 2};
            default:     return {0};
        }
    }

    // choix final envoyé au solveur
    inline int pickBest(const std::vector<int>& candidates)
    {
        if (candidates.empty())
            return 0;

        // version simple (stable)
        return candidates[std::rand() % candidates.size()];
    }


    inline int mapToBackend(int selectedId)
    {
        return pickBest(getCandidates(selectedId));
    }


    inline int ensureValidType(int type)
    {
        if (type < -3 || type > 2)
            return 0;
        return type;
    }
}