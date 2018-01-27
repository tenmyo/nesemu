set(target nes_emu)

add_executable(${target}
  main.cpp
)

target_compile_options(${target}
  PRIVATE
  ${DEFAULT_COMPILE_OPTIONS}
)

clang_format(${target})
clang_tidy(${target})
