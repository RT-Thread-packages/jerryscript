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

#ifndef __JERRY_EXTAPI_H__
#define __JERRY_EXTAPI_H__

#define JERRY_STANDALONE_EXIT_CODE_OK (0)
#define JERRY_STANDALONE_EXIT_CODE_FAIL (1)

#define __UNSED__ __attribute__((unused))

#define DECLARE_HANDLER(NAME) \
static jerry_value_t \
NAME##_handler(const jerry_value_t func_value __UNSED__, \
               const jerry_value_t this_value __UNSED__, \
               const jerry_value_t args[],               \
               const jerry_length_t args_cnt)

#define REGISTER_HANDLER(NAME) \
  register_native_function(#NAME, NAME##_handler)

void js_register_functions(void);
bool register_native_function(const char *name,
                              jerry_external_handler_t handler);
void register_js_function(const char *name_p,                  /**< name of the function */
                          jerry_external_handler_t handler_p); /**< function callback */

#endif
