//
// Créé par Chris BAKASHIKA (2026)
//

/*
//==============================================================================
   AudioPlayer.h

   Pont entre le clavier virtuel (MidiKeyboardState) et le SimpleSynth.

   Implémente l'interface juce::AudioSource : récupère les notes jouées
   sur le clavier sous forme MIDI, puis les transmet au synthétiseur pour
   produire le son entendu par l'utilisateur.
//==============================================================================
*/

#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include "../audio/synth/SimpleSynth.h"

class AudioPlayer : public juce::AudioSource
{
public:
    // Construit le lecteur en le liant à l'état du clavier virtuel.
    AudioPlayer(juce::MidiKeyboardState& state)
        : keyboardState(state)
    {}

    // Initialise le synthétiseur avec le taux d'échantillonnage utilisé.
    void prepareToPlay(int, double sampleRate) override
    {
        synth.prepare(sampleRate);
    }

    // Récupère les notes jouées au clavier et génère le bloc audio correspondant.
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

    // Aucune ressource à libérer pour ce lecteur
    void releaseResources() override {}

private:
    juce::MidiKeyboardState& keyboardState;
    SimpleSynth synth;
};