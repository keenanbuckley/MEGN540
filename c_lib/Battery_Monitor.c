#include "Battery_Monitor.h"

static const float BITS_TO_BATTERY_VOLTS = 2.0 * 0.005f;

/**
 * Function Initialize_Battery_Monitor initializes the Battery Monitor to record the current battery voltages.
 */
void Initialize_Battery_Monitor()
{

    // *** MEGN540 LAB3 YOUR CODE HERE ***

    // Enable ADC and set ADC prescaler to 128
    ADCSRA |= ( 1 << ADEN ) | ( 1 << ADPS0 ) | ( 1 << ADPS1 ) | ( 1 << ADPS2 );

    // Select ADC6 to read battery voltage
    ADMUX |= ( 1 << MUX1 ) | ( 1 << MUX2 ) | ( 1 << REFS0 );

    // Enable ADC interrupt
    // ADCSRA |= ( 1 << ADIE );
}

/**
 * Function Battery_Voltage initiates the A/D measurement and returns the result for the battery voltage.
 */
float Battery_Voltage()
{
    // A Union to assist with reading the LSB and MSB in the  16 bit register
    union {
        struct {
            uint8_t LSB;
            uint8_t MSB;
        } split;
        uint16_t value;
    } data = { .value = 0 };

    // *** MEGN540 LAB3 YOUR CODE HERE ***
    // initiate analog read
    ADCSRA |= ( 1 << ADSC );

    // block until analog read is finished
    while( bit_is_set( ADCSRA, ADSC ) ) {}

    data.split.LSB = ADCL;
    data.split.MSB = ADCH;

    return data.value * BITS_TO_BATTERY_VOLTS;
}
