#include "../include/libbeatmup.h"
#include "../../core/utils/image_resolution.h"
#include "../../core/bitmap/operator.h"
#include "../../core/basic_types.h"
#include "../../core/bitmap/crop.h"
#include "../../core/bitmap/converter.h"
#include "../../core/parallelism.h"
#include "../../core/environment.h"
#include "../../core/bitmap/resampler.h"
#include "../../core/bitmap/abstract_bitmap.h"
#include "../../core/bitmap/tools.h"
#include "../../core/geometry.h"
#include "../../core/gpu/recycle_bin.h"
#include "../../core/bitmap/internal_bitmap.h"


void BeatmupEnvironmentCreate1(handle_t * handle) {
    *handle = static_cast<handle_t>(new Beatmup::Environment());
}


void BeatmupEnvironmentCreate2(handle_t * handle, const pool_index_t _numThreadPools, const char * _swapFilePrefix, const char * _swapFileSuffix) {
    *handle = static_cast<handle_t>(new Beatmup::Environment(_numThreadPools, _swapFilePrefix, _swapFileSuffix));
}


void BeatmupEnvironmentDestroy(handle_t handle) {
    delete static_cast<Beatmup::Environment*>(handle);
}


float BeatmupEnvironmentPerformTask(handle_t handle, handle_t _task, const pool_index_t _pool) {
    return static_cast<Beatmup::Environment*>(handle)->performTask(*static_cast< Beatmup::AbstractTask* >(_task), _pool);
}


void BeatmupEnvironmentRepeatTask(handle_t handle, handle_t _task, bool _abortCurrent, const pool_index_t _pool) {
    static_cast<Beatmup::Environment*>(handle)->repeatTask(*static_cast< Beatmup::AbstractTask* >(_task), _abortCurrent, _pool);
}


job_t BeatmupEnvironmentSubmitTask(handle_t handle, handle_t _task, const pool_index_t _pool) {
    return static_cast<Beatmup::Environment*>(handle)->submitTask(*static_cast< Beatmup::AbstractTask* >(_task), _pool);
}


job_t BeatmupEnvironmentSubmitPersistentTask(handle_t handle, handle_t _task, const pool_index_t _pool) {
    return static_cast<Beatmup::Environment*>(handle)->submitPersistentTask(*static_cast< Beatmup::AbstractTask* >(_task), _pool);
}


void BeatmupEnvironmentWaitForJob(handle_t handle, job_t _job, const pool_index_t _pool) {
    static_cast<Beatmup::Environment*>(handle)->waitForJob(_job, _pool);
}


bool BeatmupEnvironmentAbortJob(handle_t handle, job_t _job, const pool_index_t _pool) {
    return static_cast<Beatmup::Environment*>(handle)->abortJob(_job, _pool);
}


void BeatmupEnvironmentWait(handle_t handle, const pool_index_t _pool) {
    static_cast<Beatmup::Environment*>(handle)->wait(_pool);
}


bool BeatmupEnvironmentBusy(handle_t handle, const pool_index_t _pool) {
    return static_cast<Beatmup::Environment*>(handle)->busy(_pool);
}


const thread_index_t BeatmupEnvironmentMaxAllowedWorkerCount(handle_t handle, const pool_index_t _pool) {
    return static_cast<const Beatmup::Environment*>(handle)->maxAllowedWorkerCount(_pool);
}


void BeatmupEnvironmentLimitWorkerCount(handle_t handle, thread_index_t _maxValue, const pool_index_t _pool) {
    static_cast<Beatmup::Environment*>(handle)->limitWorkerCount(_maxValue, _pool);
}


const memchunk_t BeatmupEnvironmentAllocateMemory(handle_t handle, msize_t _size) {
    return static_cast<Beatmup::Environment*>(handle)->allocateMemory(_size);
}


const pixptr_t BeatmupEnvironmentAcquireMemory(handle_t handle, memchunk_t _chunk) {
    return static_cast<Beatmup::Environment*>(handle)->acquireMemory(_chunk);
}


