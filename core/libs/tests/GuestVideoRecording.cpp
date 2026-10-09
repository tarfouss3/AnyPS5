#include "prx/libc/include/general/VabiMacros.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdlib>

extern "C" {
int APS5_VABI sceVideoRecordingGetStatus(void);
int APS5_VABI sceVideoRecordingSetInfo(std::int32_t info, const void* data, std::size_t size);
}

namespace {

void Require(bool value) { if (!value) std::abort(); }

}

int main() {
    constexpr int statusNone = 0;
    Require(sceVideoRecordingGetStatus() == statusNone);
    const char subtitle[] = "subtitle";
    Require(sceVideoRecordingSetInfo(2, subtitle, sizeof(subtitle)) == 0);
    Require(sceVideoRecordingGetStatus() == statusNone);
    return 0;
}
