#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>
#include "prx/libc/include/General.hpp"

struct ComputeMemoryInfo {
    std::uint64_t thisSize;
    std::uint64_t cpuGpuMemorySize;
    void* cpuGpuMemory;
};

struct ComputeConfigInfo {
    std::uint64_t thisSize;
    std::uint16_t computePipeId;
    std::uint16_t computeQueueId;
    bool checkMemoryType;
    std::uint8_t reserved0;
    std::uint16_t reserved1;
};

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

struct InputData {
    std::uint64_t thisSize;
    const void* auData;
    std::uint64_t auSize;
    std::uint64_t ptsData;
    std::uint64_t dtsData;
    std::uint64_t attachedData;
};

struct InputResult {
    std::uint64_t thisSize;
    const void* decodedAu;
    std::uint32_t outputFrameCount;
    std::uint32_t reserved0;
};

struct FrameBuffer {
    std::uint64_t thisSize;
    void* frameBuffer;
    std::uint64_t frameBufferSize;
};

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

static_assert(sizeof(DecoderConfigInfo) == 0x50);
static_assert(sizeof(OutputInfo) == 0x38);
static_assert(sizeof(AvcPictureInfo) == 0x78);

extern "C" {
int APS5_VABI sceVdecswQueryComputeMemoryInfo(ComputeMemoryInfo*);
int APS5_VABI sceVdecswAllocateComputeQueue(const ComputeConfigInfo*, const ComputeMemoryInfo*, void**);
int APS5_VABI sceVdecswReleaseComputeQueue(void*);
int APS5_VABI sceVdecswQueryDecoderMemoryInfo(const DecoderConfigInfo*, DecoderMemoryInfo*);
int APS5_VABI sceVdecswCreateDecoder(const DecoderConfigInfo*, const DecoderMemoryInfo*, void**);
int APS5_VABI sceVdecswDeleteDecoder(void*);
int APS5_VABI sceVdecswResetDecoder(void*);
int APS5_VABI sceVdecswSetDecodeInput(void*, const InputData*);
int APS5_VABI sceVdecswTrySyncDecodeInput(void*, InputResult*);
int APS5_VABI sceVdecswSetDecodeOutput(void*, FrameBuffer*);
int APS5_VABI sceVdecswTrySyncDecodeOutput(void*, OutputInfo*);
int APS5_VABI sceVdecswFinalizeDecodeSequence(void*);
int APS5_VABI sceVdecswGetAvcPictureInfo(const OutputInfo*, AvcPictureInfo*, AvcPictureInfo*);
}

