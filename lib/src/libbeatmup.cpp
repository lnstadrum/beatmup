#include "../include/libbeatmup.h"
#include "../../core/bitmap/crop.h"
#include "../../core/geometry.h"
#include "../../core/gpu/pipeline.h"
#include "../../core/scene/renderer.h"
#include "../../core/parallelism.h"
#include "../../core/bitmap/converter.h"
#include "../../core/pipelining/multitask.h"
#include "../../core/masking/flood_fill.h"
#include "../../core/scene/scene.h"
#include "../../core/exception.h"
#include "../../core/bitmap/abstract_bitmap.h"
#include "../../core/scene/rendering_context.h"
#include "../../core/pipelining/custom_pipeline.h"
#include "../../core/bitmap/resampler.h"
#include "../../core/contours/contours.h"
#include "../../core/filters/tuning.h"
#include "../../core/gpu/recycle_bin.h"
#include "../../core/filters/local/sepia.h"
#include "../../core/color/matrix.h"
#include "../../core/bitmap/operator.h"
#include "../../core/bitmap/tools.h"
#include "../../core/shading/shader_applicator.h"
#include "../../core/environment.h"
#include "../../core/bitmap/internal_bitmap.h"
#include "../../core/utils/image_resolution.h"
#include "../../core/shading/image_shader.h"
#include "../../core/basic_types.h"
#include "../../core/gpu/texture_handler.h"
#include "../../core/filters/local/color_matrix.h"


void BeatmupEnvironmentCreate1(handle_t * handle) {
    *handle = static_cast<handle_t>(new Beatmup::Environment());
}


void BeatmupEnvironmentCreate2(handle_t * handle, const beatmup_pool_index_t _numThreadPools, const char* _swapFilePrefix, const char* _swapFileSuffix) {
    *handle = static_cast<handle_t>(new Beatmup::Environment(_numThreadPools, _swapFilePrefix, _swapFileSuffix));
}


void BeatmupEnvironmentDestroy(handle_t handle) {
    if (handle) delete static_cast<Beatmup::Environment*>(handle);
}


float BeatmupEnvironmentPerformTask(handle_t handle, handle_t _task, const beatmup_pool_index_t _pool) {
    return static_cast<Beatmup::Environment*>(handle)->performTask(*static_cast<Beatmup::AbstractTask*>(_task), _pool);
}


void BeatmupEnvironmentRepeatTask(handle_t handle, handle_t _task, bool _abortCurrent, const beatmup_pool_index_t _pool) {
    static_cast<Beatmup::Environment*>(handle)->repeatTask(*static_cast<Beatmup::AbstractTask*>(_task), _abortCurrent, _pool);
}


beatmup_job_t BeatmupEnvironmentSubmitTask(handle_t handle, handle_t _task, const beatmup_pool_index_t _pool) {
    unsigned int result = static_cast<Beatmup::Environment*>(handle)->submitTask(*static_cast<Beatmup::AbstractTask*>(_task), _pool);
    return result;
}


beatmup_job_t BeatmupEnvironmentSubmitPersistentTask(handle_t handle, handle_t _task, const beatmup_pool_index_t _pool) {
    unsigned int result = static_cast<Beatmup::Environment*>(handle)->submitPersistentTask(*static_cast<Beatmup::AbstractTask*>(_task), _pool);
    return result;
}


void BeatmupEnvironmentWaitForJob(handle_t handle, beatmup_job_t _job, const beatmup_pool_index_t _pool) {
    static_cast<Beatmup::Environment*>(handle)->waitForJob(_job, _pool);
}


bool BeatmupEnvironmentAbortJob(handle_t handle, beatmup_job_t _job, const beatmup_pool_index_t _pool) {
    return static_cast<Beatmup::Environment*>(handle)->abortJob(_job, _pool);
}


void BeatmupEnvironmentWait(handle_t handle, const beatmup_pool_index_t _pool) {
    static_cast<Beatmup::Environment*>(handle)->wait(_pool);
}


bool BeatmupEnvironmentBusy(handle_t handle, const beatmup_pool_index_t _pool) {
    return static_cast<Beatmup::Environment*>(handle)->busy(_pool);
}


const beatmup_thread_index_t BeatmupEnvironmentMaxAllowedWorkerCount(chandle_t handle, const beatmup_pool_index_t _pool) {
    unsigned char result = static_cast<const Beatmup::Environment*>(handle)->maxAllowedWorkerCount(_pool);
    return result;
}


void BeatmupEnvironmentLimitWorkerCount(handle_t handle, beatmup_thread_index_t _maxValue, const beatmup_pool_index_t _pool) {
    static_cast<Beatmup::Environment*>(handle)->limitWorkerCount(_maxValue, _pool);
}


const beatmup_memchunk_t BeatmupEnvironmentAllocateMemory(handle_t handle, beatmup_msize_t _size) {
    unsigned int result = static_cast<Beatmup::Environment*>(handle)->allocateMemory(_size);
    return result;
}


void* BeatmupEnvironmentAcquireMemory(handle_t handle, beatmup_memchunk_t _chunk) {
    void* result = static_cast<Beatmup::Environment*>(handle)->acquireMemory(_chunk);
    return result;
}


void BeatmupEnvironmentReleaseMemory(handle_t handle, beatmup_memchunk_t _chunk, bool _unusedAnymore) {
    static_cast<Beatmup::Environment*>(handle)->releaseMemory(_chunk, _unusedAnymore);
}


void BeatmupEnvironmentFreeMemory(handle_t handle, beatmup_memchunk_t _chunk) {
    static_cast<Beatmup::Environment*>(handle)->freeMemory(_chunk);
}


beatmup_msize_t BeatmupEnvironmentSwapOnDisk(handle_t handle, beatmup_msize_t _howMuch) {
    unsigned int result = static_cast<Beatmup::Environment*>(handle)->swapOnDisk(_howMuch);
    return result;
}


bool BeatmupEnvironmentIsGpuQueried(chandle_t handle) {
    return static_cast<const Beatmup::Environment*>(handle)->isGpuQueried();
}


bool BeatmupEnvironmentIsGpuReady(chandle_t handle) {
    return static_cast<const Beatmup::Environment*>(handle)->isGpuReady();
}


bool BeatmupEnvironmentIsManagingThread(chandle_t handle) {
    return static_cast<const Beatmup::Environment*>(handle)->isManagingThread();
}


handle_t BeatmupEnvironmentGetGpuRecycleBin(chandle_t handle) {
    Beatmup::GL::RecycleBin* result = static_cast<const Beatmup::Environment*>(handle)->getGpuRecycleBin();
    return static_cast<handle_t>(result);
}


beatmup_msize_t BeatmupEnvironmentGetTotalRam() {
    unsigned int result = Beatmup::Environment::getTotalRam();
    return result;
}


void BeatmupEnvironmentWarmUpGpu(handle_t handle) {
    static_cast<Beatmup::Environment*>(handle)->warmUpGpu();
}


bool BeatmupImageResolutionIsEqual(const beatmup_image_resolution_t* handle, const beatmup_image_resolution_t* _par1) {
    Beatmup::ImageResolution self(handle->_1, handle->_2);
    return self.operator==(Beatmup::ImageResolution(_par1->_1, _par1->_2));
}


bool BeatmupImageResolutionIsNotEqual(const beatmup_image_resolution_t* handle, const beatmup_image_resolution_t* _par1) {
    Beatmup::ImageResolution self(handle->_1, handle->_2);
    return self.operator!=(Beatmup::ImageResolution(_par1->_1, _par1->_2));
}


beatmup_msize_t BeatmupImageResolutionNumPixels(const beatmup_image_resolution_t* handle) {
    Beatmup::ImageResolution self(handle->_1, handle->_2);
    unsigned int result = self.numPixels();
    return result;
}


float BeatmupImageResolutionMegaPixels(const beatmup_image_resolution_t* handle) {
    Beatmup::ImageResolution self(handle->_1, handle->_2);
    return self.megaPixels();
}


float BeatmupImageResolutionGetAspectRatio(const beatmup_image_resolution_t* handle) {
    Beatmup::ImageResolution self(handle->_1, handle->_2);
    return self.getAspectRatio();
}


float BeatmupImageResolutionGetInvAspectRatio(const beatmup_image_resolution_t* handle) {
    Beatmup::ImageResolution self(handle->_1, handle->_2);
    return self.getInvAspectRatio();
}


