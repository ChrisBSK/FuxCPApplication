cmake_minimum_required(VERSION 3.15)
project(Fuxophone VERSION 0.1.0)

if(APPLE)
    if(NOT CMAKE_OSX_ARCHITECTURES)
        set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Build architecture")
    endif()
endif()

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(externals/JUCE)
add_subdirectory(externals/FuxCP/FuxCP/c++)

juce_add_plugin(Fuxophone
        PRODUCT_NAME "Fuxophone"
        COMPANY_NAME "ChrisBakashika"
        IS_SYNTH TRUE                   # Dit à GarageBand de proposer le plug-in sur une piste "instrument logiciel"
        NEEDS_MIDI_INPUT TRUE           # Permet de jouer le Cantus firmus sur un clavier MIDI (amélioration future)
        NEEDS_MIDI_OUTPUT FALSE         # Pas besoin, la sortie est le fichier MIDI
        PLUGIN_MANUFACTURER_CODE Chba   # CODE à 4 lettres exigés par Apple
        PLUGIN_CODE Fuxp                # CODE à 4 lettres exigés par Apple
        FORMATS AU Standalone           # Version Standalone (perso) + Version AU (publique)
        COPY_PLUGIN_AFTER_BUILD FALSE    # installe .component généré dans ~/Library/Audio/Plug-Ins/Components/
        # à chaque build.
)

juce_add_binary_data(AppAssets
        SOURCES
        assets/logoApp.png
)

target_sources(Fuxophone PRIVATE

        #src/ui/main.cpp # Version Standalone

        # Plugin
        src/plugin/PluginProcessor.cpp
        src/plugin/PluginProcessor.h
        src/plugin/PluginEditor.h

        src/ui/MainComponent.cpp
        src/ui/MainComponent.h

        src/service/GenerationService.cpp
        src/service/GenerationService.h

        src/model/FuxTypes.h
        src/model/CantusProblem.cpp
        src/model/CantusProblem.h
        src/model/ConstraintsSettings.h
        src/model/ConstraintsSettings.cpp

        src/ui/header/HeaderPanel.cpp
        src/ui/header/HeaderPanel.h

        src/ui/pages/SolverExplanationPage.cpp
        src/ui/pages/SolverExplanationPage.h
        src/ui/pages/AboutPage.cpp
        src/ui/pages/AboutPage.h
        src/ui/pages/SavedConfigurationsPage.cpp
        src/ui/pages/SavedConfigurationsPage.h

        src/ui/keyboard/KeyboardComponent.cpp
        src/ui/keyboard/KeyboardComponent.h

        src/ui/leftPanel/LeftPanel.cpp
        src/ui/leftPanel/LeftPanel.h

        src/ui/optionsPanel/OptionsPanel.cpp
        src/ui/optionsPanel/OptionsPanel.h
        src/ui/optionsPanel/OptionsPanelHelpers.cpp
        src/ui/optionsPanel/OptionsPanelHelpers.h
        src/ui/optionsPanel/components/ClickableTitle.h
        src/ui/optionsPanel/components/ColumnBox.h
        src/ui/optionsPanel/components/SolverPriorityList.h
        src/ui/optionsPanel/components/ArrowIconButton.h
        src/ui/optionsPanel/components/VoiceWorkspace.h
        src/ui/optionsPanel/components/MinimizationModePanel.h



        src/audio/synth/SimpleSynth.cpp
        src/audio/synth/SimpleSynth.h

        src/controller/AppController.cpp
        src/controller/AppController.h
)

target_include_directories(Fuxophone PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/externals/FuxCP/FuxCP/c++/headers
)

target_link_libraries(Fuxophone PRIVATE
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
        juce::juce_recommended_lto_flags

        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_utils
        juce::juce_events
        juce::juce_core
        juce::juce_data_structures

        juce::juce_audio_processors

        AppAssets
        FuxCPLib
)
# Optionnel, pour éviter dépendances web/curl au début
target_compile_definitions(Fuxophone PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)

# =========================
# Embarque Gecode dans chaque bundle généré (AU + Standalone)
# =========================
if(CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
    set(GECODE_BINARY_TO_EMBED "${CMAKE_SOURCE_DIR}/externals/gecode-arm64/gecode")
else()
    set(GECODE_BINARY_TO_EMBED "/Library/Frameworks/gecode.framework/Versions/49/gecode")
endif()

foreach(format_target Fuxophone_AU Fuxophone_Standalone)
    add_custom_command(TARGET ${format_target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_BUNDLE_CONTENT_DIR:${format_target}>/Frameworks"
            COMMAND ${CMAKE_COMMAND} -E copy
            "${GECODE_BINARY_TO_EMBED}"
            "$<TARGET_BUNDLE_CONTENT_DIR:${format_target}>/Frameworks/libgecode.dylib"
            COMMAND install_name_tool -id "@rpath/libgecode.dylib"
            "$<TARGET_BUNDLE_CONTENT_DIR:${format_target}>/Frameworks/libgecode.dylib"
            COMMAND install_name_tool -change
            "gecode.framework/Versions/49/gecode"
            "@rpath/libgecode.dylib"
            "$<TARGET_FILE:${format_target}>"
            COMMAND install_name_tool -add_rpath
            "@loader_path/../Frameworks" "$<TARGET_FILE:${format_target}>"
            COMMAND codesign --force -s -
            "$<TARGET_BUNDLE_CONTENT_DIR:${format_target}>/Frameworks/libgecode.dylib"
            COMMAND codesign --force -s -
            "$<TARGET_BUNDLE_DIR:${format_target}>"
            VERBATIM
    )
endforeach()

#propre ajout de la copie
add_custom_command(TARGET Fuxophone_AU POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory
        "$ENV{HOME}/Library/Audio/Plug-Ins/Components/Fuxophone.component"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "$<TARGET_BUNDLE_DIR:Fuxophone_AU>"
        "$ENV{HOME}/Library/Audio/Plug-Ins/Components/Fuxophone.component"
        VERBATIM
)