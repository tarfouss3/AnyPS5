#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <deque>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include "prx/libc/include/General.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
}

namespace {

constexpr std::int32_t SCE_OK = 0;
constexpr std::int32_t SCE_VDECSW_ERROR_STRUCT_SIZE = static_cast<std::int32_t>(0x81510101);
constexpr std::int32_t SCE_VDECSW_ERROR_ARGUMENT_POINTER = static_cast<std::int32_t>(0x81510102);
constexpr std::int32_t SCE_VDECSW_ERROR_DECODER_INSTANCE = static_cast<std::int32_t>(0x81510103);
constexpr std::int32_t SCE_VDECSW_ERROR_MEMORY_POINTER = static_cast<std::int32_t>(0x81510105);
constexpr std::int32_t SCE_VDECSW_ERROR_FRAME_BUFFER_SIZE = static_cast<std::int32_t>(0x81510106);
constexpr std::int32_t SCE_VDECSW_ERROR_FRAME_BUFFER_POINTER = static_cast<std::int32_t>(0x81510107);
constexpr std::int32_t SCE_VDECSW_ERROR_ACCESS_UNIT_SIZE = static_cast<std::int32_t>(0x8151010D);
constexpr std::int32_t SCE_VDECSW_ERROR_ACCESS_UNIT_POINTER = static_cast<std::int32_t>(0x8151010E);
constexpr std::int32_t SCE_VDECSW_ERROR_OUTPUT_BUFFER_FULL = static_cast<std::int32_t>(0x81510114);
constexpr std::int32_t SCE_VDECSW_ERROR_INPUT_QUEUE_EMPTY = static_cast<std::int32_t>(0x81510116);
constexpr std::int32_t SCE_VDECSW_ERROR_DECODE_PENDING = static_cast<std::int32_t>(0x81510117);
constexpr std::int32_t SCE_VDECSW_ERROR_OUTPUT_BUFFER_EMPTY = static_cast<std::int32_t>(0x81510118);
constexpr std::int32_t SCE_VDECSW_ERROR_CONFIG_INFO = static_cast<std::int32_t>(0x81510200);
constexpr std::int32_t SCE_VDECSW_ERROR_COMPUTE_PIPE_ID = static_cast<std::int32_t>(0x81510201);
constexpr std::int32_t SCE_VDECSW_ERROR_COMPUTE_QUEUE_ID = static_cast<std::int32_t>(0x81510202);

constexpr std::uint32_t CodecAvc = 1;
constexpr std::uint32_t ResourceCompute = 1;
constexpr std::uint64_t WorkMemoryBytes = 16ull << 20u;
constexpr std::uint32_t FrameBufferAlignment = 0x100;
constexpr std::uint64_t PitchAlignment = 64;
constexpr std::uint32_t FrameFormatNv12 = 0;
constexpr std::uint16_t MaxComputePipeId = 4;
constexpr std::uint16_t MaxComputeQueueId = 7;
constexpr std::size_t SliceHeaderBytes = 64;
constexpr std::size_t PictureParameterBytes = 16;

struct ComputeMemoryInfo {
    std::uint64_t thisSize;
    std::uint64_t cpuGpuMemorySize;
    void* cpuGpuMemory;
};
static_assert(sizeof(ComputeMemoryInfo) == 0x18);

struct ComputeConfigInfo {
    std::uint64_t thisSize;
    std::uint16_t computePipeId;
    std::uint16_t computeQueueId;
    bool checkMemoryType;
    std::uint8_t reserved0;
    std::uint16_t reserved1;
};
static_assert(sizeof(ComputeConfigInfo) == 0x10);

struct DecoderConfigInfo {
    std::uint64_t thisSize;
    std::uint32_t resourceType;
    std::uint32_t codecType;
    std::uint32_t profile;
    std::uint32_t maxLevel;
    std::int32_t maxFrameWidth;
    std::int32_t maxFrameHeight;
    std::int32_t maxDpbFrameCount;
    std::uint32_t decodePipelineDepth;
    void* computeQueue;
    std::uint64_t cpuAffinityMask;
    std::int32_t cpuThreadPriority;
    bool optimizeProgressiveVideo;
    bool checkMemoryType;
    std::uint8_t reserved0;
    std::int8_t extraDpbFrameCount;
    void* extraConfigInfo;
    bool disableSyncDecode;
    std::uint32_t maxPendingSyncCount;
};
static_assert(sizeof(DecoderConfigInfo) == 0x50);
static_assert(offsetof(DecoderConfigInfo, computeQueue) == 0x28);
static_assert(offsetof(DecoderConfigInfo, extraDpbFrameCount) == 0x3F);
static_assert(offsetof(DecoderConfigInfo, disableSyncDecode) == 0x48);
constexpr std::uint64_t DecoderConfigInfoV1Size = offsetof(DecoderConfigInfo, disableSyncDecode);

struct DecoderMemoryInfo {
    std::uint64_t thisSize;
    std::uint64_t cpuMemorySize;
    void* cpuMemory;
    std::uint64_t gpuMemorySize;
    void* gpuMemory;
    std::uint64_t cpuGpuMemorySize;
    void* cpuGpuMemory;
    std::uint64_t maxFrameBufferSize;
    std::uint32_t frameBufferAlignment;
    std::uint32_t reserved0;
};
static_assert(sizeof(DecoderMemoryInfo) == 0x48);

struct InputData {
    std::uint64_t thisSize;
    void* auData;
    std::uint64_t auSize;
    std::uint64_t ptsData;
    std::uint64_t dtsData;
    std::uint64_t attachedData;
};
static_assert(sizeof(InputData) == 0x30);

struct InputResult {
    std::uint64_t thisSize;
    void* decodedAu;
    std::uint32_t outputFrameCount;
    std::uint32_t reserved0;
};
static_assert(sizeof(InputResult) == 0x18);

struct FrameBuffer {
    std::uint64_t thisSize;
    void* frameBuffer;
    std::uint64_t frameBufferSize;
};
static_assert(sizeof(FrameBuffer) == 0x18);

struct OutputInfo {
    std::uint64_t thisSize;
    bool isValid;
    bool isLastFrame;
    bool isErrorFrame;
    std::uint8_t pictureCount;
    std::uint32_t codecType;
    std::uint32_t frameWidth;
    std::uint32_t framePitch;
    std::uint32_t frameHeight;
    bool isDiscardedFrame;
    void* frameBuffer;
    std::uint64_t frameBufferSize;
    std::uint32_t frameFormat;
    std::uint32_t framePitchInBytes;
};
static_assert(sizeof(OutputInfo) == 0x38);
static_assert(offsetof(OutputInfo, pictureCount) == 0x0B);
static_assert(offsetof(OutputInfo, frameBuffer) == 0x20);
constexpr std::uint64_t OutputInfoV1Size = offsetof(OutputInfo, frameFormat);

struct AvcPictureInfo {
    std::uint64_t thisSize;
    bool isValid;
    std::uint64_t ptsData;
    std::uint64_t dtsData;
    std::uint64_t attachedData;
    std::uint8_t idrPictureFlag;
    std::uint8_t profileIdc;
    std::uint8_t levelIdc;
    std::uint32_t picWidthInMbsMinus1;
    std::uint32_t picHeightInMapUnitsMinus1;
    std::uint8_t frameMbsOnlyFlag;
    std::uint8_t frameCroppingFlag;
    std::uint32_t frameCropLeftOffset;
    std::uint32_t frameCropRightOffset;
    std::uint32_t frameCropTopOffset;
    std::uint32_t frameCropBottomOffset;
    std::uint8_t aspectRatioInfoPresentFlag;
    std::uint8_t aspectRatioIdc;
    std::uint16_t sarWidth;
    std::uint16_t sarHeight;
    std::uint8_t videoSignalTypePresentFlag;
    std::uint8_t videoFormat;
    std::uint8_t videoFullRangeFlag;
    std::uint8_t colourDescriptionPresentFlag;
    std::uint8_t colourPrimaries;
    std::uint8_t transferCharacteristics;
    std::uint8_t matrixCoefficients;
    std::uint8_t timingInfoPresentFlag;
    std::uint32_t numUnitsInTick;
    std::uint32_t timeScale;
    std::uint8_t fixedFrameRateFlag;
    std::uint8_t bitstreamRestrictionFlag;
    std::uint8_t maxDecFrameBuffering;
    std::uint8_t picStructPresentFlag;
    std::uint8_t picStruct;
    std::uint8_t fieldPicFlag;
    std::uint8_t bottomFieldFlag;
    std::uint8_t sequenceParameterSetPresentFlag;
    std::uint8_t pictureParameterSetPresentFlag;
    std::uint8_t auDelimiterPresentFlag;
    std::uint8_t endOfSequencePresentFlag;
    std::uint8_t endOfStreamPresentFlag;
    std::uint8_t fillerDataPresentFlag;
    std::uint8_t pictureTimingSeiPresentFlag;
    std::uint8_t bufferingPeriodSeiPresentFlag;
    std::uint8_t constraintSet0Flag;
    std::uint8_t constraintSet1Flag;
    std::uint8_t constraintSet2Flag;
    std::uint8_t constraintSet3Flag;
    std::uint8_t constraintSet4Flag;
    std::uint8_t constraintSet5Flag;
};
static_assert(sizeof(AvcPictureInfo) == 0x78);
static_assert(offsetof(AvcPictureInfo, picWidthInMbsMinus1) == 0x2C);
static_assert(offsetof(AvcPictureInfo, frameCropLeftOffset) == 0x38);
static_assert(offsetof(AvcPictureInfo, aspectRatioInfoPresentFlag) == 0x48);
static_assert(offsetof(AvcPictureInfo, numUnitsInTick) == 0x58);
static_assert(offsetof(AvcPictureInfo, fixedFrameRateFlag) == 0x60);
static_assert(offsetof(AvcPictureInfo, constraintSet5Flag) == 0x74);

class BitReader {
public:
    BitReader(const std::uint8_t* data, std::size_t size) : data(data), size(size) {}

