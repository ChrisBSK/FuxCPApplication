#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * Label cliquable (hover + click)
 */
class ClickableTitle : public juce::Label
{
public:
    std::function<void()> onClick;
    std::function<void()> onEnter;
    std::function<void()> onExit;

    void mouseDown(const juce::MouseEvent&) override
    {
        if (onClick) onClick();
    }

    void mouseEnter(const juce::MouseEvent&) override
    {
        if (onEnter) onEnter();
    }

    void mouseExit(const juce::MouseEvent&) override
    {
        if (onExit) onExit();
    }
};