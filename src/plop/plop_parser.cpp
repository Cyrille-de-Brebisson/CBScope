#ifdef gui_plop
#include <vcl.h>
#endif
#pragma hdrstop


#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <math.h>

#include "global.h"

#include "plate_global.h"

#include "plot.h"

#include "plop_parms.h"

#include "plop_debug.h"

#ifdef use_gd_graphics
#include "gd.h"
#endif
#include "grid_gen.h"



char const *var_def_strings [] = { "", "", "+", "-", "*", "/"};

char const *part_type_names [] = {"triangle" ,"bar", ""};
char const *part_point_type_names [] = {"point", "part", ""};

/*
 * description of the parameters that can be used. In case of a flag, the number of parms
 * is used as the flag.
 */
parm_descr parm_list [max_parm_names] = {
	{"diameter",        1,					&diameter_found,		parm_type_double,	NULL,			&diameter, 1},
	{"hole-diameter",   1,				    &hole_diameter_found,	parm_type_double,	NULL,			&hole_diameter, 1},
	{"rel-hole-diameter",1,				    &rel_hole_diameter_found,parm_type_double,	NULL,			&rel_hole_diameter, 1},
	{"thickness",       1,					&thickness_found,		parm_type_double,	NULL,			&thickness, 1},
	{"density",         1,					&density_found,			parm_type_double,	NULL,			&density, 1},
	{"modulus",         1,					&modulus_found,			parm_type_double,	NULL,			&modulus, 1},
	{"poisson",         1,					&poisson_found,			parm_type_double,	NULL,			&poisson, 1},
	{"focal-length",    1,					&focal_length_found,	parm_type_double,	NULL,			&focal_length, 1},
	{"f-ratio",         1,					&f_ratio_found,			parm_type_double,	NULL,			&f_ratio, 1},
	{"sagitta",         1,					&sagitta_found,			parm_type_double,	NULL,			&sagitta, 1},
	{"rel-sagitta",     1,					&rel_sagitta_found,		parm_type_double,	NULL,			&rel_sagitta, 1},
	{"n-mesh-rings",    1,					&n_mesh_rings_found,	parm_type_int,		&n_mesh_rings,	NULL, 0},
	{"n-mesh-depth",    1,					&n_mesh_depth_found,	parm_type_int,		&n_mesh_depth,	NULL, 0},
	{"support-radii",   max_support_radii,	&n_abs_support_radii,	parm_type_double,	NULL,			support_radii, 0},
	{"rel-support-radii",max_support_radii,&n_rel_support_radii,	parm_type_double,	NULL,			rel_support_radii, 0},
	{"mesh-radii",      max_mesh_radii,		&n_mesh_radii_found,	parm_type_double,	NULL,			mesh_radii, 0},
	{"rel-force",       max_support_radii,	&n_rel_force,			parm_type_double,	NULL,			rel_force, 0},
	{"num-support",     max_support_radii,	&n_num_support_rings,	parm_type_int,		num_support,	NULL, 0},
	{"support-mesh-ring",max_support_radii,&n_support_mesh_ring_found,parm_type_int, 	support_mesh_ring, NULL, 0},
	{"support-angle",   max_support_radii,	&n_support_angle,		parm_type_double,	NULL,			support_angle, 0},
	{"points-on-ring",  max_mesh_radii,	    &n_mesh_points_found,	parm_type_int,		n_mesh_points,	NULL, 0},
	{"basis-ring-size", max_mesh_radii,	    &n_basis_ring_found,	parm_type_int,		basis_ring,		NULL, 0},
	{"basis-ring-min",  max_mesh_radii,	    &n_basis_min_found,		parm_type_int,		basis_min,		NULL, 0},
	{"obstruction-radius",1,			    &obstruct_found,		parm_type_double,	NULL,			&obstruction_radius, 0},
	{"obstruction-diam", 1,			        &obstruct_diam_found,	parm_type_double,	NULL,			&obstruction_diam, 0},
	{"rel-obs-radius",  1,				    &rel_obstruct_found,	parm_type_double,	NULL,			&rel_obstruction_radius, 0},
//	{"tilt-angle", 		1,			        &mirror_tilt_angle_found,parm_type_double,	NULL,			&mirror_tilt_angle, 0},
	{"edge-support-sling-angle", 1,				&edge_support_sling_angle_found,parm_type_double,	NULL,	&edge_support_sling_angle, 0},
	{"edge-support-glued-angle", 1,				&edge_support_glued_angle_found,parm_type_double,	NULL,	&edge_support_glued_angle, 0},
	{"sling-included-angle", 1,			    &sling_included_angle_found,parm_type_double,NULL,			&sling_included_angle, 0},
	{"",				0,					NULL,				0,						NULL,			NULL, 0}
	};



#define toggle(x)		x = 1 - x;


void alloc_plate_globals (void);

void *malloc_or_die (int, char const *);


//double rint (double x)
//{	return (floor (x + .5));
//}


extern void prompt_exit(void);
//extern void err_msg (char const *msg);
//extern void warn_msg (char const *msg);
//
//void err_msg_s (char const *msg, char const *arg1)
//{	char str [max_line_len];
//
//	sprintf (str, msg, arg1);
//    err_msg (str);
//}
//
//void warn_msg_s (char const *msg, char const *arg1)
//{	char str [max_line_len];
//
//	sprintf (str, msg, arg1);
//    warn_msg (str);
//}
//
//void err_msg_ss (char const *msg, char const *arg1, char const *arg2)
//{	char str [max_line_len];
//
//	sprintf (str, msg, arg1, arg2);
//    err_msg (str);
//}
//
//void err_msg_d (char const *msg, int arg1)
//{	char str [max_line_len];
//
//	sprintf (str, msg, arg1);
//    err_msg (str);
//}
//
//void err_msg_dd (char const *msg, int arg1, int arg2)
//{	char str [max_line_len];
//
//	sprintf (str, msg, arg1, arg2);
//    err_msg (str);
//}
//
//void err_msg_ds (char const *msg, int arg1, char const *arg2)
//{	char str [max_line_len];
//
//	sprintf (str, msg, arg1, arg2);
//    err_msg (str);
//}
//
//void err_msg_g (char const *msg, double arg1)
//{	char str [max_line_len];
//
//	sprintf (str, msg, arg1);
//    err_msg (str);
//}
//
//void err_msg_dg (char const *msg, int arg1, double arg2)
//{	char str [max_line_len];
//
//	sprintf (str, msg, arg1, arg2);
//    err_msg (str);
//}
//
//void err_msg_gd (char const *msg, double arg1, int arg2)
//{	char str [max_line_len];
//
//	sprintf (str, msg, arg1, arg2);
//    err_msg (str);
//}

