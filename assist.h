#ifndef IGT_ASSIST_H
#define IGT_ASSIST_H
#include "hwc2_device/HwcLayer.h"
using namespace android;
#define MAX_PIPE_NUM 4
#define TILING_Y
#ifdef TILING_Y
#define MAX_PLANE_NUM 4
#else
#define MAX_PLANE_NUM 7
#endif
#define MAX_SCALING_NUM 2
#define MAX_LAYER_NUM MAX_PLANE_NUM
#if !defined ANDROID && defined UBUNTU
#include <bits/stdint-uintn.h>
#else
// typedef unsigned char uint8_t;
#endif
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
#ifdef __cplusplus
extern "C" {
#endif
using namespace android;
typedef struct _LayerInfo
{
    int w;
    int h;
    int x;
    int y;
    float alpha; // this is the alpha for whole plane
    uint8_t A; // this is the alpha in ARGB pixel value
    uint8_t R;
    uint8_t G;
    uint8_t B;
    uint8_t scaling;
    uint32_t size;
} LayerInfo;

int open_device();
void* create_fb(uint8_t layer);
LayerInfo* get_layer_info(uint8_t layer);
void* map_fb(uint8_t layer);
void unmap_fb(uint8_t layer, void *mapping_data);
void* map_fb2(DRMHwcNativeHandle handle, uint32_t w, uint32_t h);
void unmap_fb2(DRMHwcNativeHandle handle, void *mapping_data);
#ifdef __cplusplus
}
#endif
#endif

