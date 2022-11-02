/** \file WIN32_Console.cpp
*   \brief WIN32 Console Implementation of ccNOos_Tests in c++

   Copyright 2021 InMechaSol, Inc

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

Notes:

*/

// This file should only ever be included by one file, PlatformApp_Serialization.cpp
// Therefore, it is not #ifdef guarded and will cause "multiple definition" errors if included more than once.
#include "osApp_Serialization.hpp"

#include "ControlServerClass.hpp"
#include "UIServerClass.hpp"
#include "FWControlClientClass.hpp"

#include <iostream>

// 0) (Optional) Platform Config and Log Files/Devices
//std::ifstream configFile;
//std::ofstream LogFile;




// Now bring in any platform specific io devices
#include "adafruit_ft232h.h"
#include "tcp_comms.h"
#include "serial_comms.h"


// on the ts4900 platform, this function is not called.
// instead the console interface is served over TCP to
// a console front end
std::thread stdInThread;
UI_8 stdInThreadRunning = ui8FALSE;
void readStdIn(char* inStringPtr)
{
    do {
        if (stdInThreadRunning == ui8TRUE)
        {
            std::cin >> inStringPtr;
            stdInThreadRunning = ui8FALSE;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    } while (true);
}

//////////////////////////////////////////////////////////
/// \brief The ccACU_ApplicationClass class
///
class ccACU_ApplicationClass : public ccOSApplicationClass
{
public:
    // Linked Entry Points for execution of module entry points
    linkedEntryPointClass setupListHead;
    linkedEntryPointClass loopListHead;
    linkedEntryPointClass systickListHead;
    linkedEntryPointClass exceptionListHead;

    // The ccACU Compute Module and Data
    ccACU_Class ccACU_compMod;
    struct ccACUStruct ccACU_data;

    // UI api Compute Module
    struct UIServerStruct UIServer_data;
    UI_ServerClass UIServer_exeThread;
    consoleMenuClass LCDKeyPadMenuInst;
    consoleMenuClass Console1MenuInst;
    consoleMenuClass Console2MenuInst;

    // SNMP api Compute Module
    SNMP_AgentsAPIServer SNMPServer_exeThread;
    struct SNMPServerStruct SNMPServer_data;

    // "CGI" api Compute Module
    CGI_ServerClass CGIServer_exeThread;
    struct CGIServerStruct CGIServer_data;

    // Device Compute Modules from ccNOos / SatComACS layer
    APT_WMM_Class APT_WMM_exeThread; // adds the wmm file process for live declination estimates
    TPM_Class TPM_exeThread; // adds async io via usb-spi
    TxRx_Class TxRx_exeThread; // adds async io via usb-spi

    // Device Compute Modules from ccOS / ccACU layer
    ModemClass Modem_exeThread;
    ManagedSwitchClass Switch_exeThread;

    // APT Devices
    struct portParametersStruct GPSPortParams = buildportParametersStruct("/dev/ttyACM0", 9600);
    struct portParametersStruct eCompPortParams = buildportParametersStruct("/dev/ttyUSB0", 19200);
    nbserial_class GPS_NBSerial;
    nbserial_class eComp_NBSerial;
    const char* wmmCmd = "/usr/bin/acu/WMM_File.exe";
    const char* wmmArg0 = "WMM_File.exe";
    const char* wmmArg1 = "f";
    const char* wmmArg2 = "/usr/bin/acu/terminal_coords.txt";
    const char* wmmArg3 = "/usr/bin/acu/terminal_output.txt";
    const char* wmmArg4 = NULL;
    const char* wmmArgs[5] = {wmmArg0, wmmArg1, wmmArg2, wmmArg3, wmmArg4};
    struct ExtProcStruct wmmExtProcStruct = createExtProcStruct(wmmCmd,wmmArgs);


    // TPM Devices
    struct adaFruitFT232hstruct PowerMeterADC_dev = createFT232Struct();
    adaFruitUSB_SPI_class PowerMeter_ADC;
    struct adaFruitFT232hstruct PowerMeterPLL_dev = createFT232Struct();
    adaFruitUSB_SPI_class PowerMeter_PLL;

    // TxRx Devices
    struct adaFruitFT232hstruct TxRxSPI_dev = createFT232Struct();
    adaFruitUSB_SPI_class TxRxSPI;

    // Log and Config API Devices
    struct devicedatastruct StdIODevice = createDeviceStruct();
    struct logStruct StdLogStruct = createlogStruct();
    struct configStruct StdConfStruct = createconfigStruct();



    // pointer to The instance of Execution system for ccACU application
    OSexecutionSystemClass* theExecutionSystemPtr;

    // Construction of the Application
    ccACU_ApplicationClass(OSexecutionSystemClass* theExecutionSystemPtrIn) :
        // link the ccACU compute module to its ccACU data instance
        // - and all of its exeThread Modules (API and Device Modules)
        ccACU_compMod(  &ccACU_data,
                        &UIServer_exeThread,
                        &SNMPServer_exeThread,
                        &CGIServer_exeThread,
                        &APT_WMM_exeThread,
                        &TPM_exeThread,
                        &TxRx_exeThread,
                        &Modem_exeThread,
                        &Switch_exeThread),



        // link the api compute modules to the ccACU compute module
        UIServer_exeThread(&UIServer_data, &ccACU_compMod),
        SNMPServer_exeThread(&SNMPServer_data, &ccACU_compMod),
        CGIServer_exeThread(&CGIServer_data, &ccACU_compMod),

        // link the device compute modules and the data objects on which they operate
        APT_WMM_exeThread(&((SatComACSStruct*)ccACU_compMod.getModuleDataPtr())->APT, &GPS_NBSerial, &eComp_NBSerial, &wmmExtProcStruct),
        TPM_exeThread(&((SatComACSStruct*)ccACU_compMod.getModuleDataPtr())->TPM, &PowerMeter_PLL, &PowerMeter_ADC),
        TxRx_exeThread(&((SatComACSStruct*)ccACU_compMod.getModuleDataPtr())->TxRx, &TxRxSPI),

        // for future us to think about...
        Modem_exeThread(&ccACU_data.ModemData),
        Switch_exeThread(&ccACU_data.SwitchData),

        GPS_NBSerial(&GPSPortParams),
        eComp_NBSerial(&eCompPortParams),
        PowerMeter_ADC(&PowerMeterADC_dev),
        PowerMeter_PLL(&PowerMeterPLL_dev),
        TxRxSPI(&TxRxSPI_dev),


        // construct the console menu objects
        LCDKeyPadMenuInst(&((SatComACSStruct*)ccACU_compMod.getModuleDataPtr())->LCDKeyPad, &UIServer_exeThread.theMainMenuNode),
        Console1MenuInst(&((SatComACSStruct*)ccACU_compMod.getModuleDataPtr())->ConsoleMenu, &UIServer_exeThread.theMainMenuNode),
        Console2MenuInst(&ccACU_data.ConsoleMenu2, &UIServer_exeThread.theMainMenuNode),

        // create ccNOos exeSys linking for
        // - main process execution of linked compute modules
        // - systick execution of linked compute modules
        // - exception handling execution of linked compute modules
        setupListHead(&ccACU_compMod, nullptr),
        loopListHead(&ccACU_compMod, nullptr),
        systickListHead(nullptr, nullptr),
        exceptionListHead(&ccACU_compMod, nullptr)
    {
        // Link UI Server and Console Menu Objects
        UIServer_data.uiPtrArray[0] = &LCDKeyPadMenuInst;
        UIServer_data.uiPtrArray[1] = &Console1MenuInst;
        UIServer_data.uiPtrArray[2] = &Console2MenuInst;

        // Prevent execution of api modules at ccNOos level - UI Server will handle for the ccACU application
        ((SatComACSStruct*)ccACU_compMod.getModuleDataPtr())->execAPIsMainLoop = ui8FALSE;

        // link the execution system instance
        theExecutionSystemPtr = theExecutionSystemPtrIn;
        // link the execution system module lists
        theExecutionSystemPtr->LinkTheListsHead(
            &setupListHead,
            &loopListHead,
            &systickListHead,
            &exceptionListHead
        );
    }

    // when this is called, the application has been instantiated, the exe system has been instantiated, and the two have been linked together
    // - but, the ccNOos level execution system has not yet begun execution (main proc, main loop nor setup)
    void LinkAndStartExeThreads()
    {
        // Config and Log API device linking
        StdLogStruct.devptr = &StdIODevice;
        StdConfStruct.devptr = &StdIODevice;
        //stdInThread = std::thread(readStdIn, &StdConfStruct.devptr->inbuff.charbuff[0]);

        theExecutionSystemPtr->exeThreadModuleList.emplace_back(&UIServer_exeThread);
        theExecutionSystemPtr->exeThreadModuleList.emplace_back(&SNMPServer_exeThread);
        theExecutionSystemPtr->exeThreadModuleList.emplace_back(&CGIServer_exeThread);
        theExecutionSystemPtr->exeThreadModuleList.emplace_back(&APT_WMM_exeThread);
        theExecutionSystemPtr->exeThreadModuleList.emplace_back(&TPM_exeThread);
        theExecutionSystemPtr->exeThreadModuleList.emplace_back(&TxRx_exeThread);
        theExecutionSystemPtr->exeThreadModuleList.emplace_back(&Modem_exeThread);
        theExecutionSystemPtr->exeThreadModuleList.emplace_back(&Switch_exeThread);

        // Start the exe_thread modules
        theExecutionSystemPtr->exeThreadList.emplace_back(new std::thread(&UI_ServerClass::ThreadExecute, std::ref(UIServer_exeThread)));
        theExecutionSystemPtr->exeThreadList.emplace_back(new std::thread(&SNMP_AgentsAPIServer::ThreadExecute, std::ref(SNMPServer_exeThread)));
        theExecutionSystemPtr->exeThreadList.emplace_back(new std::thread(&CGI_ServerClass::ThreadExecute, std::ref(CGIServer_exeThread)));
        theExecutionSystemPtr->exeThreadList.emplace_back(new std::thread(&APT_WMM_Class::ThreadExecute, std::ref(APT_WMM_exeThread)));
        theExecutionSystemPtr->exeThreadList.emplace_back(new std::thread(&TPM_Class::ThreadExecute, std::ref(TPM_exeThread)));
        theExecutionSystemPtr->exeThreadList.emplace_back(new std::thread(&TxRx_Class::ThreadExecute, std::ref(TxRx_exeThread)));
        theExecutionSystemPtr->exeThreadList.emplace_back(new std::thread(&ModemClass::ThreadExecute, std::ref(Modem_exeThread)));
        theExecutionSystemPtr->exeThreadList.emplace_back(new std::thread(&ManagedSwitchClass::ThreadExecute, std::ref(Switch_exeThread)));
    }


    // ccNOos abstract function wrappers
    void linkAPIioDevices()
    {
        // APT Device setup
        //openComPort(&GPSPortParams);
        //openComPort(&eCompPortParams);
        // links between devices and gps/ecompass structure
        ((SatComACSStruct*)ccACU_compMod.getModuleDataPtr())->APT.GPS.devptr = &GPSPortParams.serialdev;
        ((SatComACSStruct*)ccACU_compMod.getModuleDataPtr())->APT.eCompass.devptr = &eCompPortParams.serialdev;
    }
    UI_8 readGPS()
    {
        GPSPortParams.serialdev.devdata.numbytes2Read = 1;
        GPSPortParams.serialdev.readIndex = 0;

        int iNewLine = 0;
        int tries = 0;
        do {


            if (GPS_NBSerial.ReadDev() == GPSPortParams.serialdev.devdata.numbytes2Read)
            {
                if (GPSPortParams.serialdev.devdata.inbuff.charbuff[GPSPortParams.serialdev.readIndex] == '\n')
                {
                    iNewLine = GPSPortParams.serialdev.readIndex;
                }
                GPSPortParams.serialdev.readIndex += GPSPortParams.serialdev.devdata.numbytes2Read;
            }
            else if (GPSPortParams.serialdev.readIndex > 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                tries++;
            }
            else
                break;


        } while (GPSPortParams.serialdev.readIndex < charBuffMax && iNewLine == 0 && tries < 5);

        GPSPortParams.serialdev.devdata.inbuff.charbuff[iNewLine] = '\0';

        if (iNewLine > 6)
            return ui8TRUE;
        else
            return ui8FALSE;
    }
    UI_8 readEcompass()
    {
        eCompPortParams.serialdev.devdata.numbytes2Read = 1;
        eCompPortParams.serialdev.readIndex = 0;
        int iNewLine = 0;
        int tries = 0;
        do {


            if (eComp_NBSerial.ReadDev() == eCompPortParams.serialdev.devdata.numbytes2Read)
            {
                if (eCompPortParams.serialdev.devdata.inbuff.charbuff[eCompPortParams.serialdev.readIndex] == '\n')
                {
                    iNewLine = eCompPortParams.serialdev.readIndex;
                }
                eCompPortParams.serialdev.readIndex += eCompPortParams.serialdev.devdata.numbytes2Read;
            }
            else if (eCompPortParams.serialdev.readIndex > 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                tries++;
            }
            else
                break;


        } while (eCompPortParams.serialdev.readIndex < charBuffMax && iNewLine == 0 && tries < 5);

        eCompPortParams.serialdev.devdata.inbuff.charbuff[iNewLine] = '\0';

        if (iNewLine > 10)
            return ui8TRUE;
        else
            return ui8FALSE;
    }
    void GetMenuChars()
    {
        //  Aha...GetMenuChars() will not be called if the satcomACS LCD and Console UI structure pointers are nullptr
   //  - so, in the linkapiiodevices() function, we've not linked the satcomACS api devices.
   //  - instead, we are handling the api devices from the ccACU (ccOS) layer
        ;



        // This is called cyclically in the main loop by the satcom acs module
        // - it is called once for each of the two apimodules (console menu and lcdkeypad)
        // - lets feed this from a better place...

        // When overloading in ccOS, feed this from ccOS layer
        // - then feed the ccOS layer from the Platfrom_Main


        // One option is to do nothing, then manage the entire menu from above (the ccOS layer)
        // - this essentially disables the execution of the apiModules at the ccNOos (SatComACS) layer.
        // - this would transfer full responsibility for apiModule execution the ccOS (ccACU) layer.

        // Perhaps ideal to check if the ccACU

        //// if Consolue Menu
        //if (uiStructPtrin->devptr == &ConsoleMenuTCPServerStruct.tcpData.devdata)
        //{
        //    // non-blocking server mainatainance
        //    ConsoleMenu_tcpServer.ShutdownRestartServerSocket();

        //    // non-blocking check for client connection
        //    ConsoleMenu_tcpServer.ServerListenAccept();

        //    // on each client connection, trigger menu write and show help
        //    if (lastTCPConsoleState != ConsoleMenu_tcpServer.getTCPStatus())
        //    {
        //        lastTCPConsoleState = ConsoleMenu_tcpServer.getTCPStatus();
        //        if (lastTCPConsoleState == tcpstat_connected)
        //        {
        //            uiStructPtrin->showHelp = ui8TRUE;
        //            uiStructPtrin->devptr->triggerWriteOperation = ui8TRUE;
        //        }
        //    }

        //    // non-blocking check for client packet
        //    if (ConsoleMenu_tcpServer.ReadDev() > 0)
        //    {
        //        uiStructPtrin->devptr->newDataReadIn = ui8TRUE;
        //        uiStructPtrin->parseIndex = 0;
        //    }
        //}
        //// if LCD KeyPad
        //else if (uiStructPtrin->devptr == &LCDKeyPadPortParams.serialdev.devdata)
        //{
        //    // check for chars from serial port
        //    ;
        //}
    }
};



//ccOS_APP_CLASS




