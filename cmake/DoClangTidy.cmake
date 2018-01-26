# Must be include before creating targets.
option(CLANG_TIDY_ENABLE
  "If the command clang-tidy is avilable, tidy source files.\
Turn this off if the build time is too slow."
  ON)
find_program(CLANG_TIDY_EXE clang-tidy)
if(CLANG_TIDY_EXE)
  if(CLANG_TIDY_ENABLE)
    message(STATUS "Enable Clang-Tidy")
    set(CMAKE_C_CLANG_TIDY "${CLANG_TIDY_EXE}"
      -fix
    )
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}"
      -fix
    )
  endif()
endif()

