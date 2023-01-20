#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMenu>
#include <QAbstractAxis>
#include <QValueAxis>
#include <QWidgetAction>
#include <QLineEdit>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , GripperAPI()
    , ui(new Ui::MainWindow)
{
    // temporary status message
    ui->setupUi(this);
    statusBar()->showMessage(tr("Initializing..."));

    // Menubar Actions
    stopAction = ui->menubar->addAction("STOP");
    connect(stopAction, SIGNAL(triggered()), this, SLOT(Stop()));

    openAction = ui->menubar->addAction("Open");
    connect(openAction, SIGNAL(triggered()), this, SLOT(Open()));

    closeAction = ui->menubar->addAction("Close");
    connect(closeAction, SIGNAL(triggered()), this, SLOT(Close()));

    SPDSelectionMenu0 = ui->menubar->addMenu(" | M0");
    SPDSelectionMenu1 = ui->menubar->addMenu(" | M1");
    SPDSelectionMenu2 = ui->menubar->addMenu(" | M2");
    SPDSelectionMenu3 = ui->menubar->addMenu(" | M3");

    SPDSelectionMenuActions0 = new QList<QAction*>();
    SPDSelectionMenuActions1 = new QList<QAction*>();
    SPDSelectionMenuActions2 = new QList<QAction*>();
    SPDSelectionMenuActions3 = new QList<QAction*>();
    int i;
    for(i = mcsNone+1; i<mcsEND; i++)
    {
        SPDSelectionMenuActions0->append(SPDSelectionMenu0->addAction(getSPDLabelString((enum mcsSPDSelector)i, GripperAPI.smDevPtrs[0]->getSPDArray())));
        SPDSelectionMenuActions0->at(i-(mcsNone+1))->setCheckable(true);
        connect(SPDSelectionMenuActions0->at(i-(mcsNone+1)), SIGNAL(triggered()), this, SLOT(ToggleSPDSetting()));

        SPDSelectionMenuActions1->append(SPDSelectionMenu1->addAction(getSPDLabelString((enum mcsSPDSelector)i, GripperAPI.smDevPtrs[1]->getSPDArray())));
        SPDSelectionMenuActions1->at(i-(mcsNone+1))->setCheckable(true);
        connect(SPDSelectionMenuActions1->at(i-(mcsNone+1)), SIGNAL(triggered()), this, SLOT(ToggleSPDSetting()));

        SPDSelectionMenuActions2->append(SPDSelectionMenu2->addAction(getSPDLabelString((enum mcsSPDSelector)i, GripperAPI.smDevPtrs[2]->getSPDArray())));
        SPDSelectionMenuActions2->at(i-(mcsNone+1))->setCheckable(true);
        connect(SPDSelectionMenuActions2->at(i-(mcsNone+1)), SIGNAL(triggered()), this, SLOT(ToggleSPDSetting()));

        SPDSelectionMenuActions3->append(SPDSelectionMenu3->addAction(getSPDLabelString((enum mcsSPDSelector)i, GripperAPI.smDevPtrs[3]->getSPDArray())));
        SPDSelectionMenuActions3->at(i-(mcsNone+1))->setCheckable(true);
        connect(SPDSelectionMenuActions3->at(i-(mcsNone+1)), SIGNAL(triggered()), this, SLOT(ToggleSPDSetting()));
    }

    duplicatChartAction = ui->menubar->addAction(" | Launch New Chart");
    duplicatChartAction->setEnabled(false);
    connect(duplicatChartAction, SIGNAL(triggered()), this, SLOT(LaunchNewWindow()));

    pauseRunChartsAction = ui->menubar->addAction("   Pause Chart");
    connect(pauseRunChartsAction, SIGNAL(triggered()), this, SLOT(RunPausePlots()));

    clearChartsAction = ui->menubar->addAction("   Clear Chart");
    clearChartsAction->setEnabled(false);
    connect(clearChartsAction, SIGNAL(triggered()), this, SLOT(ClearPlots()));

    showTreeViewAction = ui->menubar->addAction(" | Show Treeview");
    connect(showTreeViewAction, SIGNAL(triggered()), this, SLOT(ShowTreeView()));

    // Statusbar Labels
    ConnectedLabel = new QLabel("Gripper: Not Connected");
    ui->statusbar->addPermanentWidget(ConnectedLabel, 0);

    PositionsLabel = new QLabel(" | Rotor Pos: ???, ???, ???, ???");
    ui->statusbar->addPermanentWidget(PositionsLabel, 0);

    MotorsStatusLabel = new QLabel(" | Motor Status: ??, ??, ??, ??");
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
    ShowTreeView();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::Stop()
{
    if(GripperAPI.isConnected())
    {
        GripperAPI.stop(0);
        statusBar()->showMessage(tr("Stopping..."));
    }
    else
        statusBar()->showMessage(tr("Not Connected..."));
}
void MainWindow::Open()
{
    if(GripperAPI.isConnected())
    {
        GripperAPI.open(0);
        statusBar()->showMessage(tr("Opening..."));
    }
    else
        statusBar()->showMessage(tr("Not Connected..."));
}
void MainWindow::Close()
{
    if(GripperAPI.isConnected())
    {
        GripperAPI.close(0,1);
        statusBar()->showMessage(tr("Closing..."));
    }
    else
        statusBar()->showMessage(tr("Not Connected..."));
}
void MainWindow::RunPausePlots()
{
    runPlots = !runPlots;
    if(runPlots)
    {
        pauseRunChartsAction->setText(" | Pause Charts");
        clearChartsAction->setEnabled(false);
    }
    else
    {
        pauseRunChartsAction->setText(" | Run Charts");
        clearChartsAction->setEnabled(true);
    }

}

void MainWindow::ClearPlots()
{
    ((qSPDChart*)mainChartView->chart())->reset();
    if(childChartViews!=nullptr)
    {
        for(int i = 0; i<childChartViews->count(); i++)
            ((qSPDChart*)childChartViews->at(i)->chart())->reset();
    }
}
static bool fromEdit = false;
void MainWindow::UpdateTreeView()
{
    if(!fromEdit)
    {
        for(int iM = 0; iM < 4; iM++)
        {
            // Axis - Planning
            for(int i = mcsPlndT; i <= mcsDesiredControlMode; i++)
            {
                treeWidget->topLevelItem(iM)->child(0)->child(i-mcsPlndT)->setText(1, QString::number(getSPDFloatValue((enum mcsSPDSelector)i,GripperAPI.smDevPtrs[iM]->getSPDArray())));
            }
            // Axis - Position
            for(int i = mcsPosCMD; i <= mcsPosNLim; i++)
            {
                // DEBUG---REMOVE
                if(i==mcsPosERR)
                {
                    struct SPDStruct* myArPtr = GripperAPI.smDevPtrs[iM]->getSPDArray();
                    float myfloat = getSPDFloatValue(mcsPosERR,myArPtr);

                       myfloat++;
                       myfloat--;

                }
                treeWidget->topLevelItem(iM)->child(1)->child(i-mcsPosCMD)->setText(1, QString::number(getSPDFloatValue((enum mcsSPDSelector)i,GripperAPI.smDevPtrs[iM]->getSPDArray())));
            }
            // Axis - Position Control
            for(int i = mcsPosCtrldT; i <= mcsPosCtrlcmdVel; i++)
            {



                treeWidget->topLevelItem(iM)->child(2)->child(i-mcsPosCtrldT)->setText(1, QString::number(getSPDFloatValue((enum mcsSPDSelector)i,GripperAPI.smDevPtrs[iM]->getSPDArray())));
            }
            // Axis - Velocity
            for(int i = mcsVelCMD; i <= mcsVelNLim; i++)
            {
                treeWidget->topLevelItem(iM)->child(3)->child(i-mcsVelCMD)->setText(1, QString::number(getSPDFloatValue((enum mcsSPDSelector)i,GripperAPI.smDevPtrs[iM]->getSPDArray())));
            }
            // Axis - Velocity Control
            for(int i = mcsVelCtrldT; i <= mcsVelCtrlSaturated; i++)
            {
                treeWidget->topLevelItem(iM)->child(4)->child(i-mcsVelCtrldT)->setText(1, QString::number(getSPDFloatValue((enum mcsSPDSelector)i,GripperAPI.smDevPtrs[iM]->getSPDArray())));
            }
            // Axis - Current
            for(int i = mcsCurCMD; i <= mcsCurNLim; i++)
            {
                treeWidget->topLevelItem(iM)->child(5)->child(i-mcsCurCMD)->setText(1, QString::number(getSPDFloatValue((enum mcsSPDSelector)i,GripperAPI.smDevPtrs[iM]->getSPDArray())));
            }
            // Axis - Motor Model
            for(int i = mcsMotConTorque; i <= mcsMotEff; i++)
            {
                treeWidget->topLevelItem(iM)->child(6)->child(i-mcsMotConTorque)->setText(1, QString::number(getSPDFloatValue((enum mcsSPDSelector)i,GripperAPI.smDevPtrs[iM]->getSPDArray())));
            }
            // Axis - Current Loop
            for(int i = mcsCurCtrldT; i <= mcsCurCtrlSaturated; i++)
            {
                treeWidget->topLevelItem(iM)->child(7)->child(i-mcsCurCtrldT)->setText(1, QString::number(getSPDFloatValue((enum mcsSPDSelector)i,GripperAPI.smDevPtrs[iM]->getSPDArray())));
            }
            // Axis - Torque, Voltage, PWM
            for(int i = mcsPWMCMD; i <= mcsPWMCMD; i++)
            {
                treeWidget->topLevelItem(iM)->child(8)->child(i-mcsPWMCMD)->setText(1, QString::number(getSPDFloatValue((enum mcsSPDSelector)i,GripperAPI.smDevPtrs[iM]->getSPDArray())));
            }
        }
        // Chart - Main Window
        for(int i = guiPltWndwStart; i <= guiPltWindowRt; i++)
        {
            if(((qSPDChart*)mainChartView->chart())->getSPDArray()[i].addr != nullptr)
            treeWidget->topLevelItem(4)->child(0)->child(i-guiPltWndwStart)->setText(1, QString::number(getSPDFloatValue((enum guiSPDSelector)i, ((qSPDChart*)mainChartView->chart())->getSPDArray())));
        }
        // Chart - Line Series 0
        for(int i = guiChrt0MaxX; i <= guiChrt0DurPerSamp; i++)
        {
            if(((qSPDChart*)mainChartView->chart())->getSPDArray()[i].addr != nullptr)
            treeWidget->topLevelItem(4)->child(1)->child(i-guiChrt0MaxX)->setText(1, QString::number(getSPDFloatValue((enum guiSPDSelector)i, ((qSPDChart*)mainChartView->chart())->getSPDArray())));
        }
    }

}
void MainWindow::ShowTreeView()
{
    treeWidget = new QTreeWidget();
    treeWidget->setColumnCount(3);
    QList<QTreeWidgetItem *> items;

    treeWidget->setHeaderItem(new QTreeWidgetItem(QStringList({"Parameter","Value","Units"})));

    for(int iM = 0; iM < 4; iM++)
    {
        // Top Level Axis Structure
        items.append(new QTreeWidgetItem(QStringList({"Finger SmartMotor"+QString::number(iM), "", ""})));

        // Axis - Planning
        items.at(iM)->addChild(new QTreeWidgetItem(QStringList({"Planning Loop", "", ""})));
        for(int i = mcsPlndT; i <= mcsDesiredControlMode; i++)
            items.at(iM)->child(0)->addChild(new AxisSPDTreeWidgetItem((enum mcsSPDSelector)i,GripperAPI.smDevPtrs[iM]));

        // Axis - Position
        items.at(iM)->addChild(new QTreeWidgetItem(QStringList({"Position", "", ""})));
        for(int i = mcsPosCMD; i <= mcsPosNLim; i++)
            items.at(iM)->child(1)->addChild(new AxisSPDTreeWidgetItem((enum mcsSPDSelector)i,GripperAPI.smDevPtrs[iM]));

        // Axis - Position Control
        items.at(iM)->addChild(new QTreeWidgetItem(QStringList({"Position Loop", "", ""})));
        for(int i = mcsPosCtrldT; i <= mcsPosCtrlcmdVel; i++)
            items.at(iM)->child(2)->addChild(new AxisSPDTreeWidgetItem((enum mcsSPDSelector)i,GripperAPI.smDevPtrs[iM]));

        // Axis - Velocity
        items.at(iM)->addChild(new QTreeWidgetItem(QStringList({"Velocity", "", ""})));
        for(int i = mcsVelCMD; i <= mcsVelNLim; i++)
            items.at(iM)->child(3)->addChild(new AxisSPDTreeWidgetItem((enum mcsSPDSelector)i,GripperAPI.smDevPtrs[iM]));

        // Axis - Velocity Control
        items.at(iM)->addChild(new QTreeWidgetItem(QStringList({"Velocity Loop", "", ""})));
        for(int i = mcsVelCtrldT; i <= mcsVelCtrlSaturated; i++)
            items.at(iM)->child(4)->addChild(new AxisSPDTreeWidgetItem((enum mcsSPDSelector)i,GripperAPI.smDevPtrs[iM]));

        // Axis - Current
        items.at(iM)->addChild(new QTreeWidgetItem(QStringList({"Current", "", ""})));
        for(int i = mcsCurCMD; i <= mcsCurNLim; i++)
            items.at(iM)->child(5)->addChild(new AxisSPDTreeWidgetItem((enum mcsSPDSelector)i,GripperAPI.smDevPtrs[iM]));

        // Axis - Motor Model
        items.at(iM)->addChild(new QTreeWidgetItem(QStringList({"Motor Model", "", ""})));
        for(int i = mcsMotConTorque; i <= mcsMotEff; i++)
            items.at(iM)->child(6)->addChild(new AxisSPDTreeWidgetItem((enum mcsSPDSelector)i,GripperAPI.smDevPtrs[iM]));

        // Axis - Current Loop
        items.at(iM)->addChild(new QTreeWidgetItem(QStringList({"Current Loop", "", ""})));
        for(int i = mcsCurCtrldT; i <= mcsCurCtrlSaturated; i++)
            items.at(iM)->child(7)->addChild(new AxisSPDTreeWidgetItem((enum mcsSPDSelector)i,GripperAPI.smDevPtrs[iM]));

        // Axis - Torque, Voltage, PWM
        items.at(iM)->addChild(new QTreeWidgetItem(QStringList({"Torque, Volts, PWM", "", ""})));
        for(int i = mcsPWMCMD; i <= mcsPWMCMD; i++)
            items.at(iM)->child(8)->addChild(new AxisSPDTreeWidgetItem((enum mcsSPDSelector)i,GripperAPI.smDevPtrs[iM]));

    }


    // Top Level Charts Structure
    items.append(new QTreeWidgetItem(QStringList({"Charts", "", ""})));

    // Charts - mainwindow
    items.at(4)->addChild(new QTreeWidgetItem(QStringList({"RT Window", "", ""})));
    for(int i = guiPltWndwStart; i <= guiPltWindowRt; i++)
        items.at(4)->child(0)->addChild(new GUISPDTreeWidgetItem((enum guiSPDSelector)i, ((qSPDChart*)mainChartView->chart())));

    // Charts - first line series
    items.at(4)->addChild(new QTreeWidgetItem(QStringList({"Line Series 0", "", ""})));
    for(int i = guiChrt0MaxX; i <= guiChrt0DurPerSamp; i++)
        items.at(4)->child(1)->addChild(new GUISPDTreeWidgetItem((enum guiSPDSelector)i, ((qSPDChart*)mainChartView->chart())));

    // Now use the list of nodes to populate the Top Level TreeView
    treeWidget->insertTopLevelItems(0, items);
    //treeWidget->setEditTriggers(QAbstractItemView::DoubleClicked);
    treeWidget->show();
    treeWidget->setWindowTitle("Gripper Controller Data");
    showTreeViewAction->setEnabled(true);
    //connect(treeWidget,SIGNAL(),this,SLOT(spdTableChange(QTreeWidgetItem*,int)));
    connect(treeWidget,SIGNAL(itemChanged(QTreeWidgetItem*,int)),this,SLOT(spdTableChange(QTreeWidgetItem*,int)));
    connect(treeWidget,SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),this,SLOT(spdTableEditItem(QTreeWidgetItem*,int)));
}