void BeatmupEnvironmentReleaseMemory(handle_t handle, memchunk_t _chunk, bool _unusedAnymore) {
    static_cast<Beatmup::Environment*>(handle)->releaseMemory(_chunk, _unusedAnymore);
}


void BeatmupEnvironmentFreeMemory(handle_t handle, memchunk_t _chunk) {
    static_cast<Beatmup::Environment*>(handle)->freeMemory(_chunk);
}


msize_t BeatmupEnvironmentSwapOnDisk(handle_t handle, msize_t _howMuch) {
    return static_cast<Beatmup::Environment*>(handle)->swapOnDisk(_howMuch);
}


bool BeatmupEnvironmentIsGpuQueried(handle_t handle) {
    return static_cast<const Beatmup::Environment*>(handle)->isGpuQueried();
}


bool BeatmupEnvironmentIsGpuReady(handle_t handle) {
    return static_cast<const Beatmup::Environment*>(handle)->isGpuReady();
}


bool BeatmupEnvironmentIsManagingThread(handle_t handle) {
    return static_cast<const Beatmup::Environment*>(handle)->isManagingThread();
}


handle_t BeatmupEnvironmentGetGpuRecycleBin(handle_t handle) {
    Beatmup::GL::RecycleBin * result = static_cast<const Beatmup::Environment*>(handle)->getGpuRecycleBin();
    return static_cast<handle_t>(result);
}


msize_t BeatmupEnvironmentGetTotalRam() {
    return Beatmup::Environment::getTotalRam();
}


void BeatmupEnvironmentWarmUpGpu(handle_t handle) {
    static_cast<Beatmup::Environment*>(handle)->warmUpGpu();
}


const int BeatmupAbstractBitmapGetDepth(handle_t handle) {
    return static_cast<const Beatmup::AbstractBitmap*>(handle)->getDepth();
}


const pixel_format_t BeatmupAbstractBitmapGetPixelFormat(handle_t handle) {
    Beatmup::PixelFormat result = static_cast<const Beatmup::AbstractBitmap*>(handle)->getPixelFormat();
    return static_cast<pixel_format_t>(result);
}


const msize_t BeatmupAbstractBitmapGetMemorySize(handle_t handle) {
    return static_cast<const Beatmup::AbstractBitmap*>(handle)->getMemorySize();
}


void BeatmupAbstractBitmapUnlockPixels(handle_t handle) {
    static_cast<Beatmup::AbstractBitmap*>(handle)->unlockPixels();
}


void BeatmupAbstractBitmapLockPixels(handle_t handle, processing_target_t _par1) {
    static_cast<Beatmup::AbstractBitmap*>(handle)->lockPixels(static_cast<Beatmup::ProcessingTarget>(_par1));
}


void BeatmupAbstractBitmapInvalidate(handle_t handle, processing_target_t _par1) {
    static_cast<Beatmup::AbstractBitmap*>(handle)->invalidate(static_cast<Beatmup::ProcessingTarget>(_par1));
}


bool BeatmupAbstractBitmapIsUpToDate(handle_t handle, processing_target_t _par1) {
    return static_cast<const Beatmup::AbstractBitmap*>(handle)->isUpToDate(static_cast<Beatmup::ProcessingTarget>(_par1));
}


bool BeatmupAbstractBitmapIsDirty(handle_t handle) {
    return static_cast<const Beatmup::AbstractBitmap*>(handle)->isDirty();
}


const pixptr_t BeatmupAbstractBitmapGetData(handle_t handle, int _x, int _y) {
    return static_cast<const Beatmup::AbstractBitmap*>(handle)->getData(_x, _y);
}


int BeatmupAbstractBitmapGetPixelInt(handle_t handle, int _x, int _y, int _cha) {
    return static_cast<const Beatmup::AbstractBitmap*>(handle)->getPixelInt(_x, _y, _cha);
}


const unsigned char BeatmupAbstractBitmapGetBitsPerPixel(handle_t handle) {
    return static_cast<const Beatmup::AbstractBitmap*>(handle)->getBitsPerPixel();
}


