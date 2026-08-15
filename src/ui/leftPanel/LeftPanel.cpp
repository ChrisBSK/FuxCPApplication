#include "LeftPanel.h"
#include "../optionsPanel/OptionsPanel.h"
#include "../../controller/AppController.h"
#include "../../model/NoteConverter.h"

/*
//==============================================================================
   LeftPanel

   Interface principale de saisie utilisateur.

   Permet de :
   - entrer le Cantus Firmus
   - configurer les voix de contrepoint
   - lancer la génération
   - afficher et exporter le fichier MIDI généré
//==============================================================================
*/

LeftPanel::~LeftPanel()
{
    if (generationState.isValid())
        generationState.removeListener(this);

    // JUCE exige de détacher le LookAndFeel avant que l'objet qui le
    // possède (numVoicesLookAndFeel) ne soit détruit.
    numVoicesCB.setLookAndFeel(nullptr);
}
//==============================================================================
// PARSING : Cantus Firmus (texte --> MIDI)
//==============================================================================

static std::vector<int> parseCantusFirmus(const juce::String& text)
{
    std::vector<int> result;

    if (text.isEmpty())
        return result;

    /*
        Accepte plusieurs écritures du Cantus Firmus.

        Exemples valides :
        - 60,62,64,65
        - 60, 62, 64, 65
        - 60 62 64 65
        - 60    62   64 65

        Les virgules, points-virgules, espaces, tabulations et retours à la ligne
        sont tous considérés comme des séparateurs entre les notes.
    */
    auto tokens = juce::StringArray::fromTokens(text,
                                                " ,;\t\r\n",
                                                "\"");
    tokens.removeEmptyStrings();
    tokens.trim();

    for (const auto& t : tokens)
    {
        if (t.containsOnly("0123456789"))
        {
            int value = t.getIntValue();

            if (value < 0 || value > 127)
                return {};

            result.push_back(value);
        }
        else
        {
            int midi = NoteConverter::noteNameToMidi(t);

            if (midi == -1)
                return {};

            result.push_back(midi);
        }
    }

    return result;
}


//==============================================================================
// CONSTRUCTION UI
//==============================================================================

LeftPanel::LeftPanel(AppController& controller)
    : appController(controller)
{
    cfInput.setMultiLine(true);
    cfInput.setReturnKeyStartsNewLine(true);
    cfInput.setScrollbarsShown(true);
    cfInput.setCaretVisible(true);
    cfInput.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    cfInput.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    cfInput.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    cfInput.setColour(juce::TextEditor::textColourId, juce::Colours::white);

    /*
        Texte d'exemple affiché uniquement quand le champ est vide.

        setTextToShowWhenEmpty gère tout automatiquement : le texte
        disparaît dès que l'utilisateur tape un caractère, et réapparaît
        de lui-même s'il efface tout ensuite (retour à un champ vide) -
        que ce soit à l'état initial ou après un Clear.
    */
    cfInput.setTextToShowWhenEmpty(
        juce::String::fromUTF8("Entrez le CF\n"
                               "(MIDI ou Lettres)\n\n"
                               "Ex : 60 62 64 65 67 (MIDI)\n"
                               "Ex : C2 D2 E2 F2 G2 (Lettres)"),
        juce::Colours::white.withAlpha(0.4f));

    addAndMakeVisible(cfInput);

    // =========================
    // Nombre de voix (CF inclus)
    // =========================
    numVoicesCBLabel.setText("Number of voices",juce::sendNotification);
    numVoicesCB.addItem("2", 2);
    numVoicesCB.addItem("3", 3);
    numVoicesCB.addItem("4", 4);

    // Texte affiché tant qu'aucun nombre de voix n'est choisi
    // (au lancement de l'application, ou après un Clear).
    numVoicesCB.setTextWhenNothingSelected(juce::String::fromUTF8("Sélectionnez"));

    // Réduit la taille du texte affiché dans la ComboBox.
    numVoicesCB.setLookAndFeel(&numVoicesLookAndFeel);

    addAndMakeVisible(numVoicesCB);


    // Réaction au changement du nombre de voix
    numVoicesCB.onChange = [this]()
    {
        updateVoiceSpeciesUI(numVoicesCB.getSelectedId());
    };


}

