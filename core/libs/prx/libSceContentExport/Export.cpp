#include <cstdint>
#include <cstddef>
#include "SceTypes.hpp"
#include "prx/libc/include/General.hpp"
#include <limits>
#include <mutex>
#include <set>
#include <stdexcept>
#include <string>

namespace {
constexpr std::size_t MAX_EXECUTION = 10;
std::mutex g_mutex;
bool g_initialized = false;
std::set<int32_t> g_exports;
int32_t g_nextExportId = 1;

int32_t FollowingExportId(int32_t exportId) {
    return exportId == std::numeric_limits<int32_t>::max() ? 1 : exportId + 1;
}
}

extern "C" {

int APS5_VABI sceContentExportInit2(const ContentExportInitParam2* init_param) {
    if (init_param == nullptr) APS5_INVALID_ARG_EX;
    if (init_param->malloc_func == nullptr || init_param->free_func == nullptr) throw std::invalid_argument(std::string(__func__) + ": missing allocator functions");
    if (init_param->reserved0 != 0 || init_param->reserved1 != 0) throw std::invalid_argument(std::string(__func__) + ": reserved fields are not zero");
    std::lock_guard lock(g_mutex);
    if (g_initialized) throw std::logic_error(std::string(__func__) + ": already initialized");
    g_initialized = true;
    return 0;
}

int APS5_VABI sceContentExportFinish(int32_t exp_id) {
    std::lock_guard lock(g_mutex);
    if (!g_initialized) throw std::logic_error(std::string(__func__) + ": not initialized");
    if (g_exports.erase(exp_id) == 0) throw std::invalid_argument(std::string(__func__) + ": export " + std::to_string(exp_id) + " was not started");
    return 0;
}

int APS5_VABI sceContentExportFromData(void) {
 NotImplemented_nid_no_patch(__func__);
 return 0;
}

int APS5_VABI sceContentExportFromFile(void) {
 NotImplemented_nid_no_patch(__func__);
 return 0;
}

int APS5_VABI sceContentExportFromFileWithThumbnail(void) {
 NotImplemented_nid_no_patch(__func__);
 return 0;
}

int APS5_VABI sceContentExportStart(void) {
    std::lock_guard lock(g_mutex);
    if (!g_initialized) throw std::logic_error(std::string(__func__) + ": not initialized");
    if (g_exports.size() >= MAX_EXECUTION) throw std::runtime_error(std::string(__func__) + ": more than " + std::to_string(MAX_EXECUTION) + " exports at once");
    int32_t exportId = g_nextExportId;
    while (g_exports.contains(exportId)) exportId = FollowingExportId(exportId);
    g_exports.insert(exportId);
    g_nextExportId = FollowingExportId(exportId);
    return exportId;
}

int APS5_VABI sceContentExportTerm(void) {
    std::lock_guard lock(g_mutex);
    if (!g_initialized) throw std::logic_error(std::string(__func__) + ": not initialized");
    if (!g_exports.empty()) throw std::runtime_error(std::string(__func__) + ": " + std::to_string(g_exports.size()) + " exports were not finished");
    g_initialized = false;
    return 0;
}

}
