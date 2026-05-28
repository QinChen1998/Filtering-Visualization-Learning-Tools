#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "BandPassFilter.h"
#include "BandStopFilter.h"
#include "ComplementaryFilter.h"
#include "EwmaFilter.h"
#include "FilterBase.h"
#include "ForceTorqueWidget.h"
#include "HampelFilter.h"
#include "KalmanFilter1D.h"
#include "LowPassFilter.h"
#include "MedianFilter.h"
#include "MovingAverageFilter.h"
#include "NotchFilter.h"
#include "RobotArmWidget.h"
#include "SignalChartWidget.h"

#include <QAction>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QRandomGenerator>
#include <QSizePolicy>
#include <QSignalBlocker>
#include <QSplitter>
#include <QSpinBox>
#include <QStatusBar>
#include <QStyle>
#include <QTextBrowser>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>
#include <QtMath>

#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , sceneSelector(nullptr)
    , signalSelector(nullptr)
    , noiseSelector(nullptr)
    , playAction(nullptr)
    , pauseAction(nullptr)
    , resetAction(nullptr)
    , filterList(nullptr)
    , visualTitle(nullptr)
    , visualHint(nullptr)
    , filterNameValue(nullptr)
    , sceneValue(nullptr)
    , sampleRateValue(nullptr)
    , parameterSummaryValue(nullptr)
    , signalModeValue(nullptr)
    , noiseModeValue(nullptr)
    , playbackStateValue(nullptr)
    , frameCounterValue(nullptr)
    , cutoffFrequencyLabel(nullptr)
    , cutoffFrequencySpinBox(nullptr)
    , windowSizeLabel(nullptr)
    , windowSizeSpinBox(nullptr)
    , alphaLabel(nullptr)
    , alphaSpinBox(nullptr)
    , hampelThresholdLabel(nullptr)
    , hampelThresholdSpinBox(nullptr)
    , centerFrequencyLabel(nullptr)
    , centerFrequencySpinBox(nullptr)
    , bandwidthLabel(nullptr)
    , bandwidthSpinBox(nullptr)
    , lowCutoffLabel(nullptr)
    , lowCutoffSpinBox(nullptr)
    , highCutoffLabel(nullptr)
    , highCutoffSpinBox(nullptr)
    , processNoiseLabel(nullptr)
    , processNoiseSpinBox(nullptr)
    , measurementNoiseLabel(nullptr)
    , measurementNoiseSpinBox(nullptr)
    , signalChart(nullptr)
    , robotArmWidget(nullptr)
    , forceTorqueWidget(nullptr)
    , explanationBrowser(nullptr)
    , refreshTimer(new QTimer(this))
    , isPlaying(false)
    , frameCounter(0)
{
    ui->setupUi(this);
    setupFilterCatalog();
    setupToolbar();
    setupCentralLayout();
    connectInteractions();
    updateSelectedFilter(0);
    updateSignalConfiguration();
    updatePlaybackState();

    setWindowTitle(QStringLiteral("机械臂滤波可视化教学工具"));
    resize(1280, 800);
    setMinimumSize(1024, 640);
    statusBar()->showMessage(QStringLiteral("阶段 6：机械臂动画已接入"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupFilterCatalog()
{
    filterCatalog = {
        {
            QStringLiteral("一阶低通滤波器"),
            QStringLiteral("关节状态滤波"),
            QStringLiteral("100 Hz"),
            {QStringLiteral("截止频率"), QStringLiteral("采样周期"), QStringLiteral("初始状态")},
            QStringLiteral("<h3>一阶低通滤波器</h3>"
                           "<p><b>解决的问题：</b>削弱高频噪声，让关节角度、速度或控制量更平滑。</p>"
                           "<p><b>机械臂场景：</b>编码器读数轻微抖动、末端轨迹显示抖动。</p>"
                           "<p><b>核心公式：</b>y[n] = y[n-1] + alpha * (x[n] - y[n-1])，alpha 由截止频率和采样周期决定。</p>"
                           "<p><b>参数影响：</b>截止频率越低越平滑，但响应越慢；截止频率越高越灵敏，但噪声保留更多。</p>"
                           "<p><b>优缺点：</b>实现简单、实时性好；缺点是会带来相位滞后。</p>")
        },
        {
            QStringLiteral("滑动平均滤波器"),
            QStringLiteral("控制指令平滑"),
            QStringLiteral("100 Hz"),
            {QStringLiteral("窗口大小"), QStringLiteral("边界策略")},
            QStringLiteral("<h3>滑动平均滤波器</h3>"
                           "<p><b>解决的问题：</b>用固定长度窗口平均近期样本，降低随机噪声。</p>"
                           "<p><b>机械臂场景：</b>平滑速度指令、力传感器低幅随机噪声。</p>"
                           "<p><b>核心公式：</b>y[n] = (x[n] + ... + x[n-N+1]) / N。</p>"
                           "<p><b>参数影响：</b>窗口越大越平滑，同时引入更明显的延迟。</p>"
                           "<p><b>优缺点：</b>直观稳定；缺点是对尖峰仍会被平均影响，且窗口越大越迟钝。</p>")
        },
        {
            QStringLiteral("EWMA 滤波器"),
            QStringLiteral("控制指令平滑"),
            QStringLiteral("100 Hz"),
            {QStringLiteral("alpha"), QStringLiteral("初始输出")},
            QStringLiteral("<h3>指数加权移动平均 EWMA</h3>"
                           "<p><b>解决的问题：</b>用指数权重保留历史趋势，计算量小，适合实时控制。</p>"
                           "<p><b>机械臂场景：</b>对控制指令做轻量平滑，避免突然抖动。</p>"
                           "<p><b>核心公式：</b>y[n] = alpha * x[n] + (1 - alpha) * y[n-1]。</p>"
                           "<p><b>参数影响：</b>alpha 越大越跟手，alpha 越小越平滑。</p>"
                           "<p><b>优缺点：</b>状态量少、计算便宜；缺点是 alpha 需要按响应速度取舍。</p>")
        },
        {
            QStringLiteral("中值滤波器"),
            QStringLiteral("力/力矩传感器滤波"),
            QStringLiteral("100 Hz"),
            {QStringLiteral("窗口大小"), QStringLiteral("奇数窗口保护")},
            QStringLiteral("<h3>中值滤波器</h3>"
                           "<p><b>解决的问题：</b>抑制孤立尖峰，比均值类滤波更不容易被异常点拖偏。</p>"
                           "<p><b>机械臂场景：</b>力/力矩传感器偶发冲击、视觉测量离群点。</p>"
                           "<p><b>核心公式：</b>y[n] = median(x[n-k] ... x[n+k])。</p>"
                           "<p><b>参数影响：</b>窗口越大，尖峰抑制更强，但细节和响应速度会下降。</p>"
                           "<p><b>优缺点：</b>抗离群点强；缺点是不适合保留非常快速的真实变化。</p>")
        },
        {
            QStringLiteral("Hampel 异常值滤波器"),
            QStringLiteral("力/力矩传感器滤波"),
            QStringLiteral("100 Hz"),
            {QStringLiteral("窗口大小"), QStringLiteral("阈值倍数"), QStringLiteral("替换策略")},
            QStringLiteral("<h3>Hampel 异常值滤波器</h3>"
                           "<p><b>解决的问题：</b>基于中位数和绝对偏差识别异常值，再用稳健估计替换。</p>"
                           "<p><b>机械臂场景：</b>接触检测中的短时尖峰、传感器偶发毛刺。</p>"
                           "<p><b>核心公式：</b>若 |x - median| > k * 1.4826 * MAD，则用 median 替换。</p>"
                           "<p><b>参数影响：</b>阈值越小越敏感，但可能误判真实快速变化。</p>"
                           "<p><b>优缺点：</b>能稳健处理异常点；缺点是窗口和阈值需要结合场景调整。</p>")
        },
        {
            QStringLiteral("陷波滤波器"),
            QStringLiteral("关节状态滤波"),
            QStringLiteral("100 Hz"),
            {QStringLiteral("中心频率"), QStringLiteral("带宽"), QStringLiteral("采样频率")},
            QStringLiteral("<h3>陷波滤波器</h3>"
                           "<p><b>解决的问题：</b>压制指定频率附近的周期干扰。</p>"
                           "<p><b>机械臂场景：</b>电机振动、结构共振或电源纹波造成的周期噪声。</p>"
                           "<p><b>核心公式：</b>使用二阶 biquad 陷波结构，在中心频率处形成深衰减。</p>"
                           "<p><b>参数影响：</b>带宽越宽抑制范围越大，但可能伤及有用信号；中心频率应对准干扰频率。</p>"
                           "<p><b>优缺点：</b>对单一周期干扰很有效；缺点是参数错位时抑制效果会明显下降。</p>")
        },
        {
            QStringLiteral("互补滤波器"),
            QStringLiteral("传感器融合"),
            QStringLiteral("200 Hz"),
            {QStringLiteral("互补系数"), QStringLiteral("传感器 A 权重"), QStringLiteral("传感器 B 权重")},
            QStringLiteral("<h3>互补滤波器</h3>"
                           "<p><b>解决的问题：</b>把低频稳定传感器和高频响应传感器结合起来。</p>"
                           "<p><b>机械臂场景：</b>IMU 与编码器姿态估计融合。</p>"
                           "<p><b>核心公式：</b>prediction = 上次融合值 + 传感器 B 的快速变化量；y = alpha * prediction + (1 - alpha) * 传感器 A。</p>"
                           "<p><b>参数影响：</b>alpha 越大越信任高频响应，alpha 越小越贴近低频稳定测量。</p>"
                           "<p><b>优缺点：</b>直观、计算量低；缺点是 alpha 需要按传感器特性调节。</p>")
        },
        {
            QStringLiteral("一维卡尔曼滤波器"),
            QStringLiteral("传感器融合"),
            QStringLiteral("200 Hz"),
            {QStringLiteral("过程噪声 Q"), QStringLiteral("测量噪声 R"), QStringLiteral("初始协方差")},
            QStringLiteral("<h3>一维卡尔曼滤波器</h3>"
                           "<p><b>解决的问题：</b>在模型预测和带噪测量之间动态分配信任。</p>"
                           "<p><b>机械臂场景：</b>位置、速度或视觉目标的一维估计。</p>"
                           "<p><b>核心公式：</b>先用 P = P + Q 预测不确定性，再用 K = P / (P + R) 计算卡尔曼增益。</p>"
                           "<p><b>参数影响：</b>Q 越大越信任新变化，R 越大越不信任测量。</p>"
                           "<p><b>优缺点：</b>能自适应平衡平滑和响应；缺点是 Q/R 没有万能值，需要结合噪声模型调参。</p>")
        },
        {
            QStringLiteral("带通滤波器"),
            QStringLiteral("关节状态滤波"),
            QStringLiteral("500 Hz"),
            {QStringLiteral("低截止频率"), QStringLiteral("高截止频率"), QStringLiteral("采样频率")},
            QStringLiteral("<h3>带通滤波器</h3>"
                           "<p><b>解决的问题：</b>只保留指定频段内的信号成分。</p>"
                           "<p><b>机械臂场景：</b>观察振动频段、诊断周期性机械扰动。</p>"
                           "<p><b>核心公式：</b>使用二阶 biquad 带通结构，按低/高截止频率换算中心频率和 Q 值。</p>"
                           "<p><b>参数影响：</b>频带过窄可能漏掉目标动态，过宽则噪声保留更多。</p>"
                           "<p><b>优缺点：</b>适合突出振动频段；缺点是会削弱直流和慢变化趋势。</p>")
        },
        {
            QStringLiteral("带阻滤波器"),
            QStringLiteral("关节状态滤波"),
            QStringLiteral("500 Hz"),
            {QStringLiteral("低截止频率"), QStringLiteral("高截止频率"), QStringLiteral("采样频率")},
            QStringLiteral("<h3>带阻滤波器</h3>"
                           "<p><b>解决的问题：</b>抑制某一段频率范围内的干扰，同时保留阻带之外的低频趋势和高频成分。</p>"
                           "<p><b>机械臂场景：</b>结构振动集中在一段频带时，可用来削弱该频段对关节测量或力传感器读数的影响。</p>"
                           "<p><b>核心公式：</b>使用二阶 biquad 带阻结构，按低/高截止频率换算中心频率和 Q 值。</p>"
                           "<p><b>参数影响：</b>阻带越宽抑制范围越大，但更容易削弱有用动态；阻带越窄越精准，但需要更准确地知道干扰频段。</p>"
                           "<p><b>优缺点：</b>适合一段已知频率干扰；缺点是参数设置不当会误伤有效信号。</p>")
        },
        {
            QStringLiteral("图像滤波"),
            QStringLiteral("视觉伺服与图像滤波"),
            QStringLiteral("30 FPS"),
            {QStringLiteral("核大小"), QStringLiteral("迭代次数"), QStringLiteral("形态学操作")},
            QStringLiteral("<h3>基础图像滤波</h3>"
                           "<p><b>解决的问题：</b>在视觉伺服前处理图像噪声、边缘模糊和孔洞。</p>"
                           "<p><b>机械臂场景：</b>目标点检测、轮廓提取、视觉测量稳定化。</p>"
                           "<p><b>参数影响：</b>核越大处理越强，但可能损失细节。</p>")
        },
    };
}

void MainWindow::setupToolbar()
{
    auto *toolBar = addToolBar(QStringLiteral("主工具栏"));
    toolBar->setMovable(false);

    sceneSelector = new QComboBox(toolBar);
    sceneSelector->addItems({
        QStringLiteral("关节状态滤波"),
        QStringLiteral("控制指令平滑"),
        QStringLiteral("传感器融合"),
        QStringLiteral("力/力矩传感器滤波"),
        QStringLiteral("视觉伺服与图像滤波"),
    });
    sceneSelector->setMinimumWidth(180);
    toolBar->addWidget(sceneSelector);
    toolBar->addSeparator();

    toolBar->addWidget(new QLabel(QStringLiteral("信号"), toolBar));
    signalSelector = new QComboBox(toolBar);
    signalSelector->addItems({
        QStringLiteral("正弦"),
        QStringLiteral("阶跃"),
        QStringLiteral("斜坡"),
        QStringLiteral("脉冲"),
        QStringLiteral("混合"),
    });
    signalSelector->setMinimumWidth(100);
    toolBar->addWidget(signalSelector);

    toolBar->addWidget(new QLabel(QStringLiteral("噪声"), toolBar));
    noiseSelector = new QComboBox(toolBar);
    noiseSelector->addItems({
        QStringLiteral("无"),
        QStringLiteral("高斯"),
        QStringLiteral("尖峰"),
        QStringLiteral("周期"),
        QStringLiteral("混合"),
    });
    noiseSelector->setCurrentIndex(1);
    noiseSelector->setMinimumWidth(100);
    toolBar->addWidget(noiseSelector);
    toolBar->addSeparator();

    playAction = toolBar->addAction(style()->standardIcon(QStyle::SP_MediaPlay), QStringLiteral("播放"));
    pauseAction = toolBar->addAction(style()->standardIcon(QStyle::SP_MediaPause), QStringLiteral("暂停"));
    resetAction = toolBar->addAction(style()->standardIcon(QStyle::SP_BrowserReload), QStringLiteral("重置"));
    toolBar->addSeparator();
    toolBar->addAction(QStringLiteral("预设参数"));
}

void MainWindow::setupCentralLayout()
{
    auto *central = new QWidget(this);
    auto *rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(8, 8, 8, 8);
    rootLayout->setSpacing(8);

    auto *mainSplitter = new QSplitter(Qt::Vertical, central);
    auto *workSplitter = new QSplitter(Qt::Horizontal, mainSplitter);

    filterList = new QListWidget(workSplitter);
    for (const FilterInfo &filter : filterCatalog) {
        filterList->addItem(filter.name);
    }
    filterList->setCurrentRow(0);
    filterList->setMinimumWidth(180);

    auto *visualFrame = new QFrame(workSplitter);
    visualFrame->setFrameShape(QFrame::StyledPanel);
    visualFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *visualLayout = new QVBoxLayout(visualFrame);
    visualTitle = new QLabel(QStringLiteral("可视化区域"), visualFrame);
    visualHint = new QLabel(QStringLiteral("曲线图、机械臂动画和图像滤波结果将在这里展示。"), visualFrame);
    visualHint->setAlignment(Qt::AlignCenter);
    visualHint->setWordWrap(true);
    signalChart = new SignalChartWidget(visualFrame);
    robotArmWidget = new RobotArmWidget(visualFrame);
    forceTorqueWidget = new ForceTorqueWidget(visualFrame);
    visualLayout->addWidget(visualTitle);
    visualLayout->addWidget(signalChart, 1);
    visualLayout->addWidget(robotArmWidget, 1);
    visualLayout->addWidget(forceTorqueWidget, 1);
    visualLayout->addWidget(visualHint);
    forceTorqueWidget->setVisible(false);

    auto *parameterFrame = new QFrame(workSplitter);
    parameterFrame->setFrameShape(QFrame::StyledPanel);
    parameterFrame->setMinimumWidth(240);
    auto *parameterLayout = new QFormLayout(parameterFrame);
    filterNameValue = new QLabel(parameterFrame);
    sceneValue = new QLabel(parameterFrame);
    sampleRateValue = new QLabel(parameterFrame);
    parameterSummaryValue = new QLabel(parameterFrame);
    signalModeValue = new QLabel(parameterFrame);
    noiseModeValue = new QLabel(parameterFrame);
    playbackStateValue = new QLabel(parameterFrame);
    frameCounterValue = new QLabel(parameterFrame);
    parameterSummaryValue->setWordWrap(true);
    parameterLayout->addRow(QStringLiteral("滤波器"), filterNameValue);
    parameterLayout->addRow(QStringLiteral("场景"), sceneValue);
    parameterLayout->addRow(QStringLiteral("采样频率"), sampleRateValue);
    parameterLayout->addRow(QStringLiteral("参数区"), parameterSummaryValue);
    parameterLayout->addRow(QStringLiteral("信号类型"), signalModeValue);
    parameterLayout->addRow(QStringLiteral("噪声类型"), noiseModeValue);
    parameterLayout->addRow(QStringLiteral("播放状态"), playbackStateValue);
    parameterLayout->addRow(QStringLiteral("刷新帧"), frameCounterValue);

    cutoffFrequencyLabel = new QLabel(QStringLiteral("截止频率"), parameterFrame);
    cutoffFrequencySpinBox = new QDoubleSpinBox(parameterFrame);
    cutoffFrequencySpinBox->setRange(0.1, 40.0);
    cutoffFrequencySpinBox->setDecimals(2);
    cutoffFrequencySpinBox->setSingleStep(0.5);
    cutoffFrequencySpinBox->setSuffix(QStringLiteral(" Hz"));
    cutoffFrequencySpinBox->setValue(2.0);
    parameterLayout->addRow(cutoffFrequencyLabel, cutoffFrequencySpinBox);

    windowSizeLabel = new QLabel(QStringLiteral("窗口大小"), parameterFrame);
    windowSizeSpinBox = new QSpinBox(parameterFrame);
    windowSizeSpinBox->setRange(1, 101);
    windowSizeSpinBox->setSingleStep(2);
    windowSizeSpinBox->setValue(9);
    parameterLayout->addRow(windowSizeLabel, windowSizeSpinBox);

    alphaLabel = new QLabel(QStringLiteral("alpha"), parameterFrame);
    alphaSpinBox = new QDoubleSpinBox(parameterFrame);
    alphaSpinBox->setRange(0.01, 1.0);
    alphaSpinBox->setDecimals(3);
    alphaSpinBox->setSingleStep(0.01);
    alphaSpinBox->setValue(0.12);
    parameterLayout->addRow(alphaLabel, alphaSpinBox);

    hampelThresholdLabel = new QLabel(QStringLiteral("Hampel 阈值"), parameterFrame);
    hampelThresholdSpinBox = new QDoubleSpinBox(parameterFrame);
    hampelThresholdSpinBox->setRange(0.5, 8.0);
    hampelThresholdSpinBox->setDecimals(1);
    hampelThresholdSpinBox->setSingleStep(0.5);
    hampelThresholdSpinBox->setSuffix(QStringLiteral(" MAD"));
    hampelThresholdSpinBox->setValue(3.0);
    parameterLayout->addRow(hampelThresholdLabel, hampelThresholdSpinBox);

    centerFrequencyLabel = new QLabel(QStringLiteral("中心频率"), parameterFrame);
    centerFrequencySpinBox = new QDoubleSpinBox(parameterFrame);
    centerFrequencySpinBox->setRange(0.1, 45.0);
    centerFrequencySpinBox->setDecimals(2);
    centerFrequencySpinBox->setSingleStep(0.5);
    centerFrequencySpinBox->setSuffix(QStringLiteral(" Hz"));
    centerFrequencySpinBox->setValue(8.0);
    parameterLayout->addRow(centerFrequencyLabel, centerFrequencySpinBox);

    bandwidthLabel = new QLabel(QStringLiteral("带宽"), parameterFrame);
    bandwidthSpinBox = new QDoubleSpinBox(parameterFrame);
    bandwidthSpinBox->setRange(0.1, 30.0);
    bandwidthSpinBox->setDecimals(2);
    bandwidthSpinBox->setSingleStep(0.5);
    bandwidthSpinBox->setSuffix(QStringLiteral(" Hz"));
    bandwidthSpinBox->setValue(1.5);
    parameterLayout->addRow(bandwidthLabel, bandwidthSpinBox);

    lowCutoffLabel = new QLabel(QStringLiteral("低截止"), parameterFrame);
    lowCutoffSpinBox = new QDoubleSpinBox(parameterFrame);
    lowCutoffSpinBox->setRange(0.1, 40.0);
    lowCutoffSpinBox->setDecimals(2);
    lowCutoffSpinBox->setSingleStep(0.5);
    lowCutoffSpinBox->setSuffix(QStringLiteral(" Hz"));
    lowCutoffSpinBox->setValue(3.0);
    parameterLayout->addRow(lowCutoffLabel, lowCutoffSpinBox);

    highCutoffLabel = new QLabel(QStringLiteral("高截止"), parameterFrame);
    highCutoffSpinBox = new QDoubleSpinBox(parameterFrame);
    highCutoffSpinBox->setRange(0.2, 45.0);
    highCutoffSpinBox->setDecimals(2);
    highCutoffSpinBox->setSingleStep(0.5);
    highCutoffSpinBox->setSuffix(QStringLiteral(" Hz"));
    highCutoffSpinBox->setValue(12.0);
    parameterLayout->addRow(highCutoffLabel, highCutoffSpinBox);

    processNoiseLabel = new QLabel(QStringLiteral("过程噪声 Q"), parameterFrame);
    processNoiseSpinBox = new QDoubleSpinBox(parameterFrame);
    processNoiseSpinBox->setRange(0.0001, 1.0);
    processNoiseSpinBox->setDecimals(4);
    processNoiseSpinBox->setSingleStep(0.001);
    processNoiseSpinBox->setValue(0.01);
    parameterLayout->addRow(processNoiseLabel, processNoiseSpinBox);

    measurementNoiseLabel = new QLabel(QStringLiteral("测量噪声 R"), parameterFrame);
    measurementNoiseSpinBox = new QDoubleSpinBox(parameterFrame);
    measurementNoiseSpinBox->setRange(0.0001, 2.0);
    measurementNoiseSpinBox->setDecimals(4);
    measurementNoiseSpinBox->setSingleStep(0.01);
    measurementNoiseSpinBox->setValue(0.08);
    parameterLayout->addRow(measurementNoiseLabel, measurementNoiseSpinBox);

    workSplitter->setStretchFactor(0, 0);
    workSplitter->setStretchFactor(1, 1);
    workSplitter->setStretchFactor(2, 0);
    workSplitter->setSizes({220, 760, 300});

    explanationBrowser = new QTextBrowser(mainSplitter);
    explanationBrowser->setMinimumHeight(160);

    mainSplitter->addWidget(workSplitter);
    mainSplitter->addWidget(explanationBrowser);
    mainSplitter->setStretchFactor(0, 1);
    mainSplitter->setStretchFactor(1, 0);
    mainSplitter->setSizes({580, 180});

    rootLayout->addWidget(mainSplitter);
    setCentralWidget(central);
}

void MainWindow::connectInteractions()
{
    refreshTimer->setInterval(33);
    connect(refreshTimer, &QTimer::timeout, this, &MainWindow::advanceFrame);
    connect(playAction, &QAction::triggered, this, [this]() {
        isPlaying = true;
        refreshTimer->start();
        updatePlaybackState();
    });
    connect(pauseAction, &QAction::triggered, this, [this]() {
        isPlaying = false;
        refreshTimer->stop();
        updatePlaybackState();
    });
    connect(resetAction, &QAction::triggered, this, &MainWindow::resetSimulation);
    connect(signalSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::updateSignalConfiguration);
    connect(noiseSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::updateSignalConfiguration);
    connect(cutoffFrequencySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this]() {
        configureActiveFilter();
        resetSimulation();
    });
    connect(windowSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this]() {
        if (windowSizeSpinBox->value() % 2 == 0) {
            windowSizeSpinBox->setValue(windowSizeSpinBox->value() + 1);
            return;
        }
        configureActiveFilter();
        resetSimulation();
    });
    connect(alphaSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this]() {
        configureActiveFilter();
        resetSimulation();
    });
    connect(hampelThresholdSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this]() {
        configureActiveFilter();
        resetSimulation();
    });
    connect(centerFrequencySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this]() {
        configureActiveFilter();
        resetSimulation();
    });
    connect(bandwidthSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this]() {
        configureActiveFilter();
        resetSimulation();
    });
    connect(lowCutoffSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this]() {
        if (lowCutoffSpinBox->value() >= highCutoffSpinBox->value()) {
            highCutoffSpinBox->setValue(lowCutoffSpinBox->value() + 0.5);
            return;
        }
        configureActiveFilter();
        resetSimulation();
    });
    connect(highCutoffSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this]() {
        if (highCutoffSpinBox->value() <= lowCutoffSpinBox->value()) {
            lowCutoffSpinBox->setValue(qMax(0.1, highCutoffSpinBox->value() - 0.5));
            return;
        }
        configureActiveFilter();
        resetSimulation();
    });
    connect(processNoiseSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this]() {
        configureActiveFilter();
        resetSimulation();
    });
    connect(measurementNoiseSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this]() {
        configureActiveFilter();
        resetSimulation();
    });
    connect(filterList, &QListWidget::currentRowChanged, this, &MainWindow::updateSelectedFilter);
    connect(sceneSelector, &QComboBox::currentTextChanged, this, [this](const QString &scene) {
        for (int row = 0; row < filterCatalog.size(); ++row) {
            if (filterCatalog.at(row).scene == scene) {
                filterList->setCurrentRow(row);
                return;
            }
        }
    });
}

