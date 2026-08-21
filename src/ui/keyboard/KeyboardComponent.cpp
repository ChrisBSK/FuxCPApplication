#include "KeyboardComponent.h"

KeyboardComponent::KeyboardComponent(juce::MidiKeyboardState& state)
    : keyboardState(state),
      midiKeyboard(state,
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

    state.addListener(this);
}

/*
    Se désinscrit de keyboardState avant sa propre destruction.

    Indispensable : keyboardState survit à cette fenêtre (il vit dans
    PluginProcessor). Sans ce désenregistrement, un pointeur mort resterait
    dans sa liste de Listener, et le thread audio temps réel pourrait
    l'appeler à la prochaine note MIDI reçue - exactement le crash observé
    (EXC_BAD_ACCESS sur com.apple.audio.IOThread.client).
*/
KeyboardComponent::~KeyboardComponent()
{
    keyboardState.removeListener(this);
}

// =============================
// Callback des notes jouées
// =============================
void KeyboardComponent::handleNoteOn(juce::MidiKeyboardState*, int,
                                     int midiNoteNumber, float)
{
//     /*
//         keyboardState est partagé avec le vrai flux MIDI du plug-in
//
//
//         un clic sur le clavier - > toujours lieu sur le thread graphique (message thread)
//
//         le MIDI reçu par le plug-in est traité sur le thread audio - > temps réel, à l'intérieur de processBlock()

//         Seul un vrai clic remplit le Cantus Firmus
//
//     */
    if (! juce::MessageManager::getInstance()->isThisTheMessageThread())
        return;

    if (onNotePressed)
        onNotePressed(midiNoteNumber);
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
