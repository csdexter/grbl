/*
  stepper.c - stepper motor driver: executes motion plans using stepper motors
  Part of Grbl

  Copyright (c) 2009-2011 Simen Svale Skogsrud
  Copyright (c) 2011-2012 Sungeun K. Jeon

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

/* The timer calculations of this module informed by the 'RepRap cartesian
 * firmware' by Zack Smith and Philipp Tiefenbacher. */

#include <stdbool.h>
#include <string.h>

#include "config.h"

#include "stepper.h"
#include "stepper-private.h"

#include "planner.h"
#include "settings.h"


static stepper_t st;
static block_t *current_block;       // A pointer to the block currently being traced
// Used by the stepper driver interrupt
static uint8_t step_pulse_time;      // Step pulse reset time after step rise
static stepper_output_t out_bits;    // The next stepping-bits to be output
static volatile uint8_t busy;        // True when OCIE1A is being serviced. Used to avoid retriggering that handler.
#if STEP_PULSE_DELAY > 0
  static stepper_output_t step_bits; // Stores out_bits output to complete the step pulse delay
#endif

//         __________________________
//        /|                        |\     _________________         ^
//       / |                        | \   /|               |\        |
//      /  |                        |  \ / |               | \       s
//     /   |                        |   |  |               |  \      p
//    /    |                        |   |  |               |   \     e
//   +-----+------------------------+---+--+---------------+----+    e
//   |               BLOCK 1            |      BLOCK 2          |    d
//
//                           time ----->
/* The trapezoid is the shape of the speed curve over time. It starts at
 * block->initial_rate, accelerates by block->rate_delta during the first
 * block->accelerate_until step_events_completed, then keeps going at constant
 * speed until step_events_completed reaches block->decelerate_after after which
 * it decelerates until the trapezoid generator is reset.
 * The slope of acceleration is always +/- block->rate_delta and is applied at
 * a constant rate following the midpoint rule by the trapezoid generator, which
 * is called ACCELERATION_TICKS_PER_SECOND times per second. */
static void set_step_events_per_minute(uint32_t steps_per_minute) {
  host_timer_set_reload(1,
      (HOST_TIMER_FOSC * 60) / (steps_per_minute < MINIMUM_STEPS_PER_MINUTE ?
          MINIMUM_STEPS_PER_MINUTE : steps_per_minute),
      st.cycles_per_step_event);
}

// Stepper state initialization
static void st_wake_up(void) {
  // Initialize stepper output bits
  out_bits.value = settings.invert.masks.stepdir;
  // Initialize step pulse timing from settings. Here to ensure updating after re-writing.
  #if STEP_PULSE_DELAY > 0
    // Set total step pulse time after direction pin set. Ad-hoc computation from oscilloscope.
    step_pulse_time =
        -(((settings.pulse_microseconds + STEP_PULSE_DELAY - 2) * TICKS_PER_MICROSECOND) >> 3);
    // Set delay between direction pin write and step command.
    host_timer_set_compare(2, HOST_TIMER_CHANNEL_A,
        -((settings.pulse_microseconds * TICKS_PER_MICROSECOND) >> 3));
  #else // Normal operation
    // Set step pulse time. Ad-hoc computation from oscilloscope. Uses two's complement.
    step_pulse_time = -(((settings.pulse_microseconds - 2) * TICKS_PER_MICROSECOND) >> 3);
  #endif
  #ifdef STEPPERS_DISABLE
    // Enable steppers by resetting the stepper disable port
    host_gpio_write(STEPPERS_DISABLE, false, HOST_GPIO_MODE_BIT);
  #endif
  // Enable stepper driver interrupt
  host_timer_enable_interrupt(1, HOST_TIMER_INTERRUPT_COMPARE_A);
}

// Stepper shutdown
void st_go_idle(void) {
  // Disable stepper driver interrupt
  host_timer_disable_interrupt(1, HOST_TIMER_INTERRUPT_COMPARE_A);

  #ifdef STEPPERS_DISABLE
    // Force stepper dwell to lock axes for a defined amount of time to ensure the axes come to a complete
    // stop and not drift from residual inertial forces at the end of the last movement.
    #if STEPPER_IDLE_LOCK_TIME > 0
      host_delay_ms(STEPPER_IDLE_LOCK_TIME);
    #endif
    // Disable steppers by setting stepper disable
    host_gpio_write(STEPPERS_DISABLE, true, HOST_GPIO_MODE_BIT);
  #endif
}

