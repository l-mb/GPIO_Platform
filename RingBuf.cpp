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

#include "Arduino.h"
#include "RingBuf.h"

RingBuf::RingBuf(void) {
	// Constructor, nothing to see here (yet)
	// TODO: make ringbuffer size configurable?
}

void RingBuf::setup() {
	_start = 0;
	_tail = 0;
	_entries = 0;
	memset(&_data, 0, sizeof(_data));
}

// Return number of entries in buffer
char RingBuf::entries() {
	return _entries;
}

// Returns true if the buffer has ever overflown
bool RingBuf::overflow() {
	return _overflow;
}

// Pull the next entry from the ring buffer
// Must only be called from non-interrupt context!
void RingBuf::pull(unsigned long *t, int *i, int *v) {
	tRingBufferEntry *d = &_data[_start];

	if (_entries == 0) {
		return;
	}

	*t = d->t;
	*i = d->i;
	*v = d->v;

	noInterrupts();
	_entries--;
	_start++;
	if (_start >= RINGBUFFER_SIZE) {
		_start = 0;
	}
	interrupts();
}

// Push an entry to the ring buffer.
// Only to be called from interrupt context!
void RingBuf::push(const unsigned long t, const int i, const int v) {
	tRingBufferEntry *d = &_data[_tail];

	if (_entries == RINGBUFFER_SIZE) {
		_overflow = true;
		return;
	}

	d->t = t;
	d->i = i;
	d->v = v;

	_entries++;
	_tail++;
	if (_tail >= RINGBUFFER_SIZE) {
		_tail = 0;
	}
}
