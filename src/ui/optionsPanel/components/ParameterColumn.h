#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "BoxParameter.h"

/*
//==============================================================================
   ParameterColumn

   Contient et place verticalement plusieurs paramètres.
//==============================================================================
*/
class ParameterColumn
{
public:
    BoxParameter* addParameter(juce::Component& parent,
                               const juce::String& label,
                               std::unique_ptr<juce::Component> control,
                               std::optional<BoxParameter::EndpointLabels> endpointLabels = std::nullopt)
    {
        auto row = std::make_unique<BoxParameter>(label,
                                                  std::move(control),
                                                  std::move(endpointLabels));
        auto* rowPtr = row.get();

        parent.addAndMakeVisible(*row);
        rows.push_back(std::move(row));

        return rowPtr;
    }

    void layout(juce::Rectangle<int> bounds, bool useInset = true)
    {
        const int inset = useInset ? juce::jlimit(6, 12, bounds.getWidth() / 18) : 0;
        auto inner = bounds.reduced(inset);

        // Lignes compactes :
        // les paramètres restent lisibles sans former de gros blocs visuels.
        const int rowCount = static_cast<int>(rows.size());
        const int spacingY = juce::jlimit(4, 7, bounds.getHeight() / 90);
        const int availableHeight = inner.getHeight()
                                  - juce::jmax(0, rowCount - 1) * spacingY;
        const int rowHeight = rowCount > 0
            ? juce::jlimit(34, 44, availableHeight / rowCount)
            : 34;

        for (auto& row : rows)
        {
            row->setBounds(inner.removeFromTop(juce::jmax(rowHeight, row->getPreferredHeight())));
            inner.removeFromTop(spacingY);
        }
    }

    // Applique la couleur de sélection à tous les paramètres de la colonne.
        void setLinkedToSelectedVoice(bool linked)
        {
            for (auto& row : rows)
                row->setLinkedToSelectedVoice(linked);
        }

private:
    std::vector<std::unique_ptr<BoxParameter>> rows;
};
