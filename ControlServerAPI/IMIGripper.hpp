/** \file IMIGripper.hpp
*   \brief Gripper Control API

Notes:
    *** Only include this header once, in the top level file for the compilation unit
    *** A single instance of an execution system class and
    *** A single instance of a gripper fw control module are
    *** Instantiated when this header is included, Data is allocated in RAM

 The execution system implements the core of data and
 execution for compute modules

 The gripper fw control module executes within
 the Data and Execution system

 The apimain() function must be called in its own thread.
 It will never return.
 It will spawn two of its own threads.

*/


#include <cstdint>
#include <array>
#include <thread>





/** \function apimain
*   \brief Main Function for the Gripper Control API

Notes:
    must be run in separate thread, will never return
    Maintains ~100Hz link with Gripper uC
    Maintains data mirror for all Gripper uC Data
    This function will spawn 2 threads
    - 1 for UDP comms with Gripper uC
    - 1 for "systick" time tracking
*/
int apimain();

/** \class IMIGripper
*   \brief The API Class, Set/Get Gripper FW Data

Notes:
    With the apimain function running, a data link is
    maintained and data mirrored.
    This class provides read/write access to
    the mirrored Gripper uC data
*/
class IMIGripper {
public:

    // Main API Thread, started at Construction of IMIGripper Object
    bool newData();
    void clearNewDataFlag();
    bool isConnected();
    std::thread apiThread;
    IMIGripper();

    enum EnumDirection{E_STILL, E_MOVING_UP, E_MOVING_DOWN};
    enum EnumMotorMode
    {
        E_MOTOR_MODE_UNKNOWN    = -2,
        E_MOTOR_MODE_NO_CHANGE  = -1,
        E_MOTOR_MODE_BRAKE      = 0,
        E_MOTOR_MODE_COAST      = 1,
        E_MOTOR_MODE_PWM        = 2,
        E_MOTOR_MODE_VOLTS      = 3,
        E_MOTOR_MODE_VELOCITY   = 4,
        E_MOTOR_MODE_ANGLE      = 5,
        E_MOTOR_MODE_TRAJECTORY = 6
    };

    // The number of motors installed for use with this API
    #define C_INSTALLED_NUM_OF_MOTORS 4
    typedef std::array<EnumDirection, C_INSTALLED_NUM_OF_MOTORS> joints_direction;
    typedef std::array<bool, C_INSTALLED_NUM_OF_MOTORS> joints_moving;
    typedef std::array<bool, C_INSTALLED_NUM_OF_MOTORS> joints_home;
    typedef std::array<EnumMotorMode, C_INSTALLED_NUM_OF_MOTORS> joints_mode;
    typedef float joint_radians; // position in radians, or speed in radians/sec
    typedef std::array<joint_radians, C_INSTALLED_NUM_OF_MOTORS> joints_state;


    /**
     * Any motor flagged in the mask will be stopped.
     * bit 0 (motor 0), bit 1 (motor 1), etc...
     * Unused (not installed) motors flagged in the mask will be ignored.
     * 1: Stop motor
     * 0: ignore
     * Ex: u32Mask = 0b00001111 will stop motors 0-3.
     *     u32Mask = 0b00000101 will stop motors 0 and 2
     * @param u32Mask
     * !! RETURNS IMMEDIATELY !! NON-BLOCKING !!
     * Triggers Stop Operation then returns
     */
    void stop(uint32_t u32Mask);

    /**
     * Close the fingers referenced by the specified mask (ex. 0x000F will close
     * fingers 0-3) with the specified pct of strength from 0.0-1.0.
     * A strength percentage of 0.0 will result in the motors using the minimum
     * force necessary to move the fingers down without sticking. A strength percentage
     * of 1.0 will drive the motors at the highest allowable level without damage
     * to the system.
     * @param u32Mask
     * @param fStrengthPct
     * !! RETURNS IMMEDIATELY !! NON-BLOCKING !!
     * Triggers Close Operation then returns
     */
    void close(uint32_t u32Mask, float fStrengthPct);

