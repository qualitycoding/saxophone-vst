# SPDX-License-Identifier: Apache-2.0
set(SAX_PLUGIN_FORMATS VST3 Standalone)
if(APPLE)
  list(APPEND SAX_PLUGIN_FORMATS AU)
endif()

juce_add_plugin(SaxophoneVST
  COMPANY_NAME "qualitycoding"
  PLUGIN_MANUFACTURER_CODE Qcod
  PLUGIN_CODE Saxa
  FORMATS ${SAX_PLUGIN_FORMATS}
  PRODUCT_NAME "Saxophone"
  IS_SYNTH TRUE
  NEEDS_MIDI_INPUT TRUE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE
  COPY_PLUGIN_AFTER_BUILD FALSE
  VST3_CATEGORIES Instrument Synth)

target_sources(SaxophoneVST PRIVATE
  src/PluginProcessor.cpp src/PluginEditor.cpp src/SaxophoneView.cpp)
target_include_directories(SaxophoneVST PUBLIC src)
target_compile_definitions(SaxophoneVST PUBLIC
  JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0 JUCE_DISPLAY_SPLASH_SCREEN=0)
target_link_libraries(SaxophoneVST
  PUBLIC sax_core juce::juce_audio_utils juce::juce_audio_processors juce::juce_gui_basics
  PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)

if(SAX_BUILD_TESTS)
  add_executable(sax_plugin_tests ${CMAKE_SOURCE_DIR}/tests/plugin/test_plugin.cpp)
  target_link_libraries(sax_plugin_tests PRIVATE SaxophoneVST Catch2::Catch2)
  add_test(NAME plugin COMMAND sax_plugin_tests)
endif()