bool BeatmupImageResolutionFat(const beatmup_image_resolution_t* handle) {
    Beatmup::ImageResolution self(handle->_1, handle->_2);
    return self.fat();
}


beatmup_int_rectangle_t BeatmupImageResolutionClientRect(const beatmup_image_resolution_t* handle) {
    Beatmup::ImageResolution self(handle->_1, handle->_2);
    Beatmup::IntRectangle result = self.clientRect();
    return beatmup_int_rectangle_t{ result.getX1(), result.getY1(), result.getX2(), result.getY2() };
}


unsigned int BeatmupImageResolutionGetWidth(const beatmup_image_resolution_t* handle) {
    Beatmup::ImageResolution self(handle->_1, handle->_2);
    return self.getWidth();
}


unsigned int BeatmupImageResolutionGetHeight(const beatmup_image_resolution_t* handle) {
    Beatmup::ImageResolution self(handle->_1, handle->_2);
    return self.getHeight();
}


void BeatmupImageResolutionSet(beatmup_image_resolution_t* handle, unsigned int _width, unsigned int _height) {
    Beatmup::ImageResolution self(handle->_1, handle->_2);
    self.set(_width, _height);
    *handle = { self.getWidth(), self.getHeight() };
}


const int BeatmupAbstractBitmapGetDepth(chandle_t handle) {
    return static_cast<const Beatmup::AbstractBitmap*>(handle)->getDepth();
}


void BeatmupAbstractBitmapUnlockPixels(handle_t handle) {
    static_cast<Beatmup::AbstractBitmap*>(handle)->unlockPixels();
}


void BeatmupAbstractBitmapLockPixels(handle_t handle, beatmup_processing_target_t _par1) {
    static_cast<Beatmup::AbstractBitmap*>(handle)->lockPixels(static_cast<Beatmup::ProcessingTarget>(_par1));
}


void BeatmupAbstractBitmapInvalidate(handle_t handle, beatmup_processing_target_t _par1) {
    static_cast<Beatmup::AbstractBitmap*>(handle)->invalidate(static_cast<Beatmup::ProcessingTarget>(_par1));
}


bool BeatmupAbstractBitmapIsUpToDate(chandle_t handle, beatmup_processing_target_t _par1) {
    return static_cast<const Beatmup::AbstractBitmap*>(handle)->isUpToDate(static_cast<Beatmup::ProcessingTarget>(_par1));
}


bool BeatmupAbstractBitmapIsDirty(chandle_t handle) {
    return static_cast<const Beatmup::AbstractBitmap*>(handle)->isDirty();
}


int BeatmupAbstractBitmapGetPixelInt(chandle_t handle, int _x, int _y, int _cha) {
    return static_cast<const Beatmup::AbstractBitmap*>(handle)->getPixelInt(_x, _y, _cha);
}


const unsigned char BeatmupAbstractBitmapGetBitsPerPixel(chandle_t handle) {
    return static_cast<const Beatmup::AbstractBitmap*>(handle)->getBitsPerPixel();
}


const unsigned char BeatmupAbstractBitmapGetNumberOfChannels(chandle_t handle) {
    return static_cast<const Beatmup::AbstractBitmap*>(handle)->getNumberOfChannels();
}


const beatmup_image_resolution_t BeatmupAbstractBitmapGetSize(chandle_t handle) {
    Beatmup::ImageResolution result = static_cast<const Beatmup::AbstractBitmap*>(handle)->getSize();
    return beatmup_image_resolution_t{ result.getWidth(), result.getHeight() };
}


handle_t BeatmupAbstractBitmapGetEnvironment(chandle_t handle) {
    Beatmup::Environment& result = static_cast<const Beatmup::AbstractBitmap*>(handle)->getEnvironment();
    return static_cast<handle_t>(&result);
}


void BeatmupAbstractBitmapZero(handle_t handle) {
    static_cast<Beatmup::AbstractBitmap*>(handle)->zero();
}


bool BeatmupAbstractBitmapIsInteger1(chandle_t handle) {
    return static_cast<const Beatmup::AbstractBitmap*>(handle)->isInteger();
}


bool BeatmupAbstractBitmapIsFloat1(chandle_t handle) {
    return static_cast<const Beatmup::AbstractBitmap*>(handle)->isFloat();
}


bool BeatmupAbstractBitmapIsMask1(chandle_t handle) {
    return static_cast<const Beatmup::AbstractBitmap*>(handle)->isMask();
}


bool BeatmupAbstractBitmapIsInteger2(beatmup_pixel_format_t _pixelFormat) {
    return Beatmup::AbstractBitmap::isInteger(static_cast<Beatmup::PixelFormat>(_pixelFormat));
}


bool BeatmupAbstractBitmapIsFloat2(beatmup_pixel_format_t _pixelFormat) {
    return Beatmup::AbstractBitmap::isFloat(static_cast<Beatmup::PixelFormat>(_pixelFormat));
}


bool BeatmupAbstractBitmapIsMask2(beatmup_pixel_format_t _pixelFormat) {
    return Beatmup::AbstractBitmap::isMask(static_cast<Beatmup::PixelFormat>(_pixelFormat));
}


void BeatmupAbstractBitmapDestroy(handle_t handle) {
    if (handle) delete static_cast<Beatmup::AbstractBitmap*>(handle);
}


void BeatmupInternalBitmapCreate1(handle_t * handle, handle_t _env, beatmup_pixel_format_t _pixelFormat, int _width, int _height, bool _allocate) {
    *handle = static_cast<handle_t>(new Beatmup::InternalBitmap(*static_cast<Beatmup::Environment*>(_env), static_cast<Beatmup::PixelFormat>(_pixelFormat), _width, _height, _allocate));
}


void BeatmupInternalBitmapCreate2(handle_t * handle, handle_t _env, const char* _filename) {
    *handle = static_cast<handle_t>(new Beatmup::InternalBitmap(*static_cast<Beatmup::Environment*>(_env), _filename));
}


void BeatmupInternalBitmapDestroy(handle_t handle) {
    if (handle) delete static_cast<Beatmup::InternalBitmap*>(handle);
}


const beatmup_pixel_format_t BeatmupInternalBitmapGetPixelFormat(chandle_t handle) {
    Beatmup::PixelFormat result = static_cast<const Beatmup::InternalBitmap*>(handle)->getPixelFormat();
    return static_cast<const beatmup_pixel_format_t>(result);
}


const int BeatmupInternalBitmapGetWidth(chandle_t handle) {
    return static_cast<const Beatmup::InternalBitmap*>(handle)->getWidth();
}


const int BeatmupInternalBitmapGetHeight(chandle_t handle) {
    return static_cast<const Beatmup::InternalBitmap*>(handle)->getHeight();
}


const beatmup_msize_t BeatmupInternalBitmapGetMemorySize(chandle_t handle) {
    unsigned int result = static_cast<const Beatmup::InternalBitmap*>(handle)->getMemorySize();
    return result;
}


beatmup_pixbyte_t* BeatmupInternalBitmapGetData(chandle_t handle, int _x, int _y) {
    return static_cast<const Beatmup::InternalBitmap*>(handle)->getData(_x, _y);
}


void BeatmupInternalBitmapUnlockPixels(handle_t handle) {
    static_cast<Beatmup::InternalBitmap*>(handle)->unlockPixels();
}


void BeatmupInternalBitmapSaveBmp(handle_t handle, const char* _filename) {
    static_cast<Beatmup::InternalBitmap*>(handle)->saveBmp(_filename);
}


void BeatmupBitmapConverterCreate(handle_t * handle) {
    *handle = static_cast<handle_t>(new Beatmup::BitmapConverter());
}


void BeatmupBitmapConverterSetBitmaps(handle_t handle, handle_t _input, handle_t _output) {
    static_cast<Beatmup::BitmapConverter*>(handle)->setBitmaps(static_cast<Beatmup::AbstractBitmap*>(_input), static_cast<Beatmup::AbstractBitmap*>(_output));
}


beatmup_thread_index_t BeatmupBitmapConverterMaxAllowedThreads(chandle_t handle) {
    unsigned char result = static_cast<const Beatmup::BitmapConverter*>(handle)->maxAllowedThreads();
    return result;
}


beatmup_abstract_task_execution_target_t BeatmupBitmapConverterGetExecutionTarget(chandle_t handle) {
    Beatmup::AbstractTask::ExecutionTarget result = static_cast<const Beatmup::BitmapConverter*>(handle)->getExecutionTarget();
    return static_cast<beatmup_abstract_task_execution_target_t>(result);
}


