#pragma once

#include <array>
#include <cmath>
#include <functional>
#include <vector>
#include <juce_gui_basics/juce_gui_basics.h>

/*
    =====================================================================
    VoiceWorkspace.h — zone centrale des contrepoints

    Ce composant affiche les contrôles propres à chaque contrepoint.

    Il contient :
    - les titres Counterpoint 1, 2, 3,
    - les ComboBox Species / Type,
    - les sliders des fonctions de coût,
    - les ComboBox Shape,
    - les 5 mini-sliders part1-part5 pour visualiser/modifier une shape.

    Le Cantus Firmus n'est pas affiché ici.
    Il reste dans le LeftPanel.

    VoiceWorkspace ne modifie pas directement le modèle.
    Il envoie des callbacks à OptionsPanel, qui transmet ensuite les valeurs
    à AppController et ConstraintSettings.
    =====================================================================
*/
class VoiceWorkspace : public juce::Component
{
public:
    /*
        Identifie quelle fonction de coût est pilotée par un slider.

        Chaque valeur correspond à une fonction définie par Dorian :
        - melodyMovement   -> steps1(s)
        - intervalColour   -> steps2(s)
        - perfectIntervals -> harmo(s)
    */
    enum class CostSliderTarget
    {
        melodyMovement = 0,
        intervalColour,
        perfectIntervals
    };

    /*
        Crée les contrôles du workspace.

        Les sélecteurs Species/Type sont préparés séparément des sliders
        pour garder l'initialisation lisible.
    */
    VoiceWorkspace()
    {
        setupCounterpointSelectors();
        setupCostSliders();
    }

    /*
        Détache le LookAndFeel personnalisé des ComboBox.

        JUCE demande de retirer un LookAndFeel avant que l'objet qui le possède
        soit détruit, sinon les composants peuvent garder un pointeur invalide.
    */
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

    //==============================================================================
    // Callbacks envoyés à OptionsPanel
    //==============================================================================

    /*
        VoiceWorkspace ne modifie pas directement AppController.
        Il prévient seulement OptionsPanel quand l'utilisateur change une valeur.
    */

    // Espèce modifiée pour un contrepoint.
    std::function<void(int counterpointIndex, int species)> onSpeciesChanged;

    // Type modifié pour un contrepoint.
    std::function<void(int counterpointIndex, int type)> onTypeChanged;

    // Slider de coût modifié pour un contrepoint.
    std::function<void(int counterpointIndex, CostSliderTarget target, double value)> onCostSliderChanged;

    // Shape ou valeur part1-part5 modifiée pour un contrepoint.
    std::function<void(int counterpointIndex,
                       CostSliderTarget target,
                       int shapeId,
                       const std::vector<double>& values)> onShapeChanged;

    /*
        Remet le workspace dans son état initial.

        Cette méthode réinitialise :
        - Species à 1,
        - Type à 0,
        - les ComboBox Shape,
        - les sliders de coût,
        - les mini-sliders part1-part5,
        - le nombre de contrepoints actifs.
    */
    void resetCounterpointSelectors()
    {
        // Réinitialise les menus principaux de chaque contrepoint.
        for (int counterpointIndex = 0; counterpointIndex < maxCounterpoints; ++counterpointIndex)
        {
            speciesSelectors[counterpointIndex].setSelectedId(1, juce::dontSendNotification);
            typeSelectors[counterpointIndex].setSelectedId(typeToComboBoxId(0), juce::dontSendNotification);

            clearShapeSelector(steps1ShapeSelectors[counterpointIndex]);
            clearShapeSelector(steps2ShapeSelectors[counterpointIndex]);
            clearShapeSelector(harmoShapeSelectors[counterpointIndex]);
        }

        // Réinitialise ensuite les familles de contrôles.
        resetCostSliders();
        resetShapeControls();

        setActiveCounterpointCount(0);
        resized();
    }

    /*
        Définit combien de contrepoints sont actifs visuellement.

        Exemple :
        - 2 voix totales -> 1 contrepoint actif
        - 4 voix totales -> 3 contrepoints actifs

        Les autres colonnes restent visibles mais grisées.
    */
    void setActiveCounterpointCount(int newActiveCounterpointCount)
    {
        activeCounterpointCount = juce::jlimit(0,
                                               maxCounterpoints,
                                               newActiveCounterpointCount);

        updateCounterpointSelectorStates();
        repaint();
    }

