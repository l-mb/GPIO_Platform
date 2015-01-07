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

#include "GPIO_Platform.h"
#include "Outputs.h"
#include "RingBuf.h"
#include "Lowlevel.h"

tOutputs Outputs;

// Putting the arrays into the Patterns array directly would require *v
// to have a static size, which I don't want to avoid wasting memory.
// e.g., since some patterns have 4096 values, I'd waste almost 4k on
// the (admittedly overly simple) flip pattern
static int _p_Flip[] = { 0, 4095 };
static int _p_Increment[4096];
static int _p_Sine[4096];

tPattern Patterns[] = {
	{ .name = "flip", .len = 2, .v = _p_Flip },
	{ .name = "inc", .len = 4096, .v = _p_Increment },
	{ .name = "sine", .len = 4096, .v = _p_Sine },
};

const int PatternCount = sizeof(Patterns)/sizeof(tPattern);

// Some patterns are pre-computed. Fill those in here.
static void pattern_setup() {
	int i;

	for (i = 0; i < 4096; i++) {
		_p_Increment[i] = i;
	}

	for (i = 0; i < 4096; i++) {
		_p_Sine[i] = (sin(2.0 * PI * i / 4095) + 1) * 2047;
	}
}

// Called with interrupts disabled only
static void output_reset(int i) {
	tOutputEntry *out = &Outputs.out[i];

	out->last_step = out->offset;
	out->countdown = out->period;

	// pinMode(out->p, OUTPUT);
	port_write(out->p, out->v->v[out->last_step]);
}

void output_del(const char k) {
	int i;
	tOutputEntry *out, *last;

	for (i = 0; i < Outputs.entries; i++) {
		out = &Outputs.out[i];
		if (out->k == k)
			break;
	}
	if (i == Outputs.entries) {
		SerialUSB.print("ERROR Unknown output referenced: ");
		SerialUSB.println(k);
		return;
	}

	last = &Outputs.out[Outputs.entries-1];
	noInterrupts();
	if (last != out)
		memcpy(out, last, sizeof(tOutputEntry));
	memset(last, 0, sizeof(tOutputEntry));
	Outputs.entries--;

	master_period();

	interrupts();
}

void outputs_reset(void) {
	int i;

	for (i = 0; i < Outputs.entries; i++) {
		output_reset(i);
	}
}

void outputs_setup(void) {
	memset(&Outputs, 0, sizeof(tOutputs));
	pattern_setup();
}

void output_add(const char k, const int p, const int period, const int step, const int offset, const int mode, const char *name) {
	int i;
	tOutputEntry *out;

	for (i = 0; i < Outputs.entries; i++) {
		if (out->k == k) {
			SerialUSB.println("ERROR Output key already in use.");
			return;
		}
	}

	for (i = 0; i <= PatternCount; i++) {
		if (strcmp(Patterns[i].name, name) == 0)
			break;
	}
	if (i > PatternCount) {
		SerialUSB.print("ERROR Unknown pattern referenced: ");
		SerialUSB.println(name);
		return;
	}

	if (Outputs.entries == OUTPUT_SIZE) {
		SerialUSB.println("ERROR Too many output patterns requested");
		return;
	}

	out = &Outputs.out[Outputs.entries];
	out->k = k;
	out->p = p;
	out->period = period;
	out->step = step;
	out->offset = offset;
	out->mode = mode;
	out->v = &Patterns[i];

	if (PIN_DIG(p)) {
		out->analog = false;
	} else if (PIN_PWM(p) || PIN_DAC(p)) {
		out->analog = true;
	} else {
		SerialUSB.println("ERROR Invalid pin for output");
		return;
	}

	noInterrupts();
	output_reset(Outputs.entries);
	Outputs.entries++;

	master_period();

	interrupts();
}

void outputs_push(void) {
	int i;
	unsigned long t;

	t = micros();

	for (i = 0; i < Outputs.entries; i++) {
		tOutputEntry *out = &Outputs.out[i];
		out->countdown -= Master.period;

		if (out->countdown <= 0) {
			int step = out->last_step + out->step;

			out->countdown = out->period;

			if (step >= out->v->len) {
				if (!out->mode) {
					step = 0;
				} else {
					out->step = -out->step;
					step = out->v->len-1;
				}
			} else if (step < 0) {
				step = 0;
				if (out->mode) {
					out->step = -out->step;
				}
			}

			out->last_step = step;

			if (out->analog) {
				analogWrite(out->p, out->v->v[step]);
			} else {
				digitalWrite(out->p, out->v->v[step]);
			}
		}
	}
}


