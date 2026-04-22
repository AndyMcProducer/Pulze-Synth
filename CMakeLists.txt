cmake_minimum_required(VERSION 3.22)

project(Pulze VERSION 0.9.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
    message(STATUS "JUCE not found locally, fetching from GitHub...")
    include(FetchContent)
    FetchContent_Declare(
        juce
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 7.0.12 # Or a specific version you want
    )
    FetchContent_MakeAvailable(juce)
    set(JUCE_PATH ${juce_SOURCE_DIR})
else()
    add_subdirectory(JUCE)
    set(JUCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/JUCE)
endif()

juce_add_plugin(Pulze
    COMPANY_NAME "P2PDAW"
    BUNDLE_ID "com.p2pdaw.pulze"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE P2PD
    PLUGIN_CODE PULZ
    FORMATS VST3 AU
    PRODUCT_NAME "Pulze"
)

juce_generate_juce_header(Pulze)

target_sources(Pulze
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
)

target_compile_definitions(Pulze
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(Pulze
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

install(TARGETS Pulze
    DESTINATION .
    COMPONENT Pulze
)

# --- CPack Configuration ---
set(CPACK_PACKAGE_NAME "Pulze Synth")
set(CPACK_PACKAGE_FILE_NAME "Pulze-Setup")
set(CPACK_PACKAGE_VENDOR "P2PDAW")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Pulze Synth - 4 Oscillator Synthesizer")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})

if(WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_MODIFY_PATH ON)
elseif(APPLE)
    set(CPACK_GENERATOR "STGZ;TGZ;DragNDrop")
endif()

include(CPack)