namespace {

constexpr int StructSize = static_cast<int>(0x81510101);
constexpr int ArgumentPointer = static_cast<int>(0x81510102);
constexpr int DecoderInstance = static_cast<int>(0x81510103);
constexpr int MemoryPointer = static_cast<int>(0x81510105);
constexpr int FrameBufferSize = static_cast<int>(0x81510106);
constexpr int FrameBufferPointer = static_cast<int>(0x81510107);
constexpr int AccessUnitSize = static_cast<int>(0x8151010D);
constexpr int AccessUnitPointer = static_cast<int>(0x8151010E);
constexpr int OutputBufferFull = static_cast<int>(0x81510114);
constexpr int InputQueueEmpty = static_cast<int>(0x81510116);
constexpr int DecodePending = static_cast<int>(0x81510117);
constexpr int OutputBufferEmpty = static_cast<int>(0x81510118);
constexpr int ConfigInfo = static_cast<int>(0x81510200);
constexpr int ComputePipeId = static_cast<int>(0x81510201);
constexpr int ComputeQueueId = static_cast<int>(0x81510202);

constexpr std::uint32_t Width = 64;
constexpr std::uint32_t Height = 40;
constexpr std::uint32_t CodedHeight = 48;
constexpr std::uint32_t Pitch = 64;
constexpr std::uint64_t WorkMemoryBytes = 16ull << 20u;

constexpr std::array<std::uint8_t, 1477> Stream{
    0x00, 0x00, 0x00, 0x01, 0x09, 0x10, 0x00, 0x00, 0x00, 0x01, 0x67, 0x64, 0x00, 0x0a, 0xac, 0xd9,
    0x44, 0x7f, 0x97, 0x01, 0x10, 0x00, 0x00, 0x03, 0x00, 0x10, 0x00, 0x00, 0x03, 0x03, 0xc0, 0xf1,
    0x22, 0x59, 0x60, 0x00, 0x00, 0x00, 0x01, 0x68, 0xeb, 0xe3, 0xcb, 0x22, 0xc0, 0x00, 0x00, 0x01,
    0x06, 0x05, 0xff, 0xff, 0xa5, 0xdc, 0x45, 0xe9, 0xbd, 0xe6, 0xd9, 0x48, 0xb7, 0x96, 0x2c, 0xd8,
    0x20, 0xd9, 0x23, 0xee, 0xef, 0x78, 0x32, 0x36, 0x34, 0x20, 0x2d, 0x20, 0x63, 0x6f, 0x72, 0x65,
    0x20, 0x31, 0x36, 0x35, 0x20, 0x72, 0x33, 0x32, 0x32, 0x32, 0x20, 0x62, 0x33, 0x35, 0x36, 0x30,
    0x35, 0x61, 0x20, 0x2d, 0x20, 0x48, 0x2e, 0x32, 0x36, 0x34, 0x2f, 0x4d, 0x50, 0x45, 0x47, 0x2d,
    0x34, 0x20, 0x41, 0x56, 0x43, 0x20, 0x63, 0x6f, 0x64, 0x65, 0x63, 0x20, 0x2d, 0x20, 0x43, 0x6f,
    0x70, 0x79, 0x6c, 0x65, 0x66, 0x74, 0x20, 0x32, 0x30, 0x30, 0x33, 0x2d, 0x32, 0x30, 0x32, 0x35,
    0x20, 0x2d, 0x20, 0x68, 0x74, 0x74, 0x70, 0x3a, 0x2f, 0x2f, 0x77, 0x77, 0x77, 0x2e, 0x76, 0x69,
    0x64, 0x65, 0x6f, 0x6c, 0x61, 0x6e, 0x2e, 0x6f, 0x72, 0x67, 0x2f, 0x78, 0x32, 0x36, 0x34, 0x2e,
    0x68, 0x74, 0x6d, 0x6c, 0x20, 0x2d, 0x20, 0x6f, 0x70, 0x74, 0x69, 0x6f, 0x6e, 0x73, 0x3a, 0x20,
    0x63, 0x61, 0x62, 0x61, 0x63, 0x3d, 0x31, 0x20, 0x72, 0x65, 0x66, 0x3d, 0x33, 0x20, 0x64, 0x65,
    0x62, 0x6c, 0x6f, 0x63, 0x6b, 0x3d, 0x31, 0x3a, 0x30, 0x3a, 0x30, 0x20, 0x61, 0x6e, 0x61, 0x6c,
    0x79, 0x73, 0x65, 0x3d, 0x30, 0x78, 0x33, 0x3a, 0x30, 0x78, 0x31, 0x31, 0x33, 0x20, 0x6d, 0x65,
    0x3d, 0x68, 0x65, 0x78, 0x20, 0x73, 0x75, 0x62, 0x6d, 0x65, 0x3d, 0x37, 0x20, 0x70, 0x73, 0x79,
    0x3d, 0x31, 0x20, 0x70, 0x73, 0x79, 0x5f, 0x72, 0x64, 0x3d, 0x31, 0x2e, 0x30, 0x30, 0x3a, 0x30,
    0x2e, 0x30, 0x30, 0x20, 0x6d, 0x69, 0x78, 0x65, 0x64, 0x5f, 0x72, 0x65, 0x66, 0x3d, 0x31, 0x20,
    0x6d, 0x65, 0x5f, 0x72, 0x61, 0x6e, 0x67, 0x65, 0x3d, 0x31, 0x36, 0x20, 0x63, 0x68, 0x72, 0x6f,
    0x6d, 0x61, 0x5f, 0x6d, 0x65, 0x3d, 0x31, 0x20, 0x74, 0x72, 0x65, 0x6c, 0x6c, 0x69, 0x73, 0x3d,
    0x31, 0x20, 0x38, 0x78, 0x38, 0x64, 0x63, 0x74, 0x3d, 0x31, 0x20, 0x63, 0x71, 0x6d, 0x3d, 0x30,
    0x20, 0x64, 0x65, 0x61, 0x64, 0x7a, 0x6f, 0x6e, 0x65, 0x3d, 0x32, 0x31, 0x2c, 0x31, 0x31, 0x20,
    0x66, 0x61, 0x73, 0x74, 0x5f, 0x70, 0x73, 0x6b, 0x69, 0x70, 0x3d, 0x31, 0x20, 0x63, 0x68, 0x72,
    0x6f, 0x6d, 0x61, 0x5f, 0x71, 0x70, 0x5f, 0x6f, 0x66, 0x66, 0x73, 0x65, 0x74, 0x3d, 0x2d, 0x32,
    0x20, 0x74, 0x68, 0x72, 0x65, 0x61, 0x64, 0x73, 0x3d, 0x31, 0x20, 0x6c, 0x6f, 0x6f, 0x6b, 0x61,
    0x68, 0x65, 0x61, 0x64, 0x5f, 0x74, 0x68, 0x72, 0x65, 0x61, 0x64, 0x73, 0x3d, 0x31, 0x20, 0x73,
    0x6c, 0x69, 0x63, 0x65, 0x64, 0x5f, 0x74, 0x68, 0x72, 0x65, 0x61, 0x64, 0x73, 0x3d, 0x30, 0x20,
    0x6e, 0x72, 0x3d, 0x30, 0x20, 0x64, 0x65, 0x63, 0x69, 0x6d, 0x61, 0x74, 0x65, 0x3d, 0x31, 0x20,
    0x69, 0x6e, 0x74, 0x65, 0x72, 0x6c, 0x61, 0x63, 0x65, 0x64, 0x3d, 0x30, 0x20, 0x62, 0x6c, 0x75,
    0x72, 0x61, 0x79, 0x5f, 0x63, 0x6f, 0x6d, 0x70, 0x61, 0x74, 0x3d, 0x30, 0x20, 0x63, 0x6f, 0x6e,
    0x73, 0x74, 0x72, 0x61, 0x69, 0x6e, 0x65, 0x64, 0x5f, 0x69, 0x6e, 0x74, 0x72, 0x61, 0x3d, 0x30,
    0x20, 0x62, 0x66, 0x72, 0x61, 0x6d, 0x65, 0x73, 0x3d, 0x32, 0x20, 0x62, 0x5f, 0x70, 0x79, 0x72,
    0x61, 0x6d, 0x69, 0x64, 0x3d, 0x32, 0x20, 0x62, 0x5f, 0x61, 0x64, 0x61, 0x70, 0x74, 0x3d, 0x30,
    0x20, 0x62, 0x5f, 0x62, 0x69, 0x61, 0x73, 0x3d, 0x30, 0x20, 0x64, 0x69, 0x72, 0x65, 0x63, 0x74,
    0x3d, 0x31, 0x20, 0x77, 0x65, 0x69, 0x67, 0x68, 0x74, 0x62, 0x3d, 0x31, 0x20, 0x6f, 0x70, 0x65,
    0x6e, 0x5f, 0x67, 0x6f, 0x70, 0x3d, 0x30, 0x20, 0x77, 0x65, 0x69, 0x67, 0x68, 0x74, 0x70, 0x3d,
    0x32, 0x20, 0x6b, 0x65, 0x79, 0x69, 0x6e, 0x74, 0x3d, 0x36, 0x20, 0x6b, 0x65, 0x79, 0x69, 0x6e,
    0x74, 0x5f, 0x6d, 0x69, 0x6e, 0x3d, 0x31, 0x20, 0x73, 0x63, 0x65, 0x6e, 0x65, 0x63, 0x75, 0x74,
    0x3d, 0x30, 0x20, 0x69, 0x6e, 0x74, 0x72, 0x61, 0x5f, 0x72, 0x65, 0x66, 0x72, 0x65, 0x73, 0x68,
    0x3d, 0x30, 0x20, 0x72, 0x63, 0x5f, 0x6c, 0x6f, 0x6f, 0x6b, 0x61, 0x68, 0x65, 0x61, 0x64, 0x3d,
    0x36, 0x20, 0x72, 0x63, 0x3d, 0x63, 0x72, 0x66, 0x20, 0x6d, 0x62, 0x74, 0x72, 0x65, 0x65, 0x3d,
    0x31, 0x20, 0x63, 0x72, 0x66, 0x3d, 0x32, 0x33, 0x2e, 0x30, 0x20, 0x71, 0x63, 0x6f, 0x6d, 0x70,
    0x3d, 0x30, 0x2e, 0x36, 0x30, 0x20, 0x71, 0x70, 0x6d, 0x69, 0x6e, 0x3d, 0x30, 0x20, 0x71, 0x70,
    0x6d, 0x61, 0x78, 0x3d, 0x36, 0x39, 0x20, 0x71, 0x70, 0x73, 0x74, 0x65, 0x70, 0x3d, 0x34, 0x20,
    0x69, 0x70, 0x5f, 0x72, 0x61, 0x74, 0x69, 0x6f, 0x3d, 0x31, 0x2e, 0x34, 0x30, 0x20, 0x61, 0x71,
    0x3d, 0x31, 0x3a, 0x31, 0x2e, 0x30, 0x30, 0x00, 0x80, 0x00, 0x00, 0x01, 0x65, 0x88, 0x84, 0x00,
    0x21, 0xff, 0x5b, 0xff, 0x78, 0x50, 0x63, 0x8c, 0x32, 0x61, 0x08, 0xb9, 0xb3, 0xab, 0x26, 0xb5,
    0xe1, 0xe9, 0x5b, 0xb7, 0x7b, 0xfd, 0x6b, 0x41, 0x3a, 0x52, 0x12, 0x59, 0x95, 0x0c, 0xa4, 0x3b,
    0xc7, 0x28, 0x9a, 0x87, 0xd9, 0x38, 0xc9, 0xcb, 0x8f, 0x05, 0x47, 0xff, 0xd1, 0x1e, 0xd3, 0x68,
    0xb5, 0x05, 0x8c, 0xb4, 0xf3, 0x18, 0x74, 0x33, 0x57, 0x68, 0xf6, 0xf0, 0xc7, 0xcb, 0xbf, 0x7b,
    0x3d, 0x43, 0x0f, 0xa7, 0x42, 0x5a, 0x78, 0x5b, 0x61, 0x0e, 0xc6, 0x5c, 0xb2, 0xdf, 0xa0, 0xe0,
    0x96, 0x11, 0xdd, 0x02, 0xec, 0x17, 0x80, 0x72, 0x4f, 0x35, 0x05, 0x1e, 0x8b, 0xa8, 0xfc, 0x38,
    0x28, 0xa8, 0xc7, 0x19, 0x5a, 0xa0, 0x8b, 0x96, 0x47, 0x6c, 0xf3, 0x37, 0x65, 0x02, 0x13, 0x8a,
    0x97, 0xbc, 0x86, 0xf8, 0x31, 0xc2, 0xab, 0x7f, 0xe1, 0xa4, 0x68, 0x21, 0xc5, 0xe1, 0x56, 0xca,
    0x96, 0xd7, 0xa3, 0x7f, 0x77, 0x7b, 0xed, 0x28, 0x08, 0xd4, 0xe3, 0x98, 0xdc, 0xc1, 0x2f, 0x0f,
    0x0c, 0x37, 0x06, 0x36, 0xd4, 0x10, 0xdb, 0x7e, 0x99, 0xd4, 0xb1, 0x67, 0xda, 0xbd, 0x28, 0x58,
    0xe6, 0x0e, 0x70, 0x33, 0xe4, 0xb0, 0xb8, 0xd2, 0xb4, 0x60, 0x16, 0x41, 0x1d, 0xda, 0xc5, 0x47,
    0xad, 0xd3, 0x55, 0x30, 0xdf, 0xca, 0xab, 0xb9, 0x91, 0xca, 0xca, 0xe6, 0xfe, 0xb4, 0x9e, 0x55,
    0x18, 0xeb, 0x74, 0x83, 0x01, 0x95, 0xb3, 0x0c, 0x92, 0x4d, 0xb8, 0xfd, 0xa6, 0x28, 0x77, 0xdb,
    0xb9, 0xbc, 0xf2, 0x2c, 0xde, 0xe2, 0x1f, 0x55, 0x39, 0x80, 0x05, 0x67, 0xf1, 0x8c, 0x46, 0xf3,
    0xf8, 0x61, 0x00, 0x00, 0x00, 0x01, 0x09, 0x30, 0x00, 0x00, 0x01, 0x41, 0x9a, 0x23, 0x6c, 0x47,
    0x7f, 0xd6, 0x70, 0xed, 0x07, 0x2d, 0x6f, 0xf0, 0xd9, 0x1c, 0xc5, 0x57, 0x33, 0x9c, 0x0b, 0xe3,
    0x29, 0x4d, 0xc8, 0x18, 0xba, 0x21, 0x49, 0x77, 0xb9, 0xc5, 0x9c, 0xab, 0xcb, 0xca, 0xe9, 0xab,
    0xf7, 0xad, 0xfb, 0x45, 0xda, 0x68, 0x98, 0x7e, 0xff, 0xbd, 0x7e, 0x91, 0xb8, 0x93, 0xb5, 0x78,
    0x93, 0x98, 0x6e, 0x6e, 0xdf, 0x1f, 0x8f, 0x3c, 0x4e, 0xc9, 0x78, 0x17, 0xbf, 0xf4, 0xfb, 0x47,
    0x64, 0x2f, 0xb3, 0xf8, 0xb6, 0xdc, 0x1a, 0xe9, 0xb7, 0x7c, 0xfc, 0x6a, 0x5d, 0xb5, 0x95, 0xb6,
    0x47, 0x6b, 0xfe, 0x5d, 0xf6, 0x8e, 0x6e, 0x85, 0x65, 0x2a, 0x2d, 0x05, 0x21, 0x45, 0x13, 0x0d,
    0x0c, 0xf3, 0xea, 0x6f, 0x58, 0x28, 0x7a, 0x66, 0x54, 0xfd, 0x7e, 0x27, 0xc6, 0x6a, 0xf0, 0x00,
    0x00, 0x00, 0x01, 0x09, 0x50, 0x00, 0x00, 0x01, 0x41, 0x9e, 0x41, 0x78, 0x84, 0x3f, 0xe3, 0x5a,
    0x3c, 0x1e, 0x34, 0x44, 0x68, 0x14, 0xb9, 0xbc, 0x32, 0x4a, 0xc3, 0x1b, 0x54, 0xd4, 0x00, 0x3c,
    0x17, 0xa1, 0xbd, 0xc0, 0x86, 0xe1, 0xbf, 0x7d, 0xf4, 0x08, 0x3c, 0xa3, 0xeb, 0xe0, 0xfd, 0x44,
    0x09, 0x7d, 0x95, 0x20, 0xf4, 0x50, 0x4b, 0x4e, 0xf3, 0x78, 0xa5, 0x9f, 0x14, 0x23, 0x7f, 0xc1,
    0x4e, 0x94, 0xf1, 0x1b, 0xa1, 0x94, 0x86, 0x6d, 0x14, 0x53, 0xa8, 0xe6, 0xa4, 0xd4, 0x28, 0xcd,
    0x86, 0xb1, 0xcb, 0x9f, 0x67, 0x72, 0xc5, 0xf8, 0x99, 0x9a, 0xf3, 0xba, 0x47, 0xdf, 0x41, 0x00,
    0x00, 0x00, 0x01, 0x09, 0x50, 0x00, 0x00, 0x01, 0x01, 0x9e, 0x62, 0x6a, 0x42, 0x5f, 0xdf, 0xac,
    0xe9, 0x76, 0x6c, 0x13, 0x5c, 0xa1, 0x08, 0xbf, 0xd4, 0xd8, 0x03, 0xef, 0xd2, 0x0d, 0xc1, 0xde,
    0x35, 0x3b, 0x9e, 0x3d, 0xe1, 0xf6, 0xd8, 0xe7, 0xae, 0xdf, 0x9f, 0x20, 0x19, 0xbc, 0x50, 0xa9,
    0x69, 0x82, 0x26, 0x04, 0xaf, 0x83, 0x44, 0x79, 0x1b, 0x74, 0x18, 0x2b, 0x00, 0xb8, 0x62, 0xa3,
    0xd1, 0x84, 0x8b, 0x61, 0x87, 0x1a, 0x4f, 0x5b, 0xef, 0x16, 0xeb, 0xb3, 0x1a, 0x06, 0xfd, 0x92,
    0x87, 0x06, 0xc3, 0xec, 0xda, 0xde, 0xec, 0x3d, 0x7d, 0x8a, 0x06, 0x91, 0xc8, 0xdf, 0x59, 0xa7,
    0x58, 0xef, 0x7a, 0x3c, 0x27, 0x1d, 0x00, 0x00, 0x00, 0x01, 0x09, 0x30, 0x00, 0x00, 0x01, 0x41,
    0x9a, 0x65, 0x49, 0xa8, 0x41, 0x68, 0x99, 0x4c, 0x14, 0xf0, 0xc7, 0xde, 0x88, 0x44, 0x37, 0xa7,
    0x7f, 0x10, 0x1e, 0xf9, 0x41, 0xb5, 0x7c, 0x9b, 0xf7, 0x1d, 0x61, 0x0e, 0xe1, 0xec, 0xbc, 0x75,
    0x08, 0x53, 0x8f, 0x68, 0x84, 0x84, 0x93, 0x11, 0x08, 0xf6, 0xb2, 0xe5, 0x6f, 0x30, 0x16, 0xd0,
    0x9d, 0xf2, 0x87, 0x44, 0x40, 0x82, 0x85, 0xcf, 0x16, 0x73, 0x64, 0x1f, 0xb0, 0x22, 0x75, 0xc8,
    0xd7, 0xd2, 0x61, 0xdc, 0x1e, 0x9c, 0xa0, 0xeb, 0x88, 0x7d, 0x1a, 0x55, 0xe4, 0x66, 0x49, 0x5e,
    0xa8, 0xff, 0xc2, 0xb5, 0xb0, 0x46, 0x2a, 0xb5, 0xcd, 0xed, 0x7c, 0xf9, 0xd8, 0xe7, 0x00, 0x00,
    0x00, 0x01, 0x09, 0x50, 0x00, 0x00, 0x01, 0x01, 0x9e, 0x84, 0x6a, 0x42, 0x5f, 0xf9, 0x0f, 0xf0,
    0x52, 0xca, 0xb7, 0x04, 0x3a, 0xd3, 0x0a, 0xb3, 0xaa, 0x47, 0xf0, 0x8c, 0x9f, 0x4b, 0x6f, 0x0e,
    0x38, 0x62, 0x53, 0x04, 0x7d, 0x8f, 0x80, 0xca, 0x62, 0x93, 0xfd, 0xbb, 0x97, 0xef, 0x71, 0xf8,
    0xbc, 0x20, 0x23, 0xd1, 0x9d, 0x50, 0xee, 0xd4, 0x9a, 0x13, 0xb6, 0xfb, 0x78, 0x71, 0x43, 0x78,
    0x65, 0xd6, 0x24, 0xe4, 0xca, 0x6f, 0xb4, 0x42, 0x4d, 0x64, 0xd5, 0xfc, 0x12, 0x29, 0x20, 0x52,
    0xd8, 0x82, 0x78, 0x3d, 0x2f,
};

constexpr std::array<std::uint64_t, 6> PictureHashes{
    0x5871fdc85dfa274eull, 0x26ce4afb974cffc4ull, 0xd2e739633733d5baull,
    0x54feb3d290e028fdull, 0xfa1342a4ba1819c8ull, 0x870125dd7f079a4cull,
};

constexpr std::array<std::uint64_t, 6> DisplayOrderUnits{0, 2, 3, 1, 5, 4};

constexpr std::array<std::uint8_t, 26> FieldPictureUnit{
    0x00, 0x00, 0x00, 0x01, 0x67, 0x4d, 0x00, 0x1e, 0xda, 0x11, 0x92, 0x00, 0x00, 0x00, 0x01, 0x68,
    0xce, 0x38, 0x80, 0x00, 0x00, 0x00, 0x01, 0x65, 0x88, 0x85,
};

constexpr std::array<std::uint8_t, 6> ExtensionUnit{0x00, 0x00, 0x00, 0x01, 0x74, 0x80};

void check(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

template <typename TAction>
void checkThrows(TAction action, const std::string& reason) {
    bool threw = false;
    try {
        action();
    } catch (const std::runtime_error& error) {
        threw = std::string(error.what()).find(reason) != std::string::npos;
    }
    check(threw, "expected an exception about: " + reason);
}

std::uint64_t hashNv12(const std::uint8_t* buffer) {
    std::uint64_t hash = 0xcbf29ce484222325ull;
    const auto mix = [&](const std::uint8_t* row) {
        for (std::uint32_t column = 0; column < Width; ++column) hash = (hash ^ row[column]) * 0x100000001b3ull;
    };
    for (std::uint32_t row = 0; row < Height; ++row) mix(buffer + static_cast<std::size_t>(row) * Pitch);
    const auto* chroma = buffer + static_cast<std::size_t>(Pitch) * CodedHeight;
    for (std::uint32_t row = 0; row < Height / 2; ++row) mix(chroma + static_cast<std::size_t>(row) * Pitch);
    return hash;
}

std::vector<std::vector<std::uint8_t>> accessUnits() {
    std::vector<std::size_t> starts;
    for (std::size_t index = 0; index + 4 < Stream.size(); ++index) {
        if (Stream[index] == 0 && Stream[index + 1] == 0 && Stream[index + 2] == 0 && Stream[index + 3] == 1 && (Stream[index + 4] & 0x1fu) == 9) starts.push_back(index);
    }
    starts.push_back(Stream.size());
    std::vector<std::vector<std::uint8_t>> units;
    for (std::size_t unit = 0; unit + 1 < starts.size(); ++unit) units.emplace_back(Stream.begin() + static_cast<std::ptrdiff_t>(starts[unit]), Stream.begin() + static_cast<std::ptrdiff_t>(starts[unit + 1]));
    return units;
}

std::size_t findNal(const std::vector<std::uint8_t>& unit, std::uint8_t header) {
    for (std::size_t index = 0; index + 3 < unit.size(); ++index) {
        if (unit[index] == 0 && unit[index + 1] == 0 && unit[index + 2] == 1 && unit[index + 3] == header) return index;
    }
    throw std::runtime_error("NAL unit missing from the test stream");
}

struct Queue {
    std::vector<std::uint8_t> memory;
    void* handle = nullptr;

    Queue() {
        ComputeMemoryInfo info{sizeof(ComputeMemoryInfo), 0, nullptr};
        check(sceVdecswQueryComputeMemoryInfo(&info) == 0, "compute memory query failed");
        memory.resize(info.cpuGpuMemorySize);
        info.cpuGpuMemory = memory.data();
        const ComputeConfigInfo config{sizeof(ComputeConfigInfo), 0, 0, false, 0, 0};
        check(sceVdecswAllocateComputeQueue(&config, &info, &handle) == 0, "compute queue allocation failed");
    }

    ~Queue() {
        try {
            sceVdecswReleaseComputeQueue(handle);
        } catch (const std::runtime_error& error) {
            std::fprintf(stderr, "%s\n", error.what());
        }
    }
};

DecoderConfigInfo makeConfig(void* queue) {
    return DecoderConfigInfo{sizeof(DecoderConfigInfo), 1, 1, 100, 40, static_cast<std::int32_t>(Width), static_cast<std::int32_t>(Height), 4, 3, queue, 0, -1, true, false, 0, 1, nullptr, false, 0};
}

void* createDecoder(const DecoderConfigInfo& config) {
    DecoderMemoryInfo memory{sizeof(DecoderMemoryInfo)};
    check(sceVdecswQueryDecoderMemoryInfo(&config, &memory) == 0, "decoder memory query failed");
    void* decoder = nullptr;
    check(sceVdecswCreateDecoder(&config, &memory, &decoder) == 0 && decoder != nullptr, "decoder creation failed");
    return decoder;
}

void testComputeQueue() {
    check(sceVdecswQueryComputeMemoryInfo(nullptr) == ArgumentPointer, "null compute memory info accepted");
    ComputeMemoryInfo memory{8, 0, nullptr};
    check(sceVdecswQueryComputeMemoryInfo(&memory) == StructSize, "wrong compute memory info size accepted");
    memory.thisSize = sizeof(ComputeMemoryInfo);
    check(sceVdecswQueryComputeMemoryInfo(&memory) == 0 && memory.cpuGpuMemorySize == WorkMemoryBytes && memory.cpuGpuMemory == nullptr, "compute memory query mismatch");
    std::vector<std::uint8_t> backing(memory.cpuGpuMemorySize);
    memory.cpuGpuMemory = backing.data();
    ComputeConfigInfo config{sizeof(ComputeConfigInfo), 4, 7, true, 0, 0};
    void* queue = nullptr;
    check(sceVdecswAllocateComputeQueue(nullptr, &memory, &queue) == ArgumentPointer, "null compute config accepted");
    check(sceVdecswAllocateComputeQueue(&config, nullptr, &queue) == ArgumentPointer, "null compute memory accepted");
    check(sceVdecswAllocateComputeQueue(&config, &memory, nullptr) == ArgumentPointer, "null compute queue output accepted");
    config.thisSize = 8;
    check(sceVdecswAllocateComputeQueue(&config, &memory, &queue) == StructSize, "wrong compute config size accepted");
    config.thisSize = sizeof(ComputeConfigInfo);
    config.reserved1 = 1;
    check(sceVdecswAllocateComputeQueue(&config, &memory, &queue) == ConfigInfo, "reserved compute config field accepted");
    config.reserved1 = 0;
    config.computePipeId = 5;
    check(sceVdecswAllocateComputeQueue(&config, &memory, &queue) == ComputePipeId, "invalid compute pipe accepted");
    config.computePipeId = 4;
    config.computeQueueId = 8;
    check(sceVdecswAllocateComputeQueue(&config, &memory, &queue) == ComputeQueueId, "invalid compute queue id accepted");
    config.computeQueueId = 7;
    memory.cpuGpuMemory = nullptr;
    check(sceVdecswAllocateComputeQueue(&config, &memory, &queue) == MemoryPointer, "null compute memory pointer accepted");
    memory.cpuGpuMemory = backing.data();
    check(queue == nullptr, "failed allocation wrote a queue");
    check(sceVdecswAllocateComputeQueue(&config, &memory, &queue) == 0 && queue == backing.data(), "compute queue allocation failed");
    void* again = nullptr;
    checkThrows([&] { sceVdecswAllocateComputeQueue(&config, &memory, &again); }, "already backs a compute queue");
    checkThrows([&] { sceVdecswReleaseComputeQueue(nullptr); }, "was not allocated");
    checkThrows([&] { sceVdecswReleaseComputeQueue(backing.data() + 1); }, "was not allocated");
    check(sceVdecswReleaseComputeQueue(queue) == 0, "compute queue release failed");
    checkThrows([&] { sceVdecswReleaseComputeQueue(queue); }, "was not allocated");
}

void testDecoderLifetime() {
    Queue queue;
    auto config = makeConfig(queue.handle);
    DecoderMemoryInfo memory{sizeof(DecoderMemoryInfo)};
    check(sceVdecswQueryDecoderMemoryInfo(nullptr, &memory) == ArgumentPointer, "null decoder config accepted");
    check(sceVdecswQueryDecoderMemoryInfo(&config, nullptr) == ArgumentPointer, "null decoder memory accepted");
    config.thisSize = 0x40;
    check(sceVdecswQueryDecoderMemoryInfo(&config, &memory) == StructSize, "wrong decoder config size accepted");
    config.thisSize = sizeof(DecoderConfigInfo);
    memory.thisSize = 0x40;
    check(sceVdecswQueryDecoderMemoryInfo(&config, &memory) == StructSize, "wrong decoder memory size accepted");
    memory.thisSize = sizeof(DecoderMemoryInfo);
    config.codecType = 974921;
    checkThrows([&] { sceVdecswQueryDecoderMemoryInfo(&config, &memory); }, "only AVC is");
    config.codecType = 1;
    config.resourceType = 0;
    checkThrows([&] { sceVdecswQueryDecoderMemoryInfo(&config, &memory); }, "resource type 0");
    config.resourceType = 1;
    config.maxFrameWidth = -1;
    checkThrows([&] { sceVdecswQueryDecoderMemoryInfo(&config, &memory); }, "frame dimensions");
    config.maxFrameWidth = static_cast<std::int32_t>(Width);
    int extra = 0;
    config.extraConfigInfo = &extra;
    checkThrows([&] { sceVdecswQueryDecoderMemoryInfo(&config, &memory); }, "extra config info");
    config.extraConfigInfo = nullptr;
    config.decodePipelineDepth = 0;
    checkThrows([&] { sceVdecswQueryDecoderMemoryInfo(&config, &memory); }, "input queue depth 0");
    config.decodePipelineDepth = 3;
    config.disableSyncDecode = true;
    checkThrows([&] { sceVdecswQueryDecoderMemoryInfo(&config, &memory); }, "without synchronization");
    config.thisSize = 0x48;
    check(sceVdecswQueryDecoderMemoryInfo(&config, &memory) == 0, "version 1 decoder config rejected");
    config.thisSize = sizeof(DecoderConfigInfo);
    config.disableSyncDecode = false;
    check(sceVdecswQueryDecoderMemoryInfo(&config, &memory) == 0, "decoder memory query failed");
    check(memory.cpuMemorySize == WorkMemoryBytes && memory.gpuMemorySize == WorkMemoryBytes && memory.cpuGpuMemorySize == WorkMemoryBytes, "decoder work memory sizes mismatch");
    check(memory.cpuMemory == nullptr && memory.gpuMemory == nullptr && memory.cpuGpuMemory == nullptr, "decoder memory pointers were not cleared");
    check(memory.maxFrameBufferSize == Pitch * 64 * 3 / 2 && memory.frameBufferAlignment == 0x100, "frame buffer requirements mismatch");
    void* decoder = nullptr;
    check(sceVdecswCreateDecoder(nullptr, &memory, &decoder) == ArgumentPointer, "null create config accepted");
    check(sceVdecswCreateDecoder(&config, nullptr, &decoder) == ArgumentPointer, "null create memory accepted");
    check(sceVdecswCreateDecoder(&config, &memory, nullptr) == ArgumentPointer, "null decoder output accepted");
    memory.thisSize = 8;
    check(sceVdecswCreateDecoder(&config, &memory, &decoder) == StructSize, "wrong create memory size accepted");
    memory.thisSize = sizeof(DecoderMemoryInfo);
    config.computeQueue = &extra;
    checkThrows([&] { sceVdecswCreateDecoder(&config, &memory, &decoder); }, "compute queue was not allocated");
    check(decoder == nullptr, "failed creation wrote a decoder");
    config.computeQueue = queue.handle;
    config.thisSize = 0x48;
    check(sceVdecswCreateDecoder(&config, &memory, &decoder) == 0 && decoder != nullptr, "decoder creation failed");
    checkThrows([&] { sceVdecswReleaseComputeQueue(queue.handle); }, "still used by a decoder");
    check(sceVdecswDeleteDecoder(nullptr) == DecoderInstance, "null decoder deleted");
    checkThrows([&] { sceVdecswDeleteDecoder(&extra); }, "was not created");
    check(sceVdecswResetDecoder(nullptr) == DecoderInstance && sceVdecswFinalizeDecodeSequence(nullptr) == DecoderInstance, "null decoder accepted");
    checkThrows([&] { sceVdecswResetDecoder(&extra); }, "was not created");
    check(sceVdecswDeleteDecoder(decoder) == 0, "decoder deletion failed");
    checkThrows([&] { sceVdecswDeleteDecoder(decoder); }, "was not created");
}

struct Session {
    void* decoder;
    std::vector<std::uint8_t> buffer;
    bool offered = false;
    std::size_t pictures = 0;

    void offer() {
        if (offered) return;
        FrameBuffer frame{sizeof(FrameBuffer), buffer.data(), buffer.size()};
        check(sceVdecswSetDecodeOutput(decoder, &frame) == 0, "frame buffer was not accepted");
        offered = true;
    }

    void take(const OutputInfo& output, std::size_t base) {
        check(output.isValid && output.pictureCount == 1 && !output.isErrorFrame && !output.isDiscardedFrame && output.codecType == 1, "picture flags mismatch");
        check(output.frameWidth == Width && output.frameHeight == CodedHeight && output.framePitch == Pitch && output.frameBuffer == buffer.data(), "picture geometry mismatch");
        check(output.frameBufferSize == Pitch * CodedHeight * 3 / 2 && output.frameFormat == 0 && output.framePitchInBytes == Pitch, "picture layout mismatch");
        check(hashNv12(buffer.data()) == PictureHashes[pictures - base], "picture " + std::to_string(pictures) + " differs from the reference");
        AvcPictureInfo info{sizeof(AvcPictureInfo)};
        check(sceVdecswGetAvcPictureInfo(&output, &info, nullptr) == 0 && info.isValid && info.thisSize == sizeof(AvcPictureInfo), "picture info missing");
        const auto unit = DisplayOrderUnits[pictures - base];
        check(info.ptsData == 1000 + unit && info.dtsData == unit && info.attachedData == 0xa0 + unit, "picture info out of display order");
        check(info.idrPictureFlag == (unit == 0 ? 1 : 0) && info.sequenceParameterSetPresentFlag == (unit == 0 ? 1 : 0) && info.pictureParameterSetPresentFlag == (unit == 0 ? 1 : 0), "picture parameter flags mismatch");
        check(info.profileIdc == 100 && info.levelIdc == 10 && info.picWidthInMbsMinus1 == 3 && info.picHeightInMapUnitsMinus1 == 2 && info.frameMbsOnlyFlag == 1, "sequence geometry mismatch");
        check(info.frameCroppingFlag == 1 && info.frameCropLeftOffset == 0 && info.frameCropRightOffset == 0 && info.frameCropTopOffset == 0 && info.frameCropBottomOffset == 4, "cropping mismatch");
        check(info.aspectRatioInfoPresentFlag == 1 && info.aspectRatioIdc == 1 && info.sarWidth == 0 && info.sarHeight == 0, "aspect ratio mismatch");
        check(info.videoSignalTypePresentFlag == 0 && info.videoFormat == 5 && info.videoFullRangeFlag == 0 && info.colourDescriptionPresentFlag == 0, "video signal type mismatch");
        check(info.colourPrimaries == 2 && info.transferCharacteristics == 2 && info.matrixCoefficients == 2, "colour description mismatch");
        check(info.timingInfoPresentFlag == 1 && info.numUnitsInTick == 1 && info.timeScale == 60 && info.fixedFrameRateFlag == 0, "timing mismatch");
        check(info.bitstreamRestrictionFlag == 1 && info.maxDecFrameBuffering == 4 && info.picStructPresentFlag == 0 && info.picStruct == 0, "restriction mismatch");
        check(info.fieldPicFlag == 0 && info.bottomFieldFlag == 0 && info.auDelimiterPresentFlag == 1 && info.endOfSequencePresentFlag == 0 && info.endOfStreamPresentFlag == 0, "picture structure mismatch");
        check(info.fillerDataPresentFlag == 0 && info.pictureTimingSeiPresentFlag == 0 && info.bufferingPeriodSeiPresentFlag == 0, "SEI flags mismatch");
        check(info.constraintSet0Flag == 0 && info.constraintSet1Flag == 0 && info.constraintSet2Flag == 0 && info.constraintSet3Flag == 0 && info.constraintSet4Flag == 0 && info.constraintSet5Flag == 0, "constraint flags mismatch");
        offered = false;
        ++pictures;
    }

    void feed(const std::vector<std::vector<std::uint8_t>>& units, std::size_t base) {
        for (std::size_t unit = 0; unit < units.size(); ++unit) {
            const InputData input{sizeof(InputData), units[unit].data(), units[unit].size(), 1000 + unit, unit, 0xa0 + unit};
            check(sceVdecswSetDecodeInput(decoder, &input) == 0, "access unit was not accepted");
            InputResult result{sizeof(InputResult), nullptr, 0, 0xffffffffu};
            check(sceVdecswTrySyncDecodeInput(decoder, &result) == 0 && result.decodedAu == units[unit].data() && result.reserved0 == 0, "input result mismatch");
            check(sceVdecswTrySyncDecodeInput(decoder, &result) == InputQueueEmpty, "input queue holds a stale result");
            for (std::uint32_t frame = 0; frame < result.outputFrameCount; ++frame) {
                offer();
                OutputInfo output{sizeof(OutputInfo)};
                check(sceVdecswTrySyncDecodeOutput(decoder, &output) == 0 && !output.isLastFrame, "announced picture missing");
                take(output, base);
            }
            offer();
            OutputInfo output{sizeof(OutputInfo)};
            output.isValid = true;
            check(sceVdecswTrySyncDecodeOutput(decoder, &output) == DecodePending && !output.isValid, "picture produced without input");
            FrameBuffer second{sizeof(FrameBuffer), buffer.data(), buffer.size()};
            check(sceVdecswSetDecodeOutput(decoder, &second) == OutputBufferFull, "second frame buffer accepted");
        }
    }

    void finish(std::size_t base) {
        check(sceVdecswFinalizeDecodeSequence(decoder) == 0, "finalize failed");
        for (;;) {
            offer();
            OutputInfo output{sizeof(OutputInfo)};
            check(sceVdecswTrySyncDecodeOutput(decoder, &output) == 0, "draining output failed");
            if (!output.isValid) {
                check(output.isLastFrame && output.pictureCount == 0, "drained output is not the end of the sequence");
                offered = false;
                break;
            }
            const bool last = output.isLastFrame;
            take(output, base);
            check(last == (pictures == base + PictureHashes.size()), "last frame flag mismatch");
        }
        check(pictures == base + PictureHashes.size(), "missing pictures after finalize");
    }
};

void testDecode() {
    Queue queue;
    const auto config = makeConfig(queue.handle);
    DecoderMemoryInfo memory{sizeof(DecoderMemoryInfo)};
    check(sceVdecswQueryDecoderMemoryInfo(&config, &memory) == 0, "decoder memory query failed");
    Session session{createDecoder(config), std::vector<std::uint8_t>(memory.maxFrameBufferSize)};
    const auto units = accessUnits();
    check(units.size() == PictureHashes.size(), "unexpected access unit count");
    InputResult result{sizeof(InputResult)};
    check(sceVdecswTrySyncDecodeInput(session.decoder, &result) == InputQueueEmpty, "empty input queue reported a result");
    OutputInfo output{sizeof(OutputInfo)};
    check(sceVdecswTrySyncDecodeOutput(session.decoder, &output) == OutputBufferEmpty, "output without a frame buffer");
    InputData input{sizeof(InputData), units[0].data(), units[0].size(), 0, 0, 0};
    check(sceVdecswSetDecodeInput(nullptr, &input) == DecoderInstance, "null decoder accepted input");
    check(sceVdecswSetDecodeInput(session.decoder, nullptr) == ArgumentPointer, "null input accepted");
    input.thisSize = 0x28;
    check(sceVdecswSetDecodeInput(session.decoder, &input) == StructSize, "wrong input size accepted");
    input.thisSize = sizeof(InputData);
    input.auData = nullptr;
    check(sceVdecswSetDecodeInput(session.decoder, &input) == AccessUnitPointer, "null access unit accepted");
    input.auData = units[0].data();
    input.auSize = 0;
    check(sceVdecswSetDecodeInput(session.decoder, &input) == AccessUnitSize, "empty access unit accepted");
    check(sceVdecswTrySyncDecodeInput(session.decoder, nullptr) == ArgumentPointer && sceVdecswTrySyncDecodeInput(nullptr, &result) == DecoderInstance, "input sync argument checks failed");
    result.thisSize = 8;
    checkThrows([&] { sceVdecswTrySyncDecodeInput(session.decoder, &result); }, "input result size");
    check(sceVdecswTrySyncDecodeOutput(session.decoder, nullptr) == ArgumentPointer && sceVdecswTrySyncDecodeOutput(nullptr, &output) == DecoderInstance, "output sync argument checks failed");
    output.thisSize = 0x20;
    checkThrows([&] { sceVdecswTrySyncDecodeOutput(session.decoder, &output); }, "output info size");
    FrameBuffer frame{sizeof(FrameBuffer), nullptr, session.buffer.size()};
    check(sceVdecswSetDecodeOutput(nullptr, &frame) == DecoderInstance && sceVdecswSetDecodeOutput(session.decoder, nullptr) == ArgumentPointer, "frame buffer argument checks failed");
    check(sceVdecswSetDecodeOutput(session.decoder, &frame) == FrameBufferPointer, "null frame buffer accepted");
    frame.frameBuffer = session.buffer.data();
    frame.frameBufferSize = 0;
    check(sceVdecswSetDecodeOutput(session.decoder, &frame) == FrameBufferSize, "empty frame buffer accepted");
    frame.thisSize = 16;
    check(sceVdecswSetDecodeOutput(session.decoder, &frame) == StructSize, "wrong frame buffer size accepted");
    session.feed(units, 0);
    session.finish(0);
    session.offer();
    output = OutputInfo{sizeof(OutputInfo)};
    check(sceVdecswTrySyncDecodeOutput(session.decoder, &output) == 0 && !output.isValid && output.isLastFrame, "finalized decoder did not report the end again");
    session.offered = false;
    session.feed(units, PictureHashes.size());
    session.finish(PictureHashes.size());
    check(sceVdecswDeleteDecoder(session.decoder) == 0, "decoder deletion failed");
}

void testPictureInfo() {
    Queue queue;
    const auto units = accessUnits();
    auto config = makeConfig(queue.handle);
    config.decodePipelineDepth = static_cast<std::uint32_t>(units.size());
    DecoderMemoryInfo memory{sizeof(DecoderMemoryInfo)};
    check(sceVdecswQueryDecoderMemoryInfo(&config, &memory) == 0, "decoder memory query failed");
    void* decoder = createDecoder(config);
    std::vector<std::uint8_t> buffer(memory.maxFrameBufferSize);
    for (std::size_t unit = 0; unit < units.size(); ++unit) {
        const InputData input{sizeof(InputData), units[unit].data(), units[unit].size(), 1000 + unit, unit, 0xa0 + unit};
        check(sceVdecswSetDecodeInput(decoder, &input) == 0, "access unit was not accepted");
    }
    check(sceVdecswFinalizeDecodeSequence(decoder) == 0, "finalize failed");
    FrameBuffer frame{sizeof(FrameBuffer), buffer.data(), buffer.size()};
    check(sceVdecswSetDecodeOutput(decoder, &frame) == 0, "frame buffer was not accepted");
    OutputInfo output{0x30};
    output.frameFormat = 0xdead;
    check(sceVdecswTrySyncDecodeOutput(decoder, &output) == 0 && output.isValid && output.frameFormat == 0xdead, "version 1 output info mismatch");
    AvcPictureInfo info{sizeof(AvcPictureInfo)};
    check(sceVdecswGetAvcPictureInfo(nullptr, &info, nullptr) == ArgumentPointer, "null output info accepted");
    check(sceVdecswGetAvcPictureInfo(&output, &info, nullptr) == 0 && info.isValid && info.ptsData == 1000 && info.idrPictureFlag == 1, "version 1 output info picture mismatch");
    AvcPictureInfo second{sizeof(AvcPictureInfo)};
    second.ptsData = 7;
    check(sceVdecswGetAvcPictureInfo(&output, &info, &second) == 0 && !second.isValid && second.ptsData == 7, "second picture info written for a frame picture");
    check(sceVdecswGetAvcPictureInfo(&output, nullptr, nullptr) == 0, "null first picture info rejected");
    OutputInfo copy = output;
    copy.thisSize = 0x20;
    check(sceVdecswGetAvcPictureInfo(&copy, &info, nullptr) == StructSize, "wrong output info size accepted");
    copy.thisSize = sizeof(OutputInfo);
    copy.pictureCount = 0;
    AvcPictureInfo untouched{sizeof(AvcPictureInfo)};
    check(sceVdecswGetAvcPictureInfo(&copy, &untouched, nullptr) == 0 && !untouched.isValid, "picture info written without pictures");
    copy.pictureCount = 2;
    checkThrows([&] { sceVdecswGetAvcPictureInfo(&copy, &info, &second); }, "two field pictures");
    copy.pictureCount = 1;
    copy.frameBuffer = buffer.data() + 1;
    checkThrows([&] { sceVdecswGetAvcPictureInfo(&copy, &info, nullptr); }, "was not produced");
    info.thisSize = 0x70;
    checkThrows([&] { sceVdecswGetAvcPictureInfo(&output, &info, nullptr); }, "picture info size");
    check(sceVdecswResetDecoder(decoder) == 0, "reset failed");
    output = OutputInfo{sizeof(OutputInfo)};
    check(sceVdecswTrySyncDecodeOutput(decoder, &output) == OutputBufferEmpty, "reset kept the frame buffer");
    check(sceVdecswSetDecodeOutput(decoder, &frame) == 0, "frame buffer was not accepted after reset");
    check(sceVdecswTrySyncDecodeOutput(decoder, &output) == DecodePending, "reset kept decoded pictures");
    InputResult result{sizeof(InputResult)};
    check(sceVdecswTrySyncDecodeInput(decoder, &result) == InputQueueEmpty, "reset kept input results");
    Session session{decoder, std::move(buffer), true};
    session.feed(units, 0);
    session.finish(0);
    check(sceVdecswDeleteDecoder(decoder) == 0, "decoder deletion failed");
}

std::size_t decodeAll(void* decoder, const std::vector<std::vector<std::uint8_t>>& units, std::vector<std::uint8_t>& buffer) {
    std::size_t pictures = 0;
    const auto take = [&] {
        FrameBuffer frame{sizeof(FrameBuffer), buffer.data(), buffer.size()};
        check(sceVdecswSetDecodeOutput(decoder, &frame) == 0, "frame buffer was not accepted");
        OutputInfo output{sizeof(OutputInfo)};
        check(sceVdecswTrySyncDecodeOutput(decoder, &output) == 0, "output failed");
        if (!output.isValid) return false;
        check(pictures < PictureHashes.size() && hashNv12(buffer.data()) == PictureHashes[pictures], "picture " + std::to_string(pictures) + " differs from the reference");
        ++pictures;
        return true;
    };
    for (const auto& unit : units) {
        const InputData input{sizeof(InputData), unit.data(), unit.size(), 0, 0, 0};
        check(sceVdecswSetDecodeInput(decoder, &input) == 0, "access unit was not accepted");
        InputResult result{sizeof(InputResult)};
        check(sceVdecswTrySyncDecodeInput(decoder, &result) == 0 && result.decodedAu == unit.data(), "input result mismatch");
        for (std::uint32_t frame = 0; frame < result.outputFrameCount; ++frame) check(take(), "announced picture missing");
    }
    check(sceVdecswFinalizeDecodeSequence(decoder) == 0, "finalize failed");
    while (take()) {
    }
    return pictures;
}

void testInputQueueAndReset() {
    Queue queue;
    const auto config = makeConfig(queue.handle);
    const auto units = accessUnits();
    std::vector<std::uint8_t> buffer(Pitch * CodedHeight * 3 / 2);
    void* decoder = createDecoder(config);
    for (std::size_t unit = 0; unit < config.decodePipelineDepth; ++unit) {
        const InputData input{sizeof(InputData), units[unit].data(), units[unit].size(), 0, 0, 0};
        check(sceVdecswSetDecodeInput(decoder, &input) == 0, "access unit within the queue depth was not accepted");
    }
    const InputData beyond{sizeof(InputData), units[3].data(), units[3].size(), 0, 0, 0};
    checkThrows([&] { sceVdecswSetDecodeInput(decoder, &beyond); }, "input queue depth 3");
    InputResult result{sizeof(InputResult)};
    check(sceVdecswTrySyncDecodeInput(decoder, &result) == 0 && result.decodedAu == units[0].data(), "first input result missing");
    check(sceVdecswSetDecodeInput(decoder, &beyond) == 0, "access unit after an input sync was not accepted");
    check(sceVdecswDeleteDecoder(decoder) == 0, "decoder deletion failed");

    decoder = createDecoder(config);
    check(decodeAll(decoder, {units[0]}, buffer) == 1, "intra picture was not decoded");
    check(sceVdecswResetDecoder(decoder) == 0, "reset failed");
    checkThrows([&] { decodeAll(decoder, std::vector<std::vector<std::uint8_t>>(units.begin() + 1, units.end()), buffer); }, "lacks its reference pictures");
    check(sceVdecswDeleteDecoder(decoder) == 0, "decoder deletion failed");

    decoder = createDecoder(config);
    const auto sei = static_cast<std::ptrdiff_t>(findNal(units[0], 0x06));
    const std::vector<std::uint8_t> parameters(units[0].begin(), units[0].begin() + sei);
    const InputData input{sizeof(InputData), parameters.data(), parameters.size(), 0, 0, 0};
    check(sceVdecswSetDecodeInput(decoder, &input) == 0, "parameter sets were not accepted");
    check(sceVdecswTrySyncDecodeInput(decoder, &result) == 0 && result.outputFrameCount == 0, "parameter sets produced a picture");
    check(sceVdecswResetDecoder(decoder) == 0, "reset failed");
    std::vector<std::vector<std::uint8_t>> pictures{std::vector<std::uint8_t>(units[0].begin() + sei, units[0].end())};
    pictures.insert(pictures.end(), units.begin() + 1, units.end());
    check(decodeAll(decoder, pictures, buffer) == PictureHashes.size(), "reset dropped the parameter sets");
    check(sceVdecswDeleteDecoder(decoder) == 0, "decoder deletion failed");
}

void testStreams() {
    Queue queue;
    auto config = makeConfig(queue.handle);
    const auto units = accessUnits();
    std::vector<std::uint8_t> buffer(Pitch * CodedHeight * 3 / 2);
    const auto expectFailure = [&](const std::vector<std::uint8_t>& unit, const std::string& reason) {
        void* decoder = createDecoder(config);
        const InputData input{sizeof(InputData), unit.data(), unit.size(), 0, 0, 0};
        checkThrows([&] { sceVdecswSetDecodeInput(decoder, &input); }, reason);
        check(sceVdecswDeleteDecoder(decoder) == 0, "decoder deletion failed");
    };
    std::vector<std::uint8_t> malformed(units[0]);
    malformed[3] = 2;
    expectFailure(malformed, "Annex B");
    expectFailure(units[1], "parameter set that was not received");
    expectFailure(std::vector<std::uint8_t>(FieldPictureUnit.begin(), FieldPictureUnit.end()), "field pictures");
    expectFailure(std::vector<std::uint8_t>(ExtensionUnit.begin(), ExtensionUnit.end()), "NAL unit type 20");
    std::vector<std::uint8_t> doubled(units[0]);
    doubled.insert(doubled.end(), units[0].begin() + static_cast<std::ptrdiff_t>(findNal(units[0], 0x65)), units[0].end());
    expectFailure(doubled, "several pictures");
    config.maxFrameWidth = 32;
    expectFailure(units[0], "exceeds the configured maximum");
    config.maxFrameWidth = static_cast<std::int32_t>(Width);
    void* decoder = createDecoder(config);
    const std::vector<std::uint8_t> parameters(units[0].begin(), units[0].begin() + static_cast<std::ptrdiff_t>(findNal(units[0], 0x06)));
    InputData input{sizeof(InputData), parameters.data(), parameters.size(), 0, 0, 0};
    check(sceVdecswSetDecodeInput(decoder, &input) == 0, "parameter sets were not accepted");
    InputResult result{sizeof(InputResult)};
    check(sceVdecswTrySyncDecodeInput(decoder, &result) == 0 && result.outputFrameCount == 0 && result.decodedAu == parameters.data(), "parameter sets produced a picture");
    const std::vector<std::uint8_t> picture(units[0].begin() + static_cast<std::ptrdiff_t>(findNal(units[0], 0x06)), units[0].end());
    input = InputData{sizeof(InputData), picture.data(), picture.size(), 0, 0, 0};
    check(sceVdecswSetDecodeInput(decoder, &input) == 0, "picture after parameter sets was not accepted");
    check(sceVdecswFinalizeDecodeSequence(decoder) == 0, "finalize failed");
    FrameBuffer frame{sizeof(FrameBuffer), buffer.data(), 100};
    check(sceVdecswSetDecodeOutput(decoder, &frame) == 0, "small frame buffer was not accepted");
    OutputInfo output{sizeof(OutputInfo)};
    checkThrows([&] { sceVdecswTrySyncDecodeOutput(decoder, &output); }, "cannot hold");
    check(sceVdecswDeleteDecoder(decoder) == 0, "decoder deletion failed");
}

}

int main() {
    try {
        testComputeQueue();
        testDecoderLifetime();
        testDecode();
        testPictureInfo();
        testInputQueueAndReset();
        testStreams();
        std::puts("Vdecsw tests passed");
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "%s\n", error.what());
        return 1;
    }
}
