#include "pos.h"

// pos control gains
float Kp = 0, Ki = 0, Kd = 0;

// pos control variables
volatile float u, e, eint, edot, eprev, pos_ref = 0;
extern volatile float current_ref;

// Graphing
volatile int pos_count = 0;
float traj_array[TRAJ_SIZE];
float pos_array[TRAJ_SIZE];
extern char out[SIZE];

int trajectory_size;

// 200Hz ISR for position control
void __ISR(_TIMER_4_VECTOR, IPL4SOFT) PosControl(void) {

    switch (get_mode()) {

        case HOLD: {
            PID();
            break;
        }

        case TRACK: {
            pos_ref = traj_array[pos_count]; // get the position reference
            PID(); // run PID control to reach position

            // termination condition
            if (pos_count > (trajectory_size - 1)) {
                pos_count = trajectory_size - 1;
                pos_ref = traj_array[trajectory_size - 1];
                mode = HOLD;
                break;
            }
            break;
        }

        // Do nothing
        default: {
            break;
        }
    }

    // clear interrupt flag
    IFS0bits.T4IF = 0;
}

// Read the trajectory and save it to traj_array
void read_pos(void) {
    NU32DIP_ReadUART1(out, SIZE);
    sscanf(out, "%d", &trajectory_size);

    for (int j = 0; j < trajectory_size; j++) {
        NU32DIP_ReadUART1(out, SIZE);
        sscanf(out, "%f", &(traj_array[j])); 
    }
}

// void print_pos(void) {
//     sprintf(out, "%d\r\n", trajectory_size);
//     NU32DIP_WriteUART1(out);
//     for (int i = 0; i < trajectory_size; ++i) {
//         sprintf(out, "%f %f\r\n", traj_array[i], pos_array[i]);
//         NU32DIP_WriteUART1(out);
//     }
// }

void PID(void) {
    // Get the encoder value
    WriteUART2("a");
    while (!get_encoder_flag()) {}
    set_encoder_flag(0);
    float pos = get_encoder_count();

    // Calculate error in units of counts
    e = (pos_ref * 1740 / 360.0) - pos;

    // Add bounds to integral term to avoid heavy oscillation
    if (eint > 200) {
        eint = 200;
    }
    else if (eint < -200) {
        eint = -200;
    }

    // Calculate derivative
    edot = (e - eprev) / 0.005;

    // Calculate current signal
    u = Kp * e + Ki * eint + Kd * edot;

    // Save position to array
    pos_array[pos_count] = pos * 360.0 / 1740;

    // Set the new desired current
    current_ref = u;

    // sprintf(out, "%f %f %f %f %f\r\n", pos, e, eint, edot, u);
    // NU32DIP_WriteUART1(out);

    // Second iteration
    eint += e;
    eprev = e;
    pos_count++;
}