void MainWindow::spdTableChange(QTreeWidgetItem* wd,int i)
{
    int minIndex = 0;
    SPDStruct* arPtr = nullptr;
    int arIndex = 99;
    bool goodParse;
    if(i==1 && fromEdit)
    {
        if(((SPDTreeWidgetItem*)wd)->GetDataPtrIn()==GripperAPI.smDevPtrs[0]->getSPDArray()[((SPDTreeWidgetItem*)wd)->GetVarSelectionIn()].addr)
        {
            minIndex = mcsNone;
            arPtr = GripperAPI.smDevPtrs[0]->getSPDArray();
            arIndex = 0;
        }
        else if(((SPDTreeWidgetItem*)wd)->GetDataPtrIn()==GripperAPI.smDevPtrs[1]->getSPDArray()[((SPDTreeWidgetItem*)wd)->GetVarSelectionIn()].addr)
        {
            minIndex = mcsNone;
            arPtr = GripperAPI.smDevPtrs[1]->getSPDArray();
            arIndex = 1;
        }
        else if(((SPDTreeWidgetItem*)wd)->GetDataPtrIn()==GripperAPI.smDevPtrs[2]->getSPDArray()[((SPDTreeWidgetItem*)wd)->GetVarSelectionIn()].addr)
        {
            minIndex = mcsNone;
            arPtr = GripperAPI.smDevPtrs[2]->getSPDArray();
            arIndex = 2;
        }
        else if(((SPDTreeWidgetItem*)wd)->GetDataPtrIn()==GripperAPI.smDevPtrs[3]->getSPDArray()[((SPDTreeWidgetItem*)wd)->GetVarSelectionIn()].addr)
        {
            minIndex = mcsNone;
            arPtr = GripperAPI.smDevPtrs[3]->getSPDArray();
            arIndex = 3;
        }
        else if(((SPDTreeWidgetItem*)wd)->GetDataPtrIn()==((qSPDChart*)mainChartView->chart())->getSPDArray()[((SPDTreeWidgetItem*)wd)->GetVarSelectionIn()].addr)
        {
            minIndex = guiNone;
            arPtr = ((qSPDChart*)mainChartView->chart())->getSPDArray();
        }

        if(arPtr!=nullptr&&((SPDTreeWidgetItem*)wd)->GetVarSelectionIn()>minIndex)
        {
            if(!arPtr[((SPDTreeWidgetItem*)wd)->GetVarSelectionIn()].readonly)
            {
                // local only, no package, only parse
                if(arIndex > 3)
                {
                    // if good parse set local SPD value
                    goodParse = setSPDFromString((char*)wd->text(i).toStdString().c_str(), &arPtr[((SPDTreeWidgetItem*)wd)->GetVarSelectionIn()]);
                }
                else // remote FW, parse to temp, then package for transmission
                {
                    // if good parse trigger packaging
                    goodParse = GripperAPI.tryPackageGripperSPDFromString((char*)wd->text(i).toStdString().c_str(),((SPDTreeWidgetItem*)wd)->GetVarSelectionIn(), arPtr);
                }
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
        if(((SPDTreeWidgetItem*)wd)->GetDataPtrIn()==GripperAPI.smDevPtrs[0]->getSPDArray()[((SPDTreeWidgetItem*)wd)->GetVarSelectionIn()].addr)
        {
            minIndex = mcsNone;
            arPtr = GripperAPI.smDevPtrs[0]->getSPDArray();
        }
        else if(((SPDTreeWidgetItem*)wd)->GetDataPtrIn()==GripperAPI.smDevPtrs[1]->getSPDArray()[((SPDTreeWidgetItem*)wd)->GetVarSelectionIn()].addr)
        {
            minIndex = mcsNone;
            arPtr = GripperAPI.smDevPtrs[1]->getSPDArray();
        }
        else if(((SPDTreeWidgetItem*)wd)->GetDataPtrIn()==GripperAPI.smDevPtrs[2]->getSPDArray()[((SPDTreeWidgetItem*)wd)->GetVarSelectionIn()].addr)
        {
            minIndex = mcsNone;
            arPtr = GripperAPI.smDevPtrs[2]->getSPDArray();
        }
        else if(((SPDTreeWidgetItem*)wd)->GetDataPtrIn()==GripperAPI.smDevPtrs[3]->getSPDArray()[((SPDTreeWidgetItem*)wd)->GetVarSelectionIn()].addr)
        {
            minIndex = mcsNone;
            arPtr = GripperAPI.smDevPtrs[3]->getSPDArray();
        }
        else if(((SPDTreeWidgetItem*)wd)->GetDataPtrIn()==((qSPDChart*)mainChartView->chart())->getSPDArray()[((SPDTreeWidgetItem*)wd)->GetVarSelectionIn()].addr)
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
        if(GripperAPI.isConnected())
            ConnectedLabel->setText("FW Time: "+QString::number(GripperAPI.getFWTimeStamp()));
        else
            ConnectedLabel->setText("FW Time: Not Connected!");

        if(GripperAPI.isConnected())
        {
            QString posLabel = " | Rotor Pos:";
            IMIGripper::joints_state s = GripperAPI.get_positions();
            for(int k = 0; k<4; k++)
                posLabel += " "+QString::number(s[k],'F',3)+",";
            PositionsLabel->setText(posLabel);
        }
        else
            PositionsLabel->setText(" | Rotor Pos: ???, ???, ???, ???");

        if(GripperAPI.isConnected())
        {
            QString statLabel = " | Motor Status:";
            for(int k = 0; k<4; k++)
                statLabel += " "+QString::fromStdString(GripperAPI.MotorStatusShortString(k))+",";
            MotorsStatusLabel->setText(statLabel);
        }
        else
            MotorsStatusLabel->setText(" | Motor Status: ??, ??, ??, ??");

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

    // latch to tempdata, if new data
    start = QTime::currentTime().msecsSinceStartOfDay();
    if(GripperAPI.newData())
    {
        if(runPlots)
        {
            // latch the data
            if(AllAxisLineSeries!=nullptr)
            {
                for(int j = 0; j < AllAxisLineSeries->count(); j++)
                {
                    AllAxisLineSeries->at(j)->getLine()->LatchTempData(GripperAPI.getFWTimeStamp());
                }
            }
        }

        GripperAPI.clearNewDataFlag();
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
    childChartViews->at(childChartViews->count()-1)->setWindowTitle("Gripper FW Plot");

    setNewMainChart();
    for(int i =0; i<SPDSelectionMenu0->actions().count(); i++)
    {
        SPDSelectionMenu0->actions().at(i)->setChecked(false);
        SPDSelectionMenu1->actions().at(i)->setChecked(false);
        SPDSelectionMenu2->actions().at(i)->setChecked(false);
        SPDSelectionMenu3->actions().at(i)->setChecked(false);
    }
    duplicatChartAction->setEnabled(false);
}
void MainWindow::closeEvent(QCloseEvent *event)
{
    runPlots = false;
    LatchTimer->stop();
    int i;
    if(childChartViews!=nullptr)
    {
        for(i = 0; i < childChartViews->count(); i++)
        {
            childChartViews->at(i)->close();
        }
    }

    if(treeWidget!=nullptr)
        treeWidget->close();
    QWidget::closeEvent(event);
}
void MainWindow::ToggleSPDSetting()
{
    AxisLineSeriesMap* myAxisLineSeries = nullptr;
    SPDLineSeries* myLineSeries = nullptr;
    QObject* obj = sender();
    bool isPresent=false;
    // is the selected SPD present on the main chart?
    int i;
    for(i = mcsNone+1; i<mcsEND; i++)
    {
        if(isPresent)
            break;
        if(SPDSelectionMenuActions0->at(i-(mcsNone+1))==obj)
        {
           // is the Axis SPD at this index on the main chart?
           myLineSeries = ((qSPDChart*)mainChartView->chart())->getFromChart(GripperAPI.smDevPtrs[0]->getSPDArray()[i].addr);

        }
        else if(SPDSelectionMenuActions1->at(i-(mcsNone+1))==obj)
        {
           // is the Axis SPD at this index on the main chart?
           myLineSeries = ((qSPDChart*)mainChartView->chart())->getFromChart(GripperAPI.smDevPtrs[1]->getSPDArray()[i].addr);

        }
        else if(SPDSelectionMenuActions2->at(i-(mcsNone+1))==obj)
        {
           // is the Axis SPD at this index on the main chart?
           myLineSeries = ((qSPDChart*)mainChartView->chart())->getFromChart(GripperAPI.smDevPtrs[2]->getSPDArray()[i].addr);

        }
        else if(SPDSelectionMenuActions3->at(i-(mcsNone+1))==obj)
        {
           // is the Axis SPD at this index on the main chart?
           myLineSeries = ((qSPDChart*)mainChartView->chart())->getFromChart(GripperAPI.smDevPtrs[3]->getSPDArray()[i].addr);

        }
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
            if(AllAxisLineSeries->count()==0)
            {
                ((qSPDChart*)mainChartView->chart())->getSPDArray()[guiChrt0MaxX].size = 0;
                ((qSPDChart*)mainChartView->chart())->getSPDArray()[guiChrt0MaxX].addr = nullptr;
                ((qSPDChart*)mainChartView->chart())->getSPDArray()[guiChrt0MinX].size = 0;
                ((qSPDChart*)mainChartView->chart())->getSPDArray()[guiChrt0MinX].addr = nullptr;
                ((qSPDChart*)mainChartView->chart())->getSPDArray()[guiChrt0MaxY].size = 0;
                ((qSPDChart*)mainChartView->chart())->getSPDArray()[guiChrt0MaxY].addr = nullptr;
                ((qSPDChart*)mainChartView->chart())->getSPDArray()[guiChrt0MinY].size = 0;
                ((qSPDChart*)mainChartView->chart())->getSPDArray()[guiChrt0MinY].addr = nullptr;
            }
         }
    }

    // if should, add the SPD to the main chart
    if(!isPresent)
    {        
        for(i = mcsNone+1; i<mcsEND; i++)
        {
           if(SPDSelectionMenuActions0->at(i-(mcsNone+1))==obj)
           {
                myAxisLineSeries = new AxisLineSeriesMap(0,(enum mcsSPDSelector)i, GripperAPI.smDevPtrs[0], ((qSPDChart*)mainChartView->chart()));

               break;
           }
           else if(SPDSelectionMenuActions1->at(i-(mcsNone+1))==obj)
           {
                myAxisLineSeries = new AxisLineSeriesMap(1,(enum mcsSPDSelector)i, GripperAPI.smDevPtrs[1], ((qSPDChart*)mainChartView->chart()));

               break;
           }
           else if(SPDSelectionMenuActions2->at(i-(mcsNone+1))==obj)
           {
                myAxisLineSeries = new AxisLineSeriesMap(2,(enum mcsSPDSelector)i, GripperAPI.smDevPtrs[2], ((qSPDChart*)mainChartView->chart()));

               break;
           }
           else if(SPDSelectionMenuActions3->at(i-(mcsNone+1))==obj)
           {
                myAxisLineSeries = new AxisLineSeriesMap(3,(enum mcsSPDSelector)i, GripperAPI.smDevPtrs[3], ((qSPDChart*)mainChartView->chart()));

               break;
           }

        }


        if(AllAxisLineSeries==nullptr){
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
            AllAxisLineSeries=new QList<AxisLineSeriesMap*>();
        }

        if(myAxisLineSeries!=nullptr)
            AllAxisLineSeries->append(myAxisLineSeries);

        if(AllAxisLineSeries!=nullptr)
        {
            if(AllAxisLineSeries->count()>0)
            {
                ((qSPDChart*)mainChartView->chart())->getSPDArray()[guiChrt0MaxX].size = sizeof(AllAxisLineSeries->at(0)->getLine()->getLimits()->maxX);
                ((qSPDChart*)mainChartView->chart())->getSPDArray()[guiChrt0MaxX].addr = &AllAxisLineSeries->at(0)->getLine()->getLimits()->maxX;
                ((qSPDChart*)mainChartView->chart())->getSPDArray()[guiChrt0MinX].size = sizeof(AllAxisLineSeries->at(0)->getLine()->getLimits()->minX);
                ((qSPDChart*)mainChartView->chart())->getSPDArray()[guiChrt0MinX].addr = &AllAxisLineSeries->at(0)->getLine()->getLimits()->minX;
                ((qSPDChart*)mainChartView->chart())->getSPDArray()[guiChrt0MaxY].size = sizeof(AllAxisLineSeries->at(0)->getLine()->getLimits()->maxY);
                ((qSPDChart*)mainChartView->chart())->getSPDArray()[guiChrt0MaxY].addr = &AllAxisLineSeries->at(0)->getLine()->getLimits()->maxY;
                ((qSPDChart*)mainChartView->chart())->getSPDArray()[guiChrt0MinY].size = sizeof(AllAxisLineSeries->at(0)->getLine()->getLimits()->minY);
                ((qSPDChart*)mainChartView->chart())->getSPDArray()[guiChrt0MinY].addr = &AllAxisLineSeries->at(0)->getLine()->getLimits()->minY;
            }
        }

    }

    mainChartView->chart()->update();

    if(AllAxisLineSeries!=nullptr)
    {
        if(AllAxisLineSeries->count()>0)
            duplicatChartAction->setEnabled(true);
        else
            duplicatChartAction->setEnabled(false);
    }

}


