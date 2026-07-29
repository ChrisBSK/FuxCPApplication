#pragma once

#include <algorithm>
#include <array>
#include <functional>
#include <vector>

#include <juce_gui_basics/juce_gui_basics.h>

/*
    =====================================================================
    SolverPriorityList.h — liste des priorités du solveur

    Ce composant affiche et modifie l'ordre des priorités utilisé par FuxCP
    en mode Lexicographic.

    - chaque ligne affiche un rang visible : 1, 2, 3...
    - chaque ligne contient le nom d'une contrainte FuxCP,
    - l'utilisateur peut sélectionner une ligne,
    - puis la déplacer avec les flèches haut / bas.

    Le composant ne modifie pas directement le modèle.
    Il renvoie seulement le nouvel ordre via onPriorityOrderChanged.
    =====================================================================
*/
class SolverPriorityList : public juce::Component
{
public:

    /*
        Crée les lignes de priorité et connecte leur sélection.

        Chaque PriorityRow connaît seulement son clic.
        SolverPriorityList garde l'ordre global et décide quelle ligne est sélectionnée.
    */
    SolverPriorityList()
    {
        for (size_t i = 0; i < rows.size(); ++i)
        {
            // Chaque ligne sélectionne son propre index quand on clique dessus.
            rows[i].onClick = [this, i]()
            {

                selectRow(static_cast<int>(i));
            };
            // Rend toutes les lignes visibles dans le composant.
            addAndMakeVisible(rows[i]);
        }
        // Applique l'ordre initial aux lignes.
        refreshRows();
    }

    // Callback envoyé au parent quand l'ordre des priorités change.
    std::function<void(const std::vector<int>&)> onPriorityOrderChanged;

    /*
        Active ou désactive l'affichage compact.

        Le mode compact réduit les hauteurs, marges et tailles de texte.
        Il est utilisé quand la liste est placée dans une colonne étroite.
    */
    void setCompactMode(bool shouldUseCompactMode)
    {
        compactMode = shouldUseCompactMode;

        // Chaque ligne doit adopter le même mode d'affichage que la liste.
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
        // On ignore les vecteurs incomplets ou incompatibles.
        if (importanceCosts.size() != priorityCount)
            return;

        // indexes contient les index fixes des contraintes FuxCP : 0, 1, 2...
        std::array<size_t, priorityCount> indexes {};

        for (size_t i = 0; i < indexes.size(); ++i)
            indexes[i] = i;

        // Trie les index selon leur rang d'importance.
        std::sort(indexes.begin(), indexes.end(),
                  [&importanceCosts](size_t left, size_t right)
                  {
                      return importanceCosts[left] < importanceCosts[right];
                  });

        // Reconstruit l'ordre visuel à partir des index triés.
        for (size_t rank = 0; rank < priorityCount; ++rank)
            priorityNames[rank] = fixedPriorityNames[indexes[rank]];

        selectedIndex = -1;
        refreshRows();
    }

