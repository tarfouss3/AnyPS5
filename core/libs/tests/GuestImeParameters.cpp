#include "SceTypes.hpp"
#include <array>
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <vector>

extern "C" {
void APS5_VABI sceImeParamInit(Param* param);
int APS5_VABI sceImeGetPanelSize(const Param* param, uint32_t* width, uint32_t* height);
int APS5_VABI sceImeClose_nid_postfix(void);
int APS5_VABI sceImeOpen_nid_postfix(const Param* param, const ExtendedParam* extended);
int APS5_VABI sceImeUpdate(EventHandler handler);
int APS5_VABI sceImeSetCaret(const Caret* caret);
int APS5_VABI sceImeSetText(const char16_t* text, uint32_t length);
int APS5_VABI sceImeSetTextGeometry(TextAreaMode mode, const TextGeometry* geometry);
int APS5_VABI sceImeKeyboardOpen(int32_t user_id, const KeyboardParam* param);
int APS5_VABI sceImeKeyboardClose(int32_t user_id);
int APS5_VABI sceImeKeyboardGetResourceId(int32_t user_id, KeyboardResourceIdArray* resource_ids);
}

static_assert(sizeof(Param) == 96);
static_assert(offsetof(Param, option) == 32);
static_assert(offsetof(Param, reserved) == 88);

static void Require(bool value, const char* message) {
    if (value) return;
    std::fprintf(stderr, "%s\n", message);
    std::abort();
}

static void CheckInitialization() {
    struct GuardedParam {
        std::array<unsigned char, 8> before;
        Param param;
        std::array<unsigned char, 8> after;
    } guarded;
    std::memset(&guarded, 0xa5, sizeof(guarded));
    sceImeParamInit(nullptr);
    sceImeParamInit(&guarded.param);
    Require(guarded.param.user_id == -1, "initial user must be invalid");
    const auto* bytes = reinterpret_cast<const unsigned char*>(&guarded.param);
    for (size_t i = sizeof(guarded.param.user_id); i < sizeof(Param); ++i) {
        Require(bytes[i] == 0, "all remaining parameter bytes must be cleared");
    }
    for (unsigned char value : guarded.before) Require(value == 0xa5, "parameter underrun");
    for (unsigned char value : guarded.after) Require(value == 0xa5, "parameter overrun");
    guarded.param.type = 4;
    guarded.param.option = 0x4000;
    guarded.param.max_text_length = 32;
    sceImeParamInit(&guarded.param);
    Require(guarded.param.user_id == -1 && guarded.param.type == 0 &&
            guarded.param.option == 0 && guarded.param.max_text_length == 0,
            "reinitialization must reset previous values");
}

static void CheckPanelSizes() {
    Param param;
    std::memset(&param, 0xa5, sizeof(param));
    for (uint32_t type = 0; type <= 4; ++type) {
        for (uint32_t options : {0u, 0x4000u, 0x7bffu}) {
            param.type = type;
            param.option = options;
            const Param original = param;
            struct GuardedSize { uint32_t before; uint32_t value; uint32_t after; };
            GuardedSize width{0x12345678, 0, 0x87654321};
            GuardedSize height = width;
            Require(sceImeGetPanelSize(&param, &width.value, &height.value) == 0,
                    "valid panel query failed");
            const bool scaled = (options & 0x4000) != 0;
            Require(width.value == (type == 4 ? (scaled ? 740u : 370u) : (scaled ? 1586u : 793u)),
                    "incorrect panel width");
            Require(height.value == (type == 4 ? (scaled ? 804u : 402u) : (scaled ? 816u : 408u)),
                    "incorrect panel height");
            Require(width.before == 0x12345678 && width.after == 0x87654321 &&
                    height.before == 0x12345678 && height.after == 0x87654321,
                    "panel query wrote outside its outputs");
            Require(std::memcmp(&param, &original, sizeof(param)) == 0,
                    "panel query modified parameters");
        }
    }
}

