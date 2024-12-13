#include "assist.h"
#include "i915_drm.h"


#include "hwc2_device/HwcLayer.h"
using namespace android;

DRMHwcNativeHandle g_layer_handle[MAX_LAYER_NUM];
std::unique_ptr<NativeBufferHandler> buffer_handler_;
#ifdef TILING_Y
LayerInfo layers[MAX_PLANE_NUM] =
{
    {1516,  908, 404,  76, 0.5, 0xff, 0xff, 0x00, 0x00, 0, 5701632},
    {1920, 1080,   0,   0, 0.5, 0xff, 0x00, 0xff, 0x00, 0, 8355840},
    {1920,   76,   0,   0, 0.5, 0xff, 0x00, 0x00, 0xff, 0,  737280},
    {1920 ,  96,   0, 984, 0.5, 0xff, 0xff, 0x00, 0x00, 0,  737280},
};
#else
LayerInfo layers[MAX_PLANE_NUM] =
{
    {1920          , 1080          ,   0,   0, 0.5, 0xff, 0xff, 0x00, 0x00, 0, 0},
    {1920 -  50 * 2, 1080 -  50 * 2,  50,  50, 0.5, 0xff, 0x00, 0xff, 0x00, 0, 0},
    {1920 - 100 * 2, 1080 - 100 * 2, 100, 100, 0.5, 0xff, 0x00, 0x00, 0xff, 0, 0},
    {1920 - 150 * 2, 1080 - 150 * 2, 150, 150, 0.5, 0xff, 0xff, 0x00, 0x00, 0, 0},
    {1920 - 200 * 2, 1080 - 200 * 2, 200, 200, 0.5, 0xff, 0x00, 0xff, 0x00, 0, 0},
    {1920 - 250 * 2, 1080 - 250 * 2, 250, 250, 0.5, 0xff, 0x00, 0x00, 0xff, 0, 0},
    {1920 - 300 * 2, 1080 - 300 * 2, 300, 300, 0.5, 0xff, 0xff, 0x00, 0x00, 0, 0},
};
#endif

LayerInfo* get_layer_info(uint8_t layer) {
    return &layers[layer];
}

int open_device() {
    int fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
    if (fd <= 0) {
        // NOLINTNEXTLINE(concurrency-mt-unsafe): Fixme
        ALOGE("Failed to open dri %s: %s", "/dev/dri/card0", strerror(errno));
        return -ENODEV;
    }
    buffer_handler_.reset(NativeBufferHandler::CreateInstance(fd));
    return fd;
}


void* create_fb(uint8_t layer) {
    bool modifer_succeeded;
    buffer_handler_->CreateBuffer(layers[layer].w,layers[layer].h, DRM_FORMAT_ABGR8888,
                                    &g_layer_handle[layer],3,&modifer_succeeded,0);
    uint32_t stride = 0;
    void* mapping_data = nullptr;
    if (!buffer_handler_->Map(g_layer_handle[layer], 
            0, 0, layers[layer].w,layers[layer].h, 
            &stride, &mapping_data, 1)) {
        ALOGE("map error");
        return nullptr;
    }
#ifdef TILING_Y
    char imgname[32] = {0};
    sprintf(imgname, "%d.img", layer);
    FILE* fp = fopen(imgname, "rb");
    if (!fp) {
        printf("failed to open input\n");
        return 0;
    }
    fread(mapping_data, 1, layers[layer].size, fp);
    fclose(fp);
#else
    int width = layers[layer].w;
    int height = layers[layer].h;
    uint8_t A = layers[layer].A;
    uint8_t R = layers[layer].R;
    uint8_t G = layers[layer].G;
    uint8_t B = layers[layer].B;
    uint32_t *pintbuffer = (uint32_t *)mapping_data;
    uint32_t value = (A << 24) | (B << 16) | (G << 8) | R;
    for (int i = 0; i < width * height; i++)
    {
        pintbuffer[i] = value;
    }
#endif
    buffer_handler_->UnMap(g_layer_handle[layer], mapping_data);
    ALOGE("hwc %s, create %d bufer", __FUNCTION__, layer);
    return (void*)g_layer_handle[layer]->handle_;
}

void* map_fb(uint8_t layer)
{
    return map_fb2(g_layer_handle[layer], layers[layer].w,layers[layer].h);
}

void unmap_fb(uint8_t layer, void *mapping_data)
{
    unmap_fb2(g_layer_handle[layer], mapping_data);    
}

void* map_fb2(DRMHwcNativeHandle handle, uint32_t w, uint32_t h)
{
    uint32_t stride = 0;
    void* mapping_data = nullptr;
    if (!buffer_handler_->Map(handle, 
            0, 0, w,h, 
            &stride, &mapping_data, 1)) {
        ALOGE("map error");
        return nullptr;
    }
    return mapping_data;    
}

void unmap_fb2(DRMHwcNativeHandle handle, void *mapping_data)
{
    buffer_handler_->UnMap(handle, mapping_data);
}
