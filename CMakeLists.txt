cmake_minimum_required(VERSION 3.7)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompileOptions)
include(EnableMeasureTime)
include(EnableCcache)
include(DoClangTidy)
include(FormatFilesWithClangFormat)

set(MY_PROJECT_NAME   "nes_emu")
set(MY_VERSION_MAJOR  "0")
set(MY_VERSION_MINOR  "1")
set(MY_VERSION_PATCH  "0")
set(MY_VERSION        "${MY_VERSION_MAJOR}.${MY_VERSION_MINOR}.${MY_VERSION_PATCH}")

project(${MY_PROJECT_NAME}
  VERSION ${MY_VERSION}
  LANGUAGES C CXX
  )

add_subdirectory(source)

include(CTest)
if(BUILD_TESTING)
  message(STATUS "Build testing")
  enable_testing()
  include(gtest)
  add_subdirectory(test)
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

