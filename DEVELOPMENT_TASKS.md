# 机械臂滤波可视化教学软件开发任务拆分

## 1. 项目定位

本项目采用 **Qt Widgets + C++** 开发桌面端教学软件，用可视化方式展示机械臂控制、传感器融合、力/力矩传感器处理、视觉伺服中常见滤波方法的效果。

核心目标：

- 让用户通过交互参数理解滤波器的作用、代价和适用场景。
- 将抽象滤波算法放到机械臂语境中展示，例如关节抖动、末端轨迹、力传感器尖峰、视觉目标跟踪。
- 提供较完整的中文说明，包括原理、公式、参数影响、优缺点和调参建议。
- 对复杂图像滤波演示引入 **OpenCV**，普通信号滤波算法使用 C++ 手写实现。

当前工程基础：

- 已有 Qt Widgets `.pro` 工程骨架。
- 后续优先沿用 qmake / `.pro` 推进。
- 如果后期 OpenCV、Qt Charts、部署需求变复杂，可再评估迁移到 CMake。

## 2. 第一版功能范围

第一版目标是做出一个可交互、可演示、结构清晰的 MVP。

优先实现的滤波器：

1. 一阶低通滤波器
2. 滑动平均滤波器
3. 指数加权移动平均 EWMA
4. 中值滤波器
5. Hampel 异常值滤波器
6. 陷波滤波器
7. 互补滤波器
8. 简单一维卡尔曼滤波器
9. 带通滤波器
10. 基础图像滤波：均值、高斯、中值、形态学开/闭运算

第一版场景分类：

1. 关节状态滤波
2. 控制指令平滑
3. 传感器融合
4. 力/力矩传感器滤波
5. 视觉伺服与图像滤波

## 3. 推荐界面结构

主窗口采用工程软件风格布局：

```text
┌──────────────────────────────────────────────┐
│ 顶部工具栏：场景选择 / 播放暂停 / 重置 / 预设参数 │
├──────────────┬───────────────────┬───────────┤
│ 左侧滤波列表   │ 中间可视化区域       │ 右侧参数面板 │
│              │ 曲线图 + 动画/图像    │ 控件实时调参 │
├──────────────┴───────────────────┴───────────┤
│ 下方说明区：原理 / 应用场景 / 参数解释 / 注意事项  │
└──────────────────────────────────────────────┘
```

建议控件：

- `QMainWindow`：主窗口
- `QSplitter`：左右/上下区域可拖拽
- `QListWidget` 或 `QTreeWidget`：场景和滤波器列表
- `QStackedWidget`：不同演示页面切换
- `QSlider` + `QDoubleSpinBox`：参数调节
- `QTextBrowser`：详细文字说明
- `QTimer`：动态信号刷新
- `QPainter` / 自定义 `QWidget`：机械臂和轨迹动画
- `Qt Charts` 或 `QCustomPlot`：曲线显示
- `OpenCV` + `QImage` 转换：图像滤波演示

## 4. 建议目录结构

当前项目根目录以本机实际路径为准：

```text
D:\WORK\Filtering-Visualization-Learning-Tools
```

