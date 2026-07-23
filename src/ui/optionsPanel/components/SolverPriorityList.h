#pragma once

#include <algorithm>
#include <array>
#include <functional>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

/*
//==============================================================================
   SolverPriorityList

   Affiche les priorités du vecteur importance envoyé au solveur.

   Chaque ligne correspond à une position précise du vecteur :
   - l'ordre visuel est le même que l'ordre attendu par FuxCP
   - les noms restent courts pour préparer l'ajout futur de contrôles
//==============================================================================
*/
class SolverPriorityList : public juce::Component
{
public:
    SolverPriorityList()
    {
        for (size_t i = 0; i < rows.size(); ++i)
        {
            rows[i].onClick = [this, i]()
            {
                selectRow(static_cast<int>(i));
            };

            addAndMakeVisible(rows[i]);
        }

        refreshRows();
    }

    std::function<void(const std::vector<int>&)> onPriorityOrderChanged;

    // Active une version plus dense de la liste pour les colonnes etroites.
    void setCompactMode(bool shouldUseCompactMode)
    {
        compactMode = shouldUseCompactMode;

        for (auto& row : rows)
            row.setCompactMode(compactMode);

        resized();
        repaint();
    }

    /*
        Charge le vecteur importance venant du modèle.
        FuxCP stocke un rang par contrainte ; l'interface affiche l'ordre trié.
    */
    void setImportanceCosts(const std::vector<int>& importanceCosts)
    {
        if (importanceCosts.size() != priorityCount)
            return;

        std::array<size_t, priorityCount> indexes {};

        for (size_t i = 0; i < indexes.size(); ++i)
            indexes[i] = i;

        std::sort(indexes.begin(), indexes.end(),
                  [&importanceCosts](size_t left, size_t right)
                  {
                      return importanceCosts[left] < importanceCosts[right];
                  });

        for (size_t rank = 0; rank < priorityCount; ++rank)
            priorityNames[rank] = fixedPriorityNames[indexes[rank]];

        selectedIndex = -1;
        refreshRows();
    }

    /*
        Convertit l'ordre affiché en vecteur importance pour FuxCP.
        La position visuelle 1 devient la valeur 1 dans le bon index FuxCP.
    */
    std::vector<int> getImportanceCosts() const
    {
        std::vector<int> importanceCosts(priorityCount, 0);

        for (size_t rank = 0; rank < priorityNames.size(); ++rank)
        {
            const int fuxIndex = findPriorityIndex(priorityNames[rank]);

            if (fuxIndex >= 0)
                importanceCosts[static_cast<size_t>(fuxIndex)] = static_cast<int>(rank + 1);
        }

        return importanceCosts;
    }

    /*
        Monte la priorité sélectionnée d'un cran.
        La numérotation reste fixe : seule la contrainte change de place.
        Si la première ligne monte, elle revient en dernière position.
    */
    void moveSelectedUp()
    {
        if (selectedIndex < 0)
            return;

        const int lastIndex = static_cast<int>(priorityNames.size()) - 1;

        if (selectedIndex == 0)
        {
            moveFirstPriorityToEnd();
            selectedIndex = lastIndex;
        }
        else
        {
            swapSelectedPriorityWith(selectedIndex - 1);
            --selectedIndex;
        }

        refreshRows();
        notifyPriorityOrderChanged();
    }

    /*
        Descend la priorité sélectionnée d'un cran.
        La numérotation reste fixe : seule la contrainte change de place.
        Si la dernière ligne descend, elle revient en première position.
    */
    void moveSelectedDown()
    {
        const int lastIndex = static_cast<int>(priorityNames.size()) - 1;

        if (selectedIndex < 0)
            return;

        if (selectedIndex == lastIndex)
        {
            moveLastPriorityToStart();
            selectedIndex = 0;
        }
        else
        {
            swapSelectedPriorityWith(selectedIndex + 1);
            ++selectedIndex;
        }

        refreshRows();
        notifyPriorityOrderChanged();
    }

