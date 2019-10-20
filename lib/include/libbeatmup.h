#ifndef __LIBBEATMUP_INCLUDE_H__
#define __LIBBEATMUP_INCLUDE_H__
#include <stdint.h>


#ifdef BEATMUP_PLATFORM_64BIT_
	//typedef uint64_t handle_t;
#else
	//typedef uint32_t handle_t;
#endif
typedef void* handle_t;

typedef unsigned int memchunk_t;
typedef enum {
    doNotUseGPU = 0,
    useGPUIfAvailable = 1,
    useGPU = 2
} execution_target_t;
typedef struct {
  unsigned int _1;
  unsigned int _2;
} image_resolution_t;
typedef struct {
  int _1;
  int _2;
  int _3;
  int _4;
} int_rectangle_t;
typedef unsigned char pool_index_t;
typedef struct {
  int _1;
  int _2;
} int_point_t;
typedef enum {
    CPU = 0,
    GPU = 1
} processing_target_t;
typedef unsigned int job_t;
typedef enum {
    NONE = 0,
    ADD = 1,
    MULTIPLY = 2
} operation_t;
typedef enum {
    SingleByte = 0,
    TripleByte = 1,
    QuadByte = 2,
    SingleFloat = 3,
    TripleFloat = 4,
    QuadFloat = 5,
    BinaryMask = 6,
    QuaternaryMask = 7,
    HexMask = 8
} pixel_format_t;
typedef unsigned char thread_index_t;
typedef uint8_t pixbyte_t;
typedef uint32_t msize_t;
typedef pixbyte_t * pixptrbyte_t;
typedef pixptrbyte_t pixptr_t;

