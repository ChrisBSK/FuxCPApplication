#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

/*
//==============================================================================
   ParameterHelpText

   Textes d'aide affichés au survol des labels de paramètres.

   Le solveur reste séparé de l'interface :
   - le label donne le nom visible du paramètre
   - ce fichier donne son explication utilisateur
   - juce::TooltipWindow affiche le texte au survol
//==============================================================================
*/
namespace ParameterHelpText
{
    /*
        Crée une chaîne JUCE à partir d'un texte UTF-8.

        On passe par cette méthode pour garder les accents français fiables,
        quel que soit l'encodage utilisé par le compilateur.
    */
    inline juce::String text(const char* utf8Text)
    {
        return juce::String::fromUTF8(utf8Text);
    }

    /*
        Retourne le texte d'aide associé à un label.

        Si aucun texte n'est défini pour ce label, on retourne une chaîne vide :
        JUCE n'affiche alors aucun tooltip.
    */
    inline juce::String getForLabel(const juce::String& label)
    {
        if (label == "Borrow Mode")
        {
            return text(
                "Contrainte liée : G4_counterpointMustBeInTheSameKey.\n\n"
                "Ce paramètre agit aussi sur le domaine de notes construit dans les parties FuxCP "
                "avec borrowMode.\n\n"
                "0 = le contrepoint reste dans le mode principal.\n\n"
                "1 = FuxCP peut proposer des notes empruntées, mais elles reçoivent un coût via "
                "borrowCost.\n\n"
                "Effet attendu : une mélodie parfois plus souple et moins bloquée, avec quelques "
                "couleurs modales supplémentaires si elles aident la solution."
            );
        }

        if (label == "Melody movement")
        {
            return text(
                "Contraintes liées : G7_melodicIntervalsShouldBeSmall, "
                "M1_1_2v_melodicIntervalsNotExceedMinorSixth, "
                "M1_1_3v_melodicIntervalsNotExceedMinorSixth et M1_2_octaveLeap.\n\n"
                "Le slider ne change pas les interdictions strictes de M1. "
                "Il modifie seulement les coûts mélodiques envoyés à G7.\n\n"
                "0 = quartes, quintes, sixtes, septièmes et octaves coûtent peu.\n\n"
                "1 = ces grands sauts coûtent davantage, surtout la quarte, la sixte et la "
                "septième.\n\n"
                "Effet attendu : plus la valeur monte, plus la mélodie devrait privilégier les "
                "mouvements conjoints et éviter les grands écarts audibles."
            );
        }

        if (label == "Avoid Repeated Notes")
        {
            return text(
                "Contrainte liée : M2_1_varietyCost.\n\n"
                "Ce paramètre ne modifie pas le code du solveur. "
                "Il pilote uniquement varietyCost, le coût déjà prévu par FuxCP pour favoriser "
                "des notes plus diverses dans chaque partie.\n\n"
                "0 = varietyCost vaut 0, donc cette pénalité est relâchée.\n\n"
                "1 = varietyCost reçoit une valeur forte, donc les répétitions repérées par FuxCP "
                "sont davantage pénalisées.\n\n"
                "Effet attendu : une ligne mélodique moins statique, avec moins de retours "
                "insistants sur les mêmes hauteurs lorsque les autres contraintes le permettent."
            );
        }

        if (label == "Avoid Octave Leaps")
        {
            return text(
                "Contraintes liées : M1_1_3v_melodicIntervalsNotExceedMinorSixth "
                "et M1_2_octaveLeap.\n\n"
                "En trois et quatre voix, FuxCP autorise les sauts d'octave comme exception "
                "aux limites normales des intervalles mélodiques.\n\n"
                "Ce paramètre ne supprime pas la contrainte dans le solveur. "
                "Il pilote uniquement octaveCost, le coût mélodique associé aux sauts de "
                "12 demi-tons.\n\n"
                "0 = octaveCost vaut 0, donc les sauts d'octave sont acceptés plus librement.\n\n"
                "1 = octaveCost reçoit une valeur forte, donc les sauts d'octave sont fortement "
                "pénalisés.\n\n"
                "Effet attendu : à 1, les contrepoints devraient éviter les grands bonds "
                "d'une octave lorsque d'autres solutions restent possibles."
            );
        }

        return {};
    }
}
