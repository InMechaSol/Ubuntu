﻿#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMenu>
#include <QAbstractAxis>
#include <QValueAxis>
#include <QWidgetAction>
#include <QLineEdit>





///
/// \brief MainWindow::MainWindow
/// \param parent
///
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , smMotionTests()
    , ui(new Ui::MainWindow)
{
    // temporary status message
    ui->setupUi(this);
    statusBar()->showMessage(tr("Initializing..."));

    // Menubar Actions
    pauseRunChartsAction = ui->menubar->addAction("Run");
    connect(pauseRunChartsAction, SIGNAL(triggered()), this, SLOT(RunPausePlots()));

    jogAction = ui->menubar->addAction("Jog");
    connect(jogAction, SIGNAL(triggered()), this, SLOT(Jog()));

    resetAction = ui->menubar->addAction("Reset");
    resetAction->setEnabled(false);
    connect(resetAction, SIGNAL(triggered()), this, SLOT(Reset()));


    openAction = ui->menubar->addAction("Open");
    connect(openAction, SIGNAL(triggered()), this, SLOT(Open()));

    closeAction = ui->menubar->addAction("Close");
    connect(closeAction, SIGNAL(triggered()), this, SLOT(Close()));

    SPDSelectionMenu = ui->menubar->addMenu(" | Configure Plot");

    SPDSelectionMenuActions = new QList<QAction*>();
    int i;
    for(i = mcsNone+1; i<mcsEND; i++)
    {
        SPDSelectionMenuActions->append(SPDSelectionMenu->addAction(getSPDLabelString((enum mcsSPDSelector)i, smMotionTests.getSPDArray())));
        SPDSelectionMenuActions->at(i-(mcsNone+1))->setCheckable(true);
        connect(SPDSelectionMenuActions->at(i-(mcsNone+1)), SIGNAL(triggered()), this, SLOT(ToggleSPDSetting()));
    }


    duplicatChartAction = ui->menubar->addAction(" | Launch New Chart");
    duplicatChartAction->setEnabled(false);
    connect(duplicatChartAction, SIGNAL(triggered()), this, SLOT(LaunchNewWindow()));





    showTreeViewAction = ui->menubar->addAction(" | Show Treeview");
    connect(showTreeViewAction, SIGNAL(triggered()), this, SLOT(ShowTreeView()));

    // Statusbar Labels
    ConnectedLabel = new QLabel("Cycles and Loop Times");
    ui->statusbar->addPermanentWidget(ConnectedLabel, 0);

    PositionsLabel = new QLabel("Execution Times");
    ui->statusbar->addPermanentWidget(PositionsLabel, 0);

    MotorsStatusLabel = new QLabel("SPDs and Data");
    ui->statusbar->addPermanentWidget(MotorsStatusLabel, 0);


    // The Main ChartView

    mainChartView = new QtCharts::QChartView();
    setNewMainChart();

    plotDuration = 0.0;
    plotBegin = 0.0;

    this->setCentralWidget(mainChartView);

    // The latch / update timer
    LatchTimer = new QTimer(this);
    connect(LatchTimer, SIGNAL(timeout()), this, SLOT(LatchAndUpdate()) );
    LatchTimer->start(latchNUpdateMS);
    RunPausePlots();

//    SPDSelectionMenuActions->at(spdPosCMD-1)->trigger();
//    SPDSelectionMenuActions->at(spdPosFBK-1)->trigger();
//    SPDSelectionMenuActions->at(spdPosERR-1)->trigger();
//    SPDSelectionMenuActions->at(spdPosPLim-1)->trigger();
//    SPDSelectionMenuActions->at(spdPosNLim-1)->trigger();
//    duplicatChartAction->trigger();

//    SPDSelectionMenuActions->at(spdVelCMD-1)->trigger();
//    SPDSelectionMenuActions->at(spdVelFBK-1)->trigger();
//    SPDSelectionMenuActions->at(spdVelERR-1)->trigger();
//    SPDSelectionMenuActions->at(spdVelPLim-1)->trigger();
//    SPDSelectionMenuActions->at(spdVelNLim-1)->trigger();
//





    // do this
    //SPDSelectionMenuActions->at(mcsMotTime-1)->trigger();
    //duplicatChartAction->trigger();

    // electrical voltage/current
    //SPDSelectionMenuActions->at(mcsMotV-1)->trigger();
    //SPDSelectionMenuActions->at(mcsMotI-1)->trigger();
    //duplicatChartAction->trigger();

    // Torques
    //SPDSelectionMenuActions->at(mcsMotTorque-1)->trigger();
    //SPDSelectionMenuActions->at(mcsMotFricTor-1)->trigger();
    //duplicatChartAction->trigger();

    // Rotor Speed
    SPDSelectionMenuActions->at(mcsMotW-1)->trigger();

    // do this last
    showTreeViewAction->trigger();


}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::Jog()
{
    jogonecycle = true;
    resetAction->setEnabled(true);
}
void MainWindow::Open()
{
//    if(GripperAPI.isConnected())
//    {
//        GripperAPI.open(0);
//        statusBar()->showMessage(tr("Opening..."));
//    }
//    else
//        statusBar()->showMessage(tr("Not Connected..."));
}
void MainWindow::Close()
{
//    if(GripperAPI.isConnected())
//    {
//        GripperAPI.close(0,1);
//        statusBar()->showMessage(tr("Closing..."));
//    }
//    else
//        statusBar()->showMessage(tr("Not Connected..."));
}
void MainWindow::RunPausePlots()
{
    runPlots = !runPlots;
    if(runPlots)
    {
        pauseRunChartsAction->setText("Stop");
        resetAction->setEnabled(false);
    }
    else
    {
        pauseRunChartsAction->setText("Run");
        resetAction->setEnabled(true);
    }

}

