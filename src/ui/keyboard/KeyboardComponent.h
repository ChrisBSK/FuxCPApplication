#pragma once


#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>

/**
 * @brief Composant clavier MIDI interactif (Vue).
 *
 * Rôle :
 * - Affiche un clavier piano (MidiKeyboardComponent)
 * - Permet de jouer des notes via l’interface
 * - Émet les notes jouées via un callback (onNotePressed)
 *
 * Responsabilités :
 * - Gérer l’interaction utilisateur (clics sur le clavier)
 * - Convertir ces interactions en événements MIDI simples
 *
 * Ne contient PAS :
 * - de logique audio (synthèse)
 * - de traitement métier
 */
class KeyboardComponent : public juce::Component, public juce::MidiKeyboardStateListener
{
public:
    KeyboardComponent(juce::MidiKeyboardState& audioState);
    ~KeyboardComponent() override;

    void resized() override;

    std::function<void(int)> onNotePressed;


private:
    void handleNoteOn(juce::MidiKeyboardState*, int midiChannel,
                      int midiNoteNumber, float velocity) override;

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
        quand on clique une touche (voir handleNoteOn/handleNoteOff).

        On ne s'y abonne jamais pas comme Listener

        impossible qu'un événement MIDI y arrivant de l'extérieur déclenche quoi que ce soit
    */
    juce::MidiKeyboardState& audioKeyboardState;

    juce::MidiKeyboardComponent midiKeyboard;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeyboardComponent)
};