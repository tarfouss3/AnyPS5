#include "prx/libc/include/general/VabiMacros.hpp"
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <vector>

extern "C" {
int APS5_VABI sceAudioInOpen(int, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t);
int APS5_VABI sceAudioInHqOpen(int, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t);
int APS5_VABI sceAudioInInput(int, void*);
int APS5_VABI sceAudioInGetSilentState(int);
int APS5_VABI sceAudioInClose(int);
}

static void Require(bool value) { if (!value) std::abort(); }

template <typename TCall>
static void RequireThrows(TCall call) {
    bool threw = false;
    try {
        call();
    } catch (const std::exception&) {
        threw = true;
    }
    Require(threw);
}

static void SetEnvironment(const char* name, const std::string& value) {
#ifdef _WIN32
    _putenv_s(name, value.c_str());
#else
    setenv(name, value.c_str(), 1);
#endif
}

namespace {

constexpr int user = 0x10000000;

void TestValidation() {
    constexpr int invalidSize = static_cast<int>(0x80260102);
    constexpr int invalidFreq = static_cast<int>(0x80260103);
    constexpr int invalidType = static_cast<int>(0x80260104);
    constexpr int invalidParam = static_cast<int>(0x80260106);
    Require(sceAudioInOpen(user, 2, 0, 256, 48000, 2) == invalidType);
    Require(sceAudioInOpen(user, 0, 1, 256, 48000, 2) == invalidParam);
    Require(sceAudioInOpen(user, 0, 0, 512, 48000, 2) == invalidSize);
    Require(sceAudioInOpen(user, 0, 0, 256, 44100, 2) == invalidFreq);
    Require(sceAudioInOpen(user, 0, 0, 256, 48000, 3) == invalidParam);
    Require(sceAudioInOpen(user, 0, 0, 256, 48000, 0) == invalidParam);
    RequireThrows([] { sceAudioInOpen(user, 0, 0, 256, 48000, 0x10); });
    Require(sceAudioInHqOpen(user, 2, 0, 128, 48000, 2) == invalidType);
    Require(sceAudioInHqOpen(user, 0, 1, 128, 48000, 2) == invalidParam);
    Require(sceAudioInHqOpen(user, 0, 0, 256, 48000, 2) == invalidSize);
    Require(sceAudioInHqOpen(user, 0, 0, 512, 48000, 2) == invalidSize);
    Require(sceAudioInHqOpen(user, 0, 0, 128, 16000, 2) == invalidFreq);
    Require(sceAudioInHqOpen(user, 0, 0, 128, 44100, 2) == invalidFreq);
    Require(sceAudioInHqOpen(user, 0, 0, 128, 48000, 3) == invalidParam);
    Require(sceAudioInHqOpen(user, 0, 0, 128, 48000, 0) == invalidParam);
    RequireThrows([] { sceAudioInHqOpen(user, 0, 0, 128, 48000, 0x10); });
}

std::filesystem::path WriteRecording(const char* name, const std::vector<std::int16_t>& recorded) {
    const auto path = std::filesystem::temp_directory_path() / (std::string(name) + "-" + std::to_string(std::random_device{}()) + ".raw");
    std::ofstream(path, std::ios::binary).write(reinterpret_cast<const char*>(recorded.data()), static_cast<std::streamsize>(recorded.size() * 2));
    SetEnvironment("SDL_DISKAUDIOFILEIN", path.string());
    return path;
}

void TestCapture() {
    std::vector<std::int16_t> recorded(256 * 2 * 4);
    for (std::size_t i = 0; i < recorded.size(); ++i) recorded[i] = static_cast<std::int16_t>(i * 7 - 3000);
    SetEnvironment("SDL_AUDIODRIVER", "disk");
    auto path = WriteRecording("anyps5_audio_in_capture", recorded);

    TestValidation();
    int handle = sceAudioInOpen(user, 0, 0, 256, 48000, 2);
    Require(handle > 0 && sceAudioInGetSilentState(handle) == 0);
    std::vector<std::int16_t> block(256 * 2 + 1, 0x5555);
    std::vector<std::int16_t> captured;
    for (int i = 0; i < 4; ++i) {
        Require(sceAudioInInput(handle, block.data()) == 256);
        captured.insert(captured.end(), block.begin(), block.end() - 1);
        Require(block.back() == 0x5555);
    }
    Require(captured == recorded);
    Require(sceAudioInInput(handle, block.data()) == 256);
    for (std::size_t i = 0; i < 256 * 2; ++i) Require(block[i] == 0);
    Require(sceAudioInClose(handle) == 0);
    std::filesystem::remove(path);

    recorded.assign(128 * 4, 0);
    for (std::size_t i = 0; i < recorded.size(); ++i) recorded[i] = static_cast<std::int16_t>(5000 - i * 11);
    path = WriteRecording("anyps5_audio_in_hq_capture", recorded);
    handle = sceAudioInHqOpen(user, 1, 0, 128, 48000, 1);
    Require(handle > 0 && sceAudioInGetSilentState(handle) == 0);
    block.assign(128 + 1, 0x5555);
    captured.clear();
    for (int i = 0; i < 4; ++i) {
        Require(sceAudioInInput(handle, block.data()) == 128);
        captured.insert(captured.end(), block.begin(), block.end() - 1);
        Require(block.back() == 0x5555);
    }
    Require(captured == recorded);
    Require(sceAudioInClose(handle) == 0);
    std::filesystem::remove(path);
}

void TestNoDevice() {
    constexpr int invalidHandle = static_cast<int>(0x80260101);
    constexpr int portFull = static_cast<int>(0x80260107);
    SetEnvironment("SDL_AUDIODRIVER", "none");

    TestValidation();
    const int handle = sceAudioInOpen(user, 0, 0, 256, 16000, 0x12);
    Require(handle > 0 && sceAudioInGetSilentState(handle) == 1);
    Require(sceAudioInGetSilentState(handle + 1) == invalidHandle);
    Require(sceAudioInInput(handle + 1, nullptr) == invalidHandle);
    std::vector<std::uint8_t> buffer(256 * 8 + 1, 0xCC);
    const auto start = std::chrono::steady_clock::now();
    for (int block = 0; block < 5; ++block) Require(sceAudioInInput(handle, buffer.data()) == 256);
    const auto elapsed = std::chrono::steady_clock::now() - start;
    Require(elapsed >= std::chrono::milliseconds(4 * 16) && elapsed < std::chrono::milliseconds(1000));
    for (std::size_t i = 0; i < 256 * 8; ++i) Require(buffer[i] == 0);
    Require(buffer[256 * 8] == 0xCC);
    Require(sceAudioInInput(handle, nullptr) == 0);
    for (int port = 1; port < 8; ++port) Require(sceAudioInOpen(user, 1, 0, 128, 48000, 1) > 0);
    Require(sceAudioInOpen(user, 1, 0, 128, 48000, 1) == portFull);
    Require(sceAudioInClose(handle) == 0);
    Require(sceAudioInClose(handle) == invalidHandle);
    Require(sceAudioInInput(handle, buffer.data()) == invalidHandle);
    Require(sceAudioInOpen(user, 1, 0, 128, 48000, 0x11) == handle);
    Require(sceAudioInHqOpen(user, 0, 0, 128, 48000, 2) == portFull);
    Require(sceAudioInClose(handle) == 0);

    const int hq = sceAudioInHqOpen(user, 0, 0, 128, 48000, 0x12);
    Require(hq == handle && sceAudioInGetSilentState(hq) == 1);
    std::vector<std::uint8_t> hqBuffer(128 * 8 + 1, 0xCC);
    const auto hqStart = std::chrono::steady_clock::now();
    for (int block = 0; block < 5; ++block) Require(sceAudioInInput(hq, hqBuffer.data()) == 128);
    const auto hqElapsed = std::chrono::steady_clock::now() - hqStart;
    Require(hqElapsed >= std::chrono::microseconds(4 * 2666) && hqElapsed < std::chrono::milliseconds(1000));
    for (std::size_t i = 0; i < 128 * 8; ++i) Require(hqBuffer[i] == 0);
    Require(hqBuffer[128 * 8] == 0xCC);
    Require(sceAudioInClose(hq) == 0);
    Require(sceAudioInInput(hq, hqBuffer.data()) == invalidHandle);
}

}

int main(int argc, char** argv) {
    Require(argc == 2);
    if (std::strcmp(argv[1], "capture") == 0) TestCapture();
    else TestNoDevice();
}
