#pragma once

#include <array>
#include <functional>
#include <juce_gui_basics/juce_gui_basics.h>

/*
    VoiceWorkspace

    Zone centrale qui affiche les contrepoints.

    Le cantus firmus n'est pas affiché ici :
    on affiche uniquement CP 1, CP 2 et CP 3.

    Plus tard, les paramètres et sliders seront ajoutés
    directement sous le bandeau de chaque contrepoint.
*/
class VoiceWorkspace : public juce::Component
{
public:
    VoiceWorkspace()
    {
        setupCounterpointSelectors();
    }

    ~VoiceWorkspace() override
    {
        for (auto& selector : speciesSelectors)
            selector.setLookAndFeel(nullptr);

        for (auto& selector : typeSelectors)
            selector.setLookAndFeel(nullptr);
    }

    // Appelé quand l'espèce d'un contrepoint change.
    std::function<void(int counterpointIndex, int species)> onSpeciesChanged;

    // Appelé quand le type d'un contrepoint change.
    std::function<void(int counterpointIndex, int type)> onTypeChanged;

    /*
        Remet les menus des contrepoints dans l'état de lancement :
        Species 1, Type 0, aucun contrepoint actif.
    */
    void resetCounterpointSelectors()
    {
        for (int counterpointIndex = 0; counterpointIndex < maxCounterpoints; ++counterpointIndex)
        {
            speciesSelectors[counterpointIndex].setSelectedId(1, juce::dontSendNotification);
            typeSelectors[counterpointIndex].setSelectedId(typeToComboBoxId(0), juce::dontSendNotification);
        }

        setActiveCounterpointCount(0);
    }

    /*
        Met à jour le nombre de contrepoints actifs.
        Exemple : 4 voix au total = 3 contrepoints actifs.
    */
    void setActiveCounterpointCount(int newActiveCounterpointCount)
    {
        activeCounterpointCount = juce::jlimit(0,
                                               maxCounterpoints,
                                               newActiveCounterpointCount);
        updateCounterpointSelectorStates();
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        for (int counterpointIndex = 0; counterpointIndex < maxCounterpoints; ++counterpointIndex)
        {
            drawCounterpointBlock(g,
                                  counterpointAreas[counterpointIndex],
                                  "Counterpoint " + juce::String(counterpointIndex + 1),
                                  counterpointIndex < activeCounterpointCount);
        }
    }

    void resized() override
    {
        updateCounterpointAreas();
        layoutCounterpointSelectors();
    }

private:
    /*
        LookAndFeel compact pour les petites ComboBox du workspace.
        Il réduit la police et réserve moins de place à la flèche.
    */
    class CompactComboBoxLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        juce::Font getComboBoxFont(juce::ComboBox&) override
        {
            return juce::Font(juce::FontOptions(9.0f, juce::Font::plain));
        }

        void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override
        {
            label.setBounds(4, 0, box.getWidth() - 15, box.getHeight());
            label.setFont(getComboBoxFont(box));
            label.setJustificationType(juce::Justification::centredLeft);
        }

