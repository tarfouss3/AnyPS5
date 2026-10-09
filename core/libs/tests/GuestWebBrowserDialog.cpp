#include "prx/libc/include/general/VabiMacros.hpp"
#include <cstdint>
#include <cstdlib>
#include <stdexcept>

extern "C" {
int APS5_VABI sceWebBrowserDialogInitialize(void);
int APS5_VABI sceWebBrowserDialogTerminate(void);
int APS5_VABI sceWebBrowserDialogOpen(const void* param);
int APS5_VABI sceWebBrowserDialogOpenForPredeterminedContent(const void* param, const void* predeterminedContentParam);
int APS5_VABI sceWebBrowserDialogGetStatus(void);
int APS5_VABI sceWebBrowserDialogUpdateStatus(void);
int APS5_VABI sceWebBrowserDialogGetResult(void* result);
int APS5_VABI sceWebBrowserDialogSetCookie(const void* param);
int APS5_VABI sceWebBrowserDialogResetCookie(const void* param);
}

namespace {

constexpr int COMMON_DIALOG_STATUS_NONE = 0;
constexpr int COMMON_DIALOG_STATUS_INITIALIZED = 1;
constexpr int COMMON_DIALOG_STATUS_FINISHED = 3;
constexpr int COMMON_DIALOG_RESULT_USER_CANCELED = 1;
constexpr int COMMON_DIALOG_ERROR_NOT_INITIALIZED = static_cast<int>(0x80B80003u);
constexpr int COMMON_DIALOG_ERROR_ARG_NULL = static_cast<int>(0x80B8000Du);

struct PredeterminedContentParam {
    std::uint64_t size;
    const char* domain[20];
    std::uint8_t reserved[256];
};

struct ResetCookieParam {
    std::uint64_t size;
    std::uint8_t reserved[256];
};

struct SetCookieParam {
    std::uint64_t size;
    const char* url;
    const char* cookie;
    std::uint8_t reserved[256];
};

void Require(bool value) { if (!value) std::abort(); }

template <typename TCall>
bool Throws(TCall call) {
    try {
        call();
    } catch (const std::runtime_error&) {
        return true;
    }
    return false;
}

void CheckPredeterminedContent() {
    std::uint8_t param[328] = {};
    PredeterminedContentParam content{};
    content.size = sizeof(content);
    content.domain[0] = "https://example.com/";
    std::int32_t result[2] = {-1, -1};

    Require(sceWebBrowserDialogOpenForPredeterminedContent(param, &content) == COMMON_DIALOG_ERROR_NOT_INITIALIZED);
    Require(sceWebBrowserDialogInitialize() == 0);
    Require(sceWebBrowserDialogOpenForPredeterminedContent(nullptr, &content) == COMMON_DIALOG_ERROR_ARG_NULL);
    Require(Throws([&] { sceWebBrowserDialogOpenForPredeterminedContent(param, nullptr); }));
    PredeterminedContentParam badSize = content;
    badSize.size = sizeof(badSize) - 8;
    Require(Throws([&] { sceWebBrowserDialogOpenForPredeterminedContent(param, &badSize); }));
    Require(sceWebBrowserDialogGetStatus() == COMMON_DIALOG_STATUS_INITIALIZED);
    Require(sceWebBrowserDialogOpenForPredeterminedContent(param, &content) == 0);
    Require(sceWebBrowserDialogGetStatus() == COMMON_DIALOG_STATUS_FINISHED);
    Require(sceWebBrowserDialogUpdateStatus() == COMMON_DIALOG_STATUS_FINISHED);
    Require(sceWebBrowserDialogGetResult(result) == 0);
    Require(result[0] == COMMON_DIALOG_RESULT_USER_CANCELED);
    Require(result[1] == -1);
    Require(sceWebBrowserDialogTerminate() == 0);
}

void CheckCookies() {
    SetCookieParam set{};
    set.size = sizeof(set);
    set.url = "https://example.com/";
    set.cookie = "name=value";
    ResetCookieParam reset{};
    reset.size = sizeof(reset);

    Require(Throws([&] { sceWebBrowserDialogSetCookie(&set); }));
    Require(Throws([&] { sceWebBrowserDialogResetCookie(&reset); }));
    Require(sceWebBrowserDialogInitialize() == 0);
    Require(Throws([] { sceWebBrowserDialogSetCookie(nullptr); }));
    SetCookieParam badSize = set;
    badSize.size = 0;
    Require(Throws([&] { sceWebBrowserDialogSetCookie(&badSize); }));
    SetCookieParam noUrl = set;
    noUrl.url = nullptr;
    Require(Throws([&] { sceWebBrowserDialogSetCookie(&noUrl); }));
    SetCookieParam noCookie = set;
    noCookie.cookie = nullptr;
    Require(Throws([&] { sceWebBrowserDialogSetCookie(&noCookie); }));
    Require(sceWebBrowserDialogSetCookie(&set) == 0);
    Require(Throws([] { sceWebBrowserDialogResetCookie(nullptr); }));
    ResetCookieParam resetBadSize = reset;
    resetBadSize.size = sizeof(resetBadSize) + 8;
    Require(Throws([&] { sceWebBrowserDialogResetCookie(&resetBadSize); }));
    Require(sceWebBrowserDialogResetCookie(&reset) == 0);
    Require(sceWebBrowserDialogGetStatus() == COMMON_DIALOG_STATUS_INITIALIZED);
    Require(sceWebBrowserDialogTerminate() == 0);
}

}

int main() {
    Require(sceWebBrowserDialogGetStatus() == COMMON_DIALOG_STATUS_NONE);
    Require(sceWebBrowserDialogInitialize() == 0);
    Require(sceWebBrowserDialogGetStatus() == COMMON_DIALOG_STATUS_INITIALIZED);
    Require(sceWebBrowserDialogTerminate() == 0);
    Require(sceWebBrowserDialogGetStatus() == COMMON_DIALOG_STATUS_NONE);
    Require(sceWebBrowserDialogInitialize() == 0);
    std::uint8_t param[64] = {};
    Require(sceWebBrowserDialogOpen(param) == 0);
    Require(sceWebBrowserDialogGetStatus() == COMMON_DIALOG_STATUS_FINISHED);
    Require(sceWebBrowserDialogUpdateStatus() == COMMON_DIALOG_STATUS_FINISHED);
    Require(sceWebBrowserDialogTerminate() == 0);
    Require(sceWebBrowserDialogGetStatus() == COMMON_DIALOG_STATUS_NONE);
    CheckPredeterminedContent();
    CheckCookies();
}
