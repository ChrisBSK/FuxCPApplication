#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "ParameterHelpText.h"
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
        label.setText(formatLabelForDisplay(labelText), juce::dontSendNotification);
        label.setColour(juce::Label::textColourId, juce::Colours::white);
        label.setJustificationType(juce::Justification::centred);
        label.setTooltip(ParameterHelpText::getForLabel(labelText));

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

        // Disposition fixe :
        // - label toujours à gauche
        // - contrôle toujours à droite
        // Les largeurs restent adaptatives pour garder les deux éléments lisibles.
        const int gap = juce::jlimit(5, 10, area.getWidth() / 28);
        const int minimumControlWidth = juce::jlimit(70, 120, area.getWidth() / 2);
        const int maximumLabelWidth = juce::jlimit(82, 120, area.getWidth() / 2);
        const int availableLabelWidth = area.getWidth() - gap - minimumControlWidth;
        const int labelWidth = juce::jlimit(70, maximumLabelWidth, availableLabelWidth);

        label.setBounds(area.removeFromLeft(labelWidth));
        area.removeFromLeft(gap);

        if (control != nullptr)
            control->setBounds(area.reduced(0, 2));
    }

private:
    /*
        Prépare uniquement le texte visible.
        Le nom logique reste inchangé pour les tooltips et le reste du code.
    */
    static juce::String formatLabelForDisplay(const juce::String& labelText)
    {
        if (labelText == "Avoid Repeated Notes")
            return "Avoid Repeated\nNotes";

        return labelText;
    }

    StyledLabel label;
    std::unique_ptr<juce::Component> control;
};
