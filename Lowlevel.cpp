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

#include "GPIO_Platform.h"
#include "Lowlevel.h"
#include <ADS1115.h>

// Lookup a port in the table, returns the index entry
// returns -1 if not found
int port_lookup(char *name) {
	int i;
	for (i = 0; i < sizeof(PortList) / sizeof(tPortListEntry); i++)
		if (strcasecmp(name, PortList[i].name) == 0)
			return i;
	return -1;
}

int port_name2id(char *name) {
	int i;
	i = port_lookup(name);
	if (i >= 0)
		return PortList[i].p;
	else
		return -1;
}

void port_write(char *portname, int v) {
	int i;

	i = port_lookup(portname);
	if (i < 0) {
		SerialUSB.println("Unknown port specified.");
	}
	_port_write(i, v);
}

int port_read(char *portname) {
	int i;
	i = port_lookup(portname);
	if (i < 0) {
		SerialUSB.println("ERROR Unknown port specified.");
		return -1;
	}
	return _port_read(i);
}

// Why does he not just directly use these functions you ask, why the
// function pointer indirection? Because there'll be more complex
// read/write functions, e.g. for servos, I²C devices, etc.

int port_ana_r(int p) {
	return analogRead(p);
}

void port_ana_w(int p, int v) {
	analogWrite(p, v);
}

int port_dig_r(int p) {
	return digitalRead(p);
}

void port_dig_w(int p, int v) {
	digitalWrite(p, v);
}

static ADS1115 ads0(ADS1115_ADDRESS_ADDR_GND);

int port_ads1115_r(int p) {
	static bool _init = false;

	if (_init == false) {
		ads0.initialize();
		ads0.setRate(ADS1115_RATE_860);
		ads0.setGain(ADS1115_PGA_4P096);
		ads0.setMode(ADS1115_MODE_CONTINUOUS);
		_init = true;
	}

	switch (p) {
        case 0:	return ads0.getConversionP0GND();
		break;
        case 1:	return ads0.getConversionP1GND();
		break;
        case 2:	return ads0.getConversionP2GND();
		break;
        case 3:	return ads0.getConversionP3GND();
		break;
        case 4: return ads0.getConversionP0N1();
		break;
        case 5: return ads0.getConversionP0N3();
		break;
        case 6: return ads0.getConversionP1N3();
		break;
        case 7: return ads0.getConversionP2N3();
		break;
	default: SerialUSB.println("ERROR Unknown port for ADS1115 on default address");
	}

}

