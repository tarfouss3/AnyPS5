#include "prx/libc/include/general/VabiMacros.hpp"
#include "SceTypes.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>

extern "C" int APS5_VABI sscanf_s_nid_postfix(const char*, const char*, ...);
extern "C" int APS5_VABI vsscanf_s_nid_postfix(const char*, const char*, VaList*);

static int failures = 0;

static void Require(bool condition, const char* message) {
    if (!condition) {
        std::fprintf(stderr, "%s\n", message);
        ++failures;
    }
}

template<class TValue> static void CheckCount(const char* modifier, int count) {
    struct alignas(8) Destination {
        TValue value;
        std::array<unsigned char, 8> guard;
    } destination{static_cast<TValue>(-1), {}};
    destination.guard.fill(0xa5);
    const auto guard = destination.guard;
    const std::string input(static_cast<std::size_t>(count), 'x');
    const std::string format = input + "%" + modifier + "n";
    const int assigned = sscanf_s_nid_postfix(input.c_str(), format.c_str(), &destination.value);
    if (destination.value != count || destination.guard != guard || assigned != 0) {
        std::fprintf(stderr, "%%%sn after %d bytes: count=%lld, guard=%s, assignments=%d\n",
            modifier, count, static_cast<long long>(destination.value),
            destination.guard == guard ? "unchanged" : "overwritten", assigned);
        ++failures;
    }
}

static int APS5_VABI ScanList(const char* input, const char* format, ...) {
    __builtin_sysv_va_list args;
    __builtin_sysv_va_start(args, format);
    VaList list;
    std::memcpy(&list, args, sizeof(list));
    const VaList original = list;
    const int result = vsscanf_s_nid_postfix(input, format, &list);
    Require(std::memcmp(&list, &original, sizeof(list)) == 0, "vsscanf_s must not advance the caller's argument list");
    __builtin_sysv_va_end(args);
    return result;
}

static void CheckArgumentList() {
    int number = -1;
    long long wide = -1;
    double real = 0.0;
    char word[4] = {};
    char letters[3] = {'?', '?', '?'};
    std::int64_t count = -1;
    Require(ScanList(" 12 abc 7 2.5 xy!", "%d %3s %lld %lf %2c%jn!", &number, word, 4u, &wide, &real, letters, 2u, &count) == 5,
        "vsscanf_s must assign every converted item");
    Require(number == 12 && std::strcmp(word, "abc") == 0 && wide == 7 && real == 2.5 && letters[0] == 'x' && letters[1] == 'y'
        && letters[2] == '?' && count == 16, "vsscanf_s must store converted values and counts");
    std::memcpy(word, "zzz", 4);
    Require(ScanList("abcd", "%s", word, 4u) == 0 && word[0] == 0, "vsscanf_s must reject a string longer than its buffer");
    Require(ScanList("ab", "%3c", word, 2u) == 0, "vsscanf_s must reject a character run longer than its buffer");
    std::memcpy(word, "zzz", 4);
    Require(ScanList("abc", "%s", word, 0u) == 0 && std::strcmp(word, "zzz") == 0, "vsscanf_s must not write a zero-sized buffer");
    Require(ScanList(nullptr, "%d", &number) == EOF, "vsscanf_s must reject a null input");
    Require(ScanList("1", nullptr) == EOF, "vsscanf_s must reject a null format");
    number = -1;
    Require(ScanList("1 2", "%d %d", &number, static_cast<int*>(nullptr)) == EOF && number == 1,
        "vsscanf_s must stop at a null destination");
    Require(ScanList("ab", "ab%n", static_cast<int*>(nullptr)) == EOF, "vsscanf_s must reject a null count destination");
    Require(ScanList("x", "%d", &number) == 0, "vsscanf_s matching failure");
    Require(ScanList("", "%d", &number) == EOF, "vsscanf_s input failure");
    Require(sscanf_s_nid_postfix("5", "%d", static_cast<int*>(nullptr)) == EOF, "sscanf_s must reject a null destination");
}

int main() {
    CheckArgumentList();
    for (const int count : {0, 3, 127}) {
        CheckCount<signed char>("hh", count);
        CheckCount<short>("h", count);
        CheckCount<int>("", count);
        CheckCount<std::int64_t>("l", count);
        CheckCount<long long>("ll", count);
        CheckCount<std::intmax_t>("j", count);
        CheckCount<std::int64_t>("z", count);
        CheckCount<std::ptrdiff_t>("t", count);
    }
    CheckCount<short>("h", 257);
    CheckCount<std::int64_t>("l", 257);

    int number = -1;
    std::int64_t before = -1;
    std::int64_t after = -1;
    char word[4] = {};
    Require(sscanf_s_nid_postfix(" 12 abc!", "%ln%d %3[a-z]%jn!", &before, &number,
        word, 4u, &after) == 2, "Count conversions must not increase assignments");
    Require(before == 0 && number == 12 && std::strcmp(word, "abc") == 0 && after == 7,
        "Counts must include leading whitespace and earlier conversions");

    after = -1;
    Require(sscanf_s_nid_postfix("123x", "%*d%tn", &after) == 0 && after == 3,
        "Count must include suppressed input without consuming a capacity argument");
    after = -1;
    Require(sscanf_s_nid_postfix("x", "%d%lln", &number, &after) == 0 && after == -1,
        "Matching failure must leave a later count untouched");
    Require(sscanf_s_nid_postfix("", "%d%lln", &number, &after) == EOF && after == -1,
        "Input failure must leave a later count untouched");
    return failures == 0 ? 0 : 1;
}
