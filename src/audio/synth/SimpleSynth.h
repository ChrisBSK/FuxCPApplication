#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

class SimpleSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

class SimpleVoice : public juce::SynthesiserVoice
{
public:
    bool canPlaySound (juce::SynthesiserSound*) override;

    void startNote (int midiNoteNumber, float velocity,
                    juce::SynthesiserSound*, int) override;

    void stopNote (float velocity, bool allowTailOff) override;

    void renderNextBlock (juce::AudioBuffer<float>&, int, int) override;

    void pitchWheelMoved(int);

    void controllerMoved(int, int);

private:
    double currentAngle = 0.0;
    double angleDelta = 0.0;
    float level = 0.0f;
    float tailOff = 0.0f;
};

// 🔥 wrapper propre
class SimpleSynth
{
public:
    SimpleSynth();

    void prepare(double sampleRate);
    void render(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi);



private:
    juce::Synthesiser synth;

    double currentAngle = 0.0;
    double angleDelta = 0.0;
    float level = 0.0f;
    float tailOff = 0.0f;
};