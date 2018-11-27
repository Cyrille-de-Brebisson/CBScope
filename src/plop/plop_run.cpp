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

#include "plop_parser.h"

#include "plop_debug.h"
#include "grid_gen.h"

#ifdef use_gd_graphics
#include "gd.h"
#endif
                                                      

/*
 * Some limit checks and bug checks that need to be done:
 *
 * Check that n_basis_rings > 0 if using basis
 * Check basis_ring_min > 0 or mesh screws up
 * Check that n_mesh_rings <= max_mesh_perim
 *
*/

int check_part_descrs (void);

#ifndef standalone_grid_gen
void plate_setup (void);
void pload (void);
void formk (void);
void solve (void);
void out (void);

void calculate_zernike (void);

void refocus_mirror (void);
void calc_rms (void);

void init_global_graphics (void);
void plate_init (void);
void init_graphics (void);
void write_picture (char *, plop_cmap *);

void copy_plot_data (void);


#endif
void set_max_points (int);
void prompt_exit (void);

#ifndef __BORLANDC__
#define abs(x)		((x) > 0 ? (x) : -(x))
#endif


#include "plop_parms.h"
#include "grid_gen.h"


#ifndef standalone_grid_gen

void evaluate_parts (void)
{   int ipart;
    int iring;
    int icorner;
    int icorner_rot;
    int ncorners;
    double cg_x;
    double cg_y;
    double part_force;
    double point_x;
    double point_y;
    double base_len;
    double rot_x;
    double rot_y;
    double new_x;
	double new_y;
	double dx;

    for (ipart = 0; ipart < n_parts; ipart++)
    {   cg_x = 0;
        cg_y = 0;
        part_force = 0;
        ncorners = part_type_num_corners [part_type [ipart]];
        for (icorner = 0; icorner < ncorners; icorner++)
        {   iring = part_ring_num [ipart] [icorner];
            if (part_point_type [ipart] [icorner] == part_point_ring)
            {   polar_to_euc (support_radii [iring],
						support_angle [iring] + part_point_num [ipart] [icorner] * (double) degrees / num_support [iring],
                        &point_x, &point_y);
                part_corner_x [ipart] [icorner] = point_x;
                part_corner_y [ipart] [icorner] = point_y;
                part_corner_radius [ipart] [icorner] = support_radii [iring];
                part_corner_angle [ipart] [icorner] =
					support_angle [iring] + part_point_num [ipart] [icorner] * (double) degrees / num_support [iring];
                cg_x += point_x * rel_force [iring];
                cg_y += point_y * rel_force [iring];
                part_force += rel_force [iring];
            }
            else
            {   polar_to_euc (part_cg_radius [iring],
						part_cg_angle [iring] + part_point_num [ipart] [icorner] * (double) degrees / part_quantity [iring],
                        &point_x, &point_y);
                part_corner_x [ipart] [icorner] = point_x;
                part_corner_y [ipart] [icorner] = point_y;
                part_corner_radius [ipart] [icorner] = part_cg_radius [iring];
                part_corner_angle [ipart] [icorner] =
					part_cg_angle [iring] + part_point_num [ipart] [icorner] * (double) degrees / part_quantity [iring];
                cg_x += point_x * part_total_force [iring];
                cg_y += point_y * part_total_force [iring];
                part_force += part_total_force [iring];
            }
        }
        cg_x /= part_force;
        cg_y /= part_force;
        part_cg_x [ipart] = cg_x;
        part_cg_y [ipart] = cg_y;
		part_cg_angle [ipart] = atan2 (cg_y, cg_x) * degrees / (2 * M_PI);
        part_cg_radius [ipart] = sqrt (cg_x * cg_x + cg_y * cg_y);
        part_total_force [ipart] = part_force;
    }
    /* generate drawing data */

	for (ipart = 0; ipart < n_parts; ipart++)
	{   ncorners = part_type_num_corners [part_type [ipart]];
        for (icorner = 0; icorner < ncorners; icorner++)
        {   icorner_rot = (icorner + part_rotation [ipart]) % ncorners;
            part_draw_x [ipart] [icorner] = part_corner_x [ipart] [icorner_rot] -
                    part_corner_x [ipart] [part_rotation [ipart]];
            part_draw_y [ipart] [icorner] = part_corner_y [ipart] [icorner_rot] -
					part_corner_y [ipart] [part_rotation [ipart]];
        }
		part_draw_cg_x [ipart] = part_cg_x [ipart] - part_corner_x [ipart] [part_rotation [ipart]];
        part_draw_cg_y [ipart] = part_cg_y [ipart] - part_corner_y [ipart] [part_rotation [ipart]];

        base_len = sqrt (part_draw_x [ipart] [1] * part_draw_x [ipart] [1] +
						 part_draw_y [ipart] [1] * part_draw_y [ipart] [1]);
		rot_x = part_draw_x [ipart] [1] / base_len;
		rot_y = part_draw_y [ipart] [1] / base_len;



		/* corner 0 at origin, corner 1 on x axis, rotate the rest */

		for (icorner = 1; icorner < ncorners; icorner++)
		{   new_x =  rot_x * part_draw_x [ipart] [icorner] + rot_y * part_draw_y [ipart] [icorner];
			new_y = -rot_y * part_draw_x [ipart] [icorner] + rot_x * part_draw_y [ipart] [icorner];
			part_draw_x [ipart] [icorner] = new_x;
			part_draw_y [ipart] [icorner] = new_y;

		}
		new_x =  rot_x * part_draw_cg_x [ipart] + rot_y * part_draw_cg_y [ipart];
        new_y = -rot_y * part_draw_cg_x [ipart] + rot_x * part_draw_cg_y [ipart];
		part_draw_cg_x [ipart] = new_x;
		part_draw_cg_y [ipart] = new_y;

		/* ensure that base line is on bottom of drawing if triangle.
		 * rotate 180 and change offset point so distances are relative to leftmost point.
		 * part draw has already been rotated so corners 0 and 1 are baseline.
		 */

		if (ncorners == 3 && part_draw_y [ipart] [2] < 0)
		{	dx = part_draw_x [ipart] [1];
			for (icorner = 0; icorner < ncorners; icorner++) {
				part_draw_x [ipart] [icorner] = dx - part_draw_x [ipart] [icorner];
				part_draw_y [ipart] [icorner] = -part_draw_y [ipart] [icorner];
			}
			part_draw_cg_x [ipart] = dx - part_draw_cg_x [ipart];
			part_draw_cg_y [ipart] = -part_draw_cg_y [ipart];
		}
	}
}


