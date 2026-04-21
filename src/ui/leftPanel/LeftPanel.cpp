#include "LeftPanel.h"
#include "../optionsWindow/OptionsPanel.h"
#include "../../controller/AppController.h"
#include "../../model/NoteConverter.h"

// =============================
//  parsing du CF
// =============================


static std::vector<int> parseCantusFirmus(const juce::String& text);

// =============================
// Messages UI
// =============================

namespace Messages
{
    static const juce::String titleError   = juce::String::fromUTF8(u8"Erreur");
    static const juce::String titleSuccess = juce::String::fromUTF8(u8"Succès");

    static const juce::String cfEmpty      = juce::String::fromUTF8(u8"Le Cantus Firmus est vide.");
    static const juce::String cfNotNumbers = juce::String::fromUTF8(u8"Le Cantus Firmus ne doit contenir que des nombres (0-127).");
    static const juce::String cfInvalid    = juce::String::fromUTF8(u8"Le Cantus Firmus est invalide (0-127).");

    static const juce::String noVoices     = juce::String::fromUTF8(u8"Veuillez sélectionner un nombre de voix.");
    static const juce::String noSpecies    = juce::String::fromUTF8(u8"Veuillez sélectionner une espèce.");

    static const juce::String noSolution   = juce::String::fromUTF8(u8"Aucune solution trouvée");
}


LeftPanel::LeftPanel(AppController& controller)
    : appController(controller)  {
    label.setText("Cantus Firmus (MIDI)",juce::sendNotification);
    addAndMakeVisible(text);
    //addAndMakeVisible(label);


    voices.addItem(("2"), 2);
    voices.addItem(("3"), 3);
    voices.addItem(("4"), 4);

    labelVoices.setText("Number of voices",juce::sendNotification);
    addAndMakeVisible(voices);
    //addAndMakeVisible(labelVoices);




    // Réaction au changement du nombre de voix
    voices.onChange = [this]()
    {
        int numVoices = voices.getSelectedId();
        updateVoiceSpeciesUI(numVoices);
    };

    updateVoiceSpeciesUI(0);

    speciesHeader.setText(
        juce::String::fromUTF8(u8"Espèce"),
        juce::dontSendNotification
    );
    typeHeader.setText("Type", juce::dontSendNotification);

    addAndMakeVisible(speciesHeader);
    addAndMakeVisible(typeHeader);

}



// =============================
// Fichier MIDI de sortie
// =============================

void LeftPanel::prepareOutputFile()
{
    auto tempDir = juce::File::getSpecialLocation(
        juce::File::tempDirectory);

    auto timestamp = juce::Time::getCurrentTime().toMilliseconds();

    midiOutFileToGenerate = tempDir.getChildFile(
        "FuxCP_Solution_" + juce::String(timestamp) + ".mid");
}



