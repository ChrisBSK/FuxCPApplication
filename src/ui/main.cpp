#include <juce_gui_extra/juce_gui_extra.h>
#include "MainComponent.h"

class MainWindow : public juce::DocumentWindow
{
public:
    MainWindow()
        : juce::DocumentWindow ("myproject",
                                juce::Colours::lightgrey,
                                juce::DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar (true);
        setContentOwned (new MainComponent(), true);

        setResizable (true, true);
        setResizeLimits (420, 260, 1000, 800);

        centreWithSize (getWidth(), getHeight());
        setVisible (true);
    }

    void closeButtonPressed() override
    {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }
};

class MyApp : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override    { return "myproject"; }
    const juce::String getApplicationVersion() override { return "0.1.0"; }

    void initialise (const juce::String&) override { mainWindow.reset (new MainWindow()); }
    void shutdown() override { mainWindow = nullptr; }

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION (MyApp)