void LeftPanel::paint(juce::Graphics& g)
{
    auto panelBounds = getLocalBounds().toFloat().reduced(2.0f);

    g.setColour(juce::Colours::darkgrey.darker(0.3f));
    g.fillRoundedRectangle(panelBounds, 10.0f);

    g.setColour(juce::Colours::white.withAlpha(0.2f));
    g.drawRoundedRectangle(panelBounds, 10.0f, 1.5f);

    auto area = getLocalBounds().reduced(2);
    area.removeFromTop(8);

    const int spacing = juce::jlimit(10, 18, area.getHeight() / 42);
    const int titleHeight = juce::jlimit(20, 24, area.getHeight() / 22);
    const int numSections = 3;

    juce::FlexBox sectionColumn;
    sectionColumn.flexDirection = juce::FlexBox::Direction::column;
    sectionColumn.alignItems = juce::FlexBox::AlignItems::stretch;

    for (int i = 0; i < numSections; ++i)
    {
        const float bottomMargin = i == numSections - 1 ? 0.0f : (float) spacing;

        sectionColumn.items.add(juce::FlexItem()
            .withFlex(1.0f)
            .withMargin(juce::FlexItem::Margin(0.0f, 0.0f, bottomMargin, 0.0f)));
    }

    sectionColumn.performLayout(area);

    juce::Colour darkGreen = juce::Colour(0xff2f4f4f);

    juce::String titles[3] =
    {
        juce::String::fromUTF8("Cantus Firmus"),
        juce::String::fromUTF8("Number of voices"),
        juce::String::fromUTF8("Drag Zone")
    };

    for (int i = 0; i < numSections; ++i)
    {
        auto section = sectionColumn.items[i].currentBounds.toNearestInt();

        // uniquement la barre de titre
        auto titleArea = section.removeFromTop(titleHeight).reduced(4, 0);

        g.setColour(darkGreen);
        g.fillRoundedRectangle(titleArea.toFloat(), 6.0f);

        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        g.drawText(titles[i], titleArea.reduced(10, 0),
                   juce::Justification::centredLeft);

    }

    if (cfInput.isVisible())
    {
        auto inputBounds = cfInput.getBounds().toFloat();

        g.setColour(juce::Colour(0xff24363b));
        g.fillRoundedRectangle(inputBounds, 5.0f);

        g.setColour(juce::Colour(0xff94adb0));
        g.drawRoundedRectangle(inputBounds.reduced(0.5f), 5.0f, 1.0f);
    }
}



//==============================================================================
// LOGIQUE PRINCIPALE : GENERATION
//==============================================================================

