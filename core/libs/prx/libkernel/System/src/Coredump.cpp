#include <cstdint>
#include <cstddef>
#include <atomic>
#include <cstdio>
#include "SceTypes.hpp"
#include "prx/libc/include/General.hpp"

// The handler is recorded but never invoked: host crashes are not turned into guest core dumps.
static std::atomic<uint64_t> g_coredumpHandler{0};
static std::atomic<uint64_t> g_coredumpContext{0};
static constexpr int CoredumpErrorNotInCoredumpHandler = static_cast<int>(0x81180003u);

extern "C" {

int APS5_VABI sceCoredumpRegisterCoredumpHandler(uint64_t handler, size_t stack_size, uint64_t context) {
    (void)stack_size;
    g_coredumpHandler.store(handler, std::memory_order_relaxed);
    g_coredumpContext.store(context, std::memory_order_relaxed);
    return 0;
}

int APS5_VABI sceCoredumpUnregisterCoredumpHandler(void) {
    g_coredumpHandler.store(0, std::memory_order_relaxed);
    g_coredumpContext.store(0, std::memory_order_relaxed);
    return 0;
}

int APS5_VABI sceKernelDebugWriteCppExceptionInfo(const void* exception, uint64_t unknown, const char* typeName, const char* what) {
    (void)unknown;
    std::fprintf(stderr, "[coredump] uncaught C++ exception %p of type %s%s%s\n", exception, typeName ? typeName : "(unknown)", what ? ", what(): " : "", what ? what : "");
    return 0;
}


int APS5_VABI sceCoredumpAttachUserFile(uint32_t userValue, const char* path) {
    (void)userValue;
    (void)path;
    return CoredumpErrorNotInCoredumpHandler;
}

int APS5_VABI sceCoredumpGetStopInfoGpu_Agc(void) {
    NotImplemented_nid_no_patch(__func__);
    return 0;
}

int APS5_VABI sceCoredumpAttachMemoryRegionAsUserFile(uint32_t userValue, const void* mem, size_t size, const char* name) {
    (void)userValue;
    (void)mem;
    (void)size;
    (void)name;
    return CoredumpErrorNotInCoredumpHandler;
}

int APS5_VABI sceCoredumpSetUserDataType(void) {
    NotImplemented_nid_no_patch("Uxqkdta7wEg");
    return 0;
}

int APS5_VABI sceCoredumpDebugTextOut(void) {
    NotImplemented_nid_no_patch("dei8oUx6DbU");
    return 0;
}

int APS5_VABI sceCoredumpGetStopInfoCpu(void* info, size_t size) {
    (void)info;
    (void)size;
    return CoredumpErrorNotInCoredumpHandler;
}

int APS5_VABI sceCoredumpWriteUserString() {
    NotImplemented_nid_no_patch(__func__);
    return 0;
}

int APS5_VABI sceCoredumpAttachUserMemoryFile(uint32_t userValue, const void* mem, size_t size) {
    (void)userValue;
    (void)mem;
    (void)size;
    return CoredumpErrorNotInCoredumpHandler;
}

int APS5_VABI sceCoredumpAttachMemoryRegion(uint32_t userValue, const void* mem, size_t size) {
    (void)userValue;
    (void)mem;
    (void)size;
    return CoredumpErrorNotInCoredumpHandler;
}

int64_t APS5_VABI sceCoredumpWriteUserData(const void* data, size_t size) {
    (void)data;
    (void)size;
    return CoredumpErrorNotInCoredumpHandler;
}
}
