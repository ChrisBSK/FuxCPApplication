#include "OptionsPanel.h"

#include "../leftPanel/LeftPanel.h"
#include "../../controller/AppController.h"
#include "OptionsPanelHelpers.h"

/*
    =====================================================================
    OptionsPanel.cpp — panneau principal de configuration

    Ce fichier construit la partie centrale de l'interface.

    Il contient :
    - la grande zone des contrepoints,
    - la colonne Search,
    - les boutons Clear / Generate,
    - les connexions entre l'interface et AppController.

    Son rôle est surtout de synchroniser l'UI avec le modèle.
    =====================================================================
*/

//==============================================================================
// Fonctions internes de conversion
//==============================================================================

namespace
{
    /*
        Convertit le slider de VoiceWorkspace vers la cible utilisée par ConstraintSettings.

        L'interface parle en CostSliderTarget.
        Le modèle parle en ShapeCostTarget.

        Cette fonction fait simplement le pont entre les deux.
    */
    ConstraintSettings::ShapeCostTarget mapShapeTarget(VoiceWorkspace::CostSliderTarget target)
    {
        switch (target)
        {
            case VoiceWorkspace::CostSliderTarget::melodyMovement:
                return ConstraintSettings::ShapeCostTarget::melodyMovement;

            case VoiceWorkspace::CostSliderTarget::intervalColour:
                return ConstraintSettings::ShapeCostTarget::intervalColour;

            case VoiceWorkspace::CostSliderTarget::perfectIntervals:
                return ConstraintSettings::ShapeCostTarget::perfectIntervals;
        }

        return ConstraintSettings::ShapeCostTarget::melodyMovement;
    }

    /*
        Convertit l'identifiant de la ComboBox Shape vers le type de shape du modèle.

        Exemple :
        - 2 -> linear
        - 4 -> invertedV
        - 6 -> m

        Si l'id est inconnu, on revient à fixedZero par sécurité.
    */
    ConstraintSettings::ShapeType mapShapeId(int shapeId)
    {
        switch (shapeId)
        {
            case 1: return ConstraintSettings::ShapeType::fixedZero;
            case 2: return ConstraintSettings::ShapeType::linear;
            case 3: return ConstraintSettings::ShapeType::linearDescending;
            case 4: return ConstraintSettings::ShapeType::invertedV;
            case 5: return ConstraintSettings::ShapeType::v;
            case 6: return ConstraintSettings::ShapeType::m;
            case 7: return ConstraintSettings::ShapeType::step;
            case 8: return ConstraintSettings::ShapeType::stepDescending;
            default: return ConstraintSettings::ShapeType::fixedZero;
        }
    }
}

//==============================================================================
// Construction
//==============================================================================

/*
    Crée le panneau et initialise tous ses sous-composants.

    L'ordre est important :
    on crée d'abord les zones visuelles, puis les boutons,
    puis les connexions vers le modèle.
*/
OptionsPanel::OptionsPanel()
{
    // Initialisation de l'interface
    setupColumns();
    setupButtons();
    setupSolverPriorities();
    setupMinimizationModePanel();
}

//==============================================================================
// Initialisation UI
//==============================================================================

