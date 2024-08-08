
#ifndef CURRENT__H__
#define CURRENT__H__

#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/attribs.h>
#include "nu32dip.h"
#include "ina219.h"
#include "modes.h"

#define SAMPLES 100
#define SIZE 30

void print_current_arrays(int num_samps, const float *desire_array, const float *result_array);
void PI(void);

#endif //__CURRENT__H_