static void CheckErrors() {
    constexpr int InvalidAddress = static_cast<int>(0x80bc0031u);
    constexpr int InvalidType = static_cast<int>(0x80bc0011u);
    constexpr int InvalidOption = static_cast<int>(0x80bc0015u);
    Param param{};
    uint32_t width = 0x12345678;
    uint32_t height = 0x87654321;
    Require(sceImeGetPanelSize(nullptr, &width, &height) == InvalidAddress, "null parameter");
    Require(sceImeGetPanelSize(&param, nullptr, &height) == InvalidAddress, "null width");
    Require(sceImeGetPanelSize(&param, &width, nullptr) == InvalidAddress, "null height");
    for (uint32_t type : {5u, 0xffffffffu}) {
        param.type = type;
        Require(sceImeGetPanelSize(&param, &width, &height) == InvalidType, "invalid type");
    }
    param.type = 0;
    for (uint32_t bit = 0; bit < 32; ++bit) {
        param.option = 1u << bit;
        const bool valid = (param.option & 0x7bffu) != 0;
        uint32_t queriedWidth = width;
        uint32_t queriedHeight = height;
        Require(sceImeGetPanelSize(&param, &queriedWidth, &queriedHeight) == (valid ? 0 : InvalidOption),
                "option bit validation");
        if (!valid) Require(queriedWidth == width && queriedHeight == height, "outputs changed on invalid option");
    }
    param.type = 5;
    param.option = 0x80000000;
    Require(sceImeGetPanelSize(&param, &width, &height) == InvalidType, "type error precedence");
    Require(sceImeGetPanelSize(&param, nullptr, &height) == InvalidAddress, "address error precedence");
    Require(width == 0x12345678 && height == 0x87654321, "outputs changed on error");
}

static void CheckClosedPanel() {
    constexpr int notOpened = static_cast<int>(0x80bc0002u);
    Caret caret{};
    TextGeometry geometry{};
    const char16_t text[] = u"text";
    Require(sceImeSetCaret(&caret) == notOpened, "caret needs an open panel");
    Require(sceImeSetCaret(nullptr) == notOpened, "caret checks the panel first");
    Require(sceImeSetText(text, 4) == notOpened, "text needs an open panel");
    Require(sceImeSetTextGeometry(TextAreaMode::Edit, &geometry) == notOpened, "geometry needs an open panel");
    Require(sceImeClose_nid_postfix() == notOpened, "closing needs an open panel");
}

static void CheckKeyboardResourceIds() {
    constexpr int NotOpened = static_cast<int>(0x80bc0002u);
    constexpr int ConnectionFailed = static_cast<int>(0x80bc0004u);
    constexpr int InvalidUserId = static_cast<int>(0x80bc0010u);
    constexpr int InvalidAddress = static_cast<int>(0x80bc0031u);
    KeyboardResourceIdArray ids;
    std::memset(&ids, 0xa5, sizeof(ids));
    Require(sceImeKeyboardGetResourceId(1, nullptr) == InvalidAddress, "null resource id array");
    Require(sceImeKeyboardGetResourceId(-1, &ids) == InvalidUserId, "invalid user");
    const auto* bytes = reinterpret_cast<const unsigned char*>(&ids);
    for (size_t i = 0; i < sizeof(ids); ++i) Require(bytes[i] == 0xa5, "outputs changed on argument error");
    Require(sceImeKeyboardGetResourceId(1, &ids) == NotOpened, "keyboard not opened");
    Require(ids.user_id == 1, "user not reported for an unopened keyboard");
    for (uint32_t id : ids.resource_id) Require(id == 0, "resource id reported for an unopened keyboard");
    KeyboardParam param{};
    Require(sceImeKeyboardOpen(1, &param) == 0, "keyboard open failed");
    std::memset(&ids, 0xa5, sizeof(ids));
    Require(sceImeKeyboardGetResourceId(1, &ids) == ConnectionFailed, "a keyboard was reported as connected");
    Require(ids.user_id == 1, "user not reported");
    for (uint32_t id : ids.resource_id) Require(id == 0, "resource id reported without a keyboard");
    Require(sceImeKeyboardGetResourceId(2, &ids) == NotOpened, "keyboard of another user reported as opened");
    Require(ids.user_id == 2, "other user not reported");
    Require(sceImeKeyboardClose(1) == 0, "keyboard close failed");
    Require(sceImeKeyboardGetResourceId(1, &ids) == NotOpened, "closed keyboard reported as opened");
}

