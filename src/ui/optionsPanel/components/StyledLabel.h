#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * Label de paramètre avec fond et texte adaptatif.
 *
 * Le texte utilise la plus grande taille possible.
 * Si la largeur manque, il peut passer sur plusieurs lignes.
 */
class StyledLabel : public juce::Label
{
public:
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        g.setColour(juce::Colour(0xff2f4f4f));
        g.fillRoundedRectangle(bounds, 6.0f);

        auto textBounds = getLocalBounds().reduced(horizontalPadding, verticalPadding);
        auto layout = buildLayoutThatFits(textBounds);

        g.setColour(juce::Colours::white);
        g.setFont(layout.font);

        g.drawFittedText(getText(), textBounds, getJustificationType(), layout.lineCount);
    }

private:
    struct TextLayout
    {
        juce::Font font;
        int lineCount = 1;
    };

    static constexpr int horizontalPadding = 5;
    static constexpr int verticalPadding = 2;
    static constexpr int minimumTextLines = 1;
    static constexpr int maximumTextLines = 3;
    static constexpr float maximumFontHeight = 13.0f;
    static constexpr float minimumFontHeight = 8.0f;

    /*
        Cherche la plus grande police possible pour afficher tout le label.

        La recherche teste plusieurs nombres de lignes. Cela permet de garder
        un texte grand quand la case est large, puis de passer sur plusieurs
        lignes quand la fenêtre devient plus étroite.
    */
    TextLayout buildLayoutThatFits(juce::Rectangle<int> textBounds) const
    {
        const int forcedLineCount = getForcedLineCount();

        if (forcedLineCount > 1)
            return { getLargestFontForLineCount(textBounds, forcedLineCount),
                     forcedLineCount };

        TextLayout best { getFont().withHeight(minimumFontHeight), maximumTextLines };

        for (int lines = minimumTextLines; lines <= maximumTextLines; ++lines)
        {
            auto font = getLargestFontForLineCount(textBounds, lines);

            if (font.getHeight() > best.font.getHeight())
                best = { font, lines };
        }

        return best;
    }

    /*
        Réduit progressivement la police jusqu'à ce que le texte tienne
        dans le nombre de lignes autorisé.
    */
    juce::Font getLargestFontForLineCount(juce::Rectangle<int> textBounds,
                                          int lineCount) const
    {
        auto font = getFont().withHeight(maximumFontHeight);

        while (font.getHeight() > minimumFontHeight
            && !textFits(font, textBounds, lineCount))
        {
            font.setHeight(font.getHeight() - 0.5f);
        }

        return font;
    }

    /*
        Vérifie que le texte tient à la fois en largeur et en hauteur.
    */
    bool textFits(const juce::Font& font,
                  juce::Rectangle<int> textBounds,
                  int lineCount) const
    {
        const bool hasForcedLineBreak = getText().containsChar('\n');
        const float textWidth = hasForcedLineBreak
            ? getWidestLineWidth(font)
            : juce::GlyphArrangement::getStringWidth(font, getText());

        const float availableWidth = hasForcedLineBreak
            ? static_cast<float>(textBounds.getWidth())
            : static_cast<float>(textBounds.getWidth() * lineCount);

        const float availableHeight = static_cast<float>(textBounds.getHeight());
        const float requiredHeight = font.getHeight() * static_cast<float>(lineCount);

        return textWidth <= availableWidth
            && requiredHeight <= availableHeight;
    }

    /*
        Compte les lignes déjà imposées par le texte.
        Exemple : "Avoid Repeated\nNotes" doit rester sur deux lignes.
    */
    int getForcedLineCount() const
    {
        if (! getText().containsChar('\n'))
            return 1;

        juce::StringArray lines;
        lines.addLines(getText());

        return juce::jlimit(minimumTextLines, maximumTextLines, lines.size());
    }

    /*
        Mesure la ligne la plus longue.
        Cela permet de garder une police plus grande quand le label est déjà
        coupé proprement sur plusieurs lignes.
    */
    float getWidestLineWidth(const juce::Font& font) const
    {
        juce::StringArray lines;
        lines.addLines(getText());

        float widestLine = 0.0f;

        for (const auto& line : lines)
            widestLine = juce::jmax(widestLine,
                                    juce::GlyphArrangement::getStringWidth(font, line));

        return widestLine;
    }
};
