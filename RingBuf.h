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

#ifndef HEADER_RINGBUF
#define HEADER_RINGBUF

typedef struct {
	volatile unsigned long t;
	volatile int i;
	volatile int v;
} tRingBufferEntry;

#define RINGBUFFER_SIZE 64

class RingBuf {
public:
	RingBuf(void);
	void setup();
	char entries();
	bool overflow();
	void push(const unsigned long t, const int i, const int v);
	void pull(unsigned long *t, int *i, int *v);

private:
	volatile char _start;
	volatile char _tail;
	volatile char _entries;
	volatile bool _overflow;
	tRingBufferEntry _data[RINGBUFFER_SIZE];
};

#endif