void MainWindow::Reset()
{
    // Reset Charts
    ((qSPDChart*)mainChartView->chart())->reset();
    if(childChartViews!=nullptr)
    {
        for(int i = 0; i<childChartViews->count(); i++)
            ((qSPDChart*)childChartViews->at(i)->chart())->reset();
    }
    // Reset Model
    *((UI_8*)smMotionTests.getSPDArray()[mcsCurCtrlReset].addr) = ui8TRUE;
    *((UI_8*)smMotionTests.getSPDArray()[mcsVelCtrlReset].addr) = ui8TRUE;
    *((UI_8*)smMotionTests.getSPDArray()[mcsMotReset].addr) = ui8TRUE;
    resetAction->setEnabled(false);
}
static bool fromEdit = false;
void MainWindow::UpdateTreeView()
{
    if(!fromEdit)
    {
        // Axis - Planning
        for(int i = mcsPlndT; i <= mcsDesiredControlMode; i++)
        {
            treeWidget->topLevelItem(0)->child(0)->child(i-mcsPlndT)->setText(1, QString::number(getSPDFloatValue((enum mcsSPDSelector)i,smMotionTests.getSPDArray())));
        }
        // Axis - Position
        for(int i = mcsPosCMD; i <= mcsPosNLim; i++)
        {
            treeWidget->topLevelItem(0)->child(1)->child(i-mcsPosCMD)->setText(1, QString::number(getSPDFloatValue((enum mcsSPDSelector)i,smMotionTests.getSPDArray())));
        }
        // Axis - Position Control
        for(int i = mcsPosCtrldT; i <= mcsPosCtrlcmdVel; i++)
        {
            treeWidget->topLevelItem(0)->child(2)->child(i-mcsPosCtrldT)->setText(1, QString::number(getSPDFloatValue((enum mcsSPDSelector)i,smMotionTests.getSPDArray())));
        }
        // Axis - Velocity
        for(int i = mcsVelCMD; i <= mcsVelNLim; i++)
        {
            treeWidget->topLevelItem(0)->child(3)->child(i-mcsVelCMD)->setText(1, QString::number(getSPDFloatValue((enum mcsSPDSelector)i,smMotionTests.getSPDArray())));
        }
        // Axis - Velocity Control
        for(int i = mcsVelCtrldT; i <= mcsVelCtrlSaturated; i++)
        {
            treeWidget->topLevelItem(0)->child(4)->child(i-mcsVelCtrldT)->setText(1, QString::number(getSPDFloatValue((enum mcsSPDSelector)i,smMotionTests.getSPDArray())));
        }
        // Axis - Current
        for(int i = mcsCurCMD; i <= mcsCurNLim; i++)
        {
            treeWidget->topLevelItem(0)->child(5)->child(i-mcsCurCMD)->setText(1, QString::number(getSPDFloatValue((enum mcsSPDSelector)i,smMotionTests.getSPDArray())));
        }
        // Axis - Motor Model
        for(int i = mcsMotConTorque; i <= mcsMotEff; i++)
        {
            treeWidget->topLevelItem(0)->child(6)->child(i-mcsMotConTorque)->setText(1, QString::number(getSPDFloatValue((enum mcsSPDSelector)i,smMotionTests.getSPDArray())));
        }
        // Axis - Current Loop
        for(int i = mcsCurCtrldT; i <= mcsCurCtrlSaturated; i++)
        {
            treeWidget->topLevelItem(0)->child(7)->child(i-mcsCurCtrldT)->setText(1, QString::number(getSPDFloatValue((enum mcsSPDSelector)i,smMotionTests.getSPDArray())));
        }
        // Axis - Torque, Voltage, PWM
        for(int i = mcsPWMCMD; i <= mcsPWMCMD; i++)
        {
            treeWidget->topLevelItem(0)->child(8)->child(i-mcsPWMCMD)->setText(1, QString::number(getSPDFloatValue((enum mcsSPDSelector)i,smMotionTests.getSPDArray())));
        }
        // Chart - Main Window
        for(int i = guiPltWndwStart; i <= guiPltWindowRt; i++)
        {
            treeWidget->topLevelItem(1)->child(0)->child(i-guiPltWndwStart)->setText(1, QString::number(getSPDFloatValue((enum guiSPDSelector)i, ((qSPDChart*)mainChartView->chart())->getSPDArray())));
        }
        // Chart - Line Series 0
        for(int i = guiChrt0MaxX; i <= guiChrt0DurPerSamp; i++)
        {
            treeWidget->topLevelItem(1)->child(1)->child(i-guiChrt0MaxX)->setText(1, QString::number(getSPDFloatValue((enum guiSPDSelector)i, ((qSPDChart*)mainChartView->chart())->getSPDArray())));
        }
    }

}
void MainWindow::ShowTreeView()
{
    treeWidget = new QTreeWidget();    
    treeWidget->setColumnCount(3);
    QList<QTreeWidgetItem *> items;

    treeWidget->setHeaderItem(new QTreeWidgetItem(QStringList({"Parameter","Value","Units"})));

    // Top Level Axis Structure
    items.append(new QTreeWidgetItem(QStringList({"Smart Motor Axis", "", ""})));

    // Axis - Planning
    items.at(0)->addChild(new QTreeWidgetItem(QStringList({"Planning Loop", "", ""})));
    for(int i = mcsPlndT; i <= mcsDesiredControlMode; i++)
        items.at(0)->child(0)->addChild(new AxisSPDTreeWidgetItem((enum mcsSPDSelector)i,&smMotionTests));

    // Axis - Position
    items.at(0)->addChild(new QTreeWidgetItem(QStringList({"Position", "", ""})));
    for(int i = mcsPosCMD; i <= mcsPosNLim; i++)
        items.at(0)->child(1)->addChild(new AxisSPDTreeWidgetItem((enum mcsSPDSelector)i,&smMotionTests));

    // Axis - Position Control
    items.at(0)->addChild(new QTreeWidgetItem(QStringList({"Position Loop", "", ""})));
    for(int i = mcsPosCtrldT; i <= mcsPosCtrlcmdVel; i++)
        items.at(0)->child(2)->addChild(new AxisSPDTreeWidgetItem((enum mcsSPDSelector)i,&smMotionTests));

    // Axis - Velocity
    items.at(0)->addChild(new QTreeWidgetItem(QStringList({"Velocity", "", ""})));
    for(int i = mcsVelCMD; i <= mcsVelNLim; i++)
        items.at(0)->child(3)->addChild(new AxisSPDTreeWidgetItem((enum mcsSPDSelector)i,&smMotionTests));

    // Axis - Velocity Control
    items.at(0)->addChild(new QTreeWidgetItem(QStringList({"Velocity Loop", "", ""})));
    for(int i = mcsVelCtrldT; i <= mcsVelCtrlSaturated; i++)
        items.at(0)->child(4)->addChild(new AxisSPDTreeWidgetItem((enum mcsSPDSelector)i,&smMotionTests));

    // Axis - Current
    items.at(0)->addChild(new QTreeWidgetItem(QStringList({"Current", "", ""})));
    for(int i = mcsCurCMD; i <= mcsCurNLim; i++)
        items.at(0)->child(5)->addChild(new AxisSPDTreeWidgetItem((enum mcsSPDSelector)i,&smMotionTests));

    // Axis - Motor Model
    items.at(0)->addChild(new QTreeWidgetItem(QStringList({"Motor Model", "", ""})));
    for(int i = mcsMotConTorque; i <= mcsMotEff; i++)
        items.at(0)->child(6)->addChild(new AxisSPDTreeWidgetItem((enum mcsSPDSelector)i,&smMotionTests));

    // Axis - Current Loop
    items.at(0)->addChild(new QTreeWidgetItem(QStringList({"Current Loop", "", ""})));
    for(int i = mcsCurCtrldT; i <= mcsCurCtrlSaturated; i++)
        items.at(0)->child(7)->addChild(new AxisSPDTreeWidgetItem((enum mcsSPDSelector)i,&smMotionTests));

    // Axis - Torque, Voltage, PWM
    items.at(0)->addChild(new QTreeWidgetItem(QStringList({"Torque, Volts, PWM", "", ""})));
    for(int i = mcsPWMCMD; i <= mcsPWMCMD; i++)
        items.at(0)->child(8)->addChild(new AxisSPDTreeWidgetItem((enum mcsSPDSelector)i,&smMotionTests));

    // Top Level Charts Structure
    items.append(new QTreeWidgetItem(QStringList({"Charts", "", ""})));

    // Charts - mainwindow
    items.at(1)->addChild(new QTreeWidgetItem(QStringList({"RT Window", "", ""})));
    for(int i = guiPltWndwStart; i <= guiPltWindowRt; i++)
        items.at(1)->child(0)->addChild(new GUISPDTreeWidgetItem((enum guiSPDSelector)i, ((qSPDChart*)mainChartView->chart())));

    // Charts - first line series
    items.at(1)->addChild(new QTreeWidgetItem(QStringList({"Line Series 0", "", ""})));
    for(int i = guiChrt0MaxX; i <= guiChrt0DurPerSamp; i++)
        items.at(1)->child(1)->addChild(new GUISPDTreeWidgetItem((enum guiSPDSelector)i, ((qSPDChart*)mainChartView->chart())));

    // Now use the list of nodes to populate the Top Level TreeView
    treeWidget->insertTopLevelItems(0, items);
    //treeWidget->setEditTriggers(QAbstractItemView::DoubleClicked);
    treeWidget->show();
    treeWidget->setWindowTitle("Motion Control Unit Test Data");
    showTreeViewAction->setEnabled(true);
    //connect(treeWidget,SIGNAL(),this,SLOT(spdTableChange(QTreeWidgetItem*,int)));
    connect(treeWidget,SIGNAL(itemChanged(QTreeWidgetItem*,int)),this,SLOT(spdTableChange(QTreeWidgetItem*,int)));
    connect(treeWidget,SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),this,SLOT(spdTableEditItem(QTreeWidgetItem*,int)));
}


