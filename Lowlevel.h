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

#ifndef LOWLEVELH
#define LOWLEVELH
/////////////////////////////////////////////////////////////////////////////
// PWM ports should always be driven using analogWrite
#define PWM_MIN 2
#define PWM_MAX 13
// Multi-purpose digital IO ports - need to be set to one state or the
// other
#define DIGITAL_MIN 22
#define DIGITAL_MAX 53
// Analogs are input only
#define ANALOG_MIN 54
#define ANALOG_MAX 65
// DAC are output only
#define DAC_MIN 66
#define DAC_MAX 67

#define PIN_PWM(n) ((n >= PWM_MIN) && (n <= PWM_MAX))
#define PIN_DAC(n) ((n >= DAC_MIN) && (n <= DAC_MAX))
#define PIN_DIG(n) ((n >= DIGITAL_MIN) && (n <= DIGITAL_MAX))
#define PIN_ANA(n) ((n >= ANALOG_MIN) && (n <= ANALOG_MAX))

#define PIN_OUT(n) (PIN_PWM(n) || PIN_DAC(n) || PIN_DIG(n))
#define PIN_IN(n)  (PIN_ANA(n) || PIN_DIG(n))

#define PIN_OK(n)  (PIN_PWM(n) || PIN_DAC(n) || PIN_DIG(n) || PIN_ANA(n))

int port_ana_r(int p);
void port_ana_w(int p, int v);
int port_dig_r(int p);
void port_dig_w(int p, int v);

int port_ads1115_r(int p);

typedef struct {
	const char *name;	// user-readable name
	int p;			// Numeric port id
	int (*rfunc)(int);	// Function gets the port value as a parameter (NULL if not readable)
	void (*wfunc)(int, int); // port, new value (NULL if not writable)
} tPortListEntry;

const tPortListEntry PortList[] = {
	{ .name = "none", .p = -2, .rfunc = NULL, .wfunc = NULL }, // Dummy entry so it's easier to check
	{ .name = "D0", .p = 0, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D1", .p = 1, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D2", .p = 2, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D3", .p = 3, .rfunc = &port_dig_r, .wfunc = &port_ana_w },
	{ .name = "D4", .p = 4, .rfunc = &port_dig_r, .wfunc = &port_ana_w },
	{ .name = "D5", .p = 5, .rfunc = &port_dig_r, .wfunc = &port_ana_w },
	{ .name = "D6", .p = 6, .rfunc = &port_dig_r, .wfunc = &port_ana_w },
	{ .name = "D7", .p = 7, .rfunc = &port_dig_r, .wfunc = &port_ana_w },
	{ .name = "D8", .p = 8, .rfunc = &port_dig_r, .wfunc = &port_ana_w },
	{ .name = "D9", .p = 9, .rfunc = &port_dig_r, .wfunc = &port_ana_w },
	{ .name = "D10", .p = 10, .rfunc = &port_dig_r, .wfunc = &port_ana_w },
	{ .name = "D11", .p = 11, .rfunc = &port_dig_r, .wfunc = &port_ana_w },
	{ .name = "D12", .p = 12, .rfunc = &port_dig_r, .wfunc = &port_ana_w },
	{ .name = "D13", .p = 13, .rfunc = &port_dig_r, .wfunc = &port_ana_w },
	{ .name = "D14", .p = 14, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D15", .p = 15, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D16", .p = 16, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D17", .p = 17, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D18", .p = 18, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D19", .p = 19, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D20", .p = 20, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D21", .p = 21, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D22", .p = 22, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D23", .p = 23, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D24", .p = 24, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D25", .p = 25, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D26", .p = 26, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D27", .p = 27, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D28", .p = 28, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D29", .p = 29, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D30", .p = 30, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D31", .p = 31, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D32", .p = 32, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D33", .p = 33, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D34", .p = 34, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D35", .p = 35, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D36", .p = 36, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D37", .p = 37, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D38", .p = 38, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D39", .p = 39, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D40", .p = 40, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D41", .p = 41, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D42", .p = 42, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D43", .p = 43, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D44", .p = 44, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D45", .p = 45, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D46", .p = 46, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D47", .p = 47, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D48", .p = 48, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D49", .p = 49, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D50", .p = 50, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D51", .p = 51, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D52", .p = 52, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "D53", .p = 53, .rfunc = &port_dig_r, .wfunc = &port_dig_w },
	{ .name = "A0", .p = 54, .rfunc = &port_ana_r, .wfunc = NULL },
	{ .name = "A1", .p = 55, .rfunc = &port_ana_r, .wfunc = NULL },
	{ .name = "A2", .p = 56, .rfunc = &port_ana_r, .wfunc = NULL },
	{ .name = "A3", .p = 57, .rfunc = &port_ana_r, .wfunc = NULL },
	{ .name = "A4", .p = 58, .rfunc = &port_ana_r, .wfunc = NULL },
	{ .name = "A5", .p = 59, .rfunc = &port_ana_r, .wfunc = NULL },
	{ .name = "A6", .p = 60, .rfunc = &port_ana_r, .wfunc = NULL },
	{ .name = "A7", .p = 61, .rfunc = &port_ana_r, .wfunc = NULL },
	{ .name = "A8", .p = 62, .rfunc = &port_ana_r, .wfunc = NULL },
	{ .name = "A9", .p = 63, .rfunc = &port_ana_r, .wfunc = NULL },
	{ .name = "A10", .p = 64, .rfunc = &port_ana_r, .wfunc = NULL },
	{ .name = "A11", .p = 65, .rfunc = &port_ana_r, .wfunc = NULL },
	{ .name = "DAC0", .p = 66, .rfunc = NULL, .wfunc = &port_ana_w },
	{ .name = "DAC1", .p = 67, .rfunc = NULL, .wfunc = &port_ana_w },
	// TODO: this needs more options to configure etc
	// Right now, will only do comparison to GND
	{ .name = "ads0-0", .p = 0, .rfunc = &port_ads1115_r, .wfunc = NULL },
	{ .name = "ads0-1", .p = 1, .rfunc = &port_ads1115_r, .wfunc = NULL },
	{ .name = "ads0-2", .p = 2, .rfunc = &port_ads1115_r, .wfunc = NULL },
	{ .name = "ads0-3", .p = 3, .rfunc = &port_ads1115_r, .wfunc = NULL },
	{ .name = "ads0-0n1", .p = 4, .rfunc = &port_ads1115_r, .wfunc = NULL },
	{ .name = "ads0-0n3", .p = 5, .rfunc = &port_ads1115_r, .wfunc = NULL },
	{ .name = "ads0-1n3", .p = 6, .rfunc = &port_ads1115_r, .wfunc = NULL },
	{ .name = "ads0-2n3", .p = 7, .rfunc = &port_ads1115_r, .wfunc = NULL },
};

void port_write(char *portname, int v);
int port_read(char *portname);
int port_name2id(char *name);
int port_lookup(char *name);


// Be careful with these.
inline void _port_write(int i, int v) {
	// TODO: remove debug code
	if (!PortList[i].wfunc) {
		SerialUSB.println("CRITICAL Write to unknown index");
		return;
	}
	PortList[i].wfunc(PortList[i].p, v);
}

inline int _port_read(int i) {
	// TODO: remove debug code to speed things up
	if (!PortList[i].rfunc) {
		SerialUSB.println("CRITICAL Read from unknown index");
		return 0;
	}
	return PortList[i].rfunc(PortList[i].p);
}

#endif
