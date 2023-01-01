#include "qccos.h"
#include <math.h>
#include <QString>

// from motion control library
float sqrtFloat(float inFloat)
{
    return std::sqrt(inFloat);
}


/////////////////////////////////////////////////////
/// General SPD LineSeries
//
SPDLineSeries::SPDLineSeries(SPDClass* SPDptrIn, qSPDChart* spdChartPtr):QtCharts::QLineSeries(spdChartPtr),
    tempData()
{
    SPDptr = SPDptrIn;
    spdChart = spdChartPtr;
}
SPDLineSeries* SPDLineSeries::getOnChart(int VarSelectionIn,  qSPDChart* spdChartPtr)
{
    if( (SPDptr->getVarIndex()==VarSelectionIn && spdChart==spdChartPtr))
        return this;
    else
        return nullptr;
}
int SPDLineSeries::getNumSamples()
{
    return tempData.count();
}
//bool SPDLineSeries::isOnChart(qSPDChart* spdChartPtr)
//{
//    return (spdChart==spdChartPtr);
//}
void SPDLineSeries::clearTempData()
{
    limits.minX = std::numeric_limits<float>::max();
    limits.maxX = -std::numeric_limits<float>::max();
    limits.minY = std::numeric_limits<float>::max();
    limits.maxY = -std::numeric_limits<float>::max();
    cummulativelimits.minX = std::numeric_limits<float>::max();
    cummulativelimits.maxX = -std::numeric_limits<float>::max();
    cummulativelimits.minY = std::numeric_limits<float>::max();
    cummulativelimits.maxY = -std::numeric_limits<float>::max();
    tempData.clear();
    lastTempDataCount = tempData.count();
}
void SPDLineSeries::clearSeriesData()
{
    clearTempData();
    this->clear();
}
void SPDLineSeries::UpdateSeries(int plotWindowStartIn, int plotWindowSamplesIn)
{
    // based on window and data samples
    int windowStart = std::min(tempData.count()-1,plotWindowStartIn);
    int windowSamples = std::min(tempData.count()-windowStart, plotWindowSamplesIn);
    int i, j, h, k;

    limits.minX = std::numeric_limits<float>::max();
    limits.maxX = -std::numeric_limits<float>::max();
    limits.minY = std::numeric_limits<float>::max();
    limits.maxY = -std::numeric_limits<float>::max();

    // select and display up to LINESERIESSAMPLES points from temp data
    if(windowStart>=0 && windowSamples>1)
    {
        // number of samples to display
        if(windowSamples<LINESERIESSAMPLES)
            j = windowSamples;
        else
            j = LINESERIESSAMPLES;

        h = windowSamples%LINESERIESSAMPLES;

        for(i=0; i<j; i++)
        {
            // step size?
            if(windowSamples<LINESERIESSAMPLES)
                k = windowStart+i;
            else if(h==0)
                k = windowStart+i*windowSamples/LINESERIESSAMPLES;
            else
                k = windowStart+std::min((int)(i*(windowSamples*(1.0/LINESERIESSAMPLES))), windowSamples-1);

            // capture range of series to be transfered
            if(limits.maxX<tempData.at(k).x())
                limits.maxX=tempData.at(k).x();
            if(limits.minX>tempData.at(k).x())
                limits.minX=tempData.at(k).x();
            if(limits.maxY<tempData.at(k).y())
                limits.maxY=tempData.at(k).y();
            if(limits.minY>tempData.at(k).y())
                limits.minY=tempData.at(k).y();

            // transfer points to lineseries
            if(this->count()<LINESERIESSAMPLES)
                this->append(tempData.at(k));
            else
                this->replace(i, tempData.at(k));
        }
    }
}
void SPDLineSeries::LatchTempData(float xValue)
{    
    tempData.append( QPointF( xValue, getSPDFloatValue(SPDptr->getVarIndex(), SPDptr->getSPDArray())));
    if(cummulativelimits.maxX<tempData.at(tempData.count()-1).x())
        cummulativelimits.maxX=tempData.at(tempData.count()-1).x();
    if(cummulativelimits.minX>tempData.at(tempData.count()-1).x())
        cummulativelimits.minX=tempData.at(tempData.count()-1).x();
    if(cummulativelimits.maxY<tempData.at(tempData.count()-1).y())
        cummulativelimits.maxY=tempData.at(tempData.count()-1).y();
    if(cummulativelimits.minY>tempData.at(tempData.count()-1).y())
        cummulativelimits.minY=tempData.at(tempData.count()-1).y();
}
bool SPDLineSeries::hasNewData()
{
    return (tempData.count()>lastTempDataCount);
}
void SPDLineSeries::resetHasNewData()
{
    lastTempDataCount = tempData.count();
}
SPDClass* SPDLineSeries::getSPDptr()
{
    return SPDptr;
}
struct maxMinstruct* SPDLineSeries::getLimits()
{
    return &limits;
}
struct maxMinstruct* SPDLineSeries::getOverAllLimits()
{
    return &cummulativelimits;
}
/////////////////////////////////////////////////////
/// General SPD Chart
//
qSPDChart::qSPDChart(QGraphicsItem *parent, Qt::WindowFlags wFlags):QtCharts::QChart(parent,wFlags),SPDLines()
{

}
void qSPDChart::addSeries(SPDLineSeries* SPDLinePtr)
{
    // append to SPD Lines List and QChart,
    //  iff not already on the chart
    if(nullptr==getFromChart(SPDLinePtr->getSPDptr()->getVarIndex()))
    {
        SPDLines.append(SPDLinePtr);
        QtCharts::QChart::addSeries(SPDLinePtr);
    }
}
void qSPDChart::removeSeries(SPDLineSeries* SPDLinePtr)
{
    if(SPDLinePtr==getFromChart(SPDLinePtr->getSPDptr()->getVarIndex()))
    {
        SPDLines.removeOne(SPDLinePtr);
        QtCharts::QChart::removeSeries(SPDLinePtr);
    }
}
void qSPDChart::reset()
{
    for(int j = 0; j < SPDLines.count(); j++)
    {
        SPDLines.at(j)->clearSeriesData();
    }
}
void qSPDChart::scalePlot()
{
    struct maxMinstruct limits;

    for(int j = 0; j < SPDLines.count(); j++)
    {
        if(SPDLines.at(j)->getLimits()->maxX>limits.maxX)
            limits.maxX = SPDLines.at(j)->getLimits()->maxX;
        if(SPDLines.at(j)->getLimits()->minX<limits.minX)
            limits.minX = SPDLines.at(j)->getLimits()->minX;

        if(SPDLines.at(j)->getLimits()->maxY>limits.maxY)
            limits.maxY = SPDLines.at(j)->getLimits()->maxY;
        if(SPDLines.at(j)->getLimits()->minY<limits.minY)
            limits.minY = SPDLines.at(j)->getLimits()->minY;
    }


    // apply scaling to plot axes
    QList<QtCharts::QAbstractAxis*> chartAxes = axes();
    chartAxes.at(0)->setRange(limits.minX, limits.maxX);
    chartAxes.at(1)->setRange(limits.minY, limits.maxY);
}
SPDLineSeries* qSPDChart::getFromChart(int index)
{
    for(int i = 0; i<SPDLines.count(); i++)
    {
        if(SPDLines.at(i)->getSPDptr()->getVarIndex()==index)
            return SPDLines.at(i);
    }
    return nullptr;
}
struct SPDStruct* qSPDChart::getSPDArray(){return GUISPDStructArray;}
void qSPDChart::UpdateChartSeries(float plotDurationIn, float plotBeginIn, UI_8 plotWindowRTIn)
{
//    plotDuration = plotDurationIn;
//    plotBegin = plotBeginIn;
//    plotWindowRT = plotWindowRTIn;
    bool newData = false;
    for(int j = 0; j < SPDLines.count(); j++)
    {
        if(SPDLines.at(j)->hasNewData())
        {
            if(j==0)
            {
                // use plotDuration and plotBegin to calculate
                // plotWindowStart and plotWindowSamples
                plotShiftSamples = 0;
                maxPossibleDuration = 0;
                durationPerSample = 0;

                if( SPDLines.at(0)->getOverAllLimits()->maxX > SPDLines.at(0)->getOverAllLimits()->minX )
                {
                    maxPossibleDuration = SPDLines.at(0)->getOverAllLimits()->maxX-SPDLines.at(0)->getOverAllLimits()->minX;
                    durationPerSample = maxPossibleDuration/SPDLines.at(0)->getNumSamples();
                }


                if(plotDurationIn > maxPossibleDuration)// Bigger Window than Possible
                    plotWindowSamples=SPDLines.at(0)->getNumSamples();
                else if(plotDurationIn == 0)// Window Deactivated
                    plotWindowSamples=SPDLines.at(0)->getNumSamples();
                else// Active Sliding Window
                   plotWindowSamples = std::max(
                                        std::min((int)(plotDurationIn/durationPerSample),
                                            SPDLines.at(0)->getNumSamples()),
                                            0);

                if(!plotWindowRTIn && ((plotBeginIn+plotDurationIn)<=SPDLines.at(0)->getLimits()->maxX && (plotDurationIn-plotBeginIn)>=SPDLines.at(0)->getLimits()->minX))
                {
                    plotShiftSamples = std::max(
                                        std::min(
                                        (int)((plotBeginIn-SPDLines.at(0)->getLimits()->minX)/durationPerSample),
                                            SPDLines.at(0)->getNumSamples()- plotWindowSamples),
                                            0);
                }

                plotWindowStart = SPDLines.at(0)->getNumSamples() + plotShiftSamples - plotWindowSamples;
            }
            SPDLines.at(j)->UpdateSeries(plotWindowStart, plotWindowSamples);
            SPDLines.at(j)->resetHasNewData();
            newData = true;
        }
    }
    // update plots
    if(newData)
        scalePlot();
}

