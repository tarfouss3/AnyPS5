#include "prx/libc/include/general/VabiMacros.hpp"
#include <cstdint>
#include <cstdlib>
#include <cstring>

extern "C" {
int APS5_VABI sceNpCommerceDialogInitialize(void);
int APS5_VABI sceNpCommerceDialogOpen(const void* param);
int APS5_VABI sceNpCommerceDialogOpen2(const void* param);
int APS5_VABI sceNpCommerceDialogUpdateStatus(void);
int APS5_VABI sceNpCommerceDialogGetStatus(void);
int APS5_VABI sceNpCommerceDialogGetResult(void* result);
int APS5_VABI sceNpCommerceDialogClose(void);
int APS5_VABI sceNpCommerceDialogTerminate(void);
}

namespace {

void Require(bool value) { if (!value) std::abort(); }

constexpr int kStatusNone = 0;
constexpr int kStatusInitialized = 1;
constexpr int kStatusFinished = 3;
constexpr int kErrNotInitialized = static_cast<int>(0x80B80003);
constexpr int kErrNotFinished = static_cast<int>(0x80B80005);
constexpr int kErrArgNull = static_cast<int>(0x80B8000D);
constexpr std::int32_t kResultUserCanceled = 1;

}

int main() {
    std::uint8_t param[0x88] = {};
    std::uint8_t result[0x30];
    std::memset(result, 0xa5, sizeof(result));

    Require(sceNpCommerceDialogGetStatus() == kStatusNone);
    Require(sceNpCommerceDialogOpen2(param) == kErrNotInitialized);
    Require(sceNpCommerceDialogGetStatus() == kStatusNone);

    Require(sceNpCommerceDialogInitialize() == 0);
    Require(sceNpCommerceDialogGetStatus() == kStatusInitialized);
    Require(sceNpCommerceDialogOpen2(nullptr) == kErrArgNull);
    Require(sceNpCommerceDialogOpen(nullptr) == kErrArgNull);
    Require(sceNpCommerceDialogGetStatus() == kStatusInitialized);
    Require(sceNpCommerceDialogGetResult(result) == kErrNotFinished);
    Require(result[0] == 0xa5);

    Require(sceNpCommerceDialogOpen2(param) == 0);
    Require(sceNpCommerceDialogGetStatus() == kStatusFinished);
    Require(sceNpCommerceDialogUpdateStatus() == kStatusFinished);
    Require(sceNpCommerceDialogGetResult(nullptr) == kErrArgNull);
    Require(sceNpCommerceDialogGetResult(result) == 0);
    std::int32_t value = 0;
    std::memcpy(&value, result, sizeof(value));
    Require(value == kResultUserCanceled);
    Require(result[4] == 0);
    for (std::size_t i = 5; i < sizeof(result); ++i) Require(result[i] == 0xa5);

    Require(sceNpCommerceDialogClose() == 0);
    Require(sceNpCommerceDialogTerminate() == 0);
    Require(sceNpCommerceDialogGetStatus() == kStatusNone);
}
