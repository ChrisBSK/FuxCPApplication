#include "SimpleSynth.h"
#include <cmath>

bool SimpleVoice::canPlaySound(juce::SynthesiserSound*)
{
    return true;
}

void SimpleVoice::startNote(int midiNoteNumber, float velocity,
                           juce::SynthesiserSound*, int)
{
    level = velocity * 0.2f;
    tailOff = 0.0;

    auto freq = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    angleDelta = freq * 2.0 * juce::MathConstants<double>::pi / getSampleRate();




}

void SimpleVoice::stopNote(float, bool allowTailOff)
{
    if (allowTailOff)
        tailOff = 1.0f;
    else
        clearCurrentNote();
}

void SimpleVoice::renderNextBlock(juce::AudioBuffer<float>& buffer,
                                 int startSample, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        float sample = std::sin(currentAngle) * level;

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            buffer.addSample(ch, startSample, sample);

        currentAngle += angleDelta;
        ++startSample;
    }
}

// =======================
// SimpleSynth
// =======================

SimpleSynth::SimpleSynth()
{
    for (int i = 0; i < 4; ++i)
        synth.addVoice(new SimpleVoice());

    synth.addSound(new SimpleSound());
}

void SimpleSynth::prepare(double sampleRate)
{
    synth.setCurrentPlaybackSampleRate(sampleRate);
}

void SimpleSynth::render(juce::AudioBuffer<float>& buffer,
                        juce::MidiBuffer& midi)
{
    synth.renderNextBlock(buffer, midi, 0, buffer.getNumSamples());
}

void SimpleVoice::pitchWheelMoved(int)
{
    // pas utilisé pour l'instant
}

void SimpleVoice::controllerMoved(int, int)
{
    // pas utilisé pour l'instant
}