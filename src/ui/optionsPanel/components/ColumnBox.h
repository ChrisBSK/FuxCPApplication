#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * Colonne interactive (hover / click / active)
 */
class ColumnBox : public juce::Component
{
public:
    bool isActive = false;
    bool isHovered = false;

    std::function<void()> onEnter;
    std::function<void()> onExit;
    std::function<void()> onClick;

    void mouseEnter(const juce::MouseEvent&) override
    {
        isHovered = true;
        if (onEnter) onEnter();
    }

    void mouseExit(const juce::MouseEvent&) override
    {
        isHovered = false;
        if (onExit) onExit();
    }

    void mouseDown(const juce::MouseEvent&) override
    {
        if (onClick) onClick();
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2.0f);

        // fond normal
        g.setColour(juce::Colours::darkgrey.darker(0.3f));
        g.fillRoundedRectangle(bounds, 10.0f);

        // hover
        if (!isActive && isHovered)
        {
            g.setColour(juce::Colour(0xff2f4f4f).withAlpha(0.5f));
            g.fillRoundedRectangle(bounds, 10.0f);
        }

        // contour
        if (isActive)
        {
            g.setColour(juce::Colour(0xff2f4f4f));
            g.drawRoundedRectangle(bounds, 10.0f, 3.5f); // contour épais (colonne active)
        }
        else
        {
            g.setColour(juce::Colours::white.withAlpha(0.2f));
            g.drawRoundedRectangle(bounds, 10.0f, 1.5f); // contour fin (normal)
        }
    }
    
};