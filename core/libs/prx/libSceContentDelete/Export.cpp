#include <cstdint>
#include <cstddef>
#include "SceTypes.hpp"
#include "prx/libc/include/General.hpp"
#include <atomic>
#include <stdexcept>
#include <string>

namespace {
constexpr int CONTENT_DELETE_ERROR_FILE_NOT_FOUND = static_cast<int>(0x809D5005);
std::atomic<bool> g_initialized{false};
}

extern "C" {

int APS5_VABI sceContentDeleteInitialize(const ContentDeleteInitParam* init_param) {
    if (init_param == nullptr) APS5_INVALID_ARG_EX;
    bool expected = false;
    if (!g_initialized.compare_exchange_strong(expected, true)) throw std::logic_error(std::string(__func__) + ": already initialized");
    return 0;
}

int APS5_VABI sceContentDeleteByPath(const char* content_path) {
    if (!g_initialized.load()) throw std::logic_error(std::string(__func__) + ": not initialized");
    if (content_path == nullptr) APS5_INVALID_ARG_EX;
    return CONTENT_DELETE_ERROR_FILE_NOT_FOUND;
}

int APS5_VABI sceContentDeleteTerminate(void) {
    bool expected = true;
    if (!g_initialized.compare_exchange_strong(expected, false)) throw std::logic_error(std::string(__func__) + ": not initialized");
    return 0;
}

}
