#include "LeftPanel.h"
#include "../optionsPanel/OptionsPanel.h"
#include "../../controller/AppController.h"
#include "../../model/NoteConverter.h"


//==============================================================================
// PARSING : Cantus Firmus (texte → MIDI)
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

    const int spacing = 40;
    const int numSections = 3;

    int totalSpacing = spacing * (numSections - 1);
    int sectionHeight = (area.getHeight() - totalSpacing) / numSections;

    const int titleHeight = 24;

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
    auto& settings = appController.getVoiceSettings();

    for (int i = 0; i < numCounterpoints; ++i)
    {
        CantusProblem::Counterpoint cp;

        cp.species = settings[i].species;
        cp.type    = settings[i].type;

        v.counterpoints.push_back(cp);
    }



    auto& problem = appController.getProblem();
    problem.setVoices(v);
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

void LeftPanel::prepareOutputFile()
{
    auto tempDir = juce::File::getSpecialLocation(
        juce::File::tempDirectory);

    auto timestamp = juce::Time::getCurrentTime().toMilliseconds();

    midiOutFileToGenerate = tempDir.getChildFile(
        "FuxCP_Solution_" + juce::String(timestamp) + ".mid");
}

void LeftPanel::showAlert(juce::AlertWindow::AlertIconType icon,
                         const juce::String& title,
                         const juce::String& message)
{
    juce::AlertWindow::showMessageBoxAsync(icon, title, message);
}

void LeftPanel::resized()
{
    auto area = getLocalBounds();

    const int spacing = 40;
    const float widthRatio = 0.6f;
    const int titleReservedHeight = 30;

    int totalSpacing = spacing * 2;
    int sectionHeight = (area.getHeight() - totalSpacing) / 3;

    // ===== SECTION 1 : CF =====
    auto section1 = area.removeFromTop(sectionHeight);
    auto content1 = section1.reduced(10);
    content1.removeFromTop(titleReservedHeight);

    {
        auto row = content1.removeFromTop(22);
        bool canShowText = row.getBottom() <= section1.getBottom() && sectionHeight > 50;
        cfInput.setVisible(canShowText);

        if (canShowText)
        {
            int width = static_cast<int>(row.getWidth() * widthRatio);
            int x = row.getX() + 10;
            cfInput.setBounds(x, row.getY(), width, row.getHeight());
        }
    }

    area.removeFromTop(spacing);

    // ===== SECTION 3 : DRAG ZONE =====
    auto section3 = area.removeFromBottom(sectionHeight);

    // ===== SECTION 2 : VOICES =====
    auto section2 = area;
    auto content2 = section2.reduced(10);
    content2.removeFromTop(titleReservedHeight);

    {
        auto row = content2.removeFromTop(22);

        bool canShowVoices = row.getBottom() <= section2.getBottom()
                             && sectionHeight > 50;

        numVoicesCB.setVisible(canShowVoices);

        if (canShowVoices)
        {
            int width = static_cast<int>(row.getWidth() * widthRatio);
            int x = row.getX() + 10;

            numVoicesCB.setBounds(x, row.getY(), width, row.getHeight());
        }
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

void LeftPanel::addNoteFromKeyboard(int midiNote)
{
    auto noteStr = NoteConverter::midiToNoteName(midiNote);

    auto current = cfInput.getText();

    if (!current.isEmpty())
        current += " ";

    current += noteStr;

    cfInput.setText(current, juce::dontSendNotification);
}

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

void LeftPanel::updateVoiceSpeciesUI(int totalVoices)
{
    if (optionsPanel)
        optionsPanel->setNumVoices(totalVoices);
}