    std::uint32_t Bits(unsigned count) {
        std::uint64_t value = 0;
        for (unsigned index = 0; index < count; ++index) {
            if (position >= size * 8) throw std::runtime_error("libSceVdecsw: truncated H.264 syntax element");
            value = (value << 1u) | ((data[position / 8] >> (7 - position % 8)) & 1u);
            ++position;
        }
        return static_cast<std::uint32_t>(value);
    }

    bool Flag() {
        return Bits(1) != 0;
    }

    std::uint32_t Unsigned() {
        unsigned zeros = 0;
        while (Bits(1) == 0) {
            if (++zeros > 31) throw std::runtime_error("libSceVdecsw: invalid Exp-Golomb code");
        }
        return static_cast<std::uint32_t>((std::uint64_t{1} << zeros) - 1 + Bits(zeros));
    }

    std::int32_t Signed() {
        const std::int64_t code = Unsigned();
        return static_cast<std::int32_t>(code % 2 == 1 ? (code + 1) / 2 : -(code / 2));
    }

private:
    const std::uint8_t* data;
    std::size_t size;
    std::size_t position = 0;
};

struct SequenceParameterSet {
    std::uint8_t profileIdc = 0;
    std::array<std::uint8_t, 6> constraintSetFlags{};
    std::uint8_t levelIdc = 0;
    std::uint32_t chromaFormatIdc = 1;
    bool separateColourPlaneFlag = false;
    std::uint32_t bitDepthLumaMinus8 = 0;
    std::uint32_t bitDepthChromaMinus8 = 0;
    std::uint32_t log2MaxFrameNumMinus4 = 0;
    std::uint32_t picWidthInMbsMinus1 = 0;
    std::uint32_t picHeightInMapUnitsMinus1 = 0;
    bool frameMbsOnlyFlag = true;
    bool frameCroppingFlag = false;
    std::uint32_t frameCropLeftOffset = 0;
    std::uint32_t frameCropRightOffset = 0;
    std::uint32_t frameCropTopOffset = 0;
    std::uint32_t frameCropBottomOffset = 0;
    bool aspectRatioInfoPresentFlag = false;
    std::uint8_t aspectRatioIdc = 0;
    std::uint16_t sarWidth = 0;
    std::uint16_t sarHeight = 0;
    bool videoSignalTypePresentFlag = false;
    std::uint8_t videoFormat = 5;
    bool videoFullRangeFlag = false;
    bool colourDescriptionPresentFlag = false;
    std::uint8_t colourPrimaries = 2;
    std::uint8_t transferCharacteristics = 2;
    std::uint8_t matrixCoefficients = 2;
    bool timingInfoPresentFlag = false;
    std::uint32_t numUnitsInTick = 0;
    std::uint32_t timeScale = 0;
    bool fixedFrameRateFlag = false;
    bool cpbDpbDelaysPresentFlag = false;
    std::uint32_t cpbRemovalDelayLength = 0;
    std::uint32_t dpbOutputDelayLength = 0;
    bool picStructPresentFlag = false;
    bool bitstreamRestrictionFlag = false;
    std::uint32_t maxDecFrameBuffering = 0;
    std::string infoError;
};

struct AccessUnitInfo {
    bool hasPicture = false;
    bool idrPicture = false;
    bool fieldPicFlag = false;
    bool bottomFieldFlag = false;
    bool sequenceParameterSetPresent = false;
    bool pictureParameterSetPresent = false;
    bool auDelimiterPresent = false;
    bool endOfSequencePresent = false;
    bool endOfStreamPresent = false;
    bool fillerDataPresent = false;
    bool pictureTimingSeiPresent = false;
    bool bufferingPeriodSeiPresent = false;
    std::uint8_t picStruct = 0;
    SequenceParameterSet sequence;
    std::string infoError;
};

struct NalUnit {
    const std::uint8_t* data;
    std::size_t size;
};

std::vector<std::uint8_t> Unescape(const std::uint8_t* data, std::size_t size) {
    std::vector<std::uint8_t> payload;
    payload.reserve(size);
    std::size_t zeros = 0;
    for (std::size_t index = 0; index < size; ++index) {
        if (zeros >= 2 && data[index] == 3) {
            zeros = 0;
            continue;
        }
        payload.push_back(data[index]);
        zeros = data[index] == 0 ? zeros + 1 : 0;
    }
    return payload;
}

std::vector<NalUnit> SplitAnnexB(const std::uint8_t* data, std::size_t size) {
    std::size_t start = 0;
    while (start < size && data[start] == 0) ++start;
    if (start < 2 || start == size || data[start] != 1) throw std::runtime_error("libSceVdecsw: access unit is not an Annex B byte stream");
    ++start;
    std::vector<NalUnit> units;
    while (start < size) {
        std::size_t end = start;
        while (end + 2 < size && !(data[end] == 0 && data[end + 1] == 0 && data[end + 2] <= 1)) ++end;
        std::size_t next = size;
        if (end + 2 < size) {
            std::size_t cursor = end;
            while (cursor < size && data[cursor] == 0) ++cursor;
            if (cursor < size) {
                if (data[cursor] != 1) throw std::runtime_error("libSceVdecsw: malformed Annex B start code");
                next = cursor + 1;
            }
        } else {
            end = size;
            while (end > start && data[end - 1] == 0) --end;
        }
        if (end == start) throw std::runtime_error("libSceVdecsw: empty NAL unit");
        units.push_back({data + start, end - start});
        start = next;
    }
    return units;
}

void SkipScalingList(BitReader& reader, unsigned count) {
    std::int32_t last = 8;
    std::int32_t next = 8;
    for (unsigned index = 0; index < count; ++index) {
        if (next != 0) {
            const auto delta = reader.Signed();
            if (delta < -128 || delta > 127) throw std::runtime_error("libSceVdecsw: invalid scaling list delta");
            next = (last + delta + 256) % 256;
        }
        last = next == 0 ? last : next;
    }
}

void ParseHrd(BitReader& reader, SequenceParameterSet& sequence) {
    const auto count = reader.Unsigned();
    if (count > 31) throw std::runtime_error("libSceVdecsw: invalid cpb_cnt_minus1");
    reader.Bits(4);
    reader.Bits(4);
    for (std::uint32_t index = 0; index <= count; ++index) {
        reader.Unsigned();
        reader.Unsigned();
        reader.Flag();
    }
    reader.Bits(5);
    sequence.cpbRemovalDelayLength = reader.Bits(5) + 1;
    sequence.dpbOutputDelayLength = reader.Bits(5) + 1;
    reader.Bits(5);
}

void ParseVui(BitReader& reader, SequenceParameterSet& sequence) {
    sequence.aspectRatioInfoPresentFlag = reader.Flag();
    if (sequence.aspectRatioInfoPresentFlag) {
        sequence.aspectRatioIdc = static_cast<std::uint8_t>(reader.Bits(8));
        if (sequence.aspectRatioIdc == 255) {
            sequence.sarWidth = static_cast<std::uint16_t>(reader.Bits(16));
            sequence.sarHeight = static_cast<std::uint16_t>(reader.Bits(16));
        }
    }
    if (reader.Flag()) reader.Flag();
    sequence.videoSignalTypePresentFlag = reader.Flag();
    if (sequence.videoSignalTypePresentFlag) {
        sequence.videoFormat = static_cast<std::uint8_t>(reader.Bits(3));
        sequence.videoFullRangeFlag = reader.Flag();
        sequence.colourDescriptionPresentFlag = reader.Flag();
        if (sequence.colourDescriptionPresentFlag) {
            sequence.colourPrimaries = static_cast<std::uint8_t>(reader.Bits(8));
            sequence.transferCharacteristics = static_cast<std::uint8_t>(reader.Bits(8));
            sequence.matrixCoefficients = static_cast<std::uint8_t>(reader.Bits(8));
        }
    }
    if (reader.Flag()) {
        reader.Unsigned();
        reader.Unsigned();
    }
    sequence.timingInfoPresentFlag = reader.Flag();
    if (sequence.timingInfoPresentFlag) {
        sequence.numUnitsInTick = reader.Bits(32);
        sequence.timeScale = reader.Bits(32);
        sequence.fixedFrameRateFlag = reader.Flag();
    }
    const bool nalHrd = reader.Flag();
    if (nalHrd) ParseHrd(reader, sequence);
    const bool vclHrd = reader.Flag();
    if (vclHrd) ParseHrd(reader, sequence);
    sequence.cpbDpbDelaysPresentFlag = nalHrd || vclHrd;
    if (sequence.cpbDpbDelaysPresentFlag) reader.Flag();
    sequence.picStructPresentFlag = reader.Flag();
    sequence.bitstreamRestrictionFlag = reader.Flag();
    if (sequence.bitstreamRestrictionFlag) {
        reader.Flag();
        for (int index = 0; index < 5; ++index) reader.Unsigned();
        sequence.maxDecFrameBuffering = reader.Unsigned();
        if (sequence.maxDecFrameBuffering > std::numeric_limits<std::uint8_t>::max()) throw std::runtime_error("libSceVdecsw: max_dec_frame_buffering out of range");
    }
}

std::optional<std::uint64_t> MaxDpbMbs(const SequenceParameterSet& sequence) {
    const auto profile = sequence.profileIdc;
    if (sequence.levelIdc == 11 && sequence.constraintSetFlags[3] != 0 && (profile == 66 || profile == 77 || profile == 88)) return 396;
    switch (sequence.levelIdc) {
        case 9:
        case 10: return 396;
        case 11: return 900;
        case 12:
        case 13:
        case 20: return 2376;
        case 21: return 4752;
        case 22:
        case 30: return 8100;
        case 31: return 18000;
        case 32: return 20480;
        case 40:
        case 41: return 32768;
        case 42: return 34816;
        case 50: return 110400;
        case 51:
        case 52: return 184320;
        case 60:
        case 61:
        case 62: return 696320;
        default: return std::nullopt;
    }
}

void InferMaxDecFrameBuffering(SequenceParameterSet& sequence) {
    if (sequence.bitstreamRestrictionFlag || !sequence.infoError.empty()) return;
    const auto profile = sequence.profileIdc;
    if (sequence.constraintSetFlags[3] != 0 && (profile == 44 || profile == 86 || profile == 100 || profile == 110 || profile == 122 || profile == 244)) {
        sequence.maxDecFrameBuffering = 0;
        return;
    }
    const auto mbs = MaxDpbMbs(sequence);
    if (!mbs) {
        sequence.infoError = "libSceVdecsw: max_dec_frame_buffering cannot be inferred for level_idc " + std::to_string(sequence.levelIdc);
        return;
    }
    const std::uint64_t frameMbs = (static_cast<std::uint64_t>(sequence.picWidthInMbsMinus1) + 1) * (2 - (sequence.frameMbsOnlyFlag ? 1 : 0)) * (static_cast<std::uint64_t>(sequence.picHeightInMapUnitsMinus1) + 1);
    sequence.maxDecFrameBuffering = static_cast<std::uint32_t>(std::min<std::uint64_t>(*mbs / frameMbs, 16));
}

bool HighProfile(std::uint8_t profile) {
    switch (profile) {
        case 100:
        case 110:
        case 122:
        case 244:
        case 44:
        case 83:
        case 86:
        case 118:
        case 128:
        case 138:
        case 139:
        case 134:
        case 135: return true;
        default: return false;
    }
}

std::pair<std::uint32_t, SequenceParameterSet> ParseSequence(const NalUnit& nal) {
    const auto payload = Unescape(nal.data + 1, nal.size - 1);
    BitReader reader(payload.data(), payload.size());
    SequenceParameterSet sequence;
    sequence.profileIdc = static_cast<std::uint8_t>(reader.Bits(8));
    for (auto& flag : sequence.constraintSetFlags) flag = static_cast<std::uint8_t>(reader.Bits(1));
    reader.Bits(2);
    sequence.levelIdc = static_cast<std::uint8_t>(reader.Bits(8));
    const auto id = reader.Unsigned();
    if (id > 31) throw std::runtime_error("libSceVdecsw: invalid seq_parameter_set_id");
    if (HighProfile(sequence.profileIdc)) {
        sequence.chromaFormatIdc = reader.Unsigned();
        if (sequence.chromaFormatIdc > 3) throw std::runtime_error("libSceVdecsw: invalid chroma_format_idc");
        if (sequence.chromaFormatIdc == 3) sequence.separateColourPlaneFlag = reader.Flag();
        sequence.bitDepthLumaMinus8 = reader.Unsigned();
        sequence.bitDepthChromaMinus8 = reader.Unsigned();
        reader.Flag();
        if (reader.Flag()) {
            const unsigned lists = sequence.chromaFormatIdc != 3 ? 8 : 12;
            for (unsigned index = 0; index < lists; ++index) {
                if (reader.Flag()) SkipScalingList(reader, index < 6 ? 16 : 64);
            }
        }
    }
    sequence.log2MaxFrameNumMinus4 = reader.Unsigned();
    if (sequence.log2MaxFrameNumMinus4 > 12) throw std::runtime_error("libSceVdecsw: invalid log2_max_frame_num_minus4");
    const auto pocType = reader.Unsigned();
    if (pocType == 0) {
        if (reader.Unsigned() > 12) throw std::runtime_error("libSceVdecsw: invalid log2_max_pic_order_cnt_lsb_minus4");
    } else if (pocType == 1) {
        reader.Flag();
        reader.Signed();
        reader.Signed();
        const auto cycle = reader.Unsigned();
        if (cycle > 255) throw std::runtime_error("libSceVdecsw: invalid num_ref_frames_in_pic_order_cnt_cycle");
        for (std::uint32_t index = 0; index < cycle; ++index) reader.Signed();
    } else if (pocType != 2) {
        throw std::runtime_error("libSceVdecsw: invalid pic_order_cnt_type");
    }
    reader.Unsigned();
    reader.Flag();
    sequence.picWidthInMbsMinus1 = reader.Unsigned();
    sequence.picHeightInMapUnitsMinus1 = reader.Unsigned();
    sequence.frameMbsOnlyFlag = reader.Flag();
    if (!sequence.frameMbsOnlyFlag) reader.Flag();
    reader.Flag();
    sequence.frameCroppingFlag = reader.Flag();
    if (sequence.frameCroppingFlag) {
        sequence.frameCropLeftOffset = reader.Unsigned();
        sequence.frameCropRightOffset = reader.Unsigned();
        sequence.frameCropTopOffset = reader.Unsigned();
        sequence.frameCropBottomOffset = reader.Unsigned();
    }
    if (reader.Flag()) {
        try {
            ParseVui(reader, sequence);
        } catch (const std::runtime_error& error) {
            sequence.infoError = error.what();
        }
    }
    InferMaxDecFrameBuffering(sequence);
    return {id, sequence};
}

class AvcSyntax {
public:
    AccessUnitInfo Parse(const std::uint8_t* data, std::size_t size) {
        AccessUnitInfo unit;
        std::optional<std::vector<std::uint8_t>> timing;
        std::string seiError;
        for (const auto& nal : SplitAnnexB(data, size)) {
            if ((nal.data[0] & 0x80u) != 0) throw std::runtime_error("libSceVdecsw: forbidden_zero_bit is set");
            const auto type = static_cast<unsigned>(nal.data[0] & 0x1fu);
            switch (type) {
                case 1:
                case 5: parseSlice(nal, type == 5, unit); break;
                case 6: parseSei(nal, unit, timing, seiError); break;
                case 7: {
                    auto [id, sequence] = ParseSequence(nal);
                    sequences[id] = std::move(sequence);
                    unit.sequenceParameterSetPresent = true;
                    break;
                }
                case 8: parsePictureParameters(nal); unit.pictureParameterSetPresent = true; break;
                case 9: unit.auDelimiterPresent = true; break;
                case 10: unit.endOfSequencePresent = true; break;
                case 11: unit.endOfStreamPresent = true; break;
                case 12: unit.fillerDataPresent = true; break;
                default: throw std::runtime_error("libSceVdecsw: H.264 NAL unit type " + std::to_string(type) + " is not implemented");
            }
        }
        if (!unit.hasPicture) return unit;
        unit.infoError = !seiError.empty() ? seiError : unit.sequence.infoError;
        if (unit.infoError.empty() && timing && unit.sequence.picStructPresentFlag) {
            try {
                BitReader reader(timing->data(), timing->size());
                if (unit.sequence.cpbDpbDelaysPresentFlag) {
                    reader.Bits(unit.sequence.cpbRemovalDelayLength);
                    reader.Bits(unit.sequence.dpbOutputDelayLength);
                }
                unit.picStruct = static_cast<std::uint8_t>(reader.Bits(4));
            } catch (const std::runtime_error& error) {
                unit.infoError = error.what();
            }
        }
        return unit;
    }

private:
    void parseSlice(const NalUnit& nal, bool idr, AccessUnitInfo& unit) {
        const auto header = Unescape(nal.data + 1, std::min(nal.size - 1, SliceHeaderBytes));
        BitReader reader(header.data(), header.size());
        const auto firstMb = reader.Unsigned();
        const auto sliceType = reader.Unsigned();
        const auto pictureId = reader.Unsigned();
        if (sliceType > 9 || pictureId > 255) throw std::runtime_error("libSceVdecsw: malformed slice header");
        if (unit.hasPicture) {
            if (firstMb == 0) throw std::runtime_error("libSceVdecsw: access units with several pictures are not implemented");
            unit.idrPicture = unit.idrPicture || idr;
            return;
        }
        const auto& sequenceId = pictureSequenceIds[pictureId];
        if (!sequenceId || !sequences[*sequenceId]) throw std::runtime_error("libSceVdecsw: slice refers to a parameter set that was not received");
        const auto& sequence = *sequences[*sequenceId];
        if (sequence.chromaFormatIdc != 1 || sequence.separateColourPlaneFlag || sequence.bitDepthLumaMinus8 != 0 || sequence.bitDepthChromaMinus8 != 0) throw std::runtime_error("libSceVdecsw: only 8-bit 4:2:0 pictures are implemented");
        reader.Bits(sequence.log2MaxFrameNumMinus4 + 4);
        if (!sequence.frameMbsOnlyFlag) {
            unit.fieldPicFlag = reader.Flag();
            if (unit.fieldPicFlag) unit.bottomFieldFlag = reader.Flag();
        }
        if (unit.fieldPicFlag) throw std::runtime_error("libSceVdecsw: field pictures are not implemented");
        unit.hasPicture = true;
        unit.idrPicture = idr;
        unit.sequence = sequence;
    }

