#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include "../audio/synth/SimpleSynth.h"

class AudioPlayer : public juce::AudioSource
{
public:
    AudioPlayer(juce::MidiKeyboardState& state)
        : keyboardState(state)
    {}

    void prepareToPlay(int, double sampleRate) override
    {
        synth.prepare(sampleRate);
    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        auto* buffer = bufferToFill.buffer;
        buffer->clear();

        juce::MidiBuffer midi;

        keyboardState.processNextMidiBuffer(
            midi,
            bufferToFill.startSample,
            bufferToFill.numSamples,
            true
        );

        synth.render(*buffer, midi);
    }

    void releaseResources() override {}

private:
    juce::MidiKeyboardState& keyboardState;
    SimpleSynth synth;
};