//////////////////////////////////////////////////////////////////////////////////
/// \file Application_Platform_API
#include "Application_Platform_Main.hpp"
#include "Platform_ccOS.hpp"

ccACU_ApplicationClass theApplicationExample(&theExecutionSystem);


// this is a satcom acs only function, called in the setup function
void linkAPIioDevices(struct SatComACSStruct* satcomacsStructPtrIn)
{
    theApplicationExample.linkAPIioDevices();

}

/////////////////////////////////////////////////////////////////////////////////
// SatComACS functions, called cyclically in main loop for each api module

void GetMenuChars(struct uiStruct* uiStructPtrin)
{
    ;
}
// 5) Basic ability for user console output
void WriteMenuLine(struct uiStruct* uiStructPtrin)
{
    ;

}


// 6) (Optional) Logging Output
void WriteLogLine(struct logStruct* logStructPtrin)
{
    // Send log to stdout
    std::cout << &logStructPtrin->devptr->outbuff.charbuff[0];
}
// 7) (Optional) Config Input
void ReadConfigLine(struct configStruct* configStructPtrin)
{
    // Allow for configuration via stdin
    if (stdInThreadRunning == ui8FALSE)
    {
        if (configStructPtrin->devptr->triggerWriteOperation == ui8FALSE)
        {
            configStructPtrin->devptr->newDataReadIn = ui8TRUE;
            //configStructPtrin->parseIndex = 0;
            stdInThreadRunning = ui8TRUE;
        }
    }
}


UI_8 readGPS(struct gpsStruct* gpsStructPtrIn)
{
    return theApplicationExample.readGPS();
}
UI_8 readEcompass(struct eCompStruct* eCompStructPtrIn)
{
    return theApplicationExample.readEcompass();
}


SatComACS_TxRx_WriteFuncsTemplate(std::this_thread::sleep_for(std::chrono::milliseconds(1)))
void readFreqConv(struct freqConvStruct* freqConvStructPtrIn) { ; }
void writeFreqConv(struct freqConvStruct* freqConvStructPtrIn) { ; }
void readPowerMeter(struct powerMeterStruct* powerMeterStructPtrIn) { ; }
void writePowerMeter(struct powerMeterStruct* powerMeterStructPtrIn) { ; }

ccOSVersionsTemplate
int main(int argc, char** argv)
{
    // Start the systick thread
    std::thread systickThread(&OSexecutionSystemClass::ExecuteSysTick, std::ref(theExecutionSystem));

    // Start the exe_thread modules
    theApplicationExample.LinkAndStartExeThreads();

    // run setup
    theExecutionSystem.ExecuteSetup();
    for (;;)
    {
        // run loop
        theExecutionSystem.ExecuteLoop();
    }
    return RETURN_ERROR;
}