    /**
     * Open fingers referenced by the specified mask. Will return fingers
     * referenced by the mask to the HOME position. get_home() method can be
     * used to test whether command was successful.
     * Blocking until complete.
     * @param u32Mask
     * !! RETURNS IMMEDIATELY !! NON-BLOCKING !!
     * Triggers Open Operation then returns
     */
    void open(uint32_t u32Mask);

    /**
     * Command will return immediately but will throw exceptions if there is a
     * problem. Set the flags appropriately to turn ON or OFF the vacuum and
     * to extend or retract the cylinder.
     * @param bVacOn
     * @param bCylinderExtended
     * !! RETURNS IMMEDIATELY !! NON-BLOCKING !!
     * Triggers setVacControls Operation then returns
     */
    void setVacControls(bool bVacOn, bool bCylinderExtended);

    /*************************************************************************
     *  G E T T E R S
     *
     *  NOTE: GETTERS all return immediately and DO NOT throw exceptions that
     *        need to be caught.
     *************************************************************************/

    /**
     * Returns the current direction of each motor.
     * @return
     */
    joints_direction get_direction();

    /**
     * Returns the previous direction of each motor.
     * @return
     */
    joints_direction get_lastDirection();

    /**
     * Returns whether each motor is moving or not.
     * @return
     */
    joints_moving get_moving();

    /**
     * Returns the raw position as reported by the given motor in radians.
     * @return
     */
    joints_state get_positions();

    /**
     * Returns the current velocity of each motor in rad/sec.
     * @return
     */
    joints_state get_velocities();

    /**
     * Returns the current (Amps) of each motor.
     * @return
     */
    joints_state get_current();

    /**
     * Returns the estimated temperature of each motor in degC.
     * @return
     */
    joints_state get_temperature();

    /**
     * Following a successful calibration, returns the max position of each
     * finger. This is the physically lowest detected position of the finger during
     * calibration. By definition, the upper-most position is 0.0 for each finger.
     * @return
     */
    joints_state get_maxPosition();

    /**
     * Returns the current internal IMAX limit for each motor.
     * @return
     */
    joints_state get_motorIMax();

    /**
     * Returns whether each motor is in the HOME position at any given time.
     * @return
     */
    joints_home get_home();

    /**
     * Returns the current internal state of the motor at any given time.
     * @return
     */
    joints_mode get_mode();

    /**
     * Returns whether the vacuum is on or not.
     * @return
     */
    bool isVacOn();

    /**
     * Returns whether the cylinder is extended or not.
     * @return
     */
    bool isCylinderExtended();

    /**
     * Returns whether a part is detected in the suction cup. Before attempting
     * to use PartPresent, the vacuum must be turned on for a minimum of 1 second
     * prior to placing a part in the suction cup. This will provide the necessary
     * time to establish a vac signal baseline to detect parts.
     * @return
     */
    bool isPartPresent();

    /**
     * Returns the current averaged LSbs of the vacuum sensor for detecting
     * part-present.
     * @return
     */
    uint16_t getAverageVacLSbs();

    /**
     * Once the vac is turned on for at least one second, this flag will indicate
     * that a baseline has been established to properly detect parts in the suction
     * cup.
     * @return
     */
    bool isVacBaselineReady();

    /**
     * Returns the baseline calculated.
     * @return
     */
    uint16_t getVacBaseline();

    /**
     * Returns whether the system has been calibrated.
     * @return
     */
    bool isSystemCalibrated();
};

enum SPDSelector
{
    spdNone,
    spdPos0,
    spdPos1,
    spdPos2,
    spdPos3,
    spdEND
};

class GripperSPD
{
private:
    IMIGripper* GripperAPIPtr = nullptr;
    enum SPDSelector GripperVarSelection = spdNone;
public:
    GripperSPD(enum SPDSelector GripperVarSelectionIn, IMIGripper* GripperAPIPtrIn);
    static const char* getSPDLabelString(enum SPDSelector GripperVarSelectionIn);
    float getSPDFloatValue();
    enum SPDSelector getSPDSelector();
};
