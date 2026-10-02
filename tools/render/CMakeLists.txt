# SPDX-License-Identifier: Apache-2.0
add_executable(sax_calibrate calibrate_tuning.cpp)
target_link_libraries(sax_calibrate PRIVATE sax_core nlohmann_json::nlohmann_json)
