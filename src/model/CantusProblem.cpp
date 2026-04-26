#include "CantusProblem.h"

// =========================
// Données musicales (Voices)
// =========================

void CantusProblem::setVoices(const Voices& v)
{
    voices = v;
}

const CantusProblem::Voices& CantusProblem::getVoices() const
{
    return voices;
}

const std::vector<int>& CantusProblem::getCantusFirmus() const
{
    return voices.cf;
}

const std::vector<CantusProblem::Counterpoint>& CantusProblem::getCounterpoints() const
{
    return voices.counterpoints;
}

size_t CantusProblem::getCounterpointCount() const
{
    return voices.counterpoints.size();
}

void CantusProblem::setVoiceCount(int count)
{
    voiceCount = count;
}

int CantusProblem::getVoiceCount() const
{
    return voiceCount;
}

// =========================
// Conversion pour le solver
// =========================

std::vector<int> CantusProblem::getSpeciesList() const
{
    std::vector<int> result;
    result.reserve(voices.counterpoints.size());

    for (const auto& cp : voices.counterpoints)
        result.push_back(cp.species);

    return result;
}

std::vector<int> CantusProblem::getVoiceTypes() const
{
    std::vector<int> result;
    result.reserve(voices.counterpoints.size());

    for (const auto& cp : voices.counterpoints)
        result.push_back(cp.type);

    return result;
}

// =========================
// Paramètres du solveur
// =========================
void CantusProblem::setSettings(const ConstraintSettings& s)
{
    constraintSettings = s;
}

ConstraintSettings& CantusProblem::getSettings()
{
    return constraintSettings;
}

const ConstraintSettings& CantusProblem::getSettings() const
{
    return constraintSettings;
}

// =========================
// Métadonnées
// =========================

void CantusProblem::setTitle(const juce::String& newTitle)
{
    title = newTitle;
}

juce::String CantusProblem::getTitle() const
{
    return title;
}

// =========================
// Validation
// =========================

bool CantusProblem::isEmpty() const
{
    return voices.cf.empty() || voices.counterpoints.empty();
}