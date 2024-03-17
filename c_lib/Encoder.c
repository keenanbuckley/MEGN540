#include "Encoder.h"

static const float COUNTS_TO_RADIANS = 2 * M_PI / 909.7;

/**
 * Internal counters for the Interrupts to increment or decrement as necessary.
 */
static volatile bool _last_right_A = 0;  // Static limits it's use to this file
static volatile bool _last_right_B = 0;  // Static limits it's use to this file

static volatile bool _last_left_A   = 0;  // Static limits it's use to this file
static volatile bool _last_left_B   = 0;  // Static limits it's use to this file
static volatile bool _last_left_XOR = 0;  // Necessary to check if PB4 triggered the ISR or not

static volatile int32_t _left_counts  = 0;  // Static limits it's use to this file
static volatile int32_t _right_counts = 0;  // Static limits it's use to this file

/** Helper Funcions for Accessing Bit Information */
// *** MEGN540 Lab 3 TODO ***
// Hint, use avr's bit_is_set function to help
static inline bool Right_XOR()
{
    return bit_is_set( PINE, PINE6 );
}  // MEGN540 Lab 3 TODO
static inline bool Right_B()
{
    return bit_is_set( PINF, PINF0 );
}  // MEGN540 Lab 3 TODO
static inline bool Right_A()
{
    return Right_XOR() ^ Right_B();
}  // MEGN540 Lab 3 TODO

static inline bool Left_XOR()
{
    return bit_is_set( PINB, PINB4 );
}  // MEGN540 Lab 3 TODO
static inline bool Left_B()
{
    return bit_is_set( PINE, PINE2 );
}  // MEGN540 Lab 3 TODO
static inline bool Left_A()
{
    return Left_XOR() ^ Left_B();
}  // MEGN540 Lab 3 TODO

/**
 * Function Encoders_Init initializes the encoders, sets up the pin change interrupts, and zeros the initial encoder
 * counts.
 */
void Initialize_Encoders()
{
    // *** MEGN540 Lab3 ***
    // YOUR CODE HERE

    // Left encoder uses PB4 and PE2 pins as digital inputs. External interrupt PCINT4 is necessary to detect
    // the change in XOR flag. You'll need to see Section 11.1.5 - 11.1.7 for setup and use.
    // Note that the PCINT interrupt is trigered by any PCINT pin. In the ISR you should check to make sure
    // the interrupt triggered is the one you intend on processing.

    // Right encoder uses PE6 adn PF0 as digital inputs. External interrupt INT6 is necessary to detect
    // the changes in XOR flag. You'll need to see Sections 11.1.2-11.1.4 for setup and use.
    // You'll use the INT6_vect ISR flag.

    // Initialize static file variables. These probably need to be updated.
    _last_right_A = 0;  // MEGN540 Lab 3 TODO
    _last_right_B = 0;  // MEGN540 Lab 3 TODO

    _last_left_A   = 0;  // MEGN540 Lab 3 TODO
    _last_left_B   = 0;  // MEGN540 Lab 3 TODO
    _last_left_XOR = 0;  // MEGN540 Lab 3 TODO

    _left_counts  = 0;  // MEGN540 Lab 3 TODO
    _right_counts = 0;  // MEGN540 Lab 3 TODO

    // RIGHT ENCODER
    // Enable interrupt mask for right encoder
    EIMSK |= ( 1 << INT6 );
    // Triggers on any right encoder XOR change
    EICRB |= ( 1 << ISC60 );
    // Enable interrupt flag for right encoder
    EIFR |= ( 1 << INTF6 );

    // LEFT ENCODER
    // Enable pin change interrupt
    PCICR |= ( 1 << PCIE0 );
    // Enable interrupt flag
    PCIFR |= ( 1 << PCIF0 );
    // Interrupt mask for pin 4
    PCMSK0 |= ( 1 << PCINT4 );

    sei();  // enables interrupts
}

/**
 * Function Encoder_Counts_Left returns the number of counts from the left encoder.
 * @return [int32_t] The count number.
 */
