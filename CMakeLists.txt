cmake_minimum_required(VERSION 3.15)
project(myproject VERSION 0.1.0)

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Build architecture" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(externals/JUCE)
add_subdirectory(externals/FuxCP4/FuxCP/c++)


juce_add_gui_app(myproject
        PRODUCT_NAME "myproject"
)

target_sources(myproject PRIVATE
        src/ui/main.cpp
        src/ui/MainComponent.cpp
        src/ui/MainComponent.h

        src/service/GenerationService.cpp
        src/service/GenerationService.h

        src/model/FuxTypes.h
        src/model/CantusFirmus.h
        src/model/CounterpointSolution.h

)

target_include_directories(myproject PRIVATE
        externals/FuxCP4/FuxCP/c++/headers
        #externals/JUCE/modules
)


target_link_libraries(myproject PRIVATE
        juce::juce_gui_extra
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
        juce::juce_recommended_lto_flags

        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_utils
        juce::juce_events
        juce::juce_core

        damien
)

# Optionnel, pour éviter dépendances web/curl au début
target_compile_definitions(myproject PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)
