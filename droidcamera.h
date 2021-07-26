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

#ifndef DROID_CAMERA_H
#define DROID_CAMERA_H

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _DroidCamera DroidCamera;
typedef struct _DroidCameraBuffer DroidCameraBuffer;

/*
 * Describes the mapping of a raw YCbCr frame.
 *
 * For more information, see struct android_ycbcr at
 * https://android.googlesource.com/platform/system/core/+/master/libsystem/include/system/graphics.h
 */
typedef struct {
  void *y;
  void *cb;
  void *cr;
  size_t ystride;
  size_t cstride;
  size_t chroma_step;
} DroidCameraBufferYCbCr;

typedef enum {
  DROID_CAMERA_FACING_FRONT,
  DROID_CAMERA_FACING_REAR,
} DroidCameraFacing;

typedef struct {
  DroidCameraFacing facing;
  unsigned mount_angle;
} DroidCameraInfo;

typedef struct {
  bool (*on_frame)(void *user, DroidCameraBuffer *buffer);
  void (*on_error)(void *user, int arg);
} DroidCameraCallbacks;

bool droid_camera_init(void);
void droid_camera_fini(void);

int droid_camera_get_number_of_cameras(void);
bool droid_camera_get_camera_info(DroidCameraInfo *info, int camera_number);

DroidCamera *droid_camera_connect(int camera_number);
void droid_camera_disconnect(DroidCamera *);
bool droid_camera_lock(DroidCamera *);
bool droid_camera_unlock(DroidCamera *);
bool droid_camera_start_capture(DroidCamera *);
void droid_camera_stop_capture(DroidCamera *);

/* Direct access to Android camera parameters */
bool droid_camera_set_parameters(DroidCamera *camera, const char *params);
char *droid_camera_get_parameters(DroidCamera *camera);

void droid_camera_set_callbacks(DroidCamera *camera, DroidCameraCallbacks *cb, void *user);
void droid_camera_buffer_release(DroidCameraBuffer *);

/* Getters */

/* Returns the address of encoded data */
void *droid_camera_buffer_data(DroidCameraBuffer *);
/* Returns the size of encoded data */
unsigned int droid_camera_buffer_size(DroidCameraBuffer *);
/* Returns the raw data buffer info */
bool droid_camera_buffer_ycbcr(DroidCameraBuffer *, DroidCameraBufferYCbCr *ycbcr);

/* Utils */
char *droid_camera_params_find(const char *params, const char *key);
int droid_camera_params_get_int(const char *params, const char *key, int *value);
int droid_camera_params_get_dimensions(const char *params, const char *key, int *width, int *height);
int droid_camera_params_get_string(const char *params, const char *key, char **value);
bool droid_camera_params_modify(char **params, const char *key, const char *value);

#ifdef __cplusplus
};
#endif

#endif /* DROID_CAMERA_H */

/* vim: set ts=2 et sw=2 tw=80: */
