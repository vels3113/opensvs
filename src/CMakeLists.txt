add_executable(opensvs
    main.cpp
    MainWindow.cpp
    MainWindow.h
)

target_include_directories(opensvs PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(opensvs PRIVATE Qt6::Widgets)
