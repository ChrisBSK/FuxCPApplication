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
}
//==============================================================================
// PARSING : Cantus Firmus (texte --> MIDI)
//==============================================================================

static std::vector<int> parseCantusFirmus(const juce::String& text)
{
    std::vector<int> result;

    if (text.isEmpty())
        return result;

    auto tokens = juce::StringArray::fromTokens(text, " ,;", "\"");

    for (const auto& t : tokens)
    {
        if (t.isEmpty())
            return {};

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
    addAndMakeVisible(cfInput);

    // =========================
    // Nombre de voix (CF inclus)
    // =========================
    numVoicesCBLabel.setText("Number of voices",juce::sendNotification);
    numVoicesCB.addItem("2", 2);
    numVoicesCB.addItem("3", 3);
    numVoicesCB.addItem("4", 4);
    addAndMakeVisible(numVoicesCB);


    // Réaction au changement du nombre de voix
    numVoicesCB.onChange = [this]()
    {
        updateVoiceSpeciesUI(numVoicesCB.getSelectedId());
    };


}

void LeftPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);

    auto area = getLocalBounds();

    const int spacing = juce::jlimit(12, 40, area.getHeight() / 16);
    const int numSections = 3;

    int totalSpacing = spacing * (numSections - 1);
    int sectionHeight = juce::jmax(1, (area.getHeight() - totalSpacing) / numSections);

    const int titleHeight = juce::jlimit(20, 24, area.getHeight() / 22);

    juce::Colour darkGreen = juce::Colour(0xff2f4f4f);

    juce::String titles[3] =
    {
        juce::String::fromUTF8("Cantus Firmus"),
        juce::String::fromUTF8("Number of voices"),
        juce::String::fromUTF8("Drag Zone")
    };

    for (int i = 0; i < numSections; ++i)
    {
        auto section = area.removeFromTop(sectionHeight);

        // uniquement la barre de titre
        auto titleArea = section.removeFromTop(titleHeight);

        g.setColour(darkGreen);
        g.fillRect(titleArea);

        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        g.drawText(titles[i], titleArea.reduced(10, 0),
                   juce::Justification::centredLeft);

        // petite ligne séparatrice
        g.setColour(juce::Colours::black.withAlpha(0.3f));
        g.drawLine((float)section.getX(),
                   (float)section.getY(),
                   (float)section.getRight(),
                   (float)section.getY(),
                   1.0f);

        if (i < numSections - 1)
            area.removeFromTop(spacing);
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
    // (espèce et type pour chaque voix).
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
    auto area = getLocalBounds();

    const int spacing = juce::jlimit(12, 40, area.getHeight() / 16);
    const float widthRatio = 0.78f;
    const int titleReservedHeight = juce::jlimit(24, 30, area.getHeight() / 18);
    const int rowHeight = juce::jlimit(20, 24, area.getHeight() / 28);

    int totalSpacing = spacing * 2;
    int sectionHeight = juce::jmax(1, (area.getHeight() - totalSpacing) / 3);

    // ===== SECTION 1 : CF =====
    auto section1 = area.removeFromTop(sectionHeight);
    auto content1 = section1.reduced(10);
    content1.removeFromTop(titleReservedHeight);

    {
        auto row = content1.removeFromTop(rowHeight);
        cfInput.setVisible(true);

        int width = static_cast<int>(row.getWidth() * widthRatio);
        int x = row.getX() + juce::jlimit(0, 10, row.getWidth() / 20);
        cfInput.setBounds(x, row.getY(), width, row.getHeight());
    }

    area.removeFromTop(spacing);

    // ===== SECTION 3 : DRAG ZONE =====
    auto section3 = area.removeFromBottom(sectionHeight);

    // ===== SECTION 2 : VOICES =====
    auto section2 = area;
    auto content2 = section2.reduced(10);
    content2.removeFromTop(titleReservedHeight);

    {
        auto row = content2.removeFromTop(rowHeight);
        numVoicesCB.setVisible(true);

        int width = static_cast<int>(row.getWidth() * widthRatio);
        int x = row.getX() + juce::jlimit(0, 10, row.getWidth() / 20);

        numVoicesCB.setBounds(x, row.getY(), width, row.getHeight());
    }

    content2.removeFromTop(10);

    // ===== SECTION 3 : DRAG ZONE CONTENT =====
    auto content3 = section3.reduced(10, 15);

    // ===== DRAG ZONE (MIDI ITEM) =====
    if (midiItem != nullptr)
    {
        midiItem->setBounds(
            content3.getCentreX() - 40,
            content3.getCentreY() - 25,
            80,
            50
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
