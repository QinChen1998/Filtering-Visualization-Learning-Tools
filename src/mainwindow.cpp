#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QAction>
#include <QComboBox>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QSizePolicy>
#include <QSignalBlocker>
#include <QSplitter>
#include <QStatusBar>
#include <QStyle>
#include <QTextBrowser>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , sceneSelector(nullptr)
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
    , playbackStateValue(nullptr)
    , frameCounterValue(nullptr)
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
    updatePlaybackState();

    setWindowTitle(QStringLiteral("机械臂滤波可视化教学工具"));
    resize(1280, 800);
    setMinimumSize(1024, 640);
    statusBar()->showMessage(QStringLiteral("阶段 1：主界面框架已就绪"));
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
                           "<p><b>参数影响：</b>截止频率越低越平滑，但响应越慢；截止频率越高越灵敏，但噪声保留更多。</p>")
        },
        {
            QStringLiteral("滑动平均滤波器"),
            QStringLiteral("控制指令平滑"),
            QStringLiteral("100 Hz"),
            {QStringLiteral("窗口大小"), QStringLiteral("边界策略")},
            QStringLiteral("<h3>滑动平均滤波器</h3>"
                           "<p><b>解决的问题：</b>用固定长度窗口平均近期样本，降低随机噪声。</p>"
                           "<p><b>机械臂场景：</b>平滑速度指令、力传感器低幅随机噪声。</p>"
                           "<p><b>参数影响：</b>窗口越大越平滑，同时引入更明显的延迟。</p>")
        },
        {
            QStringLiteral("EWMA 滤波器"),
            QStringLiteral("控制指令平滑"),
            QStringLiteral("100 Hz"),
            {QStringLiteral("alpha"), QStringLiteral("初始输出")},
            QStringLiteral("<h3>指数加权移动平均 EWMA</h3>"
                           "<p><b>解决的问题：</b>用指数权重保留历史趋势，计算量小，适合实时控制。</p>"
                           "<p><b>机械臂场景：</b>对控制指令做轻量平滑，避免突然抖动。</p>"
                           "<p><b>参数影响：</b>alpha 越大越跟手，alpha 越小越平滑。</p>")
        },
        {
            QStringLiteral("中值滤波器"),
            QStringLiteral("力/力矩传感器滤波"),
            QStringLiteral("100 Hz"),
            {QStringLiteral("窗口大小"), QStringLiteral("奇数窗口保护")},
            QStringLiteral("<h3>中值滤波器</h3>"
                           "<p><b>解决的问题：</b>抑制孤立尖峰，比均值类滤波更不容易被异常点拖偏。</p>"
                           "<p><b>机械臂场景：</b>力/力矩传感器偶发冲击、视觉测量离群点。</p>"
                           "<p><b>参数影响：</b>窗口越大，尖峰抑制更强，但细节和响应速度会下降。</p>")
        },
        {
            QStringLiteral("Hampel 异常值滤波器"),
            QStringLiteral("力/力矩传感器滤波"),
            QStringLiteral("100 Hz"),
            {QStringLiteral("窗口大小"), QStringLiteral("阈值倍数"), QStringLiteral("替换策略")},
            QStringLiteral("<h3>Hampel 异常值滤波器</h3>"
                           "<p><b>解决的问题：</b>基于中位数和绝对偏差识别异常值，再用稳健估计替换。</p>"
                           "<p><b>机械臂场景：</b>接触检测中的短时尖峰、传感器偶发毛刺。</p>"
                           "<p><b>参数影响：</b>阈值越小越敏感，但可能误判真实快速变化。</p>")
        },
        {
            QStringLiteral("陷波滤波器"),
            QStringLiteral("关节状态滤波"),
            QStringLiteral("100 Hz"),
            {QStringLiteral("中心频率"), QStringLiteral("带宽"), QStringLiteral("采样频率")},
            QStringLiteral("<h3>陷波滤波器</h3>"
                           "<p><b>解决的问题：</b>压制指定频率附近的周期干扰。</p>"
                           "<p><b>机械臂场景：</b>电机振动、结构共振或电源纹波造成的周期噪声。</p>"
                           "<p><b>参数影响：</b>带宽越宽抑制范围越大，但可能伤及有用信号。</p>")
        },
        {
            QStringLiteral("互补滤波器"),
            QStringLiteral("传感器融合"),
            QStringLiteral("200 Hz"),
            {QStringLiteral("互补系数"), QStringLiteral("传感器 A 权重"), QStringLiteral("传感器 B 权重")},
            QStringLiteral("<h3>互补滤波器</h3>"
                           "<p><b>解决的问题：</b>把低频稳定传感器和高频响应传感器结合起来。</p>"
                           "<p><b>机械臂场景：</b>IMU 与编码器姿态估计融合。</p>"
                           "<p><b>参数影响：</b>系数决定更信任哪一路传感器。</p>")
        },
        {
            QStringLiteral("一维卡尔曼滤波器"),
            QStringLiteral("传感器融合"),
            QStringLiteral("200 Hz"),
            {QStringLiteral("过程噪声 Q"), QStringLiteral("测量噪声 R"), QStringLiteral("初始协方差")},
            QStringLiteral("<h3>一维卡尔曼滤波器</h3>"
                           "<p><b>解决的问题：</b>在模型预测和带噪测量之间动态分配信任。</p>"
                           "<p><b>机械臂场景：</b>位置、速度或视觉目标的一维估计。</p>"
                           "<p><b>参数影响：</b>Q 越大越信任新变化，R 越大越不信任测量。</p>")
        },
        {
            QStringLiteral("带通滤波器"),
            QStringLiteral("关节状态滤波"),
            QStringLiteral("500 Hz"),
            {QStringLiteral("低截止频率"), QStringLiteral("高截止频率"), QStringLiteral("采样频率")},
            QStringLiteral("<h3>带通滤波器</h3>"
                           "<p><b>解决的问题：</b>只保留指定频段内的信号成分。</p>"
                           "<p><b>机械臂场景：</b>观察振动频段、诊断周期性机械扰动。</p>"
                           "<p><b>参数影响：</b>频带过窄可能漏掉目标动态，过宽则噪声保留更多。</p>")
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
    visualLayout->addWidget(visualTitle);
    visualLayout->addWidget(visualHint, 1);

    auto *parameterFrame = new QFrame(workSplitter);
    parameterFrame->setFrameShape(QFrame::StyledPanel);
    parameterFrame->setMinimumWidth(240);
    auto *parameterLayout = new QFormLayout(parameterFrame);
    filterNameValue = new QLabel(parameterFrame);
    sceneValue = new QLabel(parameterFrame);
    sampleRateValue = new QLabel(parameterFrame);
    parameterSummaryValue = new QLabel(parameterFrame);
    playbackStateValue = new QLabel(parameterFrame);
    frameCounterValue = new QLabel(parameterFrame);
    parameterSummaryValue->setWordWrap(true);
    parameterLayout->addRow(QStringLiteral("滤波器"), filterNameValue);
    parameterLayout->addRow(QStringLiteral("场景"), sceneValue);
    parameterLayout->addRow(QStringLiteral("采样频率"), sampleRateValue);
    parameterLayout->addRow(QStringLiteral("参数区"), parameterSummaryValue);
    parameterLayout->addRow(QStringLiteral("播放状态"), playbackStateValue);
    parameterLayout->addRow(QStringLiteral("刷新帧"), frameCounterValue);

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
    parameterSummaryValue->setText(filter.parameters.join(QStringLiteral(" / ")));
    visualTitle->setText(QStringLiteral("可视化区域 - %1").arg(filter.name));
    visualHint->setText(QStringLiteral("%1 场景的实时曲线、动画或图像结果将在这里接入。").arg(filter.scene));
    explanationBrowser->setHtml(filter.explanationHtml
                                + QStringLiteral("<p><b>当前阶段：</b>界面联动已完成，后续阶段将接入真实信号和滤波输出。</p>"));

    {
        const QSignalBlocker blocker(sceneSelector);
        const int sceneIndex = sceneSelector->findText(filter.scene);
        if (sceneIndex >= 0) {
            sceneSelector->setCurrentIndex(sceneIndex);
        }
    }

    statusBar()->showMessage(QStringLiteral("当前滤波器：%1").arg(filter.name));
}

void MainWindow::updatePlaybackState()
{
    playAction->setEnabled(!isPlaying);
    pauseAction->setEnabled(isPlaying);
    playbackStateValue->setText(isPlaying ? QStringLiteral("播放中") : QStringLiteral("已暂停"));
    frameCounterValue->setText(QString::number(frameCounter));
}

void MainWindow::resetSimulation()
{
    frameCounter = 0;
    updatePlaybackState();
    statusBar()->showMessage(QStringLiteral("演示状态已重置"));
}

void MainWindow::advanceFrame()
{
    ++frameCounter;
    frameCounterValue->setText(QString::number(frameCounter));
}
