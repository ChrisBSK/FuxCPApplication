#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>

/*
===============================================================================
CounterpointSolution.h
-------------------------------------------------------------------------------
Objet métier représentant une solution de contrepoint générée par le solveur.

IMPORTANT :
- Cette classe appartient à la couche MODEL.
- Elle ne dépend PAS de FuxCP, Gecode ou JUCE.
- Elle représente uniquement le résultat "métier" que l'UI doit afficher.

Contenu minimal :
- les voix sous forme de notes MIDI
- un accès simple aux données
- une conversion en string pour debug/affichage
===============================================================================
*/

class CounterpointSolution
{
public:

    /*
    ---------------------------------------------------------------------------
    Constructeur vide
    ---------------------------------------------------------------------------
    */
    CounterpointSolution() = default;

    /*
    ---------------------------------------------------------------------------
    Constructeur principal

    voicesMidiNotes :
        - chaque élément = une voix
        - chaque voix = liste de notes MIDI (0..127)

    Exemple :
        CounterpointSolution sol({
            {60, 62, 64, 65},  // voix 1
            {48, 50, 52, 53}   // voix 2
        });
    ---------------------------------------------------------------------------
    */
    explicit CounterpointSolution(const std::vector<std::vector<int>>& voicesMidiNotes)
    {
        setVoices(voicesMidiNotes);
    }

    /*
    ---------------------------------------------------------------------------
    Définit toutes les voix de la solution.

    Vérifications :
    - au moins une voix
    - aucune voix vide
    - notes MIDI dans [0..127]
    ---------------------------------------------------------------------------
    */
    void setVoices(const std::vector<std::vector<int>>& voicesMidiNotes)
    {
        if (voicesMidiNotes.empty())
            throw std::invalid_argument("CounterpointSolution: aucune voix fournie");

        for (const auto& voice : voicesMidiNotes)
        {
            if (voice.empty())
                throw std::invalid_argument("CounterpointSolution: une voix est vide");

            for (int n : voice)
            {
                if (n < 0 || n > 127)
                    throw std::invalid_argument("CounterpointSolution: note MIDI hors limite (0-127)");
            }
        }

        voices = voicesMidiNotes;
    }

    /*
    ---------------------------------------------------------------------------
    Accès aux voix (lecture seule).
    ---------------------------------------------------------------------------
    */
    const std::vector<std::vector<int>>& getVoices() const noexcept
    {
        return voices;
    }

    /*
    ---------------------------------------------------------------------------
    Nombre de voix dans la solution.
    ---------------------------------------------------------------------------
    */
    size_t getVoiceCount() const noexcept
    {
        return voices.size();
    }

    /*
    ---------------------------------------------------------------------------
    Accès direct à une voix (lecture seule).
    ---------------------------------------------------------------------------
    */
    const std::vector<int>& getVoice(size_t voiceIndex) const
    {
        return voices.at(voiceIndex);
    }

    /*
    ---------------------------------------------------------------------------
    Taille (nombre de notes) d'une voix donnée.
    ---------------------------------------------------------------------------
    */
    size_t getVoiceLength(size_t voiceIndex) const
    {
        return voices.at(voiceIndex).size();
    }

    /*
    ---------------------------------------------------------------------------
    Vérifie si la solution est exploitable.
    (utile si créé vide puis rempli plus tard)
    ---------------------------------------------------------------------------
    */
    bool isValid() const noexcept
    {
        if (voices.empty())
            return false;

        for (const auto& v : voices)
            if (v.empty())
                return false;

        return true;
    }

    /*
    ---------------------------------------------------------------------------
    Format texte simple pour debug.

    Exemple :
        Voice 0: 60 62 64 65
        Voice 1: 48 50 52 53
    ---------------------------------------------------------------------------
    */
    std::string toString() const
    {
        std::ostringstream oss;

        for (size_t v = 0; v < voices.size(); ++v)
        {
            oss << "Voice " << v << ": ";
            for (size_t i = 0; i < voices[v].size(); ++i)
            {
                oss << voices[v][i];
                if (i + 1 < voices[v].size())
                    oss << " ";
            }
            if (v + 1 < voices.size())
                oss << "\n";
        }

        return oss.str();
    }

private:

    /*
    ---------------------------------------------------------------------------
    voices[voiceIndex][noteIndex] = note MIDI
    ---------------------------------------------------------------------------
    */
    std::vector<std::vector<int>> voices;
};