/*
    Ajoute les grandes zones visibles du panneau.

    Cette méthode connecte aussi VoiceWorkspace au modèle :
    - changement d'espèce,
    - changement de type,
    - changement de slider,
    - changement de shape.
*/
void OptionsPanel::setupColumns()
{
    // Ajoute les deux grandes colonnes et le workspace des contrepoints à l'interface.
    addAndMakeVisible(workspaceColumn);
    addAndMakeVisible(searchColumn);
    addAndMakeVisible(voiceWorkspace);

    /*
        Les ColumnBox servent ici uniquement de fond visuel.

        Elles ne doivent pas recevoir les clics, sinon elles peuvent bloquer
        les vrais boutons ou les contrôles placés au-dessus.
    */
    workspaceColumn.setInterceptsMouseClicks(false, false);
    searchColumn.setInterceptsMouseClicks(false, false);

    /*
            VoiceWorkspace ne modifie pas directement le modèle.

            Il envoie des événements à OptionsPanel, qui se charge ensuite
            de transmettre les changements à AppController.
    */

    // L'espèce change pour un contrepoint : on garde son type actuel.
    voiceWorkspace.onSpeciesChanged = [this](int counterpointIndex, int species)
    {
        if (appController == nullptr)
            return;

        auto& voiceSettings = appController->getVoiceSettings();

        // Sécurité : on ignore un index qui ne correspond à aucune voix.
        if (counterpointIndex >= (int) voiceSettings.size())
            return;

        // on met à jour le modèle
        appController->updateVoice(counterpointIndex,
                                   species,
                                   voiceSettings[counterpointIndex].type);
    };

    // Le type change pour un contrepoint : on garde son espèce actuelle.
    voiceWorkspace.onTypeChanged = [this](int counterpointIndex, int type)
    {
        if (appController == nullptr)
            return;

        auto& voiceSettings = appController->getVoiceSettings();
        if (counterpointIndex >= (int) voiceSettings.size())
            return;

        // on met à jour le modèle
        appController->updateVoice(counterpointIndex,
                                   voiceSettings[counterpointIndex].species,
                                   type);
    };

    // Un slider de coût change : on applique la nouvelle valeur au modèle.
    voiceWorkspace.onCostSliderChanged = [this](int counterpointIndex,
                                                VoiceWorkspace::CostSliderTarget target,
                                                double value)
    {
        if (appController == nullptr || appController->isGenerating())
            return;

        // on met à jour le modèle
        applyCostSliderValueToSettings(counterpointIndex, target, value);
    };

    // Une shape change : on stocke la shape et ses valeurs part1-part5 dans le modèle.
    voiceWorkspace.onShapeChanged = [this](int counterpointIndex,
                                           VoiceWorkspace::CostSliderTarget target,
                                           int shapeId,
                                           const std::vector<double>& values)
    {
        if (appController == nullptr || appController->isGenerating())
            return;

        // on met à jour le modèle
        applyShapeToSettings(counterpointIndex, target, shapeId, values);
    };
}


/*
    Configure les boutons principaux du panneau.

    Generate lance la génération via LeftPanel.
    Next Solution relance une génération pour demander une autre proposition.
    Save configuration est préparé pour sauvegarder la configuration courante.
    Clear remet l'interface dans son état initial.
*/
void OptionsPanel::setupButtons()
{
    OptionsPanelHelpers::setupButton(*this, generateButton, "Generate");
    OptionsPanelHelpers::setupButton(*this, nextSolutionButton, "Next solution");
    OptionsPanelHelpers::setupButton(*this, saveSolutionButton, "Save configuration");
    OptionsPanelHelpers::setupButton(*this, clearButton, "Clear");

    generateButton.onClick = [this]()
    {
        if (leftPanel != nullptr)
            leftPanel->triggerGeneration();
    };

    clearButton.onClick = [this]()
    {
        clearGenerationInputs();
    };

    nextSolutionButton.onClick = [this]()
    {
        if (leftPanel != nullptr)
            leftPanel->triggerNextSolution();
    };

    saveSolutionButton.onClick = [this]()
    {
        if (leftPanel != nullptr)
            leftPanel->triggerSaveSolution();
    };
}

