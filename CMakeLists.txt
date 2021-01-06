set(LLVM_LINK_COMPONENTS support)

add_clang_executable(module-dumper
  ModuleDumper.cpp
  )
target_link_libraries(module-dumper
  PRIVATE
  clangTooling
  clangBasic
  clangASTMatchers
  )

