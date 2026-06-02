#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * Label cliquable (hover + click)
 */
class ClickableTitle : public juce::Label
{
public:
    // Callbacks d'interaction utilisateur.
    std::function<void()> onClick;
    std::function<void()> onEnter;
    std::function<void()> onExit;


    // Déclenche le callback de clic.
    void mouseDown(const juce::MouseEvent&) override
    {
        if (onClick) onClick();
    }
    // Notifie l'entrée de la souris.
    void mouseEnter(const juce::MouseEvent&) override
    {
        if (onEnter) onEnter();
    }
    // Notifie la sortie de la souris.
    void mouseExit(const juce::MouseEvent&) override
    {
        if (onExit) onExit();
    }
};