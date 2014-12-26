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

#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <X11/Xlib.h>

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
	(void)hw_ncpu;

	// clean up
	XCloseDisplay(display);

	return 0;
}

