#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

/*
    =====================================================================
    MinimizationModePanel.h — choix de recherche et de minimisation

    Petit panneau affiché dans la colonne Search.

    Il permet de choisir :
    - la méthode de recherche : DFS ou BAB,
    - la méthode de minimisation : Lexicographic ou Weighted Sum.

    Le composant ne modifie pas directement le modèle.
    Il expose des callbacks que OptionsPanel connecte ensuite à AppController.
    =====================================================================
*/
class MinimizationModePanel : public juce::Component
{
public:
    // Callbacks envoyés à OptionsPanel quand l'utilisateur change de mode.
    // bool = true signifie BAB pour la recherche, Lexicographic pour la minimisation.
    std::function<void(bool)> onBabSearchMethodChanged;
    std::function<void(bool)> onLexicographicModeChanged;

    /*
    Crée les titres, les boutons, puis connecte leurs clics.

    Les valeurs par défaut sont :
    - BAB pour la recherche,
    - Lexicographic pour la minimisation.
*/
    MinimizationModePanel()
    {
        // Titres des deux groupes du panneau.
        setupGroupTitle(searchMethodTitle, "Search Method");
        setupGroupTitle(minimizationMethodTitle, "Minimization Method");

        // Rend tous les éléments visibles dans le composant.
        addAndMakeVisible(searchMethodTitle);
        addAndMakeVisible(minimizationMethodTitle);
        addAndMakeVisible(dfsButton);
        addAndMakeVisible(babButton);
        addAndMakeVisible(lexicographicButton);
        addAndMakeVisible(weightedSumButton);

        /*
            Explication brève au survol de chaque méthode.

            juce::Button hérite déjà de SettableTooltipClient : setTooltip()
            suffit, le TooltipWindow créé dans MainComponent affiche le
            texte automatiquement, comme pour les priorités du solveur.
        */
        dfsButton.setTooltip(juce::String::fromUTF8(
            "Depth-first Search : explore les solutions en profondeur et "
            "s'arrête à la première trouvée, sans chercher à minimiser les coûts."));

        babButton.setTooltip(juce::String::fromUTF8(
            "Branch and Bound : explore les solutions en écartant celles "
            "qui ne peuvent pas faire mieux que la meilleure déjà trouvée."));

        lexicographicButton.setTooltip(juce::String::fromUTF8(
            "Lexicographic : minimise le coût de chaque contrainte du "
            "vecteur de priorités, une à la fois, dans l'ordre choisi. Le "
            "coût d'une contrainte plus importante n'est jamais sacrifié "
            "pour réduire celui d'une contrainte moins prioritaire."));

        weightedSumButton.setTooltip(juce::String::fromUTF8(
            "Weighted Sum : minimise la somme des coûts de chaque "
            "contrainte du vecteur de priorités, chacun multiplié par son "
            "poids. Contrairement à Lexicographic, toutes les contraintes "
            "comptent en même temps, pas une à la fois."));

        // Chaque bouton sélectionne son mode correspondant.
        dfsButton.onClick = [this]()
        {
            selectSearchMethod(SearchMethod::dfs);
        };

        babButton.onClick = [this]()
        {
            selectSearchMethod(SearchMethod::bab);
        };

        lexicographicButton.onClick = [this]()
        {
            selectMinimizationMode(MinimizationMode::lexicographic);
        };

        weightedSumButton.onClick = [this]()
        {
            selectMinimizationMode(MinimizationMode::weightedSum);
        };

        // État initial affiché au lancement de l'application.
        selectSearchMethod(SearchMethod::bab);
        selectMinimizationMode(MinimizationMode::lexicographic);
    }

    //==============================================================================
    // Réaffichage depuis une configuration chargée
    //==============================================================================

    /*
        Affiche la méthode de recherche chargée (BAB ou DFS).

        Passe par selectSearchMethod : le callback onBabSearchMethodChanged
        est donc bien déclenché. Ce n'est pas un problème ici, puisque
        OptionsPanel réécrirait alors dans ConstraintSettings exactement la
        valeur qui vient d'être chargée - sans effet, mais sans code
        dupliqué non plus.
    */
    void setSearchMethod(bool useBab)
    {
        selectSearchMethod(useBab ? SearchMethod::bab : SearchMethod::dfs);
    }

