#include "MelodicParameters.h"

std::vector<int> MelodicParameters::makeSmoothnessCosts (int smoothness)
{
    smoothness = std::clamp (smoothness, 0, 100);

    return
    {
        0,                  // second
        0,                  // third
        smoothness * 1,     // fourth
        smoothness * 2,     // tritone
        smoothness * 3,     // fifth
        smoothness * 4,     // sixth
        smoothness * 5,     // seventh
        smoothness * 6      // octave
    };
}