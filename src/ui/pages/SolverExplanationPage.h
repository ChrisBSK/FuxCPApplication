//
// Créé par Chris BAKASHIKA (2026)
//

/*
//==============================================================================
   SolverExplanationPage.h

   Page pédagogique qui explique simplement le chemin suivi par le
   solveur : variables de notes, contraintes, unitedCosts, finalCosts,
   puis BAB.
//==============================================================================
*/


#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

/*
    SolverExplanationPage

    Page pédagogique qui explique simplement le chemin suivi par le solveur :
    variables de notes, contraintes, unitedCosts, finalCosts, puis BAB.
*/
class SolverExplanationPage : public juce::Component
{
public:
    SolverExplanationPage();

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    // Affiche le titre principal de la page.
    juce::Label titleLabel;

    // Affiche le texte explicatif. Le texte est multi-ligne pour rester aéré.
    juce::TextEditor explanationText;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SolverExplanationPage)
};
