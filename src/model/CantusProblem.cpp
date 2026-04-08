#include "CantusProblem.h"

// =========================
// Constructeurs
// =========================

CantusProblem::CantusProblem() = default;

CantusProblem::CantusProblem(const juce::String& name)
    : title(name)
{
}

// =========================
// Cantus firmus
// =========================

void CantusProblem::setCantusFirmus(const std::vector<int>& cf)
{
    // On remplace simplement le cantus existant
    // Le cantus est une suite de notes MIDI (ex: {60, 62, 64, ...})
    cantusFirmus = cf;
}

const std::vector<int>& CantusProblem::getCantusFirmus() const
{
    // Retourne une référence constante pour éviter une copie inutile
    return cantusFirmus;
}

// =========================
// Voix
// =========================

void CantusProblem::setVoices(const std::vector<Voice>& newVoices)
{
    // On remplace toutes les voix existantes
    // Chaque Voice contient :
    // - id : identifiant unique
    // - type : position (au-dessus / en dessous)
    // - species : type de contrepoint
    voices = newVoices;
}

const std::vector<CantusProblem::Voice>& CantusProblem::getVoices() const
{
    // Retour direct des voix (lecture seule)
    return voices;
}

size_t CantusProblem::getVoiceCount() const
{
    // Nombre de voix dans le problème
    return voices.size();
}

// =========================
// Conversion pour le solveur
// =========================

std::vector<int> CantusProblem::getSpeciesList() const
{
    // Le solveur Fux attend un vector<int> avec les espèces
    // Exemple : {FIRST_SPECIES, SECOND_SPECIES}

    std::vector<int> result;
    result.reserve(voices.size());

    for (const auto& v : voices)
    {
        result.push_back(v.species);
    }

    return result;
}

std::vector<int> CantusProblem::getVoiceTypes() const
{
    // Le solveur Fux attend un vector<int> avec les positions des voix
    // Exemple : {2, -1} → une voix au-dessus, une en dessous

    std::vector<int> result;
    result.reserve(voices.size());

    for (const auto& v : voices)
    {
        result.push_back(v.type);
    }

    return result;
}

// =========================
// Modification par voix (UI)
// =========================

void CantusProblem::setSpecies(int voiceIndex, int species) // Définit l'espèce de la voix à l'index voiceIndex
{
    // On modifie l'espèce d'une voix donnée

    if (voiceIndex < 0 || voiceIndex >= static_cast<int>(voices.size()))
        return; // index invalide → on ignore

    voices[voiceIndex].species = species;
}

int CantusProblem::getSpecies(int voiceIndex) const // Retounre un entier qui est l'espèce de la voix à l'index voiceIndex
{
    // Retourne l'espèce d'une voix

    if (voiceIndex < 0 || voiceIndex >= static_cast<int>(voices.size()))
        return -1; // valeur invalide

    return voices[voiceIndex].species;
}

void CantusProblem::setVoiceType(int voiceIndex, int type)
{
    // Modifie la position de la voix (au-dessus / en dessous)

    if (voiceIndex < 0 || voiceIndex >= static_cast<int>(voices.size()))
        return;

    voices[voiceIndex].type = type;
}

int CantusProblem::getVoiceType(int voiceIndex) const
{
    // Retourne le type d'une voix

    if (voiceIndex < 0 || voiceIndex >= static_cast<int>(voices.size()))
        return -1;

    return voices[voiceIndex].type;
}

// =========================
// Paramètres du solveur
// =========================

void CantusProblem::setCostParameters(const CostParameters& params)
{
    // Stocke les paramètres de coût utilisés par le solveur
    // Ces paramètres influencent la qualité des solutions générées
    costs = params;
}

const CantusProblem::CostParameters& CantusProblem::getCostParameters() const
{
    // Retourne les paramètres du solveur
    return costs;
}

void CantusProblem::setBorrowMode(int mode)
{
    // Définit le mode d'emprunt modal (paramètre du solveur)
    // Exemple :
    // 0 → strict
    // 1 → autorise certaines altérations
    borrowMode = mode;
}

int CantusProblem::getBorrowMode() const
{
    return borrowMode;
}

// =========================
// Infos
// =========================

void CantusProblem::setTitle(const juce::String& newTitle)
{
    // Définit le nom du problème
    title = newTitle;
}

juce::String CantusProblem::getTitle() const
{
    return title;
}

bool CantusProblem::isEmpty() const
{
    // Un problème est considéré vide si :
    // - pas de cantus
    // OU
    // - pas de voix

    return cantusFirmus.empty() || voices.empty();
}

