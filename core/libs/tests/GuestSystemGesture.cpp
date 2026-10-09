#include "SceTypes.hpp"
#include "prx/libc/include/general/VabiMacros.hpp"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

extern "C" {
int APS5_VABI sceSystemGestureInitializePrimitiveTouchRecognizer(const void* param);
int APS5_VABI sceSystemGestureFinalizePrimitiveTouchRecognizer(void);
int32_t APS5_VABI sceSystemGestureOpen(int32_t input_type, const void* param);
int APS5_VABI sceSystemGestureClose(int32_t gesture_handle);
int APS5_VABI sceSystemGestureCreateTouchRecognizer(int32_t gesture_handle, SystemGestureTouchRecognizer* recognizer, int32_t type, const SystemGestureRectangle* rectangle, const void* param);
int APS5_VABI sceSystemGestureAppendTouchRecognizer(int32_t gesture_handle, SystemGestureTouchRecognizer* recognizer);
int APS5_VABI sceSystemGestureRemoveTouchRecognizer(int32_t gesture_handle, SystemGestureTouchRecognizer* recognizer);
int APS5_VABI sceSystemGestureResetTouchRecognizer(int32_t gesture_handle, SystemGestureTouchRecognizer* recognizer);
int APS5_VABI sceSystemGestureResetPrimitiveTouchRecognizer(int32_t gesture_handle);
int APS5_VABI sceSystemGestureUpdatePrimitiveTouchRecognizer(int32_t gesture_handle, const void* param);
int APS5_VABI sceSystemGestureUpdateTouchRecognizer(int32_t gesture_handle, SystemGestureTouchRecognizer* recognizer);
int APS5_VABI sceSystemGestureUpdateAllTouchRecognizer(int32_t gesture_handle);
int APS5_VABI sceSystemGestureUpdateTouchRecognizerRectangle(int32_t gesture_handle, SystemGestureTouchRecognizer* recognizer, const SystemGestureRectangle* rectangle);
int APS5_VABI sceSystemGestureGetPrimitiveTouchEventsCount(int32_t gesture_handle);
int APS5_VABI sceSystemGestureGetPrimitiveTouchEvents(int32_t gesture_handle, SystemGesturePrimitiveTouchEvent* event_buffer, uint32_t capacity_of_buffer, uint32_t* number_of_event);
int APS5_VABI sceSystemGestureGetPrimitiveTouchEventByIndex(int32_t gesture_handle, uint32_t index, SystemGesturePrimitiveTouchEvent* event);
int APS5_VABI sceSystemGestureGetPrimitiveTouchEventByPrimitiveID(int32_t gesture_handle, uint16_t primitiveId, SystemGesturePrimitiveTouchEvent* event);
int APS5_VABI sceSystemGestureGetTouchEventsCount(int32_t gesture_handle, const SystemGestureTouchRecognizer* recognizer);
int APS5_VABI sceSystemGestureGetTouchEvents(int32_t gesture_handle, const SystemGestureTouchRecognizer* recognizer, SystemGestureTouchEvent* event_buffer, uint32_t capacity_of_buffer, uint32_t* number_of_event);
int APS5_VABI sceSystemGestureGetTouchEventByIndex(int32_t gesture_handle, const SystemGestureTouchRecognizer* recognizer, uint32_t index, SystemGestureTouchEvent* event);
int APS5_VABI sceSystemGestureGetTouchEventByEventID(int32_t gesture_handle, const SystemGestureTouchRecognizer* recognizer, uint32_t eventId, SystemGestureTouchEvent* event);
}

