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

#include "SerialMonitor.h"
#include "Sources.h"
#include "Outputs.h"
#include "Lowlevel.h"
#include <DueTimer.h>

static char serialCmd[128] = "\0";

// Ugly global variables for parsing strings
static char *P_s, *P_r;

static int parse_start() {
	P_r = NULL;
	P_s = strtok_r(serialCmd, DELIM, &P_r);
	/* Skip initial command */
	P_s = strtok_r(NULL, DELIM, &P_r);
}

static bool parse_char(char *c) {
	if (!c)
		return false;
	if (!P_s)
		return false;
	if (*P_s == 0) {
		return false;
	} else {
		*c = *P_s;
		P_s = strtok_r(NULL, DELIM, &P_r);
		return true;
	}
}

static int parse_int(int *v) {
	if (!v)
		return false;
	if (!P_s)
		return false;
	if (*P_s == 0) {
		return false;
	} else {
		*v = atoi(P_s);
		P_s = strtok_r(NULL, DELIM, &P_r);
		return true;
	}
}

static bool parse_str(char *c, int len) {
	if (!c)
		return false;
	if (!P_s)
		return false;
	if (*P_s == 0) {
		return false;
	} else {
		strlcpy(c, P_s, len);
		P_s = strtok_r(NULL, DELIM, &P_r);
		return true;
	}
}

static void cmd_stop() {
	Master.started = false;
	Timer1.stop();
}

static void cmd_start() {
	Master.started = true;
	Timer1.start(Master.period);
}

static void cmd_source_add() {
	char k;
	int p;
	int period;
	int avg;
	int mode;
	int delta;

	if (!parse_char(&k))
		return;
	if (!parse_int(&p))
		return;
	if (!parse_int(&period))
		return;
	if (!parse_int(&avg))
		return;
	if (!parse_int(&mode))
		return;
	if (!parse_int(&delta))
		return;

	if (debug) {
		SerialUSB.print("DEBUG Adding source: ");
		SerialUSB.print(k);
		SerialUSB.print(DELIM);
		SerialUSB.print(p);
		SerialUSB.print(DELIM);
		SerialUSB.print(period);
		SerialUSB.print(DELIM);
		SerialUSB.print(avg);
		SerialUSB.print(DELIM);
		SerialUSB.print(mode);
		SerialUSB.print(DELIM);
		SerialUSB.println(delta);
	}
	source_add(k, p, period, avg, mode, delta);
}

// Technically, this also only modifies the Sources table
// But that function really, really was getting too large
static void cmd_source_attach_irq() {
	char k;
	int p;
	int trigger;
	int count_ticks;

	if (!parse_char(&k))
		return;
	if (!parse_int(&p))
		return;
	if (!parse_int(&trigger))
		return;
	if (!parse_int(&count_ticks))
		return;

	if (debug) {
		SerialUSB.print("DEBUG Attaching interupt to source: ");
		SerialUSB.print(k);
		SerialUSB.print(DELIM);
		SerialUSB.print(p);
		SerialUSB.print(DELIM);
		SerialUSB.print(trigger);
		SerialUSB.print(DELIM);
		SerialUSB.println(count_ticks);
	}
	source_attach_irq(k, p, trigger, count_ticks);
}

static void cmd_output_add() {
	char k;
	int p;
	int period;
	int step;
	int offset;
	int mode;
	char name[32];

	if (!parse_char(&k))
		return;
	if (!parse_int(&p))
		return;
	if (!parse_int(&period))
		return;
	if (!parse_int(&step))
		return;
	if (!parse_int(&offset))
		return;
	if (!parse_int(&mode))
		return;
	if (!parse_str(name, sizeof(name)))
		return;

	if (debug) {
		SerialUSB.print("DEBUG Adding output: ");
		SerialUSB.print(k);
		SerialUSB.print(" port: ");
		SerialUSB.print(p);
		SerialUSB.print(" period: ");
		SerialUSB.print(period);
		SerialUSB.print(" step: ");
		SerialUSB.print(step);
		SerialUSB.print(" offset: ");
		SerialUSB.print(offset);
		SerialUSB.print(" mode: ");
		SerialUSB.print(mode);
		SerialUSB.print(" pattern: ");
		SerialUSB.println(name);
	}
	output_add(k, p, period, step, offset, mode, name);
}