void add_sphere_distortion (void) {
    int i;
    double r;
    double warp_factor;

	for (i = 0; i < plate_n_points; i++)
	{	r = sqrt (plate_point_x [i] * plate_point_x [i] + plate_point_y [i] * plate_point_y [i]);
        plate_z_displacement [i] += r * r * r * r * parab_a4;
    }
}


/*
 * Evaluate error. If map_rings is true, perform mesh generation, or basis generation
 * in case of grid gen basis.
 */

double eval_error (
	int map_rings,
	char const *reason,
	char const *line_term)
{	int j;
	double error;

	eval_vars ();

	if (grid_gen_using_basis)
	{	if (map_rings || basis_changed)
		{	gen_basis_mesh ();

			order_nodes ();

			exec_plate ();

            basis_changed = 0;
		}
		else
			compute_geom ();		/* some vars don't affect mesh, but need to recompute */

		interpolate_basis ();
	}
	else
	{	gen_mesh (map_rings);

		order_nodes ();

		exec_plate ();
	}

    if (warp_sphere_flag)
    {   add_sphere_distortion ();
    }

	interpolate_tris ();

    if (calculate_zernike_flag)
        calculate_zernike ();

	if (refocus_flag)
	{	refocus_mirror ();
		interpolate_tris ();
	}
	calc_rms ();
	if (use_p_v_error)
		error = plate_err_vis_p_v;
	else
		error = plate_err_vis_rms;

	if (trace_opt)
	{	fprintf (output_file, "eval_grid:%d %s: ", map_rings, reason);
		for (j = 0; j < n_optimize_vars; j++)
		{	if (opt_var_is_var [j])
        		fprintf (output_file, " %s: %g", var_table [opt_var_which [j]].name, var_table [opt_var_which [j]].value);
        	else
            	fprintf (output_file, " %s[%d]: %g",
					parm_list [opt_var_which [j]].name,
					opt_var_index [j],
					parm_list [opt_var_which [j]].dptr [opt_var_index [j]]);
		}
		fprintf (output_file, " err: %g", error);
		fprintf (output_file, "%s", line_term);
		fflush (output_file);
	}

    evaluate_parts ();

	return (error);
}