    /*
        Affiche la méthode de minimisation chargée (Lexicographic ou Weighted Sum).

        Déclenche aussi onLexicographicModeChanged, ce qui permet à
        OptionsPanel de remettre solverPriorityList dans le bon mode
        d'affichage (rank ou weight) sans dupliquer cette logique ici.
    */
    void setMinimizationMode(bool isLexicographic)
    {
        selectMinimizationMode(isLexicographic ? MinimizationMode::lexicographic
                                               : MinimizationMode::weightedSum);
    }

    /*
        Place les titres et les boutons dans le panneau.

        Le layout est vertical :
        - titre Search Method,
        - boutons DFS / BAB,
        - titre Minimization Method,
        - boutons Lexicographic / Weighted Sum.
    */
    void resized() override
    {
        // Zone intérieure avec une petite marge
        auto area = getLocalBounds().reduced(10, 8);

        constexpr int titleHeight = 12;
        constexpr int searchButtonHeight = 18;
        constexpr int minimizationButtonHeight = 22;
        constexpr int titleGap = 3;
        constexpr int groupGap = 8;
        constexpr int gap = 5;

        // Premier groupe : choix de la méthode de recherche.
        searchMethodTitle.setBounds(area.removeFromTop(titleHeight));
        area.removeFromTop(titleGap);

        auto searchRow = area.removeFromTop(searchButtonHeight);
        layoutButtonRow(searchRow, dfsButton, babButton, gap);

        area.removeFromTop(groupGap);

        // Deuxième groupe : choix de la méthode de minimisation.
        minimizationMethodTitle.setBounds(area.removeFromTop(titleHeight));
        area.removeFromTop(titleGap);

        auto minimizationRow = area.removeFromTop(minimizationButtonHeight);
        layoutButtonRow(minimizationRow, lexicographicButton, weightedSumButton, gap);
    }

private:

    // Modes de recherche disponibles dans le solveur.
    enum class SearchMethod
    {
        dfs,
        bab
    };

    // Modes de minimisation affichés dans l'interface.
    enum class MinimizationMode
    {
        lexicographic,
        weightedSum
    };

    /*
        Petit bouton personnalisé pour les modes Search / Minimization.
    */
    //--> Réalisé avec l'aide de ChatGPT
    class ModeButton : public juce::Button
    {
    public:
        explicit ModeButton(const juce::String& text)
            : juce::Button(text)
        {
        }

        // Met à jour l'état sélectionné du bouton puis le redessine.
        void setActive(bool shouldBeActive)
        {
            isActive = shouldBeActive;
            repaint();
        }

        /*
        Dessine le bouton selon son état.

        États pris en compte :
        - actif ou inactif,
        - survolé par la souris,
        - enfoncé pendant le clic.
    */
        void paintButton(juce::Graphics& g,
                         bool isMouseOverButton,
                         bool isButtonDown) override
        {
            auto bounds = getLocalBounds().toFloat().reduced(0.5f);

            // Couleur de base selon l'état actif/inactif.
            auto background = isActive ? juce::Colour(0xff2f4f4f)
                                       : juce::Colour(0xff3e3e3e);

            // Feedback visuel au survol et au clic.
            if (isMouseOverButton)
                background = background.brighter(0.08f);

            if (isButtonDown)
                background = background.darker(0.12f);

            // Fond + contour du bouton.
            g.setColour(background);
            g.fillRoundedRectangle(bounds, 4.0f);

            g.setColour(isActive ? juce::Colours::white.withAlpha(0.95f)
                                 : juce::Colours::white.withAlpha(0.75f));
            g.drawRoundedRectangle(bounds, 4.0f, isActive ? 1.0f : 0.7f);

            g.setColour(juce::Colours::white);

            // Texte centré, géré par une méthode dédiée.
            drawReadableButtonText(g);
        }

