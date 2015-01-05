# GPIO_Platform

Turning an Arduino Due into a multi-channel data logger and signal generator driven via USB.

## Summary

This firmware turns the Arduino Due into a multi-channel data logger and signal generator. The goal is to allow the host to do what it is good at (number crunching, FFT, displaying GUIs, data storage, that sort of boring stuff) while utilizing what the Arduino Due is really good for: an incredibly cheap, yet powerful., sensor and actuator platform.

This is a firmware that allows the periodic sampling of IO channels on the Due, and to report the values to the host. You can easily set up to have a port monitored at 100 Hz, for example. Or to report (interrupt-driven) whenever a specific digital PIN changes state, and the time between ticks. It supports several very simple filters for reporting data (averaging, delta values, etc) that are discussed in the command reference. The host doesn't have to poll, but can just set up a service for reporting data. (Of course, the ability to poll single reads is there.)

It also serves as a signal generator. The host doesn't have to supply a constant stream of writes; it can set up that a channel shall be periodically cycled through, say, a sine curve for analog outputs, flipping state for digital output pins ever so often, or a number of other things.

There is also a simple serial monitor for easy debugging by humans.

## Warnings

This code is in its infancy. I wrote it to become more acquainted with
the limitations and requirements on the Arduino Due platform. It is only
a few days old and published now only because my New Year's leave is
over. I *very much* welcome contributions, but if you're going to use
this for production usage at this point in time, please be very, very
careful. It's bound to change quite a bit.

# Installation

