#include "nu32dip.h"
#include "ina219.h"
#include "encoder.h"
#include "current.h"
#include "pos.h"
#include "modes.h"
#include <stdio.h>

// Gains, errors, and counts for control
extern float Kp, Ki, Kd, Jp, Ji;
extern volatile float e, eprev, edot, eint, current_error_int, pos_ref, current_ref;
extern volatile int current_count, pos_count;
extern float pos_array[TRAJ_SIZE];
extern float traj_array[TRAJ_SIZE];
extern float ref_array[SAMPLES];
extern float curr_array[SAMPLES];
extern int trajectory_size;

int pwm; // PWM control variable for f

void SFR_Startup();

int main(){
    char buffer[SIZE];
    NU32DIP_Startup(); 

    NU32DIP_GREEN = 1;
    NU32DIP_YELLOW = 1;

    // Startup
    __builtin_disable_interrupts();
    UART2_Startup();
    SFR_Startup();
    INA219_Startup();
    __builtin_enable_interrupts();

    while(1){
        NU32DIP_ReadUART1(buffer,SIZE); 
        NU32DIP_YELLOW = 1;
        switch (buffer[0]) {
            case 'x': {
                // test
                int n = 0;
                NU32DIP_ReadUART1(buffer,SIZE);
                sscanf(buffer, "%d", &n);
                sprintf(buffer,"%d\r\n", n + 1); // return the number + 1
                NU32DIP_WriteUART1(buffer);
                break;
            }

            case 'b':  {        
                // read current sensor (mA)
                char m[50];
                sprintf(m, "%f\r\n", INA219_read_current());
                NU32DIP_WriteUART1(m);
                break;
            }

            case 'c': {
                // read encoder (count)
                WriteUART2("a");
                while(!get_encoder_flag()){}
                set_encoder_flag(0);
                char m[50];
                int p = get_encoder_count();
                sprintf(m,"%d\r\n",p);
                NU32DIP_WriteUART1(m);
                break;
            }

            case 'd': {
                // read encoder (deg)
                WriteUART2("a");
                while(!get_encoder_flag()){}
                set_encoder_flag(0);
                char m[50];
                int p = get_encoder_count();
                sprintf(m, "%f\r\n", p * 360.0 / 1740);
                NU32DIP_WriteUART1(m);
                break;
            }

            case 'e': {
                // reset encoder
                WriteUART2("b");
                eint = 0;
                current_error_int = 0;
                eprev = 0;
                edot = 0;
                current_count = 0;
                pos_count = 0;
                pos_ref = 0;
                break;
            }

            case 'f':{
                //set PWM (-100 to 100)

                // Activate PWM
                set_mode(PWM);
                NU32DIP_WriteUART1("\r\nSet PWM (-100 to 100): ");
                NU32DIP_ReadUART1(buffer, SIZE);
                sscanf(buffer,"%d", &pwm);
                sprintf(buffer, "%d\r\n", pwm);
                NU32DIP_WriteUART1(buffer);
                break;
            }

            case 'g': {
                // set current gains
                NU32DIP_WriteUART1("\r\nSet current gains (Jp, Ji): ");
                NU32DIP_ReadUART1(buffer, SIZE);
                sscanf(buffer, "%f %f", &Jp, &Ji);
                sprintf(buffer, "\r\n%f %f\r\n", Jp, Ji);
                NU32DIP_WriteUART1(buffer);
                break;
            }

            case 'h': {
                // read current gains
                sprintf(buffer, "%f %f\r\n", Jp, Ji);
                NU32DIP_WriteUART1(buffer);
                break;
            }

            case 'i': {
                // set position gains
                NU32DIP_WriteUART1("\r\nSet position gains (Kp, Ki, Kd): ");
                NU32DIP_ReadUART1(buffer, SIZE);
                sscanf(buffer, "%f %f %f", &Kp, &Ki, &Kd);
                sprintf(buffer, "\r\n%f %f %f\r\n", Kp, Ki, Kd);
                NU32DIP_WriteUART1(buffer);
                break;
            }

            case 'j': {
                // read position gains
                sprintf(buffer, "%f %f %f\r\n", Kp, Ki, Kd);
                NU32DIP_WriteUART1(buffer);
                break;
            }

            case 'k': {
                // run ITEST
                current_error_int = 0;
                current_count = 0;
                set_mode(ITEST);
                while (get_mode() == ITEST) {}
                print_arrays(SAMPLES, ref_array, curr_array);
                break;
            }

            case 'l': {
                // go to a position
                pos_count = 0;
                eint = 0;
                eprev = 0;
                edot = 0;
                pos_ref = 0;
                current_error_int = 0;

                NU32DIP_WriteUART1("\r\nEnter a position (deg): ");
                NU32DIP_ReadUART1(buffer, SIZE);
                sscanf(buffer, "%f", &pos_ref);
                set_mode(HOLD);
                break;
            }

            case 'm': {
                // load step trajectory
                read_pos();
                break;
            }

            case 'n': {
                // load cubic trajectory
                read_pos();
                break;
            }

            case 'o': {
                // execute trajectory
                eint = 0;
                current_error_int = 0;
                eprev = 0;
                edot = 0;
                current_count = 0;
                pos_count = 0;
                pos_ref = 0;

                set_mode(TRACK);
                while (get_mode() == TRACK) {}

                print_arrays(trajectory_size, traj_array, pos_array);

                // clear arrays
                for (int k = 0; k < trajectory_size; k++) {
                    pos_array[k] = 0;
                }
                break;
            }

            case 'r':{
                //read mode
                sprintf(buffer, "\nCurrent mode of program: %d \r\n", mode);
                NU32DIP_WriteUART1(buffer);
                
                sprintf(buffer,
                    "\nGuide to program modes: \r\n"
                    "IDLE Mode: %d \r\n" 
                    "PWM Mode: %d \r\n"
                    "ITEST Mode: %d \r\n"
                    "HOLD Mode: %d \r\n"
                    "TRACK Mode: %d \r\n\n",
                    IDLE, PWM, ITEST, HOLD, TRACK
                );
                NU32DIP_WriteUART1(buffer);
                break;
            }

            case 'p':{
                //unpower motor
                NU32DIP_WriteUART1("Motor set to IDLE mode.\r\n");
                set_mode(IDLE);
                break;
            }
        
            case 'q': {
                // quit
                set_mode(IDLE);
                break;
            }

            // Command not found
            default: {
                NU32DIP_YELLOW = 0; 
                break;
            }
        }
    }
return 0;
}


