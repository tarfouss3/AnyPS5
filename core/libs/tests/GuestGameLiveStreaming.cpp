#include "prx/libc/include/general/VabiMacros.hpp"
#include <cstdint>
#include <cstdlib>
#include <cstring>

extern "C" {
int APS5_VABI sceGameLiveStreamingGetCurrentStatus2(void* status);
}

namespace {

void Require(bool value) { if (!value) std::abort(); }

}

int main() {
    constexpr std::int32_t invalidParam = static_cast<std::int32_t>(0x80A00002);
    std::uint8_t status[76];
    std::memset(status, 0x5a, sizeof(status));
    Require(sceGameLiveStreamingGetCurrentStatus2(status) == 0);
    std::int32_t userId = 0;
    std::memcpy(&userId, status, sizeof(userId));
    Require(userId == -1);
    for (std::size_t index = 4; index < 72; ++index) Require(status[index] == 0);
    for (std::size_t index = 72; index < sizeof(status); ++index) Require(status[index] == 0x5a);
    Require(sceGameLiveStreamingGetCurrentStatus2(nullptr) == invalidParam);
    return 0;
}
