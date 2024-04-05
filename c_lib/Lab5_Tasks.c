#include "Lab5_Tasks.h"

/**
 * @brief update the left and right controllers according to the target speeds/positions
 *
 * @param _time_since_last
 */
void Update_Controller( float _time_since_last )
{
    Skid_Steer_Control_Update( &controller, _time_since_last );
}