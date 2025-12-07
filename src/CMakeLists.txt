add_executable(opensvs
    main.cpp
    MainWindow.cpp
    MainWindow.hpp
    parsers/NetgenJsonParser.cpp
    parsers/NetgenJsonParser.hpp
    models/DiffEntryModel.cpp
    models/DiffEntryModel.hpp
    models/DiffFilterProxyModel.cpp
    models/DiffFilterProxyModel.hpp
    models/CircuitTreeModel.cpp
    models/CircuitTreeModel.hpp
)

target_include_directories(opensvs PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(opensvs PRIVATE Qt6::Widgets)
