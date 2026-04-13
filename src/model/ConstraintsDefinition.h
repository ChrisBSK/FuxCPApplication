#pragma once

#include <juce_core/juce_core.h>
#include <map>

// ===============================

// ===============================
static juce::String fromUTF8(const char* text)
{
    return juce::String(juce::CharPointer_UTF8(text));
}

// ===============================
// STRUCTURE
// ===============================
struct ConstraintInfo
{
    juce::String name;
    juce::String description;
    juce::String theory;
    bool isHardConstraint;
};

// ===============================
// Contraintes
// ===============================
static std::map<juce::String, ConstraintInfo> constraintDB =
{
    {
        "MaxLeap",
        {
            "Max Leap",
            fromUTF8(u8"Limite la taille maximale des intervalles entre deux notes consécutives.\n\n"
                     u8"Une valeur faible produit une mélodie plus fluide.\n"
                     u8"Une valeur élevée autorise des sauts plus expressifs."),

            fromUTF8(u8"Règles Fux :\n"
                     u8"- éviter les grands sauts\n"
                     u8"- compenser par mouvement contraire"),

            true
        }
    },

    {
        "StepBias",
        {
            "Step Bias",
            fromUTF8(u8"Favorise les mouvements conjoints (notes voisines).\n\n"
                     u8"Augmenter cette valeur rend la mélodie plus fluide et chantante."),

            fromUTF8(u8"Préférence Fux :\n"
                     u8"- privilégier le mouvement conjoint"),

            false
        }
    },

    {
        "Repetition",
        {
            "Repetition",
            fromUTF8(u8"Autorise ou interdit la répétition de notes.\n\n"
                     u8"Les répétitions sont généralement évitées pour garder une ligne mélodique vivante."),

            fromUTF8(u8"Règle Fux implicite :\n"
                     u8"- éviter les répétitions"),

            true
        }
    },

    {
        "Direction",
        {
            "Direction",
            fromUTF8(u8"Contrôle la direction globale de la mélodie.\n\n"
                     u8"Permet d'éviter des lignes trop monotones (trop montantes ou descendantes)."),

            fromUTF8(u8"Règles Fux :\n"
                     u8"- équilibrer mouvements ascendants et descendants"),

            false
        }
    }
};