    void resized() override
    {
        const int inset = compactMode ? 3 : juce::jlimit(6, 12, getWidth() / 18);
        const int spacingY = compactMode ? 1 : juce::jlimit(2, 4, getHeight() / 130);

        auto area = getLocalBounds().reduced(inset);

        const int rowCount = static_cast<int>(rows.size());
        const int availableHeight = area.getHeight()
                                  - juce::jmax(0, rowCount - 1) * spacingY;
        const int rowHeight = rowCount > 0
            ? juce::jmax(compactMode ? 12 : 18, availableHeight / rowCount)
            : compactMode ? 12 : 18;

        for (auto& row : rows)
        {
            row.setBounds(area.removeFromTop(rowHeight));
            area.removeFromTop(spacingY);
        }
    }

private:
    static constexpr size_t priorityCount = 14;

    static constexpr std::array<const char*, priorityCount> fixedPriorityNames
    {{
        "borrow",
        "fifth",
        "octave",
        "succ",
        "variety",
        "triad",
        "direct",
        "motion",
        "penult",
        "cambiata",
        "triad3",
        "m2",
        "syncopation",
        "melodic"
    }};

    /*
        Ligne visuelle d'une priorité :
        - bulle à gauche : rang d'importance visible par l'utilisateur
        - rectangle à droite : nom court de la priorité FuxCP 
    */
    class PriorityRow : public juce::Component
    {
    public:
        std::function<void()> onClick;

        void setContent(int newRank, juce::String newName)
        {
            rank = newRank;
            name = std::move(newName);
            repaint();
        }

        void setSelected(bool shouldBeSelected)
        {
            isSelected = shouldBeSelected;
            repaint();
        }

        void setCompactMode(bool shouldUseCompactMode)
        {
            compactMode = shouldUseCompactMode;
            repaint();
        }

        void paint(juce::Graphics& g) override
        {
            auto area = getLocalBounds();

            const int gap = compactMode ? 3 : juce::jlimit(4, 7, area.getWidth() / 28);
            const int bubbleSize = compactMode
                ? juce::jlimit(12, 16, area.getHeight())
                : juce::jlimit(18, 24, area.getHeight());

            auto bubble = area.removeFromLeft(bubbleSize).withSizeKeepingCentre(
                bubbleSize,
                bubbleSize
            );

            area.removeFromLeft(gap);

            drawRankBubble(g, bubble);
            drawPriorityName(g, area);
            drawSelection(g, getLocalBounds());
        }

        void mouseDown(const juce::MouseEvent&) override
        {
            if (onClick != nullptr)
                onClick();
        }

    private:
        /*
            Affiche une bande claire quand la ligne est sélectionnée.
            Elle reste transparente pour ne pas casser le style de la colonne.
        */
        void drawSelection(juce::Graphics& g, juce::Rectangle<int> bounds)
        {
            if (! isSelected)
                return;

            g.setColour(juce::Colours::white.withAlpha(0.18f));
            g.fillRoundedRectangle(bounds.toFloat(), compactMode ? 4.0f : 7.0f);
        }

        /*
            Dessine le rang 1..14 dans une bulle compacte.
            Ce chiffre sert de repère visuel pour l'ordre des priorités.
        */
        void drawRankBubble(juce::Graphics& g, juce::Rectangle<int> bubble)
        {
            g.setColour(juce::Colour(0xff203838));
            g.fillEllipse(bubble.toFloat());

            g.setColour(juce::Colours::white);
            g.setFont(juce::FontOptions(
                compactMode ? juce::jlimit(7.0f, 9.0f, bubble.getHeight() * 0.56f)
                            : juce::jlimit(10.0f, 13.0f, bubble.getHeight() * 0.55f),
                juce::Font::bold
            ));
            g.drawFittedText(juce::String(rank),
                             bubble,
                             juce::Justification::centred,
                             1);
        }

