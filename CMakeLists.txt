cmake_minimum_required(VERSION 3.24)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" PETRICHOR_VERSION)
string(STRIP "${PETRICHOR_VERSION}" PETRICHOR_VERSION)

project(Petrichor VERSION ${PETRICHOR_VERSION} LANGUAGES C CXX)

if(APPLE AND NOT DEFINED CMAKE_OSX_ARCHITECTURES)
  set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
endif()

include(FetchContent)
set(JUCE_VERSION "8.0.15" CACHE STRING "JUCE git tag to pin")
FetchContent_Declare(juce
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG        ${JUCE_VERSION}
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(juce)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(PETRICHOR_SOURCES
  Source/PluginProcessor.cpp
  Source/PluginEditor.cpp
  Source/Parameters/ParameterLayout.cpp
  Source/Engine/EngineRack.cpp
  Source/Engine/DewEngine.cpp
  Source/Engine/BloomEngine.cpp
  Source/Engine/BreezeEngine.cpp
  Source/Engine/RootsEngine.cpp
  Source/Engine/BurstEngine.cpp
  Source/Engine/Greenhouse.cpp
  Source/Presets/PresetManager.cpp
  Source/Presets/FactoryPresets.cpp
  Source/UI/LookAndFeel.cpp
  Source/UI/GlassBackground.cpp
  Source/UI/KnobComponent.cpp
  Source/UI/EngineCard.cpp
  Source/UI/Visualizer.cpp
  Source/UI/StepStrip.cpp
  Source/UI/ChordLane.cpp
  Source/UI/PresetBar.cpp
)

juce_add_plugin(Petrichor
  PRODUCT_NAME               "Petrichor"
  COMPANY_NAME               "Reflexed"
  PLUGIN_MANUFACTURER_CODE   Rflx
  PLUGIN_CODE                Ptch
  VERSION                    ${PETRICHOR_VERSION}
  FORMATS                    VST3 Standalone
  IS_SYNTH                   TRUE
  NEEDS_MIDI_INPUT           TRUE
  NEEDS_MIDI_OUTPUT          FALSE
  VST3_CATEGORIES            "Instrument|Synth"
  SOURCES                    ${PETRICHOR_SOURCES}
)

target_include_directories(Petrichor PRIVATE Source)
target_compile_definitions(Petrichor PRIVATE JUCE_VST3_CAN_REPLACE_VST2=0)

if(MSVC)
  target_compile_options(Petrichor PRIVATE /W3 /permissive- /utf-8)
else()
  target_compile_options(Petrichor PRIVATE -Wall -Wno-unused-parameter)
endif()

set(PETRICHOR_FONT_FILES "")
foreach(_font Inter-Regular.ttf Inter-Medium.ttf)
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Resources/${_font}")
    list(APPEND PETRICHOR_FONT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/Resources/${_font}")
  endif()
endforeach()

if(PETRICHOR_FONT_FILES)
  juce_add_binary_data(PetrichorBinaryData SOURCES ${PETRICHOR_FONT_FILES})
  target_link_libraries(Petrichor PRIVATE PetrichorBinaryData)
  target_compile_definitions(Petrichor PRIVATE PETRICHOR_HAS_EMBEDDED_FONTS=1)
endif()
