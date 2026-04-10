#include "LeftPanel.h"
#include "../optionsWindow/OptionsPanel.h"
#include "../../controller/AppController.h"

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


    moreOptions.setButtonText("More Options");
    addAndMakeVisible(moreOptions);


    moreOptions.onClick = [this]()
    {

        auto* content  = new OptionsPanel();

        content->setVoiceSettings(appController.getVoiceSettings());

        int numVoices = voices.getSelectedId();

        content->setNumVoices(numVoices);

        content->setVoiceSettings(appController.getVoiceSettings());

        juce::DialogWindow::LaunchOptions dialog;

        dialog.content.setOwned(content);
        dialog.dialogTitle = "More Options";
        dialog.dialogBackgroundColour = juce::Colours::darkgrey;

        dialog.escapeKeyTriggersCloseButton = true;
        dialog.useNativeTitleBar = true;
        dialog.resizable = true;

        dialog.launchAsync();
    };

    generateButton.setButtonText("Generate");
    addAndMakeVisible(generateButton);

    generateButton.onClick = [this]()
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

        // =========================
        // MISE À JOUR DU MODÈLE
        // =========================

        auto& problem = appController.getProblem();

        problem.setCantusFirmus(cf);

        std::vector<CantusProblem::Voice> voicesVec;


        int numGeneratedVoices = numVoices - 1;

        for (int i = 0; i < speciesBoxes.size(); ++i)
        {
            CantusProblem::Voice v;
            v.id = i;


            v.species = speciesBoxes[i]->getSelectedId();

            if (i < typeBoxes.size()) {
                int id = typeBoxes[i]->getSelectedId();
                v.type = id - 4;
            }
            else
                v.type = 1;

            DBG("Voice " << i
                << " species=" << v.species
                << " type=" << v.type);

            voicesVec.push_back(v);
        }

        problem.setVoices(voicesVec);


        appController.startGeneration("dummy.mid");
    };

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
    auto desktop = juce::File::getSpecialLocation(
        juce::File::userDesktopDirectory);

    midiOutFileToGenerate = desktop.getChildFile(
        "FuxCP_Solution_" +
        juce::Time::getCurrentTime().formatted("%Y%m%d_%H%M%S") +
        ".mid");
}



LeftPanel::~LeftPanel()
{
    //le thread solver est terminé avant destruction
    //generationService.stopThread(-1);
    generationService.stopThread(2000);


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
        "Cantus Firmus",
        "Number of voices",
        "Buttons"
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
    const int titleReservedHeight = 30; // L'espace qu'on laisse pour le titre

    int totalSpacing = spacing * 2;
    int sectionHeight = (area.getHeight() - totalSpacing) / 3;

    // ===== SECTION 1 : CF =====
    auto section1 = area.removeFromTop(sectionHeight);
    auto content1 = section1.reduced(10);
    content1.removeFromTop(titleReservedHeight);

    {
        auto row = content1.removeFromTop(22);
        // CONDITION : On ne montre le texte que si la ligne tient dans la section
        bool canShowText = row.getBottom() <= section1.getBottom() && sectionHeight > 50;
        text.setVisible(canShowText);

        if (canShowText) {
            int width = static_cast<int>(row.getWidth() * widthRatio);
            int x = row.getX() + 10;
            text.setBounds(x, row.getY(), width, row.getHeight());
        }
    }

    area.removeFromTop(spacing);

    // ===== SECTION 3 : BOUTONS =====
    auto section3 = area.removeFromBottom(sectionHeight);

    // ===== SECTION 2 : VOICES + PARAMÈTRES =====
    auto section2 = area; // C'est le reste (le milieu)
    auto content2 = section2.reduced(10);
    content2.removeFromTop(titleReservedHeight);

    // COMBOBOX : Number of voices
    {
        auto row = content2.removeFromTop(22);
        // CONDITION : On cache si ça dépasse de la section 2
        bool canShowVoices = row.getBottom() <= section2.getBottom() && sectionHeight > 50;
        voices.setVisible(canShowVoices);

        if (canShowVoices) {
            int width = static_cast<int>(row.getWidth() * widthRatio);
            int x = row.getX() + 10;
            voices.setBounds(x, row.getY(), width, row.getHeight());
        }
    }

    content2.removeFromTop(10);

    // ZONE DYNAMIQUE (Espèces / Types)
    auto dynamicArea = content2;
    int numRows = speciesLabels.size();

    int labelHeight = 18;
    int boxHeight = 22;
    int rowGap = 6;
    int y = dynamicArea.getY();

    if (numRows > 0) {
        // On vérifie d'abord si le Header tient
        bool canShowHeader = (y + labelHeight <= section2.getBottom()) && voices.isVisible();
        speciesHeader.setVisible(canShowHeader);
        typeHeader.setVisible(canShowHeader);

        if (canShowHeader) {
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

            for (int i = 0; i < numRows; ++i) {

                bool rowVisible = (y + boxHeight <= section2.getBottom());
                speciesLabels[i]->setVisible(rowVisible);
                speciesBoxes[i]->setVisible(rowVisible);
                typeBoxes[i]->setVisible(rowVisible);

                if (rowVisible) {
                    speciesLabels[i]->setBounds(leftX, y, labelWidth, labelHeight);
                    speciesBoxes[i]->setBounds(speciesX, y, boxWidth, boxHeight);
                    typeBoxes[i]->setBounds(typeX, y, boxWidth, boxHeight);
                }
                y += boxHeight + rowGap;
            }
        } else {
            // Cacher les lignes si le header ne passe pas
            for (int i = 0; i < numRows; ++i) {
                speciesLabels[i]->setVisible(false);
                speciesBoxes[i]->setVisible(false);
                typeBoxes[i]->setVisible(false);
            }
        }
    }

    // ===== SECTION 3 : LOGIQUE DES BOUTONS =====
    auto content3 = section3.reduced(10, 15);
    int bHeight = 22;
    int gap = 5;
    int totalReq = (bHeight * 2) + gap;

    bool showButtons = (content3.getHeight() >= totalReq) && (sectionHeight > 60);

    moreOptions.setVisible(showButtons);
    generateButton.setVisible(showButtons);

    if (showButtons) {
        int bWidth = static_cast<int>(content3.getWidth() * widthRatio);
        int startY = content3.getY() + (content3.getHeight() - totalReq) / 2;
        int bX = content3.getX() + (content3.getWidth() - bWidth) / 2;

        moreOptions.setBounds(bX, startY, bWidth, bHeight);
        generateButton.setBounds(bX, startY + bHeight + gap, bWidth, bHeight);
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

        if (!t.containsOnly("0123456789"))
            return {}; // invalide → on rejette tout

        int value = t.getIntValue();

        if (value < 0 || value > 127)
            return {}; // hors MIDI

        result.push_back(value);
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
        };

        auto* typeBox = new juce::ComboBox();

        // Exemple simple (à adapter selon ton modèle)
        typeBox->addItem("-3", 1);
        typeBox->addItem("-2", 2);
        typeBox->addItem("-1", 3);
        typeBox->addItem("0", 4);
        typeBox->addItem("1", 5);
        typeBox->addItem("2", 6);
        typeBox->addItem("3", 7);


        typeBox->setSelectedId(settings[i].type + 4);

        addAndMakeVisible(typeBox);
        typeBoxes.add(typeBox);

        //  liaison modèle -> UI
        typeBox->onChange = [this, i, typeBox]()
        {
            auto& settings = appController.getVoiceSettings();
            settings[i].type = typeBox->getSelectedId() - 4;
        };
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