// It's because of this function primarily that so many data structures
// are exposed globally, but it is quite useful for debugging.
static void cmd_dump() {
	int i;

	SerialUSB.print("INFO Timer period: ");
	SerialUSB.print(Master.period);
	SerialUSB.print(" Currently enabled: ");
	SerialUSB.println(Master.started);

	SerialUSB.print("INFO Sources: ");
	SerialUSB.println((int)Sources.entries);

	for (i = 0; i < Sources.entries; i++) {
		tSourceEntry *s = &Sources.s[i];

		SerialUSB.print(" Key: ");
		SerialUSB.print(s->k);
		SerialUSB.print(" Port: ");
		SerialUSB.print(s->p);
		SerialUSB.print(" Period: ");
		SerialUSB.print(s->period);
		SerialUSB.print(" Avg: ");
		SerialUSB.print(s->avg);
		SerialUSB.print(" Mode: ");
		SerialUSB.print(s->mode);
		SerialUSB.print(" Delta: ");
		SerialUSB.println(s->delta);

		if (s->irq) {
			SerialUSB.print("  IRQ: ");
			SerialUSB.print(s->irq);
			SerialUSB.print(" Trigger: ");
			SerialUSB.print(s->trigger);
			SerialUSB.print(" Counter: ");
			SerialUSB.println(s->count_ticks);
		}

		SerialUSB.print(" Index: ");
		SerialUSB.println(i);
	}

	SerialUSB.print("INFO Outputs: ");
	SerialUSB.println((int)Outputs.entries);

	for (i = 0; i < Outputs.entries; i++) {
		tOutputEntry *out = &Outputs.out[i];
		SerialUSB.print(i);
		SerialUSB.print(DELIM);
		SerialUSB.print("output_add ");
		SerialUSB.print(out->k);
		SerialUSB.print(DELIM);
		SerialUSB.print(out->p);
		SerialUSB.print(DELIM);
		SerialUSB.print(out->period);
		SerialUSB.print(DELIM);
		SerialUSB.print(out->step);
		SerialUSB.print(DELIM);
		SerialUSB.print(out->offset);
		SerialUSB.print(DELIM);
		SerialUSB.print(out->mode);
		SerialUSB.print(DELIM);
		SerialUSB.println(out->v->name);
		SerialUSB.print(" Status: Countdown: ");
		SerialUSB.print(out->countdown);
		SerialUSB.print(" pos: ");
		SerialUSB.println(out->last_step);
	}

	SerialUSB.print("INFO Ringbuffer entries: ");
	SerialUSB.print(rb.entries());
	SerialUSB.print(" Has overflown: ");
	SerialUSB.println((int)rb.overflow());
}

static void cmd_pattern_list() {
	int i, j;

	// Not included in general output dump because it's rather
	// annoying long
	SerialUSB.print("INFO Patterns: ");
	SerialUSB.print((int)PatternCount);

	for (i = 0; i < PatternCount; i++) {
		SerialUSB.print("Name: ");
		SerialUSB.print(Patterns[i].name);
		SerialUSB.print(" Length: ");
		SerialUSB.print(Patterns[i].len);
		SerialUSB.print(" Pattern:");
		for (j = 0; j < Patterns[i].len; j++) {
			SerialUSB.print(DELIM);
			SerialUSB.print(Patterns[i].v[j]);
		}
		SerialUSB.println("");
	}
	SerialUSB.println("");
}

static void cmd_write() {
	int port;
	int value;

	if (!parse_int(&port))
		return;
	if (!parse_int(&value))
		return;

	if (debug) {
		SerialUSB.print("DEBUG Write: ");
		SerialUSB.print(port);
		SerialUSB.print(DELIM);
		SerialUSB.println(value);
	}
	port_write(port, value);
}

static void cmd_writed() {
	int port;
	int value;

	if (!parse_int(&port))
		return;
	if (!parse_int(&value))
		return;

	if (debug) {
		SerialUSB.print("DEBUG Write Digital: ");
		SerialUSB.print(port);
		SerialUSB.print(DELIM);
		SerialUSB.println(value);
	}
	if (value > 0)
		digitalWrite(port, HIGH);
	else
		digitalWrite(port, LOW);
}

