/* Copyright JS Foundation and other contributors, http://js.foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __JERRY_RUN_H__
#define __JERRY_RUN_H__
#include <rtthread.h>

char *read_script_sources(const char *script_file_names[], int files_count, size_t *out_source_size_p);
int js_entry(const char *source_p, const size_t source_size);
int js_eval(const char *source_p, const size_t source_size);
void js_exit(void);

#endif