const unsigned char BeatmupAbstractBitmapGetNumberOfChannels(handle_t handle) {
    return static_cast<const Beatmup::AbstractBitmap*>(handle)->getNumberOfChannels();
}


const image_resolution_t BeatmupAbstractBitmapGetSize(handle_t handle) {
    Beatmup::ImageResolution result = static_cast<const Beatmup::AbstractBitmap*>(handle)->getSize();
    return image_resolution_t{ result.getWidth(), result.getHeight() };
}


handle_t BeatmupAbstractBitmapGetEnvironment(handle_t handle) {
    Beatmup::Environment & result = static_cast<const Beatmup::AbstractBitmap*>(handle)->getEnvironment();
    return static_cast<handle_t>(&result);
}


void BeatmupAbstractBitmapZero(handle_t handle) {
    static_cast<Beatmup::AbstractBitmap*>(handle)->zero();
}


bool BeatmupAbstractBitmapIsInteger1(handle_t handle) {
    return static_cast<const Beatmup::AbstractBitmap*>(handle)->isInteger();
}


bool BeatmupAbstractBitmapIsFloat1(handle_t handle) {
    return static_cast<const Beatmup::AbstractBitmap*>(handle)->isFloat();
}


bool BeatmupAbstractBitmapIsMask1(handle_t handle) {
    return static_cast<const Beatmup::AbstractBitmap*>(handle)->isMask();
}


bool BeatmupAbstractBitmapIsInteger2(pixel_format_t _pixelFormat) {
    return Beatmup::AbstractBitmap::isInteger(static_cast<Beatmup::PixelFormat>(_pixelFormat));
}


bool BeatmupAbstractBitmapIsFloat2(pixel_format_t _pixelFormat) {
    return Beatmup::AbstractBitmap::isFloat(static_cast<Beatmup::PixelFormat>(_pixelFormat));
}


bool BeatmupAbstractBitmapIsMask2(pixel_format_t _pixelFormat) {
    return Beatmup::AbstractBitmap::isMask(static_cast<Beatmup::PixelFormat>(_pixelFormat));
}


void BeatmupAbstractBitmapDestroy(handle_t handle) {
    delete static_cast<Beatmup::AbstractBitmap*>(handle);
}


void BeatmupInternalBitmapCreate1(handle_t * handle, handle_t _env, pixel_format_t _pixelFormat, int _width, int _height, bool _allocate) {
    *handle = static_cast<handle_t>(new Beatmup::InternalBitmap(*static_cast< Beatmup::Environment* >(_env), static_cast<Beatmup::PixelFormat>(_pixelFormat), _width, _height, _allocate));
}


void BeatmupInternalBitmapCreate2(handle_t * handle, handle_t _env, const char * _filename) {
    *handle = static_cast<handle_t>(new Beatmup::InternalBitmap(*static_cast< Beatmup::Environment* >(_env), _filename));
}


void BeatmupInternalBitmapDestroy(handle_t handle) {
    delete static_cast<Beatmup::InternalBitmap*>(handle);
}


const pixel_format_t BeatmupInternalBitmapGetPixelFormat(handle_t handle) {
    Beatmup::PixelFormat result = static_cast<const Beatmup::InternalBitmap*>(handle)->getPixelFormat();
    return static_cast<pixel_format_t>(result);
}


const int BeatmupInternalBitmapGetWidth(handle_t handle) {
    return static_cast<const Beatmup::InternalBitmap*>(handle)->getWidth();
}


const int BeatmupInternalBitmapGetHeight(handle_t handle) {
    return static_cast<const Beatmup::InternalBitmap*>(handle)->getHeight();
}


const msize_t BeatmupInternalBitmapGetMemorySize(handle_t handle) {
    return static_cast<const Beatmup::InternalBitmap*>(handle)->getMemorySize();
}


const pixptr_t BeatmupInternalBitmapGetData(handle_t handle, int _x, int _y) {
    return static_cast<const Beatmup::InternalBitmap*>(handle)->getData(_x, _y);
}