void getwords (
	char *line,
	char wordtable [maxwords] [wordlen],
	int *wordsfound,
	int wtabsize)
{	int i;
	int j;
	int nfound;

	nfound = 0;
	for (i = 0; line [i] != '\n' && line [i] != '\0' && nfound < wtabsize;)
	{	while (line [i] == ' ' || line [i] == '\t')
			i++;
		if (line [i] != '\n' && line [i] != '\0')
		{	if (nfound == wtabsize)
			{	err_msg_s ("too many words in this line; extras are ignored: %s\n", line);
				return;
			}
			if (line [i] == '"')
			{	i++;
				for (j = 0; j < wordlen - 1 && line [i] != '"' && line [i] != '\n'; j++, i++)

					wordtable [nfound] [j] = line [i];
				wordtable [nfound] [j] = '\0';
				if (line [i] == '"')
					i++;
			}
			else
			{	for (j = 0; j < wordlen - 1 && line [i] != ' ' &&
            		line [i] != '\t' && line [i] != '\n' && line [i] != '\0'; j++, i++)

					wordtable [nfound] [j] = line [i];
				wordtable [nfound] [j] = '\0';
			}
			nfound++;
		}
	}
	*wordsfound = nfound;
}

void polar_to_euc (
	double r,
	double a,
    double *x,
	double *y)
{	*x = r * cos (fmod (a, (double) degrees) * deg_to_rad);
	*y = r * sin (fmod (a, (double) degrees) * deg_to_rad);
}

int get_var (
	char *n,
    int addit,
    int must_exist,
    char const *line)
{	int i;

	for (i = 0; i < n_variables && strcmp (n, var_table [i].name); i++)
    	;
    if (i == n_variables)
    {	if (must_exist)
    	{	err_msg_ss ("variable %s is not defined in line %s\n", n, line);
        }
       	if (addit || must_exist)
    	{	if (n_variables == max_variables)
        	{	err_msg ("too many variables\n");
            	return -1;
            }
            strcpy (var_table [n_variables].name, n);
            var_table [n_variables].var_def = var_unbound;
            var_table [n_variables].value = 0.0;
            n_variables++;
        }
        else
			i = -1;
    }
    return (i);
}

void alloc_gr_info (void)
{   int i;

    for (i = 0; parm_list [i].name [0] != '\0'; i++)
    {	parm_list [i].bound_to_var = (int *) malloc_or_die (parm_list [i].max_num * sizeof (int), "bound_vars");
    }
}

void init_gr_info (void)
{	int i;
	int j;

	if (!gr_info_alloced)
	{	alloc_gr_info ();
		gr_info_alloced = 1;
	}
	n_optimize_vars = 0;
	n_scan_vars = 0;
	n_scan_set_vars = 0;
	n_monte_vars = 0;
	n_variables = 0;
	n_parts = 0;

	for (i = 0; parm_list [i].name [0] != '\0'; i++)
	{  	*parm_list [i].num_found = 0;
		if (parm_list [i].type_flag == parm_type_double)
		{   for (j = 0; j < parm_list [i].max_num; j++)
			{	parm_list [i].bound_to_var [j] = -1;
				parm_list [i].dptr [j] = 0;
			}
		}
		else
		{   for (j = 0; j < parm_list [i].max_num; j++)
			{	parm_list [i].iptr [j] = 0;
			}
		}
	}

	modulus = 6400.0;
	poisson = .2;
	density = 2.23e-6;
}

int parm_name_index (char *pname)
{	int iparm;

	for (iparm = 0; parm_list [iparm].name [0] != '\0' && strcmp (parm_list [iparm].name, pname); iparm++)
    	;

    if (parm_list [iparm].name [0] == '\0')
    	iparm = -1;
    return (iparm);
}

int parse_var_def (
	int v_index,
	char (*words) [wordlen],
    int n_words,
	char *line)
{	int err_found;

	err_found = 0;
    if (isalpha (words [0] [0]))
    {	var_table [v_index].op1.opnd_type = opnd_var;
        var_table [v_index].op1.opnd_which_var = get_var (words [0], 0, 1, line);
    	if (var_table [v_index].op1.opnd_which_var < 0)
        	err_found = 1;
    }
    else
    {	/* in case of num, set op1 and var to number */
        var_table [v_index].op1.opnd_type = opnd_const;
        var_table [v_index].var_def = var_number;
        sscanf (words [0], "%lg", &var_table [v_index].value);
        var_table [v_index].op1.opnd_value = var_table [v_index].value;
    }
    if (n_words == 3)
    {	if (!strcmp (words [1], "+"))
            var_table [v_index].var_def = var_sum;
        else if (!strcmp (words [1], "-"))
            var_table [v_index].var_def = var_diff;
        else if (!strcmp (words [1], "*"))
            var_table [v_index].var_def = var_mul;
        else if (!strcmp (words [1], "/"))
            var_table [v_index].var_def = var_div;
        else
        {	err_msg_s ("don't have operator in line %s\n", line);
            err_found = 1;
        }
        if (isalpha (words [2] [0]))
        {	var_table [v_index].op2.opnd_type = opnd_var;
            var_table [v_index].op2.opnd_which_var = get_var (words [2], 0, 1, line);
        	if (var_table [v_index].op2.opnd_which_var < 0)
                err_found = 1;
        }
        else
        {	var_table [v_index].op2.opnd_type = opnd_const;
            sscanf (words [2], "%lg", &var_table [v_index].op2.opnd_value);
        }
    }
    return err_found;
}

