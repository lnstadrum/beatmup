#ifndef __LIBBEATMUP_INCLUDE_H__
#define __LIBBEATMUP_INCLUDE_H__
#include <stdint.h>

typedef void* handle_t;
typedef const void* chandle_t;

typedef enum {
    BEATMUP_SCENE_LAYER_TYPE_SCENELAYER = 0,
    BEATMUP_SCENE_LAYER_TYPE_BITMAPLAYER = 1,
    BEATMUP_SCENE_LAYER_TYPE_MASKEDBITMAPLAYER = 2,
    BEATMUP_SCENE_LAYER_TYPE_SHAPEDBITMAPLAYER = 3,
    BEATMUP_SCENE_LAYER_TYPE_SHADEDBITMAPLAYER = 4
} beatmup_scene_layer_type_t;
typedef enum {
    BEATMUP_PIXEL_FORMAT_SINGLEBYTE = 0,
    BEATMUP_PIXEL_FORMAT_TRIPLEBYTE = 1,
    BEATMUP_PIXEL_FORMAT_QUADBYTE = 2,
    BEATMUP_PIXEL_FORMAT_SINGLEFLOAT = 3,
    BEATMUP_PIXEL_FORMAT_TRIPLEFLOAT = 4,
    BEATMUP_PIXEL_FORMAT_QUADFLOAT = 5,
    BEATMUP_PIXEL_FORMAT_BINARYMASK = 6,
    BEATMUP_PIXEL_FORMAT_QUATERNARYMASK = 7,
    BEATMUP_PIXEL_FORMAT_HEXMASK = 8
} beatmup_pixel_format_t;
typedef unsigned int beatmup_job_t;
typedef unsigned int beatmup_memchunk_t;
typedef enum {
    BEATMUP_ABSTRACT_TASK_EXECUTION_TARGET_DONOTUSEGPU = 0,
    BEATMUP_ABSTRACT_TASK_EXECUTION_TARGET_USEGPUIFAVAILABLE = 1,
    BEATMUP_ABSTRACT_TASK_EXECUTION_TARGET_USEGPU = 2
} beatmup_abstract_task_execution_target_t;
typedef struct {
    unsigned int _1;
    unsigned int _2;
} beatmup_image_resolution_t;
typedef unsigned char beatmup_pool_index_t;
typedef struct {
    float r;
    float g;
    float b;
} beatmup_color3f_t;
typedef enum {
    BEATMUP_SCENE_RENDERER_OUTPUT_MAPPING_STRETCH = 0,
    BEATMUP_SCENE_RENDERER_OUTPUT_MAPPING_FIT_WIDTH_TO_TOP = 1,
    BEATMUP_SCENE_RENDERER_OUTPUT_MAPPING_FIT_WIDTH = 2,
    BEATMUP_SCENE_RENDERER_OUTPUT_MAPPING_FIT_HEIGHT = 3
} beatmup_scene_renderer_output_mapping_t;
typedef enum {
    BEATMUP_FLOOD_FILL_BORDER_MORPHOLOGY_NONE = 0,
    BEATMUP_FLOOD_FILL_BORDER_MORPHOLOGY_DILATE = 1,
    BEATMUP_FLOOD_FILL_BORDER_MORPHOLOGY_ERODE = 2
} beatmup_flood_fill_border_morphology_t;
typedef enum {
    BEATMUP_G_L_TEXTURE_HANDLER_TEXTURE_FORMAT_RX8 = 0,
    BEATMUP_G_L_TEXTURE_HANDLER_TEXTURE_FORMAT_RGBX8 = 1,
    BEATMUP_G_L_TEXTURE_HANDLER_TEXTURE_FORMAT_RGBAX8 = 2,
    BEATMUP_G_L_TEXTURE_HANDLER_TEXTURE_FORMAT_RX32F = 3,
    BEATMUP_G_L_TEXTURE_HANDLER_TEXTURE_FORMAT_RGBX32F = 4,
    BEATMUP_G_L_TEXTURE_HANDLER_TEXTURE_FORMAT_RGBAX32F = 5,
    BEATMUP_G_L_TEXTURE_HANDLER_TEXTURE_FORMAT_OES_EXT = 6
} beatmup_g_l_texture_handler_texture_format_t;
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} beatmup_color4i_t;
typedef enum {
    BEATMUP_SCENE_BITMAP_LAYER_IMAGE_SOURCE_BITMAP = 0
} beatmup_scene_bitmap_layer_image_source_t;
typedef struct {
    int _1;
    int _2;
    int _3;
    int _4;
} beatmup_int_rectangle_t;
typedef unsigned char beatmup_thread_index_t;
typedef enum {
    BEATMUP_PROCESSING_TARGET_CPU = 0,
    BEATMUP_PROCESSING_TARGET_GPU = 1
} beatmup_processing_target_t;
typedef enum {
    BEATMUP_BITMAP_BINARY_OPERATION_OPERATION_NONE = 0,
    BEATMUP_BITMAP_BINARY_OPERATION_OPERATION_ADD = 1,
    BEATMUP_BITMAP_BINARY_OPERATION_OPERATION_MULTIPLY = 2
} beatmup_bitmap_binary_operation_operation_t;
typedef enum {
    BEATMUP_MULTITASK_REPETITION_POLICY_REPEAT_ALWAYS = 0,
    BEATMUP_MULTITASK_REPETITION_POLICY_REPEAT_UPDATE = 1,
    BEATMUP_MULTITASK_REPETITION_POLICY_IGNORE_IF_UPTODATE = 2,
    BEATMUP_MULTITASK_REPETITION_POLICY_IGNORE_ALWAYS = 3
} beatmup_multitask_repetition_policy_t;
typedef struct {
    int _1;
    int _2;
} beatmup_int_point_t;
typedef handle_t beatmup_bitmap_ptr_t;
typedef uint32_t beatmup_msize_t;
typedef uint8_t beatmup_pixbyte_t;