/*
//==============================================================================
   Construit et lance un problème de génération.

   Récupère les données saisies dans l'interface,
   crée la structure musicale attendue par le modèle,
   puis démarre la génération du contrepoint.
//==============================================================================
*/
void LeftPanel::triggerGeneration()
{
    // =========================
    //  VALIDATION INPUT
    // =========================

    auto rawText = cfInput.getText().trim();

    if (rawText.isEmpty())
    {
        showAlert(juce::AlertWindow::WarningIcon,
                  juce::String::fromUTF8("Erreur"),
                  juce::String::fromUTF8("Le Cantus Firmus est vide."));
        return;
    }

    auto cf = parseCantusFirmus(rawText);

    if (cf.empty())
    {
        showAlert(juce::AlertWindow::WarningIcon,
                  juce::String::fromUTF8("Erreur"),
                  juce::String::fromUTF8("Cantus Firmus invalide."));
        return;
    }

    /*
        Vérifie que le Cantus Firmus contient assez de notes.

        FuxCP utilise des règles musicales qui dépendent notamment :
        - de la première note,
        - de la dernière note,
        - de la note pénultième,
        - de plusieurs intervalles mélodiques.

        Avec moins de 5 notes, le problème devient trop court et peut produire
        des cas dégénérés dans le solveur.
    */
    constexpr int minimumCantusFirmusNotes = 5;

    if ((int) cf.size() < minimumCantusFirmusNotes)
    {
        showAlert(juce::AlertWindow::WarningIcon,
                  juce::String::fromUTF8("Cantus Firmus trop court"),
                  juce::String::fromUTF8("Veuillez entrer au moins 5 nombres dans le Cantus Firmus pour que le problème soit valide."));
        return;
    }

    if (numVoicesCB.getSelectedItemIndex() == -1)
    {
        showAlert(juce::AlertWindow::WarningIcon,
                  juce::String::fromUTF8("Erreur"),
                  juce::String::fromUTF8("Veuillez sélectionner un nombre de voix."));
        return;
    }

    // =========================
    //  CONSTRUCTION DU PROBLEME
    // =========================

    int numVoices = numVoicesCB.getSelectedId();
    int numCounterpoints = numVoices - 1;

    CantusProblem::Voices v;

    // CF
    v.cf = cf;

    // Contrepoints

    // Récupère les paramètres des contrepoints sélectionnés dans l'interface
    // (espèce et tessiture pour chaque voix).
    auto& settings = appController.getVoiceSettings();
    for (int i = 0; i < numCounterpoints; ++i)
    {
        CantusProblem::Counterpoint cp;

        cp.species = settings[i].species;
        cp.type    = settings[i].type;

        v.counterpoints.push_back(cp);
    }


    // Récupère le modèle musical central de l'application.
    // Ce modèle sera transmis ensuite au GenerationService.
    auto& problem = appController.getProblem();

    // Copie dans le modèle les voix construites à partir de l'UI.
    problem.setVoices(v);

    // Enregistre le nombre total de voix choisi par l'utilisateur.
    problem.setVoiceCount(numVoices);

    // =========================
    //  PREPARATION FICHIER
    // =========================
    prepareOutputFile();

    // =========================
    //  LANCEMENT GENERATION
    // =========================
    appController.startGeneration(midiOutFileToGenerate.getFullPathName());
}

/*
//==============================================================================
   Demande la solution suivante.

   Cette méthode ne reconstruit pas le problème depuis l'interface.
   Elle demande simplement à l'AppController de réutiliser le dernier problème
   généré et de passer à la solution suivante.
//==============================================================================
*/
void LeftPanel::triggerNextSolution()
{
    prepareOutputFile();
    appController.startNextSolution(midiOutFileToGenerate.getFullPathName());
}

