#include "Lab2_Tasks.h"

void Measure_Loop_Time( float _time_since_last )
{
    loop_time_seconds = _time_since_last;
}

void Send_Loop_Time( float _time_since_last )
{
    // start a timer
    Time_t start_time = Timing_Get_Time();

    // Do the thing you need to do.
    float time_loop = loop_time_seconds;

    // calculate time it took to do task
    float delta_time_sec = Timing_Seconds_Since( &start_time );

    float ret_val[2] = { time_loop, delta_time_sec };

    // send response right here if appropriate.
    char cmd = ( task_send_loop_time.run_period == -1 ) ? 't' : 'T';
    USB_Send_Msg( "cff", cmd, &ret_val, sizeof( ret_val ) );
}

void Send_Time_Now( float _time_since_last )
{
    // start a timer
    Time_t start_time = Timing_Get_Time();

    // Do the thing you need to do.
    float time_now = Timing_Get_Time_Sec();

    // calculate time it took to do task
    float delta_time_sec = Timing_Seconds_Since( &start_time );

    float ret_val[2] = { time_now, delta_time_sec };

    // send response right here if appropriate.
    char cmd = ( task_send_time.run_period == -1 ) ? 't' : 'T';
    USB_Send_Msg( "cff", cmd, &ret_val, sizeof( ret_val ) );
}