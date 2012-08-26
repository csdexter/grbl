#!/usr/bin/env python
"""\
Stream g-code to grbl controller

This script differs from the simple_stream.py script by 
tracking the number of characters in grbl's serial read
buffer. This allows grbl to fetch the next line directly
from the serial buffer and does not have to wait for a 
response from the computer. This effectively adds another
buffer layer to prevent buffer starvation.

Originally by Sungeun K. Jeon
"""

from collections import defaultdict, deque
import argparse
import serial
import sys
import time


# Make sure this is in sync with CONSOLE_RXBUF_SIZE in host.h, otherwise the
# whole code here is useless
RX_BUFFER_SIZE = 128
# Make sure this is in sync with CONSOLE_BAUD_RATE in host.h, otherwise this
# won't be able to talk to grbl
BAUD_RATE = 9600

def waitOnInputOrBuffer(watermark, stats=False, line=""):
    global g_count, c_line
    
    while sum(c_line) > watermark or s.inWaiting():
        out_temp = s.readline().strip() # Blocking read from grbl
        if not("ok" in out_temp or "error" in out_temp):
            print "\nDebug: ", out_temp # Debug response
        else:
            g_count += 1 # Count processed lines
            c_line.popleft()
            if stats:
                progressStatsOutput(line)

def progressStatsOutput(line):
    sys.stdout.write("TX:%5dL RX:%5dA RB:%3dB(%3d%%)     [%-40s]\r" % (
        l_count, g_count, sum(c_line),
        int(round(sum(c_line) * 100 / RX_BUFFER_SIZE)), line))
    sys.stdout.flush() 

# Define command line argument interface
parser = argparse.ArgumentParser(description="Stream a G-Code file to grbl. "
    "This version assumes the serial port speed is %d baud and grbl's Rx "
    "buffer is %d bytes deep." % (BAUD_RATE, RX_BUFFER_SIZE))
parser.add_argument("gcode_file", type=argparse.FileType("r"),
    help="filename of RS274NGC part program to be streamed")
parser.add_argument("device_file", help="serial port to stream to (e.g. "
    "/dev/ttyS0)")
parser.add_argument("-q","--quiet",action="store_true", default=False, 
    help="suppress progress statistics to console")
args = parser.parse_args()

# Initialize
s = serial.Serial(args.device_file, BAUD_RATE)
f = args.gcode_file
verbose = not args.quiet

# Wake up grbl
print "Initializing grbl..."
s.write("\r\n\r\n")

# Wait for grbl to initialize and flush startup text in serial input
time.sleep(2)
s.flushInput()

# Stream G-Code to grbl
print "Streaming %s to %s at %d baud assuming a %d bytes Rx buffer" % (
    args.gcode_file.name, args.device_file, BAUD_RATE, RX_BUFFER_SIZE)
l_count = 0
g_count = 0
# This being Python, it's acceptable to use a queue instead of a register
c_line = deque([])
if verbose:
    b_histogram = defaultdict(int)

for line in f:
    l_block = line.strip() # Remove end-of-line
    c_line.append(len(l_block) + 1) # This line + LF
    waitOnInputOrBuffer(RX_BUFFER_SIZE - 2, False)
    # Assist the user with tool changes, we assume the file is well formed
    # (i.e. no lower case G-Code)
    if "M06" in l_block or "M6" in l_block:
        # Drain the Rx buffer, then ask the operator to perform the tool change
        # taking all due precautions.
        waitOnInputOrBuffer(len(l_block) + 1, True, "<incoming M06>")
        print ("\nThe next line to be sent contains a tool change command:\n"
            "\t[%-70s]\nPlease wait for buffered movement to complete and then "
            "perform the tool change" % l_block)
        raw_input("Press <Enter> to resume machining after the tool change.")
    s.write(l_block + "\n") # Send block to grbl
    l_count += 1 # Count sent lines
    if verbose:
        progressStatsOutput(l_block[:40])
        b_histogram[sum(c_line)] += 1

# Drain the Rx buffer, make sure only buffered moves (as opposed to buffered
# commands) remain
# NOTE: since this is always a downward slope, it doesn't update the buffer
#       fill histogram
waitOnInputOrBuffer(0, True, "<EOF>")

# Turns out opening the serial port toggles DTR and makes the Arduino reset,
# not closing it so we simply exit with a warning instead of waiting for user
# input.
print "\nG-Code streaming finished!\n"
if verbose:
    print "Rx Buffer usage histogram:"
    for key, value in b_histogram.iteritems():
        print "%3d bytes (%3d%%): %-20s %5d times (%3d%%)" % (
            key, int(round(key * 100 / RX_BUFFER_SIZE)),
            "*" * int(round(value * 20 / l_count)), value,
            int(round(value * 100 / l_count)))
print ("WARNING: Moves may still be buffered in grbl, wait until all CNC "
    "movement stops before powering down and/or touching the machine or "
    "workpiece.")

# Close file and serial port
f.close()
s.close()
