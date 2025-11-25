add_executable(netgenjson_parser_tests
    parsers/NetgenJsonParserTests.cpp
    ${CMAKE_SOURCE_DIR}/src/parsers/NetgenJsonParser.cpp
)

target_include_directories(netgenjson_parser_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_compile_definitions(netgenjson_parser_tests PRIVATE FIXTURE_PATH=\"${CMAKE_SOURCE_DIR}/resources/fixtures/netgen_sample.json\")
target_link_libraries(netgenjson_parser_tests PRIVATE Qt6::Test Qt6::Core)

add_test(NAME netgenjson_parser_tests COMMAND netgenjson_parser_tests)
