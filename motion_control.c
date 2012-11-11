/*
  motion_control.c - high level interface for issuing motion commands
  Part of Grbl

  Copyright (c) 2009-2011 Simen Svale Skogsrud
  Copyright (c) 2011-2012 Sungeun K. Jeon
  Copyright (c) 2011 Jens Geisler
  
  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "config.h"

#include "limits.h"
#include "nuts_bolts.h"
#include "planner.h"
#include "runtime.h"
#include "settings.h"
#include "stepper.h"


// Execute linear motion in absolute millimeter coordinates. Feed rate given in
// millimeters/second unless invert_feed_rate is true. Then the feed_rate means
// that the motion should be completed in (1 minute)/feed_rate time.
// NOTE: This is the primary gateway to the grbl planner. All line motions,
//       including arc line segments, must pass through this routine before
//       being passed to the planner. The separation of mc_line and
//       plan_buffer_line is done primarily to make backlash compensation
//       integration simple and direct.
// NOTE: There will be no backlash compensation, the hardware we're targeting
//       has, by definition, no backlash (it's impossible to go further and
//       then backward when taking an inside cut, for example)
void mc_line(float x, float y, float z, float feed_rate, bool invert_feed_rate) {
  // If the buffer is full: good! That means we are well ahead of the robot. 
  // Remain in this loop until there is room in the buffer.
  do {
    execute_runtime(); // Check for any run-time commands
    if(sys.abort) return; // Bail, if system abort.
  } while (plan_check_full_buffer());

  #ifdef LIMIT_SOFT
    // Clip the move if it falls outside our physical extents
    if(LIMIT_X_NEG_TYPE == LIMIT_TYPE_SOFT && x < LIMIT_X_NEG_VALUE)
      x = LIMIT_X_NEG_VALUE;
    if(LIMIT_X_POS_TYPE == LIMIT_TYPE_SOFT && x > LIMIT_X_POS_VALUE)
      x = LIMIT_X_POS_VALUE;
    if(LIMIT_Y_NEG_TYPE == LIMIT_TYPE_SOFT && y < LIMIT_Y_NEG_VALUE)
      y = LIMIT_Y_NEG_VALUE;
    if(LIMIT_Y_POS_TYPE == LIMIT_TYPE_SOFT && y > LIMIT_Y_POS_VALUE)
      y = LIMIT_Y_POS_VALUE;
    if(LIMIT_Z_NEG_TYPE == LIMIT_TYPE_SOFT && z < LIMIT_Z_NEG_VALUE)
      z = LIMIT_Z_NEG_VALUE;
    if(LIMIT_Z_POS_TYPE == LIMIT_TYPE_SOFT && z > LIMIT_Z_POS_VALUE)
      z = LIMIT_Z_POS_VALUE;
  #endif

  plan_buffer_line(x, y, z, feed_rate, invert_feed_rate);
  
  // Auto-cycle start immediately after planner finishes. Enabled/disabled by
  // grbl settings. During a feed hold, auto-start is disabled momentarily until
  // the cycle is resumed by the cycle-start runtime command.
  // NOTE: This allows the user to decide to exclusively use the cycle start
  // runtime command to begin motion or let grbl auto-start it for them. This is
  // useful when: manually cycle-starting when the buffer is completely full and
  // primed; auto-starting, if there was only one g-code command sent during
  // manual operation; or if a system is prone to buffer starvation, auto-start
  // helps make sure it minimizes any dwelling/motion hiccups and keeps the
  // cycle going.
  if(sys.auto_start) st_cycle_start();
}

// Execute an arc in offset mode format. position == current xyz, target == target xyz, 
// offset == offset from current xyz, axis_XXX defines circle plane in tool space, axis_linear is
// the direction of helical travel, radius == circle radius, isclockwise boolean. Used
// for vector transformation direction.
// The arc is approximated by generating a huge number of tiny, linear segments. The length of each 
// segment is configured in settings.mm_per_arc_segment.  
void mc_arc(float *position, float *target, float *offset, uint8_t axis_0,
    uint8_t axis_1, uint8_t axis_linear, float feed_rate, bool invert_feed_rate,
    float radius, bool isclockwise) {
  float center_axis0 = position[axis_0] + offset[axis_0];
  float center_axis1 = position[axis_1] + offset[axis_1];
  float linear_travel = target[axis_linear] - position[axis_linear];
  float r_axis0 = -offset[axis_0];  // Radius vector from center to current location
  float r_axis1 = -offset[axis_1];
  float rt_axis0 = target[axis_0] - center_axis0;
  float rt_axis1 = target[axis_1] - center_axis1;
  
  // CCW angle between position and target from circle center. Only one atan2() trig computation required.
  float angular_travel = atan2(r_axis0 * rt_axis1 - r_axis1 * rt_axis0, r_axis0 * rt_axis0 + r_axis1 * rt_axis1);
  if(isclockwise) // Correct atan2 output per direction
    if(angular_travel >= 0) angular_travel -= 2 * M_PI;
  else
    if(angular_travel <= 0) angular_travel += 2 * M_PI;
  
  float millimeters_of_travel = hypot(angular_travel * radius, fabs(linear_travel));
  if(!millimeters_of_travel) return;
  uint16_t segments = floor(millimeters_of_travel / settings.mm_per_arc_segment);
  // Multiply inverse feed_rate to compensate for the fact that this movement is approximated
  // by a number of discrete segments. The inverse feed_rate should be correct for the sum of 
  // all segments.
  if(invert_feed_rate) feed_rate *= segments;
 
  float theta_per_segment = angular_travel / segments;
  float linear_per_segment = linear_travel / segments;
  
  /* Vector rotation by transformation matrix: r is the original vector, r_T is the rotated vector,
     and phi is the angle of rotation. Based on the solution approach by Jens Geisler.
         r_T = [cos(phi) -sin(phi);
                sin(phi)  cos(phi] * r ;
     
     For arc generation, the center of the circle is the axis of rotation and the radius vector is 
     defined from the circle center to the initial position. Each line segment is formed by successive
     vector rotations. This requires only two cos() and sin() computations to form the rotation
     matrix for the duration of the entire arc. Error may accumulate from numerical round-off, since
     all float numbers are single precision on the Arduino. (True float precision will not have
     round off issues for CNC applications.) Single precision error can accumulate to be greater than
     tool precision in some cases. Therefore, arc path correction is implemented. 

     Small angle approximation may be used to reduce computation overhead further. This approximation
     holds for everything, but very small circles and large mm_per_arc_segment values. In other words,
     theta_per_segment would need to be greater than 0.1 rad and N_ARC_CORRECTION would need to be large
     to cause an appreciable drift error. N_ARC_CORRECTION~=25 is more than small enough to correct for 
     numerical drift error. N_ARC_CORRECTION may be on the order a hundred(s) before error becomes an
     issue for CNC machines with the single precision Arduino calculations.
     
     This approximation also allows mc_arc to immediately insert a line segment into the planner 
     without the initial overhead of computing cos() or sin(). By the time the arc needs to be applied
     a correction, the planner should have caught up to the lag caused by the initial mc_arc overhead. 
     This is important when there are successive arc motions. 
  */
  // Vector rotation matrix values
  float cos_T = 1 - 0.5 * theta_per_segment * theta_per_segment; // Small angle approximation
  float sin_T = theta_per_segment;
  
  float arc_target[3];
  float sin_Ti;
  float cos_Ti;
  float r_axisi;
  uint16_t i;
  int8_t count = 0;

  // Initialize the linear axis
  arc_target[axis_linear] = position[axis_linear];

  for(i = 1; i < segments; i++) { // Increment (segments-1)
    if(count < N_ARC_CORRECTION) {
      // Apply vector rotation matrix 
      r_axisi = r_axis0*sin_T + r_axis1*cos_T;
      r_axis0 = r_axis0*cos_T - r_axis1*sin_T;
      r_axis1 = r_axisi;
      count++;
    } else {
      // Arc correction to radius vector. Computed only every N_ARC_CORRECTION increments.
      // Compute exact location by applying transformation matrix from initial radius vector(=-offset).
      cos_Ti = cos(i*theta_per_segment);
      sin_Ti = sin(i*theta_per_segment);
      r_axis0 = -offset[axis_0] * cos_Ti + offset[axis_1] * sin_Ti;
      r_axis1 = -offset[axis_0] * sin_Ti - offset[axis_1] * cos_Ti;
      count = 0;
    }

    // Update arc_target location
    arc_target[axis_0] = center_axis0 + r_axis0;
    arc_target[axis_1] = center_axis1 + r_axis1;
    arc_target[axis_linear] += linear_per_segment;
    mc_line(arc_target[X_AXIS], arc_target[Y_AXIS], arc_target[Z_AXIS], feed_rate, invert_feed_rate);
  }
  // Ensure last segment arrives at target location.
  mc_line(target[X_AXIS], target[Y_AXIS], target[Z_AXIS], feed_rate, invert_feed_rate);
}

// Execute dwell in seconds.
void mc_dwell(float seconds) {
   uint16_t i = floor(1000 / DWELL_TIME_STEP * seconds);
   plan_synchronize();
   host_delay_ms(floor(1000 * seconds - i * DWELL_TIME_STEP)); // Delay millisecond remainder
   while(i--) {
     // NOTE: Check and execute runtime commands during dwell every <= DWELL_TIME_STEP milliseconds.
     execute_runtime();
     if(sys.abort) return;
     host_delay_ms(DWELL_TIME_STEP); // Delay DWELL_TIME_STEP increment
   }
}

void mc_go_home() {
  limits_go_home();  
}
