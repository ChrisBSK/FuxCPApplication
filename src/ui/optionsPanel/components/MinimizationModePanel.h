#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

/*
//==============================================================================
   MinimizationModePanel

   Petit panneau visuel pour choisir :
   - la methode de recherche : DFS ou BAB
   - la methode de minimisation : Lexicographic ou Weighted Sum

   Pour l'instant, ces choix ne sont pas encore connectes au solveur.
//==============================================================================
*/
class MinimizationModePanel : public juce::Component
{
public:
    std::function<void(bool)> onBabSearchMethodChanged;
    std::function<void(bool)> onLexicographicModeChanged;

    MinimizationModePanel()
    {
        setupGroupTitle(searchMethodTitle, "Search Method");
        setupGroupTitle(minimizationMethodTitle, "Minimization Method");

        addAndMakeVisible(searchMethodTitle);
        addAndMakeVisible(minimizationMethodTitle);
        addAndMakeVisible(dfsButton);
        addAndMakeVisible(babButton);
        addAndMakeVisible(lexicographicButton);
        addAndMakeVisible(weightedSumButton);

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

        selectSearchMethod(SearchMethod::bab);
        selectMinimizationMode(MinimizationMode::lexicographic);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(10, 8);

        constexpr int titleHeight = 12;
        constexpr int searchButtonHeight = 18;
        constexpr int minimizationButtonHeight = 22;
        constexpr int titleGap = 3;
        constexpr int groupGap = 8;
        constexpr int gap = 5;

        searchMethodTitle.setBounds(area.removeFromTop(titleHeight));
        area.removeFromTop(titleGap);

        auto searchRow = area.removeFromTop(searchButtonHeight);
        layoutButtonRow(searchRow, dfsButton, babButton, gap);

        area.removeFromTop(groupGap);

        minimizationMethodTitle.setBounds(area.removeFromTop(titleHeight));
        area.removeFromTop(titleGap);

        auto minimizationRow = area.removeFromTop(minimizationButtonHeight);
        layoutButtonRow(minimizationRow, lexicographicButton, weightedSumButton, gap);
    }

private:
    enum class SearchMethod
    {
        dfs,
        bab
    };

    enum class MinimizationMode
    {
        lexicographic,
        weightedSum
    };

    /*
       Petit bouton dessine a la main pour garder une ecriture fine,
       lisible et coherente avec le reste de l'interface.
    */
    //--> Réalisé avec l'aide de ChatGPT
    class ModeButton : public juce::Button
    {
    public:
        explicit ModeButton(const juce::String& text)
            : juce::Button(text)
        {
        }

        void setActive(bool shouldBeActive)
        {
            isActive = shouldBeActive;
            repaint();
        }

        void paintButton(juce::Graphics& g,
                         bool isMouseOverButton,
                         bool isButtonDown) override
        {
            auto bounds = getLocalBounds().toFloat().reduced(0.5f);

            auto background = isActive ? juce::Colour(0xff2f4f4f)
                                       : juce::Colour(0xff3e3e3e);

            if (isMouseOverButton)
                background = background.brighter(0.08f);

            if (isButtonDown)
                background = background.darker(0.12f);

            g.setColour(background);
            g.fillRoundedRectangle(bounds, 4.0f);

            g.setColour(isActive ? juce::Colours::white.withAlpha(0.95f)
                                 : juce::Colours::white.withAlpha(0.75f));
            g.drawRoundedRectangle(bounds, 4.0f, isActive ? 1.0f : 0.7f);

            g.setColour(juce::Colours::white);
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

    SearchMethod selectedSearchMethod = SearchMethod::bab;
    MinimizationMode selectedMinimizationMode = MinimizationMode::lexicographic;

    juce::Label searchMethodTitle;
    juce::Label minimizationMethodTitle;

    ModeButton dfsButton { "DFS" };
    ModeButton babButton { "BAB" };
    ModeButton lexicographicButton { "Lexicographic" };
    ModeButton weightedSumButton { "Weighted Sum" };

    // Configure un petit titre de groupe lisible et discret.
    void setupGroupTitle(juce::Label& label, const juce::String& text)
    {
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setFont(juce::Font(juce::FontOptions(8.5f, juce::Font::bold)));
        label.setColour(juce::Label::textColourId, juce::Colours::white);
    }

    // Place deux boutons sur une meme ligne.
    void layoutButtonRow(juce::Rectangle<int> row,
                         ModeButton& leftButton,
                         ModeButton& rightButton,
                         int gap)
    {
        auto left = row.removeFromLeft((row.getWidth() - gap) / 2);
        row.removeFromLeft(gap);

        leftButton.setBounds(left);
        rightButton.setBounds(row);
    }

    // Change uniquement l'etat visuel de la methode de recherche.
    void selectSearchMethod(SearchMethod method)
    {
        selectedSearchMethod = method;

        dfsButton.setActive(selectedSearchMethod == SearchMethod::dfs);
        babButton.setActive(selectedSearchMethod == SearchMethod::bab);

        if (onBabSearchMethodChanged != nullptr)
            onBabSearchMethodChanged(selectedSearchMethod == SearchMethod::bab);
    }

    // Change uniquement l'etat visuel de la methode de minimisation.
    void selectMinimizationMode(MinimizationMode mode)
    {
        selectedMinimizationMode = mode;

        lexicographicButton.setActive(selectedMinimizationMode == MinimizationMode::lexicographic);
        weightedSumButton.setActive(selectedMinimizationMode == MinimizationMode::weightedSum);

        if (onLexicographicModeChanged != nullptr)
            onLexicographicModeChanged(selectedMinimizationMode == MinimizationMode::lexicographic);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MinimizationModePanel)
};
