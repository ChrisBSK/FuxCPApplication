//
// Créé par Chris BAKASHIKA (2026)
//

/*
//==============================================================================
   ClickableTitle.h

   Label cliquable : expose des callbacks pour le clic et le survol
   (onClick, onEnter, onExit), utilisé pour les titres interactifs de
   l'interface.

   (beaucoup utilisé pour l'ancienne version de l'interface, pour les colonnes
   de contrepoint)
//==============================================================================
*/

#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

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