double drand48 (void)
{	return ((double) rand() /32767.0);
}

void run_monte (void)
{	int i;
	int j;
    double error;
    double start_vals [max_monte_vars];
    double total_error;
    double max_error;
    double x;

   	for (j = 0; j < n_monte_vars; j++)
	{	if (monte_var_is_var [j])
        	start_vals [j] = var_table [monte_var_which [j]].value;
        else
      		start_vals [j] = parm_list [monte_var_which [j]].dptr [monte_var_index [j]];
    }

    total_error = 0;
    max_error = 0;
    for(i = 0; i < n_monte_tests && !please_stop_plop; i++)
    {	fprintf (output_file, "monte ");
    	for (j = 0; j < n_monte_vars; j++)
    	{	x = (2 * drand48 () - 1) * monte_var_delta [j] + start_vals [j];
        	if (monte_var_is_var [j])
            {	var_table [monte_var_which [j]].value = x;
            	basis_changed |= var_table [monte_var_which [j]].affects_basis;
            	fprintf (output_file, " %s: %lg", var_table [monte_var_which [j]].name, x);
            }
            else
            {	parm_list [monte_var_which [j]].dptr [monte_var_index [j]] = x;
            	if (parm_list [opt_var_which [i]].max_num > 1)
				{	if (!quiet)
						fprintf (output_file, " %s[%d]: %lg", parm_list [monte_var_which [i]].name, monte_var_index [i], x);
				}
				else
				{	if (!quiet)
						fprintf (output_file, " %s: %lg", parm_list [monte_var_which [j]].name, x);
            	}
				basis_changed |= parm_list [monte_var_which [j]].affects_basis;
			}
        }
        error = eval_error (0, "monte", "\n");
        total_error += error;
        if (error > max_error)
        	max_error = error;
#ifdef gui_plop
        UpdateMonteDisplay (i + 1, total_error / (i + 1), max_error);
#endif
        fprintf (output_file, " err: %lg\n", error);
    }
    if (please_stop_plop)
        terminate_plop ();

    fprintf (output_file, "avg error: %lg max error: %lg\n", total_error / n_monte_tests, max_error);
   	for (j = 0; j < n_monte_vars; j++)
    {	if (monte_var_is_var [j])
        	var_table [monte_var_which [j]].value= start_vals [j];
        else
      		parm_list [monte_var_which [j]].dptr [monte_var_index [j]]= start_vals [j];
    }
}


