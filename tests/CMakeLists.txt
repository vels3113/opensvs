if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  add_compile_options(-fno-omit-frame-pointer)
  if (ENABLE_ASAN)
    add_link_options(-fsanitize=address)
    add_compile_options(-fsanitize=address)
  endif()
  if (ENABLE_UBSAN)
    add_link_options(-fsanitize=undefined)
    add_compile_options(-fsanitize=undefined)
  endif()
  if (ENABLE_GLIBCXX_DEBUG AND CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    add_compile_definitions(_GLIBCXX_ASSERTIONS)
    # For heavier checking you could use:
    # add_compile_definitions(_GLIBCXX_DEBUG _GLIBCXX_DEBUG_PEDANTIC)
  endif()
endif()

add_executable(netgenjson_parser_tests
    parsers/NetgenJsonParserTests.cpp
    ${CMAKE_SOURCE_DIR}/src/parsers/NetgenJsonParser.cpp
)

target_include_directories(netgenjson_parser_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_definitions(netgenjson_parser_tests PRIVATE
    FIXTURE_PATH=\"${CMAKE_SOURCE_DIR}/resources/fixtures/netgen_tutorial/tut1/comp.json\"
    TUT2_PATH=\"${CMAKE_SOURCE_DIR}/resources/fixtures/netgen_tutorial/tut2/badnets.json\"
    TUT3_PATH=\"${CMAKE_SOURCE_DIR}/resources/fixtures/netgen_tutorial/tut3/comp.json\"
    TUT6_PATH=\"${CMAKE_SOURCE_DIR}/resources/fixtures/netgen_tutorial/tut6/comp_6c.json\"
)
target_link_libraries(netgenjson_parser_tests PRIVATE Qt6::Test Qt6::Core)

add_test(NAME netgenjson_parser_tests COMMAND netgenjson_parser_tests)

add_executable(diffentry_model_tests
    models/DiffEntryModelTests.cpp
    ${CMAKE_SOURCE_DIR}/src/models/DiffEntryModel.cpp
    ${CMAKE_SOURCE_DIR}/src/parsers/NetgenJsonParser.cpp
)

target_include_directories(diffentry_model_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(diffentry_model_tests PRIVATE Qt6::Test Qt6::Core Qt6::Widgets)

add_test(NAME diffentry_model_tests
    COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen XDG_RUNTIME_DIR=/tmp ${CMAKE_BINARY_DIR}/tests/diffentry_model_tests
)


add_executable(difffilter_model_tests
    models/DiffFilterProxyModelTests.cpp
    ${CMAKE_SOURCE_DIR}/src/models/DiffFilterProxyModel.cpp
    ${CMAKE_SOURCE_DIR}/src/models/DiffEntryModel.cpp
    ${CMAKE_SOURCE_DIR}/src/parsers/NetgenJsonParser.cpp
)

target_include_directories(difffilter_model_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(difffilter_model_tests PRIVATE Qt6::Test Qt6::Core Qt6::Widgets)

add_test(NAME difffilter_model_tests
    COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen XDG_RUNTIME_DIR=/tmp ${CMAKE_BINARY_DIR}/tests/difffilter_model_tests
)

add_executable(circuit_tree_model_tests
    models/CircuitTreeModelTests.cpp
    ${CMAKE_SOURCE_DIR}/src/models/CircuitTreeModel.cpp
    ${CMAKE_SOURCE_DIR}/src/models/DiffEntryModel.cpp
    ${CMAKE_SOURCE_DIR}/src/models/DiffFilterProxyModel.cpp
    ${CMAKE_SOURCE_DIR}/src/parsers/NetgenJsonParser.cpp
)

target_include_directories(circuit_tree_model_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(circuit_tree_model_tests PRIVATE Qt6::Test Qt6::Core Qt6::Widgets)

add_test(NAME circuit_tree_model_tests
    COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen XDG_RUNTIME_DIR=/tmp ${CMAKE_BINARY_DIR}/tests/circuit_tree_model_tests
)

add_executable(mainwindow_smoke_tests
    ui/MainWindowSmokeTests.cpp
    ${CMAKE_SOURCE_DIR}/src/MainWindow.cpp
    ${CMAKE_SOURCE_DIR}/src/models/DiffEntryModel.cpp
    ${CMAKE_SOURCE_DIR}/src/models/DiffFilterProxyModel.cpp
    ${CMAKE_SOURCE_DIR}/src/models/CircuitTreeModel.cpp
    ${CMAKE_SOURCE_DIR}/src/parsers/NetgenJsonParser.cpp
)

target_include_directories(mainwindow_smoke_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/models
    ${CMAKE_SOURCE_DIR}/src/parsers
)
target_compile_definitions(mainwindow_smoke_tests PRIVATE FIXTURE_PATH=\"${CMAKE_SOURCE_DIR}/resources/fixtures/netgen_tutorial/tut1/comp.json\")
target_link_libraries(mainwindow_smoke_tests PRIVATE Qt6::Test Qt6::Core Qt6::Widgets)

add_test(NAME mainwindow_smoke_tests
    COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen XDG_RUNTIME_DIR=/tmp ${CMAKE_BINARY_DIR}/tests/mainwindow_smoke_tests
)
