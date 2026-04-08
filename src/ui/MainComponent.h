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
class MainComponent : public juce::Component
{
public:
    MainComponent();

    ~MainComponent();
    /*
    ~MainComponent() override = default;*/


    void paint(juce::Graphics&) override;
    void resized() override;

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill);

    void prepareToPlay(int, double sampleRate);


private:


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
};

