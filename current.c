#include "current.h"

// arrays for storing data
float ref_array[SAMPLES];
float curr_array[SAMPLES];

extern int pwm; // PWM command

// gains and current trackers
float Jp = 0, Ji = 0;
volatile float current_error, current_error_int = 0;
volatile int current_count = 0;
volatile float _current, current_ref, current;

// output messages
char out[SIZE];

// 5KHz ISR current control
void __ISR(_TIMER_3_VECTOR, IPL5SOFT) CurrentControl(void)
{
    switch (get_mode())
    {
        // shut off motor
        case IDLE: {
            OC1RS = 0;
            break;
        }

        // PWM mode
        case PWM: {
            // consider direction
            if (pwm < 0) {
                LATBbits.LATB10 = 0;
            }
            else {
                LATBbits.LATB10 = 1;
            }
            OC1RS = (unsigned int) (abs(pwm) * PR2 / 100);

            break;
        }

        case ITEST: {

            // Create wave by controlling refernece techology
            if (current_count < 25) {
                current_ref = 200;
            }
            else if (current_count < 50) {
                current_ref = -200;
            }
            else if (current_count < 75) {
                current_ref = 200;
            }
            else if (current_count < 100) {
                current_ref = -200;
            }
            else {
                set_mode(IDLE);
                break;
            }

            PI();
            break;
        }

        case HOLD: {
            current_count = 0;
            PI();
            break;
        }

        case TRACK: {
            current_count = 0;
            PI();
            break;
        }
    }

    IFS0bits.T3IF = 0;
}

// Print current arrays
void print_arrays(int num_samps, const float *desire_array, const float *result_array){
    sprintf(out, "%d\r\n", num_samps);
    NU32DIP_WriteUART1(out);
    for (int i = 0; i < num_samps; ++i) {
        sprintf(out, "%f %f\r\n", desire_array[i], result_array[i]);
        NU32DIP_WriteUART1(out);
    }
}

void PI() {
    
    // Calculate error
    _current = INA219_read_current();
    current_error = current_ref - _current;
    current = Jp * current_error + Ji * current_error_int;

    // Bound output
    if (current > 100.0) {
        current = 100.0;
    } else if (current < -100.0) {
        current = -100.0;
    }

    // Directionality
    if (current < 0) {
        LATBbits.LATB10 = 1;
    } else {
        LATBbits.LATB10 = 0;
    }

    // Set OC1
    OC1RS = (unsigned int) (abs(current) * PR2 / 100);

    // Save to arrays
    curr_array[current_count] = _current;
    ref_array[current_count] = current_ref;

    // Update next iteration
    current_error_int += current_error;
    current_count++;
}