//
// Créé par Chris BAKASHIKA (2026)
//

/*
//==============================================================================
   AboutPage.h

   Page de présentation de l'application. Utilise le même principe que la
   page Solver : un titre simple et un texte déroulant pour garder la
   lecture confortable.
//==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class AboutPage : public juce::Component
{
public:
    AboutPage();

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    /*
        Contenu déroulant de la page About.

        On le dessine nous-mêmes pour pouvoir mettre certains mots en gras,
        ce que TextEditor ne permet pas simplement sur une seule partie du texte.
    */
    class AboutTextContent : public juce::Component
    {
    public:
        int getRequiredHeight(int width);

        void paint(juce::Graphics& g) override;
        void resized() override;

    private:
        void rebuildTextLayout(int width);

        juce::TextLayout textLayout;
        int lastLayoutWidth = 0;
    };

    // Titre affiché en haut de la page.
    juce::Label titleLabel;

    // Zone déroulante contenant le texte de présentation.
    juce::Viewport aboutViewport;
    AboutTextContent aboutContent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AboutPage)
};
