#include <algorithm>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <mutex>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>
#include "SceTypes.hpp"
#include "prx/libc/include/General.hpp"

namespace {

constexpr int ErrorBusy = static_cast<int>(0x80bc0001u);
constexpr int ErrorNotOpened = static_cast<int>(0x80bc0002u);
constexpr int ErrorConnectionFailed = static_cast<int>(0x80bc0004u);
constexpr int ErrorInvalidText = static_cast<int>(0x80bc0006u);
constexpr int ErrorInvalidUserId = static_cast<int>(0x80bc0010u);
constexpr int ErrorInvalidType = static_cast<int>(0x80bc0011u);
constexpr int ErrorInvalidSupportedLanguages = static_cast<int>(0x80bc0012u);
constexpr int ErrorInvalidEnterLabel = static_cast<int>(0x80bc0013u);
constexpr int ErrorInvalidInputMethod = static_cast<int>(0x80bc0014u);
constexpr int ErrorInvalidOption = static_cast<int>(0x80bc0015u);
constexpr int ErrorInvalidMaxTextLength = static_cast<int>(0x80bc0016u);
constexpr int ErrorInvalidInputTextBuffer = static_cast<int>(0x80bc0017u);
constexpr int ErrorInvalidPosX = static_cast<int>(0x80bc0018u);
constexpr int ErrorInvalidPosY = static_cast<int>(0x80bc0019u);
constexpr int ErrorInvalidHorizontalAlignment = static_cast<int>(0x80bc001au);
constexpr int ErrorInvalidVerticalAlignment = static_cast<int>(0x80bc001bu);
constexpr int ErrorInvalidExtended = static_cast<int>(0x80bc001cu);
constexpr int ErrorInvalidWork = static_cast<int>(0x80bc0020u);
constexpr int ErrorInvalidHandler = static_cast<int>(0x80bc0022u);
constexpr int ErrorInvalidParam = static_cast<int>(0x80bc0030u);
constexpr int ErrorInvalidAddress = static_cast<int>(0x80bc0031u);
constexpr int ErrorInvalidReserved = static_cast<int>(0x80bc0032u);
constexpr int ErrorNoResourceId = static_cast<int>(0x80bc0023u);
constexpr int ErrorInvalidMode = static_cast<int>(0x80bc0024u);
constexpr uint32_t ValidKeyboardModes = 0x0000007f;
constexpr uint32_t TypeDefault = 0;
constexpr uint32_t TypeBasicLatin = 1;
constexpr uint32_t TypeNumber = 4;
constexpr uint32_t ValidOptions = 0x00007bff;
constexpr uint32_t OptionMultiline = 0x00000001;
constexpr uint32_t OptionPassword = 0x00000004;
constexpr uint32_t OptionExpandedPreedit = 0x00001000;
constexpr uint32_t OptionUseOver2K = 0x00004000;
constexpr uint64_t ValidLanguages = 0x00000001ff1fffffull;
constexpr uint32_t EnterLabelGo = 3;
constexpr uint32_t InputMethodDefault = 0;
constexpr uint32_t AlignmentEnd = 2;
constexpr uint32_t MaxTextLength = 2048;
constexpr uint32_t ValidExtendedOptions = 0x00004fde;
constexpr uint32_t ExtendedOptionOver2K = 0x00004000;
constexpr uint32_t ExtendedOptionRequiredWithOver2K = 0x00000080;
constexpr uint32_t PanelPriorityAccent = 3;
constexpr uint32_t ValidDisableDevices = 7;
constexpr uint32_t ValidExtKeyboardModes = 0x1c000003;
constexpr uint32_t PreeditScratchCharacters = 0x1f;
constexpr uint32_t ExpandedPreeditScratchCharacters = 0x79;
constexpr std::size_t WorkBufferSize = 20 * 1024;
constexpr uint32_t EventOpen = 0;
constexpr uint32_t EventPressClose = 4;
constexpr int32_t UserIdInvalid = -1;
constexpr int32_t UserIdSystem = 0xff;

using GuestEventHandler = void (APS5_VABI*)(void* arg, const ImeEvent* event);

struct Panel {
 bool open = false;
 std::uint64_t generation = 0;
 Param param{};
 uint32_t textLength = 0;
 std::vector<ImeEvent> events;
};

std::mutex g_keyboardMutex;
std::set<int32_t> g_openKeyboards;
std::mutex g_panelMutex;
Panel g_panel;

bool IsZero(const std::int8_t* data, std::size_t size) {
 return std::all_of(data, data + size, [](std::int8_t value) { return value == 0; });
}

bool ValidateText(const char16_t* text, uint32_t limit, bool multiline, uint32_t& length) {
 length = 0;
 for (uint32_t i = 0; i < limit; ++i) {
  const char16_t value = text[i];
  if (value == u'\0') return true;
  if (!multiline && (value == u'\n' || value == u'\r')) return false;
  if (value >= 0xd800 && value <= 0xdbff) {
   if (++i >= limit || text[i] < 0xdc00 || text[i] > 0xdfff) return false;
  } else if (value >= 0xdc00 && value <= 0xdfff) {
   return false;
  }
  length = i + 1;
 }
 return true;
}

bool RangesOverlap(std::uintptr_t first, std::size_t firstSize, std::uintptr_t second, std::size_t secondSize) {
 if (first > UINTPTR_MAX - firstSize || second > UINTPTR_MAX - secondSize) return true;
 return first < second + secondSize && second < first + firstSize;
}

int ValidateExtendedParam(const ExtendedParam* extended) {
 if (!extended) return 0;
 if ((extended->option & ~ValidExtendedOptions) != 0) return ErrorInvalidExtended;
 if ((extended->option & ExtendedOptionOver2K) != 0 && (extended->option & ExtendedOptionRequiredWithOver2K) == 0) return ErrorInvalidExtended;
 if (extended->priority > PanelPriorityAccent || extended->disable_device > ValidDisableDevices) return ErrorInvalidExtended;
 if ((extended->ext_keyboard_mode & ~ValidExtKeyboardModes) != 0) return ErrorInvalidExtended;
 if (!IsZero(extended->reserved, sizeof(extended->reserved))) return ErrorInvalidExtended;
 return 0;
}

int ValidateOpenParam(const Param* param, const ExtendedParam* extended, uint32_t& textLength) {
 if (!param) return ErrorInvalidAddress;
 if (param->user_id < 0 || param->user_id == UserIdSystem) return ErrorInvalidUserId;
 if (param->type > TypeNumber) return ErrorInvalidType;
 if ((param->supported_languages & ~ValidLanguages) != 0) return ErrorInvalidSupportedLanguages;
 if (param->enter_label > EnterLabelGo) return ErrorInvalidEnterLabel;
 if (param->input_method != InputMethodDefault) return ErrorInvalidInputMethod;
 if ((param->option & ~ValidOptions) != 0) return ErrorInvalidOption;
 const bool multiline = (param->option & OptionMultiline) != 0;
 const bool password = (param->option & OptionPassword) != 0;
 if (multiline && password) return ErrorInvalidParam;
 if (multiline && param->type != TypeDefault && param->type != TypeBasicLatin) return ErrorInvalidParam;
 if (password && param->type != TypeBasicLatin && param->type != TypeNumber) return ErrorInvalidParam;
 if (param->max_text_length == 0 || param->max_text_length > MaxTextLength) return ErrorInvalidMaxTextLength;
 if (!param->input_text_buffer) return ErrorInvalidInputTextBuffer;
 const bool over2K = (param->option & OptionUseOver2K) != 0;
 const float maxX = over2K ? 3840.0f : 1920.0f;
 const float maxY = over2K ? 2160.0f : 1080.0f;
 if (!(param->posx >= 0.0f && param->posx < maxX)) return ErrorInvalidPosX;
 if (!(param->posy >= 0.0f && param->posy < maxY)) return ErrorInvalidPosY;
 if (param->horizontal_alignment > AlignmentEnd) return ErrorInvalidHorizontalAlignment;
 if (param->vertical_alignment > AlignmentEnd) return ErrorInvalidVerticalAlignment;
 if (const int result = ValidateExtendedParam(extended); result != 0) return result;
 if (!param->work || (reinterpret_cast<std::uintptr_t>(param->work) & 3u) != 0) return ErrorInvalidWork;
 if (!param->handler) return ErrorInvalidHandler;
 if (!IsZero(param->reserved, sizeof(param->reserved))) return ErrorInvalidReserved;
 const uint32_t scratch = (param->option & OptionExpandedPreedit) != 0 ? ExpandedPreeditScratchCharacters : PreeditScratchCharacters;
 const std::size_t textBytes = static_cast<std::size_t>(param->max_text_length + scratch) * sizeof(char16_t);
 if (RangesOverlap(reinterpret_cast<std::uintptr_t>(param->input_text_buffer), textBytes, reinterpret_cast<std::uintptr_t>(param->work), WorkBufferSize)) return ErrorInvalidParam;
 uint32_t terminator = 0;
 while (terminator <= param->max_text_length && param->input_text_buffer[terminator] != u'\0') ++terminator;
 if (terminator > param->max_text_length) return ErrorInvalidText;
 if (!ValidateText(param->input_text_buffer, terminator, multiline, textLength)) return ErrorInvalidText;
 return 0;
}

}

