#pragma once

#include <array>
#include <functional>
#include <vector>
#include <juce_gui_basics/juce_gui_basics.h>

/*
    VoiceWorkspace

    Zone centrale qui affiche uniquement les contrepoints.
    Le cantus firmus reste dans la colonne de gauche.
*/
class VoiceWorkspace : public juce::Component
{
public:
    enum class CostSliderTarget
    {
        // Paramètre s de steps1(s).
        melodyMovement = 0,

        // Paramètre s de steps2(s).
        intervalColour,

        // Paramètre s de harmo(s).
        perfectIntervals
    };

    VoiceWorkspace()
    {
        setupCounterpointSelectors();
        setupCostSliders();
    }

    ~VoiceWorkspace() override
    {
        for (auto& selector : speciesSelectors)
            selector.setLookAndFeel(nullptr);

        for (auto& selector : typeSelectors)
            selector.setLookAndFeel(nullptr);

        for (auto& selector : steps1ShapeSelectors)
            selector.setLookAndFeel(nullptr);

        for (auto& selector : steps2ShapeSelectors)
            selector.setLookAndFeel(nullptr);

        for (auto& selector : harmoShapeSelectors)
            selector.setLookAndFeel(nullptr);
    }

    // Appelé quand l'espèce d'un contrepoint change.
    std::function<void(int counterpointIndex, int species)> onSpeciesChanged;

    // Appelé quand le type d'un contrepoint change.
    std::function<void(int counterpointIndex, int type)> onTypeChanged;

    // Appelé quand un slider de coût change.
    std::function<void(int counterpointIndex, CostSliderTarget target, double value)> onCostSliderChanged;

    // Appelé quand une shape ou un de ses 4 points de contrôle change.
    std::function<void(int counterpointIndex,
                       CostSliderTarget target,
                       int shapeId,
                       const std::vector<double>& values)> onShapeChanged;

    /*
        Remet les menus des contrepoints dans l'état de lancement :
        Species 1, Type 0, sliders à 0, aucun contrepoint actif.
    */
    void resetCounterpointSelectors()
    {
        for (int counterpointIndex = 0; counterpointIndex < maxCounterpoints; ++counterpointIndex)
        {
            speciesSelectors[counterpointIndex].setSelectedId(1, juce::dontSendNotification);
            typeSelectors[counterpointIndex].setSelectedId(typeToComboBoxId(0), juce::dontSendNotification);

            clearShapeSelector(steps1ShapeSelectors[counterpointIndex]);
            clearShapeSelector(steps2ShapeSelectors[counterpointIndex]);
            clearShapeSelector(harmoShapeSelectors[counterpointIndex]);
        }

        resetCostSliders();
        resetShapeControls();

        setActiveCounterpointCount(0);
        resized();
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
            drawCounterpointTitle(g,
                                  getCounterpointTitleBounds(counterpointAreas[counterpointIndex]),
                                  "Counterpoint " + juce::String(counterpointIndex + 1),
                                  counterpointIndex < activeCounterpointCount);
        }
    }

    void resized() override
    {
        updateCounterpointAreas();
        layoutCounterpointSelectors();
        layoutCostSliders();
    }