namespace {

constexpr int InvalidArgument = static_cast<int>(0x80890001);
constexpr int EventDataNotFound = static_cast<int>(0x80890005);
constexpr int InvalidHandle = static_cast<int>(0x80890006);
constexpr std::uint32_t Untouched = 0xA5A5A5A5u;

void Require(bool condition, const char* message) {
    if (!condition) {
        std::fprintf(stderr, "SystemGesture: %s\n", message);
        std::abort();
    }
}

void CheckPrimitiveEvents(int32_t handle) {
    SystemGesturePrimitiveTouchEvent events[4];
    std::memset(events, 0xA5, sizeof(events));
    SystemGesturePrimitiveTouchEvent untouched[4];
    std::memcpy(untouched, events, sizeof(events));
    std::uint32_t count = Untouched;
    Require(sceSystemGestureGetPrimitiveTouchEventsCount(handle) == 0, "primitive events reported");
    Require(sceSystemGestureGetPrimitiveTouchEventsCount(handle + 1) == InvalidHandle, "primitive count accepted a bad handle");
    Require(sceSystemGestureGetPrimitiveTouchEvents(handle + 1, events, 4, &count) == InvalidHandle, "primitive events accepted a bad handle");
    Require(count == Untouched, "failed primitive query wrote a count");
    Require(sceSystemGestureGetPrimitiveTouchEvents(handle, events, 4, nullptr) == InvalidArgument, "null primitive count accepted");
    Require(sceSystemGestureGetPrimitiveTouchEvents(handle, events, 4, &count) == 0, "primitive query failed");
    Require(count == 0, "primitive events returned");
    Require(std::memcmp(events, untouched, sizeof(events)) == 0, "primitive buffer written");
    Require(sceSystemGestureGetPrimitiveTouchEventByIndex(handle, 0, events) == EventDataNotFound, "primitive event found by index");
    Require(sceSystemGestureGetPrimitiveTouchEventByIndex(handle + 1, 0, events) == InvalidHandle, "primitive index accepted a bad handle");
    Require(sceSystemGestureGetPrimitiveTouchEventByPrimitiveID(handle, 0, events) == EventDataNotFound, "primitive event found by id");
    Require(sceSystemGestureGetPrimitiveTouchEventByPrimitiveID(handle + 1, 0, events) == InvalidHandle, "primitive id accepted a bad handle");
    Require(std::memcmp(events, untouched, sizeof(events)) == 0, "failed look-up wrote an event");
    Require(sceSystemGestureResetPrimitiveTouchRecognizer(handle) == 0, "primitive reset failed");
    Require(sceSystemGestureResetPrimitiveTouchRecognizer(handle + 1) == InvalidHandle, "primitive reset accepted a bad handle");
}

void CheckRecognizerEvents(int32_t handle, SystemGestureTouchRecognizer* recognizer) {
    SystemGestureTouchEvent events[2];
    std::memset(events, 0xA5, sizeof(events));
    SystemGestureTouchEvent untouched[2];
    std::memcpy(untouched, events, sizeof(events));
    std::uint32_t count = Untouched;
    Require(sceSystemGestureGetTouchEventsCount(handle, recognizer) == 0, "gesture events counted");
    Require(sceSystemGestureGetTouchEventsCount(handle + 1, recognizer) == InvalidHandle, "count accepted a bad handle");
    Require(sceSystemGestureGetTouchEventsCount(handle, nullptr) == InvalidArgument, "count accepted a null recognizer");
    Require(sceSystemGestureGetTouchEvents(handle + 1, recognizer, events, 2, &count) == InvalidHandle, "events accepted a bad handle");
    Require(sceSystemGestureGetTouchEvents(handle, nullptr, events, 2, &count) == InvalidArgument, "events accepted a null recognizer");
    Require(sceSystemGestureGetTouchEvents(handle, recognizer, events, 2, nullptr) == InvalidArgument, "events accepted a null count");
    Require(count == Untouched, "failed query wrote a count");
    Require(sceSystemGestureGetTouchEvents(handle, recognizer, events, 2, &count) == 0, "event query failed");
    Require(count == 0, "gesture events returned");
    Require(sceSystemGestureGetTouchEventByIndex(handle, recognizer, 0, events) == EventDataNotFound, "event found by index");
    Require(sceSystemGestureGetTouchEventByIndex(handle + 1, recognizer, 0, events) == InvalidHandle, "index accepted a bad handle");
    Require(sceSystemGestureGetTouchEventByIndex(handle, nullptr, 0, events) == InvalidArgument, "index accepted a null recognizer");
    Require(sceSystemGestureGetTouchEventByEventID(handle, recognizer, 0, events) == EventDataNotFound, "event found by id");
    Require(sceSystemGestureGetTouchEventByEventID(handle + 1, recognizer, 0, events) == InvalidHandle, "id accepted a bad handle");
    Require(sceSystemGestureGetTouchEventByEventID(handle, nullptr, 0, events) == InvalidArgument, "id accepted a null recognizer");
    Require(std::memcmp(events, untouched, sizeof(events)) == 0, "event buffer written");
}

}

