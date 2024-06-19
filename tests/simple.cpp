// SPDX-FileCopyrightText: Copyright (c) Jaswant Sai Panchumarti
// SPDX-License-Identifier: Apache-2.0

#include <cstdlib>
#include <iostream>

#define WGPU_SKIP_DECLARATIONS
#include <webgpu.h>

#if defined(_WIN32)
#include <windows.h>
#define LOAD_LIBRARY(name) LoadLibrary(TEXT(name))
#define UNLOAD_LIBRARY(lib) FreeLibrary(lib)
#define LIBRARY_HANDLE_TYPE HINSTANCE
#define LOAD_SYMBOL(symbol, pSymbol)                                           \
  do {                                                                         \
    symbol = (pSymbol)GetProcAddress(webgpuImpl, #symbol);                     \
    if (symbol == nullptr) {                                                   \
      std::cerr << "Failed to load " #symbol << "\n";                          \
      return EXIT_FAILURE;                                                     \
    }                                                                          \
  } while (0)
#define LOAD_WGPU_SYMBOL(name) LOAD_SYMBOL(wgpu##name, WGPUProc##name)
#else
#include <dlfcn.h>
#define LOAD_LIBRARY(name) dlopen(name, RTLD_LAZY)
#define UNLOAD_LIBRARY(lib) dlclose(lib)
#define LIBRARY_HANDLE_TYPE void *
#define LOAD_SYMBOL(symbol, pSymbol)                                           \
  do {                                                                         \
    symbol = (pSymbol)dlsym(webgpuImpl, #symbol);                              \
    if (symbol == nullptr) {                                                   \
      std::cerr << "Failed to load " #symbol << "\n";                          \
      return EXIT_FAILURE;                                                     \
    }                                                                          \
  } while (0)
#define LOAD_WGPU_SYMBOL(name) LOAD_SYMBOL(wgpu##name, WGPUProc##name)
#endif

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Usage: simple_cpp /path/to/<webgpu_implementation_lib>\n";
    return EXIT_FAILURE;
  }
  LIBRARY_HANDLE_TYPE webgpuImpl = LOAD_LIBRARY(argv[1]);
  std::cerr << webgpuImpl << '\n';
  if (webgpuImpl == nullptr) {
    return EXIT_FAILURE;
  }
  WGPUProcCreateInstance wgpuCreateInstance = nullptr;
  LOAD_WGPU_SYMBOL(CreateInstance);
  std::cerr << wgpuCreateInstance << '\n';

  WGPUInstance instance = wgpuCreateInstance(nullptr);
  if (instance == nullptr) {
    std::cerr << "wgpuCreateInstance failed to create an instance!\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