void BeatmupBitmapConverterConvert(handle_t _input, handle_t _output) {
    Beatmup::BitmapConverter::convert(*static_cast<Beatmup::AbstractBitmap*>(_input), *static_cast<Beatmup::AbstractBitmap*>(_output));
}


void BeatmupBitmapConverterDestroy(handle_t handle) {
    if (handle) delete static_cast<Beatmup::BitmapConverter*>(handle);
}


void BeatmupCropCreate(handle_t * handle) {
    *handle = static_cast<handle_t>(new Beatmup::Crop());
}


beatmup_thread_index_t BeatmupCropMaxAllowedThreads(chandle_t handle) {
    unsigned char result = static_cast<const Beatmup::Crop*>(handle)->maxAllowedThreads();
    return result;
}


void BeatmupCropSetInput(handle_t handle, handle_t _input) {
    static_cast<Beatmup::Crop*>(handle)->setInput(static_cast<Beatmup::AbstractBitmap*>(_input));
}


void BeatmupCropSetOutput(handle_t handle, handle_t _output) {
    static_cast<Beatmup::Crop*>(handle)->setOutput(static_cast<Beatmup::AbstractBitmap*>(_output));
}


void BeatmupCropSetCropRect(handle_t handle, beatmup_int_rectangle_t _par1) {
    static_cast<Beatmup::Crop*>(handle)->setCropRect(Beatmup::IntRectangle(_par1._1, _par1._2, _par1._3, _par1._4));
}


void BeatmupCropSetOutputOrigin(handle_t handle, beatmup_int_point_t _par1) {
    static_cast<Beatmup::Crop*>(handle)->setOutputOrigin(Beatmup::IntPoint(_par1._1, _par1._2));
}


bool BeatmupCropIsFit(chandle_t handle) {
    return static_cast<const Beatmup::Crop*>(handle)->isFit();
}


handle_t BeatmupCropRun(handle_t _bitmap, beatmup_int_rectangle_t _clipRect) {
    Beatmup::AbstractBitmap* result = Beatmup::Crop::run(*static_cast<Beatmup::AbstractBitmap*>(_bitmap), Beatmup::IntRectangle(_clipRect._1, _clipRect._2, _clipRect._3, _clipRect._4));
    return static_cast<handle_t>(result);
}


void BeatmupCropDestroy(handle_t handle) {
    if (handle) delete static_cast<Beatmup::Crop*>(handle);
}


void BeatmupBitmapBinaryOperationCreate(handle_t * handle) {
    *handle = static_cast<handle_t>(new Beatmup::BitmapBinaryOperation());
}


void BeatmupBitmapBinaryOperationSetOperand1(handle_t handle, handle_t _op1) {
    static_cast<Beatmup::BitmapBinaryOperation*>(handle)->setOperand1(static_cast<Beatmup::AbstractBitmap*>(_op1));
}


void BeatmupBitmapBinaryOperationSetOperand2(handle_t handle, handle_t _op2) {
    static_cast<Beatmup::BitmapBinaryOperation*>(handle)->setOperand2(static_cast<Beatmup::AbstractBitmap*>(_op2));
}


void BeatmupBitmapBinaryOperationSetOutput(handle_t handle, handle_t _output) {
    static_cast<Beatmup::BitmapBinaryOperation*>(handle)->setOutput(static_cast<Beatmup::AbstractBitmap*>(_output));
}


void BeatmupBitmapBinaryOperationSetOperation(handle_t handle, const beatmup_bitmap_binary_operation_operation_t _operation) {
    static_cast<Beatmup::BitmapBinaryOperation*>(handle)->setOperation(static_cast<Beatmup::BitmapBinaryOperation::Operation>(_operation));
}


void BeatmupBitmapBinaryOperationSetCropSize(handle_t handle, int _width, int _height) {
    static_cast<Beatmup::BitmapBinaryOperation*>(handle)->setCropSize(_width, _height);
}


void BeatmupBitmapBinaryOperationSetOp1Origin(handle_t handle, const beatmup_int_point_t _origin) {
    static_cast<Beatmup::BitmapBinaryOperation*>(handle)->setOp1Origin(Beatmup::IntPoint(_origin._1, _origin._2));
}


void BeatmupBitmapBinaryOperationSetOp2Origin(handle_t handle, const beatmup_int_point_t _origin) {
    static_cast<Beatmup::BitmapBinaryOperation*>(handle)->setOp2Origin(Beatmup::IntPoint(_origin._1, _origin._2));
}


void BeatmupBitmapBinaryOperationSetOutputOrigin(handle_t handle, const beatmup_int_point_t _origin) {
    static_cast<Beatmup::BitmapBinaryOperation*>(handle)->setOutputOrigin(Beatmup::IntPoint(_origin._1, _origin._2));
}


void BeatmupBitmapBinaryOperationResetCrop(handle_t handle) {
    static_cast<Beatmup::BitmapBinaryOperation*>(handle)->resetCrop();
}


int BeatmupBitmapBinaryOperationGetCropWidth(chandle_t handle) {
    return static_cast<const Beatmup::BitmapBinaryOperation*>(handle)->getCropWidth();
}


int BeatmupBitmapBinaryOperationGetCropHeight(chandle_t handle) {
    return static_cast<const Beatmup::BitmapBinaryOperation*>(handle)->getCropHeight();
}


const beatmup_int_point_t BeatmupBitmapBinaryOperationGetOp1Origin(chandle_t handle) {
    Beatmup::IntPoint result = static_cast<const Beatmup::BitmapBinaryOperation*>(handle)->getOp1Origin();
    return beatmup_int_point_t{ result.getX(), result.getY() };
}


const beatmup_int_point_t BeatmupBitmapBinaryOperationGetOp2Origin(chandle_t handle) {
    Beatmup::IntPoint result = static_cast<const Beatmup::BitmapBinaryOperation*>(handle)->getOp2Origin();
    return beatmup_int_point_t{ result.getX(), result.getY() };
}


const beatmup_int_point_t BeatmupBitmapBinaryOperationGetOutputOrigin(chandle_t handle) {
    Beatmup::IntPoint result = static_cast<const Beatmup::BitmapBinaryOperation*>(handle)->getOutputOrigin();
    return beatmup_int_point_t{ result.getX(), result.getY() };
}


beatmup_thread_index_t BeatmupBitmapBinaryOperationMaxAllowedThreads(chandle_t handle) {
    unsigned char result = static_cast<const Beatmup::BitmapBinaryOperation*>(handle)->maxAllowedThreads();
    return result;
}


void BeatmupBitmapBinaryOperationDestroy(handle_t handle) {
    if (handle) delete static_cast<Beatmup::BitmapBinaryOperation*>(handle);
}


void BeatmupBitmapResamplerCreate(handle_t * handle) {
    *handle = static_cast<handle_t>(new Beatmup::BitmapResampler());
}


void BeatmupBitmapResamplerSetBitmaps(handle_t handle, handle_t _input, handle_t _output) {
    static_cast<Beatmup::BitmapResampler*>(handle)->setBitmaps(static_cast<Beatmup::AbstractBitmap*>(_input), static_cast<Beatmup::AbstractBitmap*>(_output));
}


void BeatmupBitmapResamplerSetInputRect(handle_t handle, const beatmup_int_rectangle_t* _rect) {
    static_cast<Beatmup::BitmapResampler*>(handle)->setInputRect(Beatmup::IntRectangle(_rect->_1, _rect->_2, _rect->_3, _rect->_4));
}


void BeatmupBitmapResamplerSetOutputRect(handle_t handle, const beatmup_int_rectangle_t* _rect) {
    static_cast<Beatmup::BitmapResampler*>(handle)->setOutputRect(Beatmup::IntRectangle(_rect->_1, _rect->_2, _rect->_3, _rect->_4));
}


beatmup_int_rectangle_t BeatmupBitmapResamplerGetInputRect(chandle_t handle) {
    Beatmup::IntRectangle result = static_cast<const Beatmup::BitmapResampler*>(handle)->getInputRect();
    return beatmup_int_rectangle_t{ result.getX1(), result.getY1(), result.getX2(), result.getY2() };
}


beatmup_int_rectangle_t BeatmupBitmapResamplerGetOutputRect(chandle_t handle) {
    Beatmup::IntRectangle result = static_cast<const Beatmup::BitmapResampler*>(handle)->getOutputRect();
    return beatmup_int_rectangle_t{ result.getX1(), result.getY1(), result.getX2(), result.getY2() };
}


