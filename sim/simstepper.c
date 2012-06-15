#include <stdio.h>
#include <avr/interrupt.h>
#include "../stepper.h"
#include "../planner.h"

double sim_time= 0.0;

void interrupt_TIMER1_COMPA_vect();

void sim_stepper(FILE *f) {
  block_t *current_block= plan_get_current_block();
  
  while(current_block==plan_get_current_block()) {
    interrupt_TIMER1_COMPA_vect();
    
    // fprintf(f, "%20.15f, %d, %d, %d\n", sim_time, sys.position[X_AXIS], sys.position[Y_AXIS], sys.position[Z_AXIS]);
    
    sim_time+= get_cycles_per_step_event()/F_CPU;
  }
}