void MainWindow::spdTableChange(QTreeWidgetItem* wd,int i)
{
    int minIndex = 0;
    SPDStruct* arPtr = nullptr;
    if(i==1 && fromEdit)
    {
        if(((SPDTreeWidgetItem*)wd)->GetDataPtrIn()==&smMotionTests)
        {
            minIndex = mcsNone;
            arPtr = smMotionTests.getSPDArray();
        }
        else if(((SPDTreeWidgetItem*)wd)->GetDataPtrIn()==((qSPDChart*)mainChartView->chart()))
        {
            minIndex = guiNone;
            arPtr = ((qSPDChart*)mainChartView->chart())->getSPDArray();
        }

        if(arPtr!=nullptr&&((SPDTreeWidgetItem*)wd)->GetVarSelectionIn()>minIndex)
        {
            if(!arPtr[((SPDTreeWidgetItem*)wd)->GetVarSelectionIn()].readonly)
            {
                // NEW SPD set from String Function - Use it HERE!

                // And set in packet function...for use on GUI API Tests for Gripper
                bool goodParse;
                float tempFloat = wd->text(i).toFloat(&goodParse);
                if(goodParse)
                    setSPDFloatValue(tempFloat, ((SPDTreeWidgetItem*)wd)->GetVarSelectionIn(), arPtr);
            }
        }
        wd->setFlags(wd->flags() & ~Qt::ItemIsEditable);
        fromEdit=false;
    }

}