beatmup_thread_index_t BeatmupBitmapResamplerMaxAllowedThreads(chandle_t handle) {
    unsigned char result = static_cast<const Beatmup::BitmapResampler*>(handle)->maxAllowedThreads();
    return result;
}


void BeatmupBitmapResamplerDestroy(handle_t handle) {
    if (handle) delete static_cast<Beatmup::BitmapResampler*>(handle);
}


handle_t BeatmupBitmapToolsMakeCopy1(handle_t _source, beatmup_pixel_format_t _newPixelFormat) {
    Beatmup::AbstractBitmap* result = Beatmup::BitmapTools::makeCopy(*static_cast<Beatmup::AbstractBitmap*>(_source), static_cast<Beatmup::PixelFormat>(_newPixelFormat));
    return static_cast<handle_t>(result);
}


handle_t BeatmupBitmapToolsMakeCopy2(handle_t _source) {
    Beatmup::AbstractBitmap* result = Beatmup::BitmapTools::makeCopy(*static_cast<Beatmup::AbstractBitmap*>(_source));
    return static_cast<handle_t>(result);
}


handle_t BeatmupBitmapToolsChessboard(handle_t _env, int _width, int _height, int _cellSize, beatmup_pixel_format_t _pixelFormat) {
    Beatmup::AbstractBitmap* result = Beatmup::BitmapTools::chessboard(*static_cast<Beatmup::Environment*>(_env), _width, _height, _cellSize, static_cast<Beatmup::PixelFormat>(_pixelFormat));
    return static_cast<handle_t>(result);
}


void BeatmupBitmapToolsMakeOpaque(handle_t _par1, beatmup_int_rectangle_t _par2) {
    Beatmup::BitmapTools::makeOpaque(*static_cast<Beatmup::AbstractBitmap*>(_par1), Beatmup::IntRectangle(_par2._1, _par2._2, _par2._3, _par2._4));
}


void BeatmupBitmapToolsInvert(handle_t _input, handle_t _output) {
    Beatmup::BitmapTools::invert(*static_cast<Beatmup::AbstractBitmap*>(_input), *static_cast<Beatmup::AbstractBitmap*>(_output));
}


void BeatmupFiltersImageTuningCreate(handle_t * handle) {
    *handle = static_cast<handle_t>(new Beatmup::Filters::ImageTuning());
}


void BeatmupFiltersImageTuningSetBitmaps(handle_t handle, handle_t _input, handle_t _output) {
    static_cast<Beatmup::Filters::ImageTuning*>(handle)->setBitmaps(static_cast<Beatmup::AbstractBitmap*>(_input), static_cast<Beatmup::AbstractBitmap*>(_output));
}


beatmup_thread_index_t BeatmupFiltersImageTuningMaxAllowedThreads(chandle_t handle) {
    unsigned char result = static_cast<const Beatmup::Filters::ImageTuning*>(handle)->maxAllowedThreads();
    return result;
}


void BeatmupFiltersImageTuningSetHueOffset(handle_t handle, float _val) {
    static_cast<Beatmup::Filters::ImageTuning*>(handle)->setHueOffset(_val);
}


void BeatmupFiltersImageTuningSetSaturationFactor(handle_t handle, float _val) {
    static_cast<Beatmup::Filters::ImageTuning*>(handle)->setSaturationFactor(_val);
}


void BeatmupFiltersImageTuningSetValueFactor(handle_t handle, float _val) {
    static_cast<Beatmup::Filters::ImageTuning*>(handle)->setValueFactor(_val);
}


void BeatmupFiltersImageTuningSetBrightness(handle_t handle, float _val) {
    static_cast<Beatmup::Filters::ImageTuning*>(handle)->setBrightness(_val);
}


void BeatmupFiltersImageTuningSetContrast(handle_t handle, float _val) {
    static_cast<Beatmup::Filters::ImageTuning*>(handle)->setContrast(_val);
}


float BeatmupFiltersImageTuningGetHueOffset(chandle_t handle) {
    return static_cast<const Beatmup::Filters::ImageTuning*>(handle)->getHueOffset();
}


float BeatmupFiltersImageTuningGetSaturationFactor(chandle_t handle) {
    return static_cast<const Beatmup::Filters::ImageTuning*>(handle)->getSaturationFactor();
}


float BeatmupFiltersImageTuningGetValueFactor(chandle_t handle) {
    return static_cast<const Beatmup::Filters::ImageTuning*>(handle)->getValueFactor();
}


float BeatmupFiltersImageTuningGetBrightness(chandle_t handle) {
    return static_cast<const Beatmup::Filters::ImageTuning*>(handle)->getBrightness();
}


float BeatmupFiltersImageTuningGetContrast(chandle_t handle) {
    return static_cast<const Beatmup::Filters::ImageTuning*>(handle)->getContrast();
}


void BeatmupFiltersImageTuningDestroy(handle_t handle) {
    if (handle) delete static_cast<Beatmup::Filters::ImageTuning*>(handle);
}


void BeatmupFiltersColorMatrixCreate(handle_t * handle) {
    *handle = static_cast<handle_t>(new Beatmup::Filters::ColorMatrix());
}


void BeatmupFiltersColorMatrixApply(handle_t handle, int _startx, int _starty, beatmup_msize_t _nPix, handle_t _thread) {
    static_cast<Beatmup::Filters::ColorMatrix*>(handle)->apply(_startx, _starty, _nPix, *static_cast<Beatmup::TaskThread*>(_thread));
}


bool BeatmupFiltersColorMatrixIsIntegerApproximationsAllowed(chandle_t handle) {
    return static_cast<const Beatmup::Filters::ColorMatrix*>(handle)->isIntegerApproximationsAllowed();
}


void BeatmupFiltersColorMatrixAllowIntegerApproximations(handle_t handle, bool _allow) {
    static_cast<Beatmup::Filters::ColorMatrix*>(handle)->allowIntegerApproximations(_allow);
}


handle_t BeatmupFiltersColorMatrixGetMatrix(handle_t handle) {
    Beatmup::Color::Matrix& result = static_cast<Beatmup::Filters::ColorMatrix*>(handle)->getMatrix();
    return static_cast<handle_t>(&result);
}


void BeatmupFiltersColorMatrixSetCoefficients(handle_t handle, int _outChannel, float _add, float _inR, float _inG, float _inB, float _inA) {
    static_cast<Beatmup::Filters::ColorMatrix*>(handle)->setCoefficients(_outChannel, _add, _inR, _inG, _inB, _inA);
}


void BeatmupFiltersColorMatrixSetHSVCorrection(handle_t handle, float _addHueDegrees, float _scaleSat, float _scaleVal) {
    static_cast<Beatmup::Filters::ColorMatrix*>(handle)->setHSVCorrection(_addHueDegrees, _scaleSat, _scaleVal);
}


void BeatmupFiltersColorMatrixSetColorInversion(handle_t handle, beatmup_color3f_t _preservedHue, float _scaleSat, float _scaleVal) {
    static_cast<Beatmup::Filters::ColorMatrix*>(handle)->setColorInversion(Beatmup::color3f{ _preservedHue.r, _preservedHue.g, _preservedHue.b }, _scaleSat, _scaleVal);
}


void BeatmupFiltersColorMatrixDestroy(handle_t handle) {
    if (handle) delete static_cast<Beatmup::Filters::ColorMatrix*>(handle);
}


void BeatmupIntegerContour2DCreate(handle_t * handle) {
    *handle = static_cast<handle_t>(new Beatmup::IntegerContour2D());
}


void BeatmupIntegerContour2DAddPoint(handle_t handle, int _x, int _y) {
    static_cast<Beatmup::IntegerContour2D*>(handle)->addPoint(_x, _y);
}


void BeatmupIntegerContour2DClear(handle_t handle) {
    static_cast<Beatmup::IntegerContour2D*>(handle)->clear();
}


int BeatmupIntegerContour2DGetPointCount(chandle_t handle) {
    return static_cast<const Beatmup::IntegerContour2D*>(handle)->getPointCount();
}


float BeatmupIntegerContour2DGetLenght(chandle_t handle) {
    return static_cast<const Beatmup::IntegerContour2D*>(handle)->getLenght();
}


