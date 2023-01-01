#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMenu>
#include <QAbstractAxis>
#include <QValueAxis>


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

    SPDSelectionMenu = ui->menubar->addMenu(" | Configure Plot");

    SPDSelectionMenuActions = new QList<QAction*>();
    int i;
    for(i = spdNone+1; i<spdEND; i++)
    {
        SPDSelectionMenuActions->append(SPDSelectionMenu->addAction(GripperSPD::getSPDLabelString((enum SPDSelector)i)));
        SPDSelectionMenuActions->at(i-(spdNone+1))->setCheckable(true);
        connect(SPDSelectionMenuActions->at(i-(spdNone+1)), SIGNAL(triggered()), this, SLOT(ToggleSPDSetting()));
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
    AllSPDLineSeries = new QList<SPDLineSeriesMap*>();
    mainChart = new QtCharts::QChartView();
    setNewMainChart();

    this->setCentralWidget(mainChart);

    // The latch / update timer
    LatchTimer = new QTimer(this);
    connect(LatchTimer, SIGNAL(timeout()), this, SLOT(LatchAndUpdate()) );
    LatchTimer->start(1);

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
    if(AllSPDLineSeries!=nullptr)
    {
        for(int j = 0; j < AllSPDLineSeries->count(); j++)
        {
            AllSPDLineSeries->at(j)->clearSeriesData();
        }
    }
}
void MainWindow::ShowTreeView()
{
    treeWidget = new QTreeWidget();
    treeWidget->setColumnCount(6);
    QList<QTreeWidgetItem *> items;
    QStringList* rowText = new QStringList();
    rowText->append("Parameter");
    rowText->append("Units");
    rowText->append("Motor 0");
    rowText->append("Motor 1");
    rowText->append("Motor 2");
    rowText->append("Motor 3");

    QTreeWidgetItem* thisItem = new QTreeWidgetItem(*rowText);
    treeWidget->setHeaderItem(thisItem);

    rowText->clear();
    rowText->append("Status");
    rowText->append("");
    rowText->append("??");
    rowText->append("??");
    rowText->append("??");
    rowText->append("??");
    thisItem = new QTreeWidgetItem(*rowText);
    items.append(thisItem);

    rowText->clear();
    rowText->append("Temperature");
    rowText->append("(degC)");
    rowText->append("??");
    rowText->append("??");
    rowText->append("??");
    rowText->append("??");
    thisItem = new QTreeWidgetItem(*rowText);
    items.append(thisItem);

    rowText->clear();
    rowText->append("Current");
    rowText->append("(A)");
    rowText->append("??");
    rowText->append("??");
    rowText->append("??");
    rowText->append("??");
    thisItem = new QTreeWidgetItem(*rowText);
    items.append(thisItem);

    rowText->clear();
    rowText->append("PWM Cmd");
    rowText->append("(-1 to 1)");
    rowText->append("??");
    rowText->append("??");
    rowText->append("??");
    rowText->append("??");
    thisItem = new QTreeWidgetItem(*rowText);
    items.append(thisItem);

    treeWidget->insertTopLevelItems(0, items);
    treeWidget->show();
    treeWidget->setWindowTitle("Gripper Smart Motor Data");
    showTreeViewAction->setEnabled(false);
}
void MainWindow::setNewMainChart()
{
    mainChart->setChart(new QtCharts::QChart());
    mainChart->chart()->legend()->setAlignment(Qt::AlignBottom);
    mainChart->chart()->legend()->setVisible(true);
    //mainChart->chart()->setTitle("My First Plot");

    QtCharts::QValueAxis *axisX = new QtCharts::QValueAxis();
    //axisX->setTickCount(10);
    axisX->setTitleText("FW Time");
    mainChart->chart()->addAxis(axisX, Qt::AlignBottom);

    QtCharts::QValueAxis *axisY = new QtCharts::QValueAxis();
    axisY->setLabelFormat("%f");
    //axisY->setTitleText("Sunspots count");
    mainChart->chart()->addAxis(axisY, Qt::AlignLeft);
}

