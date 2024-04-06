#include "Skid_Steer_Controller.h"

static const float COUNTS_TO_RADIANS = 2 * M_PI / 909.7;

/**
 * @brief Initialize_Skid_Steer initializes the skid steer object thats statically created in the c file
 *
 * @param z_transform_numerator the controller's z-transform numerator
 * @param z_transform_denominator the controller's z-transform denominator
 * @param z_transform_order the controller's order
 * @param descritization_period the period used to descritize the z-transform coefficients
 * @param error_to_control_gain the conversion from error (counts? or rad?) to control (pwm?) for the controller
 * @param max_abs_control the absolute valued maximum control for satruation in control units (pwm?)
 * @param wheel_base_width the axel-width between treds to help convert from left-right imballance to car rotation
 * @param wheel_diameter  the diameter of the wheels to convert from wheel rotation to translation
 *
 * // Optional function pointers to assist with making this code more generic
 * @param measurement_left_fcn_ptr a function pointer to the left-side measurement
 * @param measurement_right_fcn_ptr a function pointer to the right-side measurement
 * @param control_left_fcn_ptr a function pointer to the left side's control application
 * @param control_right_fcn_ptr a founction pointer to the right side's control applicaion
 */
void Initialize_Skid_Steer( Skid_Steer_Controller_t* p_skid_steer, float* z_transform_numerator, float* z_transform_denominator, uint8_t z_transform_order,
                            float descritization_period, float error_to_control_gain, int16_t max_abs_control, float wheel_base_width, float wheel_diameter,
                            int32_t ( *measurement_left_fcn_ptr )( void ), int32_t ( *measurement_right_fcn_ptr )( void ),
                            void ( *control_left_fcn_ptr )( int16_t ), void ( *control_right_fcn_ptr )( int16_t ) )
{
    Initialize_Controller( &p_skid_steer->controller_left, error_to_control_gain, z_transform_numerator, z_transform_denominator, z_transform_order,
                           descritization_period );
    Initialize_Controller( &p_skid_steer->controller_right, error_to_control_gain, z_transform_numerator, z_transform_denominator, z_transform_order,
                           descritization_period );

    p_skid_steer->conversion_speed_to_control = 1 / ( 0.5 * COUNTS_TO_RADIANS * wheel_diameter );
    p_skid_steer->wheel_base_width            = wheel_base_width;
    p_skid_steer->max_abs_control             = max_abs_control;

    p_skid_steer->measurement_left_fcn_ptr  = measurement_left_fcn_ptr;
    p_skid_steer->measurement_right_fcn_ptr = measurement_right_fcn_ptr;
    p_skid_steer->control_left_fcn_ptr      = control_left_fcn_ptr;
    p_skid_steer->control_right_fcn_ptr     = control_right_fcn_ptr;
}

/**
 * @brief Skid_Steer_Command_Displacement sets a new target diplacment for the robot to execute. This is a relative displacment to the current position, not an
 * absolute target.
 *
 * @param linear The arc-length to travel (m)
 * @param angular The angle to rotate (rad)
 */
void Skid_Steer_Command_Displacement( Skid_Steer_Controller_t* p_skid_steer, float linear, float angular )
{
    float leftM    = linear - ( p_skid_steer->wheel_base_width * 0.5 * angular );
    float rightM   = linear + ( p_skid_steer->wheel_base_width * 0.5 * angular );
    float leftEnc  = leftM * p_skid_steer->conversion_speed_to_control;
    float rightEnc = rightM * p_skid_steer->conversion_speed_to_control;
    Controller_Set_Target_Position( &p_skid_steer->controller_left, rightEnc + p_skid_steer->measurement_right_fcn_ptr() );
    Controller_Set_Target_Position( &p_skid_steer->controller_right, leftEnc + p_skid_steer->measurement_left_fcn_ptr() );
}

/**
 * @brief Skid_Steer_Command_Velocity sets a target velocity for the skid-steer system to execute
 *
 * @param linear The tangential velocity to move at (m/s)
 * @param angular The angular rate to spin at (rad/s)
 */
void Skid_Steer_Command_Velocity( Skid_Steer_Controller_t* p_skid_steer, float linear, float angular )
{
    float velR = linear + ( p_skid_steer->wheel_base_width * 0.5 * angular );
    float velL = linear - ( p_skid_steer->wheel_base_width * 0.5 * angular );

    Controller_Set_Target_Velocity( &p_skid_steer->controller_right, velR * p_skid_steer->conversion_speed_to_control );
    Controller_Set_Target_Velocity( &p_skid_steer->controller_left, velL * p_skid_steer->conversion_speed_to_control );
}

/**
 * @brief Skid_Steer_Control_Update executes a control update by comparing current measurments desired values and implements new control setpoints.
 *
 */
void Skid_Steer_Control_Update( Skid_Steer_Controller_t* p_skid_steer, float ellapsed_time )
{
    // take in measurements
    float measurement_left  = p_skid_steer->measurement_left_fcn_ptr();
    float measurement_right = p_skid_steer->measurement_right_fcn_ptr();

    // update controllers
    int16_t setpoint_left  = Controller_Update( &p_skid_steer->controller_left, measurement_left, ellapsed_time );
    int16_t setpoint_right = Controller_Update( &p_skid_steer->controller_right, measurement_right, ellapsed_time );

    // set new setpoints
    p_skid_steer->control_left_fcn_ptr( SaturateInt( setpoint_left, p_skid_steer->max_abs_control ) );
    p_skid_steer->control_right_fcn_ptr( SaturateInt( setpoint_right, p_skid_steer->max_abs_control ) );
}