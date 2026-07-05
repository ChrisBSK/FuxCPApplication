#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

/*
//==============================================================================
   OnOffSwitchButton

   Bouton binaire rouge/vert avec poignée coulissante.
//==============================================================================
*/
class OnOffSwitchButton : public juce::Button
{
public:
    OnOffSwitchButton()
        : juce::Button("OnOffSwitchButton")
    {
        setClickingTogglesState(true);
    }

    bool isOn() const
    {
        return getToggleState();
    }

    void setOn(bool shouldBeOn, juce::NotificationType notification)
    {
        setToggleState(shouldBeOn, notification);
    }

    int getPreferredHeight() const
    {
        return 36;
    }

    void paintButton(juce::Graphics& g,
                     bool shouldDrawButtonAsHighlighted,
                     bool shouldDrawButtonAsDown) override
    {
        const bool on = isOn();
        auto bounds = getLocalBounds().toFloat().reduced(1.0f);
        const float corner = bounds.getHeight() * 0.5f;

        g.setColour(juce::Colour(0x55000000));
        g.fillRoundedRectangle(bounds.translated(0.0f, 1.5f), corner);

        g.setColour(juce::Colour(0xffeeeeee));
        g.fillRoundedRectangle(bounds, corner);

        auto inner = bounds.reduced(3.0f);
        const auto fillColour = on ? juce::Colour(0xff40b82a)
                                   : juce::Colour(0xffee3333);

        g.setColour(shouldDrawButtonAsDown ? fillColour.darker(0.15f) : fillColour);
        g.fillRoundedRectangle(inner, inner.getHeight() * 0.5f);

        drawText(g, inner, on);
        drawKnob(g, inner, on, shouldDrawButtonAsHighlighted);
    }

private:
    void drawText(juce::Graphics& g, juce::Rectangle<float> area, bool on) const
    {
        const auto textArea = on
            ? area.withTrimmedLeft(area.getWidth() * 0.30f).reduced(3.0f, 0.0f)
            : area.withTrimmedRight(area.getWidth() * 0.30f).reduced(3.0f, 0.0f);

        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(juce::FontOptions(area.getHeight() * 0.54f,
                                               juce::Font::bold)));
        g.drawFittedText(on ? "ON" : "OFF",
                         textArea.toNearestInt(),
                         juce::Justification::centred,
                         1);
    }

    void drawKnob(juce::Graphics& g,
                  juce::Rectangle<float> area,
                  bool on,
                  bool highlighted) const
    {
        const float diameter = area.getHeight();
        const float x = on ? area.getX()
                           : area.getRight() - diameter;
        auto knob = juce::Rectangle<float>(x, area.getY(), diameter, diameter).reduced(1.0f);

        g.setColour(juce::Colour(0x66000000));
        g.fillEllipse(knob.translated(1.0f, 1.0f));

        auto gradient = juce::ColourGradient(juce::Colour(0xfff4f4f4),
                                             knob.getCentreX(),
                                             knob.getY(),
                                             juce::Colour(0xffbcbcbc),
                                             knob.getCentreX(),
                                             knob.getBottom(),
                                             false);
        g.setGradientFill(gradient);
        g.fillEllipse(knob);

        g.setColour(highlighted ? juce::Colour(0xff333333) : juce::Colour(0xff4a4a4a));
        g.drawEllipse(knob, 2.0f);
        drawPowerIcon(g, knob.reduced(knob.getWidth() * 0.25f));
    }

    void drawPowerIcon(juce::Graphics& g, juce::Rectangle<float> area) const
    {
        const auto centre = area.getCentre();
        const float radius = juce::jmin(area.getWidth(), area.getHeight()) * 0.38f;
        const float stroke = juce::jlimit(2.0f, 4.0f, radius * 0.35f);

        juce::Path arc;
        arc.addCentredArc(centre.x,
                          centre.y + radius * 0.12f,
                          radius,
                          radius,
                          0.0f,
                          juce::MathConstants<float>::pi * 0.32f,
                          juce::MathConstants<float>::pi * 1.68f,
                          true);

        g.strokePath(arc, juce::PathStrokeType(stroke,
                                               juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));

        g.drawLine(centre.x,
                   area.getY(),
                   centre.x,
                   centre.y + radius * 0.15f,
                   stroke);
    }
};
