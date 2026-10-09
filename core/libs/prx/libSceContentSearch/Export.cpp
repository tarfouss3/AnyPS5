#include <cstdint>
#include <cstddef>
#include "SceTypes.hpp"
#include "prx/libc/include/General.hpp"
#include <atomic>
#include <stdexcept>
#include <string>

namespace {

constexpr int CONTENT_SEARCH_ERROR_LIMIT_TOO_BIG = static_cast<int>(0x809D100A);
constexpr uint32_t MAX_LIMIT = 92;
constexpr int64_t LIBRARY_UPDATE_ID = 0;

struct ContentSearchColumnValue {
    uint32_t size;
    int64_t value;
};

struct ContentSearchContentColumn {
    int32_t column_type;
    int32_t column_operator;
    const ContentSearchColumnValue* value;
    int32_t logical_connector;
};

struct ContentSearchContentColumnSet {
    const ContentSearchContentColumn* columns;
    int32_t columns_length;
    int32_t logical_connector;
};

struct ContentSearchContentOrderByCondition {
    int32_t column_type;
    int32_t sort_type;
};

static_assert(sizeof(ContentSearchColumnValue) == 16);
static_assert(sizeof(ContentSearchContentColumn) == 24);
static_assert(sizeof(ContentSearchContentColumnSet) == 16);
static_assert(sizeof(ContentSearchContentOrderByCondition) == 8);

std::atomic<bool> g_initialized{false};

bool IsContentColumnType(int32_t columnType) {
    return (columnType >= 1 && columnType <= 7) || (columnType >= 9 && columnType <= 12) || columnType == 14;
}

bool IsContentSortColumnType(int32_t columnType) {
    return columnType == 1 || (columnType >= 4 && columnType <= 6) || columnType == 9 || columnType == 10;
}

bool IsLogicalConnector(int32_t logicalConnector) {
    return logicalConnector >= 0 && logicalConnector <= 2;
}

void ValidateColumnSets(const char* funcName, const ContentSearchContentColumnSet* columnSets, uint32_t columnSetLength) {
    if (columnSets == nullptr && columnSetLength != 0) throw std::invalid_argument(std::string(funcName) + ": null column sets");
    for (uint32_t setIndex = 0; setIndex < columnSetLength; ++setIndex) {
        const auto& columnSet = columnSets[setIndex];
        if (columnSet.columns_length < 0 || (columnSet.columns == nullptr && columnSet.columns_length != 0)) throw std::invalid_argument(std::string(funcName) + ": invalid columns of column set " + std::to_string(setIndex));
        if (!IsLogicalConnector(columnSet.logical_connector)) throw std::invalid_argument(std::string(funcName) + ": logical connector " + std::to_string(columnSet.logical_connector) + " of column set " + std::to_string(setIndex));
        for (int32_t columnIndex = 0; columnIndex < columnSet.columns_length; ++columnIndex) {
            const auto& column = columnSet.columns[columnIndex];
            if (!IsContentColumnType(column.column_type)) throw std::invalid_argument(std::string(funcName) + ": column type " + std::to_string(column.column_type));
            if (column.column_operator < 0 || column.column_operator > 5) throw std::invalid_argument(std::string(funcName) + ": column operator " + std::to_string(column.column_operator));
            if (column.value == nullptr) throw std::invalid_argument(std::string(funcName) + ": null column value");
            if (!IsLogicalConnector(column.logical_connector)) throw std::invalid_argument(std::string(funcName) + ": logical connector " + std::to_string(column.logical_connector));
        }
    }
}

void ValidateOrderByConditions(const char* funcName, const ContentSearchContentOrderByCondition* conditions, uint32_t conditionsLength) {
    if (conditions == nullptr && conditionsLength != 0) throw std::invalid_argument(std::string(funcName) + ": null order-by conditions");
    for (uint32_t index = 0; index < conditionsLength; ++index) {
        if (!IsContentSortColumnType(conditions[index].column_type)) throw std::invalid_argument(std::string(funcName) + ": sort column type " + std::to_string(conditions[index].column_type));
        if (conditions[index].sort_type != 0 && conditions[index].sort_type != 1) throw std::invalid_argument(std::string(funcName) + ": sort type " + std::to_string(conditions[index].sort_type));
    }
}

}

extern "C" {

int APS5_VABI sceContentSearchInit(const ContentSearchInitParam* init_param) {
    if (init_param == nullptr) APS5_INVALID_ARG_EX;
    bool expected = false;
    if (!g_initialized.compare_exchange_strong(expected, true)) throw std::logic_error(std::string(__func__) + ": already initialized");
    return 0;
}

int APS5_VABI sceContentSearchCloseMetadata(void) {
 NotImplemented_nid_no_patch(__func__);
 return 0;
}

int APS5_VABI sceContentSearchGetMetadataFieldInfo(void) {
 NotImplemented_nid_no_patch(__func__);
 return 0;
}

int APS5_VABI sceContentSearchGetMetadataValue(void) {
 NotImplemented_nid_no_patch(__func__);
 return 0;
}

int APS5_VABI sceContentSearchOpenMetadata(void) {
 NotImplemented_nid_no_patch(__func__);
 return 0;
}

int APS5_VABI sceContentSearchSearchContent(const ContentSearchContentColumnSet* column_set, uint32_t column_set_length, const ContentSearchContentOrderByCondition* order_by_conditions, uint32_t order_by_conditions_length, uint32_t offset, uint32_t limit, int64_t* num_of_content, void* infos, int64_t* last_update_id) {
    (void)offset;
    if (!g_initialized.load()) throw std::logic_error(std::string(__func__) + ": not initialized");
    if (num_of_content == nullptr || last_update_id == nullptr || infos == nullptr) APS5_INVALID_ARG_EX;
    ValidateColumnSets(__func__, column_set, column_set_length);
    ValidateOrderByConditions(__func__, order_by_conditions, order_by_conditions_length);
    if (limit == 0) throw std::runtime_error(std::string(__func__) + ": a zero limit is not supported");
    if (limit > MAX_LIMIT) return CONTENT_SEARCH_ERROR_LIMIT_TOO_BIG;
    *num_of_content = 0;
    *last_update_id = LIBRARY_UPDATE_ID;
    return 0;
}

int APS5_VABI sceContentSearchTerm(void) {
    bool expected = true;
    if (!g_initialized.compare_exchange_strong(expected, false)) throw std::logic_error(std::string(__func__) + ": not initialized");
    return 0;
}

}