enum guiSPDSelector GUISPD::getSPDSelector()
{
    return GUIVarSelection;
}
qSPDChart* GUISPD::getSPDChartPtr()
{
    return SPDChartPtr;
}
GUISPD::GUISPD(enum guiSPDSelector GUIVarSelectionIn, qSPDChart* SPDChartPtrIn):
    SPDClass(GUIVarSelectionIn, SPDChartPtrIn, SPDChartPtrIn->getSPDArray())
{
    GUIVarSelection = GUIVarSelectionIn;
    SPDChartPtr = SPDChartPtrIn;
}

float GUISPD::getFloatVal()
{
    return getSPDFloatValue(GUIVarSelection, SPDChartPtr->getSPDArray());
}
const char* GUISPD::getLabelString()
{
    return getSPDLabelString(GUIVarSelection, SPDChartPtr->getSPDArray());
}
const char* GUISPD::getUnitsString()
{
    return getSPDUnitsString(GUIVarSelection, SPDChartPtr->getSPDArray());
}


GUISPDTreeWidgetItem::GUISPDTreeWidgetItem(enum guiSPDSelector GUIVarSelectionIn, qSPDChart* SPDChartPtrIn)
    :SPDTreeWidgetItem(&spd, QStringList({getSPDLabelString(GUIVarSelectionIn,SPDChartPtrIn->getSPDArray()),"",getSPDUnitsString(GUIVarSelectionIn,SPDChartPtrIn->getSPDArray())}))
    ,spd(GUIVarSelectionIn, SPDChartPtrIn)
{

}
/////////////////////////////////////////////////////
/// SmartMotor, Axis SPD LineSeries
//
AxisLineSeriesMap::AxisLineSeriesMap(enum mcsSPDSelector AxisVarSelectionIn, SmartMotorDevice* smDevPtrIn, qSPDChart* spdChartPtr):
    spd(AxisVarSelectionIn,smDevPtrIn),
    spdLine(&spd,spdChartPtr)
{

    spdLine.setName(getSPDLabelString(AxisVarSelectionIn, smDevPtrIn->getSPDArray()));
    spdChartPtr->addSeries(&spdLine);
    spdLine.attachAxis(spdChartPtr->axes().at(0));
    spdLine.attachAxis(spdChartPtr->axes().at(1));
    spdLine.setUseOpenGL(true);
}
SPDLineSeries* AxisLineSeriesMap::getLine()
{
    return &spdLine;
}
/////////////////////////////////////////////////////
/// General SPD Tree Widget Item
//
SPDTreeWidgetItem::SPDTreeWidgetItem(SPDClass* spdPtrIn, QStringList columnsText):QTreeWidgetItem(columnsText)
{
    SPDptr = spdPtrIn;
}
int SPDTreeWidgetItem::GetVarSelectionIn()
{
    return SPDptr->getVarIndex();
}
void* SPDTreeWidgetItem::GetDataPtrIn()
{
    return SPDptr->getDataPtr();
}

/////////////////////////////////////////////////////
/// SmartMotor, Axis SPD Tree Widget Item
//
AxisSPDTreeWidgetItem::AxisSPDTreeWidgetItem(enum mcsSPDSelector AxisVarSelectionIn, SmartMotorDevice* smDevPtrIn):
    SPDTreeWidgetItem(&spd, QStringList({getSPDLabelString(AxisVarSelectionIn,smDevPtrIn->getSPDArray()),"",getSPDUnitsString(AxisVarSelectionIn,smDevPtrIn->getSPDArray())})),
    spd(AxisVarSelectionIn, smDevPtrIn)
{

}
enum mcsSPDSelector AxisSPDTreeWidgetItem::GetAxisVarSelectionIn()
{
    return (enum mcsSPDSelector)GetVarSelectionIn();
}

// ------------------------------------
QccOS::QccOS(QObject *parent)
    : QObject{parent}
{

}