/*
    Configure la liste des priorités du solveur.

    Cette liste sert aux deux modes de minimisation :
    - Lexicographic : elle affiche les rangs de priorité,
    - Weighted Sum : elle affiche les poids calculés à partir des rangs.

    Quand l'ordre change, on met à jour ConstraintSettings puis on recalcule
    les coûts du problème.
*/
void OptionsPanel::setupSolverPriorities()
{
    // Ajoute la liste des priorités et les deux boutons de déplacement.
    addAndMakeVisible(solverPriorityList);
    addAndMakeVisible(movePriorityUpButton);
    addAndMakeVisible(movePriorityDownButton);

    // La liste est affichée en version compacte dans la colonne Search.
    solverPriorityList.setCompactMode(true);
    solverPriorityList.setDisplayMode(SolverPriorityList::DisplayMode::rank);

    // La liste reste visible dans Lexicographic et Weighted Sum.
    updateSolverPriorityVisibility();

    /*
        La liste renvoie un nouveau vecteur d'importance dès que l'ordre change.

        Exemple :
        si l'utilisateur déplace "melodic", la liste reconstruit un nouvel ordre
        et l'envoie ici sous forme de std::vector<int>.
    */
    solverPriorityList.onPriorityOrderChanged = [this](const std::vector<int>& importanceCosts)
    {
        if (appController == nullptr || appController->isGenerating())
            return;

        auto& problem = appController->getProblem();

        // On enregistre le nouvel ordre des priorités dans les réglages.
        problem.getSettings().setImportanceCosts(importanceCosts);

        // On recalcule pour que le prochain Generate utilise cet ordre.
        problem.recalculateCosts();

        std::cout << "\n=== SOLVER PRIORITIES CHANGED ===\n";
        std::cout << "Importance costs = ";
        for (int cost : problem.getImportanceCosts())
            std::cout << cost << " ";
        std::cout << "\n";
    };

    // Déplace la priorité sélectionnée vers le haut.
    movePriorityUpButton.onClick = [this]()
    {
        if (appController == nullptr || appController->isGenerating())
            return;

        solverPriorityList.moveSelectedUp();
    };
    // Déplace la priorité sélectionnée vers le bas.
    movePriorityDownButton.onClick = [this]()
    {
        if (appController == nullptr || appController->isGenerating())
            return;

        solverPriorityList.moveSelectedDown();
    };
}

/*
    Configure le panneau Search.

    Il permet de choisir :
    - la méthode de recherche : DFS ou BAB,
    - la méthode de minimisation : Lexicographic ou Weighted Sum.

    Le choix DFS/BAB est envoyé au modèle.
    Le choix Lexicographic contrôle l'affichage de la liste des priorités.
*/
void OptionsPanel::setupMinimizationModePanel()
{
    // Ajoute le panneau Search à l'interface.
    addAndMakeVisible(minimizationModePanel);

    /*
        L'utilisateur choisit entre BAB et DFS.

        Ce choix est stocké dans ConstraintSettings.
        GenerationService le lira ensuite au moment de lancer le solveur.
    */
    minimizationModePanel.onBabSearchMethodChanged = [this](bool useBab)
    {
        if (appController == nullptr || appController->isGenerating())
            return;

        auto& settings = appController->getProblem().getSettings();

        // BAB est le mode par défaut ; DFS reste disponible pour comparer.
        settings.setSearchMethod(useBab
            ? ConstraintSettings::SearchMethod::bab
            : ConstraintSettings::SearchMethod::dfs);

        std::cout << "\n=== SEARCH METHOD CHANGED ===\n";
        std::cout << "searchMethod = " << (useBab ? "BAB" : "DFS") << "\n";
    };

    /*
        L'utilisateur choisit entre Lexicographic et Weighted Sum.

        Ce choix est stocké dans ConstraintSettings.
        GenerationService le traduira ensuite vers OBJECTIVE_LEX
        ou OBJECTIVE_SUMWEIGHTED au moment de créer FuxCP.
    */
    minimizationModePanel.onLexicographicModeChanged = [this](bool isLexicographic)
    {
        if (appController == nullptr || appController->isGenerating())
            return;

        showLexicographicPriorities = isLexicographic;

        auto& settings = appController->getProblem().getSettings();

        // Enregistre le vrai mode de minimisation dans le modèle.
        settings.setMinimizationMethod(isLexicographic
            ? ConstraintSettings::MinimizationMethod::lexicographic
            : ConstraintSettings::MinimizationMethod::weightedSum);

        // Change la lecture de la liste sans changer son ordre interne.
        solverPriorityList.setDisplayMode(isLexicographic
            ? SolverPriorityList::DisplayMode::rank
            : SolverPriorityList::DisplayMode::weight);

        std::cout << "\n=== MINIMIZATION METHOD CHANGED ===\n";
        std::cout << "minimizationMethod = "
                  << (isLexicographic ? "Lexicographic" : "Weighted Sum")
                  << "\n";

        // La liste et les flèches restent disponibles dans les deux modes.
        updateSolverPriorityVisibility();
        // Relance le layout pour réorganiser la colonne Search.
        resized();
    };
}

