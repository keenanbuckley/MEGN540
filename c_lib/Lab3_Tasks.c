#include "Lab3_Tasks.h"

static float numerator_coeffs[]   = { 0.0002f, 0.0007f, 0.0011f, 0.0007f, 0.0002f };
static float denominator_coeffs[] = { 1.0000f, -3.3441f, 4.2389f, -2.4093f, 0.5175f };
static const uint8_t order        = 4;

float check_voltage_last = 5.0f;

void Init_Battery_Voltage_Filter()
{
    Filter_Init( &battery_filter, numerator_coeffs, denominator_coeffs, order );
    Filter_ShiftBy( &battery_filter, 5 );
    battery_is_low = false;
}

void Check_Battery_Voltage( float _time_since_last )
{
    float voltage = Filter_Last_Output( &battery_filter );
    if( check_voltage_last < 4.8f && voltage && voltage > 3.0 ) {
        if( !battery_is_low ) {
            battery_is_low = true;
            Task_Activate( &task_battery_low, 1.0f );
        }
    } else {
        if( battery_is_low ) {
            battery_is_low = false;
            Task_Cancel( &task_battery_low );
        }
    }
    check_voltage_last = voltage;
}

void Update_Battery_Voltage_Filter( float _time_since_last )
{
    Filter_Value( &battery_filter, Battery_Voltage() );
}

void Send_Battery_Low( float _time_since_last )
{
    float voltage = Filter_Last_Output( &battery_filter );
    struct __attribute__( ( __packed__ ) ) {
        char let[6];
        float volt;
    } msg = { .let = { 'B', 'A', 'T', 'L', 'O', 'W' }, .volt = voltage };
    // Send Warning to Serial that batteries need to be charged
    USB_Send_Msg( "c6sf", '!', &msg, sizeof( msg ) );
    // Task_Activate( &task_battery_voltage, -1 );
    // USB_Send_Msg( "cf", '!', &msg.volt, sizeof( msg.volt ) );
    // USB_Send_Msg( "c7sf", '!', &msg, sizeof( msg ) );
}

void Send_Battery_Voltage( float _time_since_last )
{
    float voltage = Filter_Last_Output( &battery_filter );
    char cmd      = ( task_battery_voltage.run_period == -1 ) ? 'b' : 'B';
    USB_Send_Msg( "cf", cmd, &voltage, sizeof( voltage ) );
}

void Send_Encoder_Counts( float _time_since_last )
{
    // Build a meaningful structure to put your data in. Here we want two floats
    struct __attribute__( ( __packed__ ) ) {
        float left;
        float right;
    } data;

    data.left  = Encoder_Counts_Left();
    data.right = Encoder_Counts_Right();
    char cmd   = ( task_encoder_counts.run_period == -1 ) ? 'e' : 'E';
    USB_Send_Msg( "cff", cmd, &data, sizeof( data ) );
}