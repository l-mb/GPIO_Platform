/*
 * Copyright (C) 2014 Lars Marowsky-Bree <lars@marowsky-bree.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef SOURCESH
#define SOURCESH

#include "GPIO_Platform.h"
#include "RingBuf.h"

#define SAMPLES_MAX 32

typedef struct {
	char k;
	int p;		// Can be 0 for sources that are only interrupt-driven
			// Having a separate port here in addition to
			// the interrupt table allows samples to be
			// driven by an external interrupt
	int period;	// uS interval between samples
			// Can be 0 for sources that only count IRQ intervals
	int avg;	// Average over this many samples; 0 = use raw value
	int mode;	// 0 = sliding average
			// 1 = one full period every time
			// 2 = only report value if stable over the avg period
			// 3 = Oversampling to improve resolution (TODO)
	int delta;	// Report only if the value has changed by at least this

	// For IRQs:
	int irq;	// Port triggering the IRQ
	int trigger;	// Trigger mode
	int count_ticks;// Tricky!
			// If 1, measure the number of ticks per period instead

	// House keeping:
	int countdown;	// Ticks down on every invocation
	int last_v;	// Last reported value, if only reporting changes
	long last_t;	// For interrupt-driven sources: last tick
	int buf[SAMPLES_MAX]; // A buffer for averaging values
	int cur;	// cursor in the buffer
	int ticks;	// For IRQs: how often has this ticked in this period
	bool filled;	// If the buffer has been filled at least once
	unsigned char method; // Which method to use for acquiring values
} tSourceEntry;

#define SOURCES_MAX 16

typedef struct {
	char entries;
	tSourceEntry s[SOURCES_MAX];
} tSources;

extern tSources Sources;

extern RingBuf rb;

void source_add(char k, char p, int period, int avg, int mode, int delta);
void source_del(char k);
void sources_setup(void);

// Called from main code's interrupt handler
void sources_poll(void);
void sources_process(void);
void source_attach_irq(char k, int p, int trigger, int count_ticks);

#endif