namespace {

constexpr int Busy = static_cast<int>(0x80bc0001u);
constexpr int NotOpened = static_cast<int>(0x80bc0002u);
constexpr int InvalidText = static_cast<int>(0x80bc0006u);
constexpr int InvalidUserId = static_cast<int>(0x80bc0010u);
constexpr int InvalidType = static_cast<int>(0x80bc0011u);
constexpr int InvalidSupportedLanguages = static_cast<int>(0x80bc0012u);
constexpr int InvalidEnterLabel = static_cast<int>(0x80bc0013u);
constexpr int InvalidInputMethod = static_cast<int>(0x80bc0014u);
constexpr int InvalidOption = static_cast<int>(0x80bc0015u);
constexpr int InvalidMaxTextLength = static_cast<int>(0x80bc0016u);
constexpr int InvalidInputTextBuffer = static_cast<int>(0x80bc0017u);
constexpr int InvalidPosX = static_cast<int>(0x80bc0018u);
constexpr int InvalidPosY = static_cast<int>(0x80bc0019u);
constexpr int InvalidHorizontalAlignment = static_cast<int>(0x80bc001au);
constexpr int InvalidVerticalAlignment = static_cast<int>(0x80bc001bu);
constexpr int InvalidExtended = static_cast<int>(0x80bc001cu);
constexpr int InvalidWork = static_cast<int>(0x80bc0020u);
constexpr int InvalidHandler = static_cast<int>(0x80bc0022u);
constexpr int InvalidParam = static_cast<int>(0x80bc0030u);
constexpr int InvalidAddress = static_cast<int>(0x80bc0031u);
constexpr int InvalidReserved = static_cast<int>(0x80bc0032u);
constexpr size_t WorkSize = 20 * 1024;

struct Delivered {
    void* arg;
    ImeEvent event;
};

std::vector<Delivered> g_delivered;
bool g_closeOnOpen = false;

void APS5_VABI RecordEvent(void* arg, const ImeEvent* event) {
    g_delivered.push_back({arg, *event});
    if (g_closeOnOpen && event->id == 0) Require(sceImeClose_nid_postfix() == 0, "close from the handler failed");
}

void APS5_VABI OtherHandler(void*, const ImeEvent*) {
    Require(false, "events delivered to a handler the panel was not opened with");
}

EventHandler AsHandler(void (APS5_VABI* handler)(void*, const ImeEvent*)) {
    return reinterpret_cast<EventHandler>(handler);
}

struct PanelBuffers {
    alignas(16) unsigned char work[WorkSize + 16];
    char16_t text[33];
};

Param MakePanelParam(PanelBuffers& buffers, void* arg) {
    Param param;
    sceImeParamInit(&param);
    param.user_id = 1;
    param.max_text_length = 32;
    param.input_text_buffer = buffers.text;
    param.posx = 100.0f;
    param.posy = 200.0f;
    param.work = buffers.work;
    param.arg = arg;
    param.handler = AsHandler(&RecordEvent);
    return param;
}

int OpenWith(const Param& param, const ExtendedParam* extended = nullptr) {
    return sceImeOpen_nid_postfix(&param, extended);
}

}