beatmup_int_point_t BeatmupIntegerContour2DGetPoint(chandle_t handle, int _index) {
    Beatmup::IntPoint result = static_cast<const Beatmup::IntegerContour2D*>(handle)->getPoint(_index);
    return beatmup_int_point_t{ result.getX(), result.getY() };
}


void BeatmupIntegerContour2DBadSeedPointCreate1(handle_t * handle) {
    *handle = static_cast<handle_t>(new Beatmup::IntegerContour2D::BadSeedPoint());
}


void BeatmupIntegerContour2DBadSeedPointCreate2(handle_t * handle, int _x, int _y, bool _lefttop, bool _righttop, bool _leftbottom, bool _rightbottom) {
    *handle = static_cast<handle_t>(new Beatmup::IntegerContour2D::BadSeedPoint(_x, _y, _lefttop, _righttop, _leftbottom, _rightbottom));
}


void BeatmupIntegerContour2DBadSeedPointDestroy(handle_t handle) {
    if (handle) delete static_cast<Beatmup::IntegerContour2D::BadSeedPoint*>(handle);
}


void BeatmupIntegerContour2DDestroy(handle_t handle) {
    if (handle) delete static_cast<Beatmup::IntegerContour2D*>(handle);
}


void BeatmupFloodFillCreate(handle_t * handle) {
    *handle = static_cast<handle_t>(new Beatmup::FloodFill());
}


void BeatmupFloodFillDestroy(handle_t handle) {
    if (handle) delete static_cast<Beatmup::FloodFill*>(handle);
}


handle_t BeatmupFloodFillGetInput(chandle_t handle) {
    Beatmup::AbstractBitmap* result = static_cast<const Beatmup::FloodFill*>(handle)->getInput();
    return static_cast<handle_t>(result);
}


handle_t BeatmupFloodFillGetOutput(chandle_t handle) {
    Beatmup::AbstractBitmap* result = static_cast<const Beatmup::FloodFill*>(handle)->getOutput();
    return static_cast<handle_t>(result);
}


beatmup_int_rectangle_t BeatmupFloodFillGetBounds(chandle_t handle) {
    Beatmup::IntRectangle result = static_cast<const Beatmup::FloodFill*>(handle)->getBounds();
    return beatmup_int_rectangle_t{ result.getX1(), result.getY1(), result.getX2(), result.getY2() };
}


int BeatmupFloodFillGetContourCount(chandle_t handle) {
    return static_cast<const Beatmup::FloodFill*>(handle)->getContourCount();
}


chandle_t BeatmupFloodFillGetContour(chandle_t handle, int _contourIndex) {
    const Beatmup::IntegerContour2D& result = static_cast<const Beatmup::FloodFill*>(handle)->getContour(_contourIndex);
    return static_cast<chandle_t>(&result);
}


void BeatmupFloodFillSetInput(handle_t handle, handle_t _par1) {
    static_cast<Beatmup::FloodFill*>(handle)->setInput(*static_cast<Beatmup::AbstractBitmap*>(_par1));
}


void BeatmupFloodFillSetOutput(handle_t handle, handle_t _par1) {
    static_cast<Beatmup::FloodFill*>(handle)->setOutput(*static_cast<Beatmup::AbstractBitmap*>(_par1));
}


void BeatmupFloodFillSetMaskPos(handle_t handle, beatmup_int_point_t* _par1) {
    Beatmup::IntPoint __par1 = Beatmup::IntPoint(_par1->_1, _par1->_2);
    static_cast<Beatmup::FloodFill*>(handle)->setMaskPos(__par1);
    *_par1 = beatmup_int_point_t{ __par1.getX(), __par1.getY() };
}


void BeatmupFloodFillSetSeeds2(handle_t handle, const int* _seedsXY, int _seedCount) {
    static_cast<Beatmup::FloodFill*>(handle)->setSeeds(_seedsXY, _seedCount);
}


void BeatmupFloodFillSetTolerance(handle_t handle, float _tolerance) {
    static_cast<Beatmup::FloodFill*>(handle)->setTolerance(_tolerance);
}


void BeatmupFloodFillSetBorderPostprocessing(handle_t handle, beatmup_flood_fill_border_morphology_t _operation, float _holdRadius, float _releaseRadius) {
    static_cast<Beatmup::FloodFill*>(handle)->setBorderPostprocessing(static_cast<Beatmup::FloodFill::BorderMorphology>(_operation), _holdRadius, _releaseRadius);
}


void BeatmupFloodFillSetComputeContours(handle_t handle, bool _par1) {
    static_cast<Beatmup::FloodFill*>(handle)->setComputeContours(_par1);
}


beatmup_thread_index_t BeatmupFloodFillMaxAllowedThreads(chandle_t handle) {
    unsigned char result = static_cast<const Beatmup::FloodFill*>(handle)->maxAllowedThreads();
    return result;
}


bool BeatmupFloodFillProcess(handle_t handle, handle_t _thread) {
    return static_cast<Beatmup::FloodFill*>(handle)->process(*static_cast<Beatmup::TaskThread*>(_thread));
}


void BeatmupFloodFillBeforeProcessing(handle_t handle, beatmup_thread_index_t _threadCount, handle_t _gpu) {
    static_cast<Beatmup::FloodFill*>(handle)->beforeProcessing(_threadCount, static_cast<Beatmup::GraphicPipeline*>(_gpu));
}


void BeatmupFloodFillAfterProcessing(handle_t handle, beatmup_thread_index_t _threadCount, bool _aborted) {
    static_cast<Beatmup::FloodFill*>(handle)->afterProcessing(_threadCount, _aborted);
}


void BeatmupMultitaskCreate(handle_t * handle) {
    *handle = static_cast<handle_t>(new Beatmup::Multitask());
}


beatmup_multitask_repetition_policy_t BeatmupMultitaskGetRepetitionPolicy(handle_t handle, chandle_t _task) {
    Beatmup::Multitask::RepetitionPolicy result = static_cast<Beatmup::Multitask*>(handle)->getRepetitionPolicy(*static_cast<const Beatmup::CustomPipeline::TaskHolder*>(_task));
    return static_cast<beatmup_multitask_repetition_policy_t>(result);
}


void BeatmupMultitaskSetRepetitionPolicy(handle_t handle, handle_t _task, beatmup_multitask_repetition_policy_t _policy) {
    static_cast<Beatmup::Multitask*>(handle)->setRepetitionPolicy(*static_cast<Beatmup::CustomPipeline::TaskHolder*>(_task), static_cast<Beatmup::Multitask::RepetitionPolicy>(_policy));
}


void BeatmupMultitaskDestroy(handle_t handle) {
    if (handle) delete static_cast<Beatmup::Multitask*>(handle);
}


void BeatmupCustomPipelineDestroy(handle_t handle) {
    if (handle) delete static_cast<Beatmup::CustomPipeline*>(handle);
}


int BeatmupCustomPipelineGetTaskCount(chandle_t handle) {
    return static_cast<const Beatmup::CustomPipeline*>(handle)->getTaskCount();
}


handle_t BeatmupCustomPipelineGetTask(chandle_t handle, int _index) {
    Beatmup::CustomPipeline::TaskHolder& result = static_cast<const Beatmup::CustomPipeline*>(handle)->getTask(_index);
    return static_cast<handle_t>(&result);
}


int BeatmupCustomPipelineGetTaskIndex(handle_t handle, chandle_t _par1) {
    return static_cast<Beatmup::CustomPipeline*>(handle)->getTaskIndex(*static_cast<const Beatmup::CustomPipeline::TaskHolder*>(_par1));
}


handle_t BeatmupCustomPipelineAddTask(handle_t handle, handle_t _task) {
    Beatmup::CustomPipeline::TaskHolder& result = static_cast<Beatmup::CustomPipeline*>(handle)->addTask(*static_cast<Beatmup::AbstractTask*>(_task));
    return static_cast<handle_t>(&result);
}


handle_t BeatmupCustomPipelineInsertTask(handle_t handle, handle_t _task, chandle_t _succeedingHoder) {
    Beatmup::CustomPipeline::TaskHolder& result = static_cast<Beatmup::CustomPipeline*>(handle)->insertTask(*static_cast<Beatmup::AbstractTask*>(_task), *static_cast<const Beatmup::CustomPipeline::TaskHolder*>(_succeedingHoder));
    return static_cast<handle_t>(&result);
}


bool BeatmupCustomPipelineRemoveTask(handle_t handle, chandle_t _task) {
    return static_cast<Beatmup::CustomPipeline*>(handle)->removeTask(*static_cast<const Beatmup::CustomPipeline::TaskHolder*>(_task));
}


