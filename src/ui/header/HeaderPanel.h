//
// Créé par Chris BAKASHIKA (2026)
//

/*
//==============================================================================
   HeaderPanel.h

   Barre d'onglets principale : permet de choisir une page
   (Main Screen, Saved configurations, Solver,
   About)
//==============================================================================
*/

#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class HeaderPanel : public juce::Component
{
public:
    enum class Page
    {
        mainScreen,
        savedSolutions,
        solver,
        about
    };

    HeaderPanel();
    ~HeaderPanel() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    // Callback appelé quand l'utilisateur clique sur un onglet.
    std::function<void(Page)> onPageChanged;

private:
    /*
        Bouton d'onglet au rendu plat.

        Il imite l'idée des onglets Google :
        - pas de gros rectangle rempli,
        - texte discret,
        - trait sous l'onglet actif.
    */
    class TabButton : public juce::TextButton
    {
    public:
        using juce::TextButton::TextButton;

        void paintButton(juce::Graphics& g,
                         bool shouldDrawButtonAsHighlighted,
                         bool) override;
    };

    // Sélectionne visuellement un onglet et prévient MainComponent.
    void selectPage(Page page);

    // Prépare un bouton d'onglet avec le même style que les autres.
    void setupTabButton(juce::TextButton& button, const juce::String& text);

    Page selectedPage = Page::mainScreen;

    TabButton mainScreenButton;
    TabButton savedSolutionsButton;
    TabButton solverButton;
    TabButton aboutButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeaderPanel);
};