//==============================================================================
// Connexion UI -> modèle
//==============================================================================

/*
    Traduit un slider de l'interface vers le paramètre FuxCP correspondant.

    Ici, on ne crée pas les coûts directement dans l'UI :
    l'UI modifie seulement ConstraintSettings, puis CantusProblem recalcule
    les vecteurs que GenerationService enverra ensuite à FuxCP.
*/
void OptionsPanel::applyCostSliderValueToSettings(int counterpointIndex,
                                                  VoiceWorkspace::CostSliderTarget target,
                                                  double value)
{
    if (appController == nullptr)
        return;

    auto& problem = appController->getProblem();
    auto& settings = problem.getSettings();

    /*
        Chaque slider visible correspond à une fonction de coût définie par Dorian.

        Important :
        le counterpointIndex permet de modifier uniquement le contrepoint concerné.
        Bouger un slider de CP1 ne modifie donc pas CP2 ou CP3.
    */
    switch (target)
    {
        case VoiceWorkspace::CostSliderTarget::melodyMovement:
            // Melody movement pilote steps1(s) pour ce contrepoint uniquement.
            settings.setCounterpointMelodyMovement(counterpointIndex, value);
            break;

        case VoiceWorkspace::CostSliderTarget::intervalColour:
            // Interval colour pilote steps2(s) pour ce contrepoint uniquement.
            settings.setCounterpointIntervalColour(counterpointIndex, value);
            break;

        case VoiceWorkspace::CostSliderTarget::perfectIntervals:
            // Perfect intervals pilote harmo(s) pour ce contrepoint uniquement.
            settings.setCounterpointPerfectIntervals(counterpointIndex, value);
            break;
    }
    // Met à jour les coûts du problème avec les nouvelles valeurs.
    problem.recalculateCosts();
}

/*
    Applique au modèle la shape choisie dans l'interface.

    Une shape est toujours liée à :
    - un contrepoint précis,
    - une fonction de coût précise,
    - un type de shape,
    - les valeurs part1-part5 affichées dans l'interface.

    Si aucune shape n'est sélectionnée, on supprime l'assignation.
    FuxCP utilisera alors simplement la valeur du slider principal.
*/
void OptionsPanel::applyShapeToSettings(int counterpointIndex,
                                        VoiceWorkspace::CostSliderTarget target,
                                        int shapeId,
                                        const std::vector<double>& values)
{
    if (appController == nullptr)
        return;

    auto& problem = appController->getProblem();
    auto& settings = problem.getSettings();

    // Convertit la cible UI vers la cible comprise par ConstraintSettings.
    const auto modelTarget = mapShapeTarget(target);

    if (shapeId <= 0)
    {
        // Aucune shape sélectionnée : retour au comportement du slider seul.
        settings.removeShapeAssignment(counterpointIndex, modelTarget);
    }
    else
    {
        // Shape sélectionnée : on stocke la voix, la cible, le type et part1-part5.
        settings.setShapeAssignment(counterpointIndex,
                                    modelTarget,
                                    mapShapeId(shapeId),
                                    values);
    }
    // Met à jour les coûts du problème avec la nouvelle configuration.
    problem.recalculateCosts();
}