void BeatmupCustomPipelineMeasure(handle_t handle) {
    static_cast<Beatmup::CustomPipeline*>(handle)->measure();
}


handle_t BeatmupCustomPipelineTaskHolderGetTask(chandle_t handle) {
    Beatmup::AbstractTask& result = static_cast<const Beatmup::CustomPipeline::TaskHolder*>(handle)->getTask();
    return static_cast<handle_t>(&result);
}


void BeatmupCustomPipelinePipelineNotReadyCreate(handle_t * handle, const char* _message) {
    *handle = static_cast<handle_t>(new Beatmup::CustomPipeline::PipelineNotReady(_message));
}


void BeatmupCustomPipelinePipelineNotReadyDestroy(handle_t handle) {
    if (handle) delete static_cast<Beatmup::CustomPipeline::PipelineNotReady*>(handle);
}


bool BeatmupIsEqual(chandle_t _par1, chandle_t _par2) {
    return Beatmup::operator==(*static_cast<const Beatmup::CustomPipeline::TaskHolder*>(_par1), *static_cast<const Beatmup::CustomPipeline::TaskHolder*>(_par2));
}


beatmup_scene_layer_type_t BeatmupSceneLayerGetType(chandle_t handle) {
    Beatmup::Scene::Layer::Type result = static_cast<const Beatmup::Scene::Layer*>(handle)->getType();
    return static_cast<beatmup_scene_layer_type_t>(result);
}


const char* BeatmupSceneLayerGetName(chandle_t handle) {
    return static_cast<const Beatmup::Scene::Layer*>(handle)->getName();
}


void BeatmupSceneLayerSetName(handle_t handle, const char* _name) {
    static_cast<Beatmup::Scene::Layer*>(handle)->setName(_name);
}


handle_t BeatmupSceneLayerGetMapping1(handle_t handle) {
    Beatmup::AffineMapping& result = static_cast<Beatmup::Scene::Layer*>(handle)->getMapping();
    return static_cast<handle_t>(&result);
}


chandle_t BeatmupSceneLayerGetMapping2(chandle_t handle) {
    const Beatmup::AffineMapping& result = static_cast<const Beatmup::Scene::Layer*>(handle)->getMapping();
    return static_cast<chandle_t>(&result);
}


void BeatmupSceneLayerSetMapping(handle_t handle, chandle_t _mapping) {
    static_cast<Beatmup::Scene::Layer*>(handle)->setMapping(*static_cast<const Beatmup::AffineMapping*>(_mapping));
}


bool BeatmupSceneLayerTestPoint(chandle_t handle, float _x, float _y) {
    return static_cast<const Beatmup::Scene::Layer*>(handle)->testPoint(_x, _y);
}


handle_t BeatmupSceneLayerGetChild(chandle_t handle, float _x, float _y, unsigned int _recursionDepth) {
    Beatmup::Scene::Layer* result = static_cast<const Beatmup::Scene::Layer*>(handle)->getChild(_x, _y, _recursionDepth);
    return static_cast<handle_t>(result);
}


bool BeatmupSceneLayerIsVisible(chandle_t handle) {
    return static_cast<const Beatmup::Scene::Layer*>(handle)->isVisible();
}


bool BeatmupSceneLayerIsPhantom(chandle_t handle) {
    return static_cast<const Beatmup::Scene::Layer*>(handle)->isPhantom();
}


void BeatmupSceneLayerSetVisible(handle_t handle, bool _visible) {
    static_cast<Beatmup::Scene::Layer*>(handle)->setVisible(_visible);
}


void BeatmupSceneLayerSetPhantom(handle_t handle, bool _phantom) {
    static_cast<Beatmup::Scene::Layer*>(handle)->setPhantom(_phantom);
}


chandle_t BeatmupSceneSceneLayerGetScene(chandle_t handle) {
    const Beatmup::Scene& result = static_cast<const Beatmup::Scene::SceneLayer*>(handle)->getScene();
    return static_cast<chandle_t>(&result);
}


bool BeatmupSceneSceneLayerTestPoint(chandle_t handle, float _x, float _y) {
    return static_cast<const Beatmup::Scene::SceneLayer*>(handle)->testPoint(_x, _y);
}


handle_t BeatmupSceneSceneLayerGetChild(chandle_t handle, float _x, float _y, unsigned int _recursionDepth) {
    Beatmup::Scene::Layer* result = static_cast<const Beatmup::Scene::SceneLayer*>(handle)->getChild(_x, _y, _recursionDepth);
    return static_cast<handle_t>(result);
}


bool BeatmupSceneBitmapLayerTestPoint(chandle_t handle, float _x, float _y) {
    return static_cast<const Beatmup::Scene::BitmapLayer*>(handle)->testPoint(_x, _y);
}


beatmup_scene_bitmap_layer_image_source_t BeatmupSceneBitmapLayerGetImageSource(chandle_t handle) {
    Beatmup::Scene::BitmapLayer::ImageSource result = static_cast<const Beatmup::Scene::BitmapLayer*>(handle)->getImageSource();
    return static_cast<beatmup_scene_bitmap_layer_image_source_t>(result);
}


void BeatmupSceneBitmapLayerSetImageSource(handle_t handle, beatmup_scene_bitmap_layer_image_source_t _imageSource) {
    static_cast<Beatmup::Scene::BitmapLayer*>(handle)->setImageSource(static_cast<Beatmup::Scene::BitmapLayer::ImageSource>(_imageSource));
}


const beatmup_bitmap_ptr_t BeatmupSceneBitmapLayerGetBitmap(chandle_t handle) {
    Beatmup::AbstractBitmap *const result = static_cast<const Beatmup::Scene::BitmapLayer*>(handle)->getBitmap();
    return static_cast<handle_t>(result);
}


void BeatmupSceneBitmapLayerSetBitmap(handle_t handle, beatmup_bitmap_ptr_t _bitmap) {
    static_cast<Beatmup::Scene::BitmapLayer*>(handle)->setBitmap(static_cast<Beatmup::AbstractBitmap*>(_bitmap));
}


handle_t BeatmupSceneBitmapLayerGetBitmapMapping1(handle_t handle) {
    Beatmup::AffineMapping& result = static_cast<Beatmup::Scene::BitmapLayer*>(handle)->getBitmapMapping();
    return static_cast<handle_t>(&result);
}


chandle_t BeatmupSceneBitmapLayerGetBitmapMapping2(chandle_t handle) {
    const Beatmup::AffineMapping& result = static_cast<const Beatmup::Scene::BitmapLayer*>(handle)->getBitmapMapping();
    return static_cast<chandle_t>(&result);
}


void BeatmupSceneBitmapLayerSetBitmapMapping(handle_t handle, chandle_t _mapping) {
    static_cast<Beatmup::Scene::BitmapLayer*>(handle)->setBitmapMapping(*static_cast<const Beatmup::AffineMapping*>(_mapping));
}


beatmup_color4i_t BeatmupSceneBitmapLayerGetModulationColor(chandle_t handle) {
    Beatmup::color4i result = static_cast<const Beatmup::Scene::BitmapLayer*>(handle)->getModulationColor();
    return beatmup_color4i_t{ result.r, result.g, result.b, result.a };
}


void BeatmupSceneBitmapLayerSetModulationColor(handle_t handle, beatmup_color4i_t _color) {
    static_cast<Beatmup::Scene::BitmapLayer*>(handle)->setModulationColor(Beatmup::color4i{ _color.r, _color.g, _color.b, _color.a });
}


handle_t BeatmupSceneCustomMaskedBitmapLayerGetMaskMapping1(handle_t handle) {
    Beatmup::AffineMapping& result = static_cast<Beatmup::Scene::CustomMaskedBitmapLayer*>(handle)->getMaskMapping();
    return static_cast<handle_t>(&result);
}


chandle_t BeatmupSceneCustomMaskedBitmapLayerGetMaskMapping2(chandle_t handle) {
    const Beatmup::AffineMapping& result = static_cast<const Beatmup::Scene::CustomMaskedBitmapLayer*>(handle)->getMaskMapping();
    return static_cast<chandle_t>(&result);
}


void BeatmupSceneCustomMaskedBitmapLayerSetMaskMapping(handle_t handle, chandle_t _mapping) {
    static_cast<Beatmup::Scene::CustomMaskedBitmapLayer*>(handle)->setMaskMapping(*static_cast<const Beatmup::AffineMapping*>(_mapping));
}