void MainWindow::updateSelectedFilter(int row)
{
    if (row < 0 || row >= filterCatalog.size()) {
        return;
    }

    const FilterInfo &filter = filterCatalog.at(row);
    filterNameValue->setText(filter.name);
    sceneValue->setText(filter.scene);
    sampleRateValue->setText(filter.sampleRate);
    visualTitle->setText(QStringLiteral("可视化区域 - %1").arg(filter.name));
    visualHint->setText(QStringLiteral("%1 场景的实时曲线、动画或图像结果将在这里接入。").arg(filter.scene));
    QString stageNote = QStringLiteral("<p><b>当前阶段：</b>基础平滑、频域滤波和传感器融合滤波器已接入真实输出曲线；图像滤波将在后续阶段实现。</p>");
    if (row == 5) {
        stageNote += QStringLiteral("<pre>简化频率响应：低频 ───── 中心频率附近 ▼ 深衰减 ───── 高频</pre>");
    } else if (row == 6 || row == 7) {
        stageNote += QStringLiteral("<p><b>融合曲线说明：</b>蓝色为真实值，红色为慢漂移但较稳定的传感器 A，紫色为高频响应快但噪声更大的传感器 B，绿色为融合结果。</p>");
    } else if (row >= 1 && row <= 4) {
        stageNote += QStringLiteral("<p><b>力/力矩对比：</b>下方对比图模拟接触突变、短时冲击和随机尖峰，并同时显示滑动平均、EWMA、中值、Hampel 的处理结果。滑动平均/EWMA 更容易展示响应速度与平滑程度的取舍；中值/Hampel 更适合观察尖峰抑制。</p>");
    } else if (row == 8) {
        stageNote += QStringLiteral("<pre>简化频率响应：低频 ▼ 衰减 ── 通带 ── 高频 ▼ 衰减</pre>");
    } else if (row == 9) {
        stageNote += QStringLiteral("<pre>简化频率响应：低频 ── 阻带 ▼ 衰减 ── 高频</pre>");
    }
    explanationBrowser->setHtml(filter.explanationHtml + stageNote);

    {
        const QSignalBlocker blocker(sceneSelector);
        const int sceneIndex = sceneSelector->findText(filter.scene);
        if (sceneIndex >= 0) {
            sceneSelector->setCurrentIndex(sceneIndex);
        }
    }

    updateFilterParameterControls(row);
    configureActiveFilter();
    resetSimulation();
    statusBar()->showMessage(QStringLiteral("当前滤波器：%1").arg(filter.name));
}

