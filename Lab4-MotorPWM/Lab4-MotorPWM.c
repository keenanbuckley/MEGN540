/*
         MEGN540 Mechatronics Lab
    Copyright (C) Andrew Petruska, 2021.
       apetruska [at] mines [dot] edu
          www.mechanical.mines.edu
*/

/*
    Copyright (c) 2021 Andrew Petruska at Colorado School of Mines

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

*/

#include "Battery_Monitor.h"
#include "Encoder.h"
#include "Message_Handling.h"  // for translating USB messages to microcontroller tasks
#include "SerialIO.h"          // for USB communication
#include "Task_Management.h"   // for clean task management with functors
#include "Timing.h"            // for Time understanding

// Include Lab Specific Functionality
#include "Lab1_Tasks.h"
#include "Lab2_Tasks.h"
#include "Lab3_Tasks.h"
#include "Lab4_Tasks.h"

// put your global variables (only if absolutely required) here.
// Best to identify them as "static" to make them indentified as internal and start with a "_" to identify as internal.
// Try to initialize them if possible, so their values are never arbitrary.

/**
 * Function Initialize_Modules sets up all hardware and persistant memory necessary
 * for the main loop to funciton properly. It is the first thing main should call and is
 * a convenient way or resetting the system if that is requested.
 *
 * It takes a float as a parameter that is not used to allow it to integrate nicely with the Task_Management Code (which should pass it the seconds since last
 * ran)
 *
 */
void Initialize_Modules( float _time_not_used_ )
{
    // Initialize (reinitialize) all global variables

    // reset USB input buffers
    USB_Flush_Input_Buffer();

    // Initialize all modules except USB (it can only be called once without messing things up)
    Initialize_Timing();
    Initialize_Encoders();
    Initialize_Battery_Monitor();
    Initialize_MotorPWM( 500 );

    // Setup task handling
    Initialize_Task( &task_restart, Initialize_Modules /*function pointer to call*/ );

    // Setup message handling to get processed at some desired rate.
    Initialize_Task( &task_message_handling, Task_Message_Handling );
    Task_Activate( &task_message_handling, 0 );

    Initialize_Task( &task_message_handling_watchdog, Task_Message_Handling_Watchdog );
    Task_Activate( &task_message_handling_watchdog, watchdog_timer );

    // Initialize timing tasks
    Initialize_Task( &task_time_loop, Task_Send_Loop_Time );
    Initialize_Task( &task_send_time, Task_Send_Time_Now );

    // Initialize encoder task
    Initialize_Task( &task_encoder_counts, Send_Encoder_Counts );

    // Initialize battery voltage filter
    Init_Battery_Voltage_Filter();
    Initialize_Task( &task_battery_filter, Update_Battery_Voltage_Filter );
    Task_Activate( &task_battery_filter, 2e-3 );
    Initialize_Task( &task_battery_low, Send_Battery_Low );
    Initialize_Task( &task_check_voltage, Check_Battery_Voltage );
    Task_Activate( &task_check_voltage, 1e-2 );

    // Initialize battery task
    Initialize_Task( &task_battery_voltage, Send_Battery_Voltage );

    // Initialize motor tasks
    Initialize_Task( &task_enable_PWM, Enable_PWM );
    Initialize_Task( &task_disable_PWM, Stop_PWM );
    Initialize_Task( &task_sys_id, Send_Sys_ID );
}

/** Main program entry point. This routine configures the hardware required by the application, then
 *  enters a loop to run the application tasks in sequence.
 */
int main( void )
{
    Initialize_USB();
    Initialize_Modules( 0.0 );

    for( ;; ) {  // yet another way to do while (true)
        Task_USB_Upkeep();

        if( !battery_is_low ) {
            Task_Run_If_Ready( &task_enable_PWM );
        } else {
            // Task_Activate( &task_disable_PWM, -1 );
        }

        Task_Run_If_Ready( &task_encoder_counts );
        Task_Run_If_Ready( &task_sys_id );
        Task_Run_If_Ready( &task_time_loop );
        Task_Run_If_Ready( &task_disable_PWM );
        Task_Run_If_Ready( &task_battery_filter );
        Task_Run_If_Ready( &task_battery_voltage );
        Task_Run_If_Ready( &task_check_voltage );
        Task_Run_If_Ready( &task_battery_low );
        Task_Run_If_Ready( &task_message_handling );
        Task_Run_If_Ready( &task_restart );
        Task_Run_If_Ready( &task_message_handling_watchdog );
    }
}