int parse_var_def_line (
	int v_index,
	char *line)
{	char words [maxwords] [wordlen];
	int n_words;

	getwords (line, words, &n_words, maxwords);
    return (parse_var_def (v_index, words, n_words, line));
}


void read_info_file (char *pfile)
{   FILE *f;

    f = fopen (pfile, "r");
	if (f == (FILE *) NULL)
	{
#ifdef __BORLANDC__
   		err_msg_ss ("Can't open plop file %s\n%s\n", pfile,
            _sys_errlist [errno]);
#else
   		err_msg_s ("Can't open plop file %s\n%s\n", pfile);

#endif
		prompt_exit ();
	}
    fseek (f, (long) 0,  SEEK_END);
    plop_text_file_len = ftell (f);
    if (plop_text_file_len >= max_plop_file_len)  // >= because we append NUL
    {   err_msg ("File is too long for Plop text buffer");
        fclose (f);
        prompt_exit ();
    }
    fseek (f, (long) 0, SEEK_SET);
    // file may have changed under our feet, but get max of what we found before
    plop_text_file_len = fread (plop_text_file_buff, 1, plop_text_file_len, f);
    plop_text_file_buff [plop_text_file_len] = '\0';
    fclose (f);
}

void read_info_chars (void)
{
	int iparm;
    int var_flag;
	int j;
#ifndef standalone_grid_gen
    int jword;
    int jmax;
    int jcorner;
	int i;
	int v_index;
	int windex;
	int have_opt;
	int have_scan_var;
	int have_scan_set;
    int have_monte;
#endif
	char line [max_line_len];
	char words [maxwords] [wordlen];
	int n_words;
	int err_found;
    char *fcp;
    char *comment_ptr;

    init_gr_info ();

    err_found = 0;
    fcp = plop_text_file_buff;
    comment_ptr = plop_comment_file_buff;
    plop_comment_len = 0;

    while (*fcp != '\0')
    {   for (i = 0; *fcp != '\0' && *fcp != '\n'; fcp++)
        {   if (isprint (*fcp))
                line [i++] = *fcp;
        }
        if (*fcp != '\0')
            fcp++;
        line [i] = '\0';
        if (line [0] == ';')
        {   for (i = 0; plop_comment_len < max_comments_len &&
                line [i + 1] != '\0' && line [i + 1] != '\n'; i++)
            {   *comment_ptr++ = line [i + 1];
                plop_comment_len++;
            }
            if (plop_comment_len < max_comments_len - 2)
            {   if (plop_comment_len == 0 || *(comment_ptr - 1) != '\r')
                {   *comment_ptr++ = '\r';
                    plop_comment_len++;
                }
                *comment_ptr++ = '\n';
                plop_comment_len++;
            }
        }
	 	else if (line [0] != '\0' && line [0] != '\n')
		{	getwords (line, words, &n_words, maxwords);
#ifndef standalone_grid_gen
			have_opt = 0;
			have_scan_var = 0;
			have_scan_set = 0;
            have_monte = 0;
            if (n_words >= 2 && !strcmp (words [0], "var"))
            {	if ((i = get_var (words [1], 0, 0, "")) > 0)
            	{	err_msg_ss ("Duplicate definition of var %s in line %s\n", words [1], line);
                	err_found = 1;
                }
                else
                {	i = get_var (words [1], 1, 0, "");
                	strcpy (var_table [i].name, words [1]);
                    parse_var_def (i, words + 2, n_words - 2, line);
                }
            }
            else
            {	if (!strcmp (words [0], "optimize"))
    				have_opt = 1;
    			else if (!strcmp (words [0], "scan-var"))
    				have_scan_var = 1;
    			else if (!strcmp (words [0], "scan-set"))
    				have_scan_set = 1;
    			else if (!strcmp (words [0], "monte"))
    				have_monte = 1;
    			if (have_opt || have_scan_var || have_scan_set || have_monte)
    			{	if (n_words < 2)
    				{	err_msg_s ("Need a parameter to scan, monte, or optimize: %s\n", line);
    					err_found = 1;
    				}
    				else
    				{	var_flag = 0;
                    	iparm = get_var (words [1], 0, 0, "");
                        if (iparm >= 0)
                        {	var_flag = 1;
                        	var_table [iparm].var_def = var_number;
                        }
                        else
                    	{	iparm = parm_name_index (words [1]);
    						if (iparm < 0 && line [0] != '\0')
    						{	err_msg_s ("Don't understand var to scan/opt in line: %s\n", line);
    							err_found = 1;
    						}
                        }
    				}
    				if (!err_found)
    				{	windex = 2;
    					if (!var_flag && parm_list [iparm].max_num > 1)
    					{	windex = 3;
    						if (n_words < 3)
    						{	err_msg_ss ("%s is a vector, need to specify which to optimize: %s",
    								words [2], line);
    							err_found = 1;
    						}
    						else
    						{	sscanf (words [2], "%d", &v_index);
    							if (v_index < 0 || v_index > *parm_list [iparm].num_found)
    							{	err_msg_s ("Index of the parameter is out of range in line: %s\n", line);
    								err_found = 1;
    							}
    						}
    					}
    					else
    					{	v_index = 0;
    						/* next statment avoids need for dummy value of scalar variable, in
    						 * case the num_found is used as a flag to generate data (eg. rel-obs-radius).
    						 */
    						if (!var_flag)
                            	*parm_list [iparm].num_found = 1;
    					}
    					if (!var_flag && parm_list [iparm].type_flag != parm_type_double)
    					{	err_msg_s ("Parameters for optimization or scan must be real: %s\n", line);
    						err_found = 1;
    					}
    					if (!err_found)
    					{	/* optimize var [index] step_size */
    						if (have_opt)
    						{	if (n_optimize_vars >= max_opt_vars)
    							{	err_msg_s ("Too many optimization variables: %s\n", line);
    								prompt_exit ();
    							}
    							opt_var_which [n_optimize_vars] = iparm;
    							opt_var_index [n_optimize_vars] = v_index;
                                opt_var_is_var [n_optimize_vars] = var_flag;
    							if (windex > n_words)
    							{	err_msg_s ("Need a step size for opt var: %s\n", line);
    								err_found = 1;
    							}
    							else
    							{	sscanf (words [windex], "%lg", opt_var_step + n_optimize_vars);
    								n_optimize_vars++;
    							}
    						}
    						/* scan-var var [index] start stop nsteps */
    						else if (have_scan_var)
    						{	if (n_scan_vars >= max_scan_vars)
    							{	err_msg_s ("Too many scan-var variables: %s\n", line);
    								prompt_exit ();
    							}
    							scan_var_which [n_scan_vars] = iparm;
    							scan_var_index [n_scan_vars] = v_index;
                                scan_var_is_var [n_scan_vars] = var_flag;
    							if (windex + 3 > n_words)
    							{	err_msg_s ("Need start/stop/nsteps for scan var: %s\n", line);
    								err_found = 1;
    							}
    							else
    							{	sscanf (words [windex + 0], "%lg", scan_var_start + n_scan_vars);
    								sscanf (words [windex + 1], "%lg", scan_var_end + n_scan_vars);
    								sscanf (words [windex + 2], "%d", scan_var_nsteps + n_scan_vars);
    								n_scan_vars++;
    							}
    						}
    						/* scan-set var [index] step1 step2 ... */
    						else if (have_scan_set)
    						{	if (n_scan_set_vars >= max_scan_set_vars)
    							{	err_msg_s ("Too many scan-set variables: %s\n", line);
    								prompt_exit ();
    							}
    							scan_set_which [n_scan_set_vars] = iparm;
    							scan_set_index [n_scan_set_vars] = v_index;
                                scan_set_is_var [n_scan_set_vars] = var_flag;
    							if (windex >= n_words)
    							{	err_msg_s ("Need at least one step value scan-var: %s\n", line);
    								err_found = 1;
    							}
    							if (n_words - windex - 1 > max_scan_set)
    							{	err_msg_ds ("Too many steps for scan-var; max is %d: %s\n",
    									max_scan_set, line);
    								err_found = 1;
    							}
    							if (!err_found)
    							{	for (i = 0; i < n_words - windex; i++)
    								{	sscanf (words [windex + i], "%lg", &scan_set_values [n_scan_set_vars] [i]);
    								}
    								scan_set_n_values [n_scan_set_vars] = n_words - windex;
    								n_scan_set_vars++;
    							}
    						}
    						else if (have_monte)
    						{	if (n_monte_vars >= max_monte_vars)
    							{	err_msg_s ("Too many scan-var variables: %s\n", line);
    								prompt_exit ();
    							}
    							monte_var_which [n_monte_vars] = iparm;
    							monte_var_index [n_monte_vars] = v_index;
                                monte_var_is_var [n_monte_vars] = var_flag;
    							if (windex + 1 > n_words)
    							{	err_msg_s ("Need delta for monte: %s\n", line);
    								err_found = 1;
    							}
    							else
    							{	sscanf (words [windex + 0], "%lg", monte_var_delta + n_monte_vars);
    								n_monte_vars++;
    							}
    						}
    					}
    				}
    			}
    			else if (!strcmp (words [0], "part"))
                {   if (n_parts >= max_parts)
                    {   err_msg_d ("Too many parts; exceeds maximum of %d\n", max_parts);
                        err_found = 1;
                    }
                    else
                    {   part_rotation [n_parts] = 0;
                        jword = 1;
                        if (jword >= n_words)
                        {   err_msg_s ("Can't understand part type in line: %s\n", line);
                            err_found = 1;
                        }
                        else
                        {   for (j = 0; part_type_names [j] [0] != '\0' && strcmp (part_type_names [j], words [jword]); j++)
                                ;
                            jword++;
                        }
                        if (!err_found && words [j] [0] == '\0')
                        {   err_msg_s ("Can't understand part type in line: %s\n", line);
                            err_found = 1;
                        }
                        part_type [n_parts] = j;

                        if (jword >= n_words || sscanf (words [jword], "%d", &(part_quantity [n_parts])) != 1)
                        {   err_msg_s ("Can't understand quantity in line: %s", line);
                            err_found = 1;
                        }
                        jword++;

                        if (!err_found)
                        {   jmax = part_type_num_corners [part_type [n_parts]];
                            for (jcorner = 0; !err_found && jcorner < jmax; jcorner++)
                            {   if (jword >= n_words)
                                {   err_msg_s ("Can't find a corner type in line %s\n", line);
                                    err_found = 1;
                                }
                                else
                                {   for (j = 0; part_point_type_names [j] [0] != '\0' &&
                                                strcmp (part_point_type_names [j], words [jword]); j++)
                                        ;
                                    if (!err_found && words [j] [0] == '\0')
                                    {   err_msg_s ("Can't understand part type in line: %s\n", line);
                                        err_found = 1;
                                    }
                                    part_point_type [n_parts] [jcorner] = j;
                                    jword++;
                                }
                                if (!err_found)
                                {   if (jword + 1 >= n_words ||
                                            sscanf (words [jword], "%d ",
                                                &(part_ring_num [n_parts] [jcorner])) != 1 ||
                                            sscanf (words [jword + 1], "%d ",
                                                &(part_point_num [n_parts] [jcorner])) != 1)
                                    {   err_msg_s ("Can't understand corner type in line: %s\n", line);
                                        err_found = 1;
                                    }
                                    jword += 2;
                                }
                            }
                            for (; jcorner < tri_pts; jcorner++)
                            {   part_point_type [n_parts] [jcorner] = part_point_ring;
                                part_ring_num [n_parts] [jcorner] = 0;
                                part_point_num [n_parts] [jcorner] = 0;
                            }
                        }
                        if (!err_found)
                            n_parts++;
                    }
                }
                else
#endif
    			{	iparm = parm_name_index (words [0]);
                	if (iparm == -1 && line [0] != '\0')
    				{	warn_msg_s ("Don't understand line: %s\n", line);
    				}
                    else if (parm_list [iparm].name[0] != '\0')
    				{	if (n_words - 1 > parm_list [iparm].max_num)
    					{	err_msg_s ("Too many parameters in the line: %s\n", line);
    						err_msg_d ("The maximum is %d\n", parm_list [iparm].max_num);
    						err_found = 1;
    					}
    					else
    					{	*parm_list [iparm].num_found = n_words - 1;
    						switch (parm_list [iparm].type_flag)
    						{	case parm_type_int:
    								for (j = 0; j < n_words - 1; j++)
    								{	if (sscanf (words [j + 1], "%d", parm_list [iparm].iptr + j) != 1)
    									{	err_msg_ss ("Don't understand the value %s in %s\n",
                                                 words [j + 1], line);
    										err_found = 1;
    									}
    								}
    								break;

    							case parm_type_double:
    								for (j = 0; j < n_words - 1; j++)
    								{	if (isalpha (words [j + 1] [0]))
                                    	{	parm_list [iparm].bound_to_var [j] = get_var (words [j + 1], 0, 1, line);
                                        	var_table [parm_list [iparm].bound_to_var [j]].affects_basis |=
                                            	parm_list [iparm].affects_basis;
                                        }
                                    	else if (sscanf (words [j + 1], "%lg", parm_list [iparm].dptr + j) != 1)
    									{	err_msg_ss ("don't understand the value %s in %s\n",
                                                 words [j + 1], line);
    										err_found = 1;
    									}
    								}
    								break;

    							case parm_type_flag:
    								*parm_list [iparm].num_found = 1;
                                    break;
    						}
    					}
    				}
    			}
			}
    	}
	}
    *comment_ptr = '\0';
    for (i = 0; i < n_variables; i++)
    {	if (var_table [i].var_def == var_unbound)
    	{	warn_msg_s ("Variable %s has no definition and is not a scan or opt var\n",
                var_table [i].name);
        }
    }
    for (i = n_variables - 1; i >= 0; i--)
    {	if (var_table [i].affects_basis)
    	{	switch (var_table [i].var_def)
    		{
        		case var_sum:
            	case var_diff:
            	case var_mul:
            	case var_div:
            		if (var_table [i].op1.opnd_type == opnd_var)
                		var_table [var_table [i].op1.opnd_which_var].affects_basis = 1;
            		if (var_table [i].op2.opnd_type == opnd_var)
                		var_table [var_table [i].op2.opnd_which_var].affects_basis = 1;
                    break;
            }
        }
    }

	if (n_rel_support_radii != 0 && n_abs_support_radii != 0)
		warn_msg ("Input has both rel-support-radii and support-radii, the former overrides the latter\n");
	n_support_radii = n_rel_support_radii + n_abs_support_radii;
    if (n_support_angle != 0 && n_support_angle != (n_rel_support_radii + n_abs_support_radii))
    	warn_msg ("Mismatch between number of support angles and radii\n");
    if (n_rel_force != 0 && n_rel_force != (n_rel_support_radii + n_abs_support_radii))
    	warn_msg ("Mismatch between number of relative forces and radii\n");
    if (obstruct_found + rel_obstruct_found + obstruct_diam_found > 1)
		warn_msg ("Input has more than one obstruction specification\n");
	if (f_ratio_found + focal_length_found + sagitta_found + rel_sagitta_found > 1)
	{	warn_msg ("Input has more than one of rel_sagitta, sagitta, f-ratio and focal-length.\n");
	}
    err_found |= check_part_descrs ();

	if (err_found)
	{	prompt_exit ();
    }
}

