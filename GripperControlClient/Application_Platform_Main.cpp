/** \file Application_Platform_Main.cpp
 *  \brief Instantiation and Execution of the Main Application
    \details This is the "main" file.  Here the device specifice
    application instance is constructed and linked with the platform
    specific execution system.

    ccNOos layer functions are defined platform and device specific
    to call instance methods of the application class


*/
// Include the Device Specific Application Class
#include "Application_Platform_Main.hpp"
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

void WritePacketBytes(struct packAPIStruct* uiStructPtrin)
{
    ;
}
void ReadPacketBytes(struct packAPIStruct* uiStructPtrin)
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
    return getuSecTicks()/1000; }
//////////////////////////////////////////////////////////////////
// Finally
// Define the version functions using the template
ccOSVersionsTemplate
ccGripperVersionsTemplate
// Define the main function
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