void run_opt (
	int have_start,
	double *start_vect,
	double *result_vect,
	int gen_first,
	int gen_later,
	int print_result)
{	int i;
	int j;
	double error;
	double best_error;
	double starting_point [max_opt_vars];
	double best_point [max_opt_vars];
	double test_vect [2 * max_opt_vars + 1] [max_opt_vars];		/* unit length vectors in various directions to test */
	double step [max_opt_vars];		/* unit length vector in direction of last step */
	double d;
	double m;
	double rel_step_size;
	double norm;
	double corr;
	double step_fact;
	double angle_fact;

	double de_dx [2 * max_opt_vars];
	int n_evals;
	int n_directions;		/* number of directions we eval the grad */


	if (n_optimize_vars > 0)
	{	rel_step_size = .1;
		angle_fact = 3.0;
		n_evals = 0;

		for (i = 0; i < n_optimize_vars; i++)
		{	if (opt_var_is_var [i])
        		starting_point [i] = var_table [opt_var_which [i]].value;
        	else
            	starting_point [i] = parm_list [opt_var_which [i]].dptr [opt_var_index [i]];
			if (have_start && reuse_best_opt)
				best_point [i] = start_vect [i];
			else
				best_point [i] = starting_point [i];
			if (opt_var_is_var [i])
        		var_table [opt_var_which [i]].value = best_point [i];
            else
            	parm_list [opt_var_which [i]].dptr [opt_var_index [i]] = best_point [i];
		}

		error = eval_error (gen_first, "initial guess ", "\n");
#ifdef gui_plop
        DebugMsg ("initial run opt\n");
        UpdateCellEditOptVars ();
        UpdateRunOptStatus (n_evals, rel_step_size);
#endif

		best_error = error;

		for (i = 0; i < n_optimize_vars; i++)
			step [i] = 1.0 / sqrt ((double ) n_optimize_vars);
		while (rel_step_size > optimize_min_delta &&
                 n_evals < max_optimize_evals && !please_stop_plop)
		{
#ifdef gui_plop
            DebugMsg ("new opt step\n");
#endif
            m = 0;
			/* if reject previous, check +/- 1 in all dimensions */
			if (debug_opt)
				fprintf (debug_file, "step: %g angle_fact %g\n", rel_step_size, angle_fact);
			{	/* if no reject, try previous, as well as  +/- .1 in all directions */
				n_directions = 2 * n_optimize_vars + 1;
				for (i = 0; i < 2 * n_optimize_vars + 1; i++)
				{	norm = 0;
					for (j = 0; j < n_optimize_vars; j++)
					{	if (i / 2  == j)
							d = step [j] + (((i % 2) * 2) - 1) * angle_fact;
						else
							d = step [j];
						test_vect [i] [j] = d;
						norm += d * d;
					}
					norm = 1.0 / sqrt (norm);
					for (j = 0; j < n_optimize_vars; j++)
					{	test_vect [i] [j] *= norm;
                    	if (opt_var_is_var [j])
                            var_table [opt_var_which [j]].value =
								best_point [j] + test_vect [i] [j] * opt_var_step [j] * rel_step_size;
						else
							parm_list [opt_var_which [j]].dptr [opt_var_index [j]] =
								best_point [j] + test_vect [i] [j] * opt_var_step [j] * rel_step_size;
					}
					error = eval_error (gen_later, "de_dx", "\n");
					de_dx [i] = (error - best_error);
					m += de_dx [i] * de_dx [i];
				}
			}
			if (m < error * error * optimize_delta_f_too_small * optimize_delta_f_too_small)
			{	rel_step_size *= .5;
				angle_fact *= 2;
				if (angle_fact > 3.0)
					angle_fact = 3.0;
			}
			else
			{	norm = 0;
				for (i = 0; i < n_optimize_vars; i++)
				{	step [i] = 0;
					for (j = 0; j < n_directions; j++)
					{	step [i] -= test_vect [j] [i] * de_dx [j];
					}
					norm += step [i] * step [i];
				}
				norm = 1.0 / sqrt (norm);
				for (i = 0; i < n_optimize_vars; i++)
				{	step [i] *= norm;
					d = step [i] * opt_var_step [i] * rel_step_size;
                    if (opt_var_is_var [i])
                    	var_table [opt_var_which [i]].value = best_point [i] + d;
                    else
						parm_list [opt_var_which [i]].dptr [opt_var_index [i]] = best_point [i] + d;
				}
				error = eval_error (gen_later, "trial", "");
				if (trace_opt)
				{	if (error < best_error)
						fprintf (output_file, " accept\n");
					else
						fprintf (output_file, " reject\n");
					fflush (output_file);
				}
#ifdef gui_plop
                DebugMsg ("opt test\n");
                UpdateRunOptStatus (n_evals, rel_step_size);
#endif
				if (error < best_error)
				{	best_error = error;
					corr = 0;

                    /* calculate correlation between this step and previous */

					for (j = 0; j < n_optimize_vars; j++)
						corr += step [j] * test_vect [n_optimize_vars * 2] [j];
					if (debug_opt)
						fprintf (debug_file, "corr: %g\n", corr);
                    /* step_fact is factor to divide current step by */
                    if (corr < 0.01)
                        step_fact = 2;
                    else
                        step_fact = .9 / corr;
					for (j = 0; j < n_optimize_vars; j++)
					{
                    	if (opt_var_is_var [j])
                    		best_point [j] = var_table [opt_var_which [j]].value;
                        else
                    		best_point [j] = parm_list [opt_var_which [j]].dptr [opt_var_index [j]];
					}

                    /* store best point back to parms and vars */
                    for (j = 0; j < n_optimize_vars; j++)
                    {	if (opt_var_is_var [j])
                            var_table [opt_var_which [j]].value = best_point [j];
                        else
                            parm_list [opt_var_which [j]].dptr [opt_var_index [j]] = best_point [j];
                        result_vect [j] = best_point [j];
                    }
#ifdef gui_plop
                    DebugMsg ("opt accept\n");
                    UpdateCellEditOptVars ();
#endif
					if (step_fact > 2.0)
						rel_step_size *= .5;
					else if (step_fact < 0.5)
						rel_step_size *= 2.0;
					else
						rel_step_size /= step_fact;
					if (step_fact > 2.0)
						angle_fact *= 2;
					else if (step_fact < 0.5)
						angle_fact *= 0.5;
					else
						angle_fact *= step_fact;
					if (angle_fact > 3.0)
						angle_fact = 3.0;
					else if (angle_fact < .05)
						angle_fact = .05;
#ifdef foo
					rel_step_size *= 1.1;
#endif
					if (rel_step_size > 1)
						rel_step_size = 1;
					if (debug_opt)
						fprintf (debug_file, "step_fact %g new_step: %g\n", step_fact, rel_step_size);
				}
				else
				{	rel_step_size *= .5;
					angle_fact *= 2;
					if (angle_fact > 3.0)
						angle_fact = 3.0;
				}
			}
			n_evals++;
            if (n_evals % 10 == 0 && have_opt_gr)
            	write_info (opt_gr_file, 0, 0);
            if (n_evals % 10 == 0 && have_opt_phys_gr)
            	write_info (opt_phys_gr_file, 1, 0);
		}
        if(please_stop_plop)
            terminate_plop ();
            
		if (n_evals == max_optimize_evals)
		{	err_msg_d ("Optimizer ran over %d evals, stopped\n", max_optimize_evals);
			prompt_exit ();
		}
		for (j = 0; j < n_optimize_vars; j++)
		{	if (opt_var_is_var [j])
        		var_table [opt_var_which [j]].value = best_point [j];
        	else
        		parm_list [opt_var_which [j]].dptr [opt_var_index [j]] = best_point [j];
			result_vect [j] = best_point [j];
		}
	}
	else
	{	error = eval_error (gen_first, "eval", "\n");
#ifdef gui_plop
        UpdateRunOptStatus (-1, 0.0);
#endif
    }

	if (print_result)
	{	if (!quiet)
			fprintf (output_file, "results:");
		for (i = 0; i < n_scan_set_vars; i++)
		{	if (scan_set_is_var [i])
        	{	if (!quiet)
            		fprintf (output_file, " %s: ", var_table [scan_set_which [i]].name);
            	fprintf (output_file, "%lg", var_table [scan_set_which [i]].value);
            }
            else
			{	if (parm_list [scan_set_which [i]].max_num > 1)
				{	if (!quiet)
						fprintf (output_file, " %s[%d]:", parm_list [scan_set_which [i]].name, scan_set_index [i]);
					fprintf (output_file, " %lg", parm_list [scan_set_which [i]].dptr [scan_set_index [i]]);
				}
				else
				{	if (!quiet)
						fprintf (output_file, " %s:", parm_list [scan_set_which [i]].name);
					fprintf (output_file, " %lg", *parm_list [scan_set_which [i]].dptr);
				}
    		}
        }
		for (i = 0; i < n_scan_vars; i++)
		{	if (scan_var_is_var [i])
        	{	if (!quiet)
            		fprintf (output_file, " %s: ", var_table [scan_var_which [i]].name);
            	fprintf (output_file, "%lg", var_table [scan_var_which [i]].value);
            }
            else
			{	if (parm_list [scan_var_which [i]].max_num > 1)
				{	if (!quiet)
						fprintf (output_file, " %s[%d]:", parm_list [scan_var_which [i]].name, scan_var_index [i]);
					fprintf (output_file, " %lg", parm_list [scan_var_which [i]].dptr [scan_var_index [i]]);
				}
				else
				{	if (!quiet)
						fprintf (output_file, " %s:", parm_list [scan_var_which [i]].name);
					fprintf (output_file, " %lg", *parm_list [scan_var_which [i]].dptr);
				}
            }
        }
		for (i = 0; i < n_optimize_vars; i++)
		{	if(opt_var_is_var [i])
        	{	if (!quiet)
            		fprintf (output_file, " %s: ", var_table [opt_var_which [i]].name);
            	fprintf (output_file, "%lg", var_table [opt_var_which [i]].value);
            }
            else
            {	if (parm_list [opt_var_which [i]].max_num > 1)
				{	if (!quiet)
						fprintf (output_file, " %s[%d]:", parm_list [opt_var_which [i]].name, opt_var_index [i]);
					fprintf (output_file, " %lg", parm_list [opt_var_which [i]].dptr [opt_var_index [i]]);
				}
				else
				{	if (!quiet)
						fprintf (output_file, " %s:", parm_list [opt_var_which [i]].name);
					fprintf (output_file, " %lg", *parm_list [opt_var_which [i]].dptr);
				}
            }
		}
		if (!quiet)
			fprintf (output_file, " err:");
		fprintf (output_file, " %lg\n", error);
        if (n_monte_vars > 0)
        	run_monte ();
	}
#ifndef standalone_grid_gen
	if (have_opt_gr)
    	write_info (opt_gr_file, 0, 0);
    if (have_opt_phys_gr)
    	write_info (opt_phys_gr_file, 1, 0);
#endif

    /* Don't understand why this code was here. Copies original
     * starting point back to the parameters. Doesn't make sense
     * for any reason I can think of, and fucks up the display
     * if the picture is created after Plop terminates optimizing.
     */
/*
	for (i = 0; i < n_optimize_vars; i++)
	{	if (opt_var_is_var [i])
            var_table [opt_var_which [i]].value = starting_point [i];
    	else
        	parm_list [opt_var_which [i]].dptr [opt_var_index [i]] = starting_point [i];
	}
    */

	fflush (stdout);
}

