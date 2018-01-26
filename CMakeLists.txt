cmake_minimum_required(VERSION 3.7)
include(cmake/CompileOptions.cmake)
include(cmake/EnableMeasureTime.cmake)
include(cmake/EnableCcache.cmake)
include(cmake/DoClangTidy.cmake)

project(nes_emu
  VERSION 0.1.0
  LANGUAGES C CXX
  )
add_executable(${PROJECT_NAME}
  source/main.cpp
  )

target_compile_options(${PROJECT_NAME}
  PRIVATE
  ${DEFAULT_COMPILE_OPTIONS}
  )

include(cmake/FormatFilesWithClangFormat.cmake)

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