void BeatmupInternalBitmapUnlockPixels(handle_t handle) {
    static_cast<Beatmup::InternalBitmap*>(handle)->unlockPixels();
}


void BeatmupInternalBitmapSaveBmp(handle_t handle, const char * _filename) {
    static_cast<Beatmup::InternalBitmap*>(handle)->saveBmp(_filename);
}


void BeatmupBitmapConverterCreate(handle_t * handle) {
    *handle = static_cast<handle_t>(new Beatmup::BitmapConverter());
}


void BeatmupBitmapConverterSetBitmaps(handle_t handle, handle_t _input, handle_t _output) {
    static_cast<Beatmup::BitmapConverter*>(handle)->setBitmaps(static_cast< Beatmup::AbstractBitmap* >(_input), static_cast< Beatmup::AbstractBitmap* >(_output));
}


thread_index_t BeatmupBitmapConverterMaxAllowedThreads(handle_t handle) {
    return static_cast<const Beatmup::BitmapConverter*>(handle)->maxAllowedThreads();
}


execution_target_t BeatmupBitmapConverterGetExecutionTarget(handle_t handle) {
    Beatmup::AbstractTask::ExecutionTarget result = static_cast<const Beatmup::BitmapConverter*>(handle)->getExecutionTarget();
    return static_cast<execution_target_t>(result);
}


void BeatmupBitmapConverterConvert(handle_t _input, handle_t _output) {
    Beatmup::BitmapConverter::convert(*static_cast< Beatmup::AbstractBitmap* >(_input), *static_cast< Beatmup::AbstractBitmap* >(_output));
}


void BeatmupBitmapConverterDestroy(handle_t handle) {
    delete static_cast<Beatmup::BitmapConverter*>(handle);
}


void BeatmupCropCreate(handle_t * handle) {
    *handle = static_cast<handle_t>(new Beatmup::Crop());
}


thread_index_t BeatmupCropMaxAllowedThreads(handle_t handle) {
    return static_cast<const Beatmup::Crop*>(handle)->maxAllowedThreads();
}


void BeatmupCropSetInput(handle_t handle, handle_t _input) {
    static_cast<Beatmup::Crop*>(handle)->setInput(static_cast< Beatmup::AbstractBitmap* >(_input));
}


void BeatmupCropSetOutput(handle_t handle, handle_t _output) {
    static_cast<Beatmup::Crop*>(handle)->setOutput(static_cast< Beatmup::AbstractBitmap* >(_output));
}


void BeatmupCropSetCropRect(handle_t handle, int_rectangle_t _par1) {
    Beatmup::IntRectangle __par1(_par1._1, _par1._2, _par1._3, _par1._4);
    static_cast<Beatmup::Crop*>(handle)->setCropRect(__par1);
}


void BeatmupCropSetOutputOrigin(handle_t handle, int_point_t _par1) {
    Beatmup::IntPoint __par1(_par1._1, _par1._2);
    static_cast<Beatmup::Crop*>(handle)->setOutputOrigin(__par1);
}


bool BeatmupCropIsFit(handle_t handle) {
    return static_cast<const Beatmup::Crop*>(handle)->isFit();
}


handle_t BeatmupCropRun(handle_t _bitmap, int_rectangle_t _clipRect) {
    Beatmup::IntRectangle __clipRect(_clipRect._1, _clipRect._2, _clipRect._3, _clipRect._4);
    Beatmup::AbstractBitmap * result = Beatmup::Crop::run(*static_cast< Beatmup::AbstractBitmap* >(_bitmap), __clipRect);
    return static_cast<handle_t>(result);
}


void BeatmupCropDestroy(handle_t handle) {
    delete static_cast<Beatmup::Crop*>(handle);
}


void BeatmupBitmapBinaryOperationCreate(handle_t * handle) {
    *handle = static_cast<handle_t>(new Beatmup::BitmapBinaryOperation());
}


