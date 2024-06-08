// SPDX-FileCopyrightText: Copyright (c) Jaswant Sai Panchumarti
// SPDX-License-Identifier: Apache-2.0

#include <chrono>
#include <cstdlib>
#include <future>
#include <iostream>

#define WGPU_SKIP_DECLARATIONS
#include <webgpu.h>

#if defined(_WIN32)
#include <windows.h>
#define LOAD_LIBRARY(name) LoadLibrary(TEXT(name))
#define UNLOAD_LIBRARY(lib) FreeLibrary(lib)
#define LIBRARY_HANDLE_TYPE HINSTANCE
#define DAWN_LIBRARY "./webgpu_dawn.dll"
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
#if defined(__APPLE__)
#define DAWN_LIBRARY "./libwebgpu_dawn.dylib"
#else
#define DAWN_LIBRARY "./libwebgpu_dawn.so"
#endif
#endif

int main(int, char **) {
  LIBRARY_HANDLE_TYPE webgpuImpl = LOAD_LIBRARY(DAWN_LIBRARY);
  if (webgpuImpl == nullptr) {
    return EXIT_FAILURE;
  }
  WGPUProcCreateInstance wgpuCreateInstance = nullptr;
  LOAD_WGPU_SYMBOL(CreateInstance);

  WGPUInstance instance = wgpuCreateInstance(nullptr);
  if (instance == nullptr) {
    std::cerr << "wgpuCreateInstance failed to create an instance!\n";
    return EXIT_FAILURE;
  }

  WGPUProcInstanceRequestAdapter wgpuInstanceRequestAdapter = nullptr;
  LOAD_WGPU_SYMBOL(InstanceRequestAdapter);

  WGPURequestAdapterOptions adapterOpts = {};
  adapterOpts.nextInChain = nullptr;

  WGPUProcInstanceProcessEvents wgpuInstanceProcessEvents = nullptr;
  LOAD_WGPU_SYMBOL(InstanceProcessEvents);

  struct AdapterCallbackBridge {
    WGPURequestAdapterStatus status{WGPURequestAdapterStatus_Unknown};
    WGPUAdapter adapter = {nullptr};
    std::promise<void> satisfied;
  } adapterCallbackBridge;
  auto onAdapterReceived = [](WGPURequestAdapterStatus status,
                              WGPUAdapter adapter, char const *message,
                              void *userdata) {
    auto &adapterCallbackBridge =
        *(reinterpret_cast<AdapterCallbackBridge *>(userdata));
    adapterCallbackBridge.status = status;
    adapterCallbackBridge.adapter = adapter;
    adapterCallbackBridge.satisfied.set_value();
  };
  wgpuInstanceRequestAdapter(instance, &adapterOpts, onAdapterReceived,
                             &adapterCallbackBridge);
  std::future<void> f = adapterCallbackBridge.satisfied.get_future();
  do {
    wgpuInstanceProcessEvents(instance);
    auto fStatus = f.wait_for(std::chrono::milliseconds(10));
    if (fStatus == std::future_status::ready) {
      break;
    }
  } while (true);
  if (adapterCallbackBridge.status != WGPURequestAdapterStatus_Success ||
      adapterCallbackBridge.adapter == nullptr) {
    std::cerr << "wgpuInstanceRequestAdapter failed!\n";
    return EXIT_FAILURE;
  }
}