#ifdef __cplusplus
extern "C" {
#endif


/* Beatmup::Environment */
void BeatmupEnvironmentCreate1(handle_t *);
void BeatmupEnvironmentCreate2(handle_t *, const beatmup_pool_index_t numThreadPools, const char* swapFilePrefix, const char* swapFileSuffix);
void BeatmupEnvironmentDestroy(handle_t);
float BeatmupEnvironmentPerformTask(handle_t, handle_t task, const beatmup_pool_index_t pool);
void BeatmupEnvironmentRepeatTask(handle_t, handle_t task, bool abortCurrent, const beatmup_pool_index_t pool);
beatmup_job_t BeatmupEnvironmentSubmitTask(handle_t, handle_t task, const beatmup_pool_index_t pool);
beatmup_job_t BeatmupEnvironmentSubmitPersistentTask(handle_t, handle_t task, const beatmup_pool_index_t pool);
void BeatmupEnvironmentWaitForJob(handle_t, beatmup_job_t job, const beatmup_pool_index_t pool);
bool BeatmupEnvironmentAbortJob(handle_t, beatmup_job_t job, const beatmup_pool_index_t pool);
void BeatmupEnvironmentWait(handle_t, const beatmup_pool_index_t pool);
bool BeatmupEnvironmentBusy(handle_t, const beatmup_pool_index_t pool);
const beatmup_thread_index_t BeatmupEnvironmentMaxAllowedWorkerCount(chandle_t, const beatmup_pool_index_t pool);
void BeatmupEnvironmentLimitWorkerCount(handle_t, beatmup_thread_index_t maxValue, const beatmup_pool_index_t pool);
const beatmup_memchunk_t BeatmupEnvironmentAllocateMemory(handle_t, beatmup_msize_t size);
void* BeatmupEnvironmentAcquireMemory(handle_t, beatmup_memchunk_t chunk);
void BeatmupEnvironmentReleaseMemory(handle_t, beatmup_memchunk_t chunk, bool unusedAnymore);
void BeatmupEnvironmentFreeMemory(handle_t, beatmup_memchunk_t chunk);
beatmup_msize_t BeatmupEnvironmentSwapOnDisk(handle_t, beatmup_msize_t howMuch);
bool BeatmupEnvironmentIsGpuQueried(chandle_t);
bool BeatmupEnvironmentIsGpuReady(chandle_t);
bool BeatmupEnvironmentIsManagingThread(chandle_t);
handle_t BeatmupEnvironmentGetGpuRecycleBin(chandle_t);
beatmup_msize_t BeatmupEnvironmentGetTotalRam();
void BeatmupEnvironmentWarmUpGpu(handle_t);

/* Beatmup::IntPoint */
int BeatmupCustomPointGetX2(const beatmup_int_point_t*);
int BeatmupCustomPointGetY2(const beatmup_int_point_t*);
bool BeatmupCustomPointIsEqual(const beatmup_int_point_t*, const beatmup_int_point_t* _);
beatmup_int_point_t BeatmupCustomPointAdd1(const beatmup_int_point_t*, const beatmup_int_point_t* _);
beatmup_int_point_t BeatmupCustomPointSubtract1(const beatmup_int_point_t*, const beatmup_int_point_t* _);
beatmup_int_point_t BeatmupCustomPointAdd2(const beatmup_int_point_t*, int _);
beatmup_int_point_t BeatmupCustomPointSubtract2(const beatmup_int_point_t*, int _);
beatmup_int_point_t BeatmupCustomPointMultiply(const beatmup_int_point_t*, int _);
beatmup_int_point_t BeatmupCustomPointDivide(const beatmup_int_point_t*, int _);
void BeatmupCustomPointTranslate2(beatmup_int_point_t*, int x, int y);
int BeatmupCustomPointHypot22(const beatmup_int_point_t*);
bool BeatmupCustomPointIsInsideAxesSpan2(const beatmup_int_point_t*, int scaleX, int scaleY);

/* Beatmup::IntRectangle */
beatmup_int_rectangle_t BeatmupCustomRectangleMultiply(const beatmup_int_rectangle_t*, int _);
beatmup_int_rectangle_t BeatmupCustomRectangleDivide(const beatmup_int_rectangle_t*, int _);
int BeatmupCustomRectangleGetX12(const beatmup_int_rectangle_t*);
int BeatmupCustomRectangleGetY12(const beatmup_int_rectangle_t*);
int BeatmupCustomRectangleGetX22(const beatmup_int_rectangle_t*);
int BeatmupCustomRectangleGetY22(const beatmup_int_rectangle_t*);
int BeatmupCustomRectangleWidth2(const beatmup_int_rectangle_t*);
int BeatmupCustomRectangleHeight2(const beatmup_int_rectangle_t*);
int BeatmupCustomRectangleGetArea2(const beatmup_int_rectangle_t*);
void BeatmupCustomRectangleNormalize2(beatmup_int_rectangle_t*);
void BeatmupCustomRectangleTranslate2(beatmup_int_rectangle_t*, int x, int y);
void BeatmupCustomRectangleScale2(beatmup_int_rectangle_t*, int x, int y);
void BeatmupCustomRectangleLimit(beatmup_int_rectangle_t*, const beatmup_int_rectangle_t* frame);
beatmup_int_rectangle_t BeatmupCustomRectangleTranslated(beatmup_int_rectangle_t*, int x, int y);
short BeatmupCustomRectangleHorizontalPositioningTest2(const beatmup_int_rectangle_t*, int x);
short BeatmupCustomRectangleVerticalPositioningTest2(const beatmup_int_rectangle_t*, int y);
void BeatmupCustomRectangleGrow2(beatmup_int_rectangle_t*, int r);

/* Beatmup::ImageResolution */
bool BeatmupImageResolutionIsEqual(const beatmup_image_resolution_t*, const beatmup_image_resolution_t*);
bool BeatmupImageResolutionIsNotEqual(const beatmup_image_resolution_t*, const beatmup_image_resolution_t*);
beatmup_msize_t BeatmupImageResolutionNumPixels(const beatmup_image_resolution_t*);
float BeatmupImageResolutionMegaPixels(const beatmup_image_resolution_t*);
float BeatmupImageResolutionGetAspectRatio(const beatmup_image_resolution_t*);
float BeatmupImageResolutionGetInvAspectRatio(const beatmup_image_resolution_t*);
bool BeatmupImageResolutionFat(const beatmup_image_resolution_t*);
beatmup_int_rectangle_t BeatmupImageResolutionClientRect(const beatmup_image_resolution_t*);
unsigned int BeatmupImageResolutionGetWidth(const beatmup_image_resolution_t*);
unsigned int BeatmupImageResolutionGetHeight(const beatmup_image_resolution_t*);
void BeatmupImageResolutionSet(beatmup_image_resolution_t*, unsigned int width, unsigned int height);

/* Beatmup::AbstractBitmap */
const int BeatmupAbstractBitmapGetDepth(chandle_t);
void BeatmupAbstractBitmapUnlockPixels(handle_t);
void BeatmupAbstractBitmapLockPixels(handle_t, beatmup_processing_target_t);
void BeatmupAbstractBitmapInvalidate(handle_t, beatmup_processing_target_t);
bool BeatmupAbstractBitmapIsUpToDate(chandle_t, beatmup_processing_target_t);
bool BeatmupAbstractBitmapIsDirty(chandle_t);
int BeatmupAbstractBitmapGetPixelInt(chandle_t, int x, int y, int cha);
const unsigned char BeatmupAbstractBitmapGetBitsPerPixel(chandle_t);
const unsigned char BeatmupAbstractBitmapGetNumberOfChannels(chandle_t);
const beatmup_image_resolution_t BeatmupAbstractBitmapGetSize(chandle_t);
handle_t BeatmupAbstractBitmapGetEnvironment(chandle_t);
void BeatmupAbstractBitmapZero(handle_t);
bool BeatmupAbstractBitmapIsInteger1(chandle_t);
bool BeatmupAbstractBitmapIsFloat1(chandle_t);
bool BeatmupAbstractBitmapIsMask1(chandle_t);
bool BeatmupAbstractBitmapIsInteger2(beatmup_pixel_format_t pixelFormat);
bool BeatmupAbstractBitmapIsFloat2(beatmup_pixel_format_t pixelFormat);
bool BeatmupAbstractBitmapIsMask2(beatmup_pixel_format_t pixelFormat);
void BeatmupAbstractBitmapDestroy(handle_t);

/* Beatmup::InternalBitmap */
void BeatmupInternalBitmapCreate1(handle_t *, handle_t env, beatmup_pixel_format_t pixelFormat, int width, int height, bool allocate);
void BeatmupInternalBitmapCreate2(handle_t *, handle_t env, const char* filename);
void BeatmupInternalBitmapDestroy(handle_t);
const beatmup_pixel_format_t BeatmupInternalBitmapGetPixelFormat(chandle_t);
const int BeatmupInternalBitmapGetWidth(chandle_t);
const int BeatmupInternalBitmapGetHeight(chandle_t);
const beatmup_msize_t BeatmupInternalBitmapGetMemorySize(chandle_t);
beatmup_pixbyte_t* BeatmupInternalBitmapGetData(chandle_t, int x, int y);
void BeatmupInternalBitmapUnlockPixels(handle_t);
void BeatmupInternalBitmapSaveBmp(handle_t, const char* filename);

/* Beatmup::BitmapConverter */
void BeatmupBitmapConverterCreate(handle_t *);
void BeatmupBitmapConverterSetBitmaps(handle_t, handle_t input, handle_t output);
beatmup_thread_index_t BeatmupBitmapConverterMaxAllowedThreads(chandle_t);
beatmup_abstract_task_execution_target_t BeatmupBitmapConverterGetExecutionTarget(chandle_t);
void BeatmupBitmapConverterConvert(handle_t input, handle_t output);
void BeatmupBitmapConverterDestroy(handle_t);

/* Beatmup::Crop */
void BeatmupCropCreate(handle_t *);
beatmup_thread_index_t BeatmupCropMaxAllowedThreads(chandle_t);
void BeatmupCropSetInput(handle_t, handle_t input);
void BeatmupCropSetOutput(handle_t, handle_t output);
void BeatmupCropSetCropRect(handle_t, beatmup_int_rectangle_t);
void BeatmupCropSetOutputOrigin(handle_t, beatmup_int_point_t);
bool BeatmupCropIsFit(chandle_t);
handle_t BeatmupCropRun(handle_t bitmap, beatmup_int_rectangle_t clipRect);
void BeatmupCropDestroy(handle_t);

/* Beatmup::BitmapBinaryOperation */
void BeatmupBitmapBinaryOperationCreate(handle_t *);
void BeatmupBitmapBinaryOperationSetOperand1(handle_t, handle_t op1);
void BeatmupBitmapBinaryOperationSetOperand2(handle_t, handle_t op2);
void BeatmupBitmapBinaryOperationSetOutput(handle_t, handle_t output);
void BeatmupBitmapBinaryOperationSetOperation(handle_t, const beatmup_bitmap_binary_operation_operation_t operation);
void BeatmupBitmapBinaryOperationSetCropSize(handle_t, int width, int height);
void BeatmupBitmapBinaryOperationSetOp1Origin(handle_t, const beatmup_int_point_t origin);
void BeatmupBitmapBinaryOperationSetOp2Origin(handle_t, const beatmup_int_point_t origin);
void BeatmupBitmapBinaryOperationSetOutputOrigin(handle_t, const beatmup_int_point_t origin);
void BeatmupBitmapBinaryOperationResetCrop(handle_t);
int BeatmupBitmapBinaryOperationGetCropWidth(chandle_t);
int BeatmupBitmapBinaryOperationGetCropHeight(chandle_t);
const beatmup_int_point_t BeatmupBitmapBinaryOperationGetOp1Origin(chandle_t);
const beatmup_int_point_t BeatmupBitmapBinaryOperationGetOp2Origin(chandle_t);
const beatmup_int_point_t BeatmupBitmapBinaryOperationGetOutputOrigin(chandle_t);
beatmup_thread_index_t BeatmupBitmapBinaryOperationMaxAllowedThreads(chandle_t);
void BeatmupBitmapBinaryOperationDestroy(handle_t);

/* Beatmup::BitmapResampler */
void BeatmupBitmapResamplerCreate(handle_t *);
void BeatmupBitmapResamplerSetBitmaps(handle_t, handle_t input, handle_t output);
void BeatmupBitmapResamplerSetInputRect(handle_t, const beatmup_int_rectangle_t* rect);
void BeatmupBitmapResamplerSetOutputRect(handle_t, const beatmup_int_rectangle_t* rect);
beatmup_int_rectangle_t BeatmupBitmapResamplerGetInputRect(chandle_t);
beatmup_int_rectangle_t BeatmupBitmapResamplerGetOutputRect(chandle_t);
beatmup_thread_index_t BeatmupBitmapResamplerMaxAllowedThreads(chandle_t);
void BeatmupBitmapResamplerDestroy(handle_t);

/* Beatmup::BitmapTools */
handle_t BeatmupBitmapToolsMakeCopy1(handle_t source, beatmup_pixel_format_t newPixelFormat);
handle_t BeatmupBitmapToolsMakeCopy2(handle_t source);
handle_t BeatmupBitmapToolsChessboard(handle_t env, int width, int height, int cellSize, beatmup_pixel_format_t pixelFormat);
void BeatmupBitmapToolsMakeOpaque(handle_t, beatmup_int_rectangle_t);
void BeatmupBitmapToolsInvert(handle_t input, handle_t output);

/* Beatmup::Filters::ImageTuning */
void BeatmupFiltersImageTuningCreate(handle_t *);
void BeatmupFiltersImageTuningSetBitmaps(handle_t, handle_t input, handle_t output);
beatmup_thread_index_t BeatmupFiltersImageTuningMaxAllowedThreads(chandle_t);
void BeatmupFiltersImageTuningSetHueOffset(handle_t, float val);
void BeatmupFiltersImageTuningSetSaturationFactor(handle_t, float val);
void BeatmupFiltersImageTuningSetValueFactor(handle_t, float val);
void BeatmupFiltersImageTuningSetBrightness(handle_t, float val);
void BeatmupFiltersImageTuningSetContrast(handle_t, float val);
float BeatmupFiltersImageTuningGetHueOffset(chandle_t);
float BeatmupFiltersImageTuningGetSaturationFactor(chandle_t);
float BeatmupFiltersImageTuningGetValueFactor(chandle_t);
float BeatmupFiltersImageTuningGetBrightness(chandle_t);
float BeatmupFiltersImageTuningGetContrast(chandle_t);
void BeatmupFiltersImageTuningDestroy(handle_t);

/* Beatmup::Filters::ColorMatrix */
void BeatmupFiltersColorMatrixCreate(handle_t *);
void BeatmupFiltersColorMatrixApply(handle_t, int startx, int starty, beatmup_msize_t nPix, handle_t thread);
bool BeatmupFiltersColorMatrixIsIntegerApproximationsAllowed(chandle_t);
void BeatmupFiltersColorMatrixAllowIntegerApproximations(handle_t, bool allow);
handle_t BeatmupFiltersColorMatrixGetMatrix(handle_t);
void BeatmupFiltersColorMatrixSetCoefficients(handle_t, int outChannel, float add, float inR, float inG, float inB, float inA);
void BeatmupFiltersColorMatrixSetHSVCorrection(handle_t, float addHueDegrees, float scaleSat, float scaleVal);
void BeatmupFiltersColorMatrixSetColorInversion(handle_t, beatmup_color3f_t preservedHue, float scaleSat, float scaleVal);
void BeatmupFiltersColorMatrixDestroy(handle_t);

/* Beatmup::IntegerContour2D */
void BeatmupIntegerContour2DCreate(handle_t *);
void BeatmupIntegerContour2DAddPoint(handle_t, int x, int y);
void BeatmupIntegerContour2DClear(handle_t);
int BeatmupIntegerContour2DGetPointCount(chandle_t);
float BeatmupIntegerContour2DGetLenght(chandle_t);
beatmup_int_point_t BeatmupIntegerContour2DGetPoint(chandle_t, int index);

/* Beatmup::IntegerContour2D::BadSeedPoint */
void BeatmupIntegerContour2DBadSeedPointCreate1(handle_t *);
void BeatmupIntegerContour2DBadSeedPointCreate2(handle_t *, int x, int y, bool lefttop, bool righttop, bool leftbottom, bool rightbottom);
void BeatmupIntegerContour2DBadSeedPointDestroy(handle_t);
void BeatmupIntegerContour2DDestroy(handle_t);

/* Beatmup::FloodFill */
void BeatmupFloodFillCreate(handle_t *);
void BeatmupFloodFillDestroy(handle_t);
handle_t BeatmupFloodFillGetInput(chandle_t);
handle_t BeatmupFloodFillGetOutput(chandle_t);
beatmup_int_rectangle_t BeatmupFloodFillGetBounds(chandle_t);
int BeatmupFloodFillGetContourCount(chandle_t);
chandle_t BeatmupFloodFillGetContour(chandle_t, int contourIndex);
void BeatmupFloodFillSetInput(handle_t, handle_t);
void BeatmupFloodFillSetOutput(handle_t, handle_t);
void BeatmupFloodFillSetMaskPos(handle_t, beatmup_int_point_t*);
void BeatmupFloodFillSetSeeds2(handle_t, const int* seedsXY, int seedCount);
void BeatmupFloodFillSetTolerance(handle_t, float tolerance);
void BeatmupFloodFillSetBorderPostprocessing(handle_t, beatmup_flood_fill_border_morphology_t operation, float holdRadius, float releaseRadius);
void BeatmupFloodFillSetComputeContours(handle_t, bool);
beatmup_thread_index_t BeatmupFloodFillMaxAllowedThreads(chandle_t);
bool BeatmupFloodFillProcess(handle_t, handle_t thread);
void BeatmupFloodFillBeforeProcessing(handle_t, beatmup_thread_index_t threadCount, handle_t gpu);
void BeatmupFloodFillAfterProcessing(handle_t, beatmup_thread_index_t threadCount, bool aborted);

/* Beatmup::Multitask */
void BeatmupMultitaskCreate(handle_t *);
beatmup_multitask_repetition_policy_t BeatmupMultitaskGetRepetitionPolicy(handle_t, chandle_t task);
void BeatmupMultitaskSetRepetitionPolicy(handle_t, handle_t task, beatmup_multitask_repetition_policy_t policy);
void BeatmupMultitaskDestroy(handle_t);

/* Beatmup::CustomPipeline */
void BeatmupCustomPipelineDestroy(handle_t);
int BeatmupCustomPipelineGetTaskCount(chandle_t);
handle_t BeatmupCustomPipelineGetTask(chandle_t, int index);
int BeatmupCustomPipelineGetTaskIndex(handle_t, chandle_t);
handle_t BeatmupCustomPipelineAddTask(handle_t, handle_t task);
handle_t BeatmupCustomPipelineInsertTask(handle_t, handle_t task, chandle_t succeedingHoder);
bool BeatmupCustomPipelineRemoveTask(handle_t, chandle_t task);
void BeatmupCustomPipelineMeasure(handle_t);

/* Beatmup::CustomPipeline::TaskHolder */
handle_t BeatmupCustomPipelineTaskHolderGetTask(chandle_t);

/* Beatmup::CustomPipeline::PipelineNotReady */
void BeatmupCustomPipelinePipelineNotReadyCreate(handle_t *, const char* message);
void BeatmupCustomPipelinePipelineNotReadyDestroy(handle_t);
bool BeatmupIsEqual(chandle_t, chandle_t);

/* Beatmup::Scene::Layer */
beatmup_scene_layer_type_t BeatmupSceneLayerGetType(chandle_t);
const char* BeatmupSceneLayerGetName(chandle_t);
void BeatmupSceneLayerSetName(handle_t, const char* name);
handle_t BeatmupSceneLayerGetMapping1(handle_t);
chandle_t BeatmupSceneLayerGetMapping2(chandle_t);
void BeatmupSceneLayerSetMapping(handle_t, chandle_t mapping);
bool BeatmupSceneLayerTestPoint(chandle_t, float x, float y);
handle_t BeatmupSceneLayerGetChild(chandle_t, float x, float y, unsigned int recursionDepth);
bool BeatmupSceneLayerIsVisible(chandle_t);
bool BeatmupSceneLayerIsPhantom(chandle_t);
void BeatmupSceneLayerSetVisible(handle_t, bool visible);
void BeatmupSceneLayerSetPhantom(handle_t, bool phantom);

/* Beatmup::Scene::SceneLayer */
chandle_t BeatmupSceneSceneLayerGetScene(chandle_t);
bool BeatmupSceneSceneLayerTestPoint(chandle_t, float x, float y);
handle_t BeatmupSceneSceneLayerGetChild(chandle_t, float x, float y, unsigned int recursionDepth);

/* Beatmup::Scene::BitmapLayer */
bool BeatmupSceneBitmapLayerTestPoint(chandle_t, float x, float y);
beatmup_scene_bitmap_layer_image_source_t BeatmupSceneBitmapLayerGetImageSource(chandle_t);
void BeatmupSceneBitmapLayerSetImageSource(handle_t, beatmup_scene_bitmap_layer_image_source_t imageSource);
const beatmup_bitmap_ptr_t BeatmupSceneBitmapLayerGetBitmap(chandle_t);
void BeatmupSceneBitmapLayerSetBitmap(handle_t, beatmup_bitmap_ptr_t bitmap);
handle_t BeatmupSceneBitmapLayerGetBitmapMapping1(handle_t);
chandle_t BeatmupSceneBitmapLayerGetBitmapMapping2(chandle_t);
void BeatmupSceneBitmapLayerSetBitmapMapping(handle_t, chandle_t mapping);
beatmup_color4i_t BeatmupSceneBitmapLayerGetModulationColor(chandle_t);
void BeatmupSceneBitmapLayerSetModulationColor(handle_t, beatmup_color4i_t color);

/* Beatmup::Scene::CustomMaskedBitmapLayer */
handle_t BeatmupSceneCustomMaskedBitmapLayerGetMaskMapping1(handle_t);
chandle_t BeatmupSceneCustomMaskedBitmapLayerGetMaskMapping2(chandle_t);
void BeatmupSceneCustomMaskedBitmapLayerSetMaskMapping(handle_t, chandle_t mapping);
beatmup_color4i_t BeatmupSceneCustomMaskedBitmapLayerGetBackgroundColor(chandle_t);
void BeatmupSceneCustomMaskedBitmapLayerSetBackgroundColor(handle_t, beatmup_color4i_t color);

/* Beatmup::Scene::MaskedBitmapLayer */
const beatmup_bitmap_ptr_t BeatmupSceneMaskedBitmapLayerGetMask(chandle_t);
void BeatmupSceneMaskedBitmapLayerSetMask(handle_t, beatmup_bitmap_ptr_t mask);
bool BeatmupSceneMaskedBitmapLayerTestPoint(chandle_t, float x, float y);

/* Beatmup::Scene::ShapedBitmapLayer */
float BeatmupSceneShapedBitmapLayerGetBorderWidth(chandle_t);
void BeatmupSceneShapedBitmapLayerSetBorderWidth(handle_t, float borderWidth);
float BeatmupSceneShapedBitmapLayerGetSlopeWidth(chandle_t);
void BeatmupSceneShapedBitmapLayerSetSlopeWidth(handle_t, float slopeWidth);
float BeatmupSceneShapedBitmapLayerGetCornerRadius(chandle_t);
void BeatmupSceneShapedBitmapLayerSetCornerRadius(handle_t, float cornerRadius);
bool BeatmupSceneShapedBitmapLayerGetInPixels(chandle_t);
void BeatmupSceneShapedBitmapLayerSetInPixels(handle_t, bool inPixels);
bool BeatmupSceneShapedBitmapLayerTestPoint(chandle_t, float x, float y);

/* Beatmup::Scene::ShadedBitmapLayer */
handle_t BeatmupSceneShadedBitmapLayerGetLayerShader(chandle_t);
void BeatmupSceneShadedBitmapLayerSetLayerShader(handle_t, handle_t shader);
void BeatmupSceneCreate(handle_t *);
void BeatmupSceneDestroy(handle_t);
handle_t BeatmupSceneNewBitmapLayer1(handle_t, const char* name);
handle_t BeatmupSceneNewBitmapLayer2(handle_t);
handle_t BeatmupSceneNewMaskedBitmapLayer1(handle_t, const char* name);
handle_t BeatmupSceneNewMaskedBitmapLayer2(handle_t);
handle_t BeatmupSceneNewShapedBitmapLayer1(handle_t, const char* name);
handle_t BeatmupSceneNewShapedBitmapLayer2(handle_t);
handle_t BeatmupSceneNewShadedBitmapLayer1(handle_t, const char* name);
handle_t BeatmupSceneNewShadedBitmapLayer2(handle_t);
handle_t BeatmupSceneAddScene(handle_t, chandle_t scene);
handle_t BeatmupSceneGetLayer1(chandle_t, const char* name);
handle_t BeatmupSceneGetLayer2(chandle_t, int index);
handle_t BeatmupSceneGetLayer3(chandle_t, float x, float y, unsigned int recursionDepth);
int BeatmupSceneGetLayerIndex(chandle_t, chandle_t layer);
int BeatmupSceneGetLayerCount(chandle_t);
bool BeatmupSceneResolveMapping(chandle_t, chandle_t layer, handle_t mapping);
void BeatmupSceneAttachLayer(handle_t, handle_t layer);
handle_t BeatmupSceneDetachLayer(handle_t, int index);

/* Beatmup::SceneRenderer */
void BeatmupSceneRendererCreate(handle_t *);
void BeatmupSceneRendererDestroy(handle_t);
void BeatmupSceneRendererSetOutput(handle_t, handle_t bitmap);
handle_t BeatmupSceneRendererGetOutput(chandle_t);
void BeatmupSceneRendererResetOutput(handle_t);
chandle_t BeatmupSceneRendererGetScene(chandle_t);
void BeatmupSceneRendererSetScene(handle_t, handle_t scene);
void BeatmupSceneRendererSetOutputMapping(handle_t, const beatmup_scene_renderer_output_mapping_t mapping);
beatmup_scene_renderer_output_mapping_t BeatmupSceneRendererGetOutputMapping(chandle_t);
void BeatmupSceneRendererSetOutputReferenceWidth(handle_t, int newWidth);
int BeatmupSceneRendererGetOutputReferenceWidth(chandle_t);
void BeatmupSceneRendererSetBackgroundImage(handle_t, handle_t bitmap);
void BeatmupSceneRendererSetOutputPixelsFetching(handle_t, bool fetch);
bool BeatmupSceneRendererGetOutputPixelsFetching(chandle_t);
handle_t BeatmupSceneRendererPickLayer(chandle_t, float x, float y, bool normalized);
void BeatmupSceneRendererSetRenderingEventListener(handle_t, handle_t eventListener);

/* Beatmup::ImageShader */
void BeatmupImageShaderCreate(handle_t *, handle_t env);
void BeatmupImageShaderDestroy(handle_t);
void BeatmupImageShaderSetSourceCode1(handle_t, const char* sourceCode);
void BeatmupImageShaderPrepare(handle_t, handle_t gpu, handle_t input, handle_t output, chandle_t mapping);
void BeatmupImageShaderProcess(handle_t, handle_t gpu);

/* Beatmup::ImageShader::NoSource */
void BeatmupImageShaderNoSourceCreate(handle_t *);
void BeatmupImageShaderNoSourceDestroy(handle_t);

/* Beatmup::ImageShader::UnsupportedInputTextureFormat */
void BeatmupImageShaderUnsupportedInputTextureFormatCreate(handle_t *, const beatmup_g_l_texture_handler_texture_format_t* format);
void BeatmupImageShaderUnsupportedInputTextureFormatDestroy(handle_t);

/* Beatmup::ShaderApplicator */
void BeatmupShaderApplicatorCreate(handle_t *);
void BeatmupShaderApplicatorSetInputBitmap(handle_t, handle_t bitmap);
void BeatmupShaderApplicatorSetOutputBitmap(handle_t, handle_t bitmap);
void BeatmupShaderApplicatorSetShader(handle_t, handle_t shader);
handle_t BeatmupShaderApplicatorGetInputBitmap(chandle_t);
handle_t BeatmupShaderApplicatorGetOutputBitmap(chandle_t);
handle_t BeatmupShaderApplicatorGetShader(chandle_t);
void BeatmupShaderApplicatorDestroy(handle_t);

#ifdef __cplusplus
}
#endif

#endif