void scan_var_iter (
	int n,
	int have_start,
	double *start_vect,
	double *result_vect)
{	int i;
	int j;
	double x;
	double scan_vect [max_opt_vars];
	int gen_first;
	int gen_later;
	int print_result;

	if (n == n_scan_vars)
	{	if (!have_initial_grid || grid_gen_using_basis && basis_changed)
		{	gen_first = 1;
		}
		else {
			gen_first = 0;
		}
		gen_later = 0;


		run_opt (have_start, start_vect, result_vect, gen_first, gen_later, false);

		have_initial_grid = 1;
	}
	else
	{	if (have_start)
		{	for (i = 0; i < n_optimize_vars; i++)
				scan_vect [i] = start_vect [i];
		}
		for (i = 0; (i < scan_var_nsteps [n] + 1) & !please_stop_plop; i++)
		{	x = scan_var_start [n] + (scan_var_end [n] - scan_var_start [n]) * i / scan_var_nsteps [n];
			if (scan_var_is_var [n])
            {	var_table [scan_var_which [n]].value = x;
            	basis_changed |= var_table [scan_var_which [n]].affects_basis;
            }
            else
            {	parm_list [scan_var_which [n]].dptr [scan_var_index [n]] = x;
				basis_changed |= parm_list [scan_var_which [n]].affects_basis;
			}
            scan_var_iter (n + 1, (i != 0) || have_start, scan_vect, scan_vect);
			if (i == 0)
			{	for (j = 0; j < n_optimize_vars; j++)
					result_vect [j] = scan_vect [j];
			}
		}
        if (please_stop_plop)
            terminate_plop ();
	}
}

