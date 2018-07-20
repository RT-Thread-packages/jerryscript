from building import *

# get current directory
cwd = GetCurrentDir()
jerry_core_dir = cwd + '/jerryscript/jerry-core'

SOURCE_CORE                 = Glob(jerry_core_dir + '/*.c')
SOURCE_CORE_API             = Glob(jerry_core_dir + '/api/*.c')
SOURCE_CORE_DEBUG           = Glob(jerry_core_dir + '/debugger/*.c')
SOURCE_CORE_ECMA_BASE       = Glob(jerry_core_dir + '/ecma/base/*.c')
SOURCE_CORE_ECMA_BUILTINS   = Glob(jerry_core_dir + '/ecma/builtin-objects/*.c')
SOURCE_CORE_ECMA_BUILTINS_TYPEDARRAY   = Glob(jerry_core_dir + '/ecma/builtin-objects/typedarray/*.c')
SOURCE_CORE_ECMA_OPERATIONS = Glob(jerry_core_dir + '/ecma/operations/*.c')
SOURCE_CORE_JCONTEXT        = Glob(jerry_core_dir + '/jcontext/*.c')
SOURCE_CORE_JMEM            = Glob(jerry_core_dir + '/jmem/*.c')
SOURCE_CORE_JRT             = Glob(jerry_core_dir + '/jrt/*.c')
SOURCE_CORE_LIT             = Glob(jerry_core_dir + '/lit/*.c')
SOURCE_CORE_PARSER_JS       = Glob(jerry_core_dir + '/parser/js/*.c')
SOURCE_CORE_PARSER_REGEXP   = Glob(jerry_core_dir + '/parser/regexp/*.c')
SOURCE_CORE_VM              = Glob(jerry_core_dir + '/vm/*.c')

SOURCE_CORE_PORT            = Glob(cwd + '/rtthread-port/*.c')

jerry_core = SOURCE_CORE
jerry_core += SOURCE_CORE_API
jerry_core += SOURCE_CORE_DEBUG
jerry_core += SOURCE_CORE_ECMA_BASE
jerry_core += SOURCE_CORE_ECMA_BUILTINS
jerry_core += SOURCE_CORE_ECMA_BUILTINS_TYPEDARRAY
jerry_core += SOURCE_CORE_ECMA_OPERATIONS
jerry_core += SOURCE_CORE_JCONTEXT
jerry_core += SOURCE_CORE_JMEM
jerry_core += SOURCE_CORE_JRT
jerry_core += SOURCE_CORE_LIT
jerry_core += SOURCE_CORE_PARSER_JS
jerry_core += SOURCE_CORE_PARSER_REGEXP
jerry_core += SOURCE_CORE_PORT
jerry_core += SOURCE_CORE_VM

jerry_ext_dir = cwd + '/jerryscript/jerry-ext'

SOURCE_EXT_ARG              = Glob(jerry_ext_dir + '/arg/*.c')
SOURCE_EXT_HANDLER          = Glob(jerry_ext_dir + '/handler/*.c')
SOURCE_EXT_INCLUDE          = Glob(jerry_ext_dir + '/include/*.c')
SOURCE_EXT_MODULE           = Glob(jerry_ext_dir + '/module/*.c')

jerry_ext = SOURCE_EXT_ARG + SOURCE_EXT_HANDLER + SOURCE_EXT_INCLUDE + SOURCE_EXT_MODULE

src = jerry_core + jerry_ext

path = [cwd]
path += [cwd + '/rtthread-port']
path += [jerry_core_dir]
path += [jerry_core_dir + '/api']
path += [jerry_core_dir + '/debugger']
path += [jerry_core_dir + '/ecma/base']
path += [jerry_core_dir + '/ecma/builtin-objects']
path += [jerry_core_dir + '/ecma/builtin-objects/typedarray']
path += [jerry_core_dir + '/ecma/operations']
path += [jerry_core_dir + '/include']
path += [jerry_core_dir + '/jcontext']
path += [jerry_core_dir + '/jmem']
path += [jerry_core_dir + '/jrt']
path += [jerry_core_dir + '/lit']
path += [jerry_core_dir + '/parser/js']
path += [jerry_core_dir + '/parser/regexp']
path += [jerry_core_dir + '/vm']

path += [jerry_ext_dir + '/arg']
path += [jerry_ext_dir + '/handler']
path += [jerry_ext_dir + '/include']
path += [jerry_ext_dir + '/module']

LOCAL_CCFLAGS = ''
import rtconfig
if rtconfig.CROSS_TOOL == 'keil':
    LOCAL_CCFLAGS += ' --gnu'

LOCAL_CPPDEFINES = ['JERRY_JS_PARSER', 'JERRY_ENABLE_ERROR_MESSAGES']

group = DefineGroup('JerryScript', src, depend = ['PKG_USING_JERRYSCRIPT'], CPPPATH = path, 
    LOCAL_CPPDEFINES = LOCAL_CPPDEFINES, LOCAL_CCFLAGS = LOCAL_CCFLAGS)

Return('group')
