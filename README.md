# 【Robomaster 2024】超级电容固件代码及开源报告 - 五大湖联合/英属哥伦比亚大学Pacific Spirit战队

在RoboMaster赛事中，超级电容模组被用于突破底盘功率限制。电容模组通过“填谷削峰”的方式，在实际功率低时将盈余功率用于充电，在实际功率高时放电。

本超级电容使用多相并联四开源Buck-Boost架构，最大功率1200w（20V/60A），电流调整动态响应时间在十~百us级。支持CAN通信，通信协议详见``USER/Inc/cap_canmsg_protocal.h``.

#### 您可以在附件中的文档内获取详细技术报告、工程说明和硬件测试报告。 
#### 原理图PCB工程位于[立创开源平台（OSHWHUB）](https://oshwhub.com/ltyxh/psp-zhan-dui-duo-xiang-chao-dian)
#### 您可以在这里找到使用[演示视频-Bilibili](https://www.bilibili.com/video/BV1VtHreHE9r)

工程已上车验证，请放心食用。

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
修改`Makefile`中的这三行为你的项目名，下载器接口和单片机型号。
```makefile
...
PROJECT = YOUR PROJECT NAME.
TARGET = YOUR MCU. Example: stm32g4x
INTERFACE = YOUR DEBUGGER. Example: stlink-v2
...
```
然后，可以这样：

![图片](https://github.com/wele0612/ELEC291-Car-firmware/assets/59970710/cc12c696-5bdf-41e4-a68a-3f24e03480c3)
![图片](https://github.com/wele0612/ELEC291-Car-firmware/assets/59970710/5dceaed0-92b0-42b1-aff6-18c6b46943c2)

当然，也可以直接在shell中输入`make`编译，或使用
```makefile
make -f flash.mk
```
编译并下载到单片机。

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
