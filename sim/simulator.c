#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "../planner.h"
#include "../stepper.h"
#include "../gcode.h"
#include "../protocol.h"
#include "../nuts_bolts.h"
#include "../nuts_bolts.h"
#include "../settings.h"
#include "../spindle_control.h"
#include "../limits.h"
#include "../runtime.h"
#include "sim_control.h"

// Declare system global variable structure
system_t sys;

uint8_t stepping_ddr;
uint8_t stepping_port;
uint8_t spindle_ddr;
uint8_t spindle_port;

FILE *block_out_file;
FILE *step_out_file;

int main(int argc, char *argv[]) {

//  if(argc!=2) {
//    printf("expecting one arument file name\n");
//    exit(EXIT_FAILURE);
//  }
//  argv++;
//  f= fopen(*argv, "r");
//  if(f==NULL) {
//    printf("error opening file %s\n", *argv);
//    exit(EXIT_FAILURE);
//  }
  
  st_init(); // Setup stepper pins and interrupt timers
  memset(&sys, 0, sizeof(sys));  // Clear all system variables
  settings_reset();
  protocol_init(); // Clear incoming line data
  plan_init(); // Clear block buffer and planner variables
  gc_init(); // Set g-code parser to default state
  spindle_init();
  limits_init();
  st_reset(); // Clear stepper subsystem variables.
  sys.auto_start = true;
  
//  block_out_file= fopen("blocks.txt", "w");
//  step_out_file= fopen("steps.csv", "w");
  //setlinebuf(stdout);
  setvbuf(stdout, NULL, _IONBF, 1);
  setvbuf(stderr, NULL, _IONBF, 1);
  block_out_file= stdout;
  step_out_file= stderr;
  
  protocol_process();
  
  plan_synchronize();

  fclose(block_out_file);
  fclose(step_out_file);
  
  exit(EXIT_SUCCESS);
}