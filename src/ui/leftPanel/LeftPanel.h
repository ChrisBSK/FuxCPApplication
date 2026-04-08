#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

#include "../../service/GenerationService.h"

class AppController;

class LeftPanel : public juce::Component
{
public:
    LeftPanel(AppController& controller);
    ~LeftPanel() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    void showAlert(juce::AlertWindow::AlertIconType icon,
                   const juce::String& title,
                   const juce::String& message);

private:
    juce::TextEditor text;
    juce::Label label;

    juce::ComboBox voices;
    juce::Label labelVoices;

    juce::TextButton moreOptions;
    juce::TextButton generateButton;

    GenerationService generationService;
    juce::File midiOutFileToGenerate;

    juce::OwnedArray<juce::ComboBox> speciesBoxes;
    juce::OwnedArray<juce::Label> speciesLabels;

    juce::OwnedArray<juce::ComboBox> typeBoxes;

    void updateVoiceSpeciesUI(int numVoices);
    void prepareOutputFile();
 juce::Label speciesHeader;
    juce::Label typeHeader;

    AppController& appController;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LeftPanel)
};