int32_t Encoder_Counts_Left()
{
    // *** MEGN540 Lab3 ***
    // YOUR CODE HERE
    // Note: Interrupts can trigger during a function call and an int32 requires
    // multiple clock cycles to read/save. You may want to stop interrupts, copy the value,
    // and re-enable interrupts to prevent this from corrupting your read/write.

    // Disable pin change interrupt
    cli();  // PCICR &= ~( 1 << PCIE0 );
    // get encoder counts
    int32_t left_counts = _left_counts;
    // Enable pin change interrupt
    sei();  // PCICR |= ( 1 << PCIE0 );
    return left_counts;
}

/**
 * Function Encoder_Counts_Right returns the number of counts from the right encoder.
 * @return [int32_t] The count number.
 */
int32_t Encoder_Counts_Right()
{
    // *** MEGN540 Lab3 ***
    // YOUR CODE HERE
    // Note: Interrupts can trigger during a function call and an int32 requires
    // multiple clock cycles to read/save. You may want to stop interrupts, copy the value,
    // and re-enable interrupts to prevent this from corrupting your read/write.

    // Disable pin change interrupt
    cli();  // EIMSK &= ~( 1 << INT6 );
    // get encoder counts
    int32_t right_counts = _right_counts;
    // Enable pin change interrupt
    sei();  // EIMSK |= ( 1 << INT6 );
    return right_counts;
}

/**
 * Function Encoder_Rad_Left returns the number of radians for the left encoder.
 * @return [float] Encoder angle in radians
 */
float Encoder_Rad_Left()
{
    // *** MEGN540 Lab3 ***
    // YOUR CODE HERE.  How many counts per rotation???
    return Encoder_Counts_Left() * COUNTS_TO_RADIANS;
}

/**
 * Function Encoder_Rad_Right returns the number of radians for the left encoder.
 * @return [float] Encoder angle in radians
 */
float Encoder_Rad_Right()
{
    // *** MEGN540 Lab3 ***
    // YOUR CODE HERE.  How many counts per rotation???
    return Encoder_Counts_Right() * COUNTS_TO_RADIANS;
}

union enc_data {
    struct {
        unsigned currB : 1;
        unsigned currA : 1;
        unsigned lastB : 1;
        unsigned lastA : 1;
    } split;
    uint8_t value;
};

/**
 * Interrupt Service Routine for the right Encoder.
 * @return
 */
ISR( INT6_vect )
{
    union enc_data data;
    data.value       = 0;
    data.split.lastA = _last_right_A;
    data.split.lastB = _last_right_B;
    data.split.currB = Right_B();
    data.split.currA = Right_XOR() ^ data.split.currB;

    switch( data.value ) {
        case 0b1011:
        case 0b1101:
        case 0b0100:
        case 0b0010:
            /* forward case */
            _right_counts++;
            break;

        case 0b1110:
        case 0b0111:
        case 0b0001:
        case 0b1000:
            /* backward case */
            _right_counts--;
            break;

        // other cases do not change encoder ticks
        default: break;
    }

    _last_right_A = data.split.currA;
    _last_right_B = data.split.currB;
}

/**
 * Interrupt Service Routine for the left Encoder. Note: May need to check that it is actually PCINT4 that triggered, as
 * the Pin Change Interrupts can trigger for multiple pins.
 * @return
 */
ISR( PCINT0_vect )
{
    bool left_xor = Left_XOR();
    if( _last_left_XOR != left_xor ) {
        union enc_data data;
        data.value       = 0;
        data.split.lastA = _last_left_A;
        data.split.lastB = _last_left_B;
        data.split.currB = Left_B();
        data.split.currA = data.split.currB ^ left_xor;

        switch( data.value ) {
            case 0b1011:
            case 0b1101:
            case 0b0100:
            case 0b0010: _left_counts++; break;

            case 0b1110:
            case 0b0111:
            case 0b0001:
            case 0b1000: _left_counts--; break;
        }
        _last_left_A   = data.split.currA;
        _last_left_B   = data.split.currB;
        _last_left_XOR = left_xor;
    }
}