#pragma once

#include <vector>
#include <string>
#include <stdexcept>
#include <sstream>

/*
===============================================================================
CantusFirmus.h
-------------------------------------------------------------------------------
Objet métier représentant un Cantus Firmus.

IMPORTANT :
- Cette classe appartient à la couche MODEL.
- Elle ne dépend PAS de FuxCP, Gecode, JUCE ou du service.
- Elle représente uniquement les données musicales manipulées par l'utilisateur.

Responsabilités :
- stocker les notes MIDI du cantus firmus
- garantir leur validité minimale
- fournir un accès simple aux notes
===============================================================================
*/

class CantusFirmus
{
public:

    /*
    ---------------------------------------------------------------------------
    Constructeur vide
    Utile si on crée l'objet puis qu'on lui affecte les notes plus tard.
    ---------------------------------------------------------------------------
    */
    CantusFirmus() = default;


    /*
    ---------------------------------------------------------------------------
    Constructeur principal
    Prend directement un vecteur de notes MIDI.

    Exemple :
        CantusFirmus cf({60, 62, 64, 65, 67});
    ---------------------------------------------------------------------------
    */
    explicit CantusFirmus(const std::vector<int>& midiNotes)
    {
        setNotes(midiNotes);
    }


    /*
    ---------------------------------------------------------------------------
    Définit les notes du cantus firmus.

    Vérifications :
    - non vide
    - valeurs MIDI entre 0 et 127

    Lance std::invalid_argument si erreur.
    ---------------------------------------------------------------------------
    */
    void setNotes(const std::vector<int>& midiNotes)
    {
        if (midiNotes.empty())
            throw std::invalid_argument("CantusFirmus: liste de notes vide");

        for (int n : midiNotes)
        {
            if (n < 0 || n > 127)
                throw std::invalid_argument("CantusFirmus: note MIDI hors limite (0-127)");
        }

        notes = midiNotes;
    }


    /*
    ---------------------------------------------------------------------------
    Retourne les notes MIDI.
    ---------------------------------------------------------------------------
    */
    const std::vector<int>& getNotes() const noexcept
    {
        return notes;
    }


    /*
    ---------------------------------------------------------------------------
    Nombre de notes.
    ---------------------------------------------------------------------------
    */
    size_t size() const noexcept
    {
        return notes.size();
    }


    /*
    ---------------------------------------------------------------------------
    Accès direct à une note (lecture seule).
    ---------------------------------------------------------------------------
    */
    int operator[](size_t index) const
    {
        return notes.at(index);
    }


    /*
    ---------------------------------------------------------------------------
    Vérifie si l'objet contient un CF valide.

    (utile si créé vide puis rempli plus tard)
    ---------------------------------------------------------------------------
    */
    bool isValid() const noexcept
    {
        return !notes.empty();
    }


    /*
    ---------------------------------------------------------------------------
    Conversion en string pour debug / affichage simple.
    Exemple : "60 62 64 65 67"
    ---------------------------------------------------------------------------
    */
    std::string toString() const
    {
        std::ostringstream oss;

        for (size_t i = 0; i < notes.size(); ++i)
        {
            oss << notes[i];
            if (i + 1 < notes.size())
                oss << " ";
        }

        return oss.str();
    }

private:

    /*
    ---------------------------------------------------------------------------
    Notes MIDI du cantus firmus.
    ---------------------------------------------------------------------------
    */
    std::vector<int> notes;
};