1. It is assumed that you have an Arduino Due and the Arduino environment installed, and are basically familiar with its operation.
1. [Install DueTimer](https://raw.githubusercontent.com/ivanseidel/DueTimer/master/README.md) from GitHub according to Ivan's instructions. GPIO_Platform uses DueTimer for it's scheduling.
1. [Download GPIO_Platform](https://github.com/l-mb/GPIO_Platform/releases) from GitHub
1. Unzip the files and install into a sketch folder named *GPIO_Platform*
1. Open the sketch via Arduino and verify, then download, the sketch to the Due.

# Usage

GPIO_Platform is quite simple to use. Once it has booted, it awaits
commands on the native USB port of the Due via serial. (The native port
is used because it yields much higher performance; it ignores the serial
baudrate and effectively runs at native USB 2.0 speed of 480 Mbps, if
the Due and/or your host can keep up.)

The protocol is very simple. If you want to switch port 31 to output
mode and then disable it, and read back it's value, here are the
commands you'd issue:

```
pin_out 31
write 31 0
read k 31
```

# Interface

## Input

**Note:** the serial command monitor is very simplistic. Not just to
use, but also in parsing. It does not echo back every command you enter,
just once you press enter (and if debugging is enabled). It does not
support anything like commandline editing.

It will accept, and attempt to parse and execute, a command once return
('\r') is received. The maximum length of a single commandline is 128
bytes.

If you want to resync or discard, hit backspace ('\b'). That will
discard the entire command buffer and start from scratch.

## Output

GPIO_Platform does not produce a lot of output by itself, unless you
enable debug mode. The main information that you are going to be
interested in are the values read from the inputs.

These are reported in the following format: *time* *key* *value* Which
is pretty much self-explanatory:

* *time* is reported relative to the
last reported value on the serial monitor in micro-seconds (that is, one
millionth of a second).
* *key* is a one character used to identify the respective source.
* *value* the integer value read (or computed) for this value.

## Command reference

### Input

#### read

Syntax: **read** *key* *port*

    read k 31

Perform a one-shot read of the specified port number and report this
back as a data point for key *key*. GPIO_Platform automatically knows
whether to perform a digital or analog read.

#### source_add

Syntax: **source_add** *key* *port* *period* *samples* *mode* *delta*

Sets up a periodic polling of the given port, reporting the values under
the respective keys. 

*period* is also specified in micro-seconds.  (e.g., to setup a source
that is polled at a frequency of 1000 Hz, you'd specify a period of
*1000*). If *0*, this port is **not** periodically polled, but only
considered when this source is triggered via an interrupt.

*port* is the integer port number. That is *1 - 53* for the digital
ports, *54 - 65* for the analog inputs. If it is set to *0*, then this
source is never read; this only makes sense for interrupt-driven
sources, where the value then will be the time (in uS) since the last
trigger.

If *samples* is not zero, GPIO_Platform will take this many samples into
account before considering to report this value. This is an easy way to
specify very simple filters. Staying with the example, if you had set a
sampling frequency of 1000 Hz, and specified a value of *10* here, the
filter would take a period of 1 centi-second into account. Which filter
is applied is specified via the next flag.

*mode* only takes effect if *samples* is larger than *0*.
- If *mode = 0*, the computed value will simply be a sliding average over the period covered by the samples.
- If *mode = 1*, the computed value will be an average over the sampling, but starting with a fresh buffer every time. This means that the computed value from this input will only be considered for reporting every *samples* count.
- If *mode = 2*, the computed value will only be considered for reporting if all values agree. This is particularly useful for digital inputs for which it implies they must have been stable for the sampling period.

*delta* allows to cut down on reporting noise. The value (raw or
computed) must have changed by at least this much since it was last
reported before.

These filters are very simple and mainly useful for manual debugging. In
practice, the processing (and thus filtering) power of the host computer
is vastly superior to even the Arduino Due. Any filtering on the Due
takes up processing power that could be spend reporting data.

Example:
```
// Create a source that reads the analog port 3 and reports its value
// every 1/10 second if it has changed at all:
source_add A 57 100000 0 0 1

// A source that reports if digital input 31 has been stable for at
// least 1 second, and changed since the last report:
source_add X 31 100000 10 2 1
```

#### source_attach_irq

Syntax: **source_attach_irq** *key* *port* *trigger*

Attach an interrupt-trigger to a source.

- *key* refers to a previously configured source. Whenever this
  interrupt here triggers, that source is triggered for processing.
- *port* refers to the port number that this interrupt handler should
  monitor. Note that this can be different from the *port* specified for
  the source itself. i.e., you can use a digital input port to trigger
  the read of an analog input.
- *trigger* the trigger mode. If *0*, the interrupt is triggered on a
  falling edge. *1* triggers on a rising edge. And *2* triggers on any
  state change.

Note that setting *port* or *period* to *0* during *source_add* invokes
special behavior for this function, too.

```
// Report the value of analog port 3 whenever digital port 41 raises:

source_add A 57 0 0 0 0
pin_in 41
source_attach_irq A 41 1
```

### Outputs

#### write

Syntax: **write** *port* *value*

Write the value to the specified port. Digital or analog ports are automatically
handled. For digital ports, any non-zero value is treated as *HIGH*.

#### output_add

Syntax: **output_add** *key* *port* *interval* *step* *offset* *mode* *pattern*

Setup a periodic output pattern.

*key* is a unique identifier for this output. Note that this is a
different namespaces from the source definitions.

*port* is the port this output will be written to.

*interval* specifies the interval between writes. This defines the
output resolution and frequency.

*step* the step-size through the pattern buffer at every interval. Must be between *1* and
the length of the pattern.

*offset* The starting point in the pattern buffer. Must be between *0*
and the length of the pattern.

*mode* handles wrap-around. If *0*, whenever the end of the pattern is
reached, the pattern starts again from the beginning. If *1*, the
pattern direction is reversed, until the beginning is reached (and then
again output from beginning to end).

*pattern*, finally, specifies the pattern to output to this port.
GPIO_Platform has a few hardcoded output patterns for testing that can
be used, but you can also upload patterns at runtime (eventually).

- *flip* is a pattern consisting of just two entries; *0*/*LOW* or
  *4095*/*HIGH*. This can be used to easily switch a digital port on and
  off.
- *inc* is a pattern consisting of *0* through *4095* and can be used to
  easily step a PWM or DAC output through all values.
- *sine* outputs a pattern of *4096* values stepping a port through a
  sine curve centered at *2047*. This is rather pretty with LEDs.


Examples:
```
// Flip a LED on port 31 on and off every second:
pin_out 31
output_add b 31 500000 1 1 0 flip

// If you have an RGB LED connected on PWM ports 2-4, this will cycle
// the colors through the sine curve at different speeds:
output_add r 2 500 1 0 0 sine
output_add g 3 1000 1 0 0 sine
output_add b 4 1500 1 0 0 sine
start

// And this one steps red and blue at the same time, but shifted by one
// half cycle - a bit like a police cruiser:
stop
clear
write 3 0
output_add r 2 1000 1 0 0 sine
output_add b 4 1000 1 0 2047 sine
start
```

#### pattern_list

List all available patterns.

#### output_reset

Reset all outputs to their starting offset.

### Global commands

#### start

The start command enables the periodic timer. No output will be written
or sources polled before this command is given.

#### stop

Stop all periodic in- and output.

#### clear

Wipe all configured sources and outputs.

### Other commands

#### pin_out

Syntax: **pin_out** *pin*

Set the specified pin to output. (A 1:1 wrapper around the pinMode()
command.)

#### pin_out

Syntax: **pin_in** *pin*

Set the specified pin to input.

#### help

Display all valid command names.

#### debug

Syntax: **debug** *level*

Set the debug level (between *0* and *2*). *0* disables all debugging
output and is intended for production use. *1* reports commands back.
*2* enables more detailed internal logging, and you probably don't want
to enable this.

#### dump

Dump the current configuration.

# Notes

Be careful how many sources you monitor and poll. While this sketch
tries to decouple the amount of work done in interrupt handlers, clearly
even the Due is limited in processing power.

Currently, all sources and outputs are handled via one timer interrupt.
That means that the timer interrupt needs to trigger at the greatest
common divisor of all specified periods. If you specify a period of
*1000* for one source, and *1001* for another output, you are going to
overload the Due.


# Copyright and License statement

Copyright (C) 2014 Lars Marowsky-Bree <lars@marowsky-bree.de>

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

This software is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

