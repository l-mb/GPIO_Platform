/*
 * Copyright (C) 2014 Lars Marowsky-Bree <lars@marowsky-bree.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <DueTimer.h>
#include "GPIO_Platform.h"

#include "Sources.h"
#include "Outputs.h"
#include "SerialMonitor.h"
#include "Lowlevel.h"

/////////////////////////////////////////////////////////////////////////////
// Set to 1 for having all functions log what they do
// Set to 2 for much more verbose logging

int debug = 1;

tMaster Master;

///////////////////////////////////////////////////////////////////////

static int gcd(int a, int b) {
	int t;
	while (b != 0) {
		t = b;
		b = a % b;
		a = t;
	}
	return a;
}

// To be called while interrupts are disabled
void master_period(unsigned long period) {
	int i;
	int new_period;

	// We default to ticking the timer at least every 10s
	new_period = 10000000;

	for (i = 0; i < Sources.entries; i++)
		new_period = gcd(new_period, Sources.s[i].period);

	for (i = 0; i < Outputs.entries; i++)
		new_period = gcd(new_period, Outputs.out[i].period);

	if (new_period != Master.period) {
		Timer1.stop();
		Master.period = new_period;
		Timer1.setPeriod(Master.period);

		if (debug) {
			SerialUSB.print("DEBUG Timer period adjusted to ");
			SerialUSB.println(Master.period);
		}
		if (new_period < 50) {
			SerialUSB.print("WARN Timer period very short: ");
			SerialUSB.println(Master.period);
		}
	}

	if (Master.started)
		Timer1.start();
}

static void master_handler() {
	sources_poll();
	outputs_push();
}

/****************************************************************************
 Main code
 ****************************************************************************/

void setup(){
	SerialMonitor_setup();

	if (debug > 1) {
		delay(20000);
		SerialUSB.println("Setting up sources");
	}
	sources_setup();
	if (debug > 1) {
		delay(1000);
		SerialUSB.println("Setting up outputs");
	}
	outputs_setup();
	if (debug > 1) {
		delay(1000);
		SerialUSB.println("Setting up ringbuffer");
	}

	if (debug > 1) {
		delay(1000);
		SerialUSB.println("Setting resolution");
	}
	analogReadResolution(12);
	analogWriteResolution(12);

	if (debug > 1) {
		delay(1000);
		SerialUSB.println("Attaching timer");
	}
	Timer1.attachInterrupt(master_handler);
	if (debug > 1) {
		delay(1000);
		SerialUSB.println("At your service (hopefully).");
	}
}

void loop(){
	SerialMonitor_poll();

	sources_process();
}

