#include <stdio.h>
#include <stdlib.h>

#include <rtthread.h>
#include <finsh.h>

#include <jerryscript.h>
#include <jerry_util.h>
#include <jerry_module.h>

int jerry_main(int argc, char** argv)
{
    char *script = NULL;

    if (argc != 2) return -1;

    jerry_init(JERRY_INIT_EMPTY);

    /* Register 'print' function from the extensions */
    jerryx_handler_register_global((const jerry_char_t *)"print",
                                   jerryx_handler_print);
    js_util_init();

    size_t length = js_read_file(argv[1], &script);
    if (length > 0)
    {
    	/* add __filename, __dirname */
		jerry_value_t global_obj  = jerry_get_global_object();
		char *full_path = NULL;
		char *full_dir	= NULL;

		full_path = js_module_normalize_path(NULL, argv[1]);
		full_dir  = js_module_dirname(full_path);

		js_set_string_property(global_obj, "__dirname",  full_dir);
		js_set_string_property(global_obj, "__filename", full_path);
        jerry_release_value(global_obj);

        jerry_value_t parsed_code = jerry_parse((jerry_char_t*)script, length, false);
        if (jerry_value_has_error_flag(parsed_code))
        {
            printf("JavaScript parse failed!\n");
        }
        else
        {
            /* Execute the parsed source code in the Global scope */
            jerry_value_t ret = jerry_run(parsed_code);
            /* Returned value must be freed */
            jerry_release_value(ret);
        }

        /* Parsed source code must be freed */
        jerry_release_value(parsed_code);
        jerry_cleanup();

		free(full_path);
		free(full_dir );
        free(script);
    }
    else
    {
        printf("read file:%s failed!\n", argv[1]);
    }

    return 0;
}
MSH_CMD_EXPORT(jerry_main, jerryScript Demo);
