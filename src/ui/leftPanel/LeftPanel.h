#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../../service/GenerationService.h"
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include "../../model/GenerationInput.h"

class GenerationService;

class LeftPanel : public juce::Component
{
public:


    LeftPanel();
    ~LeftPanel() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    void showAlert(juce::AlertWindow::AlertIconType icon, const juce::String &title, const juce::String &message);

    void showWarning(const juce::String &message);


    // ===== LOGIQUE PRINCIPALE =====
    bool collectAndValidateInput(GenerationInput &input, juce::String &error);

    // ===== ÉTAPES =====
    void prepareOutputFile();
    bool startGeneration(const std::vector<int>& cantusFirmus,
                        int species,
                        int voiceCount,
                        const juce::String& outputPath);


    // ===== UI =====

    void showError(const juce::String& message);
    void showSuccessMessage();
    void showFailureMessage(const juce::String& error);

    void setStatusMessage(const juce::String &message, juce::Colour colour);

private:

    juce::TextEditor text;
    juce::Label label;

    juce::ComboBox voices;
    juce::Label labelVoices;

    juce::ComboBox species;
    juce::Label labelSpecies;

    juce::TextButton moreOptions;

    juce::TextButton generateButton;

    juce:: PopupMenu m;

    GenerationService generationService;


    juce::File midiOutFileToGenerate;





    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LeftPanel)
};