//
// Créé par Chris BAKASHIKA (2026)
//

/*
//==============================================================================
   main.cpp

   Point d'entrée de la version Standalone de l'application (hors plug-in
   AU).

   Définit la fenêtre principale (MainWindow) qui héberge le
   MainComponent, ainsi que le cycle de vie JUCE (Application).
//==============================================================================
*/

#include <juce_gui_basics/juce_gui_basics.h>
#include "MainComponent.h"

namespace
{
    constexpr int fixedWindowWidth = 960;
    constexpr int fixedWindowHeight = 420;
}

class MainWindow : public juce::DocumentWindow
{
public:
    MainWindow() 
        : DocumentWindow("Fuxophone",
                         juce::Colours::darkgrey,
                         DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar(true);

        setContentOwned(new MainComponent(), true);
        setResizable(false, false);
        setResizeLimits(fixedWindowWidth,
                        fixedWindowHeight,
                        fixedWindowWidth,
                        fixedWindowHeight);
        setSize(fixedWindowWidth, fixedWindowHeight);

        centreWithSize(fixedWindowWidth, fixedWindowHeight);
        setVisible(true);
    }

    void closeButtonPressed() override
    {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }
};

class Application : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "FuxCP"; }
    const juce::String getApplicationVersion() override { return "1.0"; }

    void initialise(const juce::String&) override
    {
        mainWindow.reset(new MainWindow());
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(Application)
