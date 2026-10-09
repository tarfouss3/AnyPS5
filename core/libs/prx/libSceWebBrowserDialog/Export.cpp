#include <atomic>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <cstddef>
#include "SceTypes.hpp"
#include "prx/libc/include/General.hpp"

namespace {
std::atomic<int> g_status{0};

constexpr int COMMON_DIALOG_STATUS_NONE = 0;
constexpr int COMMON_DIALOG_STATUS_RUNNING = 2;
constexpr int COMMON_DIALOG_STATUS_FINISHED = 3;
constexpr int COMMON_DIALOG_RESULT_USER_CANCELED = 1;
constexpr int COMMON_DIALOG_ERROR_NOT_INITIALIZED = static_cast<int>(0x80B80003u);
constexpr int COMMON_DIALOG_ERROR_NOT_FINISHED = static_cast<int>(0x80B80005u);
constexpr int COMMON_DIALOG_ERROR_BUSY = static_cast<int>(0x80B80008u);
constexpr int COMMON_DIALOG_ERROR_ARG_NULL = static_cast<int>(0x80B8000Du);

struct WebBrowserDialogPredeterminedContentParam {
    std::uint64_t size;
    const char* domain[20];
    std::uint8_t reserved[256];
};

struct WebBrowserDialogResetCookieParam {
    std::uint64_t size;
    std::uint8_t reserved[256];
};

struct WebBrowserDialogSetCookieParam {
    std::uint64_t size;
    const char* url;
    const char* cookie;
    std::uint8_t reserved[256];
};

static_assert(sizeof(WebBrowserDialogPredeterminedContentParam) == 424);
static_assert(sizeof(WebBrowserDialogResetCookieParam) == 264);
static_assert(sizeof(WebBrowserDialogSetCookieParam) == 280);
}

extern "C" {

int APS5_VABI sceWebBrowserDialogInitialize(void) {
    int expected = 0;
    if (!g_status.compare_exchange_strong(expected, 1)) throw std::logic_error("sceWebBrowserDialogInitialize: already initialized");
    return 0;
}

int APS5_VABI sceWebBrowserDialogTerminate(void) {
    int expected = 1;
    if (g_status.compare_exchange_strong(expected, 0)) return 0;
    expected = COMMON_DIALOG_STATUS_FINISHED;
    if (!g_status.compare_exchange_strong(expected, 0)) throw std::logic_error("sceWebBrowserDialogTerminate: not initialized or still running");
    return 0;
}

int APS5_VABI sceWebBrowserDialogClose(void) {
 if (g_status.load() == COMMON_DIALOG_STATUS_NONE) return COMMON_DIALOG_ERROR_NOT_INITIALIZED;
 return 0;
}

int APS5_VABI sceWebBrowserDialogGetResult(void* result) {
 const int status = g_status.load();
 if (status == COMMON_DIALOG_STATUS_NONE) return COMMON_DIALOG_ERROR_NOT_INITIALIZED;
 if (result == nullptr) return COMMON_DIALOG_ERROR_ARG_NULL;
 if (status != COMMON_DIALOG_STATUS_FINISHED) return COMMON_DIALOG_ERROR_NOT_FINISHED;
 *static_cast<std::int32_t*>(result) = COMMON_DIALOG_RESULT_USER_CANCELED;
 return 0;
}

int APS5_VABI sceWebBrowserDialogGetStatus(void) {
    return g_status.load();
}

int APS5_VABI sceWebBrowserDialogOpen(const void* param) {
 const int status = g_status.load();
 if (status == COMMON_DIALOG_STATUS_NONE) return COMMON_DIALOG_ERROR_NOT_INITIALIZED;
 if (status == COMMON_DIALOG_STATUS_RUNNING) return COMMON_DIALOG_ERROR_BUSY;
 if (param == nullptr) return COMMON_DIALOG_ERROR_ARG_NULL;
 g_status = COMMON_DIALOG_STATUS_FINISHED;
 return 0;
}

int APS5_VABI sceWebBrowserDialogUpdateStatus(void) {
    return g_status.load();
}

int APS5_VABI sceWebBrowserDialogSetCookie(const WebBrowserDialogSetCookieParam* param) {
    if (g_status.load() == COMMON_DIALOG_STATUS_NONE) throw std::runtime_error(std::string(__func__) + ": not initialized");
    if (param == nullptr) throw std::runtime_error(std::string(__func__) + ": null parameter");
    if (param->size != sizeof(WebBrowserDialogSetCookieParam)) throw std::runtime_error(std::string(__func__) + ": unsupported parameter size " + std::to_string(param->size));
    if (param->url == nullptr || param->cookie == nullptr) throw std::runtime_error(std::string(__func__) + ": null URL or cookie");
    return 0;
}

int APS5_VABI sceWebBrowserDialogOpenForPredeterminedContent(const void* param, const WebBrowserDialogPredeterminedContentParam* predeterminedContentParam) {
    const int status = g_status.load();
    if (status == COMMON_DIALOG_STATUS_NONE) return COMMON_DIALOG_ERROR_NOT_INITIALIZED;
    if (status == COMMON_DIALOG_STATUS_RUNNING) return COMMON_DIALOG_ERROR_BUSY;
    if (param == nullptr) return COMMON_DIALOG_ERROR_ARG_NULL;
    if (predeterminedContentParam == nullptr) throw std::runtime_error(std::string(__func__) + ": null predetermined content parameter");
    if (predeterminedContentParam->size != sizeof(WebBrowserDialogPredeterminedContentParam)) throw std::runtime_error(std::string(__func__) + ": unsupported predetermined content parameter size " + std::to_string(predeterminedContentParam->size));
    g_status = COMMON_DIALOG_STATUS_FINISHED;
    return 0;
}

int APS5_VABI sceWebBrowserDialogResetCookie(const WebBrowserDialogResetCookieParam* param) {
    if (g_status.load() == COMMON_DIALOG_STATUS_NONE) throw std::runtime_error(std::string(__func__) + ": not initialized");
    if (param == nullptr) throw std::runtime_error(std::string(__func__) + ": null parameter");
    if (param->size != sizeof(WebBrowserDialogResetCookieParam)) throw std::runtime_error(std::string(__func__) + ": unsupported parameter size " + std::to_string(param->size));
    return 0;
}

}
