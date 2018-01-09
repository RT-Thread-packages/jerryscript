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
#ifndef __JS_WRAP_TOOLS_H__
#define __JS_WRAP_TOOLS_H__

#include <stdlib.h>

#include "jerryscript.h"
#include "wrappers.h"

//
// Functions used by the wrapper registration API.
//

bool rt_js_wrap_register_global_function(const char *name,
                                          jerry_external_handler_t handler);

bool rt_js_wrap_register_class_constructor(const char *name,
                                            jerry_external_handler_t handler);

bool rt_js_wrap_register_class_function(jerry_value_t this_obj_p,
                                         const char *name,
                                         jerry_external_handler_t handler);

#endif // __JS_WRAP_TOOLS_H__
