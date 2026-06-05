#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
/*
//==============================================================================
   OptionsPanelHelpers

   Fonctions utilitaires pour configurer les éléments visuels
   réutilisables de l'OptionsPanel.
//==============================================================================
*/
namespace OptionsPanelHelpers
{
    void setupTitle(juce::Component& parent,
                    juce::Label& label,
                    const juce::String& text);

    void setupLabel(juce::Component& parent,
                    juce::Label& label,
                    const juce::String& text);

    void setupButton(juce::Component& parent,
                     juce::TextButton& button,
                     const juce::String& text);

    void setupHorizontalSlider(juce::Component& parent,
                               juce::Slider& slider,
                               double min,
                               double max,
                               double interval,
                               double defaultValue);
}