int main() {
    Require(sceSystemGestureInitializePrimitiveTouchRecognizer(nullptr) == 0, "initialization failed");
    const int32_t handle = sceSystemGestureOpen(0, nullptr);
    Require(handle > 0, "open failed");

    static SystemGestureTouchRecognizer recognizer;
    const SystemGestureRectangle rectangle{0.0f, 0.0f, 1920.0f, 943.0f, {}};
    const SystemGestureRectangle moved{960.0f, 0.0f, 960.0f, 943.0f, {}};
    Require(sceSystemGestureCreateTouchRecognizer(handle, &recognizer, 1, &rectangle, nullptr) == 0, "create failed");
    Require(sceSystemGestureAppendTouchRecognizer(handle, &recognizer) == 0, "append failed");
    Require(sceSystemGestureAppendTouchRecognizer(handle + 1, &recognizer) == InvalidHandle, "append accepted a bad handle");
    Require(sceSystemGestureAppendTouchRecognizer(handle, nullptr) == InvalidArgument, "append accepted a null recognizer");
    Require(sceSystemGestureUpdateTouchRecognizerRectangle(handle, &recognizer, &moved) == 0, "rectangle update failed");
    Require(sceSystemGestureUpdateTouchRecognizerRectangle(handle + 1, &recognizer, &moved) == InvalidHandle, "rectangle accepted a bad handle");
    Require(sceSystemGestureUpdateTouchRecognizerRectangle(handle, nullptr, &moved) == InvalidArgument, "rectangle accepted a null recognizer");
    Require(sceSystemGestureUpdateTouchRecognizerRectangle(handle, &recognizer, nullptr) == InvalidArgument, "null rectangle accepted");

    Require(sceSystemGestureUpdatePrimitiveTouchRecognizer(handle, nullptr) == 0, "primitive update failed");
    Require(sceSystemGestureUpdateTouchRecognizer(handle, &recognizer) == 0, "update failed");
    Require(sceSystemGestureUpdateTouchRecognizer(handle + 1, &recognizer) == InvalidHandle, "update accepted a bad handle");
    Require(sceSystemGestureUpdateTouchRecognizer(handle, nullptr) == InvalidArgument, "update accepted a null recognizer");
    Require(sceSystemGestureUpdateAllTouchRecognizer(handle) == 0, "update all failed");
    Require(sceSystemGestureUpdateAllTouchRecognizer(handle + 1) == InvalidHandle, "update all accepted a bad handle");
    CheckPrimitiveEvents(handle);
    CheckRecognizerEvents(handle, &recognizer);

    Require(sceSystemGestureResetTouchRecognizer(handle, &recognizer) == 0, "reset failed");
    Require(sceSystemGestureResetTouchRecognizer(handle + 1, &recognizer) == InvalidHandle, "reset accepted a bad handle");
    Require(sceSystemGestureResetTouchRecognizer(handle, nullptr) == InvalidArgument, "reset accepted a null recognizer");
    Require(sceSystemGestureRemoveTouchRecognizer(handle, &recognizer) == 0, "remove failed");
    Require(sceSystemGestureRemoveTouchRecognizer(handle + 1, &recognizer) == InvalidHandle, "remove accepted a bad handle");
    Require(sceSystemGestureRemoveTouchRecognizer(handle, nullptr) == InvalidArgument, "remove accepted a null recognizer");
    Require(sceSystemGestureUpdateAllTouchRecognizer(handle) == 0, "update all after remove failed");
    Require(sceSystemGestureClose(handle) == 0, "close failed");
    Require(sceSystemGestureFinalizePrimitiveTouchRecognizer() == 0, "finalization failed");
    std::puts("SystemGesture tests passed");
    return 0;
}
