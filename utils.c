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

#define _GNU_SOURCE     /* Use asprintf extension */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "droidcamera.h"

#define MAX_TOKEN_LEN 64

char *droid_camera_params_find(const char *params, const char *key)
{
  char substr[MAX_TOKEN_LEN];
  char *found;

  if (params && key) {
    snprintf(substr, sizeof(substr), "%s=", key);
    found = strstr(params, substr);
    if (found) {
      return found + strlen(substr);
    }
  }
  return NULL;
}

int droid_camera_params_get_int(const char *params, const char *key, int *value)
{
  char *found = droid_camera_params_find(params, key);
  char result[MAX_TOKEN_LEN];

  if (found && value) {
    if (sscanf(found, "%d", value) == 1) {
      return (found - params) + snprintf(result, sizeof(result), "%d", *value);
    }
  }
  return 0;
}

int droid_camera_params_get_dimensions(const char *params, const char *key, int *width, int *height)
{
  char *found = droid_camera_params_find(params, key);
  char result[MAX_TOKEN_LEN];

  if (found && width && height) {
    if (sscanf(found, "%dx%d", width, height) == 2) {
      return (found - params) + snprintf(result, sizeof(result), "%dx%d", *width, *height);
    }
  }
  return 0;
}

int droid_camera_params_get_string(const char *params, const char *key, char **value)
{
  char *found = droid_camera_params_find(params, key);

  if (found && value) {
    if (sscanf(found, "%m[0-9a-zA-Z-]", value) == 1) {
      return (found - params) + strlen(*value);
    }
  }
  return 0;
}

bool droid_camera_params_modify(char **params, const char *key, const char *value)
{
  char *new_params = NULL;
  char *current_value;
  int offset;

  offset = droid_camera_params_get_string(*params, key, &current_value);
  if (offset && current_value) {
    int end;
    char save_sym;

    end = offset;
    offset -= strlen(current_value);
    free(current_value);
    save_sym = (*params)[offset];
    (*params)[offset] = 0;
    if (asprintf(&new_params, "%s%s%s", *params, value, *params + end) > 0) {
      free(*params);
      *params = new_params;
      return true;
    }
    (*params)[offset] = save_sym;
  }
  return false;
}

/* vim: set ts=2 et sw=2 tw=80: */