void MainWindow::configureActiveFilter()
{
    const int row = filterList->currentRow();
    switch (row) {
    case 0:
        activeFilter = std::make_unique<LowPassFilter>(cutoffFrequencySpinBox->value());
        break;
    case 1:
        activeFilter = std::make_unique<MovingAverageFilter>(windowSizeSpinBox->value());
        break;
    case 2:
        activeFilter = std::make_unique<EwmaFilter>(alphaSpinBox->value());
        break;
    case 3:
        activeFilter = std::make_unique<MedianFilter>(windowSizeSpinBox->value());
        break;
    case 4:
        activeFilter = std::make_unique<HampelFilter>(windowSizeSpinBox->value(), hampelThresholdSpinBox->value());
        break;
    case 5:
        activeFilter = std::make_unique<NotchFilter>(centerFrequencySpinBox->value(), bandwidthSpinBox->value());
        break;
    case 6:
        activeFilter = std::make_unique<ComplementaryFilter>(alphaSpinBox->value());
        break;
    case 7:
        activeFilter = std::make_unique<KalmanFilter1D>(processNoiseSpinBox->value(), measurementNoiseSpinBox->value());
        break;
    case 8:
        activeFilter = std::make_unique<BandPassFilter>(lowCutoffSpinBox->value(), highCutoffSpinBox->value());
        break;
    case 9:
        activeFilter = std::make_unique<BandStopFilter>(lowCutoffSpinBox->value(), highCutoffSpinBox->value());
        break;
    default:
        activeFilter = std::make_unique<EwmaFilter>(alphaSpinBox->value());
        break;
    }

    if (row == 10) {
        parameterSummaryValue->setText(QStringLiteral("后续阶段实现；当前用 %1 预览输出").arg(activeFilter->parameterSummary()));
    } else if (activeFilter) {
        parameterSummaryValue->setText(activeFilter->parameterSummary());
    }
}

