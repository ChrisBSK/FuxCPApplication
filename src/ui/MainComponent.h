#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "footer/FooterPanel.h"
#include "header/HeaderPanel.h"
#include "history/HistoryPanel.h"
#include "keyboard/Keyboard.h"
#include "leftPanel/LeftPanel.h"
#include "WorkArea/WorkAreaPanel.h"
#include "optionsWindow/OptionsPanel.h"


class MainComponent : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:



    HeaderPanel header;
    LeftPanel leftPanel;
    WorkAreaPanel workArea;
    KeyboardPanel keyboard;
    HistoryPanel history;
    FooterPanel footer;
    OptionsPanel optionsPanel;
};

