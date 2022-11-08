/** \file Application_Platform_Main.hpp
*   \brief Definitioin of Main application of the Gripper Control Client
    \details Here we define an implementation of the device agnostic
    application and serialization library.  This implementation declares
    use of particular devices:
    1) TCP for the UIServerClass
    2) UDP for the FWControlClientClass

    Begin by including all of the necessary building blocks for
    1) The Device Agnostic, Cross-Platform Application Library
    2) The Cross-Platform IO Devices
        - (TCPdev for UIServerClass)
        - (UDPdev for FWControlClientClass)
    3) Any additional application files required
    *this file should only ever be included by the *.cpp of main compilation unit

    Then Define the structure of the device specifice application.
     - 'class gcControl_ApplicationClass : public ccOSApplicationClass'
     Define linking to platform specific execution system at construction
     - 'gcControl_ApplicationClass(OSexecutionSystemClass* theExecutionSystemPtrIn)'
*/

// Include the Device Agnostic, Cross-Platform Application (DACPA) Library
#include "osApp_Serialization.hpp"
// Include the API Modules from the DACPA Library
#include "ControlServerClass.hpp"
#include "UIServerClass.hpp"
#include "FWControlClientClass.hpp"
#include "IPGatewayAPI.hpp"
// Include the cross-platform tcp_comms library from ccOS
#include "tcp_comms.h"
// Include additional application files as needed
#include <iostream>  // for cin/cout

//////////////////////////////////////////////////////////
/// \class The gcControl_ApplicationClass class
/// \brief Main application class for Gripper Control Service
/// \details All required elements to create a platform specific
/// implementation of the gcControl application and serialization
/// libraries are defined in this class.
///
/// LinkedLists for each execution level of the exeSys are declared
/// and their construction is defined.
///
/// The gcControl application module and data are declared and their
/// construction is defined.
///
/// All modules, data structures, and IO devices are declared and
/// their construction is defined.
/// - UI Server API Module with
///   - the exe_thread and data
///   - 2 instances of console menu class
/// - IP Gateway Module and Data
/// - FW Control Module and Data
/// - UPS Device Module and Data
///
///
/// ///////////////////////////////////////////////////////////
class gcControl_ApplicationClass : public ccOSApplicationClass
{
public:

    // The gcControl Compute Module and Data
    gcControl_Class gcControl_compMod;
    struct gcControlClientStruct gcControl_data;

    // UI api Compute Module    
    UI_ServerClass UIServer_exeThread;
    struct UIServerStruct UIServer_data;
    consoleMenuClass Console1MenuInst;
    consoleMenuClass Console2MenuInst;

    // IP Gateway api Compute Module
    IPGatewayAPI_Class IPGateway_exeThread;
    struct IPGatewayStruct IPGateway_data;

    // FW Control Client api Compute Module
    FWCtrlClient_Class FWCtrlClient_exeThread;
    struct FWCtrlClientStruct FWCtrlClient_data;

    // AI Control Server api Compute Module
    AICtrlServer_Class AICtrlServer_exeThread;
    struct AICtrlServerStruct AICtrlServer_data;

    // UPS Device Module
    UPS_Dev_Class UPS_exeThread;

    // Log and Config API Devices
    struct devicedatastruct StdIODevice = createDeviceStruct();
    struct logStruct StdLogStruct = createlogStruct();
    struct configStruct StdConfStruct = createconfigStruct();

    // Linked Entry Points for execution of module entry points
    linkedEntryPointClass setupListHead;
    linkedEntryPointClass loopListHead;
    linkedEntryPointClass systickListHead;
    linkedEntryPointClass exceptionListHead;

    // pointer to The instance of Execution system for ccACU application
    OSexecutionSystemClass* theExecutionSystemPtr;

    // Construction of the Application
    gcControl_ApplicationClass(OSexecutionSystemClass* theExecutionSystemPtrIn) :
        // link the ccACU compute module to its ccACU data instance
        // - and all of its exeThread Modules (API and Device Modules)
        gcControl_compMod(  &gcControl_data,
                            &UIServer_exeThread,
                            &IPGateway_exeThread,
                            &FWCtrlClient_exeThread,
                            &AICtrlServer_exeThread,
                            &UPS_exeThread
                            ),
        // link the api compute modules to the compute module
        UIServer_exeThread(&UIServer_data, &gcControl_compMod),
        // construct the console menu objects
        Console1MenuInst(&((ccGripperStruct*)gcControl_compMod.getModuleDataPtr())->ConsoleMenu, &UIServer_exeThread.theMainMenuNode),
        Console2MenuInst(&gcControl_data.ConsoleMenu2, &UIServer_exeThread.theMainMenuNode),
        // link the api compute modules to the compute module
        IPGateway_exeThread(&IPGateway_data, &gcControl_compMod),
        FWCtrlClient_exeThread(&FWCtrlClient_data, &gcControl_compMod),
        AICtrlServer_exeThread(&AICtrlServer_data, &gcControl_compMod),

        // link the device compute modules and the data objects on which they operate
        UPS_exeThread(),

        // create ccNOos exeSys linking for
        // - main process execution of linked compute modules
        // - systick execution of linked compute modules
        // - exception handling execution of linked compute modules
        setupListHead(&gcControl_compMod, nullptr),
        loopListHead(&gcControl_compMod, nullptr),
        systickListHead(nullptr, nullptr),
        exceptionListHead(&gcControl_compMod, nullptr)
    {
        // Link UI Server and Console Menu Objects
        UIServer_data.uiPtrArray[0] = &Console1MenuInst;
        UIServer_data.uiPtrArray[1] = &Console2MenuInst;

        // Prevent execution of api modules at ccNOos level - UI Server will handle for the ccACU application
        ((ccGripperStruct*)gcControl_compMod.getModuleDataPtr())->execAPIsMainLoop = ui8FALSE;
        ((ccGripperStruct*)gcControl_compMod.getModuleDataPtr())->execDEVsMainLoop = ui8FALSE;

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
        theExecutionSystemPtr->exeThreadModuleList.emplace_back(&IPGateway_exeThread);
        theExecutionSystemPtr->exeThreadModuleList.emplace_back(&FWCtrlClient_exeThread);
        theExecutionSystemPtr->exeThreadModuleList.emplace_back(&AICtrlServer_exeThread);
        theExecutionSystemPtr->exeThreadModuleList.emplace_back(&UPS_exeThread);

        // Start the exe_thread modules
        theExecutionSystemPtr->exeThreadList.emplace_back(new std::thread(&UI_ServerClass::ThreadExecute, std::ref(UIServer_exeThread)));
        theExecutionSystemPtr->exeThreadList.emplace_back(new std::thread(&IPGatewayAPI_Class::ThreadExecute, std::ref(IPGateway_exeThread)));
        theExecutionSystemPtr->exeThreadList.emplace_back(new std::thread(&FWCtrlClient_Class::ThreadExecute, std::ref(FWCtrlClient_exeThread)));
        theExecutionSystemPtr->exeThreadList.emplace_back(new std::thread(&AICtrlServer_Class::ThreadExecute, std::ref(AICtrlServer_exeThread)));
        theExecutionSystemPtr->exeThreadList.emplace_back(new std::thread(&UPS_Dev_Class::ThreadExecute, std::ref(UPS_exeThread)));
    }


    // ccNOos abstract function wrappers
    void linkAPIioDevices()
    {
        ;
    }
    void GetMenuChars()
    {
        ;
    }
};



//ccOS_APP_CLASS




