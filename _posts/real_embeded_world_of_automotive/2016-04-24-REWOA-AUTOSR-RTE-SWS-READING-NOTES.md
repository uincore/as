---
layout: post
title: AUTOSAR RTE SWS文档阅读笔记
category: real embeded world of automotive
comments: true
---

好记性不如烂笔头。

# ２　ＲＴＥ概述（ｐ６０）
## ２．１　ＲＴＥ在ＡＵＴＯＳＡＲ里的上下文

运行时环境（ＲＴＥ）是ＡＵＴＯＳＡＲ电控单元ＥＣＵ软件架构的核心，运行时环境ＲＴＥ是（针对某个特定电控单元ＥＣＵ）ＡＵＴＯＳＡＲ虚拟功能总线的一个接口层实现。运行时环境ＲＴＥ提供一些基础服务来使能ＡＵＴＯＳＡＲ软件模块（ＳＷＣ）之间的通讯，同时基础软件模块通过运行时环境ＲＴＥ来访问基础软件模块ＢＳＷ提供的服务，如操作系统ＯＳ和通讯服务。

运行时环境ＲＴＥ不仅包含由于映射不同模块到电控单元ＥＣＵ而产生的系统基础架构的可变元素以及标准ＲＴＥ服务（The RTE encompasses both the variable elements of the system infrastructure that　arise from the different mappings of components to ECUs as well as standardized RTE　services.这里贴上原文，因为我的英语好烂，可能翻译有误。）。

原则上，运行时环境ＲＴＥ可以划分为如下两大类实现：

* 软件模块之间的通讯
* 软件模块的调度

为了全面的描述运行时环境ＲＴＥ的概念，基础软件调度器也需要被考虑。基础软件调度器负责调度基础软件模块BSW的可调度实体，在一些文档定义里，调度实体也被称作主（Ｍａｉｎ）功能处理函数。（所以ＢＳＷ经常能看到如Ｆｌｓ\_MainFunction类似的定义）

由于同一个操作系统任务可以用来同时调度软件模块ＳＷＣ和基础软件模块ＢＳＷ，所以运行时环境的调度部分是和基础软件模块强而有力的链接在一起而成为不可分离的一个整体。

对于每一个不同的电控单元ＥＣＵ，其ＲＴＥ是单独配置并生成的，从而保证运行时环境ＲＴＥ和基础软件模块ＢＳＷ的调度是最优化的（我认为性能最优指ＣＰＵ／ＲＯＭ／ＲＡＭ综合指标）。

## ２．２　ＡＵＴＯＳＡＲ的一些概念

### ２．２．１　ＡＵＴＯＳＡＲ软件模块ＳＷＣ

在ＡＵＴＯＳＡＲ的定义里，“应用”软件从概念上说位于ＡＵＴＯＳＡＲ运行时环境ＲＴＥ之上并由“ＡＵＴＯＳＡＲ应用软件模块”和“ＡＵＴＯＳＡＲ传感器制动器模块”组成，“ＡＵＴＯＳＡＲ应用软件模块ＳＷＣ”和电控单元以及位置无关，而“ＡＵＴＯＳＡＲ传感器制动器模块”和电控单元ＥＣＵ的硬件相关因而出于性能和效率的原因其位置不可很容易的改变。也就是说，由于系统设计者引入的一些限制因素除外，一个ＡＵＴＯＳＡＲ软件模块ＳＷＣ在配置阶段可以部署在任何电控单元ＥＣＵ上，之后，运行时环境ＲＴＥ负责保证该软件模块ＳＷＣ可以通讯并保证其功能依然和期望一致，不管该软件模块ＳＷＣ部署在哪里。对于传感器制动器软件，其可能只需要直接访问本地的电控单元ＥＣＵ抽象层即可。因此，访问远程电控单元ＥＣＵ抽象层须通过一个中间传感器制动器软件模块，其会广播信息到远程电控单元ＥＣＵ。因此，将传感器制动器软件模块在不同电控单元ＥＣＵ上部署时，可能需要将真实的设备（传感器和制动器）也移动到那个相同的电控单元ＥＣＵ上以提高访问效率。

一个ＡＵＴＯＳＡＲ软件模块是被模块接口类型定义的，当一个模块被部署到不同电控单元ＥＣＵ上时一个模块类型即被实例化。一个模块可以在一个电控单元上可以被多次实例化如果模块类型是可重复实例化类型。运行时环境支持每个实例都有其单独的内存空间，从而保证每个模块都有其自己的私有状态。(翻译的好差！可能都不对了)

### ２．２．２　基础软件模块ＢＳＷ

同ＡＵＴＯＳＡＲ软件模块ＳＷＣ一样，一个ＡＵＴＯＳＡＲ电控单元也包含基础软件模块ＢＳＷ。基础软件模块ＢＳＷ可以直接访问电控单元抽象层以及其他基础软件模块ＢＳＷ，因此其既不是电控单元ＥＣＵ无关，也不是位置无关。

一个ＡＵＴＯＳＡＲ软件模块ＳＷＣ不可以直接访问基础软件模块ＢＳＷ，所有的通讯都是通过ＡＵＴＯＳＡＲ接口并在运行时环境ＲＴＥ的控制之下。

### ２．２．３　通讯

一个ＡＵＴＯＳＡＲ软件模块ＳＷＣ的通讯接口由一个至多个端口（端口类型接口）号组成，一个ＡＵＴＯＳＡＲ软件模块ＳＷＣ可以通过其接口和其他ＡＵＴＯＳＡＲ软件模块ＳＷＣ（不管软件模块ＳＷＣ是在同一个电控单元ＥＣＵ上还是不同电控单元ＥＣＵ上）和位于同一个电控单元ＥＣＵ上有端口和运行体（如服务软件模块，ＥＣＵ抽象软件模块以及复杂驱动软件模块）的基础软件模块ＢＳＷ进行通讯。通讯只能通过模块的端口号发生。一个端口可分类为发送者－接收者类型或者客户机－服务器类型。一个发送者－接收者接口提供了消息传送的设施而一个客户机－服务器接口提供了功能调用的接口。

#### ２．２．３．１　通讯范例
运行时环境ＲＴＥ为软件模块间的通讯都提供了不同的范例，如发送者－接收者（信号传送），客户机－服务器（功能调用），模式切换以及和非易失性数据存储软件模块之间的交互。

每一个实例可以被应用到同一分区的软件模块间的通讯（包括同一任务和不同任务之间的通讯），不同分区间软件模块的通讯，ＥＣＵ之间的软件模块的通讯。同一任务间的通讯发生于不同的运行体被部署到同一个操作系统的同一个任务里，不同任务间的通讯发生于运行体被部署到同一分区内操作系统的不同任务上时，因此通讯发生时可能导致上下文的切换。分区间的通讯发生于运行体被部署到同一ＥＣＵ上的不同分区上时，因此，会导致上下文的切换并且跨越保护边界（内存保护，时间保护，同一个ＣＰＵ核上的隔离）。ＥＣＵ间的通讯发生于软件模块的运行体被部署到不同ＥＣＵ上时，因此其是通讯是并发性的，且有可能是不可靠的通讯。

_注：看英文原文时，看看就过去了，当试着去翻译，呵呵，发现其实还是好又好动东西没看明白，只能说，呵呵，那些单词我都认识，组合起来，我就晕菜了。看了这儿多，ＲＴＥ还是主要提供ＳＷＣ之间的通讯机制啊！_

#### ２．２．３．２　通讯模式
对于接收者－发送者通讯，ＡＵＴＯＳＡＲ支持两种模式：

* 显示——软件模块调用明确的ＲＴＥ接口去发送和接受数据

* 隐式——ＲＴＥ在运行体运行之前自动的读取一系列数据元素并在运行体结束运行时自动的写入更新一系列数据元素。

单词“隐式”在这里被使用是因为运行体不主动的去开始数据的传出和接收。

#### ２．２．３．３　静态通讯
静态通讯即指在ＲＴＥ生成时，通讯的源地址和目标地址已经确定的通讯链接。因为AUTOSAR的可变因素的处理机制是仅支持从一个所有可能的链接中的超大集合中的一个子集，所以静态通讯也包含了这些可能的通讯链接。

为保证运行时实时性和代码的简单性，动态的通讯配置是不被支持的，从而其通讯只支持RTE适配的那些特定的通讯设备。（好吧，我自己都晕掉了，突然觉得我的中文没英文好）

#### ２．２．３．４ 多重性

RTE不仅支持点对点（1：1）的通讯方式，同样也支持多个提供者或者要求者的通讯链接：

* 发送者-接收者模式时，运行时环境RTE同时支持1：n（一个发送者和多个接收者）和n：1（多个发送者和一个接收者）的通讯链接模式，但有一个限制条件是多个发送者时其不支持模式切换通知。

* 客户机-服务器模式时，运行时环境RTE支持n:1(多个客户机和一个服务器)的通讯方式，其不支持1：n（一个客户机和多个服务器）的客户机-服务器通讯方式。

### ２．２．４　并发性
ＡＵＴＯＳＡＲ的软件模块ＳＷＣ不可以直接访问操作系统接口，因此在ＡＵＴＯＳＡＲ的应用程序里没有任务的概念。作为替代，ＡＵＴＯＳＡＲ内部并发性的活动是基于软件模块ＳＷＣ内部的被运行时环境ＲＴＥ调用可运行体。

根据ＡＵＴＯＳＡＲ虚拟功能总线文档，一个运行体既是可被运行时环境启动执行的一系列指令。一个模块可以有１个或者多个运行体，并且每一个运行体都有一个入口点。一个入口点既是实现了软件模块的一个运行体的函数符号。（语文好差，说白了，一个运行体入口点就是一个Ｃ语言函数名）。

运行时环境ＲＴＥ负责运行体的调度——ＡＵＴＯＳＡＲ软件模块ＳＷＣ不支持动态的去创建私有线程。因此，所有ＡＵＴＯＳＡＲ应用程序的活动是被运行体调度执行，而运行体是被ＲＴＥ事件激活的。

一个ＲＴＥ事件包含了所有的可以通过ＲＴＥ触发运行体运行的状态。５．７．５章节详细的定义了ＲＴＥ事件的类别。

### ５．７．５　触发事件

###＃ ５．７．５．１　时间事件

###＃ ５．７．５．２　背景事件

###＃ ５．７．５．３　软件模块ＳＷＣ模式事件