void read_info (char *pfile)
{   read_info_file (pfile);
    read_info_chars ();
}


void eval_vars (void)
{	int i;
	int j;

	for (i = 0; i < n_variables; i++)
    {	switch (var_table [i].var_def)
    	{
        	case var_sum:
            case var_diff:
            case var_mul:
            case var_div:
            	if (var_table [i].op1.opnd_type == opnd_var)
                	var_table [i].op1.opnd_value = var_table [var_table [i].op1.opnd_which_var].value;
            	if (var_table [i].op2.opnd_type == opnd_var)
                	var_table [i].op2.opnd_value = var_table [var_table [i].op2.opnd_which_var].value;
        }
        switch (var_table [i].var_def)
    	{
        	case var_sum:
            	var_table [i].value = var_table [i].op1.opnd_value + var_table [i].op2.opnd_value;
                break;

            case var_diff:
            	var_table [i].value = var_table [i].op1.opnd_value - var_table [i].op2.opnd_value;
                break;

            case var_mul:
            	var_table [i].value = var_table [i].op1.opnd_value * var_table [i].op2.opnd_value;
                break;

            case var_div:
            	var_table [i].value = var_table [i].op1.opnd_value / var_table [i].op2.opnd_value;
                break;

		}
    }
    for(i = 0; parm_list [i].name [0] != '\0'; i++)
    {	if (parm_list [i].type_flag == parm_type_double)
    	{	for (j = 0; j < *parm_list [i].num_found; j++)
        	{	if (parm_list [i].bound_to_var [j] >= 0)
            	{	parm_list [i].dptr [j] = var_table [parm_list [i].bound_to_var [j]].value;
                }
            }
        }
    }
}

