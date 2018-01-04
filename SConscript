Import('RTT_ROOT')
Import('rtconfig')
from building import *

# get current directory
cwd = GetCurrentDir()
jerry_core_dir = cwd + '/jerryscript/jerry-core'

SOURCE_CORE_API             = Glob(jerry_core_dir + '/*.c')
SOURCE_CORE_ECMA_BASE       = Glob(jerry_core_dir + '/ecma/base/*.c')
SOURCE_CORE_ECMA_BUILTINS   = Glob(jerry_core_dir + '/ecma/builtin-objects/*.c')
SOURCE_CORE_ECMA_OPERATIONS = Glob(jerry_core_dir + '/ecma/operations/*.c')
SOURCE_CORE_JCONTEXT        = Glob(jerry_core_dir + '/jcontext/*.c')
SOURCE_CORE_JMEM            = Glob(jerry_core_dir + '/jmem/*.c')
SOURCE_CORE_JRT             = Glob(jerry_core_dir + '/jrt/*.c')
SOURCE_CORE_LIT             = Glob(jerry_core_dir + '/lit/*.c')
SOURCE_CORE_PARSER_JS       = Glob(jerry_core_dir + '/parser/js/*.c')
SOURCE_CORE_PARSER_REGEXP   = Glob(jerry_core_dir + '/parser/regexp/*.c')
SOURCE_CORE_VM              = Glob(jerry_core_dir + '/vm/*.c')

jerry_core = SOURCE_CORE_API
jerry_core += SOURCE_CORE_ECMA_BASE
jerry_core += SOURCE_CORE_ECMA_BUILTINS
jerry_core += SOURCE_CORE_ECMA_OPERATIONS
jerry_core += SOURCE_CORE_JCONTEXT
jerry_core += SOURCE_CORE_JMEM
jerry_core += SOURCE_CORE_JRT
jerry_core += SOURCE_CORE_LIT
jerry_core += SOURCE_CORE_PARSER_JS
jerry_core += SOURCE_CORE_PARSER_REGEXP
jerry_core += SOURCE_CORE_VM

src = Glob('./rtthread-port/*.c')
src += jerry_core

path = [cwd]
path += [jerry_core_dir]
path += [jerry_core_dir + '/ecma/base']
path += [jerry_core_dir + '/ecma/builtin-objects']
path += [jerry_core_dir + '/ecma/operations']
path += [jerry_core_dir + '/jcontext']
path += [jerry_core_dir + '/jmem']
path += [jerry_core_dir + '/jrt']
path += [jerry_core_dir + '/lit']
path += [jerry_core_dir + '/parser/js']
path += [jerry_core_dir + '/parser/regexp']
path += [jerry_core_dir + '/vm']

#remove other no use files
#SrcRemove(src, '*.c')

LOCAL_CCFLAGS = " -std=c99"
LOCAL_CPPDEFINES = ['JERRY_JS_PARSER']

group = DefineGroup('JerryScript', src, depend = ['PKG_USING_JERRYSCRIPT'], CPPPATH = path, LOCAL_CPPDEFINES = LOCAL_CPPDEFINES, LOCAL_CCFLAGS = LOCAL_CCFLAGS)

Return('group')