/*
    Affiche les contrôles de la liste des coûts.

    La liste est utile dans les deux modes :
    - Lexicographic lit les lignes comme des priorités,
    - Weighted Sum lit les mêmes lignes comme des poids.
*/
void OptionsPanel::updateSolverPriorityVisibility()
{
    // Les flèches restent actives : changer l'ordre change aussi les poids affichés.
    solverPriorityList.setVisible(true);
    movePriorityUpButton.setVisible(true);
    movePriorityDownButton.setVisible(true);
}



//==============================================================================
// Affichage et layout
//==============================================================================

/*
   Peint le fond général du panneau.
*/
void OptionsPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
}

/*
    Place tous les éléments du panneau.

    La méthode découpe l'espace en trois grandes zones :
    - LeftPanel à gauche,
    - workspace des contrepoints au centre,
    - colonne Search à droite.

    Les boutons Clear / Generate sont placés sous le LeftPanel.
*/
void OptionsPanel::resized()
{
    const int panelWidth = getWidth();
    const int panelHeight = getHeight();

    // Marges adaptatives : elles changent avec la taille de la fenêtre,
    // mais restent bornées pour éviter des espacements trop grands ou trop petits.
    const int outerMargin = juce::jlimit(2, 6, panelWidth / 220);
    const int horizontalInset = juce::jlimit(4, 10, panelWidth / 130);
    const int verticalInset = juce::jlimit(3, 8, panelHeight / 110);

    auto fullArea = getLocalBounds().reduced(outerMargin);

    auto contentArea = fullArea.reduced(horizontalInset, verticalInset);
    auto leftArea = contentArea;

    const int columnGap = juce::jlimit(5, 10, panelWidth / 120);
    const int leftButtonGap = juce::jlimit(4, 7, panelHeight / 120);

    // La zone centrale garde la même hauteur qu'avant :
    // on retire seulement l'espace réservé au clavier.
    if (lowerReservedHeight > 0)
        contentArea.removeFromBottom(lowerReservedHeight);

    /*
        FlexBox répartit horizontalement les grandes colonnes.

        Ici :
        - LeftPanel garde une largeur bornée,
        - workspaceColumn prend la majorité de l'espace,
        - searchColumn garde une largeur compacte.
    */
    juce::FlexBox mainRow;
    mainRow.flexDirection = juce::FlexBox::Direction::row;
    mainRow.alignItems = juce::FlexBox::AlignItems::stretch;
    mainRow.justifyContent = juce::FlexBox::JustifyContent::center;

    if (leftPanel != nullptr)
    {
        // Colonne gauche : entrées principales de génération.
        //
        // alignSelf::flexStart + une hauteur fixe l'empêchent de s'étirer
        // sur toute la hauteur disponible (contrairement au workspace et à
        // Search) : ça laisse une vraie zone libre en dessous pour les boutons,
        // sans dépendre de l'espace réservé au clavier.
        mainRow.items.add(juce::FlexItem(*leftPanel)
            .withFlex(0.0f, 1.0f, 220.0f)
            .withMinWidth(175.0f)
            .withMaxWidth(245.0f)
            .withHeight(290.0f)
            .withAlignSelf(juce::FlexItem::AlignSelf::flexStart)
            .withMargin(juce::FlexItem::Margin(0.0f, (float) columnGap, 0.0f, 0.0f)));
    }

    // Zone centrale : contrepoints, sliders et shapes.
    mainRow.items.add(juce::FlexItem(workspaceColumn)
        .withFlex(1.25f, 1.0f, 640.0f)
        .withMinWidth(540.0f)
        .withMargin(juce::FlexItem::Margin(0.0f, (float) columnGap, 0.0f, 0.0f)));

    // Colonne droite : Search et priorités du solveur.
    mainRow.items.add(juce::FlexItem(searchColumn)
        .withFlex(0.0f, 1.0f, 180.0f)
        .withMinWidth(150.0f)
        .withMaxWidth(210.0f)
        .withMargin(juce::FlexItem::Margin(0.0f)));

    // Calcule et applique les bounds finales des trois colonnes.
    mainRow.performLayout(contentArea);

    if (leftPanel != nullptr)
    {
        leftPanelBounds = leftPanel->getBounds();
        leftPanel->setBounds(leftPanelBounds);

        // Les boutons occupent tout l'espace réellement libre entre le bas
        // du LeftPanel et le bas de la zone de contenu (avant réservation du
        // clavier) : plus d'approximation, juste ce qui reste vraiment.
        const int buttonAreaHeight = juce::jmax(0,
            leftArea.getBottom() - (leftPanelBounds.getBottom() + leftButtonGap));

        auto leftButtonArea = juce::Rectangle<int>(
            leftPanelBounds.getX(),
            leftPanelBounds.getBottom() + leftButtonGap,
            leftPanelBounds.getWidth(),
            buttonAreaHeight);

        // Les boutons restent dans la colonne gauche, mais hors du contour du LeftPanel.
        layoutButtons(leftButtonArea);
    }

    // Le workspace réel vit à l'intérieur de la colonne centrale.
    workspaceBounds = workspaceColumn.getBounds();
    workspaceColumn.setBounds(workspaceBounds);
    voiceWorkspace.setBounds(workspaceBounds.reduced(3));

    /*
        On récupère la zone de la colonne Search calculée par FlexBox,
        puis on place son contenu interne.
    */
    // La colonne Search garde les bounds calculés par FlexBox.
    // On utilise ces bounds pour placer son contenu interne.
    const auto solverBounds = searchColumn.getBounds();
    layoutMinimizationModePanel(solverBounds);

    // Sécurité : les boutons restent cliquables même si un panneau passe devant.
    bringActionButtonsToFront();
}