#ifndef standalone_grid_gen
void var_def_str (var_def *var_ptr, char *str)
{	char s1 [wordlen];
	char s2 [wordlen];
    char s3 [wordlen];

	switch (var_ptr->var_def)
    {	case var_unbound:
            strcpy (str, "");

        case var_number:
            sprintf (str, " %lg", var_ptr->value);
            break;

        case var_sum:
        case var_diff:
        case var_mul:
        case var_div:
            if (var_ptr->op1.opnd_type == opnd_const)
                sprintf (s1, " %lg", var_ptr->op1.opnd_value);
            else
                sprintf (s1, " %s", var_table [var_ptr->op1.opnd_which_var].name);
            sprintf (s2, " %s", var_def_strings [var_ptr->var_def]);
            if (var_ptr->op2.opnd_type == opnd_const)
                sprintf (s3, " %lg", var_ptr->op2.opnd_value);
            else
                sprintf (s3, " %s", var_table [var_ptr->op2.opnd_which_var].name);
            sprintf (str, "%s%s%s", s1, s2, s3);
            break;
    }
}


// you must update plop_text_file_len before calling

void write_info_file (
	char *fname)
{   FILE *f;

    f = fopen (fname, "w");
	if (f == (FILE *) NULL)
	{	err_msg_s ("Can't open plop file %s\n", fname);
		prompt_exit ();
	}
    fwrite (plop_text_file_buff, 1, plop_text_file_len, f);
    fclose (f);
}

