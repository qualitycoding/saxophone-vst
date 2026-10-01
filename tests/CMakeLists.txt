# FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
# SPDX-License-Identifier: Apache-2.0
set(SAX_FIXTURE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/fixtures")

function(sax_test name)
  add_executable(${name} ${ARGN})
  target_link_libraries(${name} PRIVATE sax_core Catch2::Catch2WithMain)
  target_include_directories(${name} PRIVATE core)
  target_compile_definitions(${name} PRIVATE SAX_FIXTURE_DIR="${SAX_FIXTURE_DIR}")
endfunction()

sax_test(sax_unit_tests
  core/test_pitch.cpp core/test_keys.cpp core/test_fingering.cpp
  core/test_resonator_table.cpp core/test_reed_model.cpp core/test_state.cpp
  core/test_key_layout.cpp core/test_analysis.cpp)
sax_test(sax_integration_tests
  core/test_colinot_reference.cpp core/test_voice_pitch.cpp core/test_voice_registers.cpp
  core/test_midi_behaviour.cpp)
sax_test(sax_operational_tests core/test_voice_robustness.cpp)
sax_test(sax_alloc_tests core/test_alloc.cpp)
sax_test(sax_perf_tests core/test_perf.cpp)

add_test(NAME unit        COMMAND sax_unit_tests)
add_test(NAME integration COMMAND sax_integration_tests)
add_test(NAME operational COMMAND sax_operational_tests)
add_test(NAME alloc       COMMAND sax_alloc_tests)
add_test(NAME perf        COMMAND sax_perf_tests)
set_tests_properties(perf PROPERTIES LABELS "perf")