/*
    Place le contenu de la colonne Search.

    Le panneau DFS/BAB et Lexicographic/Weighted Sum reste en haut.
    La liste apparaît en dessous dans les deux modes.
*/
void OptionsPanel::layoutMinimizationModePanel(juce::Rectangle<int> columnBounds)
{
    // On réduit légèrement la zone pour laisser respirer le contour de la colonne.
    auto area = columnBounds.reduced(9, 10);

    constexpr int modePanelHeight = 86;
    constexpr int listGap = 7;
    constexpr int arrowColumnWidth = 24;
    constexpr int arrowSize = 22;
    constexpr int arrowGap = 6;

    // Le panneau Search reste toujours placé en haut de la colonne.
    minimizationModePanel.setBounds(area.removeFromTop(modePanelHeight));

    // Petit espace entre le panneau Search et la liste des priorités.
    area.removeFromTop(listGap);

    // La liste prend la zone principale ; les flèches occupent une petite colonne à droite.
    // removeFromRight coupe la colonne en deux :
    // - arrowArea reçoit la bande de droite pour les flèches,
    // - area garde le reste pour la liste.
    auto arrowArea = area.removeFromRight(arrowColumnWidth);
    auto listArea = area.reduced(0, 2);

    solverPriorityList.setBounds(listArea);

    // Centre verticalement les deux flèches par rapport à la liste.
    const int totalArrowHeight = arrowSize * 2 + arrowGap;
    auto arrows = juce::Rectangle<int>(
        arrowArea.getCentreX() - arrowSize / 2,
        arrowArea.getCentreY() - totalArrowHeight / 2,
        arrowSize,
        totalArrowHeight
    );

    movePriorityUpButton.setBounds(arrows.removeFromTop(arrowSize));
    arrows.removeFromTop(arrowGap);
    movePriorityDownButton.setBounds(arrows.removeFromTop(arrowSize));
}

