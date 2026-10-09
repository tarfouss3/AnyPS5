#include "SceTypes.hpp"
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace {

struct ColumnValue {
    std::uint32_t size;
    std::int64_t value;
};

struct Column {
    std::int32_t column_type;
    std::int32_t column_operator;
    const ColumnValue* value;
    std::int32_t logical_connector;
};

struct ColumnSet {
    const Column* columns;
    std::int32_t columns_length;
    std::int32_t logical_connector;
};

struct OrderByCondition {
    std::int32_t column_type;
    std::int32_t sort_type;
};

}

extern "C" int APS5_VABI sceContentSearchInit(const ContentSearchInitParam* init_param);
extern "C" int APS5_VABI sceContentSearchTerm(void);
extern "C" int APS5_VABI sceContentSearchSearchContent(const ColumnSet* column_set, std::uint32_t column_set_length, const OrderByCondition* order_by_conditions, std::uint32_t order_by_conditions_length, std::uint32_t offset, std::uint32_t limit, std::int64_t* num_of_content, void* infos, std::int64_t* last_update_id);

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

}

int main() {
    constexpr int limitTooBig = static_cast<int>(0x809D100A);
    constexpr std::size_t contentInfoSize = 2400;
    const ColumnValue photo{sizeof(std::int64_t), 1};
    const Column column{3, 0, &photo, 2};
    const ColumnSet columnSet{&column, 1, 2};
    const OrderByCondition byTitle{4, 0};
    std::vector<std::uint8_t> infos(contentInfoSize * 92, 0x5a);
    std::int64_t numOfContent = -1;
    std::int64_t lastUpdateId = -1;

    Require(Throws([&] { sceContentSearchSearchContent(&columnSet, 1, &byTitle, 1, 0, 92, &numOfContent, infos.data(), &lastUpdateId); }));
    ContentSearchInitParam param{3 * 1024 * 1024};
    Require(sceContentSearchInit(&param) == 0);

    Require(sceContentSearchSearchContent(&columnSet, 1, &byTitle, 1, 0, 92, &numOfContent, infos.data(), &lastUpdateId) == 0);
    Require(numOfContent == 0);
    const std::int64_t firstUpdateId = lastUpdateId;
    for (const std::uint8_t byte : infos) Require(byte == 0x5a);

    numOfContent = -1;
    lastUpdateId = -1;
    Require(sceContentSearchSearchContent(nullptr, 0, nullptr, 0, 10, 1, &numOfContent, infos.data(), &lastUpdateId) == 0);
    Require(numOfContent == 0 && lastUpdateId == firstUpdateId);

    Require(sceContentSearchSearchContent(&columnSet, 1, &byTitle, 1, 0, 93, &numOfContent, infos.data(), &lastUpdateId) == limitTooBig);
    Require(ThrowsRuntime([&] { sceContentSearchSearchContent(&columnSet, 1, &byTitle, 1, 0, 0, &numOfContent, infos.data(), &lastUpdateId); }));
    Require(Throws([&] { sceContentSearchSearchContent(&columnSet, 1, &byTitle, 1, 0, 92, nullptr, infos.data(), &lastUpdateId); }));
    Require(Throws([&] { sceContentSearchSearchContent(&columnSet, 1, &byTitle, 1, 0, 92, &numOfContent, infos.data(), nullptr); }));
    Require(Throws([&] { sceContentSearchSearchContent(&columnSet, 1, &byTitle, 1, 0, 92, &numOfContent, nullptr, &lastUpdateId); }));
    Require(Throws([&] { sceContentSearchSearchContent(nullptr, 1, &byTitle, 1, 0, 92, &numOfContent, infos.data(), &lastUpdateId); }));
    const Column unknownColumn{8, 0, &photo, 2};
    const ColumnSet unknownSet{&unknownColumn, 1, 2};
    Require(Throws([&] { sceContentSearchSearchContent(&unknownSet, 1, &byTitle, 1, 0, 92, &numOfContent, infos.data(), &lastUpdateId); }));
    const Column missingValue{3, 0, nullptr, 2};
    const ColumnSet missingValueSet{&missingValue, 1, 2};
    Require(Throws([&] { sceContentSearchSearchContent(&missingValueSet, 1, &byTitle, 1, 0, 92, &numOfContent, infos.data(), &lastUpdateId); }));
    const OrderByCondition byType{3, 0};
    Require(Throws([&] { sceContentSearchSearchContent(&columnSet, 1, &byType, 1, 0, 92, &numOfContent, infos.data(), &lastUpdateId); }));

    Require(sceContentSearchTerm() == 0);
    Require(Throws([&] { sceContentSearchSearchContent(&columnSet, 1, &byTitle, 1, 0, 92, &numOfContent, infos.data(), &lastUpdateId); }));
}
