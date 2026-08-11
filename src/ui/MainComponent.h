#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>

// ===== UI =====
#include "header/HeaderPanel.h"
#include "keyboard/KeyboardComponent.h"
#include "leftPanel/LeftPanel.h"
#include "optionsPanel/OptionsPanel.h"
#include "pages/AboutPage.h"
#include "pages/SolverExplanationPage.h"

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
    //MainComponent(); #Version standalone

    MainComponent(AppController& controllerToUse, juce::MidiKeyboardState& keyboardStateToUse); // # Version AU

    ~MainComponent() override;

    // ===== UI =====
    void paint(juce::Graphics&) override;
    void paintOverChildren(juce::Graphics& g) override;
    void resized() override;


    // ===== Audio =====
    // ====== Etape 1 Construction Plug-in ======
    /*
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);
    void prepareToPlay(int, double sampleRate);
    */



private:
    /*
        Page simple utilisée pour les onglets qui n'ont pas encore
        d'interface détaillée.

        Elle permet de créer les pages secondaires sans mélanger leur futur contenu
        avec le code de la page principale.
    */
    class SimplePage : public juce::Component
    {
    public:
        SimplePage(const juce::String& titleText,
                   const juce::String& bodyText);

        void paint(juce::Graphics& g) override;
        void resized() override;

    private:
        juce::Label title;
        juce::Label body;
    };

    enum class CurrentPage
    {
        mainScreen,
        savedSolutions,
        solver,
        about
    };

    // =========================
    // CONTROLLER
    // =========================

    // ====== Etape 1 Construction Plug-in ======
    //AppController appController;     #Version Standalone

    AppController& appController; // MainComponent.h reçoit les dépendances au lieu de les posséder


    // =========================
    // UI
    // =========================
    HeaderPanel header;
    LeftPanel leftPanel { appController };
    OptionsPanel optionsPanel;
    SimplePage savedSolutionsPage {
        juce::String::fromUTF8("Saved configurations"),
        juce::String::fromUTF8("Cette page servira à retrouver les configurations sauvegardées.")
    };
    SolverExplanationPage solverExplanationPage;
    AboutPage aboutPage;

    CurrentPage currentPage = CurrentPage::mainScreen;

    // =========================
    // MIDI / KEYBOARD
    // =========================

    // ====== Etape 1 Construction Plug-in ======
    //juce::MidiKeyboardState keyboardState;    # Version Standalone
    juce::MidiKeyboardState& keyboardState;

    KeyboardComponent keyboard { keyboardState };

    // =========================
    // AUDIO
    // =========================
    // ====== Etape 1 Construction Plug-in ======
        // supression de l'audio dans le main, sachant que la fenêtre peut disparaitre

    /*SimpleSynth synth;
    AudioPlayer audioPlayer { keyboardState };
    juce::AudioDeviceManager deviceManager;
    juce::AudioSourcePlayer player;

    */



    // =========================
    // UI Helpers
    // =========================
    std::unique_ptr<juce::TooltipWindow> tooltipWindow;

    int defaultVoiceCount = 0;

    // Change la page visible quand l'utilisateur clique sur un onglet.
    void showPage(CurrentPage page);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
