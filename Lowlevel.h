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

#define PIN_OUTPUT(n) (
void port_write(int p, int v);
int port_read(int p);

#endif