    void parsePictureParameters(const NalUnit& nal) {
        const auto payload = Unescape(nal.data + 1, std::min(nal.size - 1, PictureParameterBytes));
        BitReader reader(payload.data(), payload.size());
        const auto pictureId = reader.Unsigned();
        const auto sequenceId = reader.Unsigned();
        if (pictureId > 255 || sequenceId > 31) throw std::runtime_error("libSceVdecsw: malformed picture parameter set");
        pictureSequenceIds[pictureId] = sequenceId;
    }

    static void parseSei(const NalUnit& nal, AccessUnitInfo& unit, std::optional<std::vector<std::uint8_t>>& timing, std::string& error) {
        try {
            const auto payload = Unescape(nal.data + 1, nal.size - 1);
            std::size_t end = payload.size();
            while (end > 0 && payload[end - 1] == 0) --end;
            if (end == 0 || payload[end - 1] != 0x80) throw std::runtime_error("libSceVdecsw: malformed SEI NAL unit");
            --end;
            std::size_t position = 0;
            const auto readValue = [&]() {
                std::uint64_t value = 0;
                for (;;) {
                    if (position >= end) throw std::runtime_error("libSceVdecsw: malformed SEI message");
                    const auto byte = payload[position++];
                    value += byte;
                    if (byte != 0xff) return value;
                }
            };
            while (position < end) {
                const auto type = readValue();
                const auto length = readValue();
                if (length > end - position) throw std::runtime_error("libSceVdecsw: SEI message overruns its NAL unit");
                if (type == 0) unit.bufferingPeriodSeiPresent = true;
                if (type == 1) {
                    unit.pictureTimingSeiPresent = true;
                    if (!timing) timing.emplace(payload.begin() + static_cast<std::ptrdiff_t>(position), payload.begin() + static_cast<std::ptrdiff_t>(position + length));
                }
                position += length;
            }
        } catch (const std::runtime_error& failure) {
            if (error.empty()) error = failure.what();
        }
    }