void BeatmupBitmapBinaryOperationSetOperand1(handle_t handle, handle_t _op1) {
    static_cast<Beatmup::BitmapBinaryOperation*>(handle)->setOperand1(static_cast< Beatmup::AbstractBitmap* >(_op1));
}


void BeatmupBitmapBinaryOperationSetOperand2(handle_t handle, handle_t _op2) {
    static_cast<Beatmup::BitmapBinaryOperation*>(handle)->setOperand2(static_cast< Beatmup::AbstractBitmap* >(_op2));
}


void BeatmupBitmapBinaryOperationSetOutput(handle_t handle, handle_t _output) {
    static_cast<Beatmup::BitmapBinaryOperation*>(handle)->setOutput(static_cast< Beatmup::AbstractBitmap* >(_output));
}


void BeatmupBitmapBinaryOperationSetOperation(handle_t handle, const operation_t _operation) {
    static_cast<Beatmup::BitmapBinaryOperation*>(handle)->setOperation(static_cast<Beatmup::BitmapBinaryOperation::Operation>(_operation));
}


void BeatmupBitmapBinaryOperationSetCropSize(handle_t handle, int _width, int _height) {
    static_cast<Beatmup::BitmapBinaryOperation*>(handle)->setCropSize(_width, _height);
}


void BeatmupBitmapBinaryOperationSetOp1Origin(handle_t handle, const int_point_t _origin) {
    Beatmup::IntPoint __origin(_origin._1, _origin._2);
    static_cast<Beatmup::BitmapBinaryOperation*>(handle)->setOp1Origin(__origin);
}


void BeatmupBitmapBinaryOperationSetOp2Origin(handle_t handle, const int_point_t _origin) {
    Beatmup::IntPoint __origin(_origin._1, _origin._2);
    static_cast<Beatmup::BitmapBinaryOperation*>(handle)->setOp2Origin(__origin);
}


void BeatmupBitmapBinaryOperationSetOutputOrigin(handle_t handle, const int_point_t _origin) {
    Beatmup::IntPoint __origin(_origin._1, _origin._2);
    static_cast<Beatmup::BitmapBinaryOperation*>(handle)->setOutputOrigin(__origin);
}


void BeatmupBitmapBinaryOperationResetCrop(handle_t handle) {
    static_cast<Beatmup::BitmapBinaryOperation*>(handle)->resetCrop();
}


int BeatmupBitmapBinaryOperationGetCropWidth(handle_t handle) {
    return static_cast<const Beatmup::BitmapBinaryOperation*>(handle)->getCropWidth();
}


int BeatmupBitmapBinaryOperationGetCropHeight(handle_t handle) {
    return static_cast<const Beatmup::BitmapBinaryOperation*>(handle)->getCropHeight();
}


const int_point_t BeatmupBitmapBinaryOperationGetOp1Origin(handle_t handle) {
    Beatmup::IntPoint result = static_cast<const Beatmup::BitmapBinaryOperation*>(handle)->getOp1Origin();
    return int_point_t{ result.getX(), result.getY() };
}


const int_point_t BeatmupBitmapBinaryOperationGetOp2Origin(handle_t handle) {
    Beatmup::IntPoint result = static_cast<const Beatmup::BitmapBinaryOperation*>(handle)->getOp2Origin();
    return int_point_t{ result.getX(), result.getY() };
}


const int_point_t BeatmupBitmapBinaryOperationGetOutputOrigin(handle_t handle) {
    Beatmup::IntPoint result = static_cast<const Beatmup::BitmapBinaryOperation*>(handle)->getOutputOrigin();
    return int_point_t{ result.getX(), result.getY() };
}


thread_index_t BeatmupBitmapBinaryOperationMaxAllowedThreads(handle_t handle) {
    return static_cast<const Beatmup::BitmapBinaryOperation*>(handle)->maxAllowedThreads();
}


void BeatmupBitmapBinaryOperationDestroy(handle_t handle) {
    delete static_cast<Beatmup::BitmapBinaryOperation*>(handle);
}


void BeatmupBitmapResamplerCreate(handle_t * handle) {
    *handle = static_cast<handle_t>(new Beatmup::BitmapResampler());
}