        /*
            Dessine le nom de la priorité dans le même style que les labels
            des paramètres
        */
        void drawPriorityName(juce::Graphics& g, juce::Rectangle<int> bounds)
        {
            g.setColour(juce::Colour(0xff2f4f4f));
            g.fillRoundedRectangle(bounds.toFloat(), compactMode ? 4.0f : 6.0f);

            g.setColour(juce::Colours::white);
            g.setFont(juce::FontOptions(
                compactMode ? juce::jlimit(7.0f, 9.0f, bounds.getHeight() * 0.55f)
                            : juce::jlimit(10.0f, 14.0f, bounds.getHeight() * 0.52f),
                juce::Font::bold
            ));
            g.drawFittedText(name,
                             bounds.reduced(compactMode ? 3 : 5, 1),
                             juce::Justification::centred,
                             1);
        }

        int rank = 0;
        bool isSelected = false;
        bool compactMode = false;
        juce::String name;
    };

    /*
        Sélectionne une ligne avant de la déplacer.
        L'indice est interne : l'utilisateur voit seulement les bulles 1..14.
    */
    void selectRow(int index)
    {
        selectedIndex = index;
        refreshRows();
    }

    /*
        Retrouve l'index FuxCP d'une priorité à partir de son nom.
        Cet index est fixe, même quand l'ordre visuel change.
    */
    static int findPriorityIndex(const juce::String& priorityName)
    {
        for (size_t i = 0; i < fixedPriorityNames.size(); ++i)
        {
            if (priorityName == fixedPriorityNames[i])
                return static_cast<int>(i);
        }

        return -1;
    }

    /*
        Échange la priorité sélectionnée avec une autre ligne.
        Sert au déplacement simple quand on n'est pas au bord de la liste.
    */
    void swapSelectedPriorityWith(int otherIndex)
    {
        std::swap(priorityNames[static_cast<size_t>(selectedIndex)],
                  priorityNames[static_cast<size_t>(otherIndex)]);
    }

    /*
        Déplace la première priorité à la fin.
        Toutes les autres priorités remontent donc d'un rang.
    */
    void moveFirstPriorityToEnd()
    {
        const juce::String firstPriority = priorityNames.front();

        for (size_t i = 0; i + 1 < priorityNames.size(); ++i)
            priorityNames[i] = priorityNames[i + 1];

        priorityNames.back() = firstPriority;
    }

    /*
        Déplace la dernière priorité au début.
        Toutes les autres priorités descendent donc d'un rang.
    */
    void moveLastPriorityToStart()
    {
        const juce::String lastPriority = priorityNames.back();

        for (size_t i = priorityNames.size() - 1; i > 0; --i)
            priorityNames[i] = priorityNames[i - 1];

        priorityNames.front() = lastPriority;
    }

    /*
        Réapplique l'ordre courant aux lignes visibles.
        Le rang dépend toujours de la position, jamais du nom déplacé.
    */
    void refreshRows()
    {
        for (size_t i = 0; i < rows.size(); ++i)
        {
            rows[i].setContent(static_cast<int>(i + 1), priorityNames[i]);
            rows[i].setSelected(static_cast<int>(i) == selectedIndex);
        }
    }

    /*
        Informe le panneau parent que l'ordre affiché a changé.
        Le panneau met ensuite à jour le modèle et recalcule les coûts.
    */
    void notifyPriorityOrderChanged()
    {
        if (onPriorityOrderChanged != nullptr)
            onPriorityOrderChanged(getImportanceCosts());
    }

    std::array<juce::String, priorityCount> priorityNames
    {{
        fixedPriorityNames[0],
        fixedPriorityNames[1],
        fixedPriorityNames[2],
        fixedPriorityNames[3],
        fixedPriorityNames[4],
        fixedPriorityNames[5],
        fixedPriorityNames[6],
        fixedPriorityNames[7],
        fixedPriorityNames[8],
        fixedPriorityNames[9],
        fixedPriorityNames[10],
        fixedPriorityNames[11],
        fixedPriorityNames[12],
        fixedPriorityNames[13]
    }};

    std::array<PriorityRow, priorityCount> rows;
    int selectedIndex = -1;
    bool compactMode = false;
};
