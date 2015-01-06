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

	noInterrupts();
	for (i = 0; i < Sources.entries; i++)
		if (Sources.s[i].irq)
			detachInterrupt(Sources.s[i].irq);
	rb.setup();
	memset(&Sources, 0, sizeof(tSources));
	interrupts();
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

// So yes, this is a bit ugly. It's needed because IRQ handlers don't
// take arguments; there's one IRQ handler per source table entry.

#define _IRQ_Handler_X(n) static void _IRQ_Handler_##n(void) { \
	if (Sources.s[n].count_ticks) \
		Sources.s[n].ticks++; \
	else \
		source_add_value(n); \
}

_IRQ_Handler_X(0);
_IRQ_Handler_X(1);
_IRQ_Handler_X(2);
_IRQ_Handler_X(3);
_IRQ_Handler_X(4);
_IRQ_Handler_X(5);
_IRQ_Handler_X(6);
_IRQ_Handler_X(7);
_IRQ_Handler_X(8);
_IRQ_Handler_X(9);
_IRQ_Handler_X(10);
_IRQ_Handler_X(11);
_IRQ_Handler_X(12);
_IRQ_Handler_X(13);
_IRQ_Handler_X(14);
_IRQ_Handler_X(15);

// This is separate from the Source table so that that can simply
// be wiped using memset().

void (*IRQ_Handlers[SOURCES_MAX])(void) =
	{ &_IRQ_Handler_0, &_IRQ_Handler_1, &_IRQ_Handler_2, &_IRQ_Handler_3,
	&_IRQ_Handler_4, &_IRQ_Handler_5, &_IRQ_Handler_6, &_IRQ_Handler_7,
	&_IRQ_Handler_8, &_IRQ_Handler_9, &_IRQ_Handler_10, &_IRQ_Handler_11,
	&_IRQ_Handler_12, &_IRQ_Handler_13, &_IRQ_Handler_14, &_IRQ_Handler_15
};

void source_attach_irq(char k, int irq, int trigger, int count_ticks) {
	int i;
	tSourceEntry *s;

	for (i = 0; i < Sources.entries; i++) {
		s = &Sources.s[i];
		if (s->k == k)
			break;
	}

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
	
	s->irq = irq;
	s->trigger = trigger;
	s->count_ticks = count_ticks;
	
	if (s->count_ticks && !s->period) {
		SerialUSB.println("WARN Counting ticks requires period to be non-zero");
		return;
	}

	if (IRQ_Handlers[i]) {
		attachInterrupt(irq, IRQ_Handlers[i], trigger);
	} else {
		SerialUSB.println("ERROR No IRQ handler available!?");
	}
}

static void source_add_value(int i) {
	tSourceEntry *s = &Sources.s[i];
	unsigned long t = micros();
	int v;

	if (i >= Sources.entries) {
		SerialUSB.println("Uh-oh! Something really bad happened.");
	}

	/* If p is 0, this is an interrupt-driven source and we're
	 * actually sampling the interrupt interval. */
	if ((s->period == 0) && (s->p > 0)) {
		v = t - s->last_t;
		s->last_t = t;
	} else if (s->p > 0) {
		v = port_read(s->p);
	} else if (s->count_ticks) {
		v = s->ticks;
		s->ticks = 0;
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