void scan_set_var_iter (
	int n)
{	int i;
	double scan_vect [max_opt_vars];

	if (n == n_scan_set_vars)
		scan_var_iter (0, 0, scan_vect, scan_vect);
	else
	{	for (i = 0; i < scan_set_n_values [n] && !please_stop_plop; i++)
		{	if (scan_set_is_var [n])
			{	var_table [scan_set_which [n]].value = scan_set_values [n] [i];
            	basis_changed |= var_table [scan_set_which [n]].affects_basis;
            }
        	else
        	{	parm_list [scan_set_which [n]].dptr [scan_set_index [n]] = scan_set_values [n] [i];
				basis_changed |= parm_list [scan_set_which [n]].affects_basis;
        	}
			scan_set_var_iter (n + 1);
		}
	}
    if (please_stop_plop)
        terminate_plop ();
}

void print_displacement (void) {
	int ipoint;
	double x;
	double y;

	fprintf (debug_file, "mesh displacements: point, x, y, z\n");
	for (ipoint = 0; ipoint < total_mesh_points; ipoint++) {
		polar_to_euc (point_radius [ipoint], point_angle [ipoint], &x, &y);
		fprintf (debug_file, "%d: %lg %lg %lg\n", ipoint, x, y, plate_z_displacement [ipoint]);
	}
}

