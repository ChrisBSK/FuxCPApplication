//
// Créé par Chris BAKASHIKA (2026)
//

/*
//==============================================================================
   PluginProcessor.cpp

   Implémente le rendu audio du clavier virtuel (processBlock), la création
   du PluginEditor, ainsi que createPluginFilter(), le point d'entrée que
   JUCE appelle pour instancier le plug-in.
//==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

// Construit le processeur avec une seule sortie stéréo
PluginProcessor::PluginProcessor()
    : juce::AudioProcessor(BusesProperties()
          .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

// Initialise le synthé avec le taux d'échantillonnage fourni par l'hôte
void PluginProcessor::prepareToPlay(double sampleRate, int)
{
    synth.prepare(sampleRate);
}

// Récupère les notes jouées au clavier et génère le bloc audio correspondant
void PluginProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    keyboardState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);
    synth.render(buffer, midiMessages);
}

// Instancie la fenêtre du plug-in
juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor(*this);
}

// Point d'entrée que JUCE appelle pour instancier le plug-in
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}