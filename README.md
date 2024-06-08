# DynDawn - Dynamic Dawn Library

This repository builds and distributes a loadable shared library comprising the Dawn implementation of WebGPU.
Here's how you can use it without worrying about building dawn with all it's intricacies and linking it into
your app/library.

See [tests/simple.c](tests/simple.c) and [tests/simple.cpp](tests/simple.cpp) for more details.

# linux
```cpp
#include <webgpu/webgpu.h>
#include <dlfcn.h>
void* lib = dlopen(libdawn.so, RTLD_LAZY);
WGPUProcCreateInstance wgpuCreateInstance = (WGPUProcCreateInstance)dlsym(lib, "wgpuCreateInstance");
// Load more functions ..
```

# macOS
```cpp
#include <dlfcn.h>
void* lib = dlopen(libdawn.dyld, RTLD_LAZY);
WGPUProcCreateInstance wgpuCreateInstance = (WGPUProcCreateInstance)dlsym(lib, "wgpuCreateInstance");
// Load more functions ..
```

# windows
```cpp
#include <windows.h>
HINSTANCE lib = LoadLibraryA(TEXT(dawn.dll));
WGPUProcCreateInstance wgpuCreateInstance = (WGPUProcCreateInstance)GetProcAddress(lib, "wgpuCreateInstance");
// Load more functions ..
```
