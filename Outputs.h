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

#ifndef OUTPUTS_H
#define OUTPUTS_H

typedef struct {
	const char *name;
	const int len;
	const int *v; // Pointer to an array of integers
} tPattern;

typedef struct {
	char k;
	int p;
	int period;	// How often to output a new value
	int step;	// Step size through pattern buffer
	int offset;	// Initial offset into the buffer
	int mode;	// 0 = cycle, 1 = up, then down

	// Internal
	int countdown;
	int last_step;
	bool analog;	// Whether to use analog or digitalWrite
	tPattern *v;
} tOutputEntry;

#define OUTPUT_SIZE	8
typedef struct {
	int entries;
	tOutputEntry out[OUTPUT_SIZE];
} tOutputs;

extern tOutputs Outputs;
extern tPattern Patterns[];
extern const int PatternCount;

void outputs_setup(void);
void output_add(const char k, const int p, const int period, const int step, const int offset, const int mode, const char *name);
void outputs_reset(void);
void outputs_setup(void);
void outputs_push(void);

#endif
