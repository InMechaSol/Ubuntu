#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMenu>



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
    connect(duplicatChartAction, SIGNAL(triggered()), this, SLOT(LaunchNewWindow()));


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
    mainChart->setChart(new QtCharts::QChart());
    mainChart->chart()->legend()->setAlignment(Qt::AlignBottom);
    mainChart->chart()->legend()->setVisible(true);
    mainChart->chart()->setTitle("My First Plot");
    mainChart->chart()->createDefaultAxes();
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
        statusBar()->showMessage(tr("Stopping..."));
    else
        statusBar()->showMessage(tr("Not Connected..."));
}
void MainWindow::Open()
{
    if(GripperAPI.isConnected())
        statusBar()->showMessage(tr("Opening..."));
    else
        statusBar()->showMessage(tr("Not Connected..."));
}
void MainWindow::Close()
{
    if(GripperAPI.isConnected())
        statusBar()->showMessage(tr("Closing..."));
    else
        statusBar()->showMessage(tr("Not Connected..."));
}
void MainWindow::LatchAndUpdate()
{
    static unsigned int cycleCounter = 0;
    if(cycleCounter%100==0)// every hundredth cycle
    {
        // update live status
        if(GripperAPI.isConnected())
            ConnectedLabel->setText("Gripper: Connected");
        else
            ConnectedLabel->setText("Gripper: Not Connected!");

        if(GripperAPI.isConnected())
        {
            QString posLabel = " | Rotor Pos:";
            IMIGripper::joints_state s = GripperAPI.get_positions();
            for(int k = 0; k<4; k++)
                posLabel += " "+QString::number(s[k])+",";
            PositionsLabel->setText(posLabel);
        }
        else
            PositionsLabel->setText(" | Rotor Pos: ???, ???, ???, ???");

        if(GripperAPI.isConnected())
        {
            QString statLabel = " | Motor Status:";
//            IMIGripper::joints_state s = GripperAPI.get_positions();
//            for(int k = 0; k<4; k++)
//                statLabel += " "+QString::number(s[k])+",";
            MotorsStatusLabel->setText(statLabel);
        }
        else
            MotorsStatusLabel->setText(" | Motor Status: ??, ??, ??, ??");

        // update plots
        mainChart->repaint();
        if(ChildWindows!=nullptr)
        {
            for(int i = 0; i< ChildWindows->count(); i++)
            {
                ChildWindows->at(i)->repaint();
            }
        }
    }

    // update series, if new data
    if(GripperAPI.newData())
    {
        if(AllSPDLineSeries!=nullptr)
        {
            for(int j = 0; j < AllSPDLineSeries->count(); j++)
            {
                AllSPDLineSeries->at(j)->UpdateSeries();
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
    ChildWindows->at(ChildWindows->count()-1)->setWindowTitle("Gripper ChartView");
    mainChart->setChart(new QtCharts::QChart());
    mainChart->chart()->legend()->setAlignment(Qt::AlignBottom);
    mainChart->chart()->legend()->setVisible(true);
    mainChart->chart()->setTitle("My Next Plot");
    mainChart->chart()->createDefaultAxes();
    for(int i =0; i<SPDSelectionMenu->actions().count(); i++)
    {
        SPDSelectionMenu->actions().at(i)->setChecked(false);
    }
}
void MainWindow::closeEvent(QCloseEvent *event)
{
    int i;
    for(i = 0; i < ChildWindows->count(); i++)
    {
        ChildWindows->at(i)->close();
    }
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

}

SPDLineSeriesMap::SPDLineSeriesMap(enum SPDSelector GripperVarSelectionIn, IMIGripper* GripperAPIPtrIn, QtCharts::QChart* spdChartPtr):
    spd(GripperVarSelectionIn,GripperAPIPtrIn),
    spdLine()
{
    spdChart = spdChartPtr;
    spdLine.setName(GripperSPD::getSPDLabelString(GripperVarSelectionIn));
    spdChart->addSeries(&spdLine);

}

QtCharts::QLineSeries* SPDLineSeriesMap::isOnChart(enum SPDSelector GripperVarSelectionIn,  QtCharts::QChart* spdChartPtr)
{
    if( (spd.getSPDSelector()==GripperVarSelectionIn && spdChart==spdChartPtr))
        return &spdLine;
    else
        return nullptr;
}
void SPDLineSeriesMap::UpdateSeries()
{
    static unsigned int samples =0;
    spdLine.append(samples++ ,spd.getSPDFloatValue());
}
