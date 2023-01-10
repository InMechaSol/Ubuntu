#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "motionControl.h"
#include "qccos.h"

// Motion Control, Smart Motor, Axis - Line Series Mapping via SPD
// Makes each Axis SPD plotable
class AxisLineSeriesMap
{
private:
    AxisSPD spd;
    SPDLineSeries spdLine;

public:
    AxisLineSeriesMap(enum mcsSPDSelector AxisVarSelectionIn, SmartMotorDevice* smDevPtrIn, qSPDChart* spdChartPtr);//, int* plotWindowStartIn, int* plotWindowSamplesIn);
    SPDLineSeries* getLine();
};

// inherit from base class
class AxisSPDTreeWidgetItem:public SPDTreeWidgetItem
{
private:
    AxisSPD spd;
public:
    AxisSPDTreeWidgetItem(enum mcsSPDSelector AxisVarSelectionIn, SmartMotorDevice* smDevPtrIn);
    enum mcsSPDSelector GetAxisVarSelectionIn();
};



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE





class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    SmartMotorDevice smMotionTests;
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void Jog();
    void Reset();

    void Open();
    void Close();

    void LaunchNewWindow();
    void ToggleSPDSetting();
    void LatchAndUpdate();
    void RunPausePlots();

    void ShowTreeView();
    void spdTableChange(QTreeWidgetItem*,int);
    void spdTableEditItem(QTreeWidgetItem*,int);

private:
    const int latchNUpdateMS = 10;
    bool jogonecycle = false;
    UI_8 runPlots = ui8TRUE;

    float plotDuration = 0.001;
    float plotBegin = 0;
    UI_8 plotWindowRT = ui8TRUE;

    void setNewMainChart();
    void UpdateTreeView();

    Ui::MainWindow *ui = nullptr;
    QTreeWidget *treeWidget = nullptr;
    QtCharts::QChartView* mainChartView = nullptr;
    QList<QtCharts::QChartView*> *childChartViews = nullptr;
    QList<AxisLineSeriesMap*> *AllAxisLineSeries = nullptr;

    unsigned int cycleCounter = 0;
    unsigned int systemguitime = QTime::currentTime().msecsSinceStartOfDay();
    unsigned int systemmodeltime = QTime::currentTime().msecsSinceStartOfDay();
    float systemModelTimeDelta = 0;
    float systemGUITimeDelta = 0;
    float start, stop, deltaTreeView, deltaPlots, deltaModel, deltaLatchNUpdate;

    QMenu *SPDSelectionMenu = nullptr;
    QList<QAction*> *SPDSelectionMenuActions = nullptr;

    QAction *jogAction = nullptr;
    QAction *resetAction = nullptr;


    QAction *openAction = nullptr;
    QAction *closeAction = nullptr;
    QAction *duplicatChartAction = nullptr;
    QAction *pauseRunChartsAction = nullptr;

    QAction *showTreeViewAction = nullptr;

    QLabel *ConnectedLabel = nullptr;
    QLabel *PositionsLabel = nullptr;
    QLabel *MotorsStatusLabel = nullptr;

    QTimer *LatchTimer = nullptr;

};








#endif // MAINWINDOW_H
