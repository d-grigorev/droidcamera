/*
 * Copyright (C) 2021 Open Mobile Platform LLC
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <droidcamera.h>

#define TEST_DURATION_SECS 10

static unsigned frame_count = 0;

static bool testapp_find_cameras(void)
{
  int num_cameras = droid_camera_get_number_of_cameras();
  int i;

  printf("Number of cameras: %d\n", num_cameras);

  for (i = 0; i < num_cameras; i++) {
    DroidCameraInfo info;

    if (!droid_camera_get_camera_info(&info, i)) {
      printf("Cannot get DroidCameraInfo for camera %d\n", i);
      continue;
    }

    printf("  Camera %d - facing: %s mount_angle: %d\n", i,
           info.facing == DROID_CAMERA_FACING_FRONT ? "front" : "rear",
           info.mount_angle);
  }

  return num_cameras ? true : false;
}

static void dump_mem(const char *note, void *mem)
{
  uint8_t *m = mem;

  printf("%s: %.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x\n", note,
      m[0], m[1], m[2], m[3],
      m[4], m[5], m[6], m[7]);
}

static bool testapp_on_frame(void *user, DroidCameraBuffer *buffer)
{
  DroidCameraBufferYCbCr ycbcr;

  if (droid_camera_buffer_ycbcr(buffer, &ycbcr)) {
    printf("YUV buffer #%d - y=%p cb=%p cr=%p ystride=%d cstride=%d chroma_step=%d\n",
        frame_count,
        ycbcr.y, ycbcr.cb, ycbcr.cr,
        ycbcr.ystride, ycbcr.cstride,
        ycbcr.chroma_step);
    dump_mem("Y: ", ycbcr.y);
    dump_mem("Cb: ", ycbcr.cb);
    dump_mem("Cr: ", ycbcr.cr);
  } else {
    printf("Encoded frame at %p size %d\n",
        droid_camera_buffer_data(buffer),
        droid_camera_buffer_size(buffer));
  }

  frame_count++;

  return false;
}

static void testapp_on_error(void *user, int arg)
{
    printf("Error %d\n", arg);
}

static bool testapp_configure_camera(DroidCamera *camera, int width, int height, int fps)
{
  bool result = true;
  char *params;
  char param[64];

  params = droid_camera_get_parameters(camera);
  if (!params) {
    printf("Cannot get camera parameters\n");
    return false;
  }

  printf("Camera parameters: %s\n", params);

  snprintf(param, sizeof(param), "%dx%d", width, height);
  result = result & droid_camera_params_modify(&params, "preview-size", param);
  result = result & droid_camera_params_modify(&params, "preview-size-for-video", param);
  result = result & droid_camera_params_modify(&params, "video-size", param);

  snprintf(param, sizeof(param), "%d", fps);
  result = result & droid_camera_params_modify(&params, "preview-frame-rate", param);

  printf("Setting new parameters: %s\n", params);

  if (result) {
    result = droid_camera_set_parameters(camera, params);
  } else {
    printf("Cannot create parameter set\n");
  }

  free(params);

  params = droid_camera_get_parameters(camera);
  if (params) {
    int width, height;

    if (droid_camera_params_get_dimensions(params, "video-size", &width, &height)) {
      printf("Accepted video-size: %dx%d\n", width, height);
    } else {
      printf("Cannot get video-size\n");
      result = false;
    }
    free(params);
  } else {
    printf("Cannot get camera parameters\n");
    result = false;
  }

  return result;
}

bool testapp_capture(int camera_nr)
{
  DroidCamera *camera = droid_camera_connect(camera_nr);
  DroidCameraCallbacks cb;
  bool result = false;

  if (!camera) {
    printf("Cannot open the camera\n");
    return false;
  }

  bzero(&cb, sizeof(cb));
  cb.on_frame = testapp_on_frame;
  cb.on_error = testapp_on_error;
  droid_camera_set_callbacks(camera, &cb, NULL);

  if (!droid_camera_lock(camera)) {
    printf("Cannot lock the camera\n");
    goto out;
  }

  result = testapp_configure_camera(camera, 640, 480, 30);
  if (result) {
    droid_camera_start_capture(camera);
    sleep(TEST_DURATION_SECS);
    result = true;
  } else {
    printf("Cannot configure the camera\n");
  }

out:
  droid_camera_stop_capture(camera);
  droid_camera_unlock(camera);
  droid_camera_disconnect(camera);

  return result;
}

int main(int argc, char *argv[])
{
  bool result = false;

  if (droid_camera_init()) {
    if (testapp_find_cameras()) {
      result = testapp_capture(0);
      printf("Test %s: %d frames captured during %d seconds\n",
          result ? "passed" : "failed", frame_count, TEST_DURATION_SECS);
    } else {
      printf("No cameras found\n");
    }
    droid_camera_fini();
  } else {
    printf("This platform does not support droidcamera\n");
  }

  return result ? EXIT_SUCCESS : EXIT_FAILURE;
}

/* vim: set ts=2 et sw=2 tw=80: */