/*
//==============================================================================
   Demande la sauvegarde de la configuration courante.

   Contrairement à Generate, on n'exige pas qu'une solution ait déjà été
   produite : "Save configuration" sauvegarde l'état de saisie actuel
   (Cantus Firmus, voix, réglages), pas un résultat du solveur. On construit
   donc le problème exactement comme triggerGeneration le ferait, sans
   lancer le solveur, puis on demande un nom avant d'écrire le fichier.
//==============================================================================
*/
void LeftPanel::triggerSaveSolution()
{
    // =========================
    //  VALIDATION INPUT (mêmes règles que pour Generate)
    // =========================

    auto rawText = cfInput.getText().trim();

    if (rawText.isEmpty())
    {
        showAlert(juce::AlertWindow::WarningIcon,
                  juce::String::fromUTF8("Save configuration"),
                  juce::String::fromUTF8("Le Cantus Firmus est vide."));
        return;
    }

    auto cf = parseCantusFirmus(rawText);

    if (cf.empty())
    {
        showAlert(juce::AlertWindow::WarningIcon,
                  juce::String::fromUTF8("Save configuration"),
                  juce::String::fromUTF8("Cantus Firmus invalide."));
        return;
    }

    if (numVoicesCB.getSelectedItemIndex() == -1)
    {
        showAlert(juce::AlertWindow::WarningIcon,
                  juce::String::fromUTF8("Save configuration"),
                  juce::String::fromUTF8("Veuillez sélectionner un nombre de voix."));
        return;
    }

    // =========================
    //  CONSTRUCTION DU PROBLEME
    // =========================
    // Même construction que dans triggerGeneration : on sauvegarde
    // exactement ce que Generate utiliserait si on cliquait dessus.
    // numCounterpoints vient du nombre de voix choisi : rien n'est figé
    // à un nombre précis de contrepoints.

    int numVoices = numVoicesCB.getSelectedId();
    int numCounterpoints = numVoices - 1;

    CantusProblem::Voices v;
    v.cf = cf;

    auto& voiceSettings = appController.getVoiceSettings();
    for (int i = 0; i < numCounterpoints; ++i)
    {
        CantusProblem::Counterpoint cp;
        cp.species = voiceSettings[i].species;
        cp.type    = voiceSettings[i].type;
        v.counterpoints.push_back(cp);
    }

    auto& problem = appController.getProblem();
    problem.setVoices(v);
    problem.setVoiceCount(numVoices);

    // =========================
    //  DEMANDE DU NOM
    // =========================
    // JUCE ne propose pas de boîte "texte à saisir" toute prête : on
    // construit donc l'AlertWindow à la main, avec un seul champ texte.
    auto* nameWindow = new juce::AlertWindow(
        juce::String::fromUTF8("Save configuration"),
        juce::String::fromUTF8("Donnez un nom à cette configuration :"),
        juce::AlertWindow::QuestionIcon);

    nameWindow->addTextEditor("configName", juce::String::fromUTF8("Ma configuration"));
    nameWindow->addButton(juce::String::fromUTF8("Enregistrer"), 1, juce::KeyPress(juce::KeyPress::returnKey));
    nameWindow->addButton(juce::String::fromUTF8("Annuler"), 0, juce::KeyPress(juce::KeyPress::escapeKey));

    nameWindow->enterModalState(true, juce::ModalCallbackFunction::create(
        [this, nameWindow](int result)
        {
            // La fenêtre doit être détruite nous-mêmes une fois le résultat
            // lu : unique_ptr s'en charge à la sortie de ce callback.
            std::unique_ptr<juce::AlertWindow> windowToDelete(nameWindow);

            if (result != 1)
                return;

            auto name = nameWindow->getTextEditorContents("configName").trim();

            if (name.isEmpty())
                name = juce::String::fromUTF8("Configuration sans nom");

            const bool success = appController.saveConfiguration(name);

            showAlert(success ? juce::AlertWindow::InfoIcon : juce::AlertWindow::WarningIcon,
                      juce::String::fromUTF8("Save configuration"),
                      success
                          ? juce::String::fromUTF8("Configuration « ") + name + juce::String::fromUTF8(" » enregistrée.")
                          : juce::String::fromUTF8("Impossible d'enregistrer la configuration."));
        }));
}


//==============================================================================
// RESULTAT : MIDI
//==============================================================================

/*
//==============================================================================
   Réception d'un fichier MIDI généré.

   Associe le fichier MIDI au composant de drag & drop,
   affiche l'icône MIDI dans l'interface et permet ensuite
   à l'utilisateur de le glisser vers un logiciel externe.
//=============================================================================
*/
void LeftPanel::onGenerationFinished(const juce::File& file)
{
    midiItem = std::make_unique<MidiFileItem>();
    midiItem->file = file;

    addAndMakeVisible(midiItem.get());
    resized();
}


//==============================================================================
// OUTILS UI
//==============================================================================

/*
//==============================================================================
   Préparation du fichier MIDI de sortie.

   Génère un chemin unique dans le dossier temporaire
   afin d'éviter tout conflit entre plusieurs générations.
//==============================================================================
*/
void LeftPanel::prepareOutputFile()
{
    auto tempDir = juce::File::getSpecialLocation(
        juce::File::tempDirectory);

    auto timestamp = juce::Time::getCurrentTime().toMilliseconds();

    midiOutFileToGenerate = tempDir.getChildFile(
        "FuxCP_Solution_" + juce::String(timestamp) + ".mid");
}

