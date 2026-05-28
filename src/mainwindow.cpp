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
#include <QSplitter>
#include <QStatusBar>
#include <QStyle>
#include <QTextBrowser>
#include <QToolBar>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupToolbar();
    setupCentralLayout();

    setWindowTitle(QStringLiteral("机械臂滤波可视化教学工具"));
    resize(1280, 800);
    setMinimumSize(1024, 640);
    statusBar()->showMessage(QStringLiteral("阶段 0：工程基础与界面骨架已就绪"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupToolbar()
{
    auto *toolBar = addToolBar(QStringLiteral("主工具栏"));
    toolBar->setMovable(false);

    auto *sceneSelector = new QComboBox(toolBar);
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

    toolBar->addAction(style()->standardIcon(QStyle::SP_MediaPlay), QStringLiteral("播放"));
    toolBar->addAction(style()->standardIcon(QStyle::SP_MediaPause), QStringLiteral("暂停"));
    toolBar->addAction(style()->standardIcon(QStyle::SP_BrowserReload), QStringLiteral("重置"));
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

    auto *filterList = new QListWidget(workSplitter);
    filterList->addItems({
        QStringLiteral("一阶低通滤波器"),
        QStringLiteral("滑动平均滤波器"),
        QStringLiteral("EWMA 滤波器"),
        QStringLiteral("中值滤波器"),
        QStringLiteral("Hampel 异常值滤波器"),
        QStringLiteral("陷波滤波器"),
        QStringLiteral("互补滤波器"),
        QStringLiteral("一维卡尔曼滤波器"),
        QStringLiteral("带通滤波器"),
        QStringLiteral("图像滤波"),
    });
    filterList->setCurrentRow(0);
    filterList->setMinimumWidth(180);

    auto *visualFrame = new QFrame(workSplitter);
    visualFrame->setFrameShape(QFrame::StyledPanel);
    visualFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *visualLayout = new QVBoxLayout(visualFrame);
    auto *visualTitle = new QLabel(QStringLiteral("可视化区域"), visualFrame);
    auto *visualHint = new QLabel(QStringLiteral("曲线图、机械臂动画和图像滤波结果将在这里展示。"), visualFrame);
    visualHint->setAlignment(Qt::AlignCenter);
    visualHint->setWordWrap(true);
    visualLayout->addWidget(visualTitle);
    visualLayout->addWidget(visualHint, 1);

    auto *parameterFrame = new QFrame(workSplitter);
    parameterFrame->setFrameShape(QFrame::StyledPanel);
    parameterFrame->setMinimumWidth(240);
    auto *parameterLayout = new QFormLayout(parameterFrame);
    parameterLayout->addRow(QStringLiteral("滤波器"), new QLabel(QStringLiteral("一阶低通"), parameterFrame));
    parameterLayout->addRow(QStringLiteral("采样频率"), new QLabel(QStringLiteral("100 Hz"), parameterFrame));
    parameterLayout->addRow(QStringLiteral("参数区"), new QLabel(QStringLiteral("后续接入实时控件"), parameterFrame));

    workSplitter->setStretchFactor(0, 0);
    workSplitter->setStretchFactor(1, 1);
    workSplitter->setStretchFactor(2, 0);
    workSplitter->setSizes({220, 760, 300});

    auto *explanation = new QTextBrowser(mainSplitter);
    explanation->setMinimumHeight(160);
    explanation->setHtml(QStringLiteral(
        "<h3>一阶低通滤波器</h3>"
        "<p>这里将显示滤波器原理、应用场景、参数解释、优缺点和调参建议。</p>"
        "<p>当前版本先完成工程结构和界面骨架，后续逐步接入信号生成、曲线显示与滤波算法。</p>"));

    mainSplitter->addWidget(workSplitter);
    mainSplitter->addWidget(explanation);
    mainSplitter->setStretchFactor(0, 1);
    mainSplitter->setStretchFactor(1, 0);
    mainSplitter->setSizes({580, 180});

    rootLayout->addWidget(mainSplitter);
    setCentralWidget(central);
}