// This function determines an acceleration velocity change every CYCLES_PER_ACCELERATION_TICK by
// keeping track of the number of elapsed cycles during a de/ac-celeration. The code assumes that 
// step_events occur significantly more often than the acceleration velocity iterations.
inline static uint8_t iterate_trapezoid_cycle_counter(void) {
  st.trapezoid_tick_cycle_counter += st.cycles_per_step_event;  
  if(st.trapezoid_tick_cycle_counter > CYCLES_PER_ACCELERATION_TICK) {
    st.trapezoid_tick_cycle_counter -= CYCLES_PER_ACCELERATION_TICK;
    return true;
  } else return false;
}          

/* "The Stepper Driver Interrupt" - This timer interrupt is the workhorse of
 * Grbl. It is executed at the rate set with set_step_events_per_minute. It pops
 * blocks from the block_buffer and executes them by pulsing the stepper pins
 * appropriately. It is supported by The Stepper Port Reset Interrupt which it
 * uses to reset the stepper port after each pulse. The Bresenham line tracer
 * algorithm controls all three stepper outputs simultaneously with these two
 *  interrupts. */
HOST_INTERRUPT(host_timer_vector_name(1, HOST_TIMER_INTERRUPT_COMPARE_A)) {
  if(busy) return; // The busy-flag is used to avoid reentering this interrupt

  // Set the direction pins a couple of nanoseconds before we step the steppers
  host_gpio_write(DIR_X, out_bits.flags.dir_x, HOST_GPIO_MODE_BIT);
  host_gpio_write(DIR_Y, out_bits.flags.dir_y, HOST_GPIO_MODE_BIT);
  host_gpio_write(DIR_Z, out_bits.flags.dir_z, HOST_GPIO_MODE_BIT);
  // Then pulse the stepping pins
  #if STEP_PULSE_DELAY > 0
    step_bits = out_bits; // Store out_bits to prevent overwriting.
  #else  // Normal operation
    host_gpio_write(STEP_X, out_bits.flags.step_x, HOST_GPIO_MODE_BIT);
    host_gpio_write(STEP_Y, out_bits.flags.step_y, HOST_GPIO_MODE_BIT);
    host_gpio_write(STEP_Z, out_bits.flags.step_z, HOST_GPIO_MODE_BIT);
  #endif
  // Enable step pulse reset timer so that The Stepper Port Reset Interrupt can
  // reset the signal after exactly settings.pulse_microseconds microseconds,
  // independent of the operation of Timer 1.
  host_timer_set_count(2, step_pulse_time);
  host_timer_set_prescaler(2, host_prescaler_of_divisor(2, 8));

  busy = true;
  // Re-enable interrupts to allow ISR_TIMER2_OVERFLOW to trigger on-time and allow serial communications
  // regardless of time in this handler. The following code prepares the stepper driver for the next
  // step interrupt compare and will always finish before returning to the main program.
  host_sei();
  // If there is no current block, attempt to pop one from the buffer
  if(current_block == NULL) {
    // Anything in the buffer? If so, initialize next motion.
    current_block = plan_get_current_block();
    if(current_block != NULL) {
      if(!sys.feed_hold) {
        // During feed hold, do not update rate and trap counter. Keep decelerating.
        st.trapezoid_adjusted_rate = current_block->initial_rate;
        set_step_events_per_minute(st.trapezoid_adjusted_rate); // Initialize cycles_per_step_event
        st.trapezoid_tick_cycle_counter = CYCLES_PER_ACCELERATION_TICK / 2; // Start halfway for midpoint rule.
      }
      st.min_safe_rate = current_block->rate_delta + (current_block->rate_delta >> 1); // 1.5 x rate_delta
      st.counter_x = -(current_block->step_event_count >> 1);
      st.counter_y = st.counter_x;
      st.counter_z = st.counter_x;
      st.event_count = current_block->step_event_count;
      st.step_events_completed = 0;     
    } else {
      st_go_idle();
      sys.cycle_start = false;
      bit_true(sys.execute, EXEC_CYCLE_STOP); // Flag main program for cycle end
    }    
  } 

  if(current_block != NULL) {
    // Execute step displacement profile by Bresenham's line algorithm
    out_bits.value = current_block->dir_bits.value;
    st.counter_x += current_block->steps_x;
    if(st.counter_x > 0) {
      out_bits.flags.step_x = true;
      st.counter_x -= st.event_count;
      if(out_bits.flags.dir_x) sys.position[X_AXIS]--;
      else sys.position[X_AXIS]++;
    }
    st.counter_y += current_block->steps_y;
    if (st.counter_y > 0) {
      out_bits.flags.step_y = true;
      st.counter_y -= st.event_count;
      if (out_bits.flags.dir_y) sys.position[Y_AXIS]--;
      else sys.position[Y_AXIS]++;
    }
    st.counter_z += current_block->steps_z;
    if (st.counter_z > 0) {
      out_bits.flags.step_z = true;
      st.counter_z -= st.event_count;
      if (out_bits.flags.dir_z) sys.position[Z_AXIS]--;
      else sys.position[Z_AXIS]++;
    }
    
    st.step_events_completed++; // Iterate step events

    // While in block steps, check for de/ac-celeration events and execute them accordingly.
    if(st.step_events_completed < current_block->step_event_count) {
      if(sys.feed_hold) {
        // Check for and execute feed hold by enforcing a steady deceleration
        // from the moment of execution. The rate of deceleration is limited by
        // rate_delta and will never decelerate faster or slower than in normal
        // operation. If the distance required for the feed hold deceleration
        // spans more than one block, the initial rate of the following blocks
        // are not updated and deceleration is continued according to their
        // corresponding rate_delta.
        // NOTE: The trapezoid tick cycle counter is not updated intentionally.
        // This ensures that the deceleration is smooth regardless of where the
        // feed hold is initiated and if the deceleration distance spans
        // multiple blocks.
        if(iterate_trapezoid_cycle_counter()) {
          // If deceleration complete, set system flags and shutdown steppers.
          if(st.trapezoid_adjusted_rate <= current_block->rate_delta) {
            // Just go idle. Do not NULL current block. The bresenham algorithm
            // variables must remain intact to ensure the stepper path is
            // exactly the same. Feed hold is still active and is released after
            // the buffer has been reinitialized.
            st_go_idle();
            sys.cycle_start = false;
            bit_true(sys.execute, EXEC_CYCLE_STOP); // Flag main program that feed hold is complete.
          } else {
            st.trapezoid_adjusted_rate -= current_block->rate_delta;
            set_step_events_per_minute(st.trapezoid_adjusted_rate);
          }      
        }        
      } else {
        // The trapezoid generator always checks step event location to ensure
        // de/ac-celerations are executed and terminated at exactly the right
        // time. This helps prevent over/under-shooting the target position and
        // speed.
        // NOTE: By increasing the ACCELERATION_TICKS_PER_SECOND in config.h,
        // the resolution of the discrete velocity changes increase and accuracy
        // can increase as well to a point. Numerical round-off errors can
        // affect this, if set too high. This is important to note if a user has
        // very high acceleration and/or feedrate requirements for their machine.
        if(st.step_events_completed < current_block->accelerate_until) {
          // Iterate cycle counter and check if speeds need to be increased.
          if(iterate_trapezoid_cycle_counter()) {
            st.trapezoid_adjusted_rate += current_block->rate_delta;
            if(st.trapezoid_adjusted_rate >= current_block->nominal_rate)
              // Reached nominal rate a little early. Cruise at nominal rate until decelerate_after.
              st.trapezoid_adjusted_rate = current_block->nominal_rate;
            set_step_events_per_minute(st.trapezoid_adjusted_rate);
          }
        } else if(st.step_events_completed >= current_block->decelerate_after) {
          // Reset trapezoid tick cycle counter to make sure that the
          // deceleration is performed the same every time. Reset to
          // CYCLES_PER_ACCELERATION_TICK/2 to follow the midpoint rule for an
          // accurate approximation of the deceleration curve.
          if(st.step_events_completed == current_block-> decelerate_after)
            st.trapezoid_tick_cycle_counter = CYCLES_PER_ACCELERATION_TICK / 2;
          else {
            // Iterate cycle counter and check if speeds need to be reduced.
            if(iterate_trapezoid_cycle_counter()) {
              // NOTE: We will only do a full speed reduction if the result is
              // more than the minimum safe rate, initialized in trapezoid reset
              // as 1.5 x rate_delta. Otherwise, reduce the speed by half
              // increments until finished. The half increments are guaranteed
              // not to exceed the CNC acceleration limits, because they will
              // never be greater than rate_delta. This catches small errors
              // that might leave steps hanging after the last trapezoid tick or
              // a very slow step rate at the end of a full stop deceleration in
              // certain situations. The half rate reductions should only be
              // called once or twice per block and create a nice smooth end
              // deceleration.
              if (st.trapezoid_adjusted_rate > st.min_safe_rate)
                st.trapezoid_adjusted_rate -= current_block->rate_delta;
              else st.trapezoid_adjusted_rate >>= 1; // Bit shift divide by 2
              if (st.trapezoid_adjusted_rate < current_block->final_rate)
                // Reached final rate a little early. Cruise to end of block at final rate.
                st.trapezoid_adjusted_rate = current_block->final_rate;
              set_step_events_per_minute(st.trapezoid_adjusted_rate);
            }
          }
        } else {
          // No accelerations. Make sure we cruise exactly at the nominal rate.
          if (st.trapezoid_adjusted_rate != current_block->nominal_rate) {
            st.trapezoid_adjusted_rate = current_block->nominal_rate;
            set_step_events_per_minute(st.trapezoid_adjusted_rate);
          }
        }
      }            
    } else {   
      // If current block is finished, reset pointer 
      current_block = NULL;
      plan_discard_current_block();
    }
  }
  out_bits.value ^= settings.invert.masks.stepdir;  // Apply stepper invert mask
  busy = false;
}

