#include "SceTypes.hpp"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <exception>

extern "C" {
int APS5_VABI sceVoiceInit(VoiceInitParam* param, std::int32_t version);
int APS5_VABI sceVoiceEnd_nid_postfix(void);
int APS5_VABI sceVoiceCreatePort(std::uint32_t* portId, const VoicePortParam* param);
int APS5_VABI sceVoiceDeletePort(std::uint32_t portId);
int APS5_VABI sceVoiceConnectIPortToOPort(std::uint32_t inputPortId, std::uint32_t outputPortId);
int APS5_VABI sceVoiceDisconnectIPortFromOPort(std::uint32_t inputPortId, std::uint32_t outputPortId);
int APS5_VABI sceVoiceSetVolume(std::uint32_t portId, float volume);
int APS5_VABI sceVoiceGetVolume(std::uint32_t portId, float* volume);
int APS5_VABI sceVoiceGetPortInfo(std::uint32_t portId, VoicePortInfo* info);
int APS5_VABI sceVoiceSetMuteFlag(std::uint32_t portId, bool muted);
}

namespace {

constexpr std::int32_t PORT_IN_PCM = 1;
constexpr std::int32_t PORT_OUT_PCM = 3;

void Require(bool condition, const char* message) {
    if (condition) return;
    std::fprintf(stderr, "Voice: %s\n", message);
    std::abort();
}

template <typename TCall>
void RequireThrows(TCall call, const char* message) {
    bool threw = false;
    try {
        call();
    } catch (const std::exception&) {
        threw = true;
    }
    Require(threw, message);
}

std::uint32_t CreatePcmPort(std::int32_t type, std::uint16_t mute) {
    VoicePortParam param{};
    param.port_type = type;
    param.mute = mute;
    param.volume = 1.0f;
    param.pcmaudio.buffer_size = 0x1000;
    std::uint32_t id = 0xFFFFFFFF;
    Require(sceVoiceCreatePort(&id, &param) == 0, "port creation failed");
    return id;
}

}

int main() {
    RequireThrows([] { sceVoiceSetMuteFlag(0, true); }, "mute flag before initialization must throw");

    VoiceInitParam init{};
    Require(sceVoiceInit(&init, 0) == 0, "initialization failed");
    const auto input = CreatePcmPort(PORT_IN_PCM, 0);
    const auto output = CreatePcmPort(PORT_OUT_PCM, 1);

    Require(sceVoiceConnectIPortToOPort(input, output) == 0, "connection failed");
    Require(sceVoiceSetVolume(input, 0.25f) == 0, "volume change failed");
    Require(sceVoiceSetMuteFlag(input, true) == 0, "muting an input port failed");
    Require(sceVoiceSetMuteFlag(output, false) == 0, "unmuting an output port failed");
    Require(sceVoiceSetMuteFlag(input, false) == 0, "unmuting an input port failed");
    RequireThrows([&] { sceVoiceSetMuteFlag(output + 1, true); }, "mute flag of an unknown port must throw");
    float volume = 0.0f;
    Require(sceVoiceGetVolume(input, &volume) == 0 && volume == 0.25f, "mute flag changed the volume");
    VoicePortInfo info{};
    Require(sceVoiceGetPortInfo(output, &info) == 0 && info.port_type == PORT_OUT_PCM && info.byte_count == 0, "mute flag changed the port info");
    Require(sceVoiceDisconnectIPortFromOPort(input, output) == 0, "mute flag removed the connection");

    Require(sceVoiceDeletePort(input) == 0, "port deletion failed");
    RequireThrows([&] { sceVoiceSetMuteFlag(input, false); }, "mute flag of a deleted port must throw");

    Require(sceVoiceEnd_nid_postfix() == 0, "end failed");
    RequireThrows([&] { sceVoiceSetMuteFlag(output, true); }, "mute flag after end must throw");
}
