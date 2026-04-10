#include "SimpleSynth.h"
#include <cmath>

// =============================
// SimpleVoice : une voix du synthé
// =============================
bool SimpleVoice::canPlaySound(juce::SynthesiserSound*)
{
    return true;
}

// =============================
// Démarrage de note
// =============================
void SimpleVoice::startNote(int midiNoteNumber, float velocity,
                           juce::SynthesiserSound*, int)
{
    level = velocity * 0.2f;
    tailOff = 0.0;

    auto freq = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    angleDelta = freq * 2.0 * juce::MathConstants<double>::pi / getSampleRate();




}

// =============================
// Arrêt de note
// =============================
void SimpleVoice::stopNote(float, bool allowTailOff)
{
    if (allowTailOff)
        tailOff = 1.0f;
    else
        clearCurrentNote();
}

// =============================
// Génération audio
// =============================
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


// =============================
// SimpleSynth : conteneur global
// =============================


SimpleSynth::SimpleSynth()
{
    for (int i = 0; i < 4; ++i)
        synth.addVoice(new SimpleVoice());

    synth.addSound(new SimpleSound());
}


// =============================
// Inititalisation AUDIO
// =============================
void SimpleSynth::prepare(double sampleRate)
{
    synth.setCurrentPlaybackSampleRate(sampleRate);
}

// =============================
// Rendu AUDIO global
// =============================
void SimpleSynth::render(juce::AudioBuffer<float>& buffer,
                        juce::MidiBuffer& midi)
{
    synth.renderNextBlock(buffer, midi, 0, buffer.getNumSamples());
}

// =============================
// CALLBACKS MIDI (non utilisés pour l'instant)
// =============================
void SimpleVoice::pitchWheelMoved(int)
{
    // pas utilisé pour l'instant
}

void SimpleVoice::controllerMoved(int, int)
{
    // pas utilisé pour l'instant
}