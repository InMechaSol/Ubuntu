// Include the Device Agnostic, Cross-Platform Application (DACPA) Library
#include "osApp_Serialization.hpp"
// Include the API Modules from the DACPA Library
#include "UIServerClass.hpp"
#include "FWControlClientClass.hpp"
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
/// - FW Control Module and Data
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

    // FW Control Client api Compute Module
    FWCtrlClient_Class FWCtrlClient_exeThread;
    struct FWCtrlClientStruct FWCtrlClient_data;
    packetsAPIClass PacketsInst;

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
                            &FWCtrlClient_exeThread
                            ),
        // link the api compute modules to the compute module
        UIServer_exeThread(&UIServer_data, &gcControl_compMod),
        // construct the console menu objects - link with ConsoleMenu from gripperFW layer - reuse
        Console1MenuInst(&((ccGripperStruct*)gcControl_compMod.getModuleDataPtr())->ConsoleMenu, &UIServer_exeThread.theMainMenuNode),
        FWCtrlClient_exeThread(&FWCtrlClient_data, &gcControl_compMod),
        // construct the packets api object - link with PacketsAPI from gripperFW layer - reuse
        PacketsInst(&((ccGripperStruct*)gcControl_compMod.getModuleDataPtr())->PacketsAPI),
        // link the device compute modules and the data objects on which they operate
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

        // Prevent execution of api modules at ccNOos level - UI Server will handle for the ccOS application
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
        theExecutionSystemPtr->exeThreadModuleList.emplace_back(&FWCtrlClient_exeThread);
        // Start the exe_thread modules
        theExecutionSystemPtr->exeThreadList.emplace_back(new std::thread(&UI_ServerClass::ThreadExecute, std::ref(UIServer_exeThread)));
        theExecutionSystemPtr->exeThreadList.emplace_back(new std::thread(&FWCtrlClient_Class::ThreadExecute, std::ref(FWCtrlClient_exeThread)));
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

// Include the Device Specific Application Class
#include "IMIGripper.hpp"
// Include the Platform Specific Execution System
#include "Platform_ccOS.hpp"
// Create the Platform and Device Specific Application by linking
// the application and execution system at construction
gcControl_ApplicationClass theApplicationExample(&theExecutionSystem);

////////////////////////////////////////////////////////////////////
// if applicable, define any remaining ccNOos level functions that
// will depend on the application instnace in some way
//
void linkAPIioDevices(struct ccGripperStruct* ccGripperStructPtrIn)
{
    theApplicationExample.linkAPIioDevices();

}
void GetMenuChars(struct uiStruct* uiStructPtrin)
{
    ;
}
void WriteMenuLine(struct uiStruct* uiStructPtrin)
{
    ;

}
void WriteLogLine(struct logStruct* logStructPtrin)
{
    ;
}
void ReadConfigLine(struct configStruct* configStructPtrin)
{
    ;
}

void WritePacketBytes(struct packAPIStruct* packStructPtrin)
{
    ;
}
void ReadPacketBytes(struct packAPIStruct* packStructPtrin)
{
    ;
}

void readMotorData(struct smartMotorStruct* smartMotorStructPtrIn)
{
    ;
}
void writeMotorData(struct smartMotorStruct* smartMotorStructPtrIn)
{
    ;
}
UI_32 getMillis() {
    UI_32 tempepep = theExecutionSystem.getExeDataPtr()->uSecTicks;
    return tempepep/1000; }

//////////////////////////////////////////////////////////////////
// Execution
// Define the version functions using the template
ccOSVersionsTemplate
ccGripperVersionsTemplate
// Define the main function
int apimain()
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

//////////////////////////////////////////////////////////////////
// Finally
// Define the API class

IMIGripper::IMIGripper():apiThread(&apimain){}

bool IMIGripper::isConnected()
{
    return theApplicationExample.FWCtrlClient_exeThread.FWConnected;
}
bool IMIGripper::newData()
{
    return theApplicationExample.FWCtrlClient_exeThread.FreshDataIn;
}
void IMIGripper::clearNewDataFlag()
{
    theApplicationExample.FWCtrlClient_exeThread.FreshDataIn = false;
}


