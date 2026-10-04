numsim_dependency(nlohmann_json
    TARGET nlohmann_json::nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    FIND_PACKAGE_ARGS 3.11 CONFIG
    OPTIONS JSON_BuildTests=OFF JSON_Install=OFF)
numsim_add_test(json_parameter_converter_test SOURCES json_parameter_converter_test.cpp
    LINK numsim-core::json nlohmann_json::nlohmann_json)