void BeatmupBitmapResamplerSetBitmaps(handle_t handle, handle_t _input, handle_t _output) {
    static_cast<Beatmup::BitmapResampler*>(handle)->setBitmaps(static_cast< Beatmup::AbstractBitmap* >(_input), static_cast< Beatmup::AbstractBitmap* >(_output));
}


void BeatmupBitmapResamplerSetInputRect(handle_t handle, int_rectangle_t* _rect) {
    Beatmup::IntRectangle __rect(_rect->_1, _rect->_2, _rect->_3, _rect->_4);
    static_cast<Beatmup::BitmapResampler*>(handle)->setInputRect(__rect);
    *_rect = { __rect.getX1(), __rect.getY1(), __rect.getX2(), __rect.getY2() };
}


void BeatmupBitmapResamplerSetOutputRect(handle_t handle, int_rectangle_t* _rect) {
    Beatmup::IntRectangle __rect(_rect->_1, _rect->_2, _rect->_3, _rect->_4);
    static_cast<Beatmup::BitmapResampler*>(handle)->setOutputRect(__rect);
    *_rect = { __rect.getX1(), __rect.getY1(), __rect.getX2(), __rect.getY2() };
}


int_rectangle_t BeatmupBitmapResamplerGetInputRect(handle_t handle) {
    Beatmup::IntRectangle result = static_cast<const Beatmup::BitmapResampler*>(handle)->getInputRect();
    return int_rectangle_t{ result.getX1(), result.getY1(), result.getX2(), result.getY2() };
}


int_rectangle_t BeatmupBitmapResamplerGetOutputRect(handle_t handle) {
    Beatmup::IntRectangle result = static_cast<const Beatmup::BitmapResampler*>(handle)->getOutputRect();
    return int_rectangle_t{ result.getX1(), result.getY1(), result.getX2(), result.getY2() };
}


thread_index_t BeatmupBitmapResamplerMaxAllowedThreads(handle_t handle) {
    return static_cast<const Beatmup::BitmapResampler*>(handle)->maxAllowedThreads();
}


void BeatmupBitmapResamplerDestroy(handle_t handle) {
    delete static_cast<Beatmup::BitmapResampler*>(handle);
}


handle_t BeatmupBitmapToolsMakeCopy1(handle_t _source, pixel_format_t _newPixelFormat) {
    Beatmup::AbstractBitmap * result = Beatmup::BitmapTools::makeCopy(*static_cast< Beatmup::AbstractBitmap* >(_source), static_cast<Beatmup::PixelFormat>(_newPixelFormat));
    return static_cast<handle_t>(result);
}


handle_t BeatmupBitmapToolsMakeCopy2(handle_t _source) {
    Beatmup::AbstractBitmap * result = Beatmup::BitmapTools::makeCopy(*static_cast< Beatmup::AbstractBitmap* >(_source));
    return static_cast<handle_t>(result);
}


handle_t BeatmupBitmapToolsChessboard(handle_t _env, int _width, int _height, int _cellSize, pixel_format_t _pixelFormat) {
    Beatmup::AbstractBitmap * result = Beatmup::BitmapTools::chessboard(*static_cast< Beatmup::Environment* >(_env), _width, _height, _cellSize, static_cast<Beatmup::PixelFormat>(_pixelFormat));
    return static_cast<handle_t>(result);
}


void BeatmupBitmapToolsMakeOpaque(handle_t _par1, int_rectangle_t _par2) {
    Beatmup::IntRectangle __par2(_par2._1, _par2._2, _par2._3, _par2._4);
    Beatmup::BitmapTools::makeOpaque(*static_cast< Beatmup::AbstractBitmap* >(_par1), __par2);
}


void BeatmupBitmapToolsInvert(handle_t _input, handle_t _output) {
    Beatmup::BitmapTools::invert(*static_cast< Beatmup::AbstractBitmap* >(_input), *static_cast< Beatmup::AbstractBitmap* >(_output));
}

