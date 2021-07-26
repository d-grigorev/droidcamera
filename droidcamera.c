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
#include <assert.h>
#include <dlfcn.h>

#include <droidcamera.h>

#define DROID_CAMERA_PLUGIN_FILE "libdroidcamera-plugin.so"

static void *__handle = NULL;

static inline void __ensure_library(void)
{
  if (!__handle) {
    abort();
  }
}

static inline void *__resolve_sym(const char *sym)
{
  __ensure_library();

  void *ptr = dlsym(__handle, sym);
  assert(ptr != NULL);
  if (!ptr) {
    // calling abort() is bad but it does not matter anyway as we will crash.
    abort();
  }

  return ptr;
}

#define DROID_CAMERA_WRAPPER_0_0(sym)                 \
  void sym() {                                        \
    static void (* _sym)() = NULL;                    \
    if (!_sym)                                        \
      _sym = __resolve_sym(#sym);                     \
    _sym();                                           \
  }                                                   \

#define DROID_CAMERA_WRAPPER_0_1(arg0,sym)            \
  void sym(arg0 _arg0) {                              \
    static void (* _sym)(arg0) = NULL;                \
    if (!_sym)                                        \
      _sym = __resolve_sym(#sym);                     \
    _sym(_arg0);                                      \
  }                                                   \

#define DROID_CAMERA_WRAPPER_0_3(arg0,arg1,arg2,sym)  \
  void sym(arg0 _arg0, arg1 _arg1, arg2 _arg2) {      \
    static void (* _sym)(arg0, arg1, arg2) = NULL;    \
    if (!_sym)                                        \
      _sym = __resolve_sym(#sym);                     \
    _sym(_arg0,_arg1, _arg2);                         \
  }                                                   \

#define DROID_CAMERA_WRAPPER_1_0(ret,sym)             \
  ret sym(void) {                                     \
    static ret (* _sym)() = NULL;                     \
    if (!_sym)                                        \
      _sym = __resolve_sym(#sym);                     \
    return _sym();                                    \
  }                                                   \

#define DROID_CAMERA_WRAPPER_1_1(ret,arg0,sym)        \
  ret sym(arg0 _arg0) {                               \
    static ret (* _sym)(arg0) = NULL;                 \
    if (!_sym)                                        \
      _sym = __resolve_sym(#sym);                     \
    return _sym(_arg0);                               \
  }                                                   \

#define DROID_CAMERA_WRAPPER_1_2(ret,arg0,arg1,sym)   \
  ret sym(arg0 _arg0, arg1 _arg1) {                   \
    static ret (* _sym)(arg0, arg1) = NULL;           \
    if (!_sym)                                        \
      _sym = __resolve_sym(#sym);                     \
    return _sym(_arg0,_arg1);                         \
  }                                                   \

DROID_CAMERA_WRAPPER_1_0(bool,_droid_camera_init)

bool droid_camera_init(void)
{
  if (!__handle) {
    __handle = dlopen(DROID_CAMERA_PLUGIN_DIR "/" DROID_CAMERA_PLUGIN_FILE, RTLD_NOW);
    if (!__handle)
      return false;
  }
  return _droid_camera_init();
}

DROID_CAMERA_WRAPPER_0_0(droid_camera_fini)
DROID_CAMERA_WRAPPER_1_0(int,droid_camera_get_number_of_cameras)
DROID_CAMERA_WRAPPER_1_2(bool,DroidCameraInfo*,int,droid_camera_get_camera_info)
DROID_CAMERA_WRAPPER_1_1(DroidCamera*,int,droid_camera_connect)
DROID_CAMERA_WRAPPER_0_1(DroidCamera*,droid_camera_disconnect)
DROID_CAMERA_WRAPPER_1_1(bool,DroidCamera*,droid_camera_lock)
DROID_CAMERA_WRAPPER_1_1(bool,DroidCamera*,droid_camera_unlock)
DROID_CAMERA_WRAPPER_1_1(bool,DroidCamera*,droid_camera_start_capture)
DROID_CAMERA_WRAPPER_0_1(DroidCamera*,droid_camera_stop_capture)
DROID_CAMERA_WRAPPER_1_2(bool,DroidCamera*,const char*,droid_camera_set_parameters)
DROID_CAMERA_WRAPPER_1_1(char*,DroidCamera*,droid_camera_get_parameters)
DROID_CAMERA_WRAPPER_0_3(DroidCamera*,DroidCameraCallbacks*,void*,droid_camera_set_callbacks)
DROID_CAMERA_WRAPPER_0_1(DroidCameraBuffer*,droid_camera_buffer_release)
DROID_CAMERA_WRAPPER_1_1(void*,DroidCameraBuffer*,droid_camera_buffer_data)
DROID_CAMERA_WRAPPER_1_1(unsigned int,DroidCameraBuffer*,droid_camera_buffer_size)
DROID_CAMERA_WRAPPER_1_2(bool,DroidCameraBuffer*,DroidCameraBufferYCbCr*,droid_camera_buffer_ycbcr)

/* vim: set ts=2 et sw=2 tw=80: */
