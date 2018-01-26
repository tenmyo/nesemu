option(FORMAT_FILES_WITH_CLANG_FORMAT_BEFORE_EACH_BUILD
  "If the command clang-format is avilable, format source files before each build.\
Turn this off if the build time is too slow."
  ON)
find_program(CLANG_FORMAT clang-format)
if(CLANG_FORMAT)
  message(STATUS "Enable Clang-Format")
  get_target_property(MY_SOURCES ${PROJECT_NAME} SOURCES)
  add_custom_target(
    format-with-clang-format
    COMMAND clang-format -i -style=file ${MY_SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
  if(FORMAT_FILES_WITH_CLANG_FORMAT_BEFORE_EACH_BUILD)
    add_dependencies(${PROJECT_NAME} format-with-clang-format)
  endif()
endif()