    private:
        bool isActive = false;

        // Dessine le texte sur une ou deux lignes selon l'espace disponible.
        void drawReadableButtonText(juce::Graphics& g)
        {
            auto textBounds = getLocalBounds().reduced(2, 1);
            const auto text = getButtonText();


            if (text == "Weighted Sum")
            {
                // Cas spécial : texte long, donc affichage sur deux lignes.
                g.setFont(juce::Font(juce::FontOptions(8.0f, juce::Font::bold)));
                auto top = textBounds.removeFromTop(textBounds.getHeight() / 2);
                g.drawText("Weighted", top, juce::Justification::centred, true);
                g.drawText("Sum", textBounds, juce::Justification::centred, true);
                return;
            }

            g.setFont(juce::Font(juce::FontOptions(text == "Lexicographic" ? 8.0f : 8.5f,
                                                   juce::Font::bold)));
            g.drawText(text, textBounds, juce::Justification::centred, true);
        }
    };

    // État actuellement sélectionné dans le panneau. (au lancement de l'App)
    SearchMethod selectedSearchMethod = SearchMethod::bab;
    MinimizationMode selectedMinimizationMode = MinimizationMode::lexicographic;

    // Titres des deux groupes de boutons.
    juce::Label searchMethodTitle;
    juce::Label minimizationMethodTitle;

    ModeButton dfsButton { "DFS" };
    ModeButton babButton { "BAB" };
    ModeButton lexicographicButton { "Lexicographic" };
    ModeButton weightedSumButton { "Weighted Sum" };

    /*
        Configure un titre de groupe compact.

        Utilisé pour :
        - Search Method,
        - Minimization Method.
    */
    void setupGroupTitle(juce::Label& label, const juce::String& text)
    {
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setFont(juce::Font(juce::FontOptions(8.5f, juce::Font::bold)));
        label.setColour(juce::Label::textColourId, juce::Colours::white);
    }

    /*
        Place deux boutons côte à côte dans une même ligne.

        La ligne est divisée en :
        - bouton gauche,
        - espace central,
        - bouton droit.
    */
    void layoutButtonRow(juce::Rectangle<int> row,
                         ModeButton& leftButton,
                         ModeButton& rightButton,
                         int gap)
    {
        // Coupe la moitié gauche de la ligne.
        auto left = row.removeFromLeft((row.getWidth() - gap) / 2);
        // Retire l'espace entre les deux boutons.
        row.removeFromLeft(gap);

        leftButton.setBounds(left);
        rightButton.setBounds(row);
    }

    /*
        Sélectionne la méthode de recherche.

        Met à jour :
        - l'état interne,
        - l'apparence des boutons DFS/BAB,
        - le callback envoyé à OptionsPanel.
    */
    void selectSearchMethod(SearchMethod method)
    {
        selectedSearchMethod = method;

        // Met à jour l'état visuel des deux boutons.
        dfsButton.setActive(selectedSearchMethod == SearchMethod::dfs);
        babButton.setActive(selectedSearchMethod == SearchMethod::bab);

        if (onBabSearchMethodChanged != nullptr)
            // Informe OptionsPanel du choix courant.
            onBabSearchMethodChanged(selectedSearchMethod == SearchMethod::bab);
    }

    /*
        Sélectionne la méthode de minimisation.

        Met à jour :
        - l'état interne,
        - l'apparence des boutons Lexicographic/Weighted Sum,
        - le callback envoyé à OptionsPanel.
    */
    void selectMinimizationMode(MinimizationMode mode)
    {
        selectedMinimizationMode = mode;

        // Met à jour l'état visuel des deux boutons.
        lexicographicButton.setActive(selectedMinimizationMode == MinimizationMode::lexicographic);
        weightedSumButton.setActive(selectedMinimizationMode == MinimizationMode::weightedSum);

        if (onLexicographicModeChanged != nullptr)
            // Informe OptionsPanel du choix courant.
            onLexicographicModeChanged(selectedMinimizationMode == MinimizationMode::lexicographic);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MinimizationModePanel)
};