#ifdef __cplusplus
extern "C" {
#endif


/* Beatmup::Environment */
void BeatmupEnvironmentCreate1(handle_t *);
void BeatmupEnvironmentCreate2(handle_t *, const pool_index_t numThreadPools, const char * swapFilePrefix, const char * swapFileSuffix);
void BeatmupEnvironmentDestroy(handle_t);
float BeatmupEnvironmentPerformTask(handle_t, handle_t task, const pool_index_t pool);
void BeatmupEnvironmentRepeatTask(handle_t, handle_t task, bool abortCurrent, const pool_index_t pool);
job_t BeatmupEnvironmentSubmitTask(handle_t, handle_t task, const pool_index_t pool);
job_t BeatmupEnvironmentSubmitPersistentTask(handle_t, handle_t task, const pool_index_t pool);
void BeatmupEnvironmentWaitForJob(handle_t, job_t job, const pool_index_t pool);
bool BeatmupEnvironmentAbortJob(handle_t, job_t job, const pool_index_t pool);
void BeatmupEnvironmentWait(handle_t, const pool_index_t pool);
bool BeatmupEnvironmentBusy(handle_t, const pool_index_t pool);
const thread_index_t BeatmupEnvironmentMaxAllowedWorkerCount(handle_t, const pool_index_t pool);
void BeatmupEnvironmentLimitWorkerCount(handle_t, thread_index_t maxValue, const pool_index_t pool);
const memchunk_t BeatmupEnvironmentAllocateMemory(handle_t, msize_t size);
const pixptr_t BeatmupEnvironmentAcquireMemory(handle_t, memchunk_t chunk);
void BeatmupEnvironmentReleaseMemory(handle_t, memchunk_t chunk, bool unusedAnymore);
void BeatmupEnvironmentFreeMemory(handle_t, memchunk_t chunk);
msize_t BeatmupEnvironmentSwapOnDisk(handle_t, msize_t howMuch);
handle_t BeatmupEnvironmentGetEventListener(handle_t);
bool BeatmupEnvironmentIsGpuQueried(handle_t);
bool BeatmupEnvironmentIsGpuReady(handle_t);
bool BeatmupEnvironmentIsManagingThread(handle_t);
handle_t BeatmupEnvironmentGetGpuRecycleBin(handle_t);
msize_t BeatmupEnvironmentGetTotalRam();
void BeatmupEnvironmentWarmUpGpu(handle_t);

/* Beatmup::AbstractBitmap */
const int BeatmupAbstractBitmapGetDepth(handle_t);
const pixel_format_t BeatmupAbstractBitmapGetPixelFormat(handle_t);
const msize_t BeatmupAbstractBitmapGetMemorySize(handle_t);
void BeatmupAbstractBitmapUnlockPixels(handle_t);
void BeatmupAbstractBitmapLockPixels(handle_t, processing_target_t);
void BeatmupAbstractBitmapInvalidate(handle_t, processing_target_t);
bool BeatmupAbstractBitmapIsUpToDate(handle_t, processing_target_t);
bool BeatmupAbstractBitmapIsDirty(handle_t);
const pixptr_t BeatmupAbstractBitmapGetData(handle_t, int x, int y);
int BeatmupAbstractBitmapGetPixelInt(handle_t, int x, int y, int cha);
const unsigned char BeatmupAbstractBitmapGetBitsPerPixel(handle_t);
const unsigned char BeatmupAbstractBitmapGetNumberOfChannels(handle_t);
const image_resolution_t BeatmupAbstractBitmapGetSize(handle_t);
handle_t BeatmupAbstractBitmapGetEnvironment(handle_t);
void BeatmupAbstractBitmapZero(handle_t);
bool BeatmupAbstractBitmapIsInteger1(handle_t);
bool BeatmupAbstractBitmapIsFloat1(handle_t);
bool BeatmupAbstractBitmapIsMask1(handle_t);
bool BeatmupAbstractBitmapIsInteger2(pixel_format_t pixelFormat);
bool BeatmupAbstractBitmapIsFloat2(pixel_format_t pixelFormat);
bool BeatmupAbstractBitmapIsMask2(pixel_format_t pixelFormat);
void BeatmupAbstractBitmapDestroy(handle_t);

/* Beatmup::InternalBitmap */
void BeatmupInternalBitmapCreate1(handle_t *, handle_t env, pixel_format_t pixelFormat, int width, int height, bool allocate);
void BeatmupInternalBitmapCreate2(handle_t *, handle_t env, const char * filename);
void BeatmupInternalBitmapDestroy(handle_t);
const pixel_format_t BeatmupInternalBitmapGetPixelFormat(handle_t);
const int BeatmupInternalBitmapGetWidth(handle_t);
const int BeatmupInternalBitmapGetHeight(handle_t);
const msize_t BeatmupInternalBitmapGetMemorySize(handle_t);
const pixptr_t BeatmupInternalBitmapGetData(handle_t, int x, int y);
void BeatmupInternalBitmapUnlockPixels(handle_t);
void BeatmupInternalBitmapSaveBmp(handle_t, const char * filename);

/* Beatmup::BitmapConverter */
void BeatmupBitmapConverterCreate(handle_t *);
void BeatmupBitmapConverterSetBitmaps(handle_t, handle_t input, handle_t output);
thread_index_t BeatmupBitmapConverterMaxAllowedThreads(handle_t);
execution_target_t BeatmupBitmapConverterGetExecutionTarget(handle_t);
void BeatmupBitmapConverterConvert(handle_t input, handle_t output);
void BeatmupBitmapConverterDestroy(handle_t);

/* Beatmup::Crop */
void BeatmupCropCreate(handle_t *);
thread_index_t BeatmupCropMaxAllowedThreads(handle_t);
void BeatmupCropSetInput(handle_t, handle_t input);
void BeatmupCropSetOutput(handle_t, handle_t output);
void BeatmupCropSetCropRect(handle_t, int_rectangle_t);
void BeatmupCropSetOutputOrigin(handle_t, int_point_t);
bool BeatmupCropIsFit(handle_t);
handle_t BeatmupCropRun(handle_t bitmap, int_rectangle_t clipRect);
void BeatmupCropDestroy(handle_t);

/* Beatmup::BitmapBinaryOperation */
void BeatmupBitmapBinaryOperationCreate(handle_t *);
void BeatmupBitmapBinaryOperationSetOperand1(handle_t, handle_t op1);
void BeatmupBitmapBinaryOperationSetOperand2(handle_t, handle_t op2);
void BeatmupBitmapBinaryOperationSetOutput(handle_t, handle_t output);
void BeatmupBitmapBinaryOperationSetOperation(handle_t, const operation_t operation);
void BeatmupBitmapBinaryOperationSetCropSize(handle_t, int width, int height);
void BeatmupBitmapBinaryOperationSetOp1Origin(handle_t, const int_point_t origin);
void BeatmupBitmapBinaryOperationSetOp2Origin(handle_t, const int_point_t origin);
void BeatmupBitmapBinaryOperationSetOutputOrigin(handle_t, const int_point_t origin);
void BeatmupBitmapBinaryOperationResetCrop(handle_t);
int BeatmupBitmapBinaryOperationGetCropWidth(handle_t);
int BeatmupBitmapBinaryOperationGetCropHeight(handle_t);
const int_point_t BeatmupBitmapBinaryOperationGetOp1Origin(handle_t);
const int_point_t BeatmupBitmapBinaryOperationGetOp2Origin(handle_t);
const int_point_t BeatmupBitmapBinaryOperationGetOutputOrigin(handle_t);
thread_index_t BeatmupBitmapBinaryOperationMaxAllowedThreads(handle_t);
void BeatmupBitmapBinaryOperationDestroy(handle_t);

/* Beatmup::BitmapResampler */
void BeatmupBitmapResamplerCreate(handle_t *);
void BeatmupBitmapResamplerSetBitmaps(handle_t, handle_t input, handle_t output);
void BeatmupBitmapResamplerSetInputRect(handle_t, int_rectangle_t* rect);
void BeatmupBitmapResamplerSetOutputRect(handle_t, int_rectangle_t* rect);
int_rectangle_t BeatmupBitmapResamplerGetInputRect(handle_t);
int_rectangle_t BeatmupBitmapResamplerGetOutputRect(handle_t);
thread_index_t BeatmupBitmapResamplerMaxAllowedThreads(handle_t);
void BeatmupBitmapResamplerDestroy(handle_t);

/* Beatmup::BitmapTools */
handle_t BeatmupBitmapToolsMakeCopy1(handle_t source, pixel_format_t newPixelFormat);
handle_t BeatmupBitmapToolsMakeCopy2(handle_t source);
handle_t BeatmupBitmapToolsChessboard(handle_t env, int width, int height, int cellSize, pixel_format_t pixelFormat);
void BeatmupBitmapToolsMakeOpaque(handle_t, int_rectangle_t);
void BeatmupBitmapToolsInvert(handle_t input, handle_t output);

#ifdef __cplusplus
}
#endif

#endif