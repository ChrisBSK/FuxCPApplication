#pragma once

#include <vector>
#include <juce_core/juce_core.h>
#include "ConstraintsSettings.h"

class CantusProblem
{
public:
    // =========================
    // Structures de données
    // =========================

    struct Counterpoint
    {
        int type;     // position relative au CF (ex: -1, 1, 2...)
        int species;  // espèce (1 → 5)
    };

    struct Voices
    {
        std::vector<int> cf;                       // Cantus Firmus
        std::vector<Counterpoint> counterpoints;   // Voix générées
    };

    // =========================
    // Données musicales
    // =========================

    void setVoices(const Voices& v);
    const Voices& getVoices() const;

    const std::vector<int>& getCantusFirmus() const;
    const std::vector<Counterpoint>& getCounterpoints() const;

    size_t getCounterpointCount() const;

    void setVoiceCount(int count);
    int getVoiceCount() const;

    // =========================
    // Conversion pour le solver
    // =========================

    std::vector<int> getSpeciesList() const; // CP uniquement
    std::vector<int> getVoiceTypes() const;  // CP uniquement

    // =========================
    // Paramètres du solveur
    // =========================
    void setSettings(const ConstraintSettings& s);
    ConstraintSettings& getSettings();
    const ConstraintSettings& getSettings() const;


    // =========================
    // Recalcul des coûts
    // =========================
    void recalculateCosts();

    const std::vector<int> &getMelodicCosts() const;

    const std::vector<int> &getGeneralCosts() const;

    const std::vector<int> &getSpecificCosts() const;

    const std::vector<int> &getImportanceCosts() const;

    // =========================
    // Métadonnées
    // =========================

    void setTitle(const juce::String& newTitle);
    juce::String getTitle() const;

    // =========================
    // Validation
    // =========================

    bool isEmpty() const;




private:
    Voices voices;                 // Données musicales
    ConstraintSettings settings;  // Paramètres solveur
    juce::String title;            // Nom du problème

    int voiceCount = 0;

    std::vector<int> melodicCosts;
    std::vector<int> generalCosts;
    std::vector<int> specificCosts;
    std::vector<int> importanceCosts;


};