/*
    Place les boutons Generate, Next solution, Save solution et Clear sous le LeftPanel.

    Les boutons sont empilés verticalement pour rester lisibles.
    FlexBox permet de garder leur alignement propre même si la fenêtre change.
*/
void OptionsPanel::layoutButtons(juce::Rectangle<int> buttonArea)
{
    constexpr int buttonWidth = 130;
    constexpr int buttonHeight = 16;
    constexpr int spacing = 4;

    /*
        On crée une colonne de boutons.

        column -> les boutons sont placés les uns sous les autres.
        center -> ils sont centrés verticalement et horizontalement
                  dans la zone buttonArea.
    */

    juce::FlexBox buttonRow;
    buttonRow.flexDirection = juce::FlexBox::Direction::column;
    buttonRow.alignItems = juce::FlexBox::AlignItems::center;
    buttonRow.justifyContent = juce::FlexBox::JustifyContent::flexStart;

    // Bouton Generate.
    buttonRow.items.add(juce::FlexItem(generateButton)
        .withWidth((float) buttonWidth)
        .withHeight((float) buttonHeight)
        .withMargin(juce::FlexItem::Margin(0.0f, 0.0f, (float) spacing / 2.0f, 0.0f)));

    // Bouton Next Solution.
    buttonRow.items.add(juce::FlexItem(nextSolutionButton)
        .withWidth((float) buttonWidth)
        .withHeight((float) buttonHeight)
        .withMargin(juce::FlexItem::Margin((float) spacing / 2.0f, 0.0f, (float) spacing / 2.0f, 0.0f)));

    // Bouton Save Solution.
    buttonRow.items.add(juce::FlexItem(saveSolutionButton)
        .withWidth((float) buttonWidth)
        .withHeight((float) buttonHeight)
        .withMargin(juce::FlexItem::Margin((float) spacing / 2.0f, 0.0f, (float) spacing / 2.0f, 0.0f)));

    // Bouton Clear.
    buttonRow.items.add(juce::FlexItem(clearButton)
        .withWidth((float) buttonWidth)
        .withHeight((float) buttonHeight)
        .withMargin(juce::FlexItem::Margin((float) spacing / 2.0f, 0.0f, 0.0f, 0.0f)));

    // Calcule les positions finales des boutons dans buttonArea.
    buttonRow.performLayout(buttonArea);
}

/*
    Place les boutons d'action au premier plan.

    En JUCE, si deux composants se superposent, celui du dessus reçoit le clic.
    Cette méthode garantit que Generate, Next solution, Save solution et Clear
    restent au-dessus des fonds décoratifs.
*/
void OptionsPanel::bringActionButtonsToFront()
{
    clearButton.toFront(false);
    generateButton.toFront(false);
    nextSolutionButton.toFront(false);
    saveSolutionButton.toFront(false);
}

//==============================================================================
// Nettoyage/reset de la page
//==============================================================================

/*
    Remet la page dans son état initial.

    Cette méthode vide :
    - le Cantus Firmus,
    - le nombre de voix,
    - les paramètres de coûts,
    - les shapes,
    - les sélecteurs de contrepoint.
*/
void OptionsPanel::clearGenerationInputs()
{
    // Nettoie les champs visibles : Cantus Firmus, nombre de voix, Drag Zone.
    if (leftPanel != nullptr)
    {
        leftPanel->clearInputState();
        leftPanel->getGenerationService().reset();
    }

    if (appController != nullptr)
    {
        auto& generationState = appController->getGenerationState();
        generationState.setProperty("generationStatus", "idle", nullptr);
        generationState.setProperty("generationError", "", nullptr);
        generationState.setProperty("midiFilePath", "", nullptr);

        auto& problem = appController->getProblem();
        auto& settings = problem.getSettings();

        /*
            Remet les paramètres de coût à leurs valeurs neutres.

            Ces valeurs correspondent aux sliders principaux :
            - Melody movement
            - Interval colour
            - Perfect intervals
        */
        settings.setLargeLeapPenalty(0.0);
        settings.setMelodicIntervalColor(0.0);
        settings.setPerfectIntervalBalance(0.0);

        // Supprime tous les contrepoints et toutes les shapes enregistrées.
        settings.setCounterpointCount(0);
        settings.setShapeAssignments({});

        // Recalcule le problème à partir de cet état vide.
        problem.recalculateCosts();
    }

    // Réinitialise les combobox Species / Type et les contrôles du workspace.
    voiceWorkspace.resetCounterpointSelectors();
    // Repasse l'interface en mode 0 voix sélectionnées.
    setNumVoices(0);
}

