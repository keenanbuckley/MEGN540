#include "Lab5_Tasks.h"

#include "Lab4_Tasks.h"

/**
 * @brief update the left and right controllers according to the target speeds/positions
 *
 * @param _time_since_last
 */
void Update_Controller( float _time_since_last )
{
    Skid_Steer_Control_Update( &controller, _time_since_last );
}

/**
 * @brief cancel the update controller task and shutdown pwm
 *
 * @param _time_since_last
 */
void Stop_Controller( float _time_since_last )
{
    Task_Cancel( &task_stop_controller );
    Task_Activate( &task_disable_PWM, -1 );
}