    std::array<std::optional<SequenceParameterSet>, 32> sequences;
    std::array<std::optional<std::uint32_t>, 256> pictureSequenceIds;
};

struct PictureRecord {
    AvcPictureInfo info{};
    std::string error;
};

AvcPictureInfo MakePictureInfo(const AccessUnitInfo& unit, const InputData& input) {
    const auto& sequence = unit.sequence;
    AvcPictureInfo info{};
    info.thisSize = sizeof(AvcPictureInfo);
    info.isValid = true;
    info.ptsData = input.ptsData;
    info.dtsData = input.dtsData;
    info.attachedData = input.attachedData;
    info.idrPictureFlag = unit.idrPicture ? 1 : 0;
    info.profileIdc = sequence.profileIdc;
    info.levelIdc = sequence.levelIdc;
    info.picWidthInMbsMinus1 = sequence.picWidthInMbsMinus1;
    info.picHeightInMapUnitsMinus1 = sequence.picHeightInMapUnitsMinus1;
    info.frameMbsOnlyFlag = sequence.frameMbsOnlyFlag ? 1 : 0;
    info.frameCroppingFlag = sequence.frameCroppingFlag ? 1 : 0;
    info.frameCropLeftOffset = sequence.frameCropLeftOffset;
    info.frameCropRightOffset = sequence.frameCropRightOffset;
    info.frameCropTopOffset = sequence.frameCropTopOffset;
    info.frameCropBottomOffset = sequence.frameCropBottomOffset;
    info.aspectRatioInfoPresentFlag = sequence.aspectRatioInfoPresentFlag ? 1 : 0;
    info.aspectRatioIdc = sequence.aspectRatioIdc;
    info.sarWidth = sequence.sarWidth;
    info.sarHeight = sequence.sarHeight;
    info.videoSignalTypePresentFlag = sequence.videoSignalTypePresentFlag ? 1 : 0;
    info.videoFormat = sequence.videoFormat;
    info.videoFullRangeFlag = sequence.videoFullRangeFlag ? 1 : 0;
    info.colourDescriptionPresentFlag = sequence.colourDescriptionPresentFlag ? 1 : 0;
    info.colourPrimaries = sequence.colourPrimaries;
    info.transferCharacteristics = sequence.transferCharacteristics;
    info.matrixCoefficients = sequence.matrixCoefficients;
    info.timingInfoPresentFlag = sequence.timingInfoPresentFlag ? 1 : 0;
    info.numUnitsInTick = sequence.numUnitsInTick;
    info.timeScale = sequence.timeScale;
    info.fixedFrameRateFlag = sequence.fixedFrameRateFlag ? 1 : 0;
    info.bitstreamRestrictionFlag = sequence.bitstreamRestrictionFlag ? 1 : 0;
    info.maxDecFrameBuffering = static_cast<std::uint8_t>(sequence.maxDecFrameBuffering);
    info.picStructPresentFlag = sequence.picStructPresentFlag ? 1 : 0;
    info.picStruct = unit.picStruct;
    info.fieldPicFlag = unit.fieldPicFlag ? 1 : 0;
    info.bottomFieldFlag = unit.bottomFieldFlag ? 1 : 0;
    info.sequenceParameterSetPresentFlag = unit.sequenceParameterSetPresent ? 1 : 0;
    info.pictureParameterSetPresentFlag = unit.pictureParameterSetPresent ? 1 : 0;
    info.auDelimiterPresentFlag = unit.auDelimiterPresent ? 1 : 0;
    info.endOfSequencePresentFlag = unit.endOfSequencePresent ? 1 : 0;
    info.endOfStreamPresentFlag = unit.endOfStreamPresent ? 1 : 0;
    info.fillerDataPresentFlag = unit.fillerDataPresent ? 1 : 0;
    info.pictureTimingSeiPresentFlag = unit.pictureTimingSeiPresent ? 1 : 0;
    info.bufferingPeriodSeiPresentFlag = unit.bufferingPeriodSeiPresent ? 1 : 0;
    info.constraintSet0Flag = sequence.constraintSetFlags[0];
    info.constraintSet1Flag = sequence.constraintSetFlags[1];
    info.constraintSet2Flag = sequence.constraintSetFlags[2];
    info.constraintSet3Flag = sequence.constraintSetFlags[3];
    info.constraintSet4Flag = sequence.constraintSetFlags[4];
    info.constraintSet5Flag = sequence.constraintSetFlags[5];
    return info;
}

struct FrameDeleter {
    void operator()(AVFrame* frame) const {
        av_frame_free(&frame);
    }
};
using FramePointer = std::unique_ptr<AVFrame, FrameDeleter>;

struct ContextDeleter {
    void operator()(AVCodecContext* context) const {
        avcodec_free_context(&context);
    }
};

struct PacketDeleter {
    void operator()(AVPacket* packet) const {
        av_packet_free(&packet);
    }
};

struct DecodedPicture {
    FramePointer frame;
    PictureRecord record;
    bool last = false;
};

struct CompletedInput {
    void* auData = nullptr;
    std::uint32_t outputFrameCount = 0;
};

struct Decoder {
    std::mutex mutex;
    void* computeQueue = nullptr;
    std::int32_t maxFrameWidth = 0;
    std::int32_t maxFrameHeight = 0;
    std::uint32_t inputQueueDepth = 0;
    std::unique_ptr<AVCodecContext, ContextDeleter> context;
    AvcSyntax syntax;
    std::map<std::int64_t, PictureRecord> inputs;
    std::int64_t nextKey = 0;
    std::deque<DecodedPicture> pictures;
    std::deque<CompletedInput> completed;
    std::optional<FrameBuffer> output;
    std::vector<std::uint8_t> pendingUnits;
    bool drained = false;
    bool finalized = false;
};

std::mutex registryMutex;
std::map<const void*, std::shared_ptr<Decoder>> decoders;
std::set<const void*> computeQueues;
std::mutex recordMutex;
std::map<const void*, PictureRecord> pictureRecords;

std::uint64_t AlignUp(std::uint64_t value, std::uint64_t alignment) {
    return (value + alignment - 1) / alignment * alignment;
}

std::uint64_t FrameBytes(std::uint64_t width, std::uint64_t height) {
    return AlignUp(width, PitchAlignment) * height * 3 / 2;
}

bool ConfigSize(std::uint64_t size) {
    return size == sizeof(DecoderConfigInfo) || size == DecoderConfigInfoV1Size;
}

void CheckConfig(const DecoderConfigInfo& config, const char* function) {
    if (config.resourceType != ResourceCompute) throw std::runtime_error(std::string(function) + ": resource type " + std::to_string(config.resourceType) + " is not implemented");
    if (config.codecType != CodecAvc) throw std::runtime_error(std::string(function) + ": codec type " + std::to_string(config.codecType) + " is not implemented; only AVC is");
    if (config.maxFrameWidth <= 0 || config.maxFrameHeight <= 0) throw std::runtime_error(std::string(function) + ": automatic or non-positive maximum frame dimensions are not implemented");
    if (config.decodePipelineDepth == 0) throw std::runtime_error(std::string(function) + ": input queue depth 0 is not implemented");
    if (config.extraConfigInfo != nullptr) throw std::runtime_error(std::string(function) + ": extra config info for AVC is not implemented");
    if (config.thisSize == sizeof(DecoderConfigInfo) && config.disableSyncDecode) throw std::runtime_error(std::string(function) + ": decoding without synchronization is not implemented");
}

std::shared_ptr<Decoder> FindDecoder(const void* handle, const char* function) {
    std::lock_guard guard(registryMutex);
    const auto found = decoders.find(handle);
    if (found == decoders.end()) throw std::runtime_error(std::string(function) + ": decoder instance was not created by sceVdecswCreateDecoder");
    return found->second;
}

void CheckDimensions(const Decoder& decoder, const SequenceParameterSet& sequence) {
    const std::uint64_t width = (static_cast<std::uint64_t>(sequence.picWidthInMbsMinus1) + 1) * 16;
    const std::uint64_t height = (static_cast<std::uint64_t>(sequence.picHeightInMapUnitsMinus1) + 1) * 16 * (sequence.frameMbsOnlyFlag ? 1 : 2);
    const std::uint64_t cropWidth = 2 * (static_cast<std::uint64_t>(sequence.frameCropLeftOffset) + sequence.frameCropRightOffset);
    const std::uint64_t cropHeight = 2 * (sequence.frameMbsOnlyFlag ? 1 : 2) * (static_cast<std::uint64_t>(sequence.frameCropTopOffset) + sequence.frameCropBottomOffset);
    if (cropWidth >= width || cropHeight >= height) throw std::runtime_error("libSceVdecsw: frame cropping exceeds the coded picture");
    if (width - cropWidth > static_cast<std::uint64_t>(decoder.maxFrameWidth) || height - cropHeight > static_cast<std::uint64_t>(decoder.maxFrameHeight)) {
        throw std::runtime_error("libSceVdecsw: picture of " + std::to_string(width - cropWidth) + "x" + std::to_string(height - cropHeight) + " exceeds the configured maximum frame dimensions");
    }
}

void OpenCodec(Decoder& decoder) {
    const AVCodec* codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (codec == nullptr) throw std::runtime_error("libSceVdecsw: the H.264 decoder is unavailable");
    decoder.context.reset(avcodec_alloc_context3(codec));
    if (!decoder.context) throw std::runtime_error("libSceVdecsw: cannot allocate a codec context");
    decoder.context->apply_cropping = 0;
    decoder.context->flags |= AV_CODEC_FLAG_OUTPUT_CORRUPT;
    decoder.context->err_recognition = AV_EF_EXPLODE;
    decoder.context->strict_std_compliance = FF_COMPLIANCE_STRICT;
    decoder.context->thread_count = 0;
    decoder.context->thread_type = FF_THREAD_SLICE;
    if (avcodec_open2(decoder.context.get(), codec, nullptr) < 0) throw std::runtime_error("libSceVdecsw: cannot open the H.264 decoder");
}

std::uint32_t Receive(Decoder& decoder) {
    std::uint32_t count = 0;
    for (;;) {
        FramePointer frame(av_frame_alloc());
        if (!frame) throw std::runtime_error("libSceVdecsw: cannot allocate a frame");
        const int result = avcodec_receive_frame(decoder.context.get(), frame.get());
        if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) return count;
        if (result < 0) throw std::runtime_error("libSceVdecsw: picture decoding failed; error frames are not implemented");
        if (frame->decode_error_flags != 0 || (frame->flags & AV_FRAME_FLAG_CORRUPT) != 0) throw std::runtime_error("libSceVdecsw: picture is damaged or lacks its reference pictures; error frames are not implemented");
        const auto found = decoder.inputs.find(frame->pts);
        if (found == decoder.inputs.end()) throw std::runtime_error("libSceVdecsw: decoded picture has no access unit");
        DecodedPicture picture;
        picture.frame = std::move(frame);
        picture.record = std::move(found->second);
        decoder.inputs.erase(found);
        decoder.pictures.push_back(std::move(picture));
        ++count;
    }
}