static void cmd_read() {
	int port;
	int v;
	char k;

	if (!parse_char(&k))
		return;
	if (!parse_int(&port))
		return;

	v = port_read(port);
	SerialMonitor_log(micros(), k, v);
}

static void cmd_pin_in() {
	int port;

	if (!parse_int(&port))
		return;

	if (debug) {
		SerialUSB.print("DEBUG Setting pinMode input for ");
		SerialUSB.println(port);
	}

	pinMode(port, INPUT);
}

static void cmd_output_reset() {
	if (debug) SerialUSB.println("DEBUG All outputs back to 0.");
	outputs_reset();
}

static void cmd_pin_out() {
	int port;

	if (!parse_int(&port))
		return;

	if (debug) {
		SerialUSB.print("DEBUG Setting pinMode output for ");
		SerialUSB.println(port);
	}

	pinMode(port, OUTPUT);
}

static void cmd_debug() {
	int lvl;

	if (!parse_int(&lvl))
		return;

	if (debug) {
		SerialUSB.print("DEBUG Setting debug level ");
		SerialUSB.println(lvl);
	}
	debug = lvl;
}

static void cmd_clear() {
	Timer1.stop();
	Master.period = 0;
	Master.started = 0;
	sources_setup();
	outputs_setup();
	SerialUSB.println("INFO All clear");
}

static void cmd_help(void);

typedef struct tCmdTableEntry {
	const char *cmd;
	void (*handler)();
} tCmdTableEntry;

static tCmdTableEntry CmdTable[] = {
	{ .cmd = "stop", .handler = &cmd_stop },
	{ .cmd = "start", .handler = &cmd_start },

	{ .cmd = "source_add", .handler = &cmd_source_add },
	{ .cmd = "source_attach_irq", .handler = &cmd_source_attach_irq },

	{ .cmd = "output_add", .handler = &cmd_output_add },
	{ .cmd = "output_reset", .handler = &cmd_output_reset },

	{ .cmd = "pattern_list", .handler = &cmd_pattern_list },

	{ .cmd = "pin_in", .handler = &cmd_pin_in },
	{ .cmd = "pin_out", .handler = &cmd_pin_out },
	{ .cmd = "debug", .handler = &cmd_debug },
	{ .cmd = "dump", .handler = &cmd_dump },
	{ .cmd = "clear", .handler = &cmd_clear },
	{ .cmd = "help", .handler = &cmd_help },

	{ .cmd = "writed", .handler = &cmd_writed },
	{ .cmd = "write", .handler = &cmd_write },
	{ .cmd = "read", .handler = &cmd_read }
};

static void cmd_help() {
	int i;
	SerialUSB.println("INFO Valid commands:");
	for (i = 0; i < sizeof(CmdTable) / sizeof(tCmdTableEntry); i++) {
		SerialUSB.print(" ");
		SerialUSB.println(CmdTable[i].cmd);
	}
}

static void cmd_execute() {
	int i;
	if (debug) {
		SerialUSB.print("DEBUG Command was: ");
		SerialUSB.println(serialCmd);
	}

	for (i = 0; i < sizeof(CmdTable) / sizeof(tCmdTableEntry); i++) {
		if (strncmp(serialCmd, CmdTable[i].cmd, strlen(CmdTable[i].cmd)) == 0) {
			parse_start();
			CmdTable[i].handler();
		}
	}
}

void SerialMonitor_log(unsigned long t, char k, int v) {
	static unsigned long last_t = 0;

	SerialUSB.print((unsigned long)(t-last_t));
	SerialUSB.print(DELIM);
	SerialUSB.print(k);
	SerialUSB.print(DELIM);
	SerialUSB.println(v);
	last_t = t;
}

void SerialMonitor_poll(void) {
	int c;

	if (SerialUSB.available()) {
		c = SerialUSB.read();

		switch (c) {
		case '\r': cmd_execute();
		case '\b': memset(serialCmd, 0, sizeof(serialCmd));
			   break;
		default:
			int l = strlen(serialCmd);
			if (l < sizeof(serialCmd)-2)
				serialCmd[l] = c;
			else
				SerialUSB.println("ERROR Input buffer overflow");
		}
	}
}

void SerialMonitor_setup(void) {
	memset(serialCmd, 0, sizeof(serialCmd));
	SerialUSB.begin(115200);
}


