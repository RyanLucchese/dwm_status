/*
 * Copyright (c) 2013, Ryan Lucchese <ryan.lucchese@write-c.com>
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */

// This software is intended for use on FreeBSD. It is unlikely that it will work on other systems.

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/resource.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <X11/Xlib.h>

// SIGINT handler to break out of main loop
static bool done = false;
void sigint_handler(int unused)
{
	(void)unused;
	done = true;
}

// sysctl -n hw.ncpu
size_t get_number_of_cpus()
{
	int hw_ncpu;
	size_t hw_ncpu_size = sizeof(hw_ncpu);
	if (sysctlbyname("hw.ncpu", &hw_ncpu, &hw_ncpu_size, NULL, 0) < 0)
	{
		// could not get number of CPUs from sysctl hw.ncpu
		return 0;
	}

	return (size_t)(hw_ncpu);
}

// sysctl -n kern.cp_times
void get_cp_times(long* pcpu, size_t count)
{
	size_t pcpu_size = sizeof(*pcpu) * count;
	if (sysctlbyname("kern.cp_times", pcpu, &pcpu_size, NULL, 0) < 0)
	{
		return;
	}
}

// returns percent usage: 100% - idle%
double cpu_usage(long* current, long* previous)
{
	// find the difference of total ticks in all cpu states
	long total = 0;
	for (size_t i = 0; i < CPUSTATES; i++)
	{
		long prev = current[i];
		current[i] -= previous[i];
		previous[i] = prev;
		total += current[i];
	}

	// the usage is 100% minus idle%
	return (100L - (100L * current[CPUSTATES - 1] / (total ? total : 1L)));
}

size_t mem_free()
{
	int mem_free_pages;
	size_t mem_free_size = sizeof(mem_free_pages);
	if (sysctlbyname("vm.stats.vm.v_free_count", &mem_free_pages, &mem_free_size, NULL, 0) < 0)
	{
		// could not get free memory
		return 0;
	}

	// convert pages to bytes
	size_t mem_free = (size_t)(mem_free_pages) * getpagesize();

	return mem_free;
}

size_t mem_total()
{
	size_t mem_total;
	size_t mem_total_size = sizeof(mem_total);
	if (sysctlbyname("hw.realmem", &mem_total, &mem_total_size, NULL, 0) < 0)
	{
		// colud not get total memory
		return 0;
	}

	return mem_total;
}

int main()
{
	// obtain the current display
	Display* display = NULL;
	if (!(display = XOpenDisplay(NULL)))
	{
		fprintf(stderr, "Could not open display\n");
		return 1;
	}

	// get cpu count
	const size_t hw_ncpu = get_number_of_cpus();

	// allocate pcpu buffers
	const size_t pcpu_count = hw_ncpu * CPUSTATES;
	long* pcpu_current = calloc(sizeof(long), pcpu_count);
	long* pcpu_previous = calloc(sizeof(long), pcpu_count);

	// set up intitial values
	get_cp_times(pcpu_current, pcpu_count);

	// update status
	signal(SIGINT, sigint_handler);
	const size_t status_size = 256;
	char status[status_size] = { 0 };
	while (!done)
	{
		// update cpu times
		get_cp_times(pcpu_current, pcpu_count);

		// prepare status message
		// cpu usage
		size_t offset = snprintf(status, status_size, "[ CPU ");
		for (size_t i = 0; i < hw_ncpu; i++)
		{
			offset += snprintf(status + offset, status_size - offset, "%.0f%% ", cpu_usage(&pcpu_current[i * CPUSTATES], &pcpu_previous[i * CPUSTATES]));
		}

		// memory usage
		offset += snprintf(status + offset, status_size - offset, "][ MEM ");
		size_t free = mem_free();
		size_t total = mem_total();
		size_t free_mb = free / 1024 / 1024;
		size_t usage = total - free;
		offset += snprintf(status + offset, status_size - offset, "%zuM %.0f%% ]", free_mb, ((float)usage / (float)total) * 100.0f);

		// update root window name
		XStoreName(display, DefaultRootWindow(display), status);
		XSync(display, False);

		// sleep for 1 second
		sleep(1);
	}

	// clean up
	free(pcpu_current);
	free(pcpu_previous);
	XCloseDisplay(display);

	// all done
	return 0;
}