        void drawComboBox(juce::Graphics& g,
                          int width,
                          int height,
                          bool,
                          int,
                          int,
                          int,
                          int,
                          juce::ComboBox& box) override
        {
            auto bounds = juce::Rectangle<float>(0.5f, 0.5f,
                                                 (float) width - 1.0f,
                                                 (float) height - 1.0f);

            g.setColour(box.findColour(juce::ComboBox::backgroundColourId));
            g.fillRoundedRectangle(bounds, 3.0f);

            g.setColour(box.findColour(juce::ComboBox::outlineColourId));
            g.drawRoundedRectangle(bounds, 3.0f, 0.8f);

            const float arrowCentreX = (float) width - 8.0f;
            const float arrowCentreY = (float) height * 0.52f;
            const float arrowSize = 3.0f;

            juce::Path arrow;
            arrow.startNewSubPath(arrowCentreX - arrowSize, arrowCentreY - 1.0f);
            arrow.lineTo(arrowCentreX, arrowCentreY + 2.0f);
            arrow.lineTo(arrowCentreX + arrowSize, arrowCentreY - 1.0f);

            g.setColour(box.findColour(juce::ComboBox::arrowColourId));
            g.strokePath(arrow, juce::PathStrokeType(1.2f));
        }
    };

    static constexpr int maxCounterpoints = 3;
    static constexpr int outerPaddingX = 4;
    static constexpr int outerPaddingY = 4;
    static constexpr int minGapBetweenCounterpoints = 10;
    static constexpr int maxGapBetweenCounterpoints = 18;

    int activeCounterpointCount = 0;

    CompactComboBoxLookAndFeel compactComboBoxLookAndFeel;

    // Zone verticale complète réservée à chaque contrepoint.
    std::array<juce::Rectangle<int>, maxCounterpoints> counterpointAreas;
    std::array<juce::ComboBox, maxCounterpoints> speciesSelectors;
    std::array<juce::ComboBox, maxCounterpoints> typeSelectors;

    /*
        Découpe la zone centrale en 3 espaces égaux.

        Chaque espace correspond à un contrepoint possible.
        On ne dessine rien ici : on calcule seulement les positions.
    */
    void updateCounterpointAreas()
    {
        const auto availableArea = getLocalBounds().reduced(outerPaddingX, outerPaddingY);
        const float gapBetweenCounterpoints = (float) juce::jlimit(minGapBetweenCounterpoints,
                                                                   maxGapBetweenCounterpoints,
                                                                   availableArea.getWidth() / 42);

        juce::FlexBox counterpointRow;
        counterpointRow.flexDirection = juce::FlexBox::Direction::row;
        counterpointRow.alignItems = juce::FlexBox::AlignItems::stretch;
        counterpointRow.justifyContent = juce::FlexBox::JustifyContent::spaceBetween;

        for (int counterpointIndex = 0; counterpointIndex < maxCounterpoints; ++counterpointIndex)
        {
            const float leftMargin = counterpointIndex == 0 ? 0.0f : gapBetweenCounterpoints / 2.0f;
            const float rightMargin = counterpointIndex == maxCounterpoints - 1 ? 0.0f : gapBetweenCounterpoints / 2.0f;

            counterpointRow.items.add(juce::FlexItem()
                .withFlex(1.0f)
                .withMinWidth(72.0f)
                .withMargin(juce::FlexItem::Margin(0.0f, rightMargin, 0.0f, leftMargin)));
        }

        counterpointRow.performLayout(availableArea);

        for (int counterpointIndex = 0; counterpointIndex < maxCounterpoints; ++counterpointIndex)
            counterpointAreas[counterpointIndex] =
                counterpointRow.items[counterpointIndex].currentBounds.toNearestInt();
    }

    /*
        Prépare les deux menus de chaque contrepoint :
        - espèce : 1 à 5
        - type : -3 à 2
    */
    void setupCounterpointSelectors()
    {
        for (int counterpointIndex = 0; counterpointIndex < maxCounterpoints; ++counterpointIndex)
        {
            auto& speciesSelector = speciesSelectors[counterpointIndex];
            auto& typeSelector = typeSelectors[counterpointIndex];

            for (int species = 1; species <= 5; ++species)
                speciesSelector.addItem("Species " + juce::String(species), species);

            for (int type = -3; type <= 2; ++type)
                typeSelector.addItem("Type " + juce::String(type), typeToComboBoxId(type));

            styleSelector(speciesSelector);
            styleSelector(typeSelector);

            speciesSelector.setSelectedId(1, juce::dontSendNotification);
            typeSelector.setSelectedId(typeToComboBoxId(0), juce::dontSendNotification);

            speciesSelector.onChange = [this, counterpointIndex]()
            {
                if (onSpeciesChanged)
                    onSpeciesChanged(counterpointIndex,
                                     speciesSelectors[counterpointIndex].getSelectedId());
            };

            typeSelector.onChange = [this, counterpointIndex]()
            {
                if (onTypeChanged)
                    onTypeChanged(counterpointIndex,
                                  comboBoxIdToType(typeSelectors[counterpointIndex].getSelectedId()));
            };

            addAndMakeVisible(speciesSelector);
            addAndMakeVisible(typeSelector);
        }

        updateCounterpointSelectorStates();
    }

    /*
        Applique le style compact utilisé dans le workspace.
    */
    void styleSelector(juce::ComboBox& selector)
    {
        selector.setLookAndFeel(&compactComboBoxLookAndFeel);
        selector.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff26363a));
        selector.setColour(juce::ComboBox::outlineColourId, juce::Colours::white.withAlpha(0.65f));
        selector.setColour(juce::ComboBox::textColourId, juce::Colours::white);
        selector.setColour(juce::ComboBox::arrowColourId, juce::Colours::white);
        selector.setColour(juce::ComboBox::buttonColourId, juce::Colours::transparentBlack);
        selector.setJustificationType(juce::Justification::centred);
        selector.setScrollWheelEnabled(true);
    }

    /*
        Place les deux menus juste sous le bandeau du contrepoint.
    */
    void layoutCounterpointSelectors()
    {
        for (int counterpointIndex = 0; counterpointIndex < maxCounterpoints; ++counterpointIndex)
        {
            auto controlsRow = getCounterpointControlsBounds(counterpointAreas[counterpointIndex]);

            juce::FlexBox selectorRow;
            selectorRow.flexDirection = juce::FlexBox::Direction::row;
            selectorRow.alignItems = juce::FlexBox::AlignItems::stretch;

            selectorRow.items.add(juce::FlexItem(speciesSelectors[counterpointIndex])
                .withFlex(1.0f)
                .withMinWidth(34.0f)
                .withMargin(juce::FlexItem::Margin(0.0f, 3.5f, 0.0f, 0.0f)));

            selectorRow.items.add(juce::FlexItem(typeSelectors[counterpointIndex])
                .withFlex(1.0f)
                .withMinWidth(34.0f)
                .withMargin(juce::FlexItem::Margin(0.0f, 0.0f, 0.0f, 3.5f)));

            selectorRow.performLayout(controlsRow);
        }
    }

    /*
        Active uniquement les menus des contrepoints utilisés.
    */
    void updateCounterpointSelectorStates()
    {
        for (int counterpointIndex = 0; counterpointIndex < maxCounterpoints; ++counterpointIndex)
        {
            const bool isActive = counterpointIndex < activeCounterpointCount;

            speciesSelectors[counterpointIndex].setEnabled(isActive);
            typeSelectors[counterpointIndex].setEnabled(isActive);

            speciesSelectors[counterpointIndex].setAlpha(isActive ? 1.0f : 0.35f);
            typeSelectors[counterpointIndex].setAlpha(isActive ? 1.0f : 0.35f);
        }
    }

    /*
        Dessine un contrepoint complet :
        - le bandeau de titre
        - l'espace libre où les paramètres seront ajoutés plus tard
    */
    void drawCounterpointBlock(juce::Graphics& g,
                               juce::Rectangle<int> counterpointArea,
                               const juce::String& title,
                               bool isActive)
    {
        if (counterpointArea.isEmpty())
            return;

        drawCounterpointTitle(g,
                              getCounterpointTitleBounds(counterpointArea),
                              title,
                              isActive);
    }

    /*
        Calcule le bandeau de titre d'un contrepoint.
    */
    juce::Rectangle<int> getCounterpointTitleBounds(juce::Rectangle<int> counterpointArea) const
    {
        const int titleBoxWidth = juce::jlimit(78,
                                               118,
                                               static_cast<int>(counterpointArea.getWidth() * 0.60f));
        const int titleBoxHeight = 22;

        return {
            counterpointArea.getCentreX() - titleBoxWidth / 2,
            counterpointArea.getY(),
            titleBoxWidth,
            titleBoxHeight
        };
    }

    /*
        Calcule la ligne des ComboBox placées sous le titre.
    */
    juce::Rectangle<int> getCounterpointControlsBounds(juce::Rectangle<int> counterpointArea) const
    {
        auto titleBox = getCounterpointTitleBounds(counterpointArea);

        const int controlsWidth = juce::jlimit(90, 138, counterpointArea.getWidth() - 26);
        const int controlsHeight = 17;
        const int controlsY = titleBox.getBottom() + 10;

        return {
            counterpointArea.getCentreX() - controlsWidth / 2,
            controlsY,
            controlsWidth,
            controlsHeight
        };
    }

    /*
        Dessine le petit bandeau contenant le nom du contrepoint.
    */
    void drawCounterpointTitle(juce::Graphics& g,
                               juce::Rectangle<int> titleBox,
                               const juce::String& title,
                               bool isActive)
    {
        const auto bounds = titleBox.toFloat().reduced(0.5f);
        const auto backgroundColour = isActive
                                    ? juce::Colour(0xff2f4f4f)
                                    : juce::Colour(0xff3e3e3e);

        g.setColour(backgroundColour);
        g.fillRoundedRectangle(bounds, 5.0f);

        g.setColour(juce::Colours::white.withAlpha(0.9f));
        g.drawRoundedRectangle(bounds, 5.0f, 0.9f);

        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(juce::FontOptions(10.5f, juce::Font::bold)));
        g.drawText(title, titleBox, juce::Justification::centred);
    }

    /*
        Les ComboBox JUCE ont besoin d'un identifiant positif.
        On décale donc les types -3..2 vers les ids 1..6.
    */
    static int typeToComboBoxId(int type)
    {
        return type + 4;
    }

    /*
        Conversion inverse : ids 1..6 vers types -3..2.
    */
    static int comboBoxIdToType(int comboBoxId)
    {
        return comboBoxId - 4;
    }

};
