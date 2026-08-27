// PluginEditor.h
#pragma once
//
// Créé par Chris BAKASHIKA (2026)
//

/*
//==============================================================================
   PluginEditor.h

   Fenêtre affichée par l'hôte (GarageBand, etc.) lors de l'ouverture du
   plug-in. Se contente d'intégrer le MainComponent, en lui transmettant
   l'AppController et le MidiKeyboardState détenus par le PluginProcessor.

   Recréée à chaque ouverture de la fenêtre du plug-in, contrairement au
   PluginProcessor qui persiste tant que le plug-in reste chargé.
//==============================================================================
*/

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "../ui/MainComponent.h"

class PluginEditor : public juce::AudioProcessorEditor
{
public:
    explicit PluginEditor(PluginProcessor& p)
    : juce::AudioProcessorEditor(&p),
      mainComponent(p.getAppController(), p.getKeyboardState())
    {
        addAndMakeVisible(mainComponent);
        setResizable(false, false);
        setResizeLimits(960, 420, 960, 420);
        setSize(960, 420);
    }

    void resized() override
    {
        mainComponent.setBounds(getLocalBounds());
    }

private:
    MainComponent mainComponent;
};