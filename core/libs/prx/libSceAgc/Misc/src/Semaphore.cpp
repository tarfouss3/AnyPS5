#include "prx/libSceAgc/Misc/include/Semaphore.hpp"
#include "prx/libSceAgc/Command/include/Packet.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <mutex>
#include "SceTypes.hpp"
#include "prx/libc/include/General.hpp"

namespace {

constexpr int GRAPHICS5_ERROR_INVALID_ALIGNMENT = static_cast<int>(0x8a6c0002u);
constexpr int GRAPHICS5_ERROR_INVALID_VALUE = static_cast<int>(0x8a6c000bu);
constexpr int GRAPHICS5_ERROR_ALREADY_INITIALIZED = static_cast<int>(0x8a6c0048u);
constexpr int GRAPHICS5_ERROR_NOT_INITIALIZED = static_cast<int>(0x8a6c0049u);
constexpr std::uint64_t SemaphoreMemoryAlignment = 0x4000u;
constexpr std::uint64_t SemaphoreLabelBytes = 32u;

std::mutex semaphoreMutex;
std::byte* semaphoreMemory = nullptr;
std::uint64_t semaphoreMemoryBytes = 0;

}

extern "C" {

int APS5_VABI sceAgcSetAmmSemaphoreMemory(void* memory, std::uint64_t sizeInBytes) {
    const std::lock_guard lock(semaphoreMutex);
    if (semaphoreMemoryBytes != 0) return GRAPHICS5_ERROR_ALREADY_INITIALIZED;
    const auto address = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(memory));
    if (address == 0 || sizeInBytes == 0 || ((address | sizeInBytes) & (SemaphoreMemoryAlignment - 1u)) != 0) return GRAPHICS5_ERROR_INVALID_ALIGNMENT;
    Agc::Command::Require(sizeInBytes <= std::numeric_limits<std::uintptr_t>::max() - address, __func__, "semaphore memory wraps around the address space");
    std::memset(memory, 0, static_cast<std::size_t>(sizeInBytes));
    semaphoreMemory = static_cast<std::byte*>(memory);
    semaphoreMemoryBytes = sizeInBytes;
    return 0;
}

int APS5_VABI sceAgcGetSemaphoreLabel(std::uint32_t index, void** label) {
    const std::lock_guard lock(semaphoreMutex);
    if (semaphoreMemoryBytes == 0) return GRAPHICS5_ERROR_NOT_INITIALIZED;
    if ((static_cast<std::uint64_t>(index) + 1u) * SemaphoreLabelBytes > semaphoreMemoryBytes) return GRAPHICS5_ERROR_INVALID_VALUE;
    if (label == nullptr) return GRAPHICS5_ERROR_INVALID_ALIGNMENT;
    *label = semaphoreMemory + static_cast<std::uint64_t>(index) * SemaphoreLabelBytes;
    return 0;
}

}
