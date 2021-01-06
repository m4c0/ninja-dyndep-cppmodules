# ninja-dyndep-cppmodules

A Ninja Dyndep generator for C++20 modules

This is a Clang tool to generate a Ninja [Dyndep][ninja-dyndep] file with C++20 module dependency info.

The code is hacky, but it works like so:

1. Using Clang's tooling system, it process a file named `compile_commands.json`, containing the list of commands to compile a project
2. For each file in the command line, the tool runs the Clang infrastructure to find all module import statements
3. It outputs to the standard output the dyndep file

It can be used with a `build.ninja` like this:

```
rule cdb
  command=ninja -t compdb > $out

rule md
  command=module-dumper $in > $out

build $of/compile_commands.json: cdb | $if/main.cpp
build $of/build.dd: md $if/main.cpp | $of/compile_commands.json

build $of/main.o: cxx $if/main.cpp || $of/build.dd
  dyndep=$of/build.dd
build $of/module.pcm: pcm $if/module.cpp || $of/build.dd
  dyndep=$of/build.dd
build $of/exe: link $of/main.o $of/module.pcm
```

That instructs Ninja to dump a `compile_commands.json` file out of the build definition itself. Then, dyndep rule will be used to generate the dependency file - which will instruct Ninja about the order it should build the C++ files.

It needs more work to be used in an existing project, since a Ninja build generator needs to be adapted to add the dyndep rules, as well as the boilerplate to call the module dumper.

There are some limitations as well:

* It does not work with module implementation files `module X;`, since Clang process that in a different way than `export module X;`
* It assumes the exported module name is the same as the output file
* All PCM must be compiled in the same output folder
* It was not tested and probably doesn't work with module partitions (`module X:part;`)

This can be build by:

1. Clone LLVM
2. Clone this repo inside `clang-tool-extras`
3. Add `add_subdirectory(module-dumper)` to `clang-tool-extras/CMakeLists.txt`
4. Follow [LLVM's instructions][build-llvm] to build `module-dumper`

[build-llvm]: https://clang.llvm.org/docs/LibASTMatchersTutorial.html
[ninja-dyndep]: https://ninja-build.org/manual.html#ref_dyndep

