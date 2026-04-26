#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * Label custom avec fond + texte stylé
 * → purement visuel
 */
class StyledLabel : public juce::Label
{
public:
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        g.setColour(juce::Colour(0xff2f4f4f));
        g.fillRect(bounds);

        g.setColour(juce::Colours::white);
        g.setFont(getFont());

        g.drawText(getText(), getLocalBounds(), getJustificationType(), true);
    }
};