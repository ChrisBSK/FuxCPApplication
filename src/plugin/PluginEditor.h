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