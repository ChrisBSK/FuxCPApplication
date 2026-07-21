#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * Colonne interactive.
 */
class ColumnBox : public juce::Component
{
public:
    // État visuel de la colonne.
    bool isActive = false;
    bool isHovered = false;
    juce::Colour activeColour = juce::Colour(0xff2f4f4f);

    // État visuel de la colonne.
    std::function<void()> onEnter;
    std::function<void()> onExit;
    std::function<void()> onClick;

    // Met à jour l'état de survol et notifie l'UI.
    void mouseEnter(const juce::MouseEvent&) override
    {
        isHovered = true;
        if (onEnter) onEnter();
    }

    // Réinitialise l'état de survol et notifie l'UI.
    void mouseExit(const juce::MouseEvent&) override
    {
        isHovered = false;
        if (onExit) onExit();
    }

    // Déclenche l'action associée au clic.
    void mouseDown(const juce::MouseEvent&) override
    {
        if (onClick) onClick();
    }

    // Dessine le fond et le contour de la colonne.
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2.0f);

        // fond normal
        g.setColour(juce::Colours::darkgrey.darker(0.3f));
        g.fillRoundedRectangle(bounds, 10.0f);

        // contour
        if (isActive)
        {
            g.setColour(activeColour);
            g.drawRoundedRectangle(bounds, 10.0f, 3.5f); // contour épais (colonne active)
        }
        else
        {
            g.setColour(juce::Colours::white.withAlpha(0.2f));
            g.drawRoundedRectangle(bounds, 10.0f, 1.5f); // contour fin (normal)
        }
    }
    
};
