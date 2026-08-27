//
// Créé par Chris BAKASHIKA (2026)
//

/*
//==============================================================================
    HeaderPanel.cpp

    Dessine les onglets (état actif, survol, soulignement) et gère la
    sélection au clic via selectPage, qui notifie MainComponent par
    onPageChanged.
//==============================================================================
*/

#include "HeaderPanel.h"

// Dessine un onglet : fond au survol, texte, et soulignement si actif.
void HeaderPanel::TabButton::paintButton(juce::Graphics& g,
                                         bool shouldDrawButtonAsHighlighted,
                                         bool)
{
    const auto area = getLocalBounds().toFloat();
    const bool active = getToggleState();

    if (shouldDrawButtonAsHighlighted && ! active)
    {
        g.setColour(juce::Colours::white.withAlpha(0.06f));
        g.fillRoundedRectangle(area.reduced(2.0f, 3.0f), 4.0f);
    }

    g.setColour(active ? juce::Colours::white
                       : juce::Colours::white.withAlpha(0.72f));
    g.setFont(juce::Font(13.0f, active ? juce::Font::bold : juce::Font::plain));
    g.drawText(getButtonText(),
               getLocalBounds().reduced(8, 0),
               juce::Justification::centred,
               true);

    if (active)
    {
        auto underline = area.withY(area.getBottom() - 3.0f)
                             .withHeight(2.0f)
                             .reduced(16.0f, 0.0f);

        g.setColour(juce::Colour(0xff74c7b8));
        g.fillRoundedRectangle(underline, 1.0f);
    }
}

// Crée les quatre onglets et sélectionne Main Screen par défaut.
HeaderPanel::HeaderPanel()
{
    setupTabButton(mainScreenButton, "Main Screen");
    setupTabButton(savedSolutionsButton, "Saved configurations");
    setupTabButton(solverButton, "Solver");
    setupTabButton(aboutButton, "About");

    addAndMakeVisible(mainScreenButton);
    addAndMakeVisible(savedSolutionsButton);
    addAndMakeVisible(solverButton);
    addAndMakeVisible(aboutButton);

    mainScreenButton.onClick = [this]() { selectPage(Page::mainScreen); };
    savedSolutionsButton.onClick = [this]() { selectPage(Page::savedSolutions); };
    solverButton.onClick = [this]() { selectPage(Page::solver); };
    aboutButton.onClick      = [this]() { selectPage(Page::about); };

    selectPage(Page::mainScreen);
}

HeaderPanel::~HeaderPanel()
{
}

// Fond de la barre d'onglets et fine ligne de séparation en bas.
void HeaderPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);

    g.setColour(juce::Colours::white.withAlpha(0.10f));
    g.drawLine(0.0f,
               static_cast<float>(getHeight() - 1),
               static_cast<float>(getWidth()),
               static_cast<float>(getHeight() - 1),
               1.0f);
}

// Place les quatre onglets côte à côte, de gauche à droite.
void HeaderPanel::resized()
{
    auto area = getLocalBounds().reduced(18, 0);

    constexpr int tabWidth = 110;
    constexpr int gap = 4;

    mainScreenButton.setBounds(area.removeFromLeft(tabWidth));
    area.removeFromLeft(gap);

    savedSolutionsButton.setBounds(area.removeFromLeft(tabWidth));
    area.removeFromLeft(gap);

    solverButton.setBounds(area.removeFromLeft(tabWidth));
    area.removeFromLeft(gap);

    aboutButton.setBounds(area.removeFromLeft(tabWidth));
}

// Met à jour l'onglet actif et notifie MainComponent du changement de page.
void HeaderPanel::selectPage(Page page)
{
    selectedPage = page;

    mainScreenButton.setToggleState(selectedPage == Page::mainScreen, juce::dontSendNotification);
    savedSolutionsButton.setToggleState(selectedPage == Page::savedSolutions, juce::dontSendNotification);
    solverButton.setToggleState(selectedPage == Page::solver, juce::dontSendNotification);
    aboutButton.setToggleState(selectedPage == Page::about, juce::dontSendNotification);

    if (onPageChanged)
        onPageChanged(selectedPage);
}

// Configure un bouton d'onglet
void HeaderPanel::setupTabButton(juce::TextButton& button, const juce::String& text)
{
    button.setButtonText(text);
    button.setClickingTogglesState(false);
}