void MainWindow::scalePlots(QtCharts::QChart* chartPtr)
{
    struct maxMinstruct limits = {std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),-std::numeric_limits<float>::max()};
    // determine max min from all spds
    if(AllSPDLineSeries!=nullptr)
    {
        for(int j = 0; j < AllSPDLineSeries->count(); j++)
        {
            if(chartPtr==AllSPDLineSeries->at(j)->getSPDChartPtr())
            {
                if(AllSPDLineSeries->at(j)->getMinTime()<limits.minX)
                    limits.minX=AllSPDLineSeries->at(j)->getMinTime();
                if(AllSPDLineSeries->at(j)->getMaxTime()>limits.maxX)
                    limits.maxX=AllSPDLineSeries->at(j)->getMaxTime();

                if(AllSPDLineSeries->at(j)->getMinY()<limits.minY)
                    limits.minY=AllSPDLineSeries->at(j)->getMinY();
                if(AllSPDLineSeries->at(j)->getMaxY()>limits.maxY)
                    limits.maxY=AllSPDLineSeries->at(j)->getMaxY();
            }
        }
    }

    // apply scaling to plot axes
    QList<QtCharts::QAbstractAxis*> chartAxes = chartPtr->axes();
    chartAxes.at(0)->setRange(limits.minX, limits.maxX);
    chartAxes.at(1)->setRange(limits.minY, limits.maxY);
}
void MainWindow::LatchAndUpdate()
{
    static unsigned int cycleCounter = 0;
    if(cycleCounter%100==0)// every hundredth cycle
    {
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
        if(treeWidget!=nullptr)
        {

        }

        // update plot series        
        if(AllSPDLineSeries!=nullptr)
        {
            for(int j = 0; j < AllSPDLineSeries->count(); j++)
            {
                if(AllSPDLineSeries->at(j)->hasTempData())
                {
                    AllSPDLineSeries->at(j)->UpdateSeries();
                    AllSPDLineSeries->at(j)->clearTempData();
                }
            }
        }

        // update plots
        scalePlots(mainChart->chart());

        if(ChildWindows!=nullptr)
        {
            for(int i = 0; i< ChildWindows->count(); i++)
            {
                scalePlots(ChildWindows->at(i)->chart());

            }
        }
    }

    // latch to tempdata, if new data
    if(GripperAPI.newData())
    {
        if(runPlots)
        {
            if(AllSPDLineSeries!=nullptr)
            {
                for(int j = 0; j < AllSPDLineSeries->count(); j++)
                {
                    AllSPDLineSeries->at(j)->LatchTempData();
                }
            }
        }

        GripperAPI.clearNewDataFlag();
    }
    cycleCounter++;
}
void MainWindow::LaunchNewWindow()
{
    if(ChildWindows==nullptr)
    {
        ChildWindows = new QList<QtCharts::QChartView*>();
    }
    ChildWindows->append(new QtCharts::QChartView());
    ChildWindows->at(ChildWindows->count()-1)->show();    
    ChildWindows->at(ChildWindows->count()-1)->setChart(mainChart->chart());
    ChildWindows->at(ChildWindows->count()-1)->setWindowTitle("Gripper FW Plot");

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
    for(i = 0; i < ChildWindows->count(); i++)
    {
        ChildWindows->at(i)->close();
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
    for(i = spdNone+1; i<spdEND; i++)
    {
       if(SPDSelectionMenuActions->at(i-(spdNone+1))==obj)
       {
           for(int j=0; j<AllSPDLineSeries->count(); j++)
           {
               QtCharts::QLineSeries* t = AllSPDLineSeries->at(j)->isOnChart((enum SPDSelector)i,mainChart->chart());
               if(t!=nullptr)
               {
                   // is present so remove
                   // remove
                   isPresent = true;
                   mainChart->chart()->removeSeries(t);
                   AllSPDLineSeries->removeAt(j);
                   break;
               }
           }
       }
    }

    // if should, add the SPD to the main chart
    if(!isPresent)
    {
        for(i = spdNone+1; i<spdEND; i++)
        {
           if(SPDSelectionMenuActions->at(i-(spdNone+1))==obj)
           {
               AllSPDLineSeries->append(new SPDLineSeriesMap((enum SPDSelector)i, &GripperAPI, mainChart->chart() ) );
               break;
           }
        }
    }

    mainChart->chart()->update();
    if(AllSPDLineSeries->count()>0)
        duplicatChartAction->setEnabled(true);
    else
        duplicatChartAction->setEnabled(false);
}

SPDLineSeriesMap::SPDLineSeriesMap(enum SPDSelector GripperVarSelectionIn, IMIGripper* GripperAPIPtrIn, QtCharts::QChart* spdChartPtr):
    spd(GripperVarSelectionIn,GripperAPIPtrIn),
    spdLine(),
    tempXY()
{
    spdChart = spdChartPtr;
    spdLine.setName(GripperSPD::getSPDLabelString(GripperVarSelectionIn));
    spdChart->addSeries(&spdLine);
    spdLine.attachAxis(spdChart->axes().at(0));
    spdLine.attachAxis(spdChart->axes().at(1));
}

QtCharts::QLineSeries* SPDLineSeriesMap::isOnChart(enum SPDSelector GripperVarSelectionIn,  QtCharts::QChart* spdChartPtr)
{
    if( (spd.getSPDSelector()==GripperVarSelectionIn && spdChart==spdChartPtr))
        return &spdLine;
    else
        return nullptr;
}
bool SPDLineSeriesMap::isOnChart(QtCharts::QChart* spdChartPtr)
{
    return (spdChart==spdChartPtr);
}
void SPDLineSeriesMap::UpdateSeries()
{
    spdLine.append(tempXY);
}
void SPDLineSeriesMap::LatchTempData()
{
    if(spd.getGripperAPIPtr()->getFWTimeStamp()>maxTime)
        maxTime = spd.getGripperAPIPtr()->getFWTimeStamp();
    if(spd.getGripperAPIPtr()->getFWTimeStamp()<minTime)
        minTime = spd.getGripperAPIPtr()->getFWTimeStamp();
    if(spd.getSPDFloatValue()>maxY)
        maxY = spd.getSPDFloatValue();
    if(spd.getSPDFloatValue()<minY)
        minY = spd.getSPDFloatValue();
    tempXY.append( *(new QPointF(spd.getGripperAPIPtr()->getFWTimeStamp(),spd.getSPDFloatValue())));
}
bool SPDLineSeriesMap::hasTempData()
{
    return (tempXY.count()>0);
}
void SPDLineSeriesMap::clearTempData()
{
    tempXY.clear();
}
void SPDLineSeriesMap::clearSeriesData()
{
    tempXY.clear();
    spdLine.clear();
    maxTime = 0;
    minTime = std::numeric_limits<unsigned int>::max();
    maxY = -std::numeric_limits<float>::max();
    minY = std::numeric_limits<float>::max();
}