void MainWindow::spdTableEditItem(QTreeWidgetItem* wd,int i)
{
    int minIndex = 0;
    SPDStruct* arPtr = nullptr;
    if(i==1)
    {
        if(((SPDTreeWidgetItem*)wd)->GetDataPtrIn()==&smMotionTests)
        {
            minIndex = mcsNone;
            arPtr = smMotionTests.getSPDArray();
        }
        else if(((SPDTreeWidgetItem*)wd)->GetDataPtrIn()==((qSPDChart*)mainChartView->chart()))
        {
            minIndex = guiNone;
            arPtr = ((qSPDChart*)mainChartView->chart())->getSPDArray();
        }
        if(arPtr!=nullptr&&((SPDTreeWidgetItem*)wd)->GetVarSelectionIn()>minIndex)
        {
            if(!arPtr[((SPDTreeWidgetItem*)wd)->GetVarSelectionIn()].readonly)
            {
                wd->setFlags(wd->flags() | Qt::ItemIsEditable);
                wd->treeWidget()->editItem(wd, i);
                fromEdit=true;
            }
        }
    }
}
void MainWindow::setNewMainChart()
{
    mainChartView->setChart(new qSPDChart());
    mainChartView->chart()->legend()->setAlignment(Qt::AlignBottom);
    mainChartView->chart()->legend()->setVisible(true);
    //mainChart->chart()->setTitle("My First Plot");

    QtCharts::QValueAxis *axisX = new QtCharts::QValueAxis();
    //axisX->setTickCount(10);
    axisX->setTitleText("Time");
    mainChartView->chart()->addAxis(axisX, Qt::AlignBottom);

    QtCharts::QValueAxis *axisY = new QtCharts::QValueAxis();
    axisY->setLabelFormat("%f");
    //axisY->setTitleText("Sunspots count");
    mainChartView->chart()->addAxis(axisY, Qt::AlignLeft);
}

