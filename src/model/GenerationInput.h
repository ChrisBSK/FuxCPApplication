#pragma once

#include <vector>

// 🔹 Structure simple qui regroupe toutes les entrées utilisateur
struct GenerationInput
{
    std::vector<int> cantusFirmus;
    int numberOfVoices;
    int species;
};