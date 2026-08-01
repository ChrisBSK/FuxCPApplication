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
    level = velocity * 0.22f;
    tailOff = 0.0f;
    noteAgeSeconds = 0.0;

    baseFrequency = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    angleDelta = baseFrequency * 2.0 * juce::MathConstants<double>::pi / getSampleRate();
}

// =============================
// Arrêt de note
// =============================
void SimpleVoice::stopNote(float, bool allowTailOff)
{
    if (allowTailOff)
    {
        // Le noteOff lance une courte extinction au lieu de laisser la note bloquée.
        tailOff = 1.0f;
    }
    else
    {
        angleDelta = 0.0;
        tailOff = 0.0f;
        noteAgeSeconds = 0.0;
        baseFrequency = 0.0;
        clearCurrentNote();
    }
}

// =============================
// Génération audio
// =============================
void SimpleVoice::renderNextBlock(juce::AudioBuffer<float>& buffer,
                                 int startSample, int numSamples)
{
    if (angleDelta == 0.0)
        return;

    const double sampleRate = getSampleRate();
    const double secondsPerSample = sampleRate > 0.0 ? 1.0 / sampleRate : 0.0;

    for (int i = 0; i < numSamples; ++i)
    {
        /*
            Timbre piano classique sans sample :
            - attaque très courte, comme le marteau sur la corde,
            - fondamentale ronde,
            - harmoniques légèrement désaccordées pour éviter le son "synthé",
            - aigus qui disparaissent plus vite que le corps de la note.
        */
        const float attack = static_cast<float>(juce::jlimit(0.0, 1.0, noteAgeSeconds / 0.004));

        const float noteHeight = static_cast<float>(juce::jlimit(0.0, 1.0, (baseFrequency - 80.0) / 900.0));
        const float bodyDecay = 0.45f + noteHeight * 0.85f;
        const float body = static_cast<float>(std::exp(-noteAgeSeconds * bodyDecay));
        const float brightness = static_cast<float>(std::exp(-noteAgeSeconds * 7.0));
        const float hammer = static_cast<float>(std::exp(-noteAgeSeconds * 45.0));

        const float fundamental = std::sin(currentAngle) * 0.95f;
        const float harmonic2 = std::sin(currentAngle * 2.002) * 0.34f * brightness;
        const float harmonic3 = std::sin(currentAngle * 3.010) * 0.18f * brightness;
        const float harmonic4 = std::sin(currentAngle * 4.025) * 0.09f * brightness;
        const float harmonic5 = std::sin(currentAngle * 5.045) * 0.04f * brightness;

        const float hammerClick = std::sin(currentAngle * 9.0) * 0.035f * hammer;

        float sample = (fundamental + harmonic2 + harmonic3 + harmonic4 + harmonic5 + hammerClick)
                       * level
                       * attack
                       * body;

        // Petite saturation douce : évite les pics secs et rend l'attaque plus ronde.
        sample = std::tanh(sample * 1.25f) * 0.85f;

        if (tailOff > 0.0f)
            sample *= tailOff;

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            buffer.addSample(ch, startSample, sample);

        currentAngle += angleDelta;
        noteAgeSeconds += secondsPerSample;
        ++startSample;

        if (tailOff > 0.0f)
        {
            tailOff *= 0.99f;

            // Quand l'extinction devient inaudible, la voix est libérée.
            if (tailOff <= 0.005f)
            {
                angleDelta = 0.0;
                tailOff = 0.0f;
                noteAgeSeconds = 0.0;
                baseFrequency = 0.0;
                clearCurrentNote();
                break;
            }
        }
        else if (body <= 0.001f)
        {
            angleDelta = 0.0;
            noteAgeSeconds = 0.0;
            baseFrequency = 0.0;
            clearCurrentNote();
            break;
        }
    }
}


// =============================
// SimpleSynth : conteneur global
// =============================


SimpleSynth::SimpleSynth()
{
    for (int i = 0; i < 8; ++i)
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
