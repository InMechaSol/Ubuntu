#ifndef QCCOS_H
#define QCCOS_H

#include <cmath>
#include <climits>
#include "motionControl.h"
#include <QObject>
#include <QList>
#include <QPointF>
#include <QChartView>
#include <QLineSeries>
#include <QTreeWidget>

struct maxMinstruct
{
  float minX = std::numeric_limits<float>::max();
  float maxX = -std::numeric_limits<float>::max();
  float minY = std::numeric_limits<float>::max();
  float maxY = -std::numeric_limits<float>::max();
};
#define LINESERIESSAMPLES 1024

class qSPDChart;

class SPDLineSeries: public QtCharts::QLineSeries
{
private:
    SPDClass* SPDptr;
    qSPDChart* spdChart = nullptr;
    QList<QPointF> tempData;
    struct maxMinstruct limits;
    struct maxMinstruct cummulativelimits;
    //int* plotWindwStart;
    //int* plotWindwSamples;
    int lastTempDataCount = 0;
public:
    SPDLineSeries(SPDClass* SPDptrIn, qSPDChart* spdChartPtr);//, int* plotWindowStartIn, int* plotWindowSamplesIn);
    SPDLineSeries* getOnChart(int VarSelectionIn,  qSPDChart* spdChartPtr);
//    bool isOnChart(qSPDChart* spdChartPtr);
    void UpdateSeries(int plotWindowStartIn, int plotWindowSamplesIn);
    void LatchTempData(float xValue);
    bool hasNewData();
    void clearTempData();
    qSPDChart* getSPDChartPtr();
    void clearSeriesData();
    int getNumSamples();
    struct maxMinstruct* getLimits();
    struct maxMinstruct* getOverAllLimits();
    SPDClass* getSPDptr();
    void resetHasNewData();
};

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

enum guiSPDSelector
{
    guiNone = 0,
    guiPltWndwStart,
    guiPltWndwSamples,
    guiPltDuration,
    guiPltBegin,
    guiPltWindowRt,
    guiChrt0MaxX,
    guiChrt0MinX,
    guiChrt0MaxY,
    guiChrt0MinY,
    guiChrt0Shift,
    guiChrt0MaxDur,
    guiChrt0DurPerSamp,
    guiEND
};
class qSPDChart: public QtCharts::QChart
{
private:
    QList<SPDLineSeries*> SPDLines;    

    int plotShiftSamples = 0;
    float maxPossibleDuration = 0;
    float durationPerSample = 0;

    struct SPDStruct GUISPDStructArray[guiEND] ={
    {guiNone,               "",                     "",                 1,NULL_TYPE,    0,                                  nullptr},
    {guiPltWndwStart,       "Window Start",         "(index)",          1,SIGNED_TYPE,  0,                                  nullptr},
    {guiPltWndwSamples,     "Window Samples",       "(indecies)",       1,SIGNED_TYPE,  0,                                  nullptr},
    {guiPltDuration,        "Plot Duration",        "(time)",           0,FLOAT_TYPE,   0,                                  nullptr},
    {guiPltBegin,           "Plot Begin",           "(time)",           0,FLOAT_TYPE,   0,                                  nullptr},
    {guiPltWindowRt,        "Plot Window R/T",      "(yes/no)",         0,FLOAT_TYPE,   0,                                  nullptr},
    {guiChrt0MaxX,          "Max X",                "(time)",           1,FLOAT_TYPE,   0,                                  nullptr},
    {guiChrt0MinX,          "Min X",                "(time)",           1,FLOAT_TYPE,   0,                                  nullptr},
    {guiChrt0MaxY,          "Max Y",                "",                 1,FLOAT_TYPE,   0,                                  nullptr},
    {guiChrt0MinY,          "Min Y",                "",                 1,FLOAT_TYPE,   0,                                  nullptr},
    {guiChrt0Shift,         "Shift Samples",        "(indecies)",       1,SIGNED_TYPE,  sizeof(plotShiftSamples),           &plotShiftSamples},
    {guiChrt0MaxDur,        "Max Duration",         "(time)",           1,FLOAT_TYPE,   sizeof(maxPossibleDuration),        &maxPossibleDuration},
    {guiChrt0DurPerSamp,    "Duration/Sample",      "(time/indecies)",  1,FLOAT_TYPE,   sizeof(durationPerSample),          &durationPerSample}
    };
public:
    qSPDChart(QGraphicsItem *parent = nullptr, Qt::WindowFlags wFlags = Qt::WindowFlags());
    void addSeries(SPDLineSeries* SPDLinePtr);
    void removeSeries(SPDLineSeries* SPDLinePtr);
    SPDLineSeries* getFromChart(int index);
    void scalePlot();
    void reset();
    void UpdateChartSeries(float plotDurationIn, float plotBeginIn, UI_8 plotWindowRTIn);
    struct SPDStruct* getSPDArray();
    int plotWindowStart=0, plotWindowSamples=0;
//    float plotDuration = 0.001;
//    float plotBegin = 0;
//    UI_8 plotWindowRT = ui8TRUE;
};
class GUISPD: public SPDClass
{
private:
    qSPDChart* SPDChartPtr = nullptr;
    enum guiSPDSelector GUIVarSelection = guiNone;

public:
    GUISPD(enum guiSPDSelector GUIVarSelectionIn, qSPDChart* SPDChartPtrIn);
    enum guiSPDSelector getSPDSelector();
    qSPDChart* getSPDChartPtr();
    float getFloatVal();
    const char* getLabelString();
    const char* getUnitsString();
};



// base SPD treewidgetitem
class SPDTreeWidgetItem:public QTreeWidgetItem
{
private:
    SPDClass* SPDptr;
public:
    SPDTreeWidgetItem(SPDClass* spdPtrIn, QStringList columnsText);
    int GetVarSelectionIn();
    void* GetDataPtrIn();
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

// inherit from base class
class GUISPDTreeWidgetItem:public SPDTreeWidgetItem
{
private:
    GUISPD spd;
public:
    GUISPDTreeWidgetItem(enum guiSPDSelector GUIVarSelectionIn, qSPDChart* SPDChartPtrIn);
    enum guiSPDSelector GetAxisVarSelectionIn();
};

// ------------------------------------
class QccOS : public QObject
{
    Q_OBJECT
public:
    explicit QccOS(QObject *parent = nullptr);

signals:

};

#endif // QCCOS_H
