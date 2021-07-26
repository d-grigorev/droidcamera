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
#include <droidmedia.h>
#include <droidmediacamera.h>

#include "droidcamera.h"

struct _DroidCamera {
  DroidMediaCamera *handle;
  DroidCameraCallbacks cb;
  void *cb_data;
  bool started;
};

struct _DroidCameraBuffer {
  DroidCamera *camera;
  DroidMediaBuffer *handle;
  DroidCameraBufferYCbCr ycbcr;
  DroidMediaCameraRecordingData *rec_data;
};

bool _droid_camera_init(void)
{
  droid_media_init();
  return true;
}

void droid_camera_fini(void)
{
  droid_media_deinit();
}

int droid_camera_get_number_of_cameras(void)
{
  return droid_media_camera_get_number_of_cameras();
}

bool droid_camera_get_camera_info(DroidCameraInfo *info, int camera_number)
{
  DroidMediaCameraInfo droidmedia_info;

  if (info && droid_media_camera_get_info(&droidmedia_info, camera_number)) {
    info->facing =
        droidmedia_info.facing == DROID_MEDIA_CAMERA_FACING_FRONT ?
            DROID_CAMERA_FACING_FRONT : DROID_CAMERA_FACING_REAR;
    info->mount_angle = droidmedia_info.orientation;
    return true;
  }
  return false;
}

static DroidCameraBuffer *
droid_camera_buffer_alloc(DroidCamera *camera)
{
  DroidCameraBuffer *buffer = calloc(sizeof(DroidCameraBuffer), 1);

  if (buffer) {
    buffer->camera = camera;
  }
  return buffer;
}

static void
droid_camera_buffer_free(DroidCameraBuffer *buffer)
{
  free(buffer);
}

static void droid_camera_error_cb(void *user, int arg)
{
  DroidCamera *camera = (DroidCamera *)user;

  if (camera->cb.on_error) {
    camera->cb.on_error(camera->cb_data, arg);
  }
}

static void droid_camera_video_frame_cb(void *user, DroidMediaCameraRecordingData *rec_data)
{
  DroidCamera *camera = (DroidCamera *)user;
  DroidCameraBuffer *buffer = droid_camera_buffer_alloc(camera);

  if (buffer) {
    buffer->rec_data = rec_data;
    if (!camera->cb.on_frame ||
        !camera->cb.on_frame(camera->cb_data, buffer)) {
        /* The buffer is not aquired by the application, so release it */
        droid_camera_buffer_release(buffer);
    }
  }
}

static void droid_camera_buffers_released_cb(void *user)
{
  /* Nothing to do yet */
}

static bool droid_camera_buffer_created_cb(void *user, DroidMediaBuffer *buffer)
{
  /* Nothing to do yet */
  return true;
}

static bool droid_camera_frame_available_cb(void *user, DroidMediaBuffer *droid_buffer)
{
  DroidCamera *camera = (DroidCamera *)user;
  DroidMediaBufferYCbCr ycbcr;
  bool acquired = false;

  if (droid_buffer) {
    if (droid_media_buffer_lock_ycbcr(droid_buffer,
          DROID_MEDIA_BUFFER_LOCK_READ,
          &ycbcr)) {
      DroidCameraBuffer *buffer = droid_camera_buffer_alloc(camera);

      if (buffer) {
        buffer->handle = droid_buffer;
        buffer->ycbcr.y = ycbcr.y;
        buffer->ycbcr.cb = ycbcr.cb;
        buffer->ycbcr.cr = ycbcr.cr;
        buffer->ycbcr.ystride = ycbcr.ystride;
        buffer->ycbcr.cstride = ycbcr.cstride;
        buffer->ycbcr.chroma_step = ycbcr.chroma_step;

        if (camera->cb.on_frame) {
          acquired = camera->cb.on_frame(camera->cb_data, buffer);
        }
      }
    }
  }
  return acquired;
}

