/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
// #define LOG_NDEBUG 0 // Uncomment to see HWC2 API calls in logcat

#define LOG_TAG "hwc-drm-planes-test-main"

#include <cinttypes>
#include <xf86drm.h>
#include <algorithm>
#include "hwc2_device/DrmHwcTwo.h"
#include "backend/Backend.h"
#include "utils/log.h"
#include "assist.h"
#include "hwc2_device/HwcLayer.h"
#include "utils/properties.h"
#include <string.h>
#include <ui/GraphicBufferAllocator.h>
#include "utils/log.h"
using namespace android;
namespace android {
class DrmHwcTwoTest : public DrmHwcTwo {
public:
    DrmHwcTwoTest() {GetResMan().Init();}
    ~DrmHwcTwoTest() override = default;
};
struct Drmhwc2DeviceTest : hwc2_device {
  DrmHwcTwoTest drmhwctwotest;
};

}
void waitBlank(int fd) {
  drmVBlank vblank;
  vblank.request.type = DRM_VBLANK_RELATIVE;
  vblank.request.sequence = 1;
  vblank.request.signal = 0;
  drmWaitVBlank(fd, &vblank);
}

int main(/*int argc, const char** argv*/) {
  std::map<hwc2_layer_t, HwcLayer> layers;
  HwcVaLayer va_compose_layer_(0);
  va_compose_layer_.fd_ = open_device();
  for (uint8_t i = 0; i < MAX_LAYER_NUM; i++) {
    LayerInfo *layerinfo = get_layer_info(i);
    layers.emplace(static_cast<hwc2_layer_t>(i), HwcLayer(nullptr));
    layers[i].SetLayerBuffer((buffer_handle_t)create_fb(i),-1);
    layers[i].SetLayerZOrder(i);
    layers[i].SetLayerBlendMode((int32_t)HWC2::BlendMode::Premultiplied);
    layers[i].SetLayerPlaneAlpha(layerinfo->A);
    hwc_rect_t display_frame = {layerinfo->x, layerinfo->y, layerinfo->w + layerinfo->x, layerinfo->h + layerinfo->y};
    layers[i].SetLayerDisplayFrame(display_frame);
    hwc_frect_t source_crop = {0.0, 0.0,
      static_cast<float>(layerinfo->w), static_cast<float>(layerinfo->h)};
    layers[i].SetLayerSourceCrop(source_crop);
    layers[i].PopulateLayerData(false);
    va_compose_layer_.addVaLayerMapData(i, &layers[i]);
  }
  va_compose_layer_.SetLayerDisplayFrame(
      (hwc_rect_t){.left = 0,
                    .top = 0,
                    .right = 1920,
                    .bottom = 1080});
  va_compose_layer_.SetLayerSourceCrop(
      (hwc_frect_t){.left = 0,
                    .top = 0,
                    .right = 1920,
                    .bottom = 1080});
  va_compose_layer_.SetLayerBlendMode(HWC2_BLEND_MODE_PREMULTIPLIED);
  va_compose_layer_.vaPopulateLayerData(false);

  gralloc_handle va_handle;
  va_handle.handle_ = va_compose_layer_.GetBufferHandle();
  void* mapping_data = map_fb2(&va_handle, 1920, 1080);
  cros_gralloc_handle *gr_handle = (cros_gralloc_handle *)va_handle.handle_;
  int file_fd = open("dump.img", O_RDWR|O_CREAT, 0666);
  if (file_fd > 0) {
    write(file_fd, mapping_data, gr_handle->sizes[0]);
    close(file_fd);
  }
  unmap_fb2(&va_handle, mapping_data);
  return 0;
}
