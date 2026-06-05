#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "StyledLabel.h"

/*
//==============================================================================
   ParameterRow

   Ligne composée d'un label et d'un contrôle UI.
//==============================================================================
*/
class ParameterRow : public juce::Component
{
public:
    ParameterRow(const juce::String& labelText,
                 std::unique_ptr<juce::Component> controlToUse)
        : control(std::move(controlToUse))
    {
        label.setText(labelText, juce::dontSendNotification);
        label.setColour(juce::Label::textColourId, juce::Colours::white);
        label.setJustificationType(juce::Justification::centred);

        addAndMakeVisible(label);

        if (control != nullptr)
            addAndMakeVisible(*control);
    }

    template <typename ControlType>
    ControlType* getControlAs()
    {
        return dynamic_cast<ControlType*>(control.get());
    }

    void resized() override
    {
        auto area = getLocalBounds();

        constexpr int labelWidth = 95;
        constexpr int gap = 12;

        label.setBounds(area.removeFromLeft(labelWidth));
        area.removeFromLeft(gap);

        if (control != nullptr)
            control->setBounds(area.reduced(0, 3));
    }

private:
    StyledLabel label;
    std::unique_ptr<juce::Component> control;
};