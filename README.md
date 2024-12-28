# Lodeb

A frontend for LLDB written in C++ using [Dear ImGui](https://github.com/ocornut/imgui).

## Build

So far I've only built this on Mac OSX. Firstly, install LLVM via brew

```
brew install llvm
```

Next, create a `build` directory in the root folder of this repo and run CMake

```
mkdir build && cd build
cmake -G "Unix Makefiles" ..
```

I specified the `Unix Makefiles` generator but you can use whatever generator you want.
Now, you should be able to go to the root folder and run the CMake build.

```
cd ..
cmake --build build --parallel
```

This should produce a binary at `build/lodeb`.

## Attribution

I made use of this little starter project called [Scaffold](https://github.com/Varnani/Scaffold)
to bootstrap the ImGui/GLFW3 app.
