#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

/*
//==============================================================================
   ArrowIconButton

   Bouton compact qui dessine une vraie flèche.

//==============================================================================
*/
class ArrowIconButton : public juce::Button
{
public:
    enum class Direction
    {
        up,
        down
    };

    explicit ArrowIconButton(Direction directionToUse)
        : juce::Button("ArrowIconButton"),
          direction(directionToUse)
    {
        setWantsKeyboardFocus(false);
    }

    void paintButton(juce::Graphics& g,
                     bool isMouseOverButton,
                     bool isButtonDown) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(1.0f);

        const auto background = isButtonDown
            ? juce::Colour(0xff203838)
            : juce::Colour(0xff243f45);

        g.setColour(isMouseOverButton
            ? background.brighter(0.12f)
            : background);
        g.fillRoundedRectangle(bounds, 6.0f);

        g.setColour(juce::Colours::white.withAlpha(0.9f));
        g.strokePath(buildArrowPath(bounds),
                     juce::PathStrokeType(2.0f,
                                          juce::PathStrokeType::curved,
                                          juce::PathStrokeType::rounded));
    }

private:
    /*
        Construit une flèche simple en deux segments.
        La direction dépend uniquement du type du bouton.
    */
    juce::Path buildArrowPath(juce::Rectangle<float> bounds) const
    {
        const auto centre = bounds.getCentre();
        const float width = bounds.getWidth() * 0.34f;
        const float height = bounds.getHeight() * 0.24f;

        juce::Path arrow;

        if (direction == Direction::up)
        {
            arrow.startNewSubPath(centre.x - width, centre.y + height);
            arrow.lineTo(centre.x, centre.y - height);
            arrow.lineTo(centre.x + width, centre.y + height);
        }
        else
        {
            arrow.startNewSubPath(centre.x - width, centre.y - height);
            arrow.lineTo(centre.x, centre.y + height);
            arrow.lineTo(centre.x + width, centre.y - height);
        }

        return arrow;
    }

    Direction direction;
};
