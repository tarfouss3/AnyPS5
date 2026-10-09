#include "prx/libc/include/general/VabiMacros.hpp"
#include <cstdint>
#include <cstdlib>

extern "C" {
int APS5_VABI sceSigninDialogInitialize(void);
int APS5_VABI sceSigninDialogOpen(const void* param);
int APS5_VABI sceSigninDialogUpdateStatus(void);
int APS5_VABI sceSigninDialogGetStatus(void);
int APS5_VABI sceSigninDialogGetResult(void* result);
int APS5_VABI sceSigninDialogClose(void);
int APS5_VABI sceSigninDialogTerminate(void);
}

namespace {

void Require(bool value) { if (!value) std::abort(); }

constexpr int kStatusNone = 0;
constexpr int kStatusInitialized = 1;
constexpr int kStatusFinished = 3;
constexpr int kErrNotInitialized = static_cast<int>(0x80B80003);
constexpr int kErrNotFinished = static_cast<int>(0x80B80005);
constexpr int kErrNotRunning = static_cast<int>(0x80B8000B);
constexpr int kErrArgNull = static_cast<int>(0x80B8000D);
constexpr std::int32_t kResultUserCanceled = 1;

}

int main() {
    struct {
        std::int32_t size;
        std::int32_t userId;
        std::int32_t reserved[2];
    } param{16, 1, {0, 0}};
    std::int32_t result[5] = {-1, -1, -1, -1, -1};

    Require(sceSigninDialogGetStatus() == kStatusNone);
    Require(sceSigninDialogGetResult(result) == kErrNotInitialized);
    Require(sceSigninDialogGetResult(nullptr) == kErrNotInitialized);
    Require(sceSigninDialogClose() == kErrNotRunning);

    Require(sceSigninDialogInitialize() == 0);
    Require(sceSigninDialogGetStatus() == kStatusInitialized);
    Require(sceSigninDialogGetResult(nullptr) == kErrArgNull);
    Require(sceSigninDialogGetResult(result) == kErrNotFinished);
    Require(result[0] == -1);
    Require(sceSigninDialogClose() == kErrNotRunning);
    Require(sceSigninDialogGetStatus() == kStatusInitialized);

    Require(sceSigninDialogOpen(&param) == 0);
    Require(sceSigninDialogGetStatus() == kStatusFinished);
    Require(sceSigninDialogUpdateStatus() == kStatusFinished);
    Require(sceSigninDialogClose() == kErrNotRunning);
    Require(sceSigninDialogGetStatus() == kStatusFinished);

    Require(sceSigninDialogGetResult(nullptr) == kErrArgNull);
    Require(sceSigninDialogGetResult(result) == 0);
    Require(result[0] == kResultUserCanceled);
    for (int i = 1; i < 5; ++i) Require(result[i] == -1);

    Require(sceSigninDialogTerminate() == 0);
    Require(sceSigninDialogGetStatus() == kStatusNone);
    Require(sceSigninDialogGetResult(result) == kErrNotInitialized);
    Require(sceSigninDialogTerminate() == kErrNotInitialized);
}