beatmup_color4i_t BeatmupSceneCustomMaskedBitmapLayerGetBackgroundColor(chandle_t handle) {
    Beatmup::color4i result = static_cast<const Beatmup::Scene::CustomMaskedBitmapLayer*>(handle)->getBackgroundColor();
    return beatmup_color4i_t{ result.r, result.g, result.b, result.a };
}


void BeatmupSceneCustomMaskedBitmapLayerSetBackgroundColor(handle_t handle, beatmup_color4i_t _color) {
    static_cast<Beatmup::Scene::CustomMaskedBitmapLayer*>(handle)->setBackgroundColor(Beatmup::color4i{ _color.r, _color.g, _color.b, _color.a });
}


const beatmup_bitmap_ptr_t BeatmupSceneMaskedBitmapLayerGetMask(chandle_t handle) {
    Beatmup::AbstractBitmap *const result = static_cast<const Beatmup::Scene::MaskedBitmapLayer*>(handle)->getMask();
    return static_cast<handle_t>(result);
}


void BeatmupSceneMaskedBitmapLayerSetMask(handle_t handle, beatmup_bitmap_ptr_t _mask) {
    static_cast<Beatmup::Scene::MaskedBitmapLayer*>(handle)->setMask(static_cast<Beatmup::AbstractBitmap*>(_mask));
}


bool BeatmupSceneMaskedBitmapLayerTestPoint(chandle_t handle, float _x, float _y) {
    return static_cast<const Beatmup::Scene::MaskedBitmapLayer*>(handle)->testPoint(_x, _y);
}


float BeatmupSceneShapedBitmapLayerGetBorderWidth(chandle_t handle) {
    return static_cast<const Beatmup::Scene::ShapedBitmapLayer*>(handle)->getBorderWidth();
}


void BeatmupSceneShapedBitmapLayerSetBorderWidth(handle_t handle, float _borderWidth) {
    static_cast<Beatmup::Scene::ShapedBitmapLayer*>(handle)->setBorderWidth(_borderWidth);
}


float BeatmupSceneShapedBitmapLayerGetSlopeWidth(chandle_t handle) {
    return static_cast<const Beatmup::Scene::ShapedBitmapLayer*>(handle)->getSlopeWidth();
}


void BeatmupSceneShapedBitmapLayerSetSlopeWidth(handle_t handle, float _slopeWidth) {
    static_cast<Beatmup::Scene::ShapedBitmapLayer*>(handle)->setSlopeWidth(_slopeWidth);
}


float BeatmupSceneShapedBitmapLayerGetCornerRadius(chandle_t handle) {
    return static_cast<const Beatmup::Scene::ShapedBitmapLayer*>(handle)->getCornerRadius();
}


void BeatmupSceneShapedBitmapLayerSetCornerRadius(handle_t handle, float _cornerRadius) {
    static_cast<Beatmup::Scene::ShapedBitmapLayer*>(handle)->setCornerRadius(_cornerRadius);
}


bool BeatmupSceneShapedBitmapLayerGetInPixels(chandle_t handle) {
    return static_cast<const Beatmup::Scene::ShapedBitmapLayer*>(handle)->getInPixels();
}


void BeatmupSceneShapedBitmapLayerSetInPixels(handle_t handle, bool _inPixels) {
    static_cast<Beatmup::Scene::ShapedBitmapLayer*>(handle)->setInPixels(_inPixels);
}


bool BeatmupSceneShapedBitmapLayerTestPoint(chandle_t handle, float _x, float _y) {
    return static_cast<const Beatmup::Scene::ShapedBitmapLayer*>(handle)->testPoint(_x, _y);
}


handle_t BeatmupSceneShadedBitmapLayerGetLayerShader(chandle_t handle) {
    Beatmup::ImageShader* result = static_cast<const Beatmup::Scene::ShadedBitmapLayer*>(handle)->getLayerShader();
    return static_cast<handle_t>(result);
}


void BeatmupSceneShadedBitmapLayerSetLayerShader(handle_t handle, handle_t _shader) {
    static_cast<Beatmup::Scene::ShadedBitmapLayer*>(handle)->setLayerShader(static_cast<Beatmup::ImageShader*>(_shader));
}


void BeatmupSceneCreate(handle_t * handle) {
    *handle = static_cast<handle_t>(new Beatmup::Scene());
}


void BeatmupSceneDestroy(handle_t handle) {
    if (handle) delete static_cast<Beatmup::Scene*>(handle);
}


handle_t BeatmupSceneNewBitmapLayer1(handle_t handle, const char* _name) {
    Beatmup::Scene::BitmapLayer& result = static_cast<Beatmup::Scene*>(handle)->newBitmapLayer(_name);
    return static_cast<handle_t>(&result);
}


handle_t BeatmupSceneNewBitmapLayer2(handle_t handle) {
    Beatmup::Scene::BitmapLayer& result = static_cast<Beatmup::Scene*>(handle)->newBitmapLayer();
    return static_cast<handle_t>(&result);
}


handle_t BeatmupSceneNewMaskedBitmapLayer1(handle_t handle, const char* _name) {
    Beatmup::Scene::MaskedBitmapLayer& result = static_cast<Beatmup::Scene*>(handle)->newMaskedBitmapLayer(_name);
    return static_cast<handle_t>(&result);
}


handle_t BeatmupSceneNewMaskedBitmapLayer2(handle_t handle) {
    Beatmup::Scene::MaskedBitmapLayer& result = static_cast<Beatmup::Scene*>(handle)->newMaskedBitmapLayer();
    return static_cast<handle_t>(&result);
}


handle_t BeatmupSceneNewShapedBitmapLayer1(handle_t handle, const char* _name) {
    Beatmup::Scene::ShapedBitmapLayer& result = static_cast<Beatmup::Scene*>(handle)->newShapedBitmapLayer(_name);
    return static_cast<handle_t>(&result);
}


handle_t BeatmupSceneNewShapedBitmapLayer2(handle_t handle) {
    Beatmup::Scene::ShapedBitmapLayer& result = static_cast<Beatmup::Scene*>(handle)->newShapedBitmapLayer();
    return static_cast<handle_t>(&result);
}


handle_t BeatmupSceneNewShadedBitmapLayer1(handle_t handle, const char* _name) {
    Beatmup::Scene::ShadedBitmapLayer& result = static_cast<Beatmup::Scene*>(handle)->newShadedBitmapLayer(_name);
    return static_cast<handle_t>(&result);
}


handle_t BeatmupSceneNewShadedBitmapLayer2(handle_t handle) {
    Beatmup::Scene::ShadedBitmapLayer& result = static_cast<Beatmup::Scene*>(handle)->newShadedBitmapLayer();
    return static_cast<handle_t>(&result);
}


handle_t BeatmupSceneAddScene(handle_t handle, chandle_t _scene) {
    Beatmup::Scene::SceneLayer& result = static_cast<Beatmup::Scene*>(handle)->addScene(*static_cast<const Beatmup::Scene*>(_scene));
    return static_cast<handle_t>(&result);
}


handle_t BeatmupSceneGetLayer1(chandle_t handle, const char* _name) {
    Beatmup::Scene::Layer* result = static_cast<const Beatmup::Scene*>(handle)->getLayer(_name);
    return static_cast<handle_t>(result);
}


handle_t BeatmupSceneGetLayer2(chandle_t handle, int _index) {
    Beatmup::Scene::Layer& result = static_cast<const Beatmup::Scene*>(handle)->getLayer(_index);
    return static_cast<handle_t>(&result);
}


handle_t BeatmupSceneGetLayer3(chandle_t handle, float _x, float _y, unsigned int _recursionDepth) {
    Beatmup::Scene::Layer* result = static_cast<const Beatmup::Scene*>(handle)->getLayer(_x, _y, _recursionDepth);
    return static_cast<handle_t>(result);
}


int BeatmupSceneGetLayerIndex(chandle_t handle, chandle_t _layer) {
    return static_cast<const Beatmup::Scene*>(handle)->getLayerIndex(*static_cast<const Beatmup::Scene::Layer*>(_layer));
}


int BeatmupSceneGetLayerCount(chandle_t handle) {
    return static_cast<const Beatmup::Scene*>(handle)->getLayerCount();
}


