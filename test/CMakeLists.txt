set(target nes_emu_test)

include_directories("${gtest_SOURCE_DIR}/include")
add_executable(${target}
  test.cpp
)

# disable warnings
set(disable_warnings)
if(CMAKE_C_COMPILER_ID MATCHES "Clang")
  set(disable_warnings
    -Wno-global-constructors
    -Wno-zero-as-null-pointer-constant
    )
elseif(CMAKE_C_COMPILER_ID MATCHES "GNU")
elseif(CMAKE_C_COMPILER_ID MATCHES "MSVC")
endif()

target_compile_options(${target}
  PRIVATE
  ${DEFAULT_COMPILE_OPTIONS}
  ${disable_warnings}
)
target_link_libraries(${target} gtest_main)
add_test(NAME ${target} COMMAND "$<TARGET_FILE:${target}>")

get_target_property(srcs ${target} SOURCES)
foreach(src ${srcs})
  set_property(TEST ${target} APPEND PROPERTY LABELS ${src})
endforeach()

clang_format(${target})
