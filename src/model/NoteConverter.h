#pragma once

#include <juce_core/juce_core.h>
#include <vector>
#include <map>

/*
==============================================================================
    NoteConverter

    Rôle :
    - Convertir une note MIDI (0 → 127)
      en nom musical (C, C#, D, etc.)

    Pour la base :
    60 → C (Do)
    61 → C# (Do dièse)

    Et ainsi de suite

==============================================================================
*/

class NoteConverter
{
public:

    // =============================
    // Conversion d'une note MIDI
    // =============================
    static juce::String midiToNoteName(int midiNote)
    {
        if (midiNote < 0 || midiNote > 127)
            return "Invalid";

        static const juce::String noteNames[12] =
        {
            "C", "C#", "D", "D#", "E", "F",
            "F#", "G", "G#", "A", "A#", "B"
        };

        int noteIndex = midiNote % 12;
        int octave = (midiNote / 12) - 2; // convention MIDI standard (en JUCE) (24 - 108)

        return noteNames[noteIndex] + juce::String(octave);
    }

    // =============================
    // Conversion ligne input
    // =============================
    static juce::String midiLineToString(const std::vector<int>& notes)
    {
        juce::String result;

        for (size_t i = 0; i < notes.size(); ++i)
        {
            result += midiToNoteName(notes[i]);

            if (i + 1 < notes.size())
                result += " ";
        }

        return result;
    }

    static int noteNameToMidi(const juce::String& note)
    {
        juce::String n = note.toUpperCase().trim();

        static const std::map<juce::String, int> baseNotes =
        {
            {"C",0},{"C#",1},{"D",2},{"D#",3},{"E",4},{"F",5},
            {"F#",6},{"G",7},{"G#",8},{"A",9},{"A#",10},{"B",11}
        };

        //  octave pour s'y retrouver
        int octave = 4; // défaut (si on entre des lètre sans préciser l'octave)

        juce::String pitch = n;

        if (n.length() >= 2 && juce::CharacterFunctions::isDigit(n[n.length()-1]))
        {
            octave = n.substring(n.length()-1).getIntValue();
            pitch = n.substring(0, n.length()-1);
        }

        if (baseNotes.find(pitch) == baseNotes.end())
            return -1;

        int midi = baseNotes.at(pitch) + (octave + 1) * 12;

        if (midi < 0 || midi > 127)
            return -1;

        return midi;
    }


};