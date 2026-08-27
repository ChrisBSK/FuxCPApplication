//
// Créé par Chris BAKASHIKA (2026)
//

/*
//==============================================================================
   AboutPage.cpp

   Construit le texte (AttributedString) retraçant les contributions
   successives à FuxCP, avec mise en page automatique via TextLayout dans
   un viewport défilant.
//==============================================================================
*/

#include "AboutPage.h"

namespace
{
    /*
        Ajoute un paragraphe dans le texte riche.

        Les lignes vides supplémentaires donnent plus d'air entre les blocs.
    */
    void addParagraph(juce::AttributedString& text,
                      const juce::String& paragraph,
                      const juce::Font& font,
                      juce::Colour colour)
    {
        text.append(paragraph + "\n\n\n", font, colour);
    }
}

//==============================================================================
// AboutTextContent
//==============================================================================

int AboutPage::AboutTextContent::getRequiredHeight(int width)
{
    rebuildTextLayout(width);
    return static_cast<int>(std::ceil(textLayout.getHeight())) + 12;
}

void AboutPage::AboutTextContent::paint(juce::Graphics& g)
{
    rebuildTextLayout(getWidth());
    textLayout.draw(g, getLocalBounds().reduced(2, 0).toFloat());
}

void AboutPage::AboutTextContent::resized()
{
    rebuildTextLayout(getWidth());
}

void AboutPage::AboutTextContent::rebuildTextLayout(int width)
{
    if (width <= 0 || width == lastLayoutWidth)
        return;

    lastLayoutWidth = width;

    juce::AttributedString text;
    text.setJustification(juce::Justification::topLeft);
    text.setWordWrap(juce::AttributedString::WordWrap::byWord);

    const auto colour = juce::Colours::white.withAlpha(0.9f);
    const juce::Font regularFont(14.0f);
    const juce::Font boldFont(14.0f, juce::Font::bold);

    text.append(juce::String::fromUTF8("Cette application, créée par "),
                regularFont,
                colour);
    text.append(juce::String::fromUTF8("Chris BAKASHIKA"),
                boldFont,
                colour);
    text.append(juce::String::fromUTF8(", permet d'explorer la génération automatique de contrepoint à partir d'un Cantus Firmus. Elle sert à configurer les voix, les contraintes musicales, les méthodes de recherche et les préférences du solveur afin de produire des solutions musicales contrôlables et comparables.\n\n\n"),
                regularFont,
                colour);

    addParagraph(text,
                 juce::String::fromUTF8("De nombreuses contributions ont vues le jour, mais les plus intéressantes ont été amenées par Thibaut Wafflard qui a commencé à formaliser mathématiquement la théorie du contrepoint de Fux pour deux voix, permettant de démystifier certaines informations apportées par le théoricien Fux."),
                 regularFont,
                 colour);

    addParagraph(text,
                 juce::String::fromUTF8("Suite à ces avancées importantes, Anton Lamotte a lui aussi contribué aux avancées, en étendant le modèle de Thibaut Wafflard au champ du contrepoint avec 3 voix. Ceci a donc permis d'aller plus loin dans la polyphonie et ainsi créer de nouvelles possibilité dans l'enrichissiement des mélodies."),
                 regularFont,
                 colour);

    addParagraph(text,
                 juce::String::fromUTF8("Grâce à ces recherches, Luc Cleenewerk et Diego de Patoul ont pu à leurs tours apporter leurs pierres à l'édifice, en formalisant le modèle le plus avancé du contrepoint de Fux, à 4 voix."),
                 regularFont,
                 colour);

    addParagraph(text,
                juce::String::fromUTF8("Ensuite, Tom Lai a apporté des corrections à ce solveur FuxCP, afin d'affiner le respect des règles de composition de contrepoint."),
                regularFont,
                colour);

    addParagraph(text,
                 juce::String::fromUTF8("Après cela, une base solide était établie au niveau modélisation du contrepoint de Fux. S'est donc posée la question des contraintes et comment l'utilisateur peut avoir un contrôle maximal sur les informations données au solveur, dans le but d'avoir les solutions qui lui conviennent le mieux."),
                 regularFont,
                 colour);

    addParagraph(text,
                 juce::String::fromUTF8("À cet endroit précis s'est inscrit le travail de Dorian Genon qui a apporté les premières modifications concrètes des contraintes de contrepoint en créeant les concepts clés de fonctions de coûts et de shapes, introduisant ainsi les \"dynamic sliders\", une manière de modifier certaines parties précises des compositions afin d'avoir des mélodies évolutives au cours des partitions."),
                 regularFont,
                 colour);

    textLayout.createLayout(text, static_cast<float>(width - 8));
}

//==============================================================================
// AboutPage
//==============================================================================

AboutPage::AboutPage()
{
    titleLabel.setText(juce::String::fromUTF8("About"),
                       juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    titleLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    addAndMakeVisible(titleLabel);

    aboutViewport.setViewedComponent(&aboutContent, false);
    aboutViewport.setScrollBarsShown(true, false);
    addAndMakeVisible(aboutViewport);
}

void AboutPage::paint(juce::Graphics& g)
{
    auto area = getLocalBounds().reduced(22);

    g.setColour(juce::Colour(0xff3f3f3f));
    g.fillRoundedRectangle(area.toFloat(), 8.0f);

    g.setColour(juce::Colours::white.withAlpha(0.25f));
    g.drawRoundedRectangle(area.toFloat(), 8.0f, 1.0f);
}

void AboutPage::resized()
{
    auto area = getLocalBounds().reduced(42, 32);

    titleLabel.setBounds(area.removeFromTop(30));
    area.removeFromTop(12);

    aboutViewport.setBounds(area);

    const int contentWidth = juce::jmax(1, aboutViewport.getWidth() - 18);
    const int contentHeight = aboutContent.getRequiredHeight(contentWidth);
    aboutContent.setBounds(0, 0, contentWidth, contentHeight);
}
