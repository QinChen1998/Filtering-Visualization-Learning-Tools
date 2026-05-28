#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "SignalGenerator.h"

#include <QMainWindow>
#include <QString>
#include <QStringList>
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class QAction;
class QComboBox;
class QLabel;
class QListWidget;
class SignalChartWidget;
class QTextBrowser;
class QTimer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    struct FilterInfo {
        QString name;
        QString scene;
        QString sampleRate;
        QStringList parameters;
        QString explanationHtml;
    };

    void setupFilterCatalog();
    void setupToolbar();
    void setupCentralLayout();
    void connectInteractions();
    void updateSelectedFilter(int row);
    void updatePlaybackState();
    void updateSignalConfiguration();
    void resetSimulation();
    void advanceFrame();

    Ui::MainWindow *ui;
    QVector<FilterInfo> filterCatalog;
    QComboBox *sceneSelector;
    QComboBox *signalSelector;
    QComboBox *noiseSelector;
    QAction *playAction;
    QAction *pauseAction;
    QAction *resetAction;
    QListWidget *filterList;
    QLabel *visualTitle;
    QLabel *visualHint;
    QLabel *filterNameValue;
    QLabel *sceneValue;
    QLabel *sampleRateValue;
    QLabel *parameterSummaryValue;
    QLabel *signalModeValue;
    QLabel *noiseModeValue;
    QLabel *playbackStateValue;
    QLabel *frameCounterValue;
    SignalChartWidget *signalChart;
    QTextBrowser *explanationBrowser;
    SignalGenerator signalGenerator;
    QTimer *refreshTimer;
    bool isPlaying;
    int frameCounter;
};
#endif // MAINWINDOW_H
