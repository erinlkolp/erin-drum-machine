cmake_minimum_required(VERSION 3.22)
project(Erin808 VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(JUCE)

juce_add_binary_data(Erin808Samples
    HEADER_NAME "BinaryData.h"
    NAMESPACE BinaryData
    SOURCES
        samples/kick.wav
        samples/snare.wav
        samples/clap.wav
        samples/closed_hh.wav
        samples/open_hh.wav
        samples/rimshot.wav
        samples/cowbell.wav
        samples/tom.wav
)

juce_add_plugin(Erin808
    COMPANY_NAME "Erin Audio"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE Erin
    PLUGIN_CODE Er08
    FORMATS VST3 Standalone
    PRODUCT_NAME "Erin 808"
)

target_sources(Erin808
    PRIVATE
        source/PluginProcessor.cpp
        source/PluginEditor.cpp
        source/PadComponent.cpp
        source/KnobLookAndFeel.cpp
)

target_include_directories(Erin808 PRIVATE source)

target_link_libraries(Erin808
    PRIVATE
        Erin808Samples
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

target_compile_definitions(Erin808
    PUBLIC
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

# Fix trailing commas in JUCE-generated moduleinfo.json (breaks Bitwig scanner)
# then install the VST3 bundle to ~/.vst3/
add_custom_command(TARGET Erin808_VST3 POST_BUILD
    COMMAND python3 "${CMAKE_SOURCE_DIR}/scripts/fix_moduleinfo.py"
        "${CMAKE_CURRENT_BINARY_DIR}/Erin808_artefacts/$<CONFIG>/VST3/Erin 808.vst3/Contents/Resources/moduleinfo.json"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_BINARY_DIR}/Erin808_artefacts/$<CONFIG>/VST3/Erin 808.vst3"
        "$ENV{HOME}/.vst3/Erin 808.vst3"
    COMMENT "Fixing moduleinfo.json and installing VST3 to ~/.vst3/"
)
