#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QTime>
#include <QTimer>

#include "qccos.h"
#include "IMIGripper.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class SPDLineSeriesMap
{
private:
    GripperSPD spd;
    QtCharts::QLineSeries spdLine;
    QtCharts::QChart* spdChart = nullptr;
    QList<QPointF> tempXY;
    unsigned int minTime = std::numeric_limits<unsigned int>::max();
    unsigned int maxTime = 0;
    float minY = std::numeric_limits<float>::max();
    float maxY = -std::numeric_limits<float>::max();


public:
    SPDLineSeriesMap(enum SPDSelector GripperVarSelectionIn, IMIGripper* GripperAPIPtrIn, QtCharts::QChart* spdChartPtr);
    QtCharts::QLineSeries* isOnChart(enum SPDSelector GripperVarSelectionIn,  QtCharts::QChart* spdChartPtr);
    bool isOnChart(QtCharts::QChart* spdChartPtr);
    void UpdateSeries();
    void LatchTempData();
    bool hasTempData();
    void clearTempData();
    unsigned int getMaxTime(){return maxTime;}
    unsigned int getMinTime(){return minTime;}
    float getMaxY(){return maxY;}
    float getMinY(){return minY;}
    QtCharts::QChart* getSPDChartPtr(){return spdChart;}
    void clearSeriesData();
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    IMIGripper GripperAPI;
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void Stop();
    void Open();
    void Close();

    void LaunchNewWindow();
    void ToggleSPDSetting();
    void LatchAndUpdate();
    void RunPausePlots();
    void ClearPlots();
    void ShowTreeView();

private:
    bool runPlots = true;
    struct maxMinstruct
    {
      float minX, maxX, minY, maxY;
    };
    void scalePlots(QtCharts::QChart* chartPtr);
    void setNewMainChart();
    Ui::MainWindow *ui = nullptr;
    QTreeWidget *treeWidget = nullptr;
    QtCharts::QChartView* mainChart = nullptr;
    QList<QtCharts::QChartView*> *ChildWindows = nullptr;
    QList<SPDLineSeriesMap*> *AllSPDLineSeries = nullptr;

    QMenu *SPDSelectionMenu = nullptr;
    QList<QAction*> *SPDSelectionMenuActions = nullptr;

    QAction *stopAction = nullptr;
    QAction *openAction = nullptr;
    QAction *closeAction = nullptr;
    QAction *duplicatChartAction = nullptr;
    QAction *pauseRunChartsAction = nullptr;
    QAction *clearChartsAction = nullptr;
    QAction *showTreeViewAction = nullptr;

    QLabel *ConnectedLabel = nullptr;
    QLabel *PositionsLabel = nullptr;
    QLabel *MotorsStatusLabel = nullptr;

    QTimer *LatchTimer = nullptr;
};
#endif // MAINWINDOW_H
