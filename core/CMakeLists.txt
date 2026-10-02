# SPDX-License-Identifier: Apache-2.0
# Embed data/alto_resonators.json into the binary (D-009). Generated at configure time as a byte array
# (avoids string-literal length limits); re-configures automatically when the data file changes.
set(SAX_RES_JSON "${CMAKE_SOURCE_DIR}/data/alto_resonators.json")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${SAX_RES_JSON}")
file(READ "${SAX_RES_JSON}" SAX_RES_HEX HEX)
string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," SAX_RES_ARR "${SAX_RES_HEX}")
file(CONFIGURE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/embedded_resonators.cpp" @ONLY CONTENT
"// Generated from data/alto_resonators.json by core/CMakeLists.txt. Do not edit.
#include \"sax/ResonatorTable.h\"
namespace sax {
namespace {
const unsigned char kEmbeddedResonators[] = { @SAX_RES_ARR@ 0x00 };
}
std::string_view embeddedResonatorJson() {
    return std::string_view(reinterpret_cast<const char*>(kEmbeddedResonators), sizeof(kEmbeddedResonators) - 1);
}
}
")

add_library(sax_core STATIC
  src/Pitch.cpp src/Keys.cpp src/Fingering.cpp src/KeyLayout.cpp
  src/ResonatorTable.cpp src/ReedModel.cpp src/ColinotReference.cpp
  src/SaxVoice.cpp src/Analysis.cpp src/State.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/embedded_resonators.cpp)
target_include_directories(sax_core PUBLIC include)
target_compile_features(sax_core PUBLIC cxx_std_20)
target_link_libraries(sax_core PRIVATE nlohmann_json::nlohmann_json)
set_target_properties(sax_core PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(MSVC)
  target_compile_options(sax_core PRIVATE /W4 /permissive-)
else()
  target_compile_options(sax_core PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
endif()