LeftPanel::~LeftPanel()
{
    generationService.stopThread(2000); // thread destructeur qui assure la generation (IMPORTANT)
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
        text.setVisible(canShowText);

        if (canShowText)
        {
            int width = static_cast<int>(row.getWidth() * widthRatio);
            int x = row.getX() + 10;
            text.setBounds(x, row.getY(), width, row.getHeight());
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
        bool canShowVoices = row.getBottom() <= section2.getBottom() && sectionHeight > 50;
        voices.setVisible(canShowVoices);

        if (canShowVoices)
        {
            int width = static_cast<int>(row.getWidth() * widthRatio);
            int x = row.getX() + 10;
            voices.setBounds(x, row.getY(), width, row.getHeight());
        }
    }

    content2.removeFromTop(10);

    // ===== ZONE DYNAMIQUE =====
    auto dynamicArea = content2;
    int numRows = speciesLabels.size();

    int labelHeight = 18;
    int boxHeight = 22;
    int rowGap = 6;
    int y = dynamicArea.getY();

    if (numRows > 0)
    {
        bool canShowHeader = (y + labelHeight <= section2.getBottom()) && voices.isVisible();
        speciesHeader.setVisible(canShowHeader);
        typeHeader.setVisible(canShowHeader);

        if (canShowHeader)
        {
            float labelRatio = 0.4f;
            float boxRatio = 0.2f;

            int labelWidth = static_cast<int>(dynamicArea.getWidth() * labelRatio);
            int boxWidth = static_cast<int>(dynamicArea.getWidth() * boxRatio);

            int leftX = dynamicArea.getX() + 10;
            int speciesX = leftX + labelWidth + 10;
            int typeX = speciesX + boxWidth + 10;

            speciesHeader.setBounds(speciesX, y, boxWidth, labelHeight);
            typeHeader.setBounds(typeX, y, boxWidth, labelHeight);

            y += labelHeight + rowGap;

            for (int i = 0; i < numRows; ++i)
            {
                bool rowVisible = (y + boxHeight <= section2.getBottom());

                speciesLabels[i]->setVisible(rowVisible);
                speciesBoxes[i]->setVisible(rowVisible);
                typeBoxes[i]->setVisible(rowVisible);

                if (rowVisible)
                {
                    speciesLabels[i]->setBounds(leftX, y, labelWidth, labelHeight);
                    speciesBoxes[i]->setBounds(speciesX, y, boxWidth, boxHeight);
                    typeBoxes[i]->setBounds(typeX, y, boxWidth, boxHeight);
                }

                y += boxHeight + rowGap;
            }
        }
        else
        {
            for (int i = 0; i < numRows; ++i)
            {
                speciesLabels[i]->setVisible(false);
                speciesBoxes[i]->setVisible(false);
                typeBoxes[i]->setVisible(false);
            }
        }
    }

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

// =============================
// Parsing du Cantus Firmus
// =============================
static std::vector<int> parseCantusFirmus(const juce::String& text)
{
    std::vector<int> result;

    if (text.isEmpty())
        return result;

    auto tokens = juce::StringArray::fromTokens(text, " ,;", "\"");

    for (const auto& t : tokens)
    {
        if (t.isEmpty())
            return {}; // évite cas "60,,64"

        if (t.containsOnly("0123456789"))
        {
            // notes musicales (chiffres)
            int value = t.getIntValue();

            if (value < 0 || value > 127)
                return {};

            result.push_back(value);
        }
        else
        {
            // notes musicales (lettres)
            int midi = NoteConverter::noteNameToMidi(t);

            if (midi == -1)
                return {};

            result.push_back(midi);
        }
    }

    return result;
}

void LeftPanel::updateVoiceSpeciesUI(int numVoices)
{
    for (auto* box : speciesBoxes)
        removeChildComponent(box);

    for (auto* label : speciesLabels)
        removeChildComponent(label);

    speciesBoxes.clear();
    speciesLabels.clear();

    for (auto* box : typeBoxes)
        removeChildComponent(box);

    typeBoxes.clear();

    auto& settings = appController.getVoiceSettings();
    settings.resize(numVoices);

    for (int i = 0; i < numVoices; ++i)
    {
        settings[i].species = 1;
        settings[i].type = -3;
    }

    for (int i = 0; i < numVoices; ++i)
    {
        auto* rowLabel = new juce::Label();

        if (i == 0)
            rowLabel->setText(juce::String::fromUTF8(u8"Cantus Firmus"),
                              juce::dontSendNotification);
        else
            rowLabel->setText("Contrepoint " + juce::String(i) + "",
                              juce::dontSendNotification);

        addAndMakeVisible(rowLabel);
        speciesLabels.add(rowLabel);

        auto* speciesBox = new juce::ComboBox();
        speciesBox->addItem("1", 1);
        speciesBox->addItem("2", 2);
        speciesBox->addItem("3", 3);
        speciesBox->addItem("4", 4);
        speciesBox->addItem("5", 5);

        speciesBox->setSelectedId(settings[i].species);

        addAndMakeVisible(speciesBox);
        speciesBoxes.add(speciesBox);

        //  liaison modèle -> UI
        speciesBox->onChange = [this, i, speciesBox]()
        {
            auto& settings = appController.getVoiceSettings();
            settings[i].species = speciesBox->getSelectedId();

            if (optionsPanel)
            {
                optionsPanel->setVoiceSettings(settings);
            }
        };

        auto* typeBox = new juce::ComboBox();

        // Exemple simple (à adapter selon ton modèle)
        typeBox->addItem("-3", 1);
        typeBox->addItem("-2", 2);
        typeBox->addItem("-1", 3);
        typeBox->addItem("0", 4);
        typeBox->addItem("1", 5);
        typeBox->addItem("2", 6);



        typeBox->setSelectedId(settings[i].type + 4);

        addAndMakeVisible(typeBox);
        typeBoxes.add(typeBox);

        //  liaison modèle -> UI
        typeBox->onChange = [this, i, typeBox]()
        {
            auto& settings = appController.getVoiceSettings();
            settings[i].type = typeBox->getSelectedId() - 4;

            if (optionsPanel)
            {
                optionsPanel->setVoiceSettings(settings);
            }
        };
    }

    if (optionsPanel)
    {
        optionsPanel->setNumVoices(numVoices);
        optionsPanel->setVoiceSettings(appController.getVoiceSettings());
    }

    resized();
}



void LeftPanel::showAlert(juce::AlertWindow::AlertIconType icon,
                         const juce::String& title,
                         const juce::String& message)
{
    juce::AlertWindow::showMessageBoxAsync(icon, title, message);
}

juce::String LeftPanel::getCantusText() const
{
    return text.getText();
}

void LeftPanel::setCantusText(const juce::String& newText)
{
    text.setText(newText, juce::dontSendNotification);
}

void LeftPanel::triggerGeneration()
{
    juce::String error;

    auto rawText = text.getText().trim();

    if (rawText.isEmpty())
    {
        showAlert(juce::AlertWindow::WarningIcon, Messages::titleError, Messages::cfEmpty);
        return;
    }

    if (!rawText.containsOnly("0123456789 ,;"))
    {
        showAlert(juce::AlertWindow::WarningIcon, Messages::titleError, Messages::cfNotNumbers);
        return;
    }

    auto cf = parseCantusFirmus(rawText);

    if (cf.empty())
    {
        showAlert(juce::AlertWindow::WarningIcon, Messages::titleError, Messages::cfInvalid);
        return;
    }

    if (voices.getSelectedItemIndex() == -1)
    {
        showAlert(juce::AlertWindow::WarningIcon, Messages::titleError, Messages::noVoices);
        return;
    }

    int numVoices = voices.getSelectedId();

    auto& problem = appController.getProblem();
    problem.setCantusFirmus(cf);

    std::vector<CantusProblem::Voice> voicesVec;

    for (int i = 0; i < speciesBoxes.size(); ++i)
    {
        CantusProblem::Voice v;
        v.id = i;

        v.species = speciesBoxes[i]->getSelectedId();

        if (i < typeBoxes.size())
            v.type = typeBoxes[i]->getSelectedId() - 4;
        else
            v.type = 1;

        voicesVec.push_back(v);
    }

    problem.setVoices(voicesVec);

    // =========================
    // Préparer le fichier (avant la génération)
    // =========================
    prepareOutputFile();

    // =========================
    // Lancer génération
    // =========================
    appController.startGeneration(midiOutFileToGenerate.getFullPathName());

}

void LeftPanel::onGenerationFinished(const juce::File& file)
{
    midiItem = std::make_unique<MidiFileItem>();
    midiItem->file = file;

    addAndMakeVisible(midiItem.get());
    resized();
}

void LeftPanel::refreshFromModel()
{
    auto& settings = appController.getVoiceSettings();

    for (int i = 0; i < speciesBoxes.size(); ++i)
    {
        if (i < settings.size())
        {
            speciesBoxes[i]->setSelectedId(settings[i].species, juce::dontSendNotification);
            typeBoxes[i]->setSelectedId(settings[i].type + 4, juce::dontSendNotification);
        }
    }
}

void LeftPanel::addNoteFromKeyboard(int midiNote)
{
    auto noteStr = NoteConverter::midiToNoteName(midiNote);

    auto current = text.getText();

    if (!current.isEmpty())
        current += " ";

    current += noteStr;

    text.setText(current, juce::dontSendNotification);
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

    text.setText(display, juce::dontSendNotification);
}