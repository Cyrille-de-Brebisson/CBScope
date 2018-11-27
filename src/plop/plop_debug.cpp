#include <stdio.h>

#include "plop_debug.h"

int debugs [ndebugs];

char const *debug_strings [] = {
	"grid_gen",
	"opt",
	"basis",
	"triangle",
	"interpolate",
	"matrix",
    "fem",
	"plate_load",
	"displacement",
	"locale",
    ""};


void err_msg_s (char const *msg, char const *arg1)
{	char str [max_line_len];

	sprintf (str, msg, arg1);
	err_msg (str);
}

void warn_msg_s (char const *msg, char const *arg1)
{	char str [max_line_len];

	sprintf (str, msg, arg1);
	warn_msg (str);
}

void err_msg_ss (char const *msg, char const *arg1, char const *arg2)
{	char str [max_line_len];

	sprintf (str, msg, arg1, arg2);
	err_msg (str);
}

void err_msg_d (char const *msg, int arg1)
{	char str [max_line_len];

	sprintf (str, msg, arg1);
	err_msg (str);
}

void err_msg_dd (char const *msg, int arg1, int arg2)
{	char str [max_line_len];

	sprintf (str, msg, arg1, arg2);
	err_msg (str);
}

void err_msg_ds (char const *msg, int arg1, char const *arg2)
{	char str [max_line_len];

	sprintf (str, msg, arg1, arg2);
	err_msg (str);
}

void err_msg_g (char const *msg, double arg1)
{	char str [max_line_len];

	sprintf (str, msg, arg1);
	err_msg (str);
}

void err_msg_dg (char const *msg, int arg1, double arg2)
{	char str [max_line_len];

	sprintf (str, msg, arg1, arg2);
	err_msg (str);
}

void err_msg_gd (char const *msg, double arg1, int arg2)
{	char str [max_line_len];

	sprintf (str, msg, arg1, arg2);
	err_msg (str);
}


 