private:
    /*
        LookAndFeel compact pour les petites ComboBox du workspace.
    */
    //--> Réalisé avec l'aide de ChatGPT
    class CompactComboBoxLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        juce::Font getComboBoxFont(juce::ComboBox&) override
        {
            return juce::Font(juce::FontOptions(8.2f, juce::Font::plain));
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

            const float arrowCentreX = (float) width - 7.0f;
            const float arrowCentreY = (float) height * 0.52f;
            const float arrowSize = 2.6f;

            juce::Path arrow;
            arrow.startNewSubPath(arrowCentreX - arrowSize, arrowCentreY - 1.0f);
            arrow.lineTo(arrowCentreX, arrowCentreY + 2.0f);
            arrow.lineTo(arrowCentreX + arrowSize, arrowCentreY - 1.0f);

            g.setColour(box.findColour(juce::ComboBox::arrowColourId));
            g.strokePath(arrow, juce::PathStrokeType(1.2f));
        }
    };

    static constexpr int maxCounterpoints = 3;
    static constexpr int shapeControlCount = 4;
    static constexpr int outerPaddingX = 4;
    static constexpr int outerPaddingY = 4;
    static constexpr int minGapBetweenCounterpoints = 10;
    static constexpr int maxGapBetweenCounterpoints = 18;

    int activeCounterpointCount = 0;

    CompactComboBoxLookAndFeel compactComboBoxLookAndFeel;

    // Zone verticale complète réservée à chaque contrepoint.
    std::array<juce::Rectangle<int>, maxCounterpoints> counterpointAreas;

    // Menus Species / Type.
    std::array<juce::ComboBox, maxCounterpoints> speciesSelectors;
    std::array<juce::ComboBox, maxCounterpoints> typeSelectors;

    // Sliders visuels des trois fonctions de coûts.
    std::array<juce::Label, maxCounterpoints> steps1Labels;
    std::array<juce::Label, maxCounterpoints> steps2Labels;
    std::array<juce::Label, maxCounterpoints> harmoLabels;

    std::array<juce::Label, maxCounterpoints> steps1LeftReferenceLabels;
    std::array<juce::Label, maxCounterpoints> steps1RightReferenceLabels;
    std::array<juce::Label, maxCounterpoints> steps2LeftReferenceLabels;
    std::array<juce::Label, maxCounterpoints> steps2RightReferenceLabels;
    std::array<juce::Label, maxCounterpoints> harmoLeftReferenceLabels;
    std::array<juce::Label, maxCounterpoints> harmoRightReferenceLabels;

    std::array<juce::Slider, maxCounterpoints> steps1Sliders;
    std::array<juce::Slider, maxCounterpoints> steps2Sliders;
    std::array<juce::Slider, maxCounterpoints> harmoSliders;

    // Menus de shape placés à droite de chaque fonction de coût.
    std::array<juce::ComboBox, maxCounterpoints> steps1ShapeSelectors;
    std::array<juce::ComboBox, maxCounterpoints> steps2ShapeSelectors;
    std::array<juce::ComboBox, maxCounterpoints> harmoShapeSelectors;

    // 4 mini-sliders qui rendent visible la shape choisie pour chaque paramètre.
    using ShapeControlSliders = std::array<juce::Slider, shapeControlCount>;
    using ShapeControlLabels = std::array<juce::Label, shapeControlCount>;
    std::array<ShapeControlSliders, maxCounterpoints> steps1ShapeControls;
    std::array<ShapeControlSliders, maxCounterpoints> steps2ShapeControls;
    std::array<ShapeControlSliders, maxCounterpoints> harmoShapeControls;
    std::array<ShapeControlLabels, maxCounterpoints> steps1ShapeControlLabels;
    std::array<ShapeControlLabels, maxCounterpoints> steps2ShapeControlLabels;
    std::array<ShapeControlLabels, maxCounterpoints> harmoShapeControlLabels;

    /*
        Découpe la zone centrale en 3 espaces égaux.
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
        Prépare les deux menus de chaque contrepoint.
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
        Crée les sliders des trois fonctions de coûts.

        Chaque colonne de contrepoint possède ses propres valeurs :
        - Melody movement  -> steps1(s)
        - Interval colour  -> steps2(s)
        - Perfect intervals -> harmo(s)
    */
    void setupCostSliders()
    {
        for (int counterpointIndex = 0; counterpointIndex < maxCounterpoints; ++counterpointIndex)
        {
            setupSmallCostSlider(steps1Labels[counterpointIndex],
                                 steps1Sliders[counterpointIndex],
                                 steps1ShapeSelectors[counterpointIndex],
                                 steps1LeftReferenceLabels[counterpointIndex],
                                 steps1RightReferenceLabels[counterpointIndex],
                                 "Melody moves",
                                 "Smooth",
                                 "Jumpy");

            setupSmallCostSlider(steps2Labels[counterpointIndex],
                                 steps2Sliders[counterpointIndex],
                                 steps2ShapeSelectors[counterpointIndex],
                                 steps2LeftReferenceLabels[counterpointIndex],
                                 steps2RightReferenceLabels[counterpointIndex],
                                 "Interval colour",
                                 "Avoid disso.",
                                 "Avoid cons.");

            setupSmallCostSlider(harmoLabels[counterpointIndex],
                                 harmoSliders[counterpointIndex],
                                 harmoShapeSelectors[counterpointIndex],
                                 harmoLeftReferenceLabels[counterpointIndex],
                                 harmoRightReferenceLabels[counterpointIndex],
                                 "Perfect int.",
                                 "Avoid oct.",
                                 "Avoid fifths");

            auto sendSliderValue = [this, counterpointIndex](CostSliderTarget target, juce::Slider& slider)
            {
                if (onCostSliderChanged)
                    onCostSliderChanged(counterpointIndex, target, slider.getValue());
            };

            steps1Sliders[counterpointIndex].onValueChange = [this, counterpointIndex, sendSliderValue]()
            {
                sendSliderValue(CostSliderTarget::melodyMovement,
                                steps1Sliders[counterpointIndex]);
            };

            steps2Sliders[counterpointIndex].onValueChange = [this, counterpointIndex, sendSliderValue]()
            {
                sendSliderValue(CostSliderTarget::intervalColour,
                                steps2Sliders[counterpointIndex]);
            };

            harmoSliders[counterpointIndex].onValueChange = [this, counterpointIndex, sendSliderValue]()
            {
                sendSliderValue(CostSliderTarget::perfectIntervals,
                                harmoSliders[counterpointIndex]);
            };

            setupShapeControls(counterpointIndex,
                               CostSliderTarget::melodyMovement,
                               steps1ShapeSelectors[counterpointIndex]);

            setupShapeControls(counterpointIndex,
                               CostSliderTarget::intervalColour,
                               steps2ShapeSelectors[counterpointIndex]);

            setupShapeControls(counterpointIndex,
                               CostSliderTarget::perfectIntervals,
                               harmoShapeSelectors[counterpointIndex]);
        }

        updateCounterpointSelectorStates();
    }

    /*
        Prépare les 4 mini-sliders associés à une shape.
        Ils représentent les 4 parties de la ligne musicale.
    */
    void setupShapeControls(int counterpointIndex,
                            CostSliderTarget target,
                            juce::ComboBox& shapeSelector)
    {
        auto& controls = getShapeControls(counterpointIndex, target);
        auto& labels = getShapeControlLabels(counterpointIndex, target);

        for (int controlIndex = 0; controlIndex < shapeControlCount; ++controlIndex)
        {
            auto& label = labels[controlIndex];
            auto& slider = controls[controlIndex];

            label.setText("part" + juce::String(controlIndex + 1),
                          juce::dontSendNotification);
            label.setFont(juce::Font(juce::FontOptions(7.8f, juce::Font::bold)));
            label.setJustificationType(juce::Justification::centredLeft);
            label.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.9f));
            label.setVisible(false);

            slider.setRange(0.0, 1.0, 0.01);
            slider.setValue(0.0, juce::dontSendNotification);
            slider.setSliderStyle(juce::Slider::LinearHorizontal);
            slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            slider.setColour(juce::Slider::trackColourId, juce::Colour(0xff2f4f4f));
            slider.setColour(juce::Slider::thumbColourId, juce::Colours::white);
            slider.setVisible(false);

            slider.onValueChange = [this, counterpointIndex, target]()
            {
                sendShapeToModel(counterpointIndex, target);
            };

            addChildComponent(label);
            addChildComponent(slider);
        }

        auto* selectorToRead = &shapeSelector;

        shapeSelector.onChange = [this, counterpointIndex, target, selectorToRead]()
        {
            applyShapeChoice(counterpointIndex, target, selectorToRead->getSelectedId());
        };
    }

    /*
        Applique les valeurs de base d'une shape aux 4 mini-sliders.
        Ces valeurs restent ensuite modifiables à la main.
    */
    void applyShapeChoice(int counterpointIndex, CostSliderTarget target, int shapeId)
    {
        auto& controls = getShapeControls(counterpointIndex, target);
        auto& labels = getShapeControlLabels(counterpointIndex, target);
        const auto values = getDefaultShapeControlValues(shapeId);
        for (int controlIndex = 0; controlIndex < shapeControlCount; ++controlIndex)
        {
            labels[controlIndex].setVisible(true);
            controls[controlIndex].setVisible(true);
            controls[controlIndex].setValue(values[controlIndex], juce::dontSendNotification);
        }

        sendShapeToModel(counterpointIndex, target);
        resized();
    }

    /*
        Envoie au modèle la shape personnalisée affichée dans l'interface.
    */
    void sendShapeToModel(int counterpointIndex, CostSliderTarget target)
    {
        const int shapeId = getShapeSelector(counterpointIndex, target).getSelectedId();
        auto& controls = getShapeControls(counterpointIndex, target);

        std::vector<double> values;
        values.reserve(shapeControlCount);

        for (auto& control : controls)
            values.push_back(control.getValue());

        if (onShapeChanged)
            onShapeChanged(counterpointIndex, target, shapeId, values);
    }

    /*
        Retourne les 4 mini-sliders liés à un paramètre précis.
    */
    ShapeControlSliders& getShapeControls(int counterpointIndex, CostSliderTarget target)
    {
        switch (target)
        {
            case CostSliderTarget::melodyMovement:
                return steps1ShapeControls[counterpointIndex];

            case CostSliderTarget::intervalColour:
                return steps2ShapeControls[counterpointIndex];

            case CostSliderTarget::perfectIntervals:
                return harmoShapeControls[counterpointIndex];
        }

        return steps1ShapeControls[counterpointIndex];
    }

    /*
        Retourne la ComboBox Shape liée à un paramètre précis.
    */
    juce::ComboBox& getShapeSelector(int counterpointIndex, CostSliderTarget target)
    {
        switch (target)
        {
            case CostSliderTarget::melodyMovement:
                return steps1ShapeSelectors[counterpointIndex];

            case CostSliderTarget::intervalColour:
                return steps2ShapeSelectors[counterpointIndex];

            case CostSliderTarget::perfectIntervals:
                return harmoShapeSelectors[counterpointIndex];
        }

        return steps1ShapeSelectors[counterpointIndex];
    }

    /*
        Vide vraiment une ComboBox Shape.
        setSelectedId(0) ne suffit pas toujours à retirer l'item affiché.
    */
    void clearShapeSelector(juce::ComboBox& shapeSelector)
    {
        shapeSelector.setSelectedItemIndex(-1, juce::dontSendNotification);
        shapeSelector.setTextWhenNothingSelected("Shape");
    }

    /*
        Retourne les labels part1..part4 liés à un paramètre précis.
    */
    ShapeControlLabels& getShapeControlLabels(int counterpointIndex, CostSliderTarget target)
    {
        switch (target)
        {
            case CostSliderTarget::melodyMovement:
                return steps1ShapeControlLabels[counterpointIndex];

            case CostSliderTarget::intervalColour:
                return steps2ShapeControlLabels[counterpointIndex];

            case CostSliderTarget::perfectIntervals:
                return harmoShapeControlLabels[counterpointIndex];
        }

        return steps1ShapeControlLabels[counterpointIndex];
    }

    /*
        Donne les 4 valeurs de départ pour la shape choisie.
        L'utilisateur peut ensuite modifier ces valeurs librement.
    */
    std::array<double, shapeControlCount> getDefaultShapeControlValues(int shapeId) const
    {
        switch (shapeId)
        {
            case 1: return { 0.0, 0.0, 0.0, 0.0 }; // Fixed
            case 2: return { 0.0, 0.33, 0.67, 1.0 }; // Linear
            case 3: return { 1.0, 0.67, 0.33, 0.0 }; // Linear desc
            case 4: return { 0.0, 0.67, 0.67, 0.0 }; // Inverted V
            case 5: return { 1.0, 0.33, 0.33, 1.0 }; // V
            case 6: return { 0.0, 1.0, 0.0, 1.0 }; // M
            case 7: return { 0.0, 0.0, 1.0, 1.0 }; // Step
            case 8: return { 1.0, 1.0, 0.0, 0.0 }; // Step desc
            default: return { 0.0, 0.0, 0.0, 0.0 };
        }
    }

    /*
        Remet tous les sliders à zéro lors du bouton Clear.
        dontSendNotification évite de relancer les callbacks inutilement.
    */
    void resetCostSliders()
    {
        for (int counterpointIndex = 0; counterpointIndex < maxCounterpoints; ++counterpointIndex)
        {
            steps1Sliders[counterpointIndex].setValue(0.0, juce::dontSendNotification);
            steps2Sliders[counterpointIndex].setValue(0.0, juce::dontSendNotification);
            harmoSliders[counterpointIndex].setValue(0.0, juce::dontSendNotification);
        }
    }

    /*
        Cache les 4 mini-sliders de shape et remet leurs valeurs à zéro.
        Le modèle est vidé séparément par OptionsPanel quand on clique Clear.
    */
    void resetShapeControls()
    {
        for (int counterpointIndex = 0; counterpointIndex < maxCounterpoints; ++counterpointIndex)
        {
            resetShapeControlGroup(steps1ShapeControls[counterpointIndex]);
            resetShapeControlGroup(steps2ShapeControls[counterpointIndex]);
            resetShapeControlGroup(harmoShapeControls[counterpointIndex]);
            resetShapeLabelGroup(steps1ShapeControlLabels[counterpointIndex]);
            resetShapeLabelGroup(steps2ShapeControlLabels[counterpointIndex]);
            resetShapeLabelGroup(harmoShapeControlLabels[counterpointIndex]);
        }
    }

    /*
        Réinitialise les 4 valeurs d'une shape précise.
    */
    void resetShapeControlGroup(ShapeControlSliders& controls)
    {
        for (auto& control : controls)
        {
            control.setVisible(false);
            control.setValue(0.0, juce::dontSendNotification);
        }
    }

    /*
        Cache les textes part1..part4 d'une shape précise.
    */
    void resetShapeLabelGroup(ShapeControlLabels& labels)
    {
        for (auto& label : labels)
            label.setVisible(false);
    }

    /*
        Configure une ligne de paramètre : nom, slider, shape et repères.
    */
    void setupSmallCostSlider(juce::Label& label,
                              juce::Slider& slider,
                              juce::ComboBox& shapeSelector,
                              juce::Label& leftReferenceLabel,
                              juce::Label& rightReferenceLabel,
                              const juce::String& text,
                              const juce::String& leftReference,
                              const juce::String& rightReference)
    {
        label.setText(text, juce::dontSendNotification);
        label.setFont(juce::Font(juce::FontOptions(9.2f, juce::Font::bold)));
        label.setJustificationType(juce::Justification::centredLeft);
        label.setColour(juce::Label::textColourId, juce::Colours::white);
        label.setMinimumHorizontalScale(0.70f);

        slider.setRange(0.0, 1.0, 0.01);
        slider.setValue(0.0, juce::dontSendNotification);
        slider.setSliderStyle(juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider.setColour(juce::Slider::trackColourId, juce::Colour(0xff2f4f4f));
        slider.setColour(juce::Slider::thumbColourId, juce::Colours::white);

        setupShapeSelector(shapeSelector);
        setupReferenceLabel(leftReferenceLabel, leftReference, juce::Justification::centredLeft);
        setupReferenceLabel(rightReferenceLabel, rightReference, juce::Justification::centredRight);

        addAndMakeVisible(label);
        addAndMakeVisible(slider);
        addAndMakeVisible(shapeSelector);
        addAndMakeVisible(leftReferenceLabel);
        addAndMakeVisible(rightReferenceLabel);
    }

    /*
        Prépare les petits textes qui indiquent les extrémités du slider.
    */
    void setupReferenceLabel(juce::Label& referenceLabel,
                             const juce::String& text,
                             juce::Justification justification)
    {
        referenceLabel.setText(text, juce::dontSendNotification);
        referenceLabel.setFont(juce::Font(juce::FontOptions(7.2f, juce::Font::bold)));
        referenceLabel.setJustificationType(justification);
        referenceLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.9f));
        referenceLabel.setMinimumHorizontalScale(0.70f);
    }

    /*
        Prépare le petit menu qui choisit la shape appliquée au slider.
    */
    void setupShapeSelector(juce::ComboBox& shapeSelector)
    {
        shapeSelector.addItem("Fixed", 1);
        shapeSelector.addItem("Linear", 2);
        shapeSelector.addItem("Linear desc", 3);
        shapeSelector.addItem("Inverted V", 4);
        shapeSelector.addItem("V", 5);
        shapeSelector.addItem("M", 6);
        shapeSelector.addItem("Step", 7);
        shapeSelector.addItem("Step desc", 8);
        shapeSelector.setTextWhenNothingSelected("Shape");
        styleSelector(shapeSelector);
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
                .withMinWidth(26.0f)
                .withMargin(juce::FlexItem::Margin(0.0f, 2.0f, 0.0f, 0.0f)));

            selectorRow.items.add(juce::FlexItem(typeSelectors[counterpointIndex])
                .withFlex(1.0f)
                .withMinWidth(26.0f)
                .withMargin(juce::FlexItem::Margin(0.0f, 0.0f, 0.0f, 2.0f)));

            selectorRow.performLayout(controlsRow);
        }
    }

    /*
        Place les petits sliders sous les menus Species/Type.
    */
    void layoutCostSliders()
    {
        for (int counterpointIndex = 0; counterpointIndex < maxCounterpoints; ++counterpointIndex)
        {
            auto area = getCostSlidersBounds(counterpointAreas[counterpointIndex]);

            layoutCostParameter(area,
                                counterpointIndex,
                                CostSliderTarget::melodyMovement,
                                steps1Labels[counterpointIndex],
                                steps1Sliders[counterpointIndex],
                                steps1ShapeSelectors[counterpointIndex],
                                steps1LeftReferenceLabels[counterpointIndex],
                                steps1RightReferenceLabels[counterpointIndex]);

            layoutCostParameter(area,
                                counterpointIndex,
                                CostSliderTarget::intervalColour,
                                steps2Labels[counterpointIndex],
                                steps2Sliders[counterpointIndex],
                                steps2ShapeSelectors[counterpointIndex],
                                steps2LeftReferenceLabels[counterpointIndex],
                                steps2RightReferenceLabels[counterpointIndex]);

            layoutCostParameter(area,
                                counterpointIndex,
                                CostSliderTarget::perfectIntervals,
                                harmoLabels[counterpointIndex],
                                harmoSliders[counterpointIndex],
                                harmoShapeSelectors[counterpointIndex],
                                harmoLeftReferenceLabels[counterpointIndex],
                                harmoRightReferenceLabels[counterpointIndex]);
        }
    }

    /*
        Place un paramètre complet.
        La première ligne contient le slider principal.
        La deuxième ligne apparaît seulement quand une shape est choisie.
    */
    void layoutCostParameter(juce::Rectangle<int>& area,
                             int counterpointIndex,
                             CostSliderTarget target,
                             juce::Label& label,
                             juce::Slider& slider,
                             juce::ComboBox& shapeSelector,
                             juce::Label& leftReferenceLabel,
                             juce::Label& rightReferenceLabel)
    {
        layoutSmallCostSlider(area.removeFromTop(17),
                              label,
                              slider,
                              shapeSelector,
                              leftReferenceLabel,
                              rightReferenceLabel);

        area.removeFromTop(1);
        layoutShapeControls(area.removeFromTop(30),
                            getShapeControls(counterpointIndex, target),
                            getShapeControlLabels(counterpointIndex, target));
        area.removeFromTop(1);
    }

    /*
        Place une ligne compacte : label, slider, puis choix de shape.
    */
    void layoutSmallCostSlider(juce::Rectangle<int> row,
                               juce::Label& label,
                               juce::Slider& slider,
                               juce::ComboBox& shapeSelector,
                               juce::Label& leftReferenceLabel,
                               juce::Label& rightReferenceLabel)
    {
        auto labelArea = row.removeFromLeft(56);
        row.removeFromLeft(2);

        auto shapeBox = row.removeFromRight(40);
        row.removeFromRight(3);

        auto sliderArea = row;
        auto referenceArea = sliderArea.removeFromTop(8);

        label.setBounds(labelArea.withY(sliderArea.getY()).withHeight(sliderArea.getHeight()));
        slider.setBounds(sliderArea.reduced(0, 1));
        shapeSelector.setBounds(shapeBox.withHeight(14).reduced(0, 1));
        leftReferenceLabel.setBounds(referenceArea.removeFromLeft(referenceArea.getWidth() / 2));
        rightReferenceLabel.setBounds(referenceArea);
    }

    /*
        Place les 4 sliders de shape l'un sous l'autre.
    */
    void layoutShapeControls(juce::Rectangle<int> row,
                             ShapeControlSliders& controls,
                             ShapeControlLabels& labels)
    {
        row = row.withTrimmedLeft(17).withTrimmedRight(82);

        for (int controlIndex = 0; controlIndex < shapeControlCount; ++controlIndex)
        {
            auto line = row.removeFromTop(7);
            auto labelArea = line.removeFromLeft(25);
            line.removeFromLeft(2);

            labels[controlIndex].setBounds(labelArea);
            controls[controlIndex].setBounds(line.reduced(0, 1));
        }
    }

    /*
        Active uniquement les menus et sliders des contrepoints utilisés.
    */
    void updateCounterpointSelectorStates()
    {
        for (int counterpointIndex = 0; counterpointIndex < maxCounterpoints; ++counterpointIndex)
        {
            const bool isActive = counterpointIndex < activeCounterpointCount;
            const float alpha = isActive ? 1.0f : 0.35f;

            speciesSelectors[counterpointIndex].setEnabled(isActive);
            typeSelectors[counterpointIndex].setEnabled(isActive);

            speciesSelectors[counterpointIndex].setAlpha(alpha);
            typeSelectors[counterpointIndex].setAlpha(alpha);

            steps1Labels[counterpointIndex].setAlpha(alpha);
            steps2Labels[counterpointIndex].setAlpha(alpha);
            harmoLabels[counterpointIndex].setAlpha(alpha);

            steps1LeftReferenceLabels[counterpointIndex].setAlpha(alpha);
            steps1RightReferenceLabels[counterpointIndex].setAlpha(alpha);
            steps2LeftReferenceLabels[counterpointIndex].setAlpha(alpha);
            steps2RightReferenceLabels[counterpointIndex].setAlpha(alpha);
            harmoLeftReferenceLabels[counterpointIndex].setAlpha(alpha);
            harmoRightReferenceLabels[counterpointIndex].setAlpha(alpha);

            steps1Sliders[counterpointIndex].setEnabled(isActive);
            steps2Sliders[counterpointIndex].setEnabled(isActive);
            harmoSliders[counterpointIndex].setEnabled(isActive);

            steps1Sliders[counterpointIndex].setAlpha(alpha);
            steps2Sliders[counterpointIndex].setAlpha(alpha);
            harmoSliders[counterpointIndex].setAlpha(alpha);

            steps1ShapeSelectors[counterpointIndex].setEnabled(isActive);
            steps2ShapeSelectors[counterpointIndex].setEnabled(isActive);
            harmoShapeSelectors[counterpointIndex].setEnabled(isActive);

            steps1ShapeSelectors[counterpointIndex].setAlpha(alpha);
            steps2ShapeSelectors[counterpointIndex].setAlpha(alpha);
            harmoShapeSelectors[counterpointIndex].setAlpha(alpha);

            setShapeControlVisibility(steps1ShapeControls[counterpointIndex],
                                      steps1ShapeControlLabels[counterpointIndex]);
            setShapeControlVisibility(steps2ShapeControls[counterpointIndex],
                                      steps2ShapeControlLabels[counterpointIndex]);
            setShapeControlVisibility(harmoShapeControls[counterpointIndex],
                                      harmoShapeControlLabels[counterpointIndex]);

            updateShapeControlState(steps1ShapeControls[counterpointIndex], isActive, alpha);
            updateShapeControlState(steps2ShapeControls[counterpointIndex], isActive, alpha);
            updateShapeControlState(harmoShapeControls[counterpointIndex], isActive, alpha);
            updateShapeLabelState(steps1ShapeControlLabels[counterpointIndex], alpha);
            updateShapeLabelState(steps2ShapeControlLabels[counterpointIndex], alpha);
            updateShapeLabelState(harmoShapeControlLabels[counterpointIndex], alpha);
        }
    }

    /*
        Les contrôles de shape restent visibles pour toutes les voix.
        Les voix non sélectionnées sont seulement grisées et désactivées.
    */
    void setShapeControlVisibility(ShapeControlSliders& controls,
                                   ShapeControlLabels& labels)
    {
        for (int controlIndex = 0; controlIndex < shapeControlCount; ++controlIndex)
        {
            controls[controlIndex].setVisible(true);
            labels[controlIndex].setVisible(true);
        }
    }

    /*
        Active ou grise les 4 mini-sliders d'une shape.
    */
    void updateShapeControlState(ShapeControlSliders& controls,
                                 bool isActive,
                                 float alpha)
    {
        for (auto& control : controls)
        {
            control.setEnabled(isActive);
            control.setAlpha(alpha);
        }
    }

    /*
        Grise les labels part1..part4 avec le même alpha que leur contrepoint.
    */
    void updateShapeLabelState(ShapeControlLabels& labels, float alpha)
    {
        for (auto& label : labels)
            label.setAlpha(alpha);
    }

    /*
        Calcule le bandeau de titre d'un contrepoint.
    */
    juce::Rectangle<int> getCounterpointTitleBounds(juce::Rectangle<int> counterpointArea) const
    {
        const int titleBoxWidth = juce::jlimit(56,
                                               78,
                                               static_cast<int>(counterpointArea.getWidth() * 0.38f));
        const int titleBoxHeight = 14;

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

        const int controlsWidth = juce::jlimit(70, 104, counterpointArea.getWidth() - 42);
        const int controlsHeight = 13;
        const int controlsY = titleBox.getBottom() + 8;

        return {
            counterpointArea.getCentreX() - controlsWidth / 2,
            controlsY,
            controlsWidth,
            controlsHeight
        };
    }

    /*
        Calcule la petite zone réservée aux sliders.
    */
    juce::Rectangle<int> getCostSlidersBounds(juce::Rectangle<int> counterpointArea) const
    {
        auto controlsBox = getCounterpointControlsBounds(counterpointArea);

        const int slidersWidth = juce::jlimit(168, 224, counterpointArea.getWidth());
        const int slidersHeight = 144;
        const int slidersY = controlsBox.getBottom() + 8;

        return {
            counterpointArea.getCentreX() - slidersWidth / 2,
            slidersY,
            slidersWidth,
            slidersHeight
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
        if (titleBox.isEmpty())
            return;

        const auto bounds = titleBox.toFloat().reduced(0.5f);
        const auto backgroundColour = isActive
                                    ? juce::Colour(0xff2f4f4f)
                                    : juce::Colour(0xff3e3e3e);

        g.setColour(backgroundColour);
        g.fillRoundedRectangle(bounds, 5.0f);

        g.setColour(juce::Colours::white.withAlpha(0.9f));
        g.drawRoundedRectangle(bounds, 5.0f, 0.9f);

        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(juce::FontOptions(7.2f, juce::Font::bold)));
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VoiceWorkspace)
};