void MainWindow::updateFilterParameterControls(int row)
{
    const bool usesCutoff = row == 0;
    const bool usesWindow = row == 1 || row == 3 || row == 4;
    const bool usesAlpha = row == 2 || row == 6 || row == 7 || row == 10;
    const bool usesHampelThreshold = row == 4;
    const bool usesNotch = row == 5;
    const bool usesBandEdges = row == 8 || row == 9;
    const bool usesKalman = row == 7;

    cutoffFrequencyLabel->setVisible(usesCutoff);
    cutoffFrequencySpinBox->setVisible(usesCutoff);
    windowSizeLabel->setVisible(usesWindow);
    windowSizeSpinBox->setVisible(usesWindow);
    alphaLabel->setVisible(usesAlpha);
    alphaSpinBox->setVisible(usesAlpha);
    hampelThresholdLabel->setVisible(usesHampelThreshold);
    hampelThresholdSpinBox->setVisible(usesHampelThreshold);
    centerFrequencyLabel->setVisible(usesNotch);
    centerFrequencySpinBox->setVisible(usesNotch);
    bandwidthLabel->setVisible(usesNotch);
    bandwidthSpinBox->setVisible(usesNotch);
    lowCutoffLabel->setVisible(usesBandEdges);
    lowCutoffSpinBox->setVisible(usesBandEdges);
    highCutoffLabel->setVisible(usesBandEdges);
    highCutoffSpinBox->setVisible(usesBandEdges);
    processNoiseLabel->setVisible(usesKalman);
    processNoiseSpinBox->setVisible(usesKalman);
    measurementNoiseLabel->setVisible(usesKalman);
    measurementNoiseSpinBox->setVisible(usesKalman);
    forceTorqueWidget->setVisible(row >= 1 && row <= 4);
}

