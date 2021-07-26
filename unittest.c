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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "droidcamera.h"

#define STRINGIFY(x) (#x)

static unsigned int passed_tests, failed_tests;

#define ASSERT_INT_EQ(x, y) do { \
    if ((x) != (y)) { \
      failed_tests++; \
      fprintf(stderr, "Assertion at %s:%d: %s != %s but %ld instead\n", \
               __FILE__, __LINE__, STRINGIFY(x), STRINGIFY(y), (unsigned long)(x)); \
      return -1; \
    } else { \
      passed_tests++; \
    } \
  } while (0);

static int test_utils(void)
{
  char *test_params = strdup("value1=1;value2=2;dimensions=256x178;video-format=test-H264;not-used");
  int val1, val2;
  char *string;

  ASSERT_INT_EQ(
      droid_camera_params_find(test_params, "value1"),
      test_params + strlen("value1="));

  ASSERT_INT_EQ(
      droid_camera_params_find(test_params, "value2"),
      test_params + strlen("value1=1;value2="));

  ASSERT_INT_EQ(
      droid_camera_params_get_int(test_params, "value1", &val1),
      strlen("value1=1"));

  ASSERT_INT_EQ(
      val1,
      1);

  ASSERT_INT_EQ(
      droid_camera_params_get_int(test_params, "value2", &val2),
      strlen("value1=1;value2=2"));

  ASSERT_INT_EQ(
      val2,
      2);

  ASSERT_INT_EQ(
      droid_camera_params_get_dimensions(test_params, "dimensions", &val1, &val2),
      strlen("value1=1;value2=2;dimensions=256x178"));

  ASSERT_INT_EQ(
      val1,
      256);

  ASSERT_INT_EQ(
      val2,
      178);

  ASSERT_INT_EQ(
      droid_camera_params_get_string(test_params, "video-format", &string),
      strlen("value1=1;value2=2;dimensions=256x178;video-format=test-H264"));

  ASSERT_INT_EQ(
      strcmp(string, "test-H264"),
      0);

  free(string);

  ASSERT_INT_EQ(
      droid_camera_params_get_int(test_params, "no-such-key", &val1),
      0);

  ASSERT_INT_EQ(
      droid_camera_params_find(NULL, "value1"),
      0);


  ASSERT_INT_EQ(
      droid_camera_params_modify(&test_params, "value2", "3"),
      true);

  ASSERT_INT_EQ(
      strcmp(test_params, "value1=1;value2=3;dimensions=256x178;video-format=test-H264;not-used"),
      0);

  printf("%s passed\n", __func__);

  free(test_params);

  return 0;
}

static int unittest(void)
{
  int result;

  passed_tests = failed_tests = 0;

  result = \
    test_utils() |
    0;

  printf("%d tests are passed, %d failed, result %d\n",
         passed_tests, failed_tests, result);
  return result;
}

int main(int argc, char *argv[])
{
  return unittest() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

/* vim: set ts=2 et sw=2 tw=80: */