// This interrupt is set up by ISR_TIMER1_COMPAREA when it sets the motor port
// bits. It resets the motor port after a short period
// (settings.pulse_microseconds) completing one step cycle.
// TODO: It is possible for the serial interrupts to delay this interrupt by a
// few microseconds, if they execute right before this interrupt. Not a big
// deal, but could use some TLC at some point.
HOST_INTERRUPT(host_timer_vector_name(2, HOST_TIMER_INTERRUPT_OVERFLOW)) {
  // Reset stepping pins (leave the direction pins)
  host_gpio_write(STEP_X, settings.invert.flags.step_x, HOST_GPIO_MODE_BIT);
  host_gpio_write(STEP_Y, settings.invert.flags.step_y, HOST_GPIO_MODE_BIT);
  host_gpio_write(STEP_Z, settings.invert.flags.step_z, HOST_GPIO_MODE_BIT);
  host_timer_set_prescaler(2, host_prescaler_of_divisor(2, 0)); // Disable Timer 2 to prevent re-entering this interrupt when it's not needed.
}

#if STEP_PULSE_DELAY > 0
  // This interrupt is used only when STEP_PULSE_DELAY is enabled. Here, the
  // step pulse is initiated after the STEP_PULSE_DELAY time period has elapsed.
  // The ISR TIMER2_OVF interrupt will then trigger after the appropriate
  // settings.pulse_microseconds, as in normal operation.
  // The new timing between direction, step pulse, and step complete events are
  // setup in the st_wake_up() routine.
  HOST_INTERRUPT(host_timer_vector_name(2, HOST_TIMER_INTERRUPT_COMPARE_A)) {
    host_gpio_write(STEP_X, step_bits.flags.step_x, HOST_GPIO_MODE_BIT);
    host_gpio_write(STEP_Y, step_bits.flags.step_y, HOST_GPIO_MODE_BIT);
    host_gpio_write(STEP_Z, step_bits.flags.step_z, HOST_GPIO_MODE_BIT);
  }