void SFR_Startup(void){

    // Motor control B10 (pin 21)
    TRISBbits.TRISB10 = 0; // Set pin B10 as an output
    LATBbits.LATB10 = 0;

    // Timer 2, prescalar is 2, PR is 2000
    T2CONbits.TCKPS = 0b001;
    T2CONbits.T32 = 0;
    PR2 = 2000;
    
    // Timer 3, prescalar is 8, PR is 2000
    T3CONbits.TCKPS = 0b011;
    PR3 = 2000;
    IPC3bits.T3IP = 5;
    IPC3bits.T3IS = 0;

    // Timer 4, prescaler is 16, PR is 25000
    T4CONbits.TCKPS = 0b100;
    T4CONbits.T32 = 0;
    PR4 = 25000;
    IPC4bits.T4IP = 4;
    IPC4bits.T4IS = 0;

    // OC1 for PWM use B15 (pin 26)
    OC1CONbits.ON = 0;
    OC1CONbits.OCM = 0b110;
    OC1CONbits.OCTSEL = 0;
    OC1CONbits.SIDL = 1;
    OC1CONbits.OC32 = 0; 
    OC1R = PR2 / 4;
    OC1RS = PR2;
    RPB15Rbits.RPB15R = 0b0101; // Configure for RPB15

    // Turn on OC1
    OC1CONbits.ON = 1;
    // 20KHz OC PWM
    T2CONbits.ON = 1;
    // 5KHz current control ISR
    T3CONbits.ON = 1;
    // 200Hz position control ISR
    T4CONbits.ON = 1; 

    IEC0bits.T3IE = 1;
    IEC0bits.T4IE = 1;
}