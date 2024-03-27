#include "Lab4_Tasks.h"

void Enable_PWM( float _time_since_last )
{
    MotorPWM_Enable( true );
    // if( MotorPWM_Get_Left() != 0 && MotorPWM_Get_Right() != 0 ) {

    // } else {
    //     MotorPWM_Enable( false );
    // }
};

void Stop_PWM( float _time_since_last )
{
    // disable PWM
    MotorPWM_Enable( false );
    Task_Cancel( &task_disable_PWM );
}

void Send_Sys_ID( float _time_since_last )
{
    struct __attribute__( ( __packed__ ) ) {
        float time;
        int16_t PWM_L;
        int16_t PWM_R;
        int16_t Encoder_L;
        int16_t Encoder_R;
    } data;
    data.time      = Timing_Get_Time_Sec();
    data.PWM_L     = MotorPWM_Get_Left();
    data.PWM_R     = MotorPWM_Get_Right();
    data.Encoder_L = (int16_t)Encoder_Counts_Left();
    data.Encoder_R = (int16_t)Encoder_Counts_Right();

    char cmd = ( task_sys_id.run_period == -1 ) ? 'q' : 'Q';
    USB_Send_Msg( "cfhhhh", cmd, &data, sizeof( data ) );
}