std::vector<std::uint8_t> ParameterSets(const std::vector<std::uint8_t>& units) {
    std::vector<std::uint8_t> kept;
    if (units.empty()) return kept;
    for (const auto& nal : SplitAnnexB(units.data(), units.size())) {
        const auto type = static_cast<unsigned>(nal.data[0] & 0x1fu);
        if (type != 7 && type != 8) continue;
        kept.insert(kept.end(), {0, 0, 0, 1});
        kept.insert(kept.end(), nal.data, nal.data + nal.size);
    }
    return kept;
}

void WriteNv12(const AVFrame& frame, std::uint8_t* target, std::uint64_t pitch) {
    const auto width = static_cast<std::size_t>(frame.width);
    const auto height = static_cast<std::size_t>(frame.height);
    for (std::size_t row = 0; row < height; ++row) std::memcpy(target + row * pitch, frame.data[0] + static_cast<std::ptrdiff_t>(row) * frame.linesize[0], width);
    std::uint8_t* chroma = target + pitch * height;
    for (std::size_t row = 0; row < height / 2; ++row) {
        const std::uint8_t* u = frame.data[1] + static_cast<std::ptrdiff_t>(row) * frame.linesize[1];
        const std::uint8_t* v = frame.data[2] + static_cast<std::ptrdiff_t>(row) * frame.linesize[2];
        std::uint8_t* uv = chroma + row * pitch;
        for (std::size_t column = 0; column < width / 2; ++column) {
            uv[column * 2] = u[column];
            uv[column * 2 + 1] = v[column];
        }
    }
}

}

