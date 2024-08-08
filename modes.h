
#ifndef MODES__H__
#define MODES__H__

// Define mode types
enum modes
{
    IDLE,
    PWM,
    ITEST,
    HOLD,
    TRACK

} mode;

int get_mode(void);

void set_mode(int m);

#endif