#endif

// Reset and clear stepper subsystem variables
void st_reset(void) {
  memset(&st, 0, sizeof(st));
  set_step_events_per_minute(MINIMUM_STEPS_PER_MINUTE);
  current_block = NULL;
  busy = false;
}

// Initialize and start the stepper motor subsystem
void st_init(void) {
  host_gpio_direction(STEP_X, HOST_GPIO_DIRECTION_OUTPUT, HOST_GPIO_MODE_BIT);
  host_gpio_direction(STEP_Y, HOST_GPIO_DIRECTION_OUTPUT, HOST_GPIO_MODE_BIT);
  host_gpio_direction(STEP_Z, HOST_GPIO_DIRECTION_OUTPUT, HOST_GPIO_MODE_BIT);
  host_gpio_direction(DIR_X, HOST_GPIO_DIRECTION_OUTPUT, HOST_GPIO_MODE_BIT);
  host_gpio_direction(DIR_Y, HOST_GPIO_DIRECTION_OUTPUT, HOST_GPIO_MODE_BIT);
  host_gpio_direction(DIR_Z, HOST_GPIO_DIRECTION_OUTPUT, HOST_GPIO_MODE_BIT);
  host_gpio_write(STEP_X, settings.invert.flags.step_x, HOST_GPIO_MODE_BIT);
  host_gpio_write(STEP_Y, settings.invert.flags.step_y, HOST_GPIO_MODE_BIT);
  host_gpio_write(STEP_Z, settings.invert.flags.step_z, HOST_GPIO_MODE_BIT);
  host_gpio_write(DIR_X, settings.invert.flags.dir_x, HOST_GPIO_MODE_BIT);
  host_gpio_write(DIR_Y, settings.invert.flags.dir_y, HOST_GPIO_MODE_BIT);
  host_gpio_write(DIR_Z, settings.invert.flags.dir_z, HOST_GPIO_MODE_BIT);

  // Configure Timer 1
  host_timer_set_prescaler(1, host_prescaler_of_divisor(1, 0)); // Prescaler is set later, when timer is started
  host_timer_enable_ctc(1);

  // Configure Timer 2
  host_timer_set_prescaler(2, host_prescaler_of_divisor(2, 0)); // Disable timer until needed.
  host_timer_enable_interrupt(2, HOST_TIMER_INTERRUPT_OVERFLOW);
  #if STEP_PULSE_DELAY > 0
    host_timer_enable_interrupt(2, HOST_TIMER_INTERRUPT_COMPARE_A);
  #endif
  host_register_interrupt(host_timer_vector_name(1, HOST_TIMER_INTERRUPT_COMPARE_A));
  host_register_interrupt(host_timer_vector_name(2, HOST_TIMER_INTERRUPT_OVERFLOW));
  #if STEP_PULSE_DELAY > 0
    host_register_interrupt(host_timer_vector_name(2, HOST_TIMER_INTERRUPT_COMPARE_A));
  #endif
  // Start in the idle state
  st_go_idle();

}