extern "C" {

int APS5_VABI sceVdecswCreateDecoder(const DecoderConfigInfo* decoderConfigInfo, const DecoderMemoryInfo* decoderMemoryInfo, void** decoderInstance) {
    if (decoderConfigInfo == nullptr || decoderMemoryInfo == nullptr || decoderInstance == nullptr) return SCE_VDECSW_ERROR_ARGUMENT_POINTER;
    if (!ConfigSize(decoderConfigInfo->thisSize) || decoderMemoryInfo->thisSize != sizeof(DecoderMemoryInfo)) return SCE_VDECSW_ERROR_STRUCT_SIZE;
    CheckConfig(*decoderConfigInfo, __func__);
    auto decoder = std::make_shared<Decoder>();
    decoder->computeQueue = decoderConfigInfo->computeQueue;
    decoder->maxFrameWidth = decoderConfigInfo->maxFrameWidth;
    decoder->maxFrameHeight = decoderConfigInfo->maxFrameHeight;
    decoder->inputQueueDepth = decoderConfigInfo->decodePipelineDepth;
    OpenCodec(*decoder);
    std::lock_guard guard(registryMutex);
    if (computeQueues.count(decoder->computeQueue) == 0) throw std::runtime_error(std::string(__func__) + ": compute queue was not allocated by sceVdecswAllocateComputeQueue");
    decoders.emplace(decoder.get(), decoder);
    *decoderInstance = decoder.get();
    return SCE_OK;
}

int APS5_VABI sceVdecswDeleteDecoder(void* decoderInstance) {
    if (decoderInstance == nullptr) return SCE_VDECSW_ERROR_DECODER_INSTANCE;
    std::shared_ptr<Decoder> decoder;
    {
        std::lock_guard guard(registryMutex);
        const auto found = decoders.find(decoderInstance);
        if (found == decoders.end()) throw std::runtime_error(std::string(__func__) + ": decoder instance was not created by sceVdecswCreateDecoder");
        decoder = std::move(found->second);
        decoders.erase(found);
    }
    return SCE_OK;
}

int APS5_VABI sceVdecswQueryComputeMemoryInfo(ComputeMemoryInfo* computeMemInfo) {
    if (computeMemInfo == nullptr) return SCE_VDECSW_ERROR_ARGUMENT_POINTER;
    if (computeMemInfo->thisSize != sizeof(ComputeMemoryInfo)) return SCE_VDECSW_ERROR_STRUCT_SIZE;
    computeMemInfo->cpuGpuMemorySize = WorkMemoryBytes;
    computeMemInfo->cpuGpuMemory = nullptr;
    return SCE_OK;
}

int APS5_VABI sceVdecswFinalizeDecodeSequence(void* decoderInstance) {
    if (decoderInstance == nullptr) return SCE_VDECSW_ERROR_DECODER_INSTANCE;
    const auto decoder = FindDecoder(decoderInstance, __func__);
    std::lock_guard guard(decoder->mutex);
    const int result = avcodec_send_packet(decoder->context.get(), nullptr);
    if (result < 0 && result != AVERROR_EOF) throw std::runtime_error(std::string(__func__) + ": draining the decoder failed");
    Receive(*decoder);
    if (!decoder->inputs.empty()) throw std::runtime_error(std::string(__func__) + ": the decoder dropped " + std::to_string(decoder->inputs.size()) + " pictures without output; discarded frames are not implemented");
    if (!decoder->pictures.empty()) decoder->pictures.back().last = true;
    decoder->drained = true;
    decoder->finalized = true;
    return SCE_OK;
}

int APS5_VABI sceVdecswQueryDecoderMemoryInfo(const DecoderConfigInfo* decoderConfigInfo, DecoderMemoryInfo* decoderMemoryInfo) {
    if (decoderConfigInfo == nullptr || decoderMemoryInfo == nullptr) return SCE_VDECSW_ERROR_ARGUMENT_POINTER;
    if (!ConfigSize(decoderConfigInfo->thisSize) || decoderMemoryInfo->thisSize != sizeof(DecoderMemoryInfo)) return SCE_VDECSW_ERROR_STRUCT_SIZE;
    CheckConfig(*decoderConfigInfo, __func__);
    decoderMemoryInfo->cpuMemorySize = WorkMemoryBytes;
    decoderMemoryInfo->cpuMemory = nullptr;
    decoderMemoryInfo->gpuMemorySize = WorkMemoryBytes;
    decoderMemoryInfo->gpuMemory = nullptr;
    decoderMemoryInfo->cpuGpuMemorySize = WorkMemoryBytes;
    decoderMemoryInfo->cpuGpuMemory = nullptr;
    decoderMemoryInfo->maxFrameBufferSize = FrameBytes(AlignUp(static_cast<std::uint64_t>(decoderConfigInfo->maxFrameWidth), 16), AlignUp(static_cast<std::uint64_t>(decoderConfigInfo->maxFrameHeight), 32));
    decoderMemoryInfo->frameBufferAlignment = FrameBufferAlignment;
    return SCE_OK;
}

int APS5_VABI sceVdecswSetDecodeInput(void* decoderInstance, const InputData* inputData) {
    if (decoderInstance == nullptr) return SCE_VDECSW_ERROR_DECODER_INSTANCE;
    const auto decoder = FindDecoder(decoderInstance, __func__);
    if (inputData == nullptr) return SCE_VDECSW_ERROR_ARGUMENT_POINTER;
    if (inputData->thisSize != sizeof(InputData)) return SCE_VDECSW_ERROR_STRUCT_SIZE;
    if (inputData->auData == nullptr) return SCE_VDECSW_ERROR_ACCESS_UNIT_POINTER;
    if (inputData->auSize == 0) return SCE_VDECSW_ERROR_ACCESS_UNIT_SIZE;
    const auto* data = static_cast<const std::uint8_t*>(inputData->auData);
    const auto size = static_cast<std::size_t>(inputData->auSize);
    std::lock_guard guard(decoder->mutex);
    if (decoder->completed.size() >= decoder->inputQueueDepth) throw std::runtime_error(std::string(__func__) + ": input queue depth " + std::to_string(decoder->inputQueueDepth) + " is reached; access units beyond it without sceVdecswTrySyncDecodeInput are not implemented");
    if (decoder->pendingUnits.size() + inputData->auSize > static_cast<std::uint64_t>(std::numeric_limits<int>::max() - AV_INPUT_BUFFER_PADDING_SIZE)) throw std::runtime_error(std::string(__func__) + ": access unit is too large");
    const auto unit = decoder->syntax.Parse(data, size);
    if (unit.hasPicture) CheckDimensions(*decoder, unit.sequence);
    decoder->finalized = false;
    if (!unit.hasPicture) {
        decoder->pendingUnits.insert(decoder->pendingUnits.end(), data, data + size);
        decoder->completed.push_back({inputData->auData, 0});
        return SCE_OK;
    }
    if (decoder->drained) {
        avcodec_flush_buffers(decoder->context.get());
        decoder->drained = false;
    }
    std::unique_ptr<AVPacket, PacketDeleter> packet(av_packet_alloc());
    if (!packet || av_new_packet(packet.get(), static_cast<int>(decoder->pendingUnits.size() + size)) < 0) throw std::runtime_error(std::string(__func__) + ": cannot allocate a packet");
    std::copy(decoder->pendingUnits.begin(), decoder->pendingUnits.end(), packet->data);
    std::memcpy(packet->data + decoder->pendingUnits.size(), data, size);
    decoder->pendingUnits.clear();
    packet->pts = decoder->nextKey++;
    decoder->inputs[packet->pts] = PictureRecord{MakePictureInfo(unit, *inputData), unit.infoError};
    if (avcodec_send_packet(decoder->context.get(), packet.get()) < 0) throw std::runtime_error(std::string(__func__) + ": access unit decoding failed; decode errors are not implemented");
    const auto count = Receive(*decoder);
    decoder->completed.push_back({inputData->auData, count});
    return SCE_OK;
}

int APS5_VABI sceVdecswReleaseComputeQueue(void* computeQueue) {
    std::lock_guard guard(registryMutex);
    if (computeQueues.count(computeQueue) == 0) throw std::runtime_error(std::string(__func__) + ": compute queue was not allocated by sceVdecswAllocateComputeQueue");
    for (const auto& [handle, decoder] : decoders) {
        if (decoder->computeQueue == computeQueue) throw std::runtime_error(std::string(__func__) + ": compute queue is still used by a decoder");
    }
    computeQueues.erase(computeQueue);
    return SCE_OK;
}

int APS5_VABI sceVdecswAllocateComputeQueue(const ComputeConfigInfo* computeCfgInfo, const ComputeMemoryInfo* computeMemInfo, void** computeQueue) {
    if (computeCfgInfo == nullptr || computeMemInfo == nullptr || computeQueue == nullptr) return SCE_VDECSW_ERROR_ARGUMENT_POINTER;
    if (computeCfgInfo->thisSize != sizeof(ComputeConfigInfo) || computeMemInfo->thisSize != sizeof(ComputeMemoryInfo)) return SCE_VDECSW_ERROR_STRUCT_SIZE;
    if (computeCfgInfo->reserved0 != 0 || computeCfgInfo->reserved1 != 0) return SCE_VDECSW_ERROR_CONFIG_INFO;
    if (computeCfgInfo->computePipeId > MaxComputePipeId) return SCE_VDECSW_ERROR_COMPUTE_PIPE_ID;
    if (computeCfgInfo->computeQueueId > MaxComputeQueueId) return SCE_VDECSW_ERROR_COMPUTE_QUEUE_ID;
    if (computeMemInfo->cpuGpuMemory == nullptr) return SCE_VDECSW_ERROR_MEMORY_POINTER;
    std::lock_guard guard(registryMutex);
    if (!computeQueues.insert(computeMemInfo->cpuGpuMemory).second) throw std::runtime_error(std::string(__func__) + ": the memory already backs a compute queue");
    *computeQueue = computeMemInfo->cpuGpuMemory;
    return SCE_OK;
}

int APS5_VABI sceVdecswGetAvcPictureInfo(const OutputInfo* outputInfo, AvcPictureInfo* firstPictureInfo, AvcPictureInfo* secondPictureInfo) {
    static_cast<void>(secondPictureInfo);
    if (outputInfo == nullptr) return SCE_VDECSW_ERROR_ARGUMENT_POINTER;
    if ((outputInfo->thisSize | 8u) != sizeof(OutputInfo)) return SCE_VDECSW_ERROR_STRUCT_SIZE;
    if (outputInfo->pictureCount == 0) return SCE_OK;
    if (outputInfo->pictureCount > 1) throw std::runtime_error(std::string(__func__) + ": outputs with two field pictures are not implemented");
    if (firstPictureInfo == nullptr) return SCE_OK;
    if (firstPictureInfo->thisSize != sizeof(AvcPictureInfo)) throw std::runtime_error(std::string(__func__) + ": picture info size " + std::to_string(firstPictureInfo->thisSize) + " is not implemented");
    std::lock_guard guard(recordMutex);
    const auto found = pictureRecords.find(outputInfo->frameBuffer);
    if (found == pictureRecords.end()) throw std::runtime_error(std::string(__func__) + ": output info was not produced by sceVdecswTrySyncDecodeOutput");
    if (!found->second.error.empty()) throw std::runtime_error(std::string(__func__) + ": " + found->second.error);
    const auto size = firstPictureInfo->thisSize;
    *firstPictureInfo = found->second.info;
    firstPictureInfo->thisSize = size;
    return SCE_OK;
}

int APS5_VABI sceVdecswTrySyncDecodeOutput(void* decoderInstance, OutputInfo* outputInfo) {
    if (decoderInstance == nullptr) return SCE_VDECSW_ERROR_DECODER_INSTANCE;
    const auto decoder = FindDecoder(decoderInstance, __func__);
    if (outputInfo == nullptr) return SCE_VDECSW_ERROR_ARGUMENT_POINTER;
    if (outputInfo->thisSize != sizeof(OutputInfo) && outputInfo->thisSize != OutputInfoV1Size) throw std::runtime_error(std::string(__func__) + ": output info size " + std::to_string(outputInfo->thisSize) + " is not implemented");
    outputInfo->isValid = false;
    outputInfo->isLastFrame = false;
    outputInfo->isErrorFrame = false;
    outputInfo->isDiscardedFrame = false;
    outputInfo->pictureCount = 0;
    std::lock_guard guard(decoder->mutex);
    if (!decoder->output) return SCE_VDECSW_ERROR_OUTPUT_BUFFER_EMPTY;
    if (decoder->pictures.empty()) {
        if (!decoder->finalized) return SCE_VDECSW_ERROR_DECODE_PENDING;
        outputInfo->isLastFrame = true;
        decoder->output.reset();
        return SCE_OK;
    }
    auto& picture = decoder->pictures.front();
    const AVFrame& frame = *picture.frame;
    const auto format = static_cast<AVPixelFormat>(frame.format);
    if ((format != AV_PIX_FMT_YUV420P && format != AV_PIX_FMT_YUVJ420P) || frame.width <= 0 || frame.height <= 0 || frame.width % 2 != 0 || frame.height % 2 != 0) throw std::runtime_error(std::string(__func__) + ": decoded picture layout is not implemented");
    const auto width = static_cast<std::uint32_t>(frame.width);
    const auto height = static_cast<std::uint32_t>(frame.height);
    const auto pitch = static_cast<std::uint32_t>(AlignUp(width, PitchAlignment));
    const auto bytes = FrameBytes(width, height);
    if (bytes > decoder->output->frameBufferSize) throw std::runtime_error(std::string(__func__) + ": frame buffer of " + std::to_string(decoder->output->frameBufferSize) + " bytes cannot hold a " + std::to_string(bytes) + "-byte picture");
    WriteNv12(frame, static_cast<std::uint8_t*>(decoder->output->frameBuffer), pitch);
    outputInfo->isValid = true;
    outputInfo->isLastFrame = picture.last;
    outputInfo->pictureCount = 1;
    outputInfo->codecType = CodecAvc;
    outputInfo->frameWidth = width;
    outputInfo->framePitch = pitch;
    outputInfo->frameHeight = height;
    outputInfo->frameBuffer = decoder->output->frameBuffer;
    outputInfo->frameBufferSize = bytes;
    if (outputInfo->thisSize == sizeof(OutputInfo)) {
        outputInfo->frameFormat = FrameFormatNv12;
        outputInfo->framePitchInBytes = pitch;
    }
    {
        std::lock_guard recordGuard(recordMutex);
        pictureRecords[decoder->output->frameBuffer] = std::move(picture.record);
    }
    decoder->pictures.pop_front();
    decoder->output.reset();
    return SCE_OK;
}

int APS5_VABI sceVdecswTrySyncDecodeInput(void* decoderInstance, InputResult* inputResult) {
    if (decoderInstance == nullptr) return SCE_VDECSW_ERROR_DECODER_INSTANCE;
    const auto decoder = FindDecoder(decoderInstance, __func__);
    if (inputResult == nullptr) return SCE_VDECSW_ERROR_ARGUMENT_POINTER;
    if (inputResult->thisSize != sizeof(InputResult)) throw std::runtime_error(std::string(__func__) + ": input result size " + std::to_string(inputResult->thisSize) + " is not implemented");
    std::lock_guard guard(decoder->mutex);
    if (decoder->completed.empty()) return SCE_VDECSW_ERROR_INPUT_QUEUE_EMPTY;
    const auto completed = decoder->completed.front();
    decoder->completed.pop_front();
    inputResult->decodedAu = completed.auData;
    inputResult->outputFrameCount = completed.outputFrameCount;
    inputResult->reserved0 = 0;
    return SCE_OK;
}

int APS5_VABI sceVdecswSetDecodeOutput(void* decoderInstance, FrameBuffer* frameBuffer) {
    if (decoderInstance == nullptr) return SCE_VDECSW_ERROR_DECODER_INSTANCE;
    const auto decoder = FindDecoder(decoderInstance, __func__);
    if (frameBuffer == nullptr) return SCE_VDECSW_ERROR_ARGUMENT_POINTER;
    if (frameBuffer->thisSize != sizeof(FrameBuffer)) return SCE_VDECSW_ERROR_STRUCT_SIZE;
    if (frameBuffer->frameBuffer == nullptr) return SCE_VDECSW_ERROR_FRAME_BUFFER_POINTER;
    if (frameBuffer->frameBufferSize == 0) return SCE_VDECSW_ERROR_FRAME_BUFFER_SIZE;
    std::lock_guard guard(decoder->mutex);
    if (decoder->output) return SCE_VDECSW_ERROR_OUTPUT_BUFFER_FULL;
    decoder->output = *frameBuffer;
    return SCE_OK;
}

int APS5_VABI sceVdecswResetDecoder(void* decoderInstance) {
    if (decoderInstance == nullptr) return SCE_VDECSW_ERROR_DECODER_INSTANCE;
    const auto decoder = FindDecoder(decoderInstance, __func__);
    std::lock_guard guard(decoder->mutex);
    avcodec_flush_buffers(decoder->context.get());
    decoder->pendingUnits = ParameterSets(decoder->pendingUnits);
    decoder->inputs.clear();
    decoder->pictures.clear();
    decoder->completed.clear();
    decoder->output.reset();
    decoder->drained = false;
    decoder->finalized = false;
    return SCE_OK;
}

}
