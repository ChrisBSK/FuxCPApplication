// PluginEditor.h
#pragma once
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
        setResizeLimits(900, 420, 900, 420);
        setSize(900, 420);
    }

    void resized() override
    {
        mainComponent.setBounds(getLocalBounds());
    }

private:
    MainComponent mainComponent;
};