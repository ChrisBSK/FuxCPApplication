// PluginProcessor.h
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "../controller/AppController.h"
#include "../audio/synth/SimpleSynth.h"

#include <juce_audio_utils/juce_audio_utils.h>

class PluginProcessor : public juce::AudioProcessor
{
public:
    PluginProcessor();
    ~PluginProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "FuxCP"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}

    // Accès pour le PluginEditor : c'est ici que vivent
    // le contrôleur et l'état du clavier, pas dans la fenêtre.
    AppController& getAppController() { return appController; }
    juce::MidiKeyboardState& getKeyboardState() { return keyboardState; }

private:
    AppController appController;
    juce::MidiKeyboardState keyboardState;
    SimpleSynth synth;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
};