void MainWindow::updatePlaybackState()
{
    playAction->setEnabled(!isPlaying);
    pauseAction->setEnabled(isPlaying);
    playbackStateValue->setText(isPlaying ? QStringLiteral("播放中") : QStringLiteral("已暂停"));
    frameCounterValue->setText(QString::number(frameCounter));
}

void MainWindow::updateSignalConfiguration()
{
    const int signalIndex = signalSelector->currentIndex();
    switch (signalIndex) {
    case 0:
        signalGenerator.setSignalMode(SignalGenerator::SignalMode::Sine);
        break;
    case 1:
        signalGenerator.setSignalMode(SignalGenerator::SignalMode::Step);
        break;
    case 2:
        signalGenerator.setSignalMode(SignalGenerator::SignalMode::Ramp);
        break;
    case 3:
        signalGenerator.setSignalMode(SignalGenerator::SignalMode::Pulse);
        break;
    case 4:
        signalGenerator.setSignalMode(SignalGenerator::SignalMode::Mixed);
        break;
    default:
        signalGenerator.setSignalMode(SignalGenerator::SignalMode::Sine);
        break;
    }

    const int noiseIndex = noiseSelector->currentIndex();
    switch (noiseIndex) {
    case 0:
        signalGenerator.setNoiseMode(SignalGenerator::NoiseMode::None);
        break;
    case 1:
        signalGenerator.setNoiseMode(SignalGenerator::NoiseMode::Gaussian);
        break;
    case 2:
        signalGenerator.setNoiseMode(SignalGenerator::NoiseMode::Spike);
        break;
    case 3:
        signalGenerator.setNoiseMode(SignalGenerator::NoiseMode::Periodic);
        break;
    case 4:
        signalGenerator.setNoiseMode(SignalGenerator::NoiseMode::Mixed);
        break;
    default:
        signalGenerator.setNoiseMode(SignalGenerator::NoiseMode::Gaussian);
        break;
    }

    signalModeValue->setText(signalSelector->currentText());
    noiseModeValue->setText(noiseSelector->currentText());
    resetSimulation();
    statusBar()->showMessage(QStringLiteral("信号配置已更新：%1 / %2").arg(signalSelector->currentText(), noiseSelector->currentText()));
}

