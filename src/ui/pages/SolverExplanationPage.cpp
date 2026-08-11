#include "SolverExplanationPage.h"

namespace
{
    /*
        Texte affiché dans la page.

        Il est gardé ici, dans le fichier de la page, pour éviter de surcharger
        MainComponent avec du contenu pédagogique.
    */
    juce::String buildSolverExplanationText()
    {
        return juce::String::fromUTF8(
            "1. Les notes ne sont pas choisies directement\n"
            "\n"
            "Quand on lance Generate, FuxCP ne crée pas tout de suite une mélodie finale.\n"
            "Il crée d'abord des variables de notes :\n"
            "\n"
            "CP1[0], CP1[1], CP1[2], ...\n"
            "\n"
            "Chaque variable contient plusieurs notes possibles.\n"
            "Exemple : CP1[0] peut être 60, 64, 67.\n"
            "\n"
            "\n"
            "2. Les règles de Fux retirent les configurations impossibles\n"
            "\n"
            "Les règles strictes ne retirent pas seulement des notes isolées.\n"
            "Elles retirent surtout des combinaisons impossibles :\n"
            "\n"
            "- mauvais intervalle avec le Cantus Firmus,\n"
            "- mouvement interdit,\n"
            "- parallèle interdit,\n"
            "- fin incorrecte,\n"
            "- contrainte d'espèce non respectée.\n"
            "\n"
            "Donc le solveur garde seulement les configurations musicales autorisées.\n"
            "\n"
            "\n"
            "3. Chaque solution possible reçoit un vecteur de coûts\n"
            "\n"
            "Une solution complète produit plusieurs coûts.\n"
            "Dans le code, le vecteur brut s'appelle unitedCosts.\n"
            "\n"
            "unitedCosts contient les 14 familles de coûts :\n"
            "\n"
            "[borrow, fifth, octave, succ, variety, triad, direct, motion,\n"
            " penult, cambiata, triad3, m2, syncopation, melodic]\n"
            "\n"
            "Exemple pour une solution :\n"
            "\n"
            "unitedCosts = [0, 2, 1, 4, 3, 0, 1, 2, 0, 0, 1, 0, 0, 7]\n"
            "\n"
            "Cela veut dire par exemple :\n"
            "- fifth = 2,\n"
            "- octave = 1,\n"
            "- succ = 4,\n"
            "- melodic = 7.\n"
            "\n"
            "\n"
            "4. Les sliders changent le prix des événements\n"
            "\n"
            "Les sliders ne disent pas quelle note choisir.\n"
            "Ils changent combien coûte un événement musical.\n"
            "\n"
            "Exemple : Melody moves pilote steps1(s).\n"
            "Si le slider favorise les mouvements conjoints, les grands sauts coûtent plus cher.\n"
            "\n"
            "Ensuite, si une solution fait réellement des sauts, ces pénalités sont ajoutées\n"
            "dans la famille melodic de unitedCosts.\n"
            "\n"
            "\n"
            "5. Le vecteur importance donne l'ordre des priorités\n"
            "\n"
            "Le vecteur importance indique quels coûts sont les plus importants.\n"
            "\n"
            "Exemple :\n"
            "\n"
            "syncopation priorité 1\n"
            "succ priorité 2\n"
            "melodic priorité 3\n"
            "fifth priorité 4\n"
            "\n"
            "FuxCP utilise cet ordre pour construire finalCosts.\n"
            "\n"
            "\n"
            "6. finalCosts est le vecteur réellement minimisé\n"
            "\n"
            "finalCosts est unitedCosts réorganisé selon importance.\n"
            "\n"
            "Exemple :\n"
            "\n"
            "unitedCosts = [borrow, fifth, octave, succ, melodic]\n"
            "importance  = succ d'abord, puis melodic, puis fifth, puis octave, puis borrow\n"
            "\n"
            "finalCosts = [succ, melodic, fifth, octave, borrow]\n"
            "\n"
            "Si deux coûts ont la même priorité, ils sont additionnés.\n"
            "\n"
            "Exemple :\n"
            "\n"
            "priorité 2 : fifth + octave\n"
            "\n"
            "Alors finalCosts contient :\n"
            "\n"
            "[succ, fifth + octave, melodic, ...]\n"
            "\n"
            "\n"
            "7. BAB cherche la meilleure solution\n"
            "\n"
            "BAB trouve d'abord une solution valide.\n"
            "Puis il continue à chercher une meilleure solution selon finalCosts.\n"
            "\n"
            "Exemple :\n"
            "\n"
            "Solution A : finalCosts = [0, 2, 8]\n"
            "Solution B : finalCosts = [0, 1, 10]\n"
            "\n"
            "Le premier coût est égal : 0 = 0.\n"
            "Le deuxième coût est meilleur pour B : 1 < 2.\n"
            "Donc B est préférée, même si son troisième coût est moins bon.\n"
            "\n"
            "\n"
            "Résumé très simple\n"
            "\n"
            "Les contraintes de Fux définissent ce qui est autorisé.\n"
            "Les coûts définissent ce qui est préférable.\n"
            "Le vecteur importance définit quelles préférences passent avant les autres.\n"
            "BAB cherche la meilleure solution autorisée en minimisant finalCosts."
        );
    }
}

SolverExplanationPage::SolverExplanationPage()
{
    titleLabel.setText(juce::String::fromUTF8("How the solver works?"),
                       juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    titleLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    addAndMakeVisible(titleLabel);

    explanationText.setMultiLine(true);
    explanationText.setReadOnly(true);
    explanationText.setScrollbarsShown(true);
    explanationText.setText(buildSolverExplanationText(), juce::dontSendNotification);
    explanationText.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff3f3f3f));
    explanationText.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    explanationText.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    explanationText.setColour(juce::TextEditor::textColourId, juce::Colours::white.withAlpha(0.9f));
    explanationText.setFont(juce::Font(14.0f));
    addAndMakeVisible(explanationText);
}

void SolverExplanationPage::paint(juce::Graphics& g)
{
    auto area = getLocalBounds().reduced(22);

    g.setColour(juce::Colour(0xff3f3f3f));
    g.fillRoundedRectangle(area.toFloat(), 8.0f);

    g.setColour(juce::Colours::white.withAlpha(0.25f));
    g.drawRoundedRectangle(area.toFloat(), 8.0f, 1.0f);
}

void SolverExplanationPage::resized()
{
    auto area = getLocalBounds().reduced(42, 32);

    titleLabel.setBounds(area.removeFromTop(30));
    area.removeFromTop(12);

    explanationText.setBounds(area);
}
