//
// Créé par Chris BAKASHIKA (2026)
//

/*
//==============================================================================
   KeyboardComponent.h

   Clavier MIDI interactif (Vue). Affiche un MidiKeyboardComponent et émet
   les notes jouées via onNotePressed.

//==============================================================================
*/

#pragma once


#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>


class KeyboardComponent : public juce::Component, public juce::MidiKeyboardStateListener
{
public:
    // Construit le clavier visuel et s'abonne à ownKeyboardState pour détecter les clics
    KeyboardComponent(juce::MidiKeyboardState& audioState);
    ~KeyboardComponent() override;

    void resized() override;

    // Callback déclenché quand une note est jouée sur le clavier.
    std::function<void(int)> onNotePressed;


private:

    // Déclenche le son (audioKeyboardState) et le callback onNotePressed pour une note pressée
    void handleNoteOn(juce::MidiKeyboardState*, int midiChannel,
                      int midiNoteNumber, float velocity) override;
    // Relâche le son correspondant sur audioKeyboardState
    void handleNoteOff(juce::MidiKeyboardState*, int midiChannel,
                       int midiNoteNumber, float velocity) override;

    /*
        État MIDI propre à ce clavier visuel, utilisé UNIQUEMENT en
        lecture : le seul état auquel on s'abonne comme Listener

        Seul un vrai clic sur une
        touche peut donc déclencher handleNoteOn()/handleNoteOff()

    */
    juce::MidiKeyboardState ownKeyboardState;

    /*
        État MIDI du plug-in (PluginProcessor::keyboardState), utilisé
        UNIQUEMENT en écriture, pour déclencher le son via SimpleSynth
        quand on clique une touche (voir handleNoteOn/handleNoteOff)

        On ne s'y abonne jamais pas comme Listener

        impossible qu'un événement MIDI y arrivant de l'extérieur déclenche quoi que ce soit
    */
    juce::MidiKeyboardState& audioKeyboardState;

    juce::MidiKeyboardComponent midiKeyboard;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeyboardComponent)
};