void write_info_line (char const *lp)
{   while (*lp != '\0' && plop_text_file_ptr < plop_text_file_buff + max_plop_file_len - 1)
    {
#ifdef __BORLANDC__
       // check that a LF is preceded by CR
       if (*lp == '\n' && (plop_text_file_ptr == plop_text_file_buff ||
                *(plop_text_file_ptr - 1) != '\r'))
            *plop_text_file_ptr++ = '\r';
#endif
        *plop_text_file_ptr++ = *lp++;
    }
    if (*lp != '\0')
    {   err_msg ("Plop file is too long");
        prompt_exit ();
    }
    plop_text_file_len = plop_text_file_ptr - plop_text_file_buff;
}

int check_part_descrs (void)
{   int ipart;
    int ncorners;
    int jcorner;
    int err_found = 0;

    for (ipart = 0; ipart < n_parts; ipart++)
    {   ncorners = part_type_num_corners [part_type [ipart]];
        for (jcorner = 0; jcorner < ncorners; jcorner++)
        {   if (part_point_type [ipart] [jcorner] == part_point_ring)
            {   if (part_ring_num [ipart] [jcorner] < 0 ||
                    part_ring_num [ipart] [jcorner] >= n_support_radii)
                {   err_msg_dd ("Point %d in part %d refers to an illegal support ring",
                            ipart + 1, jcorner + 1);
                    err_found = 1;
                }
                if (part_point_num [ipart] [jcorner] < 0 ||
                    part_point_num [ipart] [jcorner] >=
                    num_support [part_ring_num [ipart] [jcorner]])
                {   err_msg_dd  ("Point %d in part %d refers to an illegal point in a support ring",
							jcorner + 1, ipart + 1);
                    err_found = 1;
                }
            }
            else
            {   if (part_ring_num [ipart] [jcorner] < 0 ||
                    part_ring_num [ipart] [jcorner] >= ipart)
                {   err_msg_dd ("Point %d in part %d refers to an illegal part.\n"
                        "The part number must be a lower numbered part",
							 jcorner + 1, ipart + 1);
                    err_found = 1;
                }
                if (part_point_num [ipart] [jcorner] < 0 ||
                    part_point_num [ipart] [jcorner] >=
                    part_quantity [part_ring_num [ipart] [jcorner]])
                {   err_msg_dd  ("Point %d in part %d refers to an illegal part number",
							jcorner + 1, ipart + 1);
                    err_found = 1;
                }
            }

        }
    }
    return (err_found);
}

/* write the plop info out to a file.
 * Use physical parameters, not variables, if phys is true.
 * Include optimizations, scans, etc.,  if write_opts is true.
 */

