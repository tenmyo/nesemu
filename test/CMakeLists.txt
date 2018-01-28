
find_package(GTest REQUIRED)

set(target nes_emu_test)

# Disable warnings
set(disable_warnings)
if(CMAKE_C_COMPILER_ID MATCHES "Clang")
  set(disable_warnings
    -Wno-global-constructors
    -Wno-zero-as-null-pointer-constant
    )
elseif(CMAKE_C_COMPILER_ID MATCHES "GNU")
elseif(CMAKE_C_COMPILER_ID MATCHES "MSVC")
endif()

# Build
file(GLOB_RECURSE SRCS "*.cpp")
add_executable(${target} ${SRCS})
target_compile_options(${target}
  PRIVATE
  ${DEFAULT_COMPILE_OPTIONS}
  ${disable_warnings}
)
target_link_libraries(${target} GTest::Main ${PROJECT_NAME})
GTEST_ADD_TESTS(${target} "" AUTO)

clang_format(${target})
