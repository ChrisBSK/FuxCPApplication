#pragma once

#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>

#include "ConstraintsSettings.h"

class CantusProblem {

public:
    CantusProblem();
    explicit CantusProblem(const juce::String& name);

    // =========================
    // Cantus firmus
    // =========================
    void setCantusFirmus(const std::vector<int>& cf);
    const std::vector<int>& getCantusFirmus() const;

    // =========================
    //  Voix
    // =========================
    struct Voice
    {
        int id;
        int type;     // position (2, 1, 0, -1, etc.)
        int species;  // FIRST_SPECIES, etc.
    };

    /** Définit toutes les voix */
    void setVoices(const std::vector<Voice>& newVoices);
    /** Retourne toutes les voix */
    const std::vector<Voice>& getVoices() const;

    /** Nombre de voix */
    size_t getVoiceCount() const;

    // =========================
    //  Accès pour le solveur
    // =========================

    /** Convertit les voix → species list (pour create_problem) */
    std::vector<int> getSpeciesList() const; // { 1, 2} --> {FIRST_SPECIES, SECOND_SPECIES}

    /** Convertit les voix → voice types (pour create_problem) */
    std::vector<int> getVoiceTypes() const;  // {2, -1} → une voix au-dessus et une voix en dessous du cantus


    // =========================
    //  Modification par voix (UI)
    // =========================

    void setSpecies(int voiceIndex, int species); //Espèce par voix: ex -> Voix 1 - Espèce 4 (splist)
    int getSpecies(int voiceIndex) const;

    void setVoiceType(int voiceIndex, int type); //Type par voix: ex -> Voix 2 - Type -1(v_type)
    int getVoiceType(int voiceIndex) const;

    // =========================
    //  Paramètres du solveur
    // =========================




    void setCostParameters(const CostParameters& params);
    const CostParameters& getCostParameters() const;

    void setBorrowMode(int mode);
    int getBorrowMode() const;

    // =========================
    // Infos
    // =========================

    void setTitle(const juce::String& newTitle);
    juce::String getTitle() const;

    bool isEmpty() const;


    //============ Contraintes ============
    //Colonne 1: Melodic Constraints
    ConstraintSettings& getSettings();

    const ConstraintSettings& getSettings() const;

    void setSettings(const ConstraintSettings& s);





private:
    // Données musicales
    std::vector<int> cantusFirmus;
    std::vector<Voice> voices;

    ConstraintSettings settings;


    // Métadonnées
    juce::String title;





};

