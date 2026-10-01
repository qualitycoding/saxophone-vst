# SPDX-License-Identifier: Apache-2.0
add_library(sax_core STATIC
  src/Pitch.cpp src/Keys.cpp src/Fingering.cpp src/KeyLayout.cpp
  src/ResonatorTable.cpp src/ReedModel.cpp src/ColinotReference.cpp
  src/SaxVoice.cpp src/Analysis.cpp src/State.cpp)
target_include_directories(sax_core PUBLIC include)
target_compile_features(sax_core PUBLIC cxx_std_20)
target_link_libraries(sax_core PRIVATE nlohmann_json::nlohmann_json)
set_target_properties(sax_core PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(MSVC)
  target_compile_options(sax_core PRIVATE /W4 /permissive-)
else()
  target_compile_options(sax_core PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
endif()
