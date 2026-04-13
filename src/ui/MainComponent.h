#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "footer/FooterPanel.h"
#include "header/HeaderPanel.h"
#include "history/HistoryPanel.h"
#include "keyboard/KeyboardComponent.h"
#include "leftPanel/LeftPanel.h"
#include "WorkArea/WorkAreaPanel.h"
#include "optionsWindow/OptionsPanel.h"

#include "../controller/AppController.h"
#include "../audio/synth/SimpleSynth.h"
#include "../audio/AudioPlayer.h"

/**
 * @brief Composant principal de l’application (Vue globale + orchestration).
 *
 * Rôle :
 * - Assemble tous les sous-composants UI (header, leftPanel, workArea, etc.)
 * - Gère l’audio (synthé + lecture MIDI)
 * - Fait le lien entre interface utilisateur et moteur audio
 *
 * Responsabilités :
 * - Organiser le layout global de l’application
 * - Initialiser et connecter les composants (UI, contrôleur, audio)
 * - Router les événements MIDI vers le synthé
 *
 * Ne contient PAS :
 * - de logique métier (gérée par AppController / modèle)
 * - de logique de génération musicale
 */

class MainComponent : public juce::Component, public juce::DragAndDropContainer
{
public:
    MainComponent();

    ~MainComponent();
    /*
    ~MainComponent() override = default;*/


    void paint(juce::Graphics&) override;

    void paintOverChildren(juce::Graphics &g);

    void resized() override;

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill);

    void prepareToPlay(int, double sampleRate);


private:

    OptionsPanel optionsPanel;
    HeaderPanel header;
    LeftPanel leftPanel { appController };
    WorkAreaPanel workArea;

    HistoryPanel history;
    FooterPanel footer;

    juce::MidiKeyboardState keyboardState;
    KeyboardComponent keyboard;


    SimpleSynth synth;
    AppController appController;


    juce::AudioDeviceManager deviceManager;
    juce::AudioSourcePlayer player;

    AudioPlayer audioPlayer;

   // fenêtre explicative pour les contraintes dans OptionPanel
    std::unique_ptr<juce::TooltipWindow> tooltipWindow;
};