bool MainWindow::isFusionFilterSelected() const
{
    const int row = filterList ? filterList->currentRow() : -1;
    return row == 6 || row == 7;
}

bool MainWindow::isForceTorqueFilterSelected() const
{
    const int row = filterList ? filterList->currentRow() : -1;
    return row >= 1 && row <= 4;
}

SignalFrame MainWindow::nextFusionFrame()
{
    SignalFrame baseFrame = signalGenerator.nextFrame();
    const double t = baseFrame.timeSeconds;
    const double trueValue = baseFrame.referenceValue;
    const double slowDrift = 0.35 * qSin(0.18 * t) + 0.10 * qSin(0.05 * t);
    const double sensorANoise = (QRandomGenerator::global()->generateDouble() - 0.5) * 0.08;
    const double sensorBNoise = (QRandomGenerator::global()->generateDouble() - 0.5) * 0.55;

    SignalFrame frame;
    frame.timeSeconds = t;
    frame.referenceValue = trueValue;
    frame.noisyValue = trueValue + slowDrift + sensorANoise;
    frame.auxiliaryValue = trueValue + sensorBNoise + 0.10 * qSin(18.0 * t);
    frame.filteredValue = frame.noisyValue;
    frame.hasAuxiliaryValue = true;
    return frame;
}