extern "C" {

int APS5_VABI sceImeClose_nid_postfix(void) {
 std::lock_guard lock(g_panelMutex);
 if (!g_panel.open) return ErrorNotOpened;
 g_panel.open = false;
 g_panel.events.clear();
 return 0;
}

int APS5_VABI sceImeGetPanelSize(const Param* param, uint32_t* width, uint32_t* height) {
 if (!param || !width || !height) return ErrorInvalidAddress;
 if (param->type > TypeNumber) return ErrorInvalidType;
 if ((param->option & ~ValidOptions) != 0) return ErrorInvalidOption;
 const uint32_t scale = (param->option & OptionUseOver2K) != 0 ? 2 : 1;
 *width = (param->type == TypeNumber ? 370u : 793u) * scale;
 *height = (param->type == TypeNumber ? 402u : 408u) * scale;
 return 0;
}

int APS5_VABI sceImeKeyboardClose(int32_t user_id) {
 std::lock_guard lock(g_keyboardMutex);
 if (g_openKeyboards.erase(user_id) == 0) throw std::logic_error("sceImeKeyboardClose: keyboard not open for user " + std::to_string(user_id));
 return 0;
}

int APS5_VABI sceImeKeyboardGetInfo(uint32_t resource_id, KeyboardInfo* info) {
 (void)resource_id;
 if (!info) return ErrorInvalidAddress;
 std::lock_guard lock(g_keyboardMutex);
 return g_openKeyboards.empty() ? ErrorNotOpened : ErrorNoResourceId;
}

int APS5_VABI sceImeKeyboardGetResourceId(int32_t user_id, KeyboardResourceIdArray* resource_ids) {
 if (!resource_ids) return ErrorInvalidAddress;
 if (user_id == UserIdInvalid) return ErrorInvalidUserId;
 *resource_ids = {};
 resource_ids->user_id = user_id;
 std::lock_guard lock(g_keyboardMutex);
 return g_openKeyboards.contains(user_id) ? ErrorConnectionFailed : ErrorNotOpened;
}

int APS5_VABI sceImeKeyboardOpen(int32_t user_id, const KeyboardParam* param) {
 if (!param) APS5_INVALID_ARG_EX;
 std::lock_guard lock(g_keyboardMutex);
 if (!g_openKeyboards.insert(user_id).second) throw std::logic_error("sceImeKeyboardOpen: keyboard already open for user " + std::to_string(user_id));
 return 0;
}

int APS5_VABI sceImeKeyboardSetMode(int32_t user_id, uint32_t mode) {
 if (user_id == UserIdInvalid) return ErrorInvalidUserId;
 std::lock_guard lock(g_keyboardMutex);
 if (!g_openKeyboards.contains(user_id)) return ErrorNotOpened;
 return (mode & ~ValidKeyboardModes) == 0 ? 0 : ErrorInvalidMode;
}

int APS5_VABI sceImeOpen_nid_postfix(const Param* param, const ExtendedParam* extended) {
 std::lock_guard lock(g_panelMutex);
 if (g_panel.open) return ErrorBusy;
 uint32_t textLength = 0;
 if (const int result = ValidateOpenParam(param, extended, textLength); result != 0) return result;
 uint32_t width = 0;
 uint32_t height = 0;
 sceImeGetPanelSize(param, &width, &height);
 std::memset(param->work, 0, WorkBufferSize);
 ImeEvent opened;
 std::memset(&opened, 0, sizeof(opened));
 opened.id = EventOpen;
 opened.param.rect.x = param->posx;
 opened.param.rect.y = param->posy;
 opened.param.rect.width = width;
 opened.param.rect.height = height;
 ImeEvent closePressed;
 std::memset(&closePressed, 0, sizeof(closePressed));
 closePressed.id = EventPressClose;
 g_panel.open = true;
 ++g_panel.generation;
 g_panel.param = *param;
 g_panel.textLength = textLength;
 g_panel.events = {opened, closePressed};
 return 0;
}

void APS5_VABI sceImeParamInit(Param* param) {
 if (!param) return;
 std::memset(param, 0, sizeof(*param));
 param->user_id = -1;
}

int APS5_VABI sceImeSetCaret(const Caret* caret) {
 std::lock_guard lock(g_panelMutex);
 if (!g_panel.open) return ErrorNotOpened;
 if (!caret) return ErrorInvalidAddress;
 if (caret->index > g_panel.textLength) return ErrorInvalidParam;
 return 0;
}

int APS5_VABI sceImeSetText(const char16_t* text, uint32_t length) {
 std::lock_guard lock(g_panelMutex);
 if (!g_panel.open) return ErrorNotOpened;
 if (!text) return ErrorInvalidAddress;
 const bool multiline = (g_panel.param.option & OptionMultiline) != 0;
 uint32_t textLength = 0;
 if (!ValidateText(text, std::min(length, g_panel.param.max_text_length), multiline, textLength)) return ErrorInvalidText;
 std::memmove(g_panel.param.input_text_buffer, text, textLength * sizeof(char16_t));
 g_panel.param.input_text_buffer[textLength] = u'\0';
 g_panel.textLength = textLength;
 return 0;
}

int APS5_VABI sceImeSetTextGeometry(TextAreaMode mode, const TextGeometry* geometry) {
 std::lock_guard lock(g_panelMutex);
 if (!g_panel.open) return ErrorNotOpened;
 if (!geometry) return ErrorInvalidAddress;
 const bool over2K = (g_panel.param.option & OptionUseOver2K) != 0;
 const float maxX = over2K ? 3840.0f : 1920.0f;
 const float maxY = over2K ? 2160.0f : 1080.0f;
 if (!(geometry->x >= 0.0f && geometry->x < maxX && geometry->y >= 0.0f && geometry->y < maxY)) return ErrorInvalidParam;
 if (mode != TextAreaMode::Preedit && mode != TextAreaMode::Select) return ErrorInvalidParam;
 return 0;
}

int APS5_VABI sceImeUpdate(EventHandler handler) {
 if (!handler) APS5_INVALID_ARG_EX;
 std::vector<ImeEvent> events;
 void* arg = nullptr;
 std::uint64_t generation = 0;
 {
  std::lock_guard lock(g_panelMutex);
  if (!g_panel.open || g_panel.param.handler != handler) return 0;
  events.swap(g_panel.events);
  arg = g_panel.param.arg;
  generation = g_panel.generation;
 }
 const auto guestHandler = reinterpret_cast<GuestEventHandler>(handler);
 for (const ImeEvent& event : events) {
  {
   std::lock_guard lock(g_panelMutex);
   if (!g_panel.open || g_panel.generation != generation) break;
  }
  guestHandler(arg, &event);
 }
 return 0;
}

}
