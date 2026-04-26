#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../../../controller/AppController.h"
/**
 * Représente UNE voix (CF ou contrepoint)
 * → contient espèces + type
 */
class VoiceBox : public juce::Component
{
public:
    VoiceBox(const juce::String& name);

    void paint(juce::Graphics&) override;
    void resized() override;

    bool isActive = false;

    juce::ComboBox speciesBox;
    juce::ComboBox typeBox;


    AppController::VoiceSettings* settings = nullptr;

private:
    juce::Label title;
};