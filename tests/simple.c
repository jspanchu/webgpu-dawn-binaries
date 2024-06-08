// SPDX-FileCopyrightText: Copyright (c) Jaswant Sai Panchumarti
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>

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
    if (symbol == NULL) {                                                      \
      fprintf(stderr, "Failed to load %s\n", #symbol);                         \
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
    if (symbol == NULL) {                                                      \
      fprintf(stderr, "Failed to load %s\n", #symbol);                         \
      return EXIT_FAILURE;                                                     \
    }                                                                          \
  } while (0)
#define LOAD_WGPU_SYMBOL(name) LOAD_SYMBOL(wgpu##name, WGPUProc##name)
#endif

typedef struct AdapterCallbackBridge {
  WGPURequestAdapterStatus status;
  WGPUAdapter adapter;
  int satisfied;
} AdapterCallbackBridge;

void onAdapterReceived(WGPURequestAdapterStatus status, WGPUAdapter adapter,
                       char const *message, void *userdata) {
  AdapterCallbackBridge *adapterCallbackBridge =
      (AdapterCallbackBridge *)userdata;
  adapterCallbackBridge->status = status;
  adapterCallbackBridge->adapter = adapter;
  adapterCallbackBridge->satisfied = 1;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: simple_c /path/to/<webgpu_implementation_lib>\n");
    return EXIT_FAILURE;
  }
  LIBRARY_HANDLE_TYPE webgpuImpl = NULL;
  webgpuImpl = LOAD_LIBRARY(argv[1]);
  if (webgpuImpl == NULL) {
    return EXIT_FAILURE;
  }
  WGPUProcCreateInstance wgpuCreateInstance = NULL;
  LOAD_WGPU_SYMBOL(CreateInstance);

  WGPUInstance instance = wgpuCreateInstance(NULL);
  if (instance == NULL) {
    fprintf(stderr, "wgpuCreateInstance failed to create an instance!\n");
    return EXIT_FAILURE;
  }

  WGPUProcInstanceRequestAdapter wgpuInstanceRequestAdapter = NULL;
  LOAD_WGPU_SYMBOL(InstanceRequestAdapter);

  WGPURequestAdapterOptions adapterOpts = {};
  adapterOpts.nextInChain = NULL;

  WGPUProcInstanceProcessEvents wgpuInstanceProcessEvents = NULL;
  LOAD_WGPU_SYMBOL(InstanceProcessEvents);

  AdapterCallbackBridge adapterCallbackBridge;
  adapterCallbackBridge.adapter = NULL;
  adapterCallbackBridge.status = WGPURequestAdapterStatus_Unknown;
  adapterCallbackBridge.satisfied = 0;
  wgpuInstanceRequestAdapter(instance, &adapterOpts, onAdapterReceived,
                             &adapterCallbackBridge);
  do {
    wgpuInstanceProcessEvents(instance);
  } while (!adapterCallbackBridge.satisfied);
  if (adapterCallbackBridge.status != WGPURequestAdapterStatus_Success ||
      adapterCallbackBridge.adapter == NULL) {
    fprintf(stderr, "wgpuInstanceRequestAdapter failed!\n");
    return EXIT_FAILURE;
  }
}