```text
Filtering-Visualization-Learning-Tools/
  Filtering.pro
  main.cpp
  mainwindow.h
  mainwindow.cpp
  mainwindow.ui
  DEVELOPMENT_TASKS.md

  src/
    core/
      FilterTypes.h
      SignalFrame.h
      DemoContext.h

    filters/
      FilterBase.h
      LowPassFilter.h
      LowPassFilter.cpp
      MovingAverageFilter.h
      MovingAverageFilter.cpp
      EwmaFilter.h
      EwmaFilter.cpp
      MedianFilter.h
      MedianFilter.cpp
      HampelFilter.h
      HampelFilter.cpp
      NotchFilter.h
      NotchFilter.cpp
      BandPassFilter.h
      BandPassFilter.cpp
      ComplementaryFilter.h
      ComplementaryFilter.cpp
      KalmanFilter1D.h
      KalmanFilter1D.cpp

    demos/
      SignalGenerator.h
      SignalGenerator.cpp
      JointStateDemo.h
      JointStateDemo.cpp
      CommandSmoothingDemo.h
      CommandSmoothingDemo.cpp
      ForceTorqueDemo.h
      ForceTorqueDemo.cpp
      SensorFusionDemo.h
      SensorFusionDemo.cpp
      VisionServoDemo.h
      VisionServoDemo.cpp

    widgets/
      SignalChartWidget.h
      SignalChartWidget.cpp
      ParameterPanel.h
      ParameterPanel.cpp
      ExplanationPanel.h
      ExplanationPanel.cpp
      RobotArmWidget.h
      RobotArmWidget.cpp
      ImageFilterWidget.h
      ImageFilterWidget.cpp

    content/
      FilterDescriptions.h
      FilterDescriptions.cpp

    utils/
      MathUtils.h
      MathUtils.cpp
      ImageQtCvBridge.h
      ImageQtCvBridge.cpp

  assets/
    images/
    samples/
```

## 5. 分阶段开发任务

### 阶段 0：工程基础整理

目标：把当前最小 Qt 工程整理成可扩展结构。

任务：

- [x] 确认当前 Qt 版本、编译器、qmake 路径。
- [x] 保留现有 `Filtering.pro`，添加 C++17 配置。
- [x] 新建 `src/`、`assets/` 基础目录。
- [x] 调整 `.pro`，支持分目录源文件。
- [ ] 确认 Debug / Release 都能启动空窗口。

验收标准：

- [x] 程序可编译运行。
- [x] 主窗口标题、初始尺寸、基础布局正常。
- [x] 工程目录清晰，没有无关生成文件纳入源码结构。

### 阶段 1：主界面框架

目标：先搭出完整壳子，不实现复杂算法。

任务：

- [x] 实现左侧场景/滤波器导航。
- [x] 实现中间可视化占位区。
- [x] 实现右侧参数面板占位区。
- [x] 实现下方说明区。
- [x] 增加顶部播放、暂停、重置按钮。
- [x] 使用 `QTimer` 建立统一刷新节拍。

验收标准：

- [x] 点击不同滤波器时，界面标题、参数区、说明区能同步变化。
- [x] 播放/暂停/重置按钮状态正确。
- [ ] 窗口缩放时布局不重叠、不溢出。

### 阶段 2：信号生成与基础曲线显示

目标：建立所有一维滤波演示共用的数据流。

任务：

- [x] 实现 `SignalGenerator`。
- [x] 支持正弦、阶跃、斜坡、脉冲、混合信号。
- [x] 支持高斯噪声、尖峰噪声、周期干扰。
- [x] 实现曲线组件，显示原始信号、带噪信号、滤波信号。
- [x] 实现时间窗口滚动显示。

验收标准：

- [ ] 不接入滤波器时，能稳定显示实时信号。
- [x] 参数变化后曲线立即更新。
- [x] 曲线颜色、图例、坐标轴可读。

### 阶段 3：基础平滑滤波器

目标：实现最常用、最容易理解的一组滤波器。

任务：

- [x] 定义统一滤波器接口 `FilterBase`。
- [x] 实现一阶低通滤波。
- [x] 实现滑动平均滤波。
- [x] 实现 EWMA。
- [x] 实现中值滤波。
- [x] 实现 Hampel 异常值滤波。
- [x] 为每个滤波器配置独立参数面板。
- [x] 为每个滤波器补充中文说明。

验收标准：

- [ ] 各滤波器输出曲线合理。
- [x] 窗口大小、alpha、截止频率等参数变化能实时生效。
- [x] 说明区包含用途、公式、参数影响、优缺点。

### 阶段 4：频域相关滤波器

目标：加入机械臂振动抑制和周期干扰处理相关演示。

任务：

