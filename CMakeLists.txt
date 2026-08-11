cmake_minimum_required(VERSION 3.15)
project(myproject VERSION 0.1.0)

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Build architecture" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(externals/JUCE)
add_subdirectory(externals/FuxCP/FuxCP/c++)

juce_add_gui_app(myproject
        PRODUCT_NAME "myproject"
)

juce_add_binary_data(AppAssets
        SOURCES
        assets/logoApp.png
)

target_sources(myproject PRIVATE
        src/ui/main.cpp
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

target_include_directories(myproject PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/externals/FuxCP/FuxCP/c++/headers
)

target_link_libraries(myproject PRIVATE
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

        AppAssets
        FuxCPLib
)
# Optionnel, pour éviter dépendances web/curl au début
target_compile_definitions(myproject PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)