    /*
        Dessine les titres Counterpoint 1, 2, 3.

        Le reste des contrôles est dessiné par les composants JUCE eux-mêmes.
    */
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

    /*
        Recalcule tout le layout interne du workspace.

        À chaque changement de taille, on recalcule :
        - les zones de contrepoints,
        - les ComboBox Species/Type,
        - les sliders et shapes.
    */
    void resized() override
    {
        updateCounterpointAreas();
        clampCostScrollOffset();
        layoutCounterpointSelectors();
        layoutCostSliders();
    }

    /*
        Permet de défiler dans les paramètres si leur contenu devient trop haut.

        Objectif :
        ne jamais couper les mini-sliders, même si on ajoute plus tard
        d'autres fonctions de coût dans chaque colonne.
    */
    void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails& wheel) override
    {
        const int scrollStep = 42;
        const int delta = static_cast<int>(std::round(-wheel.deltaY * (float) scrollStep));

        costScrollOffsetY += delta;
        clampCostScrollOffset();
        resized();
    }

private:
    /*
        LookAndFeel compact pour les petites ComboBox du workspace.

        Il réduit :
        - la taille du texte,
        - la taille de la flèche,
        - l'épaisseur du contour.

        Objectif : rendre Species, Type et Shape lisibles malgré leur petite taille.
    */
    class CompactComboBoxLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        // Police compacte utilisée dans toutes les petites ComboBox.
        juce::Font getComboBoxFont(juce::ComboBox&) override
        {
            return juce::Font(juce::FontOptions(9.8f, juce::Font::plain));
        }

        // Place le texte en laissant de la place à la flèche à droite.
        void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override
        {
            label.setBounds(4, 0, box.getWidth() - 15, box.getHeight());
            label.setFont(getComboBoxFont(box));
            label.setJustificationType(juce::Justification::centredLeft);
        }

        /*
            Dessine manuellement le fond, le contour et la flèche de la ComboBox.
        */
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

            // Fond et contour arrondis.
            g.setColour(box.findColour(juce::ComboBox::backgroundColourId));
            g.fillRoundedRectangle(bounds, 3.0f);

            g.setColour(box.findColour(juce::ComboBox::outlineColourId));
            g.drawRoundedRectangle(bounds, 3.0f, 0.8f);

            const float arrowCentreX = (float) width - 7.0f;
            const float arrowCentreY = (float) height * 0.52f;
            const float arrowSize = 2.6f;

            // Petite flèche dessinée à droite.
            juce::Path arrow;
            arrow.startNewSubPath(arrowCentreX - arrowSize, arrowCentreY - 1.0f);
            arrow.lineTo(arrowCentreX, arrowCentreY + 2.0f);
            arrow.lineTo(arrowCentreX + arrowSize, arrowCentreY - 1.0f);

            g.setColour(box.findColour(juce::ComboBox::arrowColourId));
            g.strokePath(arrow, juce::PathStrokeType(1.2f));
        }
    };

    //==============================================================================
    // Constantes de structure et de layout
    //==============================================================================

    // Nombre maximal de contrepoints affichables dans le workspace.
    static constexpr int maxCounterpoints = 3;

    // Nombre de points visibles pour une shape : part1, part2, part3, part4, part5.
    static constexpr int shapeControlCount = 5;

    // Hauteurs centralisées pour éviter que les paramètres soient coupés.
    static constexpr int costParameterCount = 3;
    static constexpr int mainCostRowHeight = 23;
    static constexpr int shapeControlsHeight = 54;
    static constexpr int costParameterSpacing = 8;

    // Marges internes du workspace.
    static constexpr int outerPaddingX = 4;
    static constexpr int outerPaddingY = 4;

    // Bornes de l'espace horizontal entre les colonnes de contrepoint.
    static constexpr int minGapBetweenCounterpoints = 18;
    static constexpr int maxGapBetweenCounterpoints = 30;

    //==============================================================================
    // État courant
    //==============================================================================

    int activeCounterpointCount = 0;
    int costScrollOffsetY = 0;

    CompactComboBoxLookAndFeel compactComboBoxLookAndFeel;

    //==============================================================================
    // Composants visibles
    //==============================================================================

    // Zone verticale complète réservée à chaque contrepoint.
    std::array<juce::Rectangle<int>, maxCounterpoints> counterpointAreas;

    // Menus Species / Type.
    std::array<juce::ComboBox, maxCounterpoints> speciesSelectors;
    std::array<juce::ComboBox, maxCounterpoints> typeSelectors;

    //==============================================================================
    // Contrôles des fonctions de coût
    //==============================================================================

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

    //==============================================================================
    // Contrôles des shapes
    //==============================================================================

    // 5 mini-sliders qui rendent visible la shape choisie pour chaque paramètre.
    using ShapeControlSliders = std::array<juce::Slider, shapeControlCount>;
    using ShapeControlLabels = std::array<juce::Label, shapeControlCount>;
    std::array<ShapeControlSliders, maxCounterpoints> steps1ShapeControls;
    std::array<ShapeControlSliders, maxCounterpoints> steps2ShapeControls;
    std::array<ShapeControlSliders, maxCounterpoints> harmoShapeControls;
    std::array<ShapeControlLabels, maxCounterpoints> steps1ShapeControlLabels;
    std::array<ShapeControlLabels, maxCounterpoints> steps2ShapeControlLabels;
    std::array<ShapeControlLabels, maxCounterpoints> harmoShapeControlLabels;

    /*
        Découpe la zone centrale en colonnes de contrepoint.

        Chaque contrepoint reçoit une zone verticale.
        FlexBox permet de garder une répartition équilibrée quand la fenêtre change.
    */
    void updateCounterpointAreas()
    {
        // Zone disponible après retrait des petites marges internes.
        const auto availableArea = getLocalBounds().reduced(outerPaddingX, outerPaddingY);

        // Espace horizontal adaptatif entre les contrepoints.
        const float gapBetweenCounterpoints = (float) juce::jlimit(minGapBetweenCounterpoints,
                                                                   maxGapBetweenCounterpoints,
                                                                   availableArea.getWidth() / 42);

        juce::FlexBox counterpointRow;
        counterpointRow.flexDirection = juce::FlexBox::Direction::row;
        counterpointRow.alignItems = juce::FlexBox::AlignItems::stretch;
        counterpointRow.justifyContent = juce::FlexBox::JustifyContent::spaceBetween;

        // Crée trois colonnes flexibles de même poids.
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

        // Sauvegarde les bounds calculés pour les autres méthodes de layout.
        for (int counterpointIndex = 0; counterpointIndex < maxCounterpoints; ++counterpointIndex)
            counterpointAreas[counterpointIndex] =
                counterpointRow.items[counterpointIndex].currentBounds.toNearestInt();
    }

    /*
        Prépare les ComboBox Species et Type pour chaque contrepoint.

        Species propose les valeurs 1 à 5.
        Type propose les valeurs -3 à 2, converties en ids positifs pour JUCE.
    */
    void setupCounterpointSelectors()
    {
        for (int counterpointIndex = 0; counterpointIndex < maxCounterpoints; ++counterpointIndex)
        {
            auto& speciesSelector = speciesSelectors[counterpointIndex];
            auto& typeSelector = typeSelectors[counterpointIndex];

            // Remplit les choix disponibles.
            for (int species = 1; species <= 5; ++species)
                speciesSelector.addItem("Species " + juce::String(species), species);

            for (int type = -3; type <= 2; ++type)
                typeSelector.addItem("Type " + juce::String(type), typeToComboBoxId(type));

            // Applique le style compact commun aux ComboBox.
            styleSelector(speciesSelector);
            styleSelector(typeSelector);

            // Valeurs par défaut : Species 1, Type 0.
            speciesSelector.setSelectedId(1, juce::dontSendNotification);
            typeSelector.setSelectedId(typeToComboBoxId(0), juce::dontSendNotification);

            // Envoie le changement d'espèce à OptionsPanel.
            speciesSelector.onChange = [this, counterpointIndex]()
            {
                if (onSpeciesChanged)
                    onSpeciesChanged(counterpointIndex,
                                     speciesSelectors[counterpointIndex].getSelectedId());
            };

            // Envoie le changement de type à OptionsPanel.
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
        - Melody moves    -> steps1(s)
        - Interval colour -> steps2(s)
        - Perfect intervals -> harmo(s)

        Chaque slider possède aussi une ComboBox Shape à droite.
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
                                 "Consonant",
                                 "Dissonant");

            setupSmallCostSlider(harmoLabels[counterpointIndex],
                                 harmoSliders[counterpointIndex],
                                 harmoShapeSelectors[counterpointIndex],
                                 harmoLeftReferenceLabels[counterpointIndex],
                                 harmoRightReferenceLabels[counterpointIndex],
                                 "Perfect intervals",
                                 "Less Octaves",
                                 "Less fifths");

            // Fonction locale utilisée par les trois sliders pour éviter la répétition.
            auto sendSliderValue = [this, counterpointIndex](CostSliderTarget target, juce::Slider& slider)
            {
                if (onCostSliderChanged)
                    onCostSliderChanged(counterpointIndex, target, slider.getValue());
            };

            // Connecte chaque slider à sa fonction de coût.
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

            // Prépare les 5 mini-sliders de shape pour chaque fonction de coût.
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
        Prépare les 5 mini-sliders associés à une shape.

        Ils sont cachés au départ.
        Ils deviennent visibles quand une shape est choisie dans la ComboBox.
    */
    void setupShapeControls(int counterpointIndex,
                            CostSliderTarget target,
                            juce::ComboBox& shapeSelector)
    {
        // Récupère le bon groupe de mini-sliders selon le paramètre ciblé.
        auto& controls = getShapeControls(counterpointIndex, target);
        auto& labels = getShapeControlLabels(counterpointIndex, target);

        for (int controlIndex = 0; controlIndex < shapeControlCount; ++controlIndex)
        {
            auto& label = labels[controlIndex];
            auto& slider = controls[controlIndex];

            label.setText("Part. " + juce::String(controlIndex + 1),
                          juce::dontSendNotification);
            label.setFont(juce::Font(juce::FontOptions(9.2f, juce::Font::bold)));
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

            // Chaque mini-slider modifie une partie de la shape.
            slider.onValueChange = [this, counterpointIndex, target]()
            {
                sendShapeToModel(counterpointIndex, target);
            };

            // addChildComponent : le composant existe mais reste invisible au départ.
            addChildComponent(label);
            addChildComponent(slider);
        }

        auto* selectorToRead = &shapeSelector;

        // Quand la ComboBox Shape change, on applique les valeurs de base de cette shape.
        shapeSelector.onChange = [this, counterpointIndex, target, selectorToRead]()
        {
            applyShapeChoice(counterpointIndex, target, selectorToRead->getSelectedId());
        };
    }

    /*
        Applique une shape choisie dans la ComboBox.

        La shape donne des valeurs de départ aux 5 mini-sliders.
        L'utilisateur peut ensuite ajuster ces valeurs manuellement.
    */
    void applyShapeChoice(int counterpointIndex, CostSliderTarget target, int shapeId)
    {
        auto& controls = getShapeControls(counterpointIndex, target);
        auto& labels = getShapeControlLabels(counterpointIndex, target);

        // Valeurs initiales correspondant à Fixed, Linear, V, M, etc.
        const auto values = getDefaultShapeControlValues(shapeId);

        // Affiche les 5 parties de shape et leur donne leurs valeurs initiales.
        for (int controlIndex = 0; controlIndex < shapeControlCount; ++controlIndex)
        {
            labels[controlIndex].setVisible(true);
            controls[controlIndex].setVisible(true);
            controls[controlIndex].setValue(values[controlIndex], juce::dontSendNotification);
        }

        // Envoie immédiatement cette shape au modèle.
        sendShapeToModel(counterpointIndex, target);
        resized();
    }

    /*
        Envoie la shape courante à OptionsPanel.

        La méthode lit :
        - la shape sélectionnée dans la ComboBox,
        - les 5 valeurs part1-part5,
        puis appelle onShapeChanged.
    */
    void sendShapeToModel(int counterpointIndex, CostSliderTarget target)
    {
        const int shapeId = getShapeSelector(counterpointIndex, target).getSelectedId();
        auto& controls = getShapeControls(counterpointIndex, target);

        std::vector<double> values;
        values.reserve(shapeControlCount);

        // Lit les 5 valeurs visibles dans l'interface.
        for (auto& control : controls)
            values.push_back(control.getValue());

        // Prévient OptionsPanel que la shape a changé.
        if (onShapeChanged)
            onShapeChanged(counterpointIndex, target, shapeId, values);
    }

    /*
        Retourne les 5 mini-sliders liés à une fonction de coût précise.

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
        Retourne la ComboBox Shape liée à une fonction de coût précise.
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

        setSelectedId(0) ne suffit pas toujours à retirer le texte affiché.
        On force donc l'état "rien sélectionné".
    */
    void clearShapeSelector(juce::ComboBox& shapeSelector)
    {
        shapeSelector.setSelectedItemIndex(-1, juce::dontSendNotification);
        shapeSelector.setTextWhenNothingSelected("Shape");
    }

    /*
        Retourne les labels part1..part5 liés à une fonction de coût précise.
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
        Donne les 5 valeurs initiales d'une shape.

        Ces valeurs correspondent aux fonctions de Dorian évaluées avec n = 5.

        Exemple :
        - Inverted V -> {0.0, 0.5, 1.0, 0.5, 0.0}
        - V          -> {1.0, 0.5, 0.0, 0.5, 1.0}
        - M          -> {0.0, 1.0, 0.0, 1.0, 0.0}

        L'utilisateur peut ensuite modifier ces valeurs.
    */
    std::array<double, shapeControlCount> getDefaultShapeControlValues(int shapeId) const
    {
        switch (shapeId)
        {
            case 1: return { 0.0, 0.0, 0.0, 0.0, 0.0 }; // Fixed
            case 2: return { 0.0, 0.25, 0.5, 0.75, 1.0 }; // Linear
            case 3: return { 1.0, 0.75, 0.5, 0.25, 0.0 }; // Linear desc
            case 4: return { 0.0, 0.5, 1.0, 0.5, 0.0 }; // Inverted V
            case 5: return { 1.0, 0.5, 0.0, 0.5, 1.0 }; // V
            case 6: return { 0.0, 1.0, 0.0, 1.0, 0.0 }; // M
            case 7: return { 0.0, 0.0, 1.0, 1.0, 1.0 }; // Step
            case 8: return { 1.0, 1.0, 0.0, 0.0, 0.0 }; // Step desc
            default: return { 0.0, 0.0, 0.0, 0.0, 0.0 };
        }
    }

    /*
        Remet tous les sliders principaux à zéro.

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
        Cache et remet à zéro tous les contrôles part1-part5.

        Le modèle est vidé séparément par OptionsPanel.
        Ici, on nettoie seulement l'affichage.
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
        Réinitialise un groupe de 5 mini-sliders de shape.
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
        Cache les labels part1..part5 d'un groupe de shape.
    */
    void resetShapeLabelGroup(ShapeControlLabels& labels)
    {
        for (auto& label : labels)
            label.setVisible(false);
    }

    /*
        Configure une ligne complète de paramètre de coût.

        Une ligne contient :
        - le nom du paramètre,
        - le slider principal,
        - la ComboBox Shape,
        - les repères gauche/droite du slider.
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
        // Nom visible du paramètre.
        label.setText(text, juce::dontSendNotification);
        label.setFont(juce::Font(juce::FontOptions(10.5f, juce::Font::bold)));
        label.setJustificationType(juce::Justification::centredLeft);
        label.setColour(juce::Label::textColourId, juce::Colours::white);
        label.setMinimumHorizontalScale(0.70f);

        // Slider principal entre 0 et 1.
        slider.setRange(0.0, 1.0, 0.01);
        slider.setValue(0.0, juce::dontSendNotification);
        slider.setSliderStyle(juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider.setColour(juce::Slider::trackColourId, juce::Colour(0xff2f4f4f));
        slider.setColour(juce::Slider::thumbColourId, juce::Colours::white);

        // Shape à droite + repères sous le slider.
        setupShapeSelector(shapeSelector);
        setupReferenceLabel(leftReferenceLabel, leftReference, juce::Justification::centredLeft);
        setupReferenceLabel(rightReferenceLabel, rightReference, juce::Justification::centredRight);

        // Tous les éléments de la ligne deviennent visibles.
        addAndMakeVisible(label);
        addAndMakeVisible(slider);
        addAndMakeVisible(shapeSelector);
        addAndMakeVisible(leftReferenceLabel);
        addAndMakeVisible(rightReferenceLabel);
    }

    /*
        Configure un petit repère de slider.

        Exemple :
        Smooth à gauche, Jumpy à droite.
    */
    void setupReferenceLabel(juce::Label& referenceLabel,
                             const juce::String& text,
                             juce::Justification justification)
    {
        referenceLabel.setText(text, juce::dontSendNotification);
        referenceLabel.setFont(juce::Font(juce::FontOptions(8.4f, juce::Font::bold)));
        referenceLabel.setJustificationType(justification);
        referenceLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.9f));
        referenceLabel.setMinimumHorizontalScale(0.70f);
    }

    /*
        Remplit la ComboBox avec les shapes disponibles.

        Les ids doivent rester cohérents avec mapShapeId dans OptionsPanel.
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
        Applique le style commun aux petites ComboBox du workspace.
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
        Place les ComboBox Species et Type sous le titre du contrepoint.
    */
    void layoutCounterpointSelectors()
    {
        for (int counterpointIndex = 0; counterpointIndex < maxCounterpoints; ++counterpointIndex)
        {
            auto controlsRow = getCounterpointControlsBounds(counterpointAreas[counterpointIndex]);

            juce::FlexBox selectorRow;
            selectorRow.flexDirection = juce::FlexBox::Direction::row;
            selectorRow.alignItems = juce::FlexBox::AlignItems::stretch;

            // FlexBox partage la ligne en deux menus de même largeur.
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
        Place les trois paramètres de coût dans chaque colonne de contrepoint.
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
        Calcule la hauteur totale nécessaire pour afficher tous les paramètres.

        Si on ajoute une fonction de coût plus tard, on augmente seulement
        costParameterCount ou on centralise la liste des paramètres ici.
    */
    int getRequiredCostSlidersHeight() const
    {
        return costParameterCount * (mainCostRowHeight + shapeControlsHeight + costParameterSpacing);
    }

    /*
        Calcule combien on peut défiler verticalement dans les paramètres.

        Si le contenu tient dans la zone visible, le maximum vaut 0 :
        aucun scroll n'est nécessaire.
    */
    int getMaxCostScrollOffset() const
    {
        if (counterpointAreas[0].isEmpty())
            return 0;

        const auto visibleArea = getCostSlidersVisibleBounds(counterpointAreas[0]);
        return juce::jmax(0, getRequiredCostSlidersHeight() - visibleArea.getHeight());
    }

    /*
        Garde le défilement dans des limites valides.
    */
    void clampCostScrollOffset()
    {
        costScrollOffsetY = juce::jlimit(0, getMaxCostScrollOffset(), costScrollOffsetY);
    }

    /*
        Place un paramètre complet.

        Il contient :
        - une ligne principale : label, slider, ComboBox Shape,
        - une zone secondaire : les 5 mini-sliders part1-part5.
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
        layoutSmallCostSlider(area.removeFromTop(mainCostRowHeight),
                              label,
                              slider,
                              shapeSelector,
                              leftReferenceLabel,
                              rightReferenceLabel);

        area.removeFromTop(1);
        layoutShapeControls(area.removeFromTop(shapeControlsHeight),
                            getShapeControls(counterpointIndex, target),
                            getShapeControlLabels(counterpointIndex, target));
        area.removeFromTop(1);
    }

    /*
        Place une ligne compacte de slider.

        Découpage horizontal :
        - label à gauche,
        - slider au centre,
        - ComboBox Shape à droite.
    */
    void layoutSmallCostSlider(juce::Rectangle<int> row,
                               juce::Label& label,
                               juce::Slider& slider,
                               juce::ComboBox& shapeSelector,
                               juce::Label& leftReferenceLabel,
                               juce::Label& rightReferenceLabel)
    {
        // Réserve la colonne du label.
        auto labelArea = row.removeFromLeft(70);
        row.removeFromLeft(4);

        // Réserve la ComboBox Shape à droite.
        auto shapeBox = row.removeFromRight(48);
        row.removeFromRight(5);

        // Le reste devient la zone du slider et de ses repères.
        auto sliderArea = row;
        auto referenceArea = sliderArea.removeFromTop(10);

        label.setBounds(labelArea.withY(sliderArea.getY()).withHeight(sliderArea.getHeight()));
        slider.setBounds(sliderArea.reduced(0, 1));
        shapeSelector.setBounds(shapeBox.withHeight(17).reduced(0, 1));
        leftReferenceLabel.setBounds(referenceArea.removeFromLeft(referenceArea.getWidth() / 2));
        rightReferenceLabel.setBounds(referenceArea);
    }

    /*
        Place les 5 mini-sliders part1-part5 sous le slider principal.
    */
    void layoutShapeControls(juce::Rectangle<int> row,
                             ShapeControlSliders& controls,
                             ShapeControlLabels& labels)
    {
        // Indentation pour aligner les mini-sliders sous le slider principal.
        row = row.withTrimmedLeft(18).withTrimmedRight(96);

        for (int controlIndex = 0; controlIndex < shapeControlCount; ++controlIndex)
        {
            // Chaque ligne contient un label partX puis son mini-slider.
            auto line = row.removeFromTop(10);
            auto labelArea = line.removeFromLeft(37);
            line.removeFromLeft(4);

            labels[controlIndex].setBounds(labelArea);
            controls[controlIndex].setBounds(line.reduced(0, 1));
        }
    }

    /*
        Active ou grise les contrôles selon le nombre de contrepoints choisis.

        Les contrepoints actifs sont utilisables.
        Les autres restent visibles mais désactivés avec une transparence.
    */
    void updateCounterpointSelectorStates()
    {
        for (int counterpointIndex = 0; counterpointIndex < maxCounterpoints; ++counterpointIndex)
        {
            // isActive indique si ce contrepoint fait partie du nombre de voix choisi.
            const bool isActive = counterpointIndex < activeCounterpointCount;

            // Même alpha pour tous les éléments visuels de cette colonne.
            const float alpha = isActive ? 1.0f : 0.35f;

            // Menus Species / Type.
            speciesSelectors[counterpointIndex].setEnabled(isActive);
            typeSelectors[counterpointIndex].setEnabled(isActive);

            speciesSelectors[counterpointIndex].setAlpha(alpha);
            typeSelectors[counterpointIndex].setAlpha(alpha);

            // Labels et repères des sliders.
            steps1Labels[counterpointIndex].setAlpha(alpha);
            steps2Labels[counterpointIndex].setAlpha(alpha);
            harmoLabels[counterpointIndex].setAlpha(alpha);

            steps1LeftReferenceLabels[counterpointIndex].setAlpha(alpha);
            steps1RightReferenceLabels[counterpointIndex].setAlpha(alpha);
            steps2LeftReferenceLabels[counterpointIndex].setAlpha(alpha);
            steps2RightReferenceLabels[counterpointIndex].setAlpha(alpha);
            harmoLeftReferenceLabels[counterpointIndex].setAlpha(alpha);
            harmoRightReferenceLabels[counterpointIndex].setAlpha(alpha);

            // Sliders principaux.
            steps1Sliders[counterpointIndex].setEnabled(isActive);
            steps2Sliders[counterpointIndex].setEnabled(isActive);
            harmoSliders[counterpointIndex].setEnabled(isActive);

            steps1Sliders[counterpointIndex].setAlpha(alpha);
            steps2Sliders[counterpointIndex].setAlpha(alpha);
            harmoSliders[counterpointIndex].setAlpha(alpha);

            // ComboBox Shape.
            steps1ShapeSelectors[counterpointIndex].setEnabled(isActive);
            steps2ShapeSelectors[counterpointIndex].setEnabled(isActive);
            harmoShapeSelectors[counterpointIndex].setEnabled(isActive);

            steps1ShapeSelectors[counterpointIndex].setAlpha(alpha);
            steps2ShapeSelectors[counterpointIndex].setAlpha(alpha);
            harmoShapeSelectors[counterpointIndex].setAlpha(alpha);

            // Mini-sliders part1-part5.
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
        Rend visibles les contrôles part1-part5.

        Ils peuvent ensuite être désactivés/grisés si le contrepoint n'est pas actif.
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
        Active ou désactive les mini-sliders d'une shape.
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
        Applique la transparence du contrepoint aux labels part1-part5.
    */
    void updateShapeLabelState(ShapeControlLabels& labels, float alpha)
    {
        for (auto& label : labels)
            label.setAlpha(alpha);
    }

    /*
        Calcule la zone du titre Counterpoint X.

        Le titre reste centré en haut de la colonne.
    */
    juce::Rectangle<int> getCounterpointTitleBounds(juce::Rectangle<int> counterpointArea) const
    {
        const int titleBoxWidth = juce::jlimit(78,
                                               118,
                                               static_cast<int>(counterpointArea.getWidth() * 0.52f));
        const int titleBoxHeight = 18;

        return {
            counterpointArea.getCentreX() - titleBoxWidth / 2,
            counterpointArea.getY() + 6,
            titleBoxWidth,
            titleBoxHeight
        };
    }

    /*
        Calcule la zone des ComboBox Species / Type.

        Cette zone est placée juste sous le titre du contrepoint.
    */
    juce::Rectangle<int> getCounterpointControlsBounds(juce::Rectangle<int> counterpointArea) const
    {
        auto titleBox = getCounterpointTitleBounds(counterpointArea);

        const int controlsWidth = juce::jlimit(86, 126, counterpointArea.getWidth() - 34);
        const int controlsHeight = 16;
        const int controlsY = titleBox.getBottom() + 12;

        return {
            counterpointArea.getCentreX() - controlsWidth / 2,
            controlsY,
            controlsWidth,
            controlsHeight
        };
    }

    /*
        Calcule la zone visible réservée aux paramètres de coût.

        Elle commence sous les ComboBox Species / Type.
    */
    juce::Rectangle<int> getCostSlidersVisibleBounds(juce::Rectangle<int> counterpointArea) const
    {
        auto controlsBox = getCounterpointControlsBounds(counterpointArea);

        const int slidersWidth = juce::jmin(250, juce::jmax(190, counterpointArea.getWidth() - 18));
        const int slidersY = controlsBox.getBottom() + 10;
        const int slidersHeight = juce::jmax(0, counterpointArea.getBottom() - slidersY);

        return {
            counterpointArea.getCentreX() - slidersWidth / 2,
            slidersY,
            slidersWidth,
            slidersHeight
        };
    }

    /*
        Calcule la zone réelle du contenu des paramètres.

        Cette zone peut être plus haute que la zone visible.
        Le décalage costScrollOffsetY permet alors de défiler sans couper
        les derniers sliders.
    */
    juce::Rectangle<int> getCostSlidersBounds(juce::Rectangle<int> counterpointArea) const
    {
        auto visibleArea = getCostSlidersVisibleBounds(counterpointArea);

        return visibleArea
            .withY(visibleArea.getY() - costScrollOffsetY)
            .withHeight(getRequiredCostSlidersHeight());
    }

    /*
        Dessine le bandeau Counterpoint X.

        Les contrepoints actifs sont verts.
        Les contrepoints inactifs sont gris.
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

        // Fond, contour, puis texte centré.
        g.setColour(backgroundColour);
        g.fillRoundedRectangle(bounds, 5.0f);

        g.setColour(juce::Colours::white.withAlpha(0.9f));
        g.drawRoundedRectangle(bounds, 5.0f, 0.9f);

        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(juce::FontOptions(9.2f, juce::Font::bold)));
        g.drawText(title, titleBox, juce::Justification::centred);
    }

    /*
        Convertit un type musical en id positif pour JUCE.

        Les types vont de -3 à 2.
        Les ComboBox JUCE utilisent des ids positifs.
    */
    static int typeToComboBoxId(int type)
    {
        return type + 4;
    }

    /*
        Convertit l'id positif de la ComboBox vers le type musical réel.
    */
    static int comboBoxIdToType(int comboBoxId)
    {
        return comboBoxId - 4;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VoiceWorkspace)
};
