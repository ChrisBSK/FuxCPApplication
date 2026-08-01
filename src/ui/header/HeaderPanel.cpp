#include "HeaderPanel.h"

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

HeaderPanel::HeaderPanel()
{
    setupTabButton(mainScreenButton, "Main Screen");
    setupTabButton(glossaryButton, "Glossary");
    setupTabButton(aboutButton, "About");

    addAndMakeVisible(mainScreenButton);
    addAndMakeVisible(glossaryButton);
    addAndMakeVisible(aboutButton);

    mainScreenButton.onClick = [this]() { selectPage(Page::mainScreen); };
    glossaryButton.onClick   = [this]() { selectPage(Page::glossary); };
    aboutButton.onClick      = [this]() { selectPage(Page::about); };

    selectPage(Page::mainScreen);
}

HeaderPanel::~HeaderPanel()
{
}

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

void HeaderPanel::resized()
{
    auto area = getLocalBounds().reduced(18, 0);

    constexpr int tabWidth = 110;
    constexpr int gap = 4;

    mainScreenButton.setBounds(area.removeFromLeft(tabWidth));
    area.removeFromLeft(gap);

    glossaryButton.setBounds(area.removeFromLeft(tabWidth));
    area.removeFromLeft(gap);

    aboutButton.setBounds(area.removeFromLeft(tabWidth));
}

void HeaderPanel::selectPage(Page page)
{
    selectedPage = page;

    mainScreenButton.setToggleState(selectedPage == Page::mainScreen, juce::dontSendNotification);
    glossaryButton.setToggleState(selectedPage == Page::glossary, juce::dontSendNotification);
    aboutButton.setToggleState(selectedPage == Page::about, juce::dontSendNotification);

    if (onPageChanged)
        onPageChanged(selectedPage);
}

void HeaderPanel::setupTabButton(juce::TextButton& button, const juce::String& text)
{
    button.setButtonText(text);
    button.setClickingTogglesState(false);
}