// Planner external interface to start stepper interrupt and execute the blocks
// in queue. Called by the main program functions: planner auto-start and
// run-time command execution.
void st_cycle_start(void) {
  if(!(sys.cycle_start || sys.feed_hold)) {
    sys.cycle_start = true;
    st_wake_up();
  }
}

// Execute a feed hold with deceleration, only during cycle. Called by main program.
void st_feed_hold(void) {
  if (!sys.feed_hold && sys.cycle_start) {
    sys.auto_start = false; // Disable planner auto start upon feed hold.
    sys.feed_hold = true;
  }
}

// Reinitializes the cycle plan and stepper system after a feed hold for a
// resume. Called by runtime command execution in the main program, ensuring
// that the planner re-plans safely.
// NOTE: Bresenham algorithm variables are still maintained through both the
// planner and stepper cycle reinitializations. The stepper path should continue
// exactly as if nothing has happened. Only the planner de/ac-celerations
// profiles and stepper rates have been updated.
void st_cycle_reinitialize(void) {
  if(current_block != NULL) {
    // Replan buffer from the feed hold stop location.
    plan_cycle_reinitialize(current_block->step_event_count - st.step_events_completed);
    // Update initial rate and timers after feed hold.
    st.trapezoid_adjusted_rate = 0; // Resumes from rest
    set_step_events_per_minute(st.trapezoid_adjusted_rate);
    st.trapezoid_tick_cycle_counter = CYCLES_PER_ACCELERATION_TICK / 2; // Start halfway for midpoint rule.
    st.step_events_completed = 0;
  }
  sys.feed_hold = false; // Release feed hold. Cycle is ready to re-start.
}
