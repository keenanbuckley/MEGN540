#include "MotorPWM.h"

static volatile uint16_t _TOP      = 0;
static volatile int16_t _left_pwm  = 0;
static volatile int16_t _right_pwm = 0;

/**
 * Function Initialize_Motor_PWM initializes the motor PWM on Timer 1 for PWM based voltage control of the motors.
 * The Motor PWM system shall initialize in the disabled state for safety reasons. You should specifically enable
 * Motor PWM outputs only as necessary.
 * @param [uint16_t] MAX_PWM is the maximum PWM value to use. This controls the PWM frequency.
 */
void Initialize_MotorPWM( uint16_t MAX_PWM )
{
    // set WGM1 to 8
    TCCR1B |= ( 1 << WGM13 );

    // use no prescaler
    TCCR1B |= ( 1 << CS10 );

    // enable OC1A and OC1B output drivers
    DDRB |= ( 1 << DDB5 ) | ( 1 << DDB6 );

    MotorPWM_Set_Max( MAX_PWM );
    MotorPWM_Enable( false );
}

/**
 * Function MotorPWM_Enable enables or disables the motor PWM outputs.
 * @param [bool] enable (true set enable, false set disable)
 */
void MotorPWM_Enable( bool enable )
{
    if( enable ) {
        TCCR1A |= ( ( 1 << COM1A1 ) | ( 1 << COM1B1 ) );
    } else {
        TCCR1A &= ~( ( 1 << COM1A1 ) | ( 1 << COM1B1 ) );
        MotorPWM_Set_Left( 0 );
        MotorPWM_Set_Right( 0 );
    }
}

/**
 * Function MotorPWM_Is_Enabled returns if the motor PWM is enabled for output.
 * @param [bool] true if enabled, false if disabled
 */
bool MotorPWM_Is_Enabled()
{
    if( ( TCCR1A & ( ( 1 << COM1A1 ) | ( 1 << COM1B1 ) ) ) == 0 ) {
        return false;
    } else {
        return true;
    }
}

/**
 * Function MotorPWM_Set_Left sets the PWM duty cycle for the left motor.
 * @return [int32_t] The count number.
 */
void MotorPWM_Set_Left( int16_t pwm )
{
    if( pwm != 0 ) {
        if( pwm > 0 ) {
            PORTB &= ~( 1 << PB2 );
        } else {
            PORTB |= ( 1 << PB2 );
        }

        _left_pwm = ( ( abs( pwm ) * _TOP ) / ( 100 - MOTOR_DEADBAND ) ) + ( ( MOTOR_DEADBAND * _TOP ) / 100 );
    } else {
        _left_pwm = 0;
    }
    OCR1B = _left_pwm;
}

/**
 * Function MototPWM_Set_Right sets the PWM duty cycle for the right motor.
 * @return [int32_t] The count number.
 */
void MotorPWM_Set_Right( int16_t pwm )
{
    if( pwm != 0 ) {
        if( pwm > 0 ) {
            PORTB &= ~( 1 << PB1 );
        } else {
            PORTB |= ( 1 << PB1 );
        }

        _right_pwm = ( ( abs( pwm ) * _TOP ) / ( 100 - MOTOR_DEADBAND ) ) + ( ( MOTOR_DEADBAND * _TOP ) / 100 );
    } else {
        _right_pwm = 0;
    }
    OCR1A = _right_pwm;
}

/**
 * Function MotorPWM_Get_Left returns the current PWM duty cycle for the left motor. If disabled it returns what the
 * PWM duty cycle would be.
 * @return [int16_t] duty-cycle for the left motor's pwm
 */
int16_t MotorPWM_Get_Left()
{
    return _left_pwm;
}

/**
 * Function MotorPWM_Get_Right returns the current PWM duty cycle for the right motor. If disabled it returns what the
 * PWM duty cycle would be.
 * @return [int16_t] duty-cycle for the right motor's pwm
 */
int16_t MotorPWM_Get_Right()
{
    return _right_pwm;
}

/**
 * Function MotorPWM_Get_Max() returns the PWM count that corresponds to 100 percent duty cycle (all on), this is the
 * same as the value written into ICR1 as (TOP).
 */
uint16_t MotorPWM_Get_Max()
{
    return ICR1;
}

/**
 * Function MotorPWM_Set_Max sets the maximum pwm count. This function sets the timer counts to zero because
 * the ICR1 can cause undesired behaviors if change dynamically below the current counts.  See page 128 of the
 * atmega32U4 datasheat.
 */
void MotorPWM_Set_Max( uint16_t MAX_PWM )
{
    _TOP = MAX_PWM;
    // unsigned char sreg = SREG;
    // cli();
    // Put value in TOP register
    ICR1 = _TOP;
    // Initialize Timer1
    TCNT1 = 0;
    // SREG  = sreg;
    // sei();
}