//==============================================================================
// Synchronisation avec les autres composants
//==============================================================================

/*
    Met à jour le nombre de contrepoints visibles.

    L'utilisateur choisit un nombre total de voix.
    Le workspace affiche seulement les contrepoints, donc on retire le Cantus Firmus.
*/
void OptionsPanel::setNumVoices(int numVoices)
{
    const int numCounterpoints = juce::jmax(0, numVoices - 1);
    voiceWorkspace.setActiveCounterpointCount(numCounterpoints);

    if (appController == nullptr)
        return;

    // Le workspace affiche uniquement les contrepoints.
    // On garde seulement les réglages internes par défaut pour que Generate fonctionne encore.
    appController->getVoiceSettings().resize(numCounterpoints);
    appController->getProblem().getSettings().setCounterpointCount(numCounterpoints);
}

/*
    Retourne la zone occupée par le workspace des contrepoints.

    D'autres composants peuvent utiliser cette zone pour se synchroniser
    avec le placement central de l'interface.
*/
juce::Rectangle<int> OptionsPanel::getWorkspaceBounds() const
{
    return workspaceBounds;
}

/*
    Retourne la zone occupée par la colonne gauche.

    MainComponent s'en sert pour placer le clavier uniquement à droite
    du LeftPanel.
*/
juce::Rectangle<int> OptionsPanel::getLeftPanelBounds() const
{
    return leftPanelBounds;
}

/*
    Indique à OptionsPanel combien de pixels garder libres en bas.

    Cette zone est réservée au clavier :
    - le workspace central et Search restent au-dessus,
    - le LeftPanel peut continuer jusqu'en bas.
*/
void OptionsPanel::setLowerReservedHeight(int height)
{
    const int newHeight = juce::jmax(0, height);

    if (lowerReservedHeight == newHeight)
        return;

    lowerReservedHeight = newHeight;
    resized();
}

//==============================================================================
// Connexions externes
//==============================================================================

/*
    Connecte OptionsPanel à AppController.

    AppController est le point d'accès vers :
    - le problème courant,
    - les réglages de génération,
    - l'état du solveur.

    Sans AppController, OptionsPanel peut afficher l'interface,
    mais ne peut pas modifier le modèle.
*/

void OptionsPanel::setAppController(AppController* app_controller)
{
    // On garde un pointeur vers AppController pour synchroniser l'UI avec le modèle.
    appController = app_controller;

    if (appController != nullptr)
    {
        /*
            Au moment de la connexion, la liste des priorités reprend
            l'ordre actuellement stocké dans ConstraintSettings.
        */
        solverPriorityList.setImportanceCosts(
            appController->getProblem().getSettings().getImportanceCosts()
        );

    }
}

/*
    Connecte le LeftPanel à OptionsPanel.

    Le LeftPanel contient les entrées principales :
    Cantus Firmus, nombre de voix et Drag Zone.
*/
void OptionsPanel::setLeftPanel(LeftPanel* panel)
{
    // On garde une référence vers le panneau gauche.
    leftPanel = panel;

    if (leftPanel != nullptr)
    {
        // Le LeftPanel devient un composant enfant visible de OptionsPanel.
        addAndMakeVisible(*leftPanel);

        /*
            Le fond du LeftPanel ne doit pas voler les clics.

            Ses vrais contrôles restent cliquables grâce au second paramètre :
            Cantus Firmus, Number of voices et Drag Zone continuent de fonctionner.
        */
        leftPanel->setInterceptsMouseClicks(false, true);

        // On relance le layout pour lui donner immédiatement sa bonne position.
        resized();
    }
}

/*
    Retourne le LeftPanel actuellement connecté.
*/
LeftPanel* OptionsPanel::getLeftPanel() const
{
    return leftPanel;
}
