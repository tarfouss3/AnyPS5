#include "SceTypes.hpp"
#include <cstdlib>
#include <stdexcept>

extern "C" int APS5_VABI sceContentDeleteInitialize(const ContentDeleteInitParam* init_param);
extern "C" int APS5_VABI sceContentDeleteByPath(const char* content_path);
extern "C" int APS5_VABI sceContentDeleteTerminate(void);

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

}

int main() {
    constexpr int fileNotFound = static_cast<int>(0x809D5005);
    ContentDeleteInitParam param{};
    param.heap_size = 16 * 1024;
    Require(Throws([] { sceContentDeleteByPath("/user/av_contents/photo/capture.jpg"); }));
    Require(sceContentDeleteInitialize(&param) == 0);
    Require(sceContentDeleteByPath("/user/av_contents/photo/capture.jpg") == fileNotFound);
    Require(sceContentDeleteByPath("") == fileNotFound);
    Require(Throws([] { sceContentDeleteByPath(nullptr); }));
    Require(sceContentDeleteTerminate() == 0);
    Require(Throws([] { sceContentDeleteByPath("/user/av_contents/photo/capture.jpg"); }));
}