///
/// \brief MainWindow::LatchAndUpdate
///
///

void MainWindow::LatchAndUpdate()
{
    LatchTimer->stop();
    systemModelTimeDelta = 0.001*(QTime::currentTime().msecsSinceStartOfDay()-systemmodeltime);
    systemmodeltime = QTime::currentTime().msecsSinceStartOfDay();

    // update GUI for active user view
    if((QTime::currentTime().msecsSinceStartOfDay()-systemguitime)>249)// 4 times per second
    {
        systemGUITimeDelta = 0.001*(QTime::currentTime().msecsSinceStartOfDay()-systemguitime);
        systemguitime = QTime::currentTime().msecsSinceStartOfDay();

        // update live status
        ConnectedLabel->setText(
                    "Motion Time: "+QString::number(getSPDFloatValue(mcsPlnTMotion, smMotionTests.getSPDArray()),'f',3)+
                    " Cycles: "+QString::number(cycleCounter)+
                    " GUITimeDelta: "+QString::number(systemGUITimeDelta,'f',3)+
                    " ModelTimeDelta: "+QString::number(systemModelTimeDelta,'f',3)
                    );



        PositionsLabel->setText(
                    "  | dT-TreeView: "+QString::number(deltaTreeView*0.001,'f',3)+
                    " dT-Plots: "+QString::number(deltaPlots*0.001,'f',3)+
                    " dT-Model: "+QString::number(deltaModel*0.001,'f',3)
                    );


        if(AllAxisLineSeries!=nullptr)
        {
            if(AllAxisLineSeries->count()>0)
            {
                MotorsStatusLabel->setText(
                            "  | SPDs: " + QString::number(AllAxisLineSeries->count())+
                            " Samples: " + QString::number(AllAxisLineSeries->at(0)->getLine()->getNumSamples())+
                            " MBs: " + QString::number(AllAxisLineSeries->count()*AllAxisLineSeries->at(0)->getLine()->getNumSamples()*0.000008,'f',6)
                            );
            }
            else
                MotorsStatusLabel->setText(
                            "  | SPDs: " + QString::number(AllAxisLineSeries->count())+
                            " Samples: " + QString::number(0)+
                            " MBs: " + QString::number(0,'f',6)
                            );

        }
        else
            MotorsStatusLabel->setText(
                        "  | SPDs: " + QString::number(0)+
                        " Samples: " + QString::number(0)+
                        " MBs: " + QString::number(0,'f',6)
                        );

        // Data Treeview
        start = QTime::currentTime().msecsSinceStartOfDay();
        if(treeWidget!=nullptr)
        {            
            UpdateTreeView();
        }
        stop = QTime::currentTime().msecsSinceStartOfDay();
        deltaTreeView = stop-start;

        // update charts - plot series
        start = QTime::currentTime().msecsSinceStartOfDay();
        mainChartView->setUpdatesEnabled(false);
        ((qSPDChart*)mainChartView->chart())->UpdateChartSeries(plotDuration, plotBegin, plotWindowRT);
        mainChartView->setUpdatesEnabled(true);
        if(childChartViews!=nullptr){
            for(int i = 0; i< childChartViews->count(); i++)
            {
                childChartViews->at(i)->setUpdatesEnabled(false);
                ((qSPDChart*)childChartViews->at(i)->chart())->UpdateChartSeries(plotDuration, plotBegin, plotWindowRT);
                childChartViews->at(i)->setUpdatesEnabled(true);
            }}

        stop = QTime::currentTime().msecsSinceStartOfDay();
        deltaPlots = stop-start;
    }

    // latch to tempdata, run the model
    start = QTime::currentTime().msecsSinceStartOfDay();
    if(runPlots || jogonecycle)
    {
        for(int kd = 0; kd < latchNUpdateMS; kd++)
        {
            // run the model
            smMotionTests.execute();

            // latch the data
            if(AllAxisLineSeries!=nullptr)
            {
                for(int j = 0; j < AllAxisLineSeries->count(); j++)
                {
                    AllAxisLineSeries->at(j)->getLine()->LatchTempData(getSPDFloatValue(mcsMotTime,smMotionTests.getSPDArray()));
                }
            }
        }
        if(jogonecycle)
            jogonecycle = false;
    }
    stop = QTime::currentTime().msecsSinceStartOfDay();
    deltaModel = stop-start;
    deltaLatchNUpdate = stop-systemmodeltime;
    cycleCounter++;
    int timerMS;
    if(latchNUpdateMS <= deltaLatchNUpdate)
        timerMS = 1;
    else
        timerMS = latchNUpdateMS - deltaLatchNUpdate;
    LatchTimer->start(timerMS);
}
void MainWindow::LaunchNewWindow()
{
    if(childChartViews==nullptr)
    {
        childChartViews = new QList<QtCharts::QChartView*>();
    }
    childChartViews->append(new QtCharts::QChartView());
    childChartViews->at(childChartViews->count()-1)->show();
    childChartViews->at(childChartViews->count()-1)->setChart(mainChartView->chart());
    childChartViews->at(childChartViews->count()-1)->setWindowTitle("Motion Control Plot");

    setNewMainChart();
    for(int i =0; i<SPDSelectionMenu->actions().count(); i++)
    {
        SPDSelectionMenu->actions().at(i)->setChecked(false);
    }
    duplicatChartAction->setEnabled(false);
}
void MainWindow::closeEvent(QCloseEvent *event)
{
    int i;
    for(i = 0; i < childChartViews->count(); i++)
    {
        childChartViews->at(i)->close();
    }
    if(treeWidget!=nullptr)
        treeWidget->close();
    QWidget::closeEvent(event);
}
void MainWindow::ToggleSPDSetting()
{
    QObject* obj = sender();
    bool isPresent=false;
    // is the selected SPD present on the main chart?
    int i;
    for(i = mcsNone+1; i<mcsEND; i++)
    {
        if(isPresent)
            break;
        if(SPDSelectionMenuActions->at(i-(mcsNone+1))==obj)
        {
           // is the Axis SPD at this index on the main chart?
           SPDLineSeries* myLineSeries = ((qSPDChart*)mainChartView->chart())->getFromChart(i);
           if(myLineSeries!=nullptr)
           {
               isPresent = true;
               ((qSPDChart*)mainChartView->chart())->removeSeries(myLineSeries);
               for(int q = 0; q<AllAxisLineSeries->count(); q++)
               {
                   if(AllAxisLineSeries->at(q)->getLine() == myLineSeries)
                   {
                       AllAxisLineSeries->removeAt(q);
                       break;
                   }
               }
            }
        }
    }

    // if should, add the SPD to the main chart
    if(!isPresent)
    {
        for(i = mcsNone+1; i<mcsEND; i++)
        {
           if(SPDSelectionMenuActions->at(i-(mcsNone+1))==obj)
           {
                AxisLineSeriesMap* myAxisLineSeries = new AxisLineSeriesMap((enum mcsSPDSelector)i, &smMotionTests, ((qSPDChart*)mainChartView->chart()));

               if(AllAxisLineSeries==nullptr)
               {
                   ((qSPDChart*)mainChartView->chart())->getSPDArray()[guiPltDuration].size = sizeof(plotDuration);
                   ((qSPDChart*)mainChartView->chart())->getSPDArray()[guiPltDuration].addr = &plotDuration;
                   ((qSPDChart*)mainChartView->chart())->getSPDArray()[guiPltBegin].size = sizeof(plotBegin);
                   ((qSPDChart*)mainChartView->chart())->getSPDArray()[guiPltBegin].addr = &plotBegin;
                   ((qSPDChart*)mainChartView->chart())->getSPDArray()[guiPltWindowRt].size = sizeof(plotWindowRT);
                   ((qSPDChart*)mainChartView->chart())->getSPDArray()[guiPltWindowRt].addr = &plotWindowRT;

                   ((qSPDChart*)mainChartView->chart())->getSPDArray()[guiPltWndwStart].size = sizeof(((qSPDChart*)mainChartView->chart())->plotWindowStart);
                   ((qSPDChart*)mainChartView->chart())->getSPDArray()[guiPltWndwStart].addr = &((qSPDChart*)mainChartView->chart())->plotWindowStart;
                   ((qSPDChart*)mainChartView->chart())->getSPDArray()[guiPltWndwSamples].size = sizeof(((qSPDChart*)mainChartView->chart())->plotWindowSamples);
                   ((qSPDChart*)mainChartView->chart())->getSPDArray()[guiPltWndwSamples].addr = &((qSPDChart*)mainChartView->chart())->plotWindowSamples;
                   ((qSPDChart*)mainChartView->chart())->getSPDArray()[guiChrt0MaxX].size = sizeof(myAxisLineSeries->getLine()->getLimits()->maxX);
                   ((qSPDChart*)mainChartView->chart())->getSPDArray()[guiChrt0MaxX].addr = &myAxisLineSeries->getLine()->getLimits()->maxX;
                   ((qSPDChart*)mainChartView->chart())->getSPDArray()[guiChrt0MinX].size = sizeof(myAxisLineSeries->getLine()->getLimits()->minX);
                   ((qSPDChart*)mainChartView->chart())->getSPDArray()[guiChrt0MinX].addr = &myAxisLineSeries->getLine()->getLimits()->minX;
                   ((qSPDChart*)mainChartView->chart())->getSPDArray()[guiChrt0MaxY].size = sizeof(myAxisLineSeries->getLine()->getLimits()->maxY);
                   ((qSPDChart*)mainChartView->chart())->getSPDArray()[guiChrt0MaxY].addr = &myAxisLineSeries->getLine()->getLimits()->maxY;
                   ((qSPDChart*)mainChartView->chart())->getSPDArray()[guiChrt0MinY].size = sizeof(myAxisLineSeries->getLine()->getLimits()->minY);
                   ((qSPDChart*)mainChartView->chart())->getSPDArray()[guiChrt0MinY].addr = &myAxisLineSeries->getLine()->getLimits()->minY;

                   AllAxisLineSeries=new QList<AxisLineSeriesMap*>();
               }
                AllAxisLineSeries->append(myAxisLineSeries);
               break;
           }
        }
    }

    mainChartView->chart()->update();

    if(AllAxisLineSeries->count()>0)
        duplicatChartAction->setEnabled(true);
    else
        duplicatChartAction->setEnabled(false);
}