DroidCamera *droid_camera_connect(int camera_number)
{
  DroidCamera *camera = calloc(sizeof(DroidCamera), 1);
  DroidMediaBufferQueue *queue;
  DroidMediaCameraCallbacks camera_cb;

  if (camera) {
    camera->handle = droid_media_camera_connect(camera_number);
    if (!camera->handle) {
      free(camera);
      return NULL;
    }

    queue = droid_media_camera_get_recording_buffer_queue(camera->handle);
    if (queue) {
      DroidMediaBufferQueueCallbacks buffer_cb;

      bzero(&buffer_cb, sizeof(buffer_cb));
      buffer_cb.buffers_released = droid_camera_buffers_released_cb;
      buffer_cb.frame_available = droid_camera_frame_available_cb;
      buffer_cb.buffer_created = droid_camera_buffer_created_cb;
      droid_media_buffer_queue_set_callbacks(queue, &buffer_cb, camera);
    }

    bzero(&camera_cb, sizeof(camera_cb));
    camera_cb.error_cb = droid_camera_error_cb;
    if (!queue) {
      /* Capture with camera callback if buffer queue is not supported */
      camera_cb.video_frame_cb = droid_camera_video_frame_cb;
    }
    droid_media_camera_set_callbacks(camera->handle, &camera_cb, camera);
  }
  return camera;
}

void droid_camera_disconnect(DroidCamera *camera)
{
  droid_media_camera_disconnect(camera->handle);
  free(camera);
}

bool droid_camera_lock(DroidCamera *camera)
{
  return droid_media_camera_lock(camera->handle);
}

bool droid_camera_unlock(DroidCamera *camera)
{
  return droid_media_camera_unlock(camera->handle);
}

bool droid_camera_start_capture(DroidCamera *camera)
{
  if (!camera->started) {
    if (!droid_media_camera_start_preview(camera->handle)) {
      return false;
    }

    if (!droid_media_camera_start_recording(camera->handle)) {
      droid_media_camera_stop_preview(camera->handle);
      return false;
    }

    camera->started = true;
  }
  return true;
}

void droid_camera_stop_capture(DroidCamera *camera)
{
  if (camera->started) {
    droid_media_camera_stop_recording(camera->handle);
    droid_media_camera_stop_preview(camera->handle);
  }
}

bool droid_camera_set_parameters(DroidCamera *camera, const char *params)
{
  if (params) {
    return droid_media_camera_set_parameters(camera->handle, params);
  }
  return false;
}

char *droid_camera_get_parameters(DroidCamera *camera)
{
  return droid_media_camera_get_parameters(camera->handle);
}

void droid_camera_set_callbacks(DroidCamera *camera, DroidCameraCallbacks *cb, void *data)
{
  memcpy(&camera->cb, cb, sizeof(*cb));
  camera->cb_data = data;
}

void droid_camera_buffer_release(DroidCameraBuffer *buffer)
{
  if (buffer->handle) {
    droid_media_buffer_unlock(buffer->handle);
    droid_media_buffer_release(buffer->handle, NULL, 0);
  } else {
    droid_media_camera_release_recording_frame(buffer->camera->handle, buffer->rec_data);
  }
  /* TODO: Implement buffer pool */
  droid_camera_buffer_free(buffer);
}

void *droid_camera_buffer_data(DroidCameraBuffer *buffer)
{
  return droid_media_camera_recording_frame_get_data(buffer->rec_data);
}

unsigned int droid_camera_buffer_size(DroidCameraBuffer *buffer)
{
  return droid_media_camera_recording_frame_get_size(buffer->rec_data);
}

bool droid_camera_buffer_ycbcr(DroidCameraBuffer *buffer, DroidCameraBufferYCbCr *ycbcr)
{
  /* If the buffer contains raw data */
  if (buffer->handle) {
    memcpy(ycbcr, &buffer->ycbcr, sizeof(buffer->ycbcr));
    return true;
  }
  return false;
}

/* vim: set ts=2 et sw=2 tw=80: */
