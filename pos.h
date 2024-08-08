
#ifndef POS__H__
#define POS__H__

#include <xc.h>
#include <stdio.h>
#include <sys/attribs.h> // __ISR macro
#include "nu32dip.h"
#include "encoder.h"
#include "current.h"
#include "modes.h"

#define TRAJ_SIZE 2000

void PID(void);
// void print_pos(void);
void read_pos(void);

#endif //__POS__H_