    /*
        Convertit l'ordre visible en vecteur importance pour FuxCP.

        Exemple :
        si "melodic" est affiché en première ligne,
        alors son index FuxCP reçoit la valeur 1.

        La position visible devient donc le rang envoyé au solveur.
    */
    std::vector<int> getImportanceCosts() const
    {
        // Vecteur final attendu par ConstraintSettings/FuxCP.
        std::vector<int> importanceCosts(priorityCount, 0);

        // Pour chaque ligne visible, on retrouve son index fixe côté FuxCP.
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

        Le rang visuel reste attaché à la ligne.
        C'est le nom de la contrainte qui change de position.

        Si la première ligne monte, elle passe en dernière position.
    */
    void moveSelectedUp()
    {
        // Aucune ligne sélectionnée : rien à déplacer.
        if (selectedIndex < 0)
            return;

        const int lastIndex = static_cast<int>(priorityNames.size()) - 1;

        // Cas circulaire : la première priorité repart en bas de la liste.
        if (selectedIndex == 0)
        {
            moveFirstPriorityToEnd();
            selectedIndex = lastIndex;
        }
        // Cas normal : on échange avec la ligne juste au-dessus.
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

        Le rang visuel reste attaché à la ligne.
        C'est le nom de la contrainte qui change de position.

        Si la dernière ligne descend, elle passe en première position.
    */
    void moveSelectedDown()
    {
        const int lastIndex = static_cast<int>(priorityNames.size()) - 1;

        // Aucune ligne sélectionnée : rien à déplacer.
        if (selectedIndex < 0)
            return;

        // Cas circulaire : la dernière priorité repart en haut de la liste.
        if (selectedIndex == lastIndex)
        {
            moveLastPriorityToStart();
            selectedIndex = 0;
        }
        // Cas normal : on échange avec la ligne juste en dessous.
        else
        {
            swapSelectedPriorityWith(selectedIndex + 1);
            ++selectedIndex;
        }

        refreshRows();
        notifyPriorityOrderChanged();
    }

    /*
        Place toutes les lignes dans la liste.

        La hauteur disponible est divisée entre les 14 priorités.
        En mode compact, les marges et espacements sont réduits.
    */
    void resized() override
    {
        // Marges et espacements adaptés au mode normal ou compact.
        const int inset = compactMode ? 3 : juce::jlimit(6, 12, getWidth() / 18);
        const int spacingY = compactMode ? 1 : juce::jlimit(2, 4, getHeight() / 130);

        auto area = getLocalBounds().reduced(inset);

        const int rowCount = static_cast<int>(rows.size());

        // Hauteur réellement disponible après retrait des espaces entre lignes.
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

    // Nombre de priorités connues par FuxCP.
    static constexpr size_t priorityCount = 14;

    /*
        Noms fixes des contraintes dans l'ordre attendu par FuxCP.

        Important :
        cet ordre ne doit pas changer.
        C'est l'ordre visuel dans priorityNames qui change quand l'utilisateur
        déplace les priorités.
    */
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
        Ligne visuelle d'une priorité.

        Elle affiche :
        - une bulle avec le rang visible,
        - un rectangle avec le nom de la contrainte,
        - un fond léger si la ligne est sélectionnée.
    */
    class PriorityRow : public juce::Component
    {
    public:
        // Callback appelé quand l'utilisateur clique sur cette ligne.
        std::function<void()> onClick;

        // Met à jour le rang et le nom affichés par la ligne.
        void setContent(int newRank, juce::String newName)
        {
            rank = newRank;
            name = std::move(newName);
            repaint();
        }

        // Active ou désactive l'état sélectionné de la ligne.
        void setSelected(bool shouldBeSelected)
        {
            isSelected = shouldBeSelected;
            repaint();
        }

        // Adapte le rendu de la ligne au mode compact.
        void setCompactMode(bool shouldUseCompactMode)
        {
            compactMode = shouldUseCompactMode;
            repaint();
        }

        /*
            Dessine la ligne complète.

            La ligne est découpée en deux zones :
            - la bulle de rang à gauche,
            - le nom de priorité à droite.
        */
        void paint(juce::Graphics& g) override
        {
            auto area = getLocalBounds();

            // Taille de la bulle et espace avec le nom.
            const int gap = compactMode ? 3 : juce::jlimit(4, 7, area.getWidth() / 28);
            const int bubbleSize = compactMode
                ? juce::jlimit(12, 16, area.getHeight())
                : juce::jlimit(18, 24, area.getHeight());

            // Coupe la zone gauche pour y placer la bulle.
            auto bubble = area.removeFromLeft(bubbleSize).withSizeKeepingCentre(
                bubbleSize,
                bubbleSize
            );

            area.removeFromLeft(gap);

            // Dessine les trois éléments visuels de la ligne.
            drawRankBubble(g, bubble);
            drawPriorityName(g, area);
            drawSelection(g, getLocalBounds());
        }

        // Un clic sur la ligne prévient SolverPriorityList.
        void mouseDown(const juce::MouseEvent&) override
        {
            if (onClick != nullptr)
                onClick();
        }

    private:
        /*
            Dessine le fond de sélection si cette ligne est active.
        */
        void drawSelection(juce::Graphics& g, juce::Rectangle<int> bounds)
        {
            if (! isSelected)
                return;

            g.setColour(juce::Colours::white.withAlpha(0.18f));
            g.fillRoundedRectangle(bounds.toFloat(), compactMode ? 4.0f : 7.0f);
        }

        /*
            Dessine la bulle contenant le rang visible : 1, 2, 3...
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
            Dessine le nom court de la contrainte FuxCP.
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
        Sélectionne une ligne.

        La sélection sert ensuite aux boutons haut / bas pour savoir
        quelle priorité déplacer.
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
        Déplace la première priorité à la fin de la liste.

        Cela permet un déplacement circulaire :
        monter la première ligne la fait revenir en bas.
    */
    void moveFirstPriorityToEnd()
    {
        const juce::String firstPriority = priorityNames.front();

        for (size_t i = 0; i + 1 < priorityNames.size(); ++i)
            priorityNames[i] = priorityNames[i + 1];

        priorityNames.back() = firstPriority;
    }

    /*
        Déplace la dernière priorité au début de la liste.

        Cela permet un déplacement circulaire :
        descendre la dernière ligne la fait revenir en haut.
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

    /*
        Ordre actuellement affiché dans l'interface.

        Contrairement à fixedPriorityNames, cet ordre change quand l'utilisateur
        déplace une priorité.
    */
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

    // Lignes visibles de la liste.
    std::array<PriorityRow, priorityCount> rows;
    // Index de la ligne sélectionnée, ou -1 si aucune ligne n'est sélectionnée.
    int selectedIndex = -1;
    // Mode d'affichage dense utilisé dans la colonne Search.
    bool compactMode = false;
};
