#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// ===== UI =====
#include "footer/FooterPanel.h"
#include "header/HeaderPanel.h"
#include "history/HistoryPanel.h"
#include "keyboard/KeyboardComponent.h"
#include "leftPanel/LeftPanel.h"
#include "WorkArea/WorkAreaPanel.h"
#include "optionsPanel/OptionsPanel.h"

// ===== Core =====
#include "../controller/AppController.h"

// ===== Audio =====
#include "../audio/synth/SimpleSynth.h"
#include "../audio/AudioPlayer.h"

/**
 * MainComponent = racine de l’application
 *
 * Rôle :
 * - assemble toute l’UI
 * - connecte les composants entre eux
 * - gère l’audio (synth + MIDI)
 *
 *
 */
class MainComponent : public juce::Component,
                      public juce::DragAndDropContainer
{
public:
    MainComponent();
    ~MainComponent() override;

    // ===== UI =====
    void paint(juce::Graphics&) override;
    void paintOverChildren(juce::Graphics& g) override;
    void resized() override;

    // ===== Audio =====
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);
    void prepareToPlay(int, double sampleRate);

private:
    // =========================
    // CONTROLLER
    // =========================
    AppController appController;

    // =========================
    // UI
    // =========================
    HeaderPanel header;
    LeftPanel leftPanel { appController };
    WorkAreaPanel workArea;
    OptionsPanel optionsPanel;

    HistoryPanel history;
    FooterPanel footer;

    // =========================
    // MIDI / KEYBOARD
    // =========================
    juce::MidiKeyboardState keyboardState;
    KeyboardComponent keyboard { keyboardState };

    // =========================
    // AUDIO
    // =========================
    SimpleSynth synth;
    AudioPlayer audioPlayer { keyboardState };

    juce::AudioDeviceManager deviceManager;
    juce::AudioSourcePlayer player;

    // =========================
    // UI Helpers
    // =========================
    std::unique_ptr<juce::TooltipWindow> tooltipWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};