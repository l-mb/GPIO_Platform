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
pin d31 0
write D31 0
read k d31
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

### Portnames

GPIO_Platform knows about descriptive port names, from *d0* to *d53*,
from *a0* to *a11*, and *DAC0* and *Dac1*. These are case-insensitive.

Do not specify numeric port ids.

In the future, this will be expanded to handle I²C, SPI and other
addresses.

## Output

GPIO_Platform does not produce a lot of output by itself, unless you
enable debug mode. The main information that you are going to be
interested in are the values read from the inputs.

These are reported in the following format: __*time* *key* *value*__

* *time* is reported relative to the last reported value on the serial monitor in micro-seconds (that is, one millionth of a second)
* *key* is a one character used to identify the respective source (specified when you configure it)
* *value* the integer value read (or computed) for this source

## Command reference

### Input

#### read

Syntax: **read** *key* *port*

    read k d31

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

*port* references the portname. If this is **none**, then this source is
never read; this only makes sense for interrupt-driven sources. See
**source_attach_irq** for details on which value is considered then.

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
source_add A a3 100000 0 0 1

// A source that reports if digital input 31 has been stable for at
// least 1 second, and changed since the last report:
source_add X D31 100000 10 2 1
```

#### source_attach_irq

Syntax: **source_attach_irq** *key* *irq* *trigger* *count_ticks*

Attach an interrupt-trigger to a source.

- *key* refers to a previously configured source. Whenever this
  interrupt here triggers, that source is triggered for processing.
- *irq* refers to the port that this interrupt handler should
  monitor.
- *trigger* the trigger mode. If *0*, the interrupt is triggered on a
  falling edge. *1* triggers on a rising edge. And *2* triggers on any
  state change.
- *count_ticks* defines whether to count ticks since the last sampling
  period.
  - If set to *0* **and** the port on the source is *0*, the source measures the µS between triggers (interval).
  - If set to *0* **and** the port on the source is set, the source samples the port whenever the interrupt triggers.
  - If set to *1*, and the source has a *non-zero* period, the source will count the number of times the interrupt has triggered since the last period. This is an easy way to count pulses.

```
// Report the value of analog port 3 whenever digital pin 41 falls:
source_add A a3 0 0 0 0
pin d41 0
source_attach_irq A d41 0 0

// Count the number of times digital pin 31 is raised per second:
source_add D none 1000000 0 0 0
source_attach_irq D d31 1 1

// Something more complex. Report the average interval between state
// changes of pin 45, averaged over 8 samples each:
source_add C none 0 8 1 0
source_attach_irq C d45 2 0
// (Note that there is a catch here - *if* the final round of times
//  interrupt 45 triggers is less than 8 times, that last period is
//  never reported. There is currently no final timeout.)
```

#### source_del

Syntax: **source_del** *key*

Delete the given source.

### Outputs

#### write

Syntax: **write** *portname* *value*

Write the value to the specified port. Digital or analog ports are automatically
handled. For digital ports, any non-zero value is treated as *HIGH*.

#### output_add

Syntax: **output_add** *key* *portname* *interval* *step* *offset* *mode* *pattern*

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
// Flip a LED on pin 13 on and off every second:
output_add b d13 500000 1 1 0 flip

// If you have an RGB LED connected on PWM ports 2-4, this will cycle
// the colors through the sine curve at different speeds:
output_add r d2 500 1 0 0 sine
output_add g d3 1000 1 0 0 sine
output_add b d4 1500 1 0 0 sine
start

// And this one steps red and blue at the same time, but shifted by one
// half cycle - a bit like a police cruiser:
stop
clear
write d3 0
output_add r d2 1000 1 0 0 sine
output_add b d4 1000 1 0 2047 sine
start
```

#### pattern_list

List all available patterns.

#### output_del

Syntax: **output_del** *key*

Delete the given output.

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

#### pin

Syntax: **portname** *pin* *mode*

Set the specified *portname* to the specified *mode*.

- *mode* set to *0* corresponds to *INPUT*
- *mode* set to *1* corresponds to *INPUT_PULLUP*
- *mode* set to *2* corresponds to *OUTPUT*

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

Copyright (C) 2014 Lars Marowsky-Bree (lars@marowsky-bree.de)

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