void write_info_chars (
    int phys,
    int write_opts)
{	int iparm;
	int i;
    int j;
    int jmax;
    char s [max_line_len];
    char line [max_line_len];

    plop_text_file_ptr = plop_text_file_buff;

    // no need for bounds check as long as plop_text_file =_buff > 2 * plop_comment_len
    for (i = 0; i < plop_comment_len;)
    {   *plop_text_file_ptr++ = ';';
        while (i < plop_comment_len && plop_comment_file_buff [i] != '\n')
            *plop_text_file_ptr++ = plop_comment_file_buff [i++];
        if (i < plop_comment_len)
            *plop_text_file_ptr++ = plop_comment_file_buff [i++];
        else
        {
#ifdef __BORLANDC__
            // check that a LF is preceded by CR
            if (plop_text_file_ptr == plop_text_file_buff ||
                *(plop_text_file_ptr - 1) != '\r')
                *plop_text_file_ptr++ = '\r';
#endif
            *plop_text_file_ptr++ = '\n';
        }
    }
    if (!phys)
    {   for (i = 0; i < n_variables; i++)
        {	var_def_str (var_table + i, s);
			sprintf (line, "var %s %s\n", var_table [i].name, s);
            write_info_line (line);
        }
    }

    for (iparm = 0; parm_list [iparm].name [0] != '\0'; iparm++)
    {	if (*parm_list [iparm].num_found > 0)
    	{	sprintf (line, "%s", parm_list [iparm].name);
            write_info_line (line);
        	for (i = 0; i < *parm_list [iparm].num_found; i++)
            {	if (!phys && parm_list [iparm].type_flag == parm_type_double &&
                        parm_list [iparm].bound_to_var [i] >= 0)
        		{   sprintf (line, " %s", var_table [parm_list [iparm].bound_to_var [i]].name);
                    write_info_line (line);
                }
            	else
        		{	switch (parm_list [iparm].type_flag)
            		{	case parm_type_int:
                			sprintf (line, " %d", parm_list [iparm].iptr [i]);
                            write_info_line (line);
                	        break;

                		case parm_type_double:
                    		sprintf (line, " %g", parm_list [iparm].dptr [i]);
                            write_info_line (line);
                            break;
                    }
                }
            }
            sprintf (line, "\n");
            write_info_line (line);
        }
    }
    if (write_opts)
    {   for (i = 0; i < n_optimize_vars; i++)
        {   sprintf (line, "optimize");
            write_info_line (line);
            if (opt_var_is_var [i])
            {   sprintf (line, " %s", var_table [opt_var_which [i]].name);
                write_info_line (line);
            }
            else
            {   sprintf (line, " %s", parm_list [opt_var_which [i]].name);
                write_info_line (line);
                if (parm_list [opt_var_which [i]].max_num > 1)
                {   sprintf (line, " %d", opt_var_index [i]);
                    write_info_line (line);
                }
            }
            sprintf (line, " %lg\n", opt_var_step [i]);
            write_info_line (line);
        }

        for (i = 0; i < n_monte_vars; i++)
        {   sprintf (line, "monte");
            write_info_line (line);
            if (monte_var_is_var [i])
            {   sprintf (line, " %s", var_table [monte_var_which [i]].name);
                write_info_line (line);
            }
            else
            {   sprintf (line, " %s", parm_list [monte_var_which [i]].name);
                write_info_line (line);
                if (parm_list [monte_var_which [i]].max_num > 1)
                {   sprintf (line, " %d", monte_var_index [i]);
                    write_info_line (line);
                }
            }
            sprintf (line, " %lg\n", monte_var_delta [i]);
            write_info_line (line);
        }

        for (i = 0; i < n_scan_vars; i++)
        {   sprintf (line, "scan-var ");
            write_info_line (line);
            if (scan_var_is_var [i])
            {   sprintf (line, " %s", var_table [scan_var_which [i]].name);
                write_info_line (line);
            }
            else
            {   sprintf (line, " %s", parm_list [scan_var_which [i]].name);
                write_info_line (line);
                if (parm_list [scan_var_which [i]].max_num > 1)
                {   sprintf (line, "%d ", scan_var_index [i]);
                    write_info_line (line);
                }
            }
            sprintf (line, " %lg %lg %d\n", scan_var_start [i], scan_var_end [i],
                 scan_var_nsteps [i]);
            write_info_line (line);
        }

        for (i = 0; i < n_scan_set_vars; i++)
        {   sprintf (line, "scan-set");
            write_info_line (line);
            if (scan_set_is_var [i])
            {   sprintf (line, " %s", var_table [scan_set_which [i]].name);
                write_info_line (line);
            }
            else
            {   sprintf (line, " %s", parm_list [scan_set_which [i]].name);
                write_info_line (line);
                if (parm_list [scan_set_which [i]].max_num > 1)
                {   sprintf (line, " %d", scan_set_index [i]);
                    write_info_line (line);
                }
            }
            for (j = 0; j < scan_set_n_values [i]; j++)
            {	sprintf (line, " %lg ", scan_set_values [i] [j]);
                write_info_line (line);
			}
            sprintf (line, "\n");
            write_info_line (line);
        }
    }
    for (i = 0; i < n_parts; i++)
    {   sprintf (line, "part %s %d", part_type_names [part_type [i]],
                part_quantity [i]);
        write_info_line (line);
        jmax = part_type_num_corners [part_type [i]];
        for (j = 0; j < jmax; j++)
        {   sprintf (line, " %s %d %d", part_point_type_names [part_point_type [i] [j]],
                    part_ring_num [i] [j], part_point_num [i] [j]);
            write_info_line (line);
        }
        
        write_info_line ("\n");

    }
    *plop_text_file_ptr++ = '\0';
}

void write_info (
	char *fname,
    int phys,
    int write_opts)
{   write_info_chars (phys, write_opts);
    write_info_file (fname);
}
#endif

void print_plate (
	char *plate_name)
{	int i;
	int j;
	double x;
	double y;
	int pmin;
	int p_temp [tri_pts];
	int band_width;

	FILE *plate_file;

	plate_file = fopen (plate_name, "w");
	if (plate_file == (FILE *) NULL)
	{	err_msg_s ("Can't open %s\n", plate_name);
		prompt_exit ();
	}
	fprintf (plate_file, "/title/\n");
	fprintf (plate_file, "diam = %13.6g, focal = %13.6g, edge = %13.6g, n_points = %d\n",
		radius * 2, focal_length, thickness, n_supports);

	/* plate needs 3 z constraints to make the problem defined. for now,
	 * just pick 3 points on first circle.
	 */

	fprintf (plate_file, "/ NODES / ELEMENTS / CONSTRAINED NODES / MATERIALS /\n");
	fprintf (plate_file, " %d %d %d %d\n", total_mesh_points, n_triangles, 3, 1);

	fprintf (plate_file, "/ MATERIAL NO. / YOUNG'S MODULUS / POISSON'S RATIO /\n");
	fprintf (plate_file, " %d %g %g\n", 1, modulus, poisson);

	fprintf (plate_file, "/ NODE NO. /   X   /   Y   /\n");
	for (i = 0; i < total_mesh_points; i++)
	{	j = point_sorted_order [i];
		polar_to_euc (point_radius [j], point_angle [j], &x, &y);
		fprintf (plate_file, " %d %20.12g %20.12g\n", i + 1, x, y);
	}

	fprintf (plate_file, "/ ELEM. NO. / NODE1 / NODE2 / NODE3 / THICKNESS / MATERIAL NO. / SHEAR FACTOR /\n");
	band_width = 0;
	for (i = 0; i < n_triangles; i++)
	{	/* sort the point set because plate uses banded matrices */
		pmin = 0;
		for (j = 1; j < tri_pts; j++)
		{	if (point_reordered_num [triangle_points [i] [j]] < point_reordered_num [triangle_points [i] [pmin]])
				pmin = j;
		}
		for (j = 0; j < tri_pts; j++)
		{	p_temp [j] = point_reordered_num [triangle_points [i] [(j + pmin) % tri_pts]];
			if (p_temp [j] - p_temp [0] > band_width)
				band_width = p_temp [j] - p_temp [0];
		}
		fprintf (plate_file, " %d %d %d %d %20.12g %d %20.12g\n",
			i + 1,
			p_temp [0] + 1,
			p_temp [1] + 1,
			p_temp [2] + 1,
			triangle_thickness [i],
			1, 5.0 / 6.0);
	}
	printf ("band width = %d\n", band_width * 3);

	/* easiest way to deal with need for contraints is to fix center in all dimensions. */

	fprintf (plate_file, "BOUNDARY CONDITION / NODE NO. / CONSTRAINT  /\n");
	fprintf (plate_file, " %d 100\n", point_reordered_num [0] + 1);
	fprintf (plate_file, " %d 010\n", point_reordered_num [0] + 1);
	fprintf (plate_file, " %d 001\n", point_reordered_num [0] + 1);

	fprintf (plate_file, "FORCES ON NODES / NODE NO. /   FZ   /   MX   /   MY   /\n");
	for (i = 0; i < n_supports; i++)
	{	fprintf (plate_file, " %d %20.12g %20.12g %20.12g\n",
			point_reordered_num [support_point [i]] + 1, -support_force [i], 0.0, 0.0);
	}
	fprintf (plate_file, " %d 0.0 0.0 0.0\n", total_mesh_points + 1);

	fprintf (plate_file, "PRESSURE ON ELEMENTS / ELEMENT NO. / PRESSURE /\n");
	for (i = 0; i < n_triangles; i++)
	{	fprintf (plate_file, " %d %20.12g\n", i + 1, triangle_pressure [i]);
	}
}