bool BeatmupSceneResolveMapping(chandle_t handle, chandle_t _layer, handle_t _mapping) {
    return static_cast<const Beatmup::Scene*>(handle)->resolveMapping(*static_cast<const Beatmup::Scene::Layer*>(_layer), *static_cast<Beatmup::AffineMapping*>(_mapping));
}


void BeatmupSceneAttachLayer(handle_t handle, handle_t _layer) {
    static_cast<Beatmup::Scene*>(handle)->attachLayer(*static_cast<Beatmup::Scene::Layer*>(_layer));
}


handle_t BeatmupSceneDetachLayer(handle_t handle, int _index) {
    Beatmup::Scene::Layer* result = static_cast<Beatmup::Scene*>(handle)->detachLayer(_index);
    return static_cast<handle_t>(result);
}


void BeatmupSceneRendererCreate(handle_t * handle) {
    *handle = static_cast<handle_t>(new Beatmup::SceneRenderer());
}


void BeatmupSceneRendererDestroy(handle_t handle) {
    if (handle) delete static_cast<Beatmup::SceneRenderer*>(handle);
}


void BeatmupSceneRendererSetOutput(handle_t handle, handle_t _bitmap) {
    static_cast<Beatmup::SceneRenderer*>(handle)->setOutput(*static_cast<Beatmup::AbstractBitmap*>(_bitmap));
}


handle_t BeatmupSceneRendererGetOutput(chandle_t handle) {
    Beatmup::AbstractBitmap* result = static_cast<const Beatmup::SceneRenderer*>(handle)->getOutput();
    return static_cast<handle_t>(result);
}


void BeatmupSceneRendererResetOutput(handle_t handle) {
    static_cast<Beatmup::SceneRenderer*>(handle)->resetOutput();
}


chandle_t BeatmupSceneRendererGetScene(chandle_t handle) {
    const Beatmup::Scene* result = static_cast<const Beatmup::SceneRenderer*>(handle)->getScene();
    return static_cast<chandle_t>(result);
}


void BeatmupSceneRendererSetScene(handle_t handle, handle_t _scene) {
    static_cast<Beatmup::SceneRenderer*>(handle)->setScene(*static_cast<Beatmup::Scene*>(_scene));
}


void BeatmupSceneRendererSetOutputMapping(handle_t handle, const beatmup_scene_renderer_output_mapping_t _mapping) {
    static_cast<Beatmup::SceneRenderer*>(handle)->setOutputMapping(static_cast<Beatmup::SceneRenderer::OutputMapping>(_mapping));
}


beatmup_scene_renderer_output_mapping_t BeatmupSceneRendererGetOutputMapping(chandle_t handle) {
    Beatmup::SceneRenderer::OutputMapping result = static_cast<const Beatmup::SceneRenderer*>(handle)->getOutputMapping();
    return static_cast<beatmup_scene_renderer_output_mapping_t>(result);
}


void BeatmupSceneRendererSetOutputReferenceWidth(handle_t handle, int _newWidth) {
    static_cast<Beatmup::SceneRenderer*>(handle)->setOutputReferenceWidth(_newWidth);
}


int BeatmupSceneRendererGetOutputReferenceWidth(chandle_t handle) {
    return static_cast<const Beatmup::SceneRenderer*>(handle)->getOutputReferenceWidth();
}


void BeatmupSceneRendererSetBackgroundImage(handle_t handle, handle_t _bitmap) {
    static_cast<Beatmup::SceneRenderer*>(handle)->setBackgroundImage(static_cast<Beatmup::AbstractBitmap*>(_bitmap));
}


void BeatmupSceneRendererSetOutputPixelsFetching(handle_t handle, bool _fetch) {
    static_cast<Beatmup::SceneRenderer*>(handle)->setOutputPixelsFetching(_fetch);
}


bool BeatmupSceneRendererGetOutputPixelsFetching(chandle_t handle) {
    return static_cast<const Beatmup::SceneRenderer*>(handle)->getOutputPixelsFetching();
}


handle_t BeatmupSceneRendererPickLayer(chandle_t handle, float _x, float _y, bool _normalized) {
    Beatmup::Scene::Layer* result = static_cast<const Beatmup::SceneRenderer*>(handle)->pickLayer(_x, _y, _normalized);
    return static_cast<handle_t>(result);
}


void BeatmupSceneRendererSetRenderingEventListener(handle_t handle, handle_t _eventListener) {
    static_cast<Beatmup::SceneRenderer*>(handle)->setRenderingEventListener(static_cast<Beatmup::RenderingContext::EventListener*>(_eventListener));
}


void BeatmupImageShaderCreate(handle_t * handle, handle_t _env) {
    *handle = static_cast<handle_t>(new Beatmup::ImageShader(*static_cast<Beatmup::Environment*>(_env)));
}


void BeatmupImageShaderDestroy(handle_t handle) {
    if (handle) delete static_cast<Beatmup::ImageShader*>(handle);
}


void BeatmupImageShaderSetSourceCode1(handle_t handle, const char* _sourceCode) {
    static_cast<Beatmup::ImageShader*>(handle)->setSourceCode(_sourceCode);
}


void BeatmupImageShaderPrepare(handle_t handle, handle_t _gpu, handle_t _input, handle_t _output, chandle_t _mapping) {
    static_cast<Beatmup::ImageShader*>(handle)->prepare(*static_cast<Beatmup::GraphicPipeline*>(_gpu), static_cast<Beatmup::GL::TextureHandler*>(_input), static_cast<Beatmup::AbstractBitmap*>(_output), *static_cast<const Beatmup::AffineMapping*>(_mapping));
}


void BeatmupImageShaderProcess(handle_t handle, handle_t _gpu) {
    static_cast<Beatmup::ImageShader*>(handle)->process(*static_cast<Beatmup::GraphicPipeline*>(_gpu));
}


void BeatmupImageShaderNoSourceCreate(handle_t * handle) {
    *handle = static_cast<handle_t>(new Beatmup::ImageShader::NoSource());
}


void BeatmupImageShaderNoSourceDestroy(handle_t handle) {
    if (handle) delete static_cast<Beatmup::ImageShader::NoSource*>(handle);
}


void BeatmupImageShaderUnsupportedInputTextureFormatCreate(handle_t * handle, const beatmup_g_l_texture_handler_texture_format_t* _format) {
    *handle = static_cast<handle_t>(new Beatmup::ImageShader::UnsupportedInputTextureFormat(static_cast<Beatmup::GL::TextureHandler::TextureFormat>(*_format)));
}


void BeatmupImageShaderUnsupportedInputTextureFormatDestroy(handle_t handle) {
    if (handle) delete static_cast<Beatmup::ImageShader::UnsupportedInputTextureFormat*>(handle);
}


void BeatmupShaderApplicatorCreate(handle_t * handle) {
    *handle = static_cast<handle_t>(new Beatmup::ShaderApplicator());
}


void BeatmupShaderApplicatorSetInputBitmap(handle_t handle, handle_t _bitmap) {
    static_cast<Beatmup::ShaderApplicator*>(handle)->setInputBitmap(static_cast<Beatmup::AbstractBitmap*>(_bitmap));
}


void BeatmupShaderApplicatorSetOutputBitmap(handle_t handle, handle_t _bitmap) {
    static_cast<Beatmup::ShaderApplicator*>(handle)->setOutputBitmap(static_cast<Beatmup::AbstractBitmap*>(_bitmap));
}


void BeatmupShaderApplicatorSetShader(handle_t handle, handle_t _shader) {
    static_cast<Beatmup::ShaderApplicator*>(handle)->setShader(static_cast<Beatmup::ImageShader*>(_shader));
}


handle_t BeatmupShaderApplicatorGetInputBitmap(chandle_t handle) {
    Beatmup::AbstractBitmap* result = static_cast<const Beatmup::ShaderApplicator*>(handle)->getInputBitmap();
    return static_cast<handle_t>(result);
}


handle_t BeatmupShaderApplicatorGetOutputBitmap(chandle_t handle) {
    Beatmup::AbstractBitmap* result = static_cast<const Beatmup::ShaderApplicator*>(handle)->getOutputBitmap();
    return static_cast<handle_t>(result);
}


handle_t BeatmupShaderApplicatorGetShader(chandle_t handle) {
    Beatmup::ImageShader* result = static_cast<const Beatmup::ShaderApplicator*>(handle)->getShader();
    return static_cast<handle_t>(result);
}


void BeatmupShaderApplicatorDestroy(handle_t handle) {
    if (handle) delete static_cast<Beatmup::ShaderApplicator*>(handle);
}