- [ ] 实现陷波滤波器。
- [ ] 实现带通滤波器。
- [ ] 可选实现带阻滤波器。
- [ ] 生成带周期干扰的关节信号。
- [ ] 增加频率响应简化图或说明图。
- [ ] 参数支持中心频率、带宽、采样频率。

验收标准：

- [ ] 陷波能明显压制指定频率干扰。
- [ ] 带通能保留指定频段趋势。
- [ ] 参数设置不应导致程序崩溃或输出 NaN。

### 阶段 5：传感器融合演示

目标：展示 IMU / 编码器等多传感器融合中的典型滤波思想。

任务：

- [ ] 实现互补滤波。
- [ ] 实现一维卡尔曼滤波。
- [ ] 构造位置/角度真实值、慢漂移测量、高频噪声测量。
- [ ] 显示真实值、传感器 A、传感器 B、融合结果。
- [ ] 参数支持互补系数、过程噪声 Q、测量噪声 R。
- [ ] 补充卡尔曼滤波的直观解释，避免只堆公式。

验收标准：

- [ ] 用户能看出互补滤波如何结合低频稳定和高频响应。
- [ ] 用户能通过 Q/R 变化观察卡尔曼滤波信任模型或测量的差异。
- [ ] 说明区解释清楚“滤波不是越强越好”。

### 阶段 6：机械臂语境动画

目标：让滤波效果不只停留在曲线上，而是能映射到机械臂运动。

任务：

- [ ] 实现 `RobotArmWidget`，绘制 2D 二连杆或三连杆机械臂。
- [ ] 将关节角信号映射到机械臂姿态。
- [ ] 显示滤波前末端轨迹与滤波后末端轨迹。
- [ ] 提供关节抖动、指令突变、周期振动等预设。
- [ ] 增加末端轨迹拖尾显示。

验收标准：

- [ ] 用户能直观看到滤波前后的机械臂抖动差异。
- [ ] 动画刷新稳定，不明显卡顿。
- [ ] 轨迹和机械臂不会因为窗口缩放显示异常。

### 阶段 7：力/力矩传感器演示

目标：展示力传感器常见尖峰、噪声、接触突变的处理方式。

任务：

- [ ] 构造力/力矩模拟信号。
- [ ] 支持接触突变、短时冲击、随机尖峰。
- [ ] 对比滑动平均、EWMA、中值、Hampel。
- [ ] 添加“响应速度 vs 平滑程度”的解释。

验收标准：

- [ ] 滑动平均/EWMA 的延迟能被观察到。
- [ ] 中值/Hampel 对尖峰的抑制效果明显。
- [ ] 说明区包含力控场景中的注意事项。

### 阶段 8：OpenCV 图像滤波演示

目标：引入图像滤波和视觉伺服中的预处理概念。

任务：

- [ ] 在 `.pro` 中配置 OpenCV include/lib 路径。
- [ ] 实现 `QImage` 与 `cv::Mat` 的转换工具。
- [ ] 实现图像加载和默认示例图。
- [ ] 实现均值滤波、GaussianBlur、中值滤波。
- [ ] 实现腐蚀、膨胀、开运算、闭运算。
- [ ] 展示原图、噪声图、处理结果。
- [ ] 参数支持核大小、迭代次数、形态学操作类型。

验收标准：

- [ ] OpenCV 相关功能可编译运行。
- [ ] 图像显示颜色正确，不出现 RGB/BGR 通道错乱。
- [ ] 用户能通过参数观察噪声抑制、边缘模糊、孔洞填补等效果。

### 阶段 9：视觉伺服目标跟踪演示

目标：展示视觉测量噪声如何影响目标点，以及卡尔曼滤波如何平滑目标轨迹。

任务：

- [ ] 构造二维目标运动轨迹。
- [ ] 添加像素测量噪声和偶发丢帧。
- [ ] 实现二维简化卡尔曼跟踪，或复用两个一维卡尔曼。
- [ ] 在图像/画布中显示真实轨迹、测量点、滤波估计点。
- [ ] 补充视觉伺服场景说明。

验收标准：

- [ ] 用户能看到测量点抖动和估计轨迹平滑的差异。
- [ ] 丢帧时估计不会立刻崩溃。
- [ ] 说明区解释清楚视觉滤波可能引入的响应滞后。

### 阶段 10：内容完善与交互打磨

目标：让项目从“能跑”变成“适合学习”。

任务：

- [ ] 为每个滤波器补全中文说明。
- [ ] 增加参数预设：轻度滤波、强滤波、响应优先、平滑优先。
- [ ] 增加“重置当前参数”功能。
- [ ] 增加异常参数保护，例如窗口大小必须为奇数、截止频率范围限制。
- [ ] 统一图表颜色和控件命名。
- [ ] 检查高 DPI 显示效果。

验收标准：

- [ ] 新用户能不看代码就理解每个演示在讲什么。
- [ ] 常见误操作不会导致程序崩溃。
- [ ] UI 风格统一，文字不拥挤。

### 阶段 11：构建、测试与交付

目标：形成稳定可演示版本。

任务：

- [ ] 编译 Debug 和 Release。
- [ ] 手动测试所有滤波器切换。
- [ ] 手动测试关键参数边界。
- [ ] 测试 OpenCV 图像加载和滤波。
- [ ] 整理运行说明。
- [ ] 可选：使用 `windeployqt` 打包 Windows 运行目录。

验收标准：

- [ ] Release 程序可独立启动。
- [ ] 演示流程无明显卡顿或崩溃。
- [ ] 文档包含依赖、构建、运行说明。

## 6. 滤波器说明模板

每个滤波器的说明建议统一使用以下结构：

```text
名称：

解决的问题：

机械臂中的典型使用场景：

基本原理：

核心公式：

主要参数：

参数调大/调小的影响：

优点：

缺点：

使用注意事项：

推荐调参思路：
```

## 7. 依赖策略

### Qt 模块

优先使用：

- `core`
- `gui`
- `widgets`

可选使用：

- `charts`：如果本地 Qt 安装包含 Qt Charts，可用于曲线图。

如果 Qt Charts 不可用：

- 方案 A：用 `QPainter` 自绘曲线。
- 方案 B：引入 `QCustomPlot` 单头文件/源文件。

### OpenCV

OpenCV 仅用于图像相关演示，不用于普通一维滤波。

需要确认：

- OpenCV 安装路径
- include 路径
- lib 路径
- Debug / Release 库命名
- 运行时 DLL 是否能被程序找到

`.pro` 中后续可能需要类似配置：

```qmake
INCLUDEPATH += D:/opencv/build/include
LIBS += -LD:/opencv/build/x64/vc16/lib
LIBS += -lopencv_world4xx
```

实际路径和库名以本机安装为准。

## 8. 开发顺序建议

建议严格按下面顺序推进：

1. 先完成主界面框架。
2. 再完成信号生成和曲线显示。
3. 再实现基础一维滤波器。
4. 再加入机械臂动画。
5. 再处理传感器融合和力/力矩演示。
6. 最后接入 OpenCV 图像滤波。

原因：

- 一维信号和界面框架是所有演示的基础。
- OpenCV 配置容易消耗时间，应在主项目稳定后接入。
- 机械臂动画比图像滤波更能体现项目主题，优先级应更高。

## 9. 后续可扩展功能

第一版稳定后可以考虑：

- 二阶低通滤波器
- Butterworth 滤波器
- Savitzky-Golay 平滑
- Alpha-Beta Filter
- 扩展卡尔曼滤波 EKF 概念演示
- 输入整形 Input Shaping
- 速度/加速度/Jerk 限制指令平滑
- 频谱图 FFT 显示
- 参数导入导出
- 演示截图导出
- 教学模式：逐步解释滤波过程

## 10. 每次迭代记录模板

每次开发可以在这里追加记录。

```text
日期：

本次目标：

完成内容：

测试结果：

遗留问题：

下一步：
```

