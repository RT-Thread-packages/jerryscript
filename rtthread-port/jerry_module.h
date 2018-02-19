#ifndef JERRY_MODULE_H__
#define JERRY_MODULE_H__

char *js_module_dirname(char *path);
char *js_module_normalize_path(const char *directory, const char *filename);

int   js_module_init(void);

#endif

