# jerryscript on RT-Thread

## 1、介绍

JerryScript是一个轻量级的JavaScript引擎，用于资源受限的设备，如微控制器。它可以运行在小于64 kb RAM和小于200 kb的闪存的设备上。由于这个特性，它被移植到了RT-Thread中。

API Reference ：http://jerryscript.net/api-reference/

### 1.1 目录结构

Jerryscript on RT-Thread软件包目录如下所示：

|  名称 |  说明 |
| - | - |
| example | 示例代码，示范如何导入C++的API与添加builtin module |
| jerryscript | jerryscript 官方库 |
| rtthread-port | RT-Thread 移植代码目录 |

### 1.2 许可证

Jerryscript on RT-Thread软件包遵循Apache-2.0 许可，详见 LICENSE 文件。

### 1.3 依赖

- RT-Thread 3.0+
- finsh 软件包

## 2、 获取软件包

使用 Jerryscript on RT-Thread 软件包需要在 RT-Thread 的包管理中选中它，具体路径如下：

    RT-Thread online packages
        language packages  --->
            [ ] Lua: A lightweight, embeddable scripting language.  --->
            [*] JerryScript: Ultra-lightweight JavaScript engine for the Internet of Things.  --->
            [ ] MicroPython: A lean and efficient Python implementation for microcontrollers and constrained systems.  --->

然后让 RT-Thread 的包管理器自动更新，或者使用 `pkgs --update` 命令更新包到 BSP 中。

## 3、使用Jerryscript on RT-Thread软件包

- 如何导入C++API，请参考[ImportC++](examples/ImportC++/ImportCpp.md)
- 如何添加builtin module，请参考[ImportModule](examples/ImportModule/ImportModule.md)

## 4、 注意事项

- Jerryscript on RT-Thread软件包版本为`latest`，请勿选择版本`V1.0.0`。
- Jerryscript on RT-Thread软件包依赖finsh软件包，请确认在 RT-Thread 的包管理中选中了`finsh`，具体路径如下：
    ```
    RT-Thread Components
        Command shell  --->
            [*] finsh shell
    ```
## 5、 联系方式 & 感谢

- 维护: RT-Thread 开发团队
- 主页: https://github.com/RT-Thread-packages/jerryscript