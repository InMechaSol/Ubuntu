#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QList>
#include <QChartView>
#include <QLineSeries>
#include <QTimer>
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

public:
    SPDLineSeriesMap(enum SPDSelector GripperVarSelectionIn, IMIGripper* GripperAPIPtrIn, QtCharts::QChart* spdChartPtr);
    QtCharts::QLineSeries* isOnChart(enum SPDSelector GripperVarSelectionIn,  QtCharts::QChart* spdChartPtr);
    void UpdateSeries();
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

private:    
    Ui::MainWindow *ui = nullptr;
    QtCharts::QChartView* mainChart = nullptr;

    QList<QtCharts::QChartView*> *ChildWindows = nullptr;
    QList<SPDLineSeriesMap*> *AllSPDLineSeries = nullptr;
    QMenu *SPDSelectionMenu = nullptr;
    QList<QAction*> *SPDSelectionMenuActions = nullptr;

    QAction *stopAction = nullptr;
    QAction *openAction = nullptr;
    QAction *closeAction = nullptr;
    QAction *duplicatChartAction = nullptr;

    QLabel *ConnectedLabel = nullptr;
    QLabel *PositionsLabel = nullptr;
    QLabel *MotorsStatusLabel = nullptr;

    QTimer *LatchTimer = nullptr;
};
#endif // MAINWINDOW_H
