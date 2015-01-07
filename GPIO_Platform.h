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

#ifndef LOGGER_H
#define LOGGER_H

#include "Arduino.h"

/****************************************************************************
 Global timer handling
 ****************************************************************************/

typedef struct {
	unsigned long period; // Global timer period
	bool started;
} tMaster;

// Functions from the main sketch that other modules need to use -
// especially the command module:
extern int debug;

extern tMaster Master;

void master_period(unsigned long period);

#endif