void run_plate (void)
{	basis_changed = 1;

    if (n_scan_set_vars == 0 && n_scan_vars == 0 && n_optimize_vars == 0 && n_monte_vars == 0)
	{	eval_vars ();

		if (grid_gen_using_basis)
		{	gen_basis_mesh ();

			order_nodes ();

			exec_plate ();

			interpolate_basis ();
		}
		else
		{	gen_mesh (1);

			order_nodes ();

			exec_plate ();
		}
        if (warp_sphere_flag)
        {   add_sphere_distortion ();
        }

		interpolate_tris ();
		calc_rms ();

        evaluate_parts ();


		fprintf (output_file, "raw rms error = %18.10g visible rms = %18.10g\n", plate_err_rms, plate_err_vis_rms);
		fprintf (output_file, "raw p-v error = %18.10lg, visible p-v = %18.10lg\n", plate_err_p_v, plate_err_vis_p_v);

        if (calculate_zernike_flag)
			calculate_zernike ();

		if (debug_displacement) {
			print_displacement ();
		}


		if (refocus_flag)
		{	refocus_mirror ();
			interpolate_tris ();
			calc_rms ();
			fprintf (output_file, "refocused rms error = %18.10g visible rms = %18.10g\n", plate_err_rms, plate_err_vis_rms);
			fprintf (output_file, "refocused p-v error = %18.10lg, visible p-v = %18.10lg\n", plate_err_p_v, plate_err_vis_p_v);
		}
#ifdef gui_plop
        UpdateRunOptStatus (-1, 0.0);
#endif
        if (have_opt_gr)
    		write_info (opt_gr_file, 0, 0);
    	if (have_opt_phys_gr)
    		write_info (opt_phys_gr_file, 1, 0);
		gen_z88_input ();

	}
	else
	{	scan_set_var_iter (0);
		gen_z88_input ();
	}
}
#endif

void add_debug (
	char *s)
{	int i;

	for (i = 0; debug_strings [i] [0] != '\0' && strcmp (debug_strings [i], s); i++)
		;
	if (debug_strings [i] [0] != '\0')
		debugs [i] = 1;
	else
	{	for (i = 0; i < ndebugs; i++)
		debugs [i] = 1;
	}
}

void alloc_globals (void)
{
	max_triangles = max_points * 2;
	point_radius = (double *) malloc_or_die (max_points * sizeof (double), "point_radius");
	point_angle = (double *) malloc_or_die (max_points * sizeof (double), "point_angle");
	point_reordered_num = (int *) malloc_or_die (max_points * sizeof (int), "point_reordered_num");
	point_sorted_order = (int *) malloc_or_die (max_points * sizeof (int), "point_print_order");
	point_sorted_order_temp = (int *) malloc_or_die (max_points * sizeof (int), "point_print_order_temp");
	triangle_points = (tri_int_corners *) malloc_or_die (max_triangles * sizeof (tri_int_corners), "triangle_points");
	triangle_thickness = (double *) malloc_or_die (max_triangles * sizeof (double), "triangle_thickness");
	triangle_pressure = (double *) malloc_or_die (max_triangles * sizeof (double), "triangle_pressure");
	triangle_mass = (double *) malloc_or_die (max_triangles * sizeof (double), "triangle_mass");
	z88_node_loads = (dvec *) malloc_or_die (max_points * sizeof (dvec), "z88_node_loads");
	support_point = (int *) malloc_or_die (max_points * sizeof (int), "support_point");
	support_force = (double *) malloc_or_die (max_points * sizeof (double), "support_force");
	quad_points = (quad_int_corners *) malloc_or_die (max_quads * sizeof (quad_int_corners), "quad_points");

}


void init_plop (void)
{   int i;

	deg_to_rad = 2 * M_PI / degrees;

    tri_fract_inv = 1.0 / tri_fract;
    tri_fract_sq_inv = tri_fract_inv * tri_fract_inv;

	set_max_points (default_max_points);

    total_mem_alloced = 0;

    init_global_graphics ();

	have_initial_grid = 0;

    warp_sphere_flag = 0;
	reuse_best_opt = 1;
	refocus_flag = 1;
    refocus_xyr_flag = 0;
    calculate_zernike_flag = 0;
	grid_gen_using_basis = 1;

    fact_table [0] = 1;
    for (i = 1; i < max_refocus_distortion_order; i++)
        fact_table [i] = fact_table [i - 1] * i;
}
