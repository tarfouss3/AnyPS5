#include "SceTypes.hpp"
#include <cstdint>
#include <cstdlib>
#include <set>
#include <stdexcept>
#include <vector>

extern "C" int APS5_VABI sceContentExportInit2(const ContentExportInitParam2* init_param);
extern "C" int APS5_VABI sceContentExportTerm(void);
extern "C" int APS5_VABI sceContentExportStart(void);
extern "C" int APS5_VABI sceContentExportFinish(std::int32_t exp_id);

namespace {

void Require(bool value) { if (!value) std::abort(); }

template <typename TAction>
bool Throws(TAction action) {
    try {
        action();
    } catch (const std::logic_error&) {
        return true;
    }
    return false;
}

template <typename TAction>
bool ThrowsRuntime(TAction action) {
    try {
        action();
    } catch (const std::runtime_error&) {
        return true;
    }
    return false;
}

int allocator = 0;

}

int main() {
    ContentExportInitParam2 param{&allocator, &allocator, nullptr, 0x10000, 0, 0};
    Require(Throws([] { sceContentExportInit2(nullptr); }));
    Require(Throws([] { sceContentExportTerm(); }));
    ContentExportInitParam2 missing = param;
    missing.free_func = nullptr;
    Require(Throws([&] { sceContentExportInit2(&missing); }));
    ContentExportInitParam2 reserved = param;
    reserved.reserved1 = 1;
    Require(Throws([&] { sceContentExportInit2(&reserved); }));
    Require(sceContentExportInit2(&param) == 0);
    Require(Throws([&] { sceContentExportInit2(&param); }));
    Require(sceContentExportTerm() == 0);
    Require(Throws([] { sceContentExportTerm(); }));

    Require(Throws([] { sceContentExportStart(); }));
    Require(Throws([] { sceContentExportFinish(1); }));
    Require(sceContentExportInit2(&param) == 0);
    std::vector<int> exports;
    std::set<int> distinct;
    for (int index = 0; index < 10; ++index) {
        const int exportId = sceContentExportStart();
        Require(exportId >= 0);
        exports.push_back(exportId);
        distinct.insert(exportId);
    }
    Require(distinct.size() == exports.size());
    Require(ThrowsRuntime([] { sceContentExportStart(); }));
    Require(ThrowsRuntime([] { sceContentExportTerm(); }));
    Require(sceContentExportFinish(exports.front()) == 0);
    Require(Throws([&] { sceContentExportFinish(exports.front()); }));
    distinct.erase(exports.front());
    const int restarted = sceContentExportStart();
    Require(restarted >= 0 && !distinct.contains(restarted));
    exports.front() = restarted;
    for (const int exportId : exports) Require(sceContentExportFinish(exportId) == 0);
    Require(sceContentExportTerm() == 0);
    Require(Throws([] { sceContentExportStart(); }));
    Require(sceContentExportInit2(&param) == 0);
    Require(sceContentExportTerm() == 0);
}
