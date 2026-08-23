#include "KeyboardComponent.h"

KeyboardComponent::KeyboardComponent(juce::MidiKeyboardState& audioState)
    : audioKeyboardState(audioState),
      midiKeyboard(ownKeyboardState,
        juce::MidiKeyboardComponent::Orientation::horizontalKeyboard)
{
    addAndMakeVisible(midiKeyboard);

    midiKeyboard.setAvailableRange(24, 108); //piano standard en général
    //midiKeyboard.setAvailableRange(0, 127); // plusieurs notes inutilisables muscialement don inutile
    midiKeyboard.setKeyWidth(30.0f);

    midiKeyboard.setScrollButtonsVisible(true);

    /*
        Seul le clavier interne doit recevoir les clics.

        Si le composant parent dépasse visuellement autour du clavier,
        cette zone vide laisse passer la souris aux autres composants.
    */
    setInterceptsMouseClicks(false, true);

    setWantsKeyboardFocus(true);

    // On s'abonne uniquement à ownKeyboardState : jamais à
    // audioKeyboardState, qui ne sert qu'en écriture (voir handleNoteOn).
    ownKeyboardState.addListener(this);
}

/*
    Se désinscrit de ownKeyboardState avant sa propre destruction.

    ownKeyboardState est un membre de cette classe : il partage exactement
    le même cycle de vie que KeyboardComponent, donc ce désenregistrement
    n'est plus strictement nécessaire pour éviter un crash (contrairement
    à l'ancienne version, qui s'abonnait à un état externe survivant à la
    fenêtre). On le garde par prudence et par symétrie avec addListener().
*/
KeyboardComponent::~KeyboardComponent()
{
    ownKeyboardState.removeListener(this);
}

// =============================
// Callback des notes jouées
// =============================

/*
    Ce callback ne peut être déclenché QUE par ownKeyboardState (voir
    addListener ci-dessus), donc uniquement par un vrai clic sur ce
    clavier - jamais par le MIDI reçu par le plug-in, puisqu'on ne s'abonne
    jamais à audioKeyboardState. Peu importe le thread ou le mécanisme que
    l'hôte utilise pour envoyer ce MIDI : il ne peut simplement pas
    atteindre ce code.
*/
void KeyboardComponent::handleNoteOn(juce::MidiKeyboardState*, int midiChannel,
                                     int midiNoteNumber, float velocity)
{
    // Ajoute la note au Cantus Firmus.
    if (onNotePressed)
        onNotePressed(midiNoteNumber);

    /*
        Déclenche aussi le son : on écrit directement dans l'état audio du
        plug-in, sans jamais s'y être abonné comme Listener.

        SimpleSynth entendra la note au prochain bloc audio (PluginProcessor::processBlock)
    */
    audioKeyboardState.noteOn(midiChannel, midiNoteNumber, velocity);
}

/*
    Relâche la note déclenchée dans audioKeyboardState par handleNoteOn(),
    pour que le son ne reste pas tenu indéfiniment après avoir relâché la
    touche.
*/
void KeyboardComponent::handleNoteOff(juce::MidiKeyboardState*, int midiChannel,
                                      int midiNoteNumber, float velocity)
{
    audioKeyboardState.noteOff(midiChannel, midiNoteNumber, velocity);
}

void KeyboardComponent::resized()
{
    auto width = getWidth();
    int numKeys = 108 - 24 + 1;

    float keyWidth = width / (float)numKeys;

    midiKeyboard.setKeyWidth(keyWidth);
    auto area = getLocalBounds();

    // largeur réelle du clavier
    float totalWidth = midiKeyboard.getTotalKeyboardWidth();

    // centre horizontalement
    int x = (area.getWidth() - totalWidth) / 2;

    midiKeyboard.setBounds(x, 0, totalWidth, area.getHeight());
}