#ifdef use_gd_graphics
void print_pic (
	int mesh_flag,
	char *pic_name)
{	int i;
	int j;
	double scale;
    double angle;
	FILE *gd_file;
	double xs, ys;
	double xe, ye;
    int xsi, ysi;
    int xei, yei;
    gdImagePtr gd_image;
    int gd_color_black;
    int gd_color_white;


	gd_file = fopen (pic_name, "wb");

	if (gd_file == (FILE *) NULL)
	{	err_msg_s ("Can't open %s\n", pic_name);
		prompt_exit ();
	}
    gd_image = gdImageCreate (pic_size, pic_size);

    gd_color_black = gdImageColorAllocate(gd_image, 0, 0, 0);
    gd_color_white = gdImageColorAllocate(gd_image, 255, 255, 255);
    for (i = 0; i < pic_size; i++)
    	for (j = 0; j < pic_size; j++)
    		gdImageSetPixel (gd_image, i, j, gd_color_white);
	scale = pic_size * 0.95 / diameter;
    if (mesh_flag)
    {	for (i = 0; i < n_triangles; i++)
        {	for (j = 0; j < tri_pts; j++)
            {	polar_to_euc (point_radius [triangle_points [i] [j]],
                        point_angle [triangle_points [i] [j]], &xs, &ys);
                polar_to_euc (point_radius [triangle_points [i] [(j + 1) %tri_pts]],
                        point_angle [triangle_points [i] [(j + 1) %tri_pts]], &xe, &ye);
                xs *= scale;
                ys *= scale;
                xe *= scale;
                ye *= scale;
                xsi = (int) xs + pic_size / 2;
                ysi = (int) ys + pic_size / 2;
                xei = (int) xe + pic_size / 2;
                yei = (int) ye + pic_size / 2;
                gdImageLine (gd_image, xsi, ysi, xei, yei, gd_color_black);
            }
        }
    }
    else
    	gdImageArc (gd_image, pic_size / 2, pic_size / 2,
        	(int) (pic_size * .95), (int) (pic_size * .95), 0, 360, gd_color_black);
	for (i = 0; i < n_support_radii; i++)
    {	for (j = 0; j < num_support [i]; j++)
    	{	angle = support_angle [i] + j * (double) degrees / num_support [i];
        	polar_to_euc (support_radii [i], angle, &xs, &ys);
            xs *= scale;
            ys *= scale;
            xsi = (int) xs + pic_size / 2;
            ysi = (int) ys + pic_size / 2;
            gdImageArc (gd_image, xsi, ysi, pic_size / 70 + 1, pic_size / 70 + 1, 0, 360, gd_color_black);
        }
    }

    gdImageGif (gd_image, gd_file);
    fclose (gd_file);
}

#else

void print_pic (
	char *pic_name)
{	int i;
	int j;
	double scale;
	FILE *pic_file;
	double xs, ys;
	double xe, ye;

	pic_file = fopen (pic_name, "w");
	if (pic_file == (FILE *) NULL)
	{	err_msg_s ("Can't open %s\n", pic_name);
		prompt_exit ();
	}

	scale = 7.0 / diameter;
	fprintf (pic_file, ".PS\n");
	for (i = 0; i < n_triangles; i++)
	{	for (j = 0; j < tri_pts; j++)
		{	polar_to_euc (point_radius [triangle_points [i] [j]],
					point_angle [triangle_points [i] [j]], &xs, &ys);
			polar_to_euc (point_radius [triangle_points [i] [(j + 1) %tri_pts]],
					point_angle [triangle_points [i] [(j + 1) %tri_pts]], &xe, &ye);
			xs *= scale;
			ys *= scale;
			xe *= scale;
			ye *= scale;
			fprintf (pic_file, "line from %g,%g to %g,%g\n", xs, ys, xe, ye);
		}
	}
	for (i = 0; i < n_supports; i++)
	{
		polar_to_euc (point_radius [support_point [i]],
				point_angle [support_point [i]], &xs, &ys);
		xs *= scale;
		ys *= scale;
		fprintf (pic_file, "circle radius .05 at %g,%g\n", xs, ys);
	}
	fprintf (pic_file, ".PE\n");
    fclose (pic_file);
}
#endif