#define FWDataPtr theApplicationExample.gcControl_compMod.getGripperFWDataPtr()
const char* IMIGripper::MotorStatusShortString(int motorIndex)
{
    if(FWDataPtr->SmartMotors[motorIndex].Connected)
    {
        if(FWDataPtr->SmartMotors[motorIndex].StatusOK)
            return "OK";
        else
            return "!!";
    }
    else
        return "NC";
}
unsigned int IMIGripper::getFWTimeStamp()
{
    return FWDataPtr->TimeStamp;
}
void IMIGripper::stop(uint32_t u32Mask)
{
    FWDataPtr->PackageCMDStop = ui8TRUE;
}
void IMIGripper::close(uint32_t u32Mask, float fStrengthPct)
{
    FWDataPtr->PackageCMDClose = ui8TRUE;
}
void IMIGripper::open(uint32_t u32Mask)
{
    FWDataPtr->PackageCMDOpen = ui8TRUE;
}
IMIGripper::joints_state IMIGripper::get_positions()
{
    IMIGripper::joints_state pos;

    for(int i=0; i<C_INSTALLED_NUM_OF_MOTORS; i++)
        pos[i] = FWDataPtr->SmartMotors[i].RotorPositionFbk;

    return pos;
}

///////////////////////////////////////////////////////////////////////////
//
//
enum SPDSelector GripperSPD::getSPDSelector()
{
    return GripperVarSelection;
}

GripperSPD::GripperSPD(enum SPDSelector GripperVarSelectionIn, IMIGripper* GripperAPIPtrIn)
{
    GripperVarSelection = GripperVarSelectionIn;
    GripperAPIPtr =GripperAPIPtrIn;
}
IMIGripper* GripperSPD::getGripperAPIPtr()
{
    return GripperAPIPtr;
}
const char* GripperSPD::getSPDLabelString(enum SPDSelector GripperVarSelectionIn)
{
    switch(GripperVarSelectionIn)
    {
    case spdPos0: return "Position0";
    case spdPos1: return "Position1";
    case spdPos2: return "Position2";
    case spdPos3: return "Position3";
    case spdVel0: return "Velocity0";
    case spdVel1: return "Velocity1";
    case spdVel2: return "Velocity2";
    case spdVel3: return "Velocity3";
    case spdPWM0: return "PWM0";
    case spdPWM1: return "PWM1";
    case spdPWM2: return "PWM2";
    case spdPWM3: return "PWM3";
    case spdCur0: return "Current0";
    case spdCur1: return "Current1";
    case spdCur2: return "Current2";
    case spdCur3: return "Current3";

    default: return "Invalid!";
    }
}
float GripperSPD::getSPDFloatValue()
{
    switch(GripperVarSelection)
    {
    case spdPos0: return FWDataPtr->SmartMotors[0].RotorPositionFbk;
    case spdPos1: return FWDataPtr->SmartMotors[1].RotorPositionFbk;
    case spdPos2: return FWDataPtr->SmartMotors[2].RotorPositionFbk;
    case spdPos3: return FWDataPtr->SmartMotors[3].RotorPositionFbk;
    case spdVel0: return FWDataPtr->SmartMotors[0].RotorVelocityFbk;
    case spdVel1: return FWDataPtr->SmartMotors[1].RotorVelocityFbk;
    case spdVel2: return FWDataPtr->SmartMotors[2].RotorVelocityFbk;
    case spdVel3: return FWDataPtr->SmartMotors[3].RotorVelocityFbk;
    case spdPWM0: return FWDataPtr->SmartMotors[0].MotorPWMCmd;
    case spdPWM1: return FWDataPtr->SmartMotors[1].MotorPWMCmd;
    case spdPWM2: return FWDataPtr->SmartMotors[2].MotorPWMCmd;
    case spdPWM3: return FWDataPtr->SmartMotors[3].MotorPWMCmd;
    case spdCur0: return FWDataPtr->SmartMotors[0].MotorCurrentFbk;
    case spdCur1: return FWDataPtr->SmartMotors[1].MotorCurrentFbk;
    case spdCur2: return FWDataPtr->SmartMotors[2].MotorCurrentFbk;
    case spdCur3: return FWDataPtr->SmartMotors[3].MotorCurrentFbk;

    default: return 0.0;
    }
}