static void CheckPanelOpenErrors() {
    static PanelBuffers buffers;
    std::memcpy(buffers.text, u"abc", sizeof(u"abc"));
    int arg = 0;
    const Param valid = MakePanelParam(buffers, &arg);
    Require(sceImeOpen_nid_postfix(nullptr, nullptr) == InvalidAddress, "null open parameter");
    Param param = valid;
    param.user_id = -1;
    Require(OpenWith(param) == InvalidUserId, "invalid user");
    param.user_id = 0xff;
    Require(OpenWith(param) == InvalidUserId, "system user");
    param = valid;
    param.type = 5;
    Require(OpenWith(param) == InvalidType, "type");
    param = valid;
    param.supported_languages = 0x200000;
    Require(OpenWith(param) == InvalidSupportedLanguages, "languages");
    param = valid;
    param.enter_label = 4;
    Require(OpenWith(param) == InvalidEnterLabel, "enter label");
    param = valid;
    param.input_method = 1;
    Require(OpenWith(param) == InvalidInputMethod, "input method");
    param = valid;
    param.option = 0x400;
    Require(OpenWith(param) == InvalidOption, "option");
    param.option = 0x1 | 0x4;
    Require(OpenWith(param) == InvalidParam, "multiline password");
    param.option = 0x1;
    param.type = 2;
    Require(OpenWith(param) == InvalidParam, "multiline url");
    param.option = 0x4;
    param.type = 0;
    Require(OpenWith(param) == InvalidParam, "password default type");
    param = valid;
    param.max_text_length = 0;
    Require(OpenWith(param) == InvalidMaxTextLength, "zero text length");
    param.max_text_length = 2049;
    Require(OpenWith(param) == InvalidMaxTextLength, "text length");
    param = valid;
    param.input_text_buffer = nullptr;
    Require(OpenWith(param) == InvalidInputTextBuffer, "text buffer");
    param = valid;
    param.posx = 1920.0f;
    Require(OpenWith(param) == InvalidPosX, "posx");
    param.posx = std::nanf("");
    Require(OpenWith(param) == InvalidPosX, "posx nan");
    param = valid;
    param.posy = -1.0f;
    Require(OpenWith(param) == InvalidPosY, "posy");
    param = valid;
    param.horizontal_alignment = 3;
    Require(OpenWith(param) == InvalidHorizontalAlignment, "horizontal alignment");
    param = valid;
    param.vertical_alignment = 3;
    Require(OpenWith(param) == InvalidVerticalAlignment, "vertical alignment");
    ExtendedParam extended{};
    extended.option = 0x1;
    Require(OpenWith(valid, &extended) == InvalidExtended, "extended option");
    extended.option = 0x4000;
    Require(OpenWith(valid, &extended) == InvalidExtended, "extended option without 0x80");
    extended.option = 0;
    extended.priority = 4;
    Require(OpenWith(valid, &extended) == InvalidExtended, "extended priority");
    extended.priority = 0;
    extended.disable_device = 8;
    Require(OpenWith(valid, &extended) == InvalidExtended, "extended devices");
    extended.disable_device = 0;
    extended.ext_keyboard_mode = 0x4;
    Require(OpenWith(valid, &extended) == InvalidExtended, "extended keyboard mode");
    extended.ext_keyboard_mode = 0;
    extended.reserved[59] = 1;
    Require(OpenWith(valid, &extended) == InvalidExtended, "extended reserved");
    param = valid;
    param.work = nullptr;
    Require(OpenWith(param) == InvalidWork, "null work");
    param.work = buffers.work + 2;
    Require(OpenWith(param) == InvalidWork, "misaligned work");
    param = valid;
    param.handler = nullptr;
    Require(OpenWith(param) == InvalidHandler, "handler");
    param = valid;
    param.reserved[7] = 1;
    Require(OpenWith(param) == InvalidReserved, "reserved");
    param = valid;
    param.input_text_buffer = reinterpret_cast<char16_t*>(buffers.work + WorkSize - 2);
    param.input_text_buffer[0] = u'\0';
    Require(OpenWith(param) == InvalidParam, "text buffer overlaps work");
    param = valid;
    for (char16_t& c : buffers.text) c = u'a';
    Require(OpenWith(param) == InvalidText, "unterminated text");
    std::memcpy(buffers.text, u"a\nb", sizeof(u"a\nb"));
    Require(OpenWith(param) == InvalidText, "newline in a single-line text");
    buffers.text[0] = static_cast<char16_t>(0xdc00);
    Require(OpenWith(param) == InvalidText, "lone low surrogate");
    Require(sceImeClose_nid_postfix() == NotOpened, "a rejected open left the panel open");
}

