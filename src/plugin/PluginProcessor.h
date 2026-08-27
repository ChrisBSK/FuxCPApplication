//
// Créé par Chris BAKASHIKA (2026)
//

/*
//==============================================================================
   PluginProcessor.h

   Point d'entrée du plug-in (juce::AudioProcessor). Détient l'AppController
   et le MidiKeyboardState, partagés avec le PluginEditor.
//==============================================================================
*/

#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "../controller/AppController.h"
#include "../audio/synth/SimpleSynth.h"

#include <juce_audio_utils/juce_audio_utils.h>

class PluginProcessor : public juce::AudioProcessor
{
public:
    // Construit le processeur (initialise l'AppController et le synthé interne)
    PluginProcessor();
    ~PluginProcessor() override = default;

    // Initialise le synthé avec le taux d'échantillonnage de l'hôte
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;

    // Aucune ressource à libérer
    void releaseResources() override {}

    // Traite un bloc audio/MIDI (rendu du clavier virtuel)
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    // Crée la fenêtre du plug-in (PluginEditor)
    juce::AudioProcessorEditor* createEditor() override;

    // Indique que ce plug-in a une interface graphique.
    bool hasEditor() const override { return true; }

    // Nom du plug-in affiché par l'hôte.
    const juce::String getName() const override { return "FuxCP"; }

    // Le plug-in accepte le MIDI entrant (clavier virtuel).
    bool acceptsMidi() const override { return true; }
    // Le plug-in ne génère pas de MIDI en sortie.
    bool producesMidi() const override { return false; }
    // Pas de traînée audio après l'arrêt d'une note.
    double getTailLengthSeconds() const override { return 0.0; }


    // Un seul programme, non utilisé par ce plug-in.
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    // Sauvegarde/restauration de l'état par l'hôte : non utilisées ici.
    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}

    // Accès pour le PluginEditor : c'est ici que vivent
    // le contrôleur et l'état du clavier, pas dans la fenêtre.
    AppController& getAppController() { return appController; }
    juce::MidiKeyboardState& getKeyboardState() { return keyboardState; }

private:
    AppController appController; // Contrôleur principal, partagé avec le PluginEditor
    juce::MidiKeyboardState keyboardState; // État du clavier virtuel, partagé avec le PluginEditor
    SimpleSynth synth; // Synthé utilisé pour rendre l'audio du clavier virtuel

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
};