```text
日期：2026-05-28

本次目标：启动阶段 0 工程基础整理

完成内容：新增 .gitignore；确认 Qt 6.9.2、qmake、llvm-mingw 工具链；创建 src/ 与 assets/ 目录骨架；将主窗口源码迁入 src/；调整 Filtering.pro 支持分目录源码；搭建主窗口标题、尺寸、工具栏、三栏工作区和底部说明区占位布局。

测试结果：Release 构建通过，生成 build/release/Filtering.exe；Debug 构建通过，生成 build-debug/debug/Filtering.exe。构建过程中 moc 输出 Qt license client 锁文件警告，但最终退出码为 0。

遗留问题：尚未通过工具启动 GUI 窗口做人工视觉确认；阶段 1 仍需接入真实控件状态同步和 QTimer 刷新节拍。

下一步：进入阶段 1，完善主界面框架的交互状态、参数面板占位组件和说明区联动。
```

```text
日期：2026-05-28

本次目标：推进阶段 1 主界面框架

完成内容：建立滤波器目录数据；左侧滤波器列表与顶部场景选择联动；点击滤波器时同步更新可视化标题、参数摘要和说明区；实现播放、暂停、重置动作状态；使用 QTimer 建立 33 ms 统一刷新节拍并显示刷新帧计数。

测试结果：Release 构建通过，生成 build/release/Filtering.exe；Debug 构建通过，生成 build-debug/debug/Filtering.exe。构建过程中仍出现 Qt license client 锁文件警告，但最终退出码为 0。

遗留问题：尚未打开 GUI 做窗口缩放和视觉布局人工确认；中间区域仍是占位说明，阶段 2 需要接入真实信号生成和曲线显示。

下一步：进入阶段 2，实现 SignalGenerator，并建立原始信号、带噪信号、滤波信号的基础曲线显示数据流。
```

```text
日期：2026-05-28

本次目标：推进阶段 2 信号生成与基础曲线显示

完成内容：新增 SignalFrame 数据结构；实现 SignalGenerator，支持正弦、阶跃、斜坡、脉冲、混合信号以及无噪声、高斯噪声、尖峰噪声、周期噪声、混合噪声；新增 SignalChartWidget 自绘滚动曲线组件，显示原始信号、带噪信号和滤波预览三条曲线；主窗口工具栏增加信号类型和噪声类型选择，切换后立即重置并刷新数据流。

测试结果：Release 构建通过，生成 build/release/Filtering.exe；Debug 构建通过，生成 build-debug/debug/Filtering.exe。构建过程中仍出现 Qt license client 锁文件警告，但最终退出码为 0。

遗留问题：尚未打开 GUI 做长时间播放、窗口缩放和曲线视觉检查；当前“滤波信号”为 EWMA 预览曲线，阶段 3 将替换为各滤波器的真实输出。

下一步：进入阶段 3，定义 FilterBase，并实现一阶低通、滑动平均、EWMA、中值和 Hampel 等基础平滑滤波器。
```

```text
日期：2026-05-28

本次目标：推进阶段 3 基础平滑滤波器

完成内容：新增 FilterBase 统一接口；实现 LowPassFilter、MovingAverageFilter、EwmaFilter、MedianFilter、HampelFilter；主窗口将滤波预览线替换为当前滤波器真实输出；右侧参数面板根据当前滤波器显示截止频率、窗口大小、alpha、Hampel 阈值等控件；参数变化时重置并立即使用新滤波器配置；补充前五个基础滤波器的中文说明，包括用途、公式、参数影响和优缺点。

测试结果：Release 构建通过，生成 build/release/Filtering.exe；Debug 构建通过，生成 build-debug/debug/Filtering.exe。构建过程中仍出现 Qt license client 锁文件警告，但最终退出码为 0。

遗留问题：尚未打开 GUI 人工观察各滤波器曲线是否符合直觉；陷波、带通、互补、卡尔曼和图像滤波仍按后续阶段实现。

下一步：进入阶段 4，优先实现陷波滤波器和带通滤波器，并增加周期干扰场景下的频域相关演示。
```
