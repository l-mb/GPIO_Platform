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
#include "Lowlevel.h"

void port_write(int p, int v) {
	if (PIN_PWM(p) || PIN_DAC(p)) {
		analogWrite(p, v);
	} else if (PIN_DIG(p)) {
		if (v > 0)
			digitalWrite(p, HIGH);
		else
			digitalWrite(p, LOW);
	} else {
		SerialUSB.print("ERROR IO Port out of range for write: ");
		SerialUSB.println(p);
	}
}

int port_read(int p) {
	if (p >= ANALOG_MIN && p <= ANALOG_MAX) {
		return analogRead(p);
	} else if (p >= DIGITAL_MIN && p <= DIGITAL_MAX) {
		return digitalRead(p);
	} else {
		SerialUSB.print("ERROR IO Port out of range for read: ");
		SerialUSB.println(p);
		return 0;
	}
}