static void CheckPanelLifecycle() {
    static PanelBuffers buffers;
    std::memset(buffers.work, 0xa5, sizeof(buffers.work));
    std::memcpy(buffers.text, u"abc", sizeof(u"abc"));
    int arg = 0;
    const Param param = MakePanelParam(buffers, &arg);
    g_delivered.clear();
    Require(OpenWith(param) == 0, "valid open failed");
    for (size_t i = 0; i < WorkSize; ++i) Require(buffers.work[i] == 0, "work buffer not cleared");
    for (size_t i = WorkSize; i < sizeof(buffers.work); ++i) Require(buffers.work[i] == 0xa5, "write past the work buffer");
    Require(OpenWith(param) == Busy, "second open accepted");
    Require(sceImeUpdate(AsHandler(&OtherHandler)) == 0, "update with another handler");
    Require(g_delivered.empty(), "events delivered before the update");
    Require(sceImeUpdate(param.handler) == 0, "update failed");
    Require(g_delivered.size() == 2, "open and close events expected");
    Require(g_delivered[0].arg == &arg && g_delivered[1].arg == &arg, "handler argument");
    Require(g_delivered[0].event.id == 0, "first event is not the open event");
    Require(g_delivered[0].event.param.rect.x == 100.0f && g_delivered[0].event.param.rect.y == 200.0f, "open event position");
    Require(g_delivered[0].event.param.rect.width == 793 && g_delivered[0].event.param.rect.height == 408, "open event size");
    Require(g_delivered[1].event.id == 4, "second event is not the close button");
    for (uint64_t value : g_delivered[1].event.param.reserved) Require(value == 0, "close event carries a parameter");
    Require(sceImeUpdate(param.handler) == 0 && g_delivered.size() == 2, "events delivered twice");

    Caret caret{};
    Require(sceImeSetCaret(nullptr) == InvalidAddress, "null caret");
    caret.index = 3;
    Require(sceImeSetCaret(&caret) == 0, "caret at the end of the text");
    caret.index = 4;
    Require(sceImeSetCaret(&caret) == InvalidParam, "caret past the text");

    Require(sceImeSetText(nullptr, 1) == InvalidAddress, "null text");
    Require(sceImeSetText(u"a\nb", 3) == InvalidText, "newline text");
    Require(std::memcmp(buffers.text, u"abc", sizeof(u"abc")) == 0, "rejected text written");
    Require(sceImeSetText(u"hello", 5) == 0, "set text");
    Require(std::memcmp(buffers.text, u"hello", sizeof(u"hello")) == 0, "text not in the input buffer");
    caret.index = 5;
    Require(sceImeSetCaret(&caret) == 0, "caret at the end of the new text");
    Require(sceImeSetText(u"hello", 2) == 0, "set text prefix");
    Require(std::memcmp(buffers.text, u"he", sizeof(u"he")) == 0, "text length not applied");
    Require(sceImeSetCaret(&caret) == InvalidParam, "caret past the shorter text");
    char16_t longText[40];
    for (char16_t& c : longText) c = u'x';
    Require(sceImeSetText(longText, 40) == 0, "long text");
    for (int i = 0; i < 32; ++i) Require(buffers.text[i] == u'x', "long text not copied");
    Require(buffers.text[32] == u'\0', "long text not truncated");
    Require(sceImeSetText(u"", 0) == 0 && buffers.text[0] == u'\0', "empty text");

    TextGeometry geometry{10.0f, 10.0f, 100, 20};
    Require(sceImeSetTextGeometry(TextAreaMode::Preedit, nullptr) == InvalidAddress, "null geometry");
    Require(sceImeSetTextGeometry(TextAreaMode::Edit, &geometry) == InvalidParam, "edit area geometry");
    Require(sceImeSetTextGeometry(TextAreaMode::Preedit, &geometry) == 0, "preedit geometry");
    Require(sceImeSetTextGeometry(TextAreaMode::Select, &geometry) == 0, "select geometry");
    geometry.x = 1920.0f;
    Require(sceImeSetTextGeometry(TextAreaMode::Select, &geometry) == InvalidParam, "geometry outside the screen");

    Require(sceImeClose_nid_postfix() == 0, "close failed");
    Require(sceImeClose_nid_postfix() == NotOpened, "second close accepted");
    Require(sceImeSetText(u"a", 1) == NotOpened, "text after close");
    Require(sceImeUpdate(param.handler) == 0 && g_delivered.size() == 2, "events after close");
}

static void CheckPanelClosedFromHandler() {
    static PanelBuffers buffers;
    std::memcpy(buffers.text, u"", sizeof(u""));
    int arg = 0;
    Param param = MakePanelParam(buffers, &arg);
    param.type = 4;
    param.option = 0x4000;
    param.posx = 3000.0f;
    ExtendedParam extended{};
    extended.option = 0x4080;
    g_delivered.clear();
    Require(OpenWith(param, &extended) == 0, "open with extended parameters failed");
    g_closeOnOpen = true;
    Require(sceImeUpdate(param.handler) == 0, "update failed");
    g_closeOnOpen = false;
    Require(g_delivered.size() == 1, "events delivered after the handler closed the panel");
    Require(g_delivered[0].event.param.rect.x == 3000.0f, "high resolution position");
    Require(g_delivered[0].event.param.rect.width == 740 && g_delivered[0].event.param.rect.height == 804, "number panel size");
    Require(sceImeClose_nid_postfix() == NotOpened, "panel still open");
}

int main() {
    CheckInitialization();
    CheckPanelSizes();
    CheckErrors();
    CheckClosedPanel();
    CheckKeyboardResourceIds();
    CheckPanelOpenErrors();
    CheckPanelLifecycle();
    CheckPanelClosedFromHandler();
}