SignalFrame MainWindow::nextForceTorqueFrame(double timeSeconds)
{
    const double phase = std::fmod(timeSeconds, 10.0);
    const double contact = phase < 2.0 ? 0.0 : (phase < 2.45 ? (phase - 2.0) / 0.45 : 1.0);
    const double release = phase > 8.0 ? qMax(0.0, 1.0 - (phase - 8.0) / 0.8) : 1.0;
    const double trueForce = contact * release * 1.4 + 0.08 * qSin(2.0 * timeSeconds);
    const double impulse = std::fmod(timeSeconds, 5.0) < 0.08 ? 1.15 : 0.0;
    const double randomSpike = QRandomGenerator::global()->generateDouble() < 0.018
        ? (QRandomGenerator::global()->generateDouble() < 0.5 ? -1.0 : 1.0) * (0.7 + QRandomGenerator::global()->generateDouble() * 0.7)
        : 0.0;
    const double noise = (QRandomGenerator::global()->generateDouble() - 0.5) * 0.16;

    SignalFrame frame;
    frame.timeSeconds = timeSeconds;
    frame.referenceValue = trueForce;
    frame.noisyValue = trueForce + impulse + randomSpike + noise;
    frame.filteredValue = frame.noisyValue;
    return frame;
}

void MainWindow::resetSimulation()
{
    frameCounter = 0;
    signalGenerator.reset();
    if (activeFilter) {
        activeFilter->reset();
    }
    signalChart->clear();
    robotArmWidget->clearTrajectories();
    forceTorqueWidget->clear();
    updatePlaybackState();
    statusBar()->showMessage(QStringLiteral("演示状态已重置"));
}

void MainWindow::advanceFrame()
{
    ++frameCounter;
    SignalFrame frame = isFusionFilterSelected() ? nextFusionFrame() : signalGenerator.nextFrame();
    if (activeFilter) {
        frame.filteredValue = activeFilter->processFrame(frame, 1.0 / signalGenerator.sampleRate());
    }
    signalChart->appendFrame(frame);
    robotArmWidget->updatePose(frame.noisyValue, frame.filteredValue);
    if (isForceTorqueFilterSelected()) {
        const SignalFrame forceFrame = nextForceTorqueFrame(frame.timeSeconds);
        forceTorqueWidget->appendSample(forceFrame.timeSeconds, forceFrame.referenceValue, forceFrame.noisyValue, 1.0 / signalGenerator.sampleRate());
    }
    frameCounterValue->setText(QString::number(frameCounter));
}
