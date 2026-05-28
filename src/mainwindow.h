#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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
    void resetSimulation();
    void advanceFrame();

    Ui::MainWindow *ui;
    QVector<FilterInfo> filterCatalog;
    QComboBox *sceneSelector;
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
    QLabel *playbackStateValue;
    QLabel *frameCounterValue;
    QTextBrowser *explanationBrowser;
    QTimer *refreshTimer;
    bool isPlaying;
    int frameCounter;
};
#endif // MAINWINDOW_H