/*
//==============================================================================
 Affichage d'un message utilisateur.

 Affiche une fenêtre d'information, d'avertissement
 ou d'erreur selon le contexte.
//==============================================================================
*/
void LeftPanel::showAlert(juce::AlertWindow::AlertIconType icon,
                         const juce::String& title,
                         const juce::String& message)
{
    juce::AlertWindow::showMessageBoxAsync(icon, title, message);
}

void LeftPanel::resized()
{
    auto area = getLocalBounds().reduced(2);
    area.removeFromTop(8);

    const int spacing = juce::jlimit(10, 18, area.getHeight() / 42);
    const float widthRatio = 0.88f;
    const int titleReservedHeight = juce::jlimit(24, 30, area.getHeight() / 18);
    const int rowHeight = juce::jlimit(20, 24, area.getHeight() / 28);
    const int numSections = 3;

    juce::FlexBox sectionColumn;
    sectionColumn.flexDirection = juce::FlexBox::Direction::column;
    sectionColumn.alignItems = juce::FlexBox::AlignItems::stretch;

    for (int i = 0; i < numSections; ++i)
    {
        const float bottomMargin = i == numSections - 1 ? 0.0f : (float) spacing;

        sectionColumn.items.add(juce::FlexItem()
            .withFlex(1.0f)
            .withMargin(juce::FlexItem::Margin(0.0f, 0.0f, bottomMargin, 0.0f)));
    }

    sectionColumn.performLayout(area);

    // ===== SECTION 1 : CF =====
    auto section1 = sectionColumn.items[0].currentBounds.toNearestInt();
    auto content1 = section1.reduced(12, 10);
    content1.removeFromTop(titleReservedHeight);

    {
        auto row = content1.removeFromTop(juce::jmax(rowHeight * 3, content1.getHeight() - 8));
        cfInput.setVisible(true);

        int width = static_cast<int>(row.getWidth() * widthRatio);
        int x = row.getX() + juce::jlimit(0, 10, row.getWidth() / 20);
        cfInput.setBounds(x, row.getY(), width, row.getHeight());
    }

    // ===== SECTION 2 : VOICES =====
    auto section2 = sectionColumn.items[1].currentBounds.toNearestInt();
    auto content2 = section2.reduced(12, 10);
    content2.removeFromTop(titleReservedHeight);

    {
        auto row = content2.removeFromTop(rowHeight);
        numVoicesCB.setVisible(true);

        int width = static_cast<int>(row.getWidth() * widthRatio);
        int x = row.getX() + juce::jlimit(0, 10, row.getWidth() / 20);

        numVoicesCB.setBounds(x, row.getY(), width, row.getHeight());
    }

    content2.removeFromTop(10);

    // ===== SECTION 3 : DRAG ZONE =====
    auto section3 = sectionColumn.items[2].currentBounds.toNearestInt();

    // ===== SECTION 3 : DRAG ZONE CONTENT =====
    auto content3 = section3.reduced(10, 15);

    // ===== DRAG ZONE (MIDI ITEM) =====
    if (midiItem != nullptr)
    {
        constexpr int midiItemWidth = 30;
        constexpr int midiItemHeight = 50;
        constexpr int midiItemVerticalOffset = 11;

        midiItem->setBounds(
            content3.getCentreX() - midiItemWidth / 2,
            content3.getCentreY() - midiItemHeight / 2 + midiItemVerticalOffset,
            midiItemWidth,
            midiItemHeight
        );
    }
}


/*
//==============================================================================
   Ajoute une note provenant du clavier virtuel
   à la fin du Cantus Firmus affiché
//==============================================================================
*/
void LeftPanel::addNoteFromKeyboard(int midiNote)
{
    auto noteStr = NoteConverter::midiToNoteName(midiNote);

    auto current = cfInput.getText();

    if (!current.isEmpty())
        current += " ";

    current += noteStr;

    cfInput.setText(current, juce::dontSendNotification);
}

