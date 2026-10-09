#include "prx/libSceAgc/Misc/include/Instrumentation.hpp"
#include "prx/libSceAgc/Command/include/Packet.hpp"

#include <atomic>
#include <cstdint>
#include "SceTypes.hpp"
#include "prx/libc/include/General.hpp"

namespace {

std::atomic<std::uint32_t> shaderInstrumentation{0};

}

extern "C" {

int APS5_VABI sceAgcSetShaderInstrumentation(std::uint32_t flags) {
    Agc::Command::Require(flags == 0, __func__, "shader instrumentation is not implemented; only 0 is accepted");
    shaderInstrumentation.store(flags);
    return 0;
}

std::uint32_t APS5_VABI sceAgcGetShaderInstrumentation() {
    return shaderInstrumentation.load();
}

}
