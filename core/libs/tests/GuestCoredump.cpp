#include "prx/libc/include/general/VabiMacros.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

extern "C" {
int APS5_VABI sceCoredumpRegisterCoredumpHandler(std::uint64_t handler, std::size_t stackSize, std::uint64_t context);
int APS5_VABI sceCoredumpUnregisterCoredumpHandler(void);
int APS5_VABI sceCoredumpAttachUserFile(std::uint32_t userValue, const char* path);
int APS5_VABI sceCoredumpAttachMemoryRegionAsUserFile(std::uint32_t userValue, const void* mem, std::size_t size, const char* name);
int APS5_VABI sceCoredumpAttachMemoryRegion(std::uint32_t userValue, const void* mem, std::size_t size);
int APS5_VABI sceCoredumpAttachUserMemoryFile(std::uint32_t userValue, const void* mem, std::size_t size);
int APS5_VABI sceCoredumpGetStopInfoCpu(void* info, std::size_t size);
std::int64_t APS5_VABI sceCoredumpWriteUserData(const void* data, std::size_t size);
}

static constexpr int NotInCoredumpHandler = static_cast<int>(0x81180003u);

static void Require(bool value, int line) {
    if (!value) {
        std::fprintf(stderr, "Coredump check failed at line %d\n", line);
        std::abort();
    }
}
#define Check(value) Require((value), __LINE__)

static int APS5_VABI Handler(void*) {
    return 0;
}

static void CheckOutsideHandler() {
    char region[64];
    std::memset(region, 0x5a, sizeof(region));
    Check(sceCoredumpAttachUserFile(1, "/app0/log.txt") == NotInCoredumpHandler);
    Check(sceCoredumpAttachMemoryRegionAsUserFile(2, region, sizeof(region), "region.bin") == NotInCoredumpHandler);
    Check(sceCoredumpAttachMemoryRegion(3, region, sizeof(region)) == NotInCoredumpHandler);
    Check(sceCoredumpAttachUserMemoryFile(4, region, sizeof(region)) == NotInCoredumpHandler);
    Check(sceCoredumpWriteUserData(region, sizeof(region)) == static_cast<std::int64_t>(NotInCoredumpHandler));
    unsigned char stopInfo[16];
    std::memset(stopInfo, 0xa5, sizeof(stopInfo));
    Check(sceCoredumpGetStopInfoCpu(stopInfo, sizeof(stopInfo)) == NotInCoredumpHandler);
    for (const unsigned char byte : stopInfo) Check(byte == 0xa5);
    for (const char byte : region) Check(byte == 0x5a);
}

int main() {
    CheckOutsideHandler();
    Check(sceCoredumpRegisterCoredumpHandler(reinterpret_cast<std::uint64_t>(&Handler), 0x4000, 0) == 0);
    CheckOutsideHandler();
    Check(sceCoredumpUnregisterCoredumpHandler() == 0);
    CheckOutsideHandler();
    return 0;
}
