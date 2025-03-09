## 重要提示！！！PCB开源工程中ADC基准电压使用的可能是REF3033（3.3V）或REF3030 (3.0V)，请务必核实代码中dcdc.h头文件内定义的基准电压是否和PCB元件一致！！！！已经多个队伍复刻时出现这个问题。
### 为何要使用REF3030？如果3.3V供电实际电压低于3.3V，REF3033将无法正确输出电压。

# 【Robomaster 2024】超级电容固件代码及开源报告 - 五大湖联合/英属哥伦比亚大学Pacific Spirit战队

在RoboMaster赛事中，超级电容模组被用于突破底盘功率限制。电容模组通过“填谷削峰”的方式，在实际功率低时将盈余功率用于充电，在实际功率高时放电。

本超级电容使用多相并联四开源Buck-Boost架构，最大功率1200w（20V/60A），电流调整动态响应时间在十~百us级。支持CAN通信，通信协议详见``USER/Inc/cap_canmsg_protocal.h``. 本工程用户代码和CubeMX生成代码为尽可能解耦的形式，主要控制代码位于``USER``文件夹中。

#### 您可以在附件中的文档内获取详细技术报告、工程说明和硬件测试报告。 
#### 原理图PCB工程位于[立创开源平台（OSHWHUB）](https://oshwhub.com/ltyxh/psp-zhan-dui-duo-xiang-chao-dian)
#### 您可以在这里找到使用[演示视频-Bilibili](https://www.bilibili.com/video/BV1VtHreHE9r)

工程已上车验证，请放心食用。

## 更新日志

``V1.2.2`` 本开源工程的版本。进行了多处小修改，PWM信号上加入了RC滤波器，未再次出现稳定性问题。

``V1.2.1`` 历史版本，未开源。怀疑此前问题由辅助供电的DCDC导致，尝试更换DCDC方案。新方案电压耐受不足，继续使用原有方案。

``V1.1/V1.2`` 历史版本，未开源。两相并联架构，尝试了LM5106和UCC27201为驱动芯片。UCC27201上电过程依旧容易出现烧毁问题，LM5106较稳定。给PWM信号上连接示波器探头/加入22pF电容后稳定性改善，怀疑PWM存在串扰导致。模拟采样存在偏差。多个脚位缺少下拉电阻。

``V1.0`` 历史版本，未开源。两相并联架构，使用UCC27211为驱动芯片。上电过程中极易出现烧毁问题。模拟采样存在偏差。

``V0.3`` 暂未开源。使用DrMOS架构，体积极小（仅3*4cm）。控制信号电平有问题，待修复。

``V0.2``在OSHWHUB单独开源，使用SC8802主控芯片。实测在高环路频率下容易失稳，不建议使用。

``V0.1``在OSHWHUB单独开源，为单相四开关设计。模拟部分尤其电流采样存在较大问题，不建议使用。

## 固件构建环境

本工程开发中使用了``GCC 12.2.0``和``GNU make 4.4``进行构建，并使用``Open On-Chip Debugger 0.12.0``下载和调试。

#### gnu make安装
安装gnu make并将路径加入PATH。

#### gcc-arm-none-eabi安装
安装gcc-arm-none-eabi并将路径加入PATH。

注意：如果使用windows安装文件的话，可以在完成安装的界面选择“Add to path"来加入环境变量。

#### OPENOCD安装

OPENOCD可以从[这个release页面](https://github.com/openocd-org/openocd/releases)下载。

1. 解压下载好的OPENOCD，将OPENOCD文件夹下的/bin目录加入PATH。

2. 创建名为`OPENOCD_HOME`的环境变量。变量值为解压后的OPENOCD文件夹路径。

#### 下载程序到MCU
修改`flash.mk`中的这三行为你的项目名，下载器接口和单片机型号。
```makefile
...
PROJECT = YOUR PROJECT NAME.
TARGET = YOUR MCU. Example: stm32g4x
INTERFACE = YOUR DEBUGGER. Example: stlink-v2
...
```
TARGET需要匹配编译出的ELF的文件名，否则会找不到文件。
本工程中可能已经为你改好了。
然后，可以这样：

![图片](https://github.com/user-attachments/assets/12337571-52ef-417b-bfb9-2b61b50ccb31)

![图片](https://github.com/user-attachments/assets/9e7da938-75a6-4fb1-ac71-7e1215893b5a)


当然，也可以直接在shell中输入`make`编译，或使用
```makefile
make -f flash.mk
```
编译并下载到单片机。
## 调试
首先，请在VScode的插件市场中安装`cortex-debug`插件。

![图片](https://github.com/user-attachments/assets/678fb78e-b98d-450f-a36c-37499e926dc5)

在项目的`.vscode`文件夹中新建一个`launch.json`文件。在文件中加入以下内容：

```
{
    "version": "0.2.0",
    "configurations": [
        {
            "cwd": "${workspaceRoot}",
            "executable": "./build/{你的项目名称，例如supercapv4}.elf",
            "name": "Debug with OpenOCD",
            "request": "launch",
            "type": "cortex-debug",
            "servertype": "openocd",
            "configFiles": [
                "{OPENOCD文件夹}/share/openocd/scripts/interface/{你的调试器，如cmsis-dap}.cfg",
                "{OPENOCD文件夹}/share/openocd/scripts/target/{你的MCU，如stm32g4x}.cfg"
            ],
            "searchDir": [],
            "runToEntryPoint": "main",
            "showDevDebugOutput": "none"
        }
    ]
}
```
在下载固件到单片机后，你可以这样启动调试：

![图片](https://github.com/user-attachments/assets/825daeaa-3938-48c7-8c08-9bbb622775e8)


## 许可证

本工程文档以``CC-BY-SA授权``，工程以``GPL3.0``授权。简单来说，您可以自由使用或修改本项目，但必须注明出处并以相同协议开源。

---

# [Robomaster 2024] Open Source Super Capacitor - Team [The Great Lakes/Pacific Spirit], UBC
In RoboMaster, supercapacitor modules are used to break through the base power limitations. By “filling in the valleys and shaving the peaks”, the capacitor modules use the surplus power to charge when the motor power is low and discharge when the motor power is high.

#### This project is an open source supercapacitor for Pacific Spirit/Great Lakes Union. You can access the detailed technical report and project description within the attached PDF document. Note: Currently there's only Chinese version avaliable.
This supercapacitor controller is based on Four-Switch-Buck-Boost structure, with max power 1200w (20V/60A). Current response time < 200us. CAN bus protocal can be found at ``USER/Inc/cap_canmsg_protocal.h``.
#### You can find the PCB project at ``OSHWHUB.com``.[Link](https://oshwhub.com/ltyxh/psp-zhan-dui-duo-xiang-chao-dian)
#### You can find the demonstration video here [supercapcitor-Bilibili](https://www.bilibili.com/video/BV1VtHreHE9r)

The project has been tested on the robot.

The project documentation is licensed under ``CC-BY-SA`` and the project is licensed under ``GPL3.0``. In short, you are free to use or modify the project, provided you credit the source and open source it under the same license.


![oshw_cover](https://github.com/user-attachments/assets/11c34929-6fbd-4c02-ac69-c3f23819f20b)
