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

#include "Sources.h"

tSources Sources;

RingBuf rb;

static void source_add_value(int i);

void sources_setup(void) {
	int i;

	for (i = 0; i < ISREntries; i++) {
		if (ISRTable[i].p)
			detachInterrupt(ISRTable[i].p);
		ISRTable[i].p = 0;
		ISRTable[i].trigger = 0;
		ISRTable[i].s = 0;
	}
	ISREntries = 0;
	rb.setup();
	memset(&Sources, 0, sizeof(tSources));
}

// Not to be called in interrupt context!
void source_add(char k, char p, int period, int avg, int mode, int delta) {
	int i;
	tSourceEntry *s;

	if (Sources.entries >= SOURCES_MAX)
		return;

	for (i = 0; i < Sources.entries; i++) {
		if (Sources.s[i].k == k)
			return;
	}

	noInterrupts();
	s = &Sources.s[Sources.entries];
	memset(s, 0, sizeof(tSourceEntry));
	s->k = k;
	s->p = p;
	s->mode = mode;
	s->period = period;
	s->avg = avg;
	s->delta = delta;

	Sources.entries++;

	master_period(period);
	interrupts();
}

// This is a little bit ugly!
static void _ISR_Handler_0(void) {
	source_add_value(ISRTable[0].s);		
}

static void _ISR_Handler_1(void) {
	source_add_value(ISRTable[1].s);		
}

static void _ISR_Handler_2(void) {
	source_add_value(ISRTable[2].s);		
}

static void _ISR_Handler_3(void) {
	source_add_value(ISRTable[3].s);		
}

tISREntry ISRTable[ISRSIZE] = {
	{ .p = 0, .s = 0, .trigger = 0, .handler = &_ISR_Handler_0 },
	{ .p = 0, .s = 0, .trigger = 0, .handler = &_ISR_Handler_1 },
	{ .p = 0, .s = 0, .trigger = 0, .handler = &_ISR_Handler_2 },
	{ .p = 0, .s = 0, .trigger = 0, .handler = &_ISR_Handler_3 },
};

int ISREntries = 0;

void source_attach_irq(char k, int p, int trigger) {
	int i;
	int isr;
	tSourceEntry *s;

	for (i = 0; i < Sources.entries; i++)
		if (Sources.s[i].k == k)
			break;

	if (i == Sources.entries) {
			SerialUSB.println("WARN This source key does not exist");
			return;
	}
	
	if (trigger == 0)
		trigger = FALLING;
	else if (trigger == 1)
		trigger = RISING;
	else if (trigger == 2)
		trigger == CHANGE;
	else {
		SerialUSB.print("WARN Unknown IRQ trigger specified.");
		return;
	}

	if (ISREntries == ISRSIZE) {
		SerialUSB.print("WARN Too many ISRs");
		return;
	}
	
	isr = ISREntries;
	
	ISRTable[isr].p = p;
	ISRTable[isr].s = i;
	ISRTable[isr].trigger = trigger;
	attachInterrupt(p, ISRTable[isr].handler, trigger);
	
	ISREntries++;
}

static void source_add_value(int i) {
	tSourceEntry *s = &Sources.s[i];
	unsigned long t = micros();
	int v;
	
	/* If p is 0, this is an interrupt-driven source and we're
	 * actually sampling the interrupt interval. */
	if (s->p) {
		v = port_read(s->p);
	} else {
		v = t - s->last_t;
		s->last_t = t;
	}

	rb.push(t, i, v);
}

// Only to be called in interrupt context!
void sources_poll() {
	int i;

	for (i = 0; i < Sources.entries; i++) {
		tSourceEntry *s = &Sources.s[i];

		/* Interrupt-driven source */
		if (s->period == 0)
			continue;

		s->countdown -= Master.period;
		if (s->countdown <= 0) {
			s->countdown = s->period;
		
		source_add_value(i);
		}
	}
}

void sources_process(void) {
	static unsigned long last_t;
	tSourceEntry *s;
	unsigned long t;
	int v;
	int i;

	if ((debug > 1) && rb.overflow()) {
		SerialUSB.println("Warning: Ring buffer has overflown!");
	}

	while (rb.entries()) {
		/* Pull a value from the ring buffer and process it */
		rb.pull(&t, &i, &v);
		s = &Sources.s[i];
		
		if (s->avg) {
			long sum = 0;
			int j;

			s->buf[s->cur] = v;

			if (s->cur == s->avg - 1)
				s->filled = true;
		
			/* Always take a full sample first */
			if (!s->filled)
				continue;
			
			// Mode 0 = sliding average
			if (s->mode == 0 || s->mode == 1) {
				for (j = 0; j < s->avg; j++)
					sum += s->buf[j];
				v = sum / s->avg;
				// For mode 1, start each average fresh
				s->filled = 0;
			} else if (s->mode == 2) {
				// mode 2 only reports if all values agree
				// Especially for digital values this allows
				// the source to settle
				for (j = 1; j < s->avg; j++)
					if (s->buf[0] != s->buf[j]) {
						v = s->last_v;
						break;
					}
			}
			
			if (s->cur == s->avg-1)
				s->cur = 0;
			else
				s->cur++;
		}

		if (abs(s->last_v - v) >= s->delta) {
			SerialUSB.print((unsigned long)(t-last_t));
			SerialUSB.print(";");
			SerialUSB.print(s->k);
			SerialUSB.print(";");
			SerialUSB.println(v);
			s->last_v = v;
		}
	}
}