/*
//==============================================================================
   Synchronise l'affichage du Cantus Firmus avec les données
   actuellement stockées dans le modèle.
//==============================================================================
*/
void LeftPanel::updateCantusDisplay()
{
    const auto& cf = appController.getProblem().getCantusFirmus();

    juce::String display;

    for (size_t i = 0; i < cf.size(); ++i)
    {
        display += NoteConverter::midiToNoteName(cf[i]);

        if (i + 1 < cf.size())
            display += " ";
    }

    cfInput.setText(display, juce::dontSendNotification);
}

/*
//==============================================================================
   Affiche un nombre de voix donné, comme si l'utilisateur l'avait
   sélectionné lui-même dans la ComboBox.

   sendNotification déclenche onChange, qui met à jour OptionsPanel et le
   workspace des contrepoints : c'est exactement la cascade voulue au
   chargement d'une configuration sauvegardée.
//==============================================================================
*/
void LeftPanel::setNumVoicesDisplay(int numVoices)
{
    numVoicesCB.setSelectedId(numVoices, juce::sendNotification);
}

/*
//==============================================================================
   Retire le fichier MIDI affiché dans la Drag Zone.

   Utilisé au chargement d'une configuration : l'ancien fichier MIDI généré
   ne correspond plus à la nouvelle configuration affichée.
//==============================================================================
*/
void LeftPanel::clearGeneratedMidiDisplay()
{
    midiItem.reset();
    resized();
    repaint();
}

/*
//==============================================================================
   Nettoie la saisie comme au lancement de l'application.

   Le Cantus Firmus, le nombre de voix, le MIDI affiché
   et les données musicales du modèle sont vidés.
//==============================================================================
*/
void LeftPanel::clearInputState()
{
    cfInput.clear();
    numVoicesCB.setSelectedId(0, juce::dontSendNotification);

    midiItem.reset();

    CantusProblem::Voices emptyVoices;
    auto& problem = appController.getProblem();
    problem.setVoices(emptyVoices);
    problem.setVoiceCount(0);

    resized();
    repaint();
}

/*
//==============================================================================
   Vérifie si la saisie est déjà vide.

   Les trois mêmes champs que clearInputState() remet à zéro sont vérifiés
   ici : Cantus Firmus, nombre de voix, fichier MIDI affiché. Si les trois
   sont déjà vides, alors appuyer sur Clear ne changerait rien à l'écran.
//==============================================================================
*/
bool LeftPanel::isAtInitialState() const
{
    const bool cantusFirmusIsEmpty = cfInput.getText().trim().isEmpty();
    const bool noVoiceCountSelected = numVoicesCB.getSelectedId() == 0;
    const bool noGeneratedMidiShown = midiItem == nullptr;

    return cantusFirmusIsEmpty && noVoiceCountSelected && noGeneratedMidiShown;
}

/*
//==============================================================================
   Met à jour l'affichage des voix dans l'OptionsPanel
   lorsque le nombre de voix change.
//==============================================================================
*/
void LeftPanel::updateVoiceSpeciesUI(int totalVoices)
{
    if (optionsPanel)
        optionsPanel->setNumVoices(totalVoices);
}

/*
//==============================================================================
   Connecte le LeftPanel au ValueTree de génération.

   Permet de recevoir automatiquement les changements
   d'état produits par l'AppController.
//==============================================================================
*/
void LeftPanel::connectToGenerationState(juce::ValueTree state)
{
    generationState = state;
    generationState.addListener(this);
}

/*
//==============================================================================
   Réagit aux changements de génération.

   Lorsque la génération est terminée, récupère le
   fichier MIDI produit et l'affiche dans la drag zone.
//==============================================================================
*/
void LeftPanel::valueTreePropertyChanged(juce::ValueTree& tree,
                                         const juce::Identifier& property)
{
    if (property != juce::Identifier("generationStatus"))
        return;

    auto status = tree.getProperty("generationStatus").toString();

    if (status == "completed")
    {
        auto midiPath = tree.getProperty("midiFilePath").toString();
        juce::File midiFile(midiPath);

        if (midiFile.existsAsFile())
            onGenerationFinished(midiFile);
    }
}
