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

#ifndef __BORLANDC__
#define abs(x)		((x) > 0 ? (x) : -(x))
#endif


#ifdef gui_plop
extern void UpdateCellEditOptVars ();
extern void UpdateRunOptStatus (int, double);
extern void UpdateMonteDisplay (int, double, double);

/* must acquire lock to access support points, mesh, and z displacement */

extern void LockPlopData (int);
extern void UnlockPlopData (void);

extern void DebugMsg (char *);

#endif

double deg_to_rad;

int diameter_found;
double diameter = 100;

int please_stop_plop = 0;

#ifdef gui_plop
extern void terminate_plop (void);
#else
void terminate_plop (void)
{}
#endif

int hole_diameter_found;
double hole_diameter;
double hole_radius = 0;

int rel_hole_diameter_found;
double rel_hole_diameter;

double parab_a0;			/* coefficients of parabola for thickness vs. radius */
double parab_a2;
double parab_a4;

int thickness_found;
double thickness = 15;

int focal_length_found;
double focal_length = 0;

int f_ratio_found;
double f_ratio = 6.0;

int sagitta_found;
double sagitta;

int rel_sagitta_found;           
double rel_sagitta;

/*
 * We'll automatically generate radii of mesh, and which supports are on
 * which ring, but this can be overridden by direct input of the data.
 */

int n_support_radii;

int n_abs_support_radii;
double support_radii [max_support_radii];

int n_rel_support_radii;
double rel_support_radii [max_support_radii];

int n_rel_force;
double rel_force [max_support_radii];

int n_num_support_rings;
int num_support [max_support_radii];				/* number of supports on each ring of supports */

int n_support_angle;
double support_angle [max_support_radii];			/* angle of first support */

int n_support_mesh_ring_found;
int support_mesh_ring [max_support_radii];			/* which mesh ring the support ring is on */

int n_mesh_points_found;
int n_mesh_points [max_mesh_radii];					/* number of points on each ring in mesh */
int mesh_first_point [max_mesh_radii];				/* index number of points on this mesh ring */
double mesh_delta_angle [max_mesh_radii];			/* spacing angle between mesh points */

/*
 * To use the decomposition, specify a basis consisting of the number of support points on the mesh rings,
 * and the minimum mesh ring that the support ring will be used on. Each support ring must be a
 * multiple of the basis size for some ring.
 */

int n_basis_ring_found;
int basis_ring [max_mesh_radii];

int n_basis_min_found;
int basis_min [max_mesh_radii];

int basis_data_alloced = 0;								/* 1 if have allocated data structures for basis gen */

int n_mesh_radii_found;
double mesh_radii [max_mesh_radii];

int n_mesh_rings_found;
int n_mesh_rings;								/* number of rings of triangles in the mesh*/
int n_mesh_depth_found;
int n_mesh_depth;

double mirror_tilt_angle;
int mirror_tilt_angle_found;

double edge_support_sling_angle;
int edge_support_sling_angle_found;
double edge_support_glued_angle;
int edge_support_glued_angle_found;


double sling_included_angle;
int sling_included_angle_found;
int tilt_support_mode;

double modulus;
int modulus_found;

double poisson;
int poisson_found;

int density_found;
double density;

int obstruct_found;
int obstruct_diam_found;
int rel_obstruct_found;

int total_mesh_points;
double *point_radius;	/* [max_points] */
double *point_angle;		/* [max_points] */

/*
 * plate uses banded matrices. we reorder the
 * the nodes to minimize band width.
 */

int *point_reordered_num;	/*[max_points] */		/* number of this point after reordering */
int *point_sorted_order;	/* [max_points] */				/* sequence to print points */
int *point_sorted_order_temp;	/* [max_points] */		/* temp  for sorting */

int n_triangles;
tri_int_corners *triangle_points;	/* int [max_triangles] [tri_pts] */
double *triangle_thickness;			/* [max_triangles] */
double *triangle_pressure;			/* [max_triangles] */
double *triangle_mass;				/* [max_triangles] */
double total_mass;

int n_quads;
quad_int_corners *quad_points;
int n_z88_points;
int n_tetras;

dvec *z88_node_loads;


int n_supports;
int *support_point;				/* [max_points] */
double *support_force;			/* [max_points] */

int basis_changed;			/* set to 1 if need to regenerate basis */

int basis_test_index [max_basis_size] [max_mesh_radii];

/* basis_n_supports is aliased to plate_basis_n_forces */

int basis_n_supports [max_basis_tests];
int *basis_support_points [max_basis_tests];
double *basis_support_force [max_basis_tests];

int part_type_num_corners [2] = {3, 2};

int n_parts;
int part_type [max_parts];
int part_quantity [max_parts];
int part_point_type [max_parts] [tri_pts];
int part_ring_num [max_parts] [tri_pts];
int part_point_num [max_parts] [tri_pts];
double part_corner_x [max_parts] [tri_pts];        /* absolute */
double part_corner_y [max_parts] [tri_pts];
double part_corner_radius [max_parts] [tri_pts];        /* absolute */
double part_corner_angle [max_parts] [tri_pts];
double part_cg_x [max_parts];
double part_cg_y [max_parts];
double part_cg_radius [max_parts];
double part_cg_angle [max_parts];
double part_total_force [max_parts];
double part_draw_x [max_parts] [tri_pts];
double part_draw_y [max_parts] [tri_pts];
double part_draw_cg_x [max_parts];
double part_draw_cg_y [max_parts];
int part_rotation [max_parts];

var_def var_table [max_variables];

int n_variables;

int gr_info_alloced = 0;

// we suck the plop file to/from this buffer so we have access to it
// as a bunch of chars so can also swap with the memo in gui_plop

// static declaration of it is not a great thing.

char plop_text_file_buff [max_plop_file_len];
char *plop_text_file_ptr;
int plop_text_file_len;

// comments without leading ';'

char plop_comment_file_buff [max_comments_len];
int plop_comment_len;


int verbose = 0;
int trace_opt = 0;
int quiet = 0;

int use_p_v_error = 0;

int n_optimize_vars;
int n_scan_vars;
int n_scan_set_vars;
int n_monte_vars;

int n_optimize_steps = 10;
int max_optimize_evals = 5000;
double optimize_delta_f_too_small = 1e-9;
double optimize_min_delta = 1e-4; 

/*
 * Changing map of supports to mesh rings can cause discontinuities.
 * Optionally disallow this, but user must be careful not to span a large range.
 */

int grid_gen_using_basis;
int have_initial_grid;


int GridMeshFlags [] = {grid_gen_never, grid_gen_basis};


int opt_var_which [max_opt_vars];		/* which variable, either predefined or a var */
int opt_var_index [max_opt_vars];		/* index into set */
int opt_var_is_var [max_opt_vars];		/* 1 if optimization refers to var, 0 for predefined */
double opt_var_step [max_opt_vars];

int scan_var_which [max_scan_vars];
int scan_var_index [max_scan_vars];
double scan_var_start [max_scan_vars];
double scan_var_end [max_scan_vars];
int scan_var_nsteps [max_scan_vars];
int scan_var_is_var [max_scan_vars];

int scan_set_which [max_scan_set_vars];
int scan_set_index [max_scan_set_vars];
int scan_set_n_values [max_scan_set_vars];
double scan_set_values [max_scan_set_vars] [max_scan_set];
int scan_set_is_var [max_scan_set_vars];

int monte_var_which [max_scan_vars];
int monte_var_index [max_scan_vars];
double monte_var_delta [max_scan_vars];
int monte_var_is_var [max_scan_vars];

int n_monte_tests = 100;

int reuse_best_opt = 1;

#ifndef standalone_grid_gen
char picfile [max_line_len];
char datfile [max_line_len];
char grfile [max_line_len];
char plotfile [max_line_len];
char contourplotfile [max_line_len];
char opt_gr_file [max_line_len];
char opt_phys_gr_file [max_line_len];
int have_plot = 0;
int have_contour_plot = 0;
int have_pic = 0;
int have_pic_mesh = 0;
int have_dat = 0;
int have_gr = 0;
int have_opt_gr = 0;
int have_opt_phys_gr = 0;
#endif

FILE *output_file;
FILE *debug_file;

char z88_struct_file_name [1000];
char z88_load_file_name [1000];
char z88_deflection_file_name [1000];

#define toggle(x)		x = 1 - x;



void *malloc_or_die (int, char const *);


//double rint (double x)
//{	return (floor (x + .5));
//}


extern void err_msg (char const *msg);
extern void warn_msg (char const *msg);
extern void prompt_exit (void);

typedef struct {
	double x_cg, y_cg;
	double rel_force;
	int n_components;		/* cell nodes if a point, other loads if a part */
	int component_list [4];
	double component_rel_force [4];
	int component_index;		/* which ring or part */
	int component_sub_index;		/* which point or part on the ring */
	bool is_part;
	bool is_primary;				/* true if this is one of the primary supports for the cell */
	} t_load_descr;

int max_loads;
t_load_descr *cell_loadings;
int n_loads;
int point_load_index [max_mesh_radii];		/* index into cell_loadings for first point on a ring */
int part_load_index [max_mesh_radii];		/* index into cell_loadings for first part in a ring */
int primary_load_index [n_deg_free];

void set_max_points (int n)
{	max_points = n;
	max_triangles = max_points * 2;
	max_quads = n;			/* merge pairs of triangles into quads */
//	max_z88_points = max_points * max_z88_depth;
}

void alloc_plate_globals ()
{	int i;

	plate_point_x = (double *) malloc_or_die (max_points * sizeof (double),
			"plate_point_x");
	plate_point_y = (double *) malloc_or_die (max_points * sizeof (double),
			"plate_point_y");
	plate_forces = (double *) malloc_or_die (max_points * sizeof (double),
			"plate_forces");
	plate_force_points = (int *) malloc_or_die (max_points * sizeof (int),
			"plate_force_points");
	plate_triangle_points = (tri_int_corners *) malloc_or_die (max_triangles * sizeof (tri_int_corners),
			"plate_force_points");
	plate_tri_x_cg = (tri_dbl_corners *) malloc_or_die (max_triangles * sizeof (tri_dbl_corners),
			"plate_tri_x_cg");
	plate_tri_y_cg = (tri_dbl_corners *) malloc_or_die (max_triangles * sizeof (tri_dbl_corners),
			"plate_tri_y_cg");
	plate_thickness = (double *) malloc_or_die (max_triangles * sizeof (double),
			"plate_triangles");
	plate_tri_pressure = (double *) malloc_or_die (max_triangles * sizeof (double),
			"plate_tri_pressure");
	plate_area = (double *) malloc_or_die (max_triangles * sizeof (double),
			"plate_area");
	plate_mx = (double *) malloc_or_die (max_triangles * sizeof (double),
	  		"plate_mx");
	plate_my = (double *) malloc_or_die (max_triangles * sizeof (double),
			"plate_my");
	plate_mxy = (double *) malloc_or_die (max_triangles * sizeof (double),
			"plate_mxy");
	plate_xcg = (double *) malloc_or_die (max_triangles * sizeof (double),
			"plate_xcg");
	plate_ycg = (double *) malloc_or_die (max_triangles * sizeof (double),
			"plate_ycg");
	plate_rhs = (double *) malloc_or_die (max_eqn * sizeof (double),
			"plate_rhs");
	plate_result = (double *) malloc_or_die (max_eqn * sizeof (double),
			"plate_result");
	plate_z_displacement = (double *) malloc_or_die (max_points * sizeof (double),
			"plate_z_displacement");
	plate_plot_point_x = (double *) malloc_or_die (max_points * sizeof (double),
			"plate_plot_point_x");
 	plate_plot_point_y = (double *) malloc_or_die (max_points * sizeof (double),
			"plate_plot_point_y");
	plate_plot_triangle_points = (tri_int_corners *) malloc_or_die (max_triangles * sizeof (tri_int_corners),
			"plate_plot_force_points");
	plate_plot_z_displacement = (double *) malloc_or_die (max_points * sizeof (double),
			"plate_plot_z_displacement");
 	z88_plot_point_x = (double *) malloc_or_die (max_points * sizeof (double),
			"z88_plot_point_x");
	z88_plot_point_y = (double *) malloc_or_die (max_points * sizeof (double),
			"z88_plot_point_y");
	z88_plot_triangle_points = (tri_int_corners *) malloc_or_die (max_triangles * sizeof (tri_int_corners),
			"z88_plot_force_points");
	z88_plot_z_displacement = (double *) malloc_or_die (max_points * sizeof (double),
			"z88_plot_z_displacement");

	for (i = 0; i < max_refocus_distortion_order; i++) {
		tmp_function_vals [i] = (double *) NULL;
	}

	max_loads = ((max_mesh_radii + max_parts) * max_points_per_ring);
	cell_loadings = (t_load_descr *) malloc_or_die (max_loads * sizeof (t_load_descr), "cell_loadings");
	
}

/* if in gui_plop must own lock on plop data to enter */

void create_triangle (
	int p1, int p2, int p3)
{	int i;
	int point_set [tri_pts];
	double tri_x [tri_pts];
	double tri_y [tri_pts];
	double center_x;
	double center_y;
	double area;
	double thick;

	if (debug_triangle)
		fprintf (debug_file, "create tri %d %d %d %d\n", n_triangles, p1, p2, p3);
	if (n_triangles == max_triangles)
	{	err_msg_d ("Too many triangles, max is %d\n", max_triangles);
		prompt_exit ();
	}
	point_set [0] = p1;
	point_set [1] = p2;
	point_set [2] = p3;

	thick = 0.0;
	center_x = 0.0;
	center_y = 0.0;
	for (i = 0; i < tri_pts; i++)
	{	triangle_points [n_triangles] [i] = point_set [i];
		polar_to_euc (point_radius [point_set [i]], point_angle [point_set [i]], tri_x + i, tri_y + i);
		center_x += tri_x [i];
		center_y += tri_y [i];
		thick += parab_a0 + parab_a2 * point_radius [point_set [i]] * point_radius [point_set [i]];
	}
	thick = thick / 3.0;
	center_x = center_x / 3.0;
	center_y = center_y / 3.0;
	area = 0.5 * (
		(tri_x [0] - center_x) * (tri_y [1] - center_y) +
		(tri_x [1] - center_x) * (tri_y [2] - center_y) +
		(tri_x [2] - center_x) * (tri_y [0] - center_y) -
		(tri_x [0] - center_x) * (tri_y [2] - center_y) -
		(tri_x [1] - center_x) * (tri_y [0] - center_y) -
		(tri_x [2] - center_x) * (tri_y [1] - center_y));
	triangle_pressure [n_triangles] = density * thick;
	triangle_thickness [n_triangles] = thick;
	triangle_mass [n_triangles] = density * thick * area;
	total_mass += triangle_mass [n_triangles];
	n_triangles++;
}

/* if in gui_plop must own lock on plop data to enter */

void compute_geom (void)
{	int i;

	if (f_ratio_found)
		focal_length = f_ratio * diameter;

	if (rel_sagitta_found)
		sagitta = rel_sagitta * thickness;

	radius = diameter * .5;

	if (rel_sagitta_found || sagitta_found)
		parab_a2 = sagitta / (radius * radius);
	else
    	parab_a2 = 1.0 / (4.0 * focal_length);
    parab_a4 = parab_a2 * parab_a2 * parab_a2;
    plate_parab_a2 = parab_a2;
	plate_focal_length = focal_length;

	if (verbose)
	{	printf ("diameter = %lg\n", diameter);
		if (focal_length_found || f_ratio_found)
        	printf ("focal length = %lg\n", focal_length);
	}

    if (rel_hole_diameter_found)
    	hole_diameter = diameter * rel_hole_diameter;
    if (rel_hole_diameter_found || hole_diameter_found)
    	hole_radius = hole_diameter * .5;
    else
        hole_radius = 0;

	parab_a0 = thickness - parab_a2 * radius * radius;

    if (obstruct_diam_found)
		obstruction_radius = obstruction_diam * 0.5;

	if (rel_obstruct_found)
		obstruction_radius = radius * rel_obstruction_radius;

	if (n_rel_support_radii > 0)
	{	n_support_radii = n_rel_support_radii;
		for (i = 0; i < n_rel_support_radii; i++)
			support_radii [i] = rel_support_radii [i] * radius;
	}
    else
    	n_support_radii = n_abs_support_radii;

	if (debug_grid_gen)
		fprintf (debug_file, "parab a0 = %lg a2 = %lg\n", parab_a0, parab_a2);
}


/* create a triangle from points indexed by radius and angle on the mesh, and allow
 * indices past the number of points on the ring.
 */

void gen_tri_from_ring_angle_idx (int r0, int a0, int r1, int a1, int r2, int a2) {
	a0 = a0 % n_mesh_points [r0];
	a1 = a1 % n_mesh_points [r1];
	a2 = a2 % n_mesh_points [r2];

	create_triangle (
		mesh_first_point [r0] + a0,
		mesh_first_point [r1] + a1,
		mesh_first_point [r2] + a2);
}


void generate_triangles (void)
{
	int iring;
	int j;
	int n_inner;
	int n_outer;
	int i_outer;
	int i_outer_1;
	int i_inner;
	int i_inner_1;
	int p0, p1, p2;
	int a;
	int outer_pt [max_mesh_perim];

	n_triangles = 0;
	total_mass = 0.0;

	for (iring = 0; iring < n_mesh_rings - 1; iring++)
	{	n_inner = n_mesh_points [iring];
		n_outer = n_mesh_points [iring + 1];
		for (i_outer = 0; i_outer < n_outer; i_outer++) {
			i_outer_1 = i_outer + 1;
			i_inner = (i_outer * n_inner + n_outer / 2) / n_outer;
			i_inner_1 = (i_outer_1 * n_inner + n_outer / 2) / n_outer;
			gen_tri_from_ring_angle_idx (iring, i_inner, iring + 1, i_outer, iring + 1, i_outer_1);
			if (i_inner_1 > i_inner && n_inner > 1) {
				gen_tri_from_ring_angle_idx (iring, i_inner_1, iring, i_inner, iring + 1, i_outer_1);
			}


		}
	}
	if (verbose)
	{	printf ("number of triangles: %d\n", n_triangles);
		printf ("total mass: %20.12g\n", total_mass);
	}
}
void generate_triangles_old (void)
{
	int i;
	int j;
	int n_inner;
	int n_outer;
	int p0, p1, p2;
	int a;
	int outer_pt [max_mesh_perim];


	if (n_triangles > 0) {
		return;
	}

	n_triangles = 0;
	total_mass = 0.0;

	for (i = 0; i < n_mesh_rings - 1; i++)
	{	n_inner = n_mesh_points [i];
		n_outer = n_mesh_points [i + 1];

		if (n_inner == n_outer)
		{
			/* for equal inner and outer rings generate 4 triangles with pairs symmetry about j+1 */
			for (j = 0; j < n_outer; j += 2) {
				gen_tri_from_ring_angle_idx (i, j, i + 1, j, i + 1, j + 1);
				gen_tri_from_ring_angle_idx (i + 1, j + 1, i, j + 1, i, j);
				gen_tri_from_ring_angle_idx (i, j + 2, i, j + 1, i + 1, j + 1);
				gen_tri_from_ring_angle_idx (i + 1, j + 1, i + 1, j + 2, i, j + 2);
			}
		} else {
			{	if (n_inner > 1) {
					for (j = 0; j < n_inner; j++) {
						a = (int) rint ((j + 0.5) * n_outer / n_inner);
						gen_tri_from_ring_angle_idx (i, j + 1, i, j, i + 1, a);
						outer_pt [j] = a;
					}
				}
				p2 = 0;
				for (j = 0; j < n_outer; j++) {
					if (n_inner > 1 && j == outer_pt [p2] ) {
						p2++;
						if (p2 == n_inner) {
							p2 = 0;
						}
					}
					gen_tri_from_ring_angle_idx (i + 1, j, i + 1, j + 1, i, p2);
				}
#ifdef foo
				for (j = 0; j < n_outer; j += 2) {
					gen_tri_from_ring_angle_idx (i + 1, j, i + 1, j + 1, i, j / 2);
					gen_tri_from_ring_angle_idx (i + 1, j + 1, i + 1, j + 2, i, j / 2 + 1);
				}
#endif
			}
		}
	}
	if (verbose)
	{	printf ("number of triangles: %d\n", n_triangles);
		printf ("total mass: %20.12g\n", total_mass);
	}
}


void gen_mesh (
	int map_supports_to_rings)
{	int i;
	int j;
	int r;
	int r_prev;
	int err_found;
	int snum;
	int last_rnum;
	int next_rnum;
	double r_inner;
	double r_outer;
	int pmin;
	int psmin;
	int rnum;
	int fp_inner;
	double total_rel_force;
	double force;

	compute_geom ();

	/*
	 * user can supply radii of mesh rings in which case everything must
	 * be supplied correctly or we'll fail.
	 */

	err_found = 0;
	r_prev = -1;
	if (n_support_mesh_ring_found == 0 && map_supports_to_rings)
	{	for (i = 0; i < n_support_radii; i++)
		{	if (support_radii [i] < hole_radius || support_radii [i] > radius)
			{	err_msg_g ("Bad radius for support: %lg\n", support_radii [i]);
				err_found = 1;
                prompt_exit ();
			}
			r = (int) rint ((support_radii [i] - hole_radius) / (radius - hole_radius) * (n_mesh_rings - 1));

			/* make sure don't use ring on the edge unless at the exact edge */

			if (r == n_mesh_rings - 1 && support_radii [i] < radius)
				r = n_mesh_rings - 2;
			else if (r == 0 && support_radii [i] > 0)
				r = 1;
			if (r == r_prev)
			{	err_msg ("Supports are too close for this number of mesh rings, increase mesh rings\n");
				err_found = 1;
			}
			support_mesh_ring [i] = r;
			r_prev = r;
			if (n_support_angle == 0)
            {	support_angle [i] = 0;
            }
            else if (support_angle [i] >= (double) degrees / num_support [i])
			{	support_angle [i] = fmod (support_angle [i], (double) degrees / num_support [i]);
			}
		}
	}

	for (i = 0; i < n_support_radii; i++)
	{	mesh_radii [support_mesh_ring [i]] = support_radii [i];
	}

	if (debug_grid_gen)
	{	fprintf (debug_file, "supports on mesh rings:");
		for (i = 0; i < n_support_radii; i++)
			fprintf (debug_file, " %d", support_mesh_ring [i]);
		fprintf (debug_file, "\n");
	}
	if (n_mesh_radii_found == 0)
	{	mesh_radii [0] = hole_radius;
		last_rnum = 0;
		snum = 0;
		while (last_rnum < n_mesh_rings - 1)
		{	r_inner = mesh_radii [last_rnum];
			if (snum == n_support_radii)
			{	next_rnum = n_mesh_rings - 1;
				r_outer = radius;
			}
			else
			{	next_rnum = support_mesh_ring [snum];
				r_outer = support_radii [snum];
			}
			for (i = 1; i <= next_rnum - last_rnum; i++)
			{	mesh_radii [i + last_rnum] = i * (r_outer - r_inner) / (next_rnum - last_rnum) + mesh_radii [last_rnum];
			}
			last_rnum = next_rnum;
			snum++;
		}
	}
	if (debug_grid_gen)
	{	fprintf (debug_file, "mesh radii:");
		for (i = 0; i < n_mesh_rings; i++)
			fprintf (debug_file, " %13.6g", mesh_radii [i]);
		fprintf (debug_file, "\n");
	}
	if (n_support_radii != n_num_support_rings ||
    		(n_support_angle != 0 && n_support_radii != n_support_angle))
	{	err_msg ("Inconsistent number of number / angles / radii of supports\n");
		err_found = 1;
	}
	if (n_mesh_points_found != 0 && n_mesh_points [0] != 1)
	{	err_msg ("The number of points in inner ring must be 1\n");
	}

	total_mesh_points = 0;

    /*
     * snum keeps track of which support ring we will run into next as we scan
     * across all mesh rings.
     */
	if (support_radii [0] == 0)
		snum = 1;
	else
		snum = 0;
	for (i = 0; i < n_mesh_rings; i++)
	{	if (n_mesh_points_found == 0)
		{

        	/*
            pmin = 6 * n_mesh_rings *
        		(hole_radius + i * (radius - hole_radius) / n_mesh_rings) / radius;
            */
            pmin = (int) (mesh_radii [i] / radius * 6 * (n_mesh_rings - 1));
        	if (pmin == 0)
            	pmin = 1;
        	if (i > 0 && pmin < n_mesh_points [i - 1])
				pmin = n_mesh_points [i - 1];
			if (pmin > 1)
            {	for (j = 0; (6 << j) < pmin; j++)
					;
				pmin = 6 << j;
            }
			if (snum < n_support_radii && i == support_mesh_ring [snum])
			{	/* make triangles a multiple of the number of supports */
            	pmin = (pmin + num_support [snum] - 1) / num_support [snum] * num_support [snum];
            	if (support_angle [snum] != 0)
				{	psmin = degrees / support_angle [snum];
					if (psmin > pmin || pmin % psmin != 0)
					{	pmin = psmin * ((pmin + psmin - 1) / psmin);
					}
				}
				snum++;
			}
			n_mesh_points [i] = pmin;
		}
		mesh_first_point [i] = total_mesh_points;
		if (total_mesh_points + n_mesh_points [i] > max_points)
		{	err_msg_d ("Too many points, max is %d\n", max_points);
			prompt_exit ();
		}
		mesh_delta_angle [i] = (double) degrees / n_mesh_points [i];
		for (j = 0; j < n_mesh_points [i]; j++)
		{	point_radius [j + total_mesh_points] = mesh_radii [i];
			point_angle [j + total_mesh_points] = j * mesh_delta_angle [i];
		}
		total_mesh_points += n_mesh_points [i];
	}
	if (verbose)
		printf ("total number of mesh points: %d\n", total_mesh_points);
	if (debug_grid_gen)
	{	fprintf (debug_file, "number of mesh points:");
		for (i = 0; i < n_mesh_rings; i++)
			fprintf (debug_file, " %d", n_mesh_points [i]);
		fprintf (debug_file, "\n");
	}

	generate_triangles ();

	if (n_rel_force == 0)
	{	for (i = 0; i < n_support_radii; i++)
			rel_force [i] = 1.0;
	}
	total_rel_force = 0.0;
	for (i = 0; i < n_support_radii; i++)
		total_rel_force += num_support [i] * rel_force [i];
	n_supports = 0;

	/* note start supports at ring 1 in case force at 0; we ignore this since
     * this is where the constraints are placed.
     */

	for (i = 0; i < n_support_radii; i++)
	{	rnum = support_mesh_ring [i];
    	if (rnum != 0 || hole_diameter_found || rel_hole_diameter_found)
        {	fp_inner = mesh_first_point [rnum];
			force = rel_force [i] * total_mass / total_rel_force;
			for (j = 0; j < num_support [i]; j++)
			{	support_point [n_supports] = fp_inner + support_angle [i] / mesh_delta_angle [rnum] +
					j * n_mesh_points [rnum] / num_support [i];
				support_force [n_supports] = force;
				n_supports++;
            }
		}
	}
    basis_n_supports [0] = n_supports;
	if (err_found)
		prompt_exit ();

}

int gcd (
	int x,
	int y)
{	while (x != 0 && y != 0)
	{	if (x > y)
			x -= (x / y) * y;
		else
			y -= (y / x) * x;
	}
	return (x + y);
}

/*
 * Init all basis vectors to statically allocated data in case we are not using basis.
 * A bit of a hack, but result of the way the program evolved.
 */

void init_default_basis (void)
{	plate_n_basis_tests = 1;
	basis_support_points [0] = support_point;
	basis_support_force [0] = support_force;
}


void gen_basis_mesh (void)
{	int i;
	int j;
	int k;
	int pmin;
	int smin;
	int b_test;


	compute_geom ();

	if (n_basis_ring_found != n_basis_min_found)
	{	err_msg ("Mismatch between n_basis_ring and n_basis_min\n");
		prompt_exit ();
	}
	total_mesh_points = 0;
	for (i = 0; i < n_mesh_rings; i++)
		mesh_radii [i] = hole_radius + i * (radius - hole_radius) / (n_mesh_rings - 1);
	for (i = 0; i < n_mesh_rings; i++)
	{
    	pmin = mesh_radii [i] / radius * 6 * (n_mesh_rings - 1);
        if (pmin == 0)
           	pmin = 1;
        if (pmin > 1)
        {	for (j = 0; (6 << j) < pmin; j++)
				;
        	pmin = 6 << j;
        }
		smin = 1;
		for (j = 0; j < n_basis_min_found; j++)
		{	if (basis_min [j] <= i && (i > 0 || hole_radius > 0))
			{	smin = (smin * basis_ring [j]) / gcd (smin, basis_ring [j]);
			}
		}
		pmin = smin * ((pmin + smin - 1) / smin);
		if (pmin > 1 && (pmin & 1)) {
			pmin *= 2;
		}
		n_mesh_points [i] = pmin;
		mesh_first_point [i] = total_mesh_points;
		if (total_mesh_points + n_mesh_points [i] > max_points)
		{	err_msg_d ("Too many points, max is %d\n", max_points);
			prompt_exit ();
		}
		mesh_delta_angle [i] = (double) degrees / n_mesh_points [i];
		for (j = 0; j < n_mesh_points [i]; j++)
		{	point_radius [j + total_mesh_points] = mesh_radii [i];
			point_angle [j + total_mesh_points] = j * mesh_delta_angle [i];
		}
		total_mesh_points += n_mesh_points [i];
	}

	if (verbose)
		printf ("total number of mesh points: %d\n", total_mesh_points);
	if (debug_grid_gen)
	{	fprintf (debug_file, "number of mesh points:");
		for (i = 0; i < n_mesh_rings; i++)
			fprintf (debug_file, " %d", n_mesh_points [i]);
		fprintf (debug_file, "\n");
	}

	n_triangles = 0;
	total_mass = 0.0;

	generate_triangles ();

    plate_n_basis_tests = 0;
    for (i = 0; i < n_basis_min_found; i++)
    {	for (j = 0; j < n_mesh_rings; j++)
        {	if (basis_min [i] <= j)
                basis_test_index [i] [j] = plate_n_basis_tests++;
            else
                basis_test_index [i] [j] = -99999999;		/* good chance of core dump for bad ref. */
        }
    }
    for (i = 0; i < plate_n_basis_tests; i++)
    {	if (plate_basis_rhs [i] == NULL)
            plate_basis_rhs [i] = (double *) malloc_or_die (max_points * n_deg_free * sizeof (double),
                "basis_rhs");
    }

    for (i = 0; i < n_basis_min_found; i++)
    {	for (j = 0; j < n_mesh_rings; j++)
        {	b_test = basis_test_index [i] [j];
            if (basis_min [i] <= j)
            {	if (j == 0 && n_mesh_points [j] == 1)
                    basis_n_supports [b_test] = 0;
                else
                {	basis_n_supports [b_test] = basis_ring [i];
                    if (basis_support_points [b_test] == NULL)
                        basis_support_points [b_test] =
                            (int *) malloc_or_die (max_points_per_ring * sizeof (int), "basis_support_points");
                    if (plate_basis_force_points [b_test] == NULL)
                        plate_basis_force_points [b_test] =
                            (int *) malloc_or_die (max_points_per_ring * sizeof (int), "plate_basis_force_points");
                    if (basis_support_force [b_test] == NULL)
                        basis_support_force [b_test] =
                            (double *) malloc_or_die (max_points_per_ring * sizeof (double), "basis_support_force");
                    if (plate_basis_forces [b_test] == NULL)
                        plate_basis_forces [b_test] =
                            (double *) malloc_or_die (max_points_per_ring * sizeof (double), "plate_basis_forces");
                }
            }
        }
    }
    for (i = 0; i < n_basis_min_found; i++)
    {	for (j = 0; j < n_mesh_rings; j++)
        {	b_test = basis_test_index [i] [j];
            if (basis_min [i] <= j)
            {	for (k = 0; k < basis_ring [i]; k++)
                {	basis_support_points [b_test] [k] = mesh_first_point [j] +
                        n_mesh_points [j] * k / basis_ring [i];
                    basis_support_force [b_test] [k] =
                        total_mass / basis_ring [i];
                }
                if (debug_basis_gen)
                {	fprintf (debug_file, "basis supports mesh %d radius %d index %d supports: %d",
                        i, j, b_test, basis_n_supports [b_test]);
                    for (k = 0; k < basis_n_supports [b_test]; k++)
                    {	fprintf (debug_file, " [%d]:pt: %d r:%g a:%g f:%g", k, basis_support_points [b_test] [k],
                            point_radius [basis_support_points [b_test] [k]],
                            point_angle [basis_support_points [b_test] [k]],
                            basis_support_force [b_test] [k]);
                    }
                    fprintf (debug_file, "\n");
                }
            }
        }
    }
	if (verbose)
		printf ("basis size: %d\n", plate_n_basis_tests);
}

/* return 1 if key i < key j */

int key_cmp_lt (
	int i,
	int j)
{
	if (point_radius [i] == 0)
		return (0);
	else if (point_radius [j] == 0)
		return (1);
	if (point_angle [i] < point_angle [j])
		return (1);
	else if (point_radius [i] > point_radius [j])
		return (1);
		return (0);
}

void sort_nodes (
	int low,
	int high,
	int *a,
	int *a_temp)
{	int i, m;
	int jl, jh;

	if (high == low + 1)
		return;
	m = (high + low) / 2;
	sort_nodes (low, m, a, a_temp);
	sort_nodes (m, high, a, a_temp);
	for (jl = low, jh = m, i = low; i < high; i++)
	{	if (jl < m && (jh == high || key_cmp_lt (a [jl], a [jh])))
		{	a_temp [i] = a [jl];
			jl++;
		}
		else
		{	a_temp [i] = a [jh];
			jh++;
		}
	}
	for (i = low; i < high; i++)
	{	a [i] = a_temp [i];
	}
}

void order_nodes (void)
{	int i;

	for (i = 0; i < total_mesh_points; i++)
		point_sorted_order [i] = i;

	sort_nodes (0, total_mesh_points, point_sorted_order, point_sorted_order_temp);

	for (i = 0; i < total_mesh_points; i++)
	{	point_reordered_num [point_sorted_order [i]] = i;
	}
#ifdef foo
	for (i = 0; i < total_mesh_points; i++)
	{	point_reordered_num [i] = total_mesh_points - i - 1;
		point_sorted_order [i] = total_mesh_points - i - 1;
	}
#endif
}
void generate_quad (int iquad, int itri) {
	quad_int_corners tq;
	int itri1;
	int it1, it1_1, it2, it2_1, it2_2;
	int iq;
	int it;

	/* both tri1 and tri2 go clockwise. find a segment of
	 * t2 that has same points as a segment in t1. However since
	 * they are both clockwise the points will be in opposite order.
	 * when found, the extra point in t2 goes inbetween the two points in t1.
	 */
	itri1 = itri + 1;
	for (it1 = 0; it1 < tri_pts; it1++) {
		it1_1 = (it1 + 1) % tri_pts;
		for (it2 = 0; it2 < tri_pts; it2++) {
			it2_1 = (it2 + 1) % tri_pts;
			if (triangle_points [itri] [it1] == triangle_points [itri1] [it2_1] &&
				triangle_points [itri] [it1_1] == triangle_points [itri1] [it2])
				break;
		}
		if (it2 != tri_pts)
			break;
	}
	if (it1 != tri_pts) {
		it2_2 = (it2 + 2) % tri_pts;
		for (iq = 0, it = 0; iq < quad_pts;) {
			quad_points [iquad] [iq++] = triangle_points [itri] [it];
			if (it == it1) {
				quad_points [iquad] [iq++] = triangle_points [itri1] [it2_2];
			}
			it++;
		}
	}


}

/* find 3D location of a z88 node */

void get_z88_node_location (
	int ipoint,
	double *location)
{	int i_tri_point;	/* point on the 2D mesh */
	int izdepth;
	double zdepth;

	i_tri_point = ipoint % total_mesh_points;
	izdepth = ipoint / total_mesh_points;
	zdepth = parab_a0 + parab_a2 * point_radius [i_tri_point] * point_radius [i_tri_point];
	location [dim_z] = zdepth * izdepth / (double) (n_mesh_depth - 1);
	polar_to_euc (point_radius [i_tri_point], point_angle [i_tri_point], &(location [dim_x]), &(location [dim_y]));
}


void calculate_z88_nodal_moments (
	double *m0,
	double m1 [n_deg_free] [n_deg_free],
	double *torque)
{	double node_loc [n_deg_free];
	int inode;
	int idim;
	int jdim;


	/* m1 [i] [j] is momemnt of [xi] [f_xj] */

	for (idim = 0; idim < n_deg_free; idim++) {
		m0 [idim] = 0;
		for (jdim = 0; jdim < n_deg_free; jdim++) {
			m1 [idim] [jdim]= 0;
		}
	}
	for (inode = 0; inode < n_z88_points; inode++) {
		get_z88_node_location (inode, node_loc);
		for (idim = 0; idim < n_deg_free; idim++) {
			m0 [idim] += z88_node_loads [inode] [idim];
			for (jdim = 0; jdim < n_deg_free; jdim++) {
				m1 [idim] [jdim] += node_loc [jdim] * z88_node_loads [inode] [idim];
			}
		}
	}
	torque [dim_x] = m1 [dim_z] [dim_y] - m1 [dim_y] [dim_z];
	torque [dim_y] = m1 [dim_x] [dim_z] - m1 [dim_z] [dim_x];
	torque [dim_z] = m1 [dim_y] [dim_x] - m1 [dim_x] [dim_y];

}



void write_z88_structure (void) {
	FILE *ofile;
	int ipoint;
	int i_tri_point;
	int iquad;
	int itetra;
	int izdepth;
	int iz_base_depth;
	int it_point_idx;
	int it_point;
	int iqpoint;
	double zdepth;
	double p_loc [n_deg_free];

	ofile = fopen (z88_struct_file_name, "wb");
	if (ofile == NULL) {
		err_msg_s ("can't open z88 file %s", z88_struct_file_name);
		prompt_exit ();
	}
	fprintf (ofile, "%d %d %d %d 1 0 0 0 0\n",
		n_deg_free, n_z88_points, n_tetras, n_z88_points * n_deg_free);
	for (ipoint = 0; ipoint < n_z88_points; ipoint++) {
#ifdef foooo
		i_tri_point = ipoint % total_mesh_points;
		izdepth = ipoint / total_mesh_points;
		zdepth = parab_a0 + parab_a2 * point_radius [i_tri_point] * point_radius [i_tri_point];
		pz = zdepth * izdepth / (double) (n_mesh_depth - 1);
		polar_to_euc (point_radius [i_tri_point], point_angle [i_tri_point], &px, &py);
		fprintf (ofile, "%d %d %g %g %g\n", ipoint + 1, n_deg_free,
			px, py, pz);
#endif
		get_z88_node_location (ipoint, p_loc);
		fprintf (ofile, "%d %d %g %g %g\n", ipoint + 1, n_deg_free,
			p_loc [dim_x], p_loc [dim_y], p_loc [dim_z]);

	}
	for (itetra = 0; itetra < n_tetras; itetra++) {
		iquad = itetra % n_quads;
		iz_base_depth = itetra / n_quads;
		fprintf (ofile, "%d 1\n", itetra + 1);
		for (it_point_idx = 0; it_point_idx < tetra_pts; it_point_idx++) {
			izdepth = 1 - it_point_idx / quad_pts;
			iqpoint = quad_points [iquad] [it_point_idx % quad_pts];
			it_point = (izdepth + iz_base_depth) * total_mesh_points + iqpoint + 1;
			fprintf (ofile, "%8d", it_point);
		}
		fprintf (ofile, "\n");
	}
	fprintf (ofile, "1 %d %g %g 2 0\n", n_tetras, modulus, poisson);
	fclose (ofile);
}


/* generate wieghtings of node points to cg of the mirror.*/

void generate_node_loads (void)
{	int i;
	int j;

	int best_basis_ring;
	int n_best_basis_ring;

	int r_inner, r_outer;
	int r_sum;
	double w_inner, w_outer;
	double angle;
	int a_first, a_next;
	double wa_first, wa_next;
	double total_rel_force;
	double force;
	double tilt_sin;
	double tilt_cos;
	t_load_descr *lp;
	double total_load_force;
	int icomp;
	double px, py;
	if (n_rel_force == 0)
	{	for (i = 0; i < n_support_radii; i++)
			rel_force [i] = 1.0;
	}
	total_rel_force = 0.0;
	for (i = 0; i < n_support_radii; i++)
		total_rel_force += num_support [i] * rel_force [i];

	n_loads = 0;
	for (i = 0; i < n_support_radii; i++)
	{    point_load_index [i] = n_loads;

		/* find the best basis ring to use for the support ring.
		 * This is one with minimum points, and usable outside the support ring.
		 */


		force = rel_force [i] / total_rel_force;
		r_inner = floor ((support_radii [i] - hole_radius) * (n_mesh_rings - 1) / (radius - hole_radius));
		if (r_inner >= n_mesh_rings - 1)
			r_inner = n_mesh_rings - 2;
		r_outer = r_inner + 1;
		if (r_outer == n_mesh_rings)
		{	w_inner = 1;
			w_outer = 0;
			r_outer = n_mesh_rings - 1;
		}
		/* weighting is done on polar basis, should actually do the trig to do
		 * in cartesian
		 */
		else
		{	w_outer = (support_radii [i] - hole_radius) * (n_mesh_rings - 1) /
				(radius - hole_radius) - r_inner;
			w_inner = 1 - w_outer;
		}
		if (debug_interpolate)
			fprintf (debug_file, "support %d radius %g on basis %d r_inner %d w_inner %g r_outer %d w_outer %g\n",
					i, support_radii [i], best_basis_ring, r_inner, w_inner, r_outer, w_outer);
		for (j = 0; j < num_support [i]; j++)
		{	angle = fmod (support_angle [i] + j * (double) degrees / num_support [i] + (double) degrees, (double) degrees) ;
			lp = &(cell_loadings [n_loads++]);
			lp->is_part = false;
			lp->is_primary = true;
			lp->n_components = 4;
			lp->component_index = i;
			lp->component_sub_index = j;

			if (n_mesh_points [r_inner] == 1)
			{	a_first = 0;
				a_next = 0;
				wa_first = 1.0;
				wa_next = 0.0;
			}
			else
			{	a_first = floor (angle / mesh_delta_angle [r_inner]);
				a_next = (a_first + 1) % n_mesh_points [r_inner];
				wa_next = (angle / mesh_delta_angle [r_inner]) - a_first;
				wa_first = 1.0 - wa_next;
			}

			lp->component_list [0] = mesh_first_point [r_inner] + a_first;
			lp->component_rel_force [0] = wa_first * w_inner * force;
			lp->component_list [1] = mesh_first_point [r_inner] + a_next;
			lp->component_rel_force [1] = wa_next * w_inner * force;

			a_first = floor (angle / mesh_delta_angle [r_outer]);
			a_next = (a_first + 1) % n_mesh_points [r_outer];
			wa_next = (angle / mesh_delta_angle [r_outer]) - a_first;
			wa_first = 1.0 - wa_next;


			lp->component_list [2] = mesh_first_point [r_outer] + a_first;
			lp->component_rel_force [2] = wa_first * w_outer * force;
			lp->component_list [3] = mesh_first_point [r_outer] + a_next;
			lp->component_rel_force [3] = wa_next * w_outer * force;

			/* find the cg of the component */
			lp->x_cg = 0;
			lp->y_cg = 0;
			total_load_force = 0;
			for (icomp = 0; icomp < 4; icomp++) {
				polar_to_euc (point_radius [lp->component_list [icomp]],
								point_angle [lp->component_list [icomp]], &px, &py);
				lp->x_cg += px * lp->component_rel_force [icomp];
				lp->y_cg += py * lp->component_rel_force [icomp];
				total_load_force += lp->component_rel_force [icomp];
			}
			lp->rel_force = total_load_force;
			lp->x_cg /= total_load_force;
			lp->y_cg /= total_load_force;
		}
	}
}

void generate_primary_support_info (void) {
	int iload;
	int ipart;
	int isubpart;
	t_load_descr *lp;
	t_load_descr *lp_sub;
	int icomp;
	int n_primary;

	for (ipart = 0; ipart < n_parts; ipart++) {
		part_load_index [ipart] = n_loads;
		for (isubpart = 0; isubpart < part_quantity [ipart]; isubpart++) {
			lp = &(cell_loadings [n_loads++]);
			lp->is_part = true;
			lp->is_primary = true;
			lp->n_components = part_type_num_corners [part_type [ipart]];
			lp->component_index = ipart;
			lp->component_sub_index = isubpart;
			lp->x_cg = 0;
			lp->y_cg = 0;
			lp->rel_force = 0;
			for (icomp = 0; icomp < lp->n_components; icomp++) {
				if (part_point_type [ipart] [icomp] == part_point_ring) {
					lp->component_list [icomp] = point_load_index [part_ring_num [ipart] [icomp]] +
							(part_point_num [ipart] [icomp] +
							isubpart * num_support [part_ring_num [ipart] [icomp]] / part_quantity [ipart])
									 % num_support [part_ring_num [ipart] [icomp]];

				} else {
					lp->component_list [icomp] = part_load_index [part_ring_num [ipart] [icomp]] +
						(part_point_num [ipart] [icomp] +
						isubpart * part_quantity [part_ring_num [ipart] [icomp]] / part_quantity [ipart]) %
								 part_quantity [part_ring_num [ipart] [icomp]];
				}
				lp_sub = &(cell_loadings [lp->component_list [icomp]]);
				lp->x_cg += lp_sub->x_cg * lp_sub->rel_force;
				lp->y_cg += lp_sub->y_cg * lp_sub->rel_force;
				lp->rel_force += lp_sub->rel_force;
				lp_sub->is_primary = false;
			}
			 lp->x_cg /= lp->rel_force;
			 lp->y_cg /= lp->rel_force;
		}
	}
	n_primary = 0;
	for (iload = 0; iload < n_loads; iload++) {
		if (cell_loadings [iload].is_primary) {
			if (n_primary == n_deg_free) {
				err_msg ("too many primary supports in the cell, must be 3");
				prompt_exit ();
			}
			primary_load_index [n_primary++] = iload;
		}
	}
	if (n_primary != n_deg_free) {
		err_msg ("mot enough primary supports in the cell, must be 3");
		prompt_exit ();
	}

}

void generate_glued_x_loads (double *m0) {
	double total_x_force;
	double x_force_scale_fac;
	int iload;
	int icomp;
	t_load_descr *lp;

	/* calculate total x loading and total wieghting of supports, then
	 * use as scale factor for x loads
	 */

	total_x_force = m0 [dim_x];
	x_force_scale_fac = total_x_force / (
		cell_loadings [primary_load_index [0]].rel_force +
		cell_loadings [primary_load_index [1]].rel_force +
		cell_loadings [primary_load_index [2]].rel_force);

	/* walk through all the loads, adding x load to the points but can exit as soon as hit a part */
	for (iload = 0; iload < n_loads; iload++) {
		lp = &(cell_loadings [iload]);
		if (lp->is_part) {
			break;
		}
		for (icomp = 0; icomp < lp->n_components; icomp++) {
			z88_node_loads [lp->component_list [icomp]] [dim_x] -= lp->component_rel_force [icomp] * x_force_scale_fac;
		}
	}
}
void distribute_prim_load (int iload, double load) {
	t_load_descr *lp;
	int icomp;

	lp = &(cell_loadings [iload]);
	if (lp->is_part) {
		for (icomp = 0; icomp < lp->n_components; icomp++) {
			distribute_prim_load (lp->component_list [icomp], load);
		}
	} else {
		for (icomp = 0; icomp < lp->n_components; icomp++) {
			z88_node_loads [lp->component_list [icomp]] [dim_z] += lp->component_rel_force [icomp] * load;
		}
	}

}

void generate_z_loading (double *m0, double *tq) {
	double a [max_refocus_distortion_order] [max_refocus_distortion_order];
	double rhs [max_refocus_distortion_order];
	int iprim;
	double prim_load_fac;


	/* force balance on z axis */

	a [0] [0] = 1;
	a [0] [1] = 1;
	a [0] [2] = 1;
	rhs [0] = -m0 [dim_z];

	/* torque balance on x axis */

	/* tq_x = y * fz - z * fy but fy = 0 */

	a [1] [0] = cell_loadings [primary_load_index [0]].y_cg;
	a [1] [1] = cell_loadings [primary_load_index [1]].y_cg;
	a [1] [2] = cell_loadings [primary_load_index [2]].y_cg;

	rhs [1] = -tq [dim_x];

	/* torque balance on y axis */
	/* tq_y = z * fx - x * fz but z = 0 */

	a [2] [0] = -cell_loadings [primary_load_index [0]].x_cg;
	a [2] [1] = -cell_loadings [primary_load_index [1]].x_cg;
	a [2] [2] = -cell_loadings [primary_load_index [2]].x_cg;

	rhs [2] = -tq [dim_y];

	solve_matrix (a, rhs, 3);

	for (iprim = 0; iprim < 3; iprim++) {
		prim_load_fac = rhs [iprim] / cell_loadings [primary_load_index [iprim]].rel_force;
		distribute_prim_load (primary_load_index [iprim], prim_load_fac);
	}
}

/* apply a sling edge loading in x to compensate out x force.
 * don't try and distribute along z axis to neturalize torque in this version.
 * this will leave some residual torque that is handled by the cell supports
 */

void generate_sling_x_loads (double *m0, double m1 [n_deg_free] [n_deg_free],
	double *tq) {

	int n_edge_points;
	int edge_point_first, edge_point_last;
	int iedge;
	double edge_load_fac;
	double load_per_point;
	int ilayer;
	int inode;

	n_edge_points = ceil (sling_included_angle * n_mesh_points [n_mesh_rings - 1] / degrees);
	n_edge_points |= 1;

	/* points used in edge support from first to last are inclusive */
	edge_point_first = n_mesh_points [n_mesh_rings - 1] / 2 - n_edge_points / 2;
	edge_point_last = n_mesh_points [n_mesh_rings - 1] / 2 + n_edge_points / 2;

	/* calculate net x loading for the set of nodes */

	edge_load_fac = 0;
	for (iedge = edge_point_first; iedge <= edge_point_last; iedge++) {
		edge_load_fac += cos (iedge * 2.0 * M_PI / n_mesh_points [n_mesh_rings - 1]);
	}

	for (ilayer = 0; ilayer < n_mesh_depth; ilayer++) {
		load_per_point = -m0 [dim_x] / (n_mesh_depth - 1) / edge_load_fac;

		/* top and bottom nodes get half the force */

		if (ilayer == 0 || ilayer == n_mesh_depth - 1) {
			load_per_point *= 0.5;
		}
		for (iedge = edge_point_first; iedge <= edge_point_last; iedge++) {
			inode = total_mesh_points * ilayer + mesh_first_point [n_mesh_rings - 1] + iedge;
			z88_node_loads [inode] [dim_x] += load_per_point * cos (iedge * 2.0 * M_PI / n_mesh_points [n_mesh_rings - 1]);
			z88_node_loads [inode] [dim_y] += load_per_point * sin (iedge * 2.0 * M_PI / n_mesh_points [n_mesh_rings - 1]);
		}
	}
}


void write_z88_loads () {
	FILE *ofile;
	int itri;
	int izdepth;
	int itri_pt;
	int inode;
	int ilayer;
	int idim;
	dvec node_loads;
	double f_node;
	double m0 [n_deg_free], m1 [n_deg_free] [n_deg_free];
	double tq [n_deg_free];
	double tilt_sin;
	double tilt_cos;


	if (!mirror_tilt_angle_found) {
		mirror_tilt_angle = 0;
	}
	if (edge_support_sling_angle_found) {
		mirror_tilt_angle = edge_support_sling_angle;
	}
	if (edge_support_glued_angle_found) {
		mirror_tilt_angle = edge_support_glued_angle;
	}
	
	tilt_sin = sin (deg_to_rad * mirror_tilt_angle);
	tilt_cos = cos (deg_to_rad * mirror_tilt_angle);
	
	ofile = fopen (z88_load_file_name, "wb");
	for (inode = 0; inode < n_z88_points; inode++) {
		for (idim = 0; idim < n_deg_free; idim++) {
			z88_node_loads [inode] [idim] = 0;
		}
	}
	
	for (itri = 0; itri < n_triangles; itri++) {
		/* divide total load across all corners of the triangle. f_node is per slice */
		f_node = triangle_mass [itri] / 2 / tri_pts / (n_mesh_depth - 1);
		/* vertical for now */
		node_loads [dim_x] = tilt_sin * f_node;
		node_loads [dim_y] = 0;
		node_loads [dim_z] = tilt_cos * f_node;

		/* iterate across depth and add to all nodes */

		for (izdepth = 0; izdepth < n_mesh_depth - 1; izdepth++) {
			/* iterate across top and bottom */
			for (ilayer = 0; ilayer < 2; ilayer++) {
				for (itri_pt = 0; itri_pt < tri_pts; itri_pt++) {
					inode = triangle_points [itri] [itri_pt] + (izdepth + ilayer) * total_mesh_points;
					for (idim = 0; idim < n_deg_free; idim++) {
						z88_node_loads [inode] [idim] -= node_loads [idim];
					}
				}
			}
		}
	}
	generate_node_loads ();

	generate_primary_support_info ();

	calculate_z88_nodal_moments (m0, m1, tq);

	switch (tilt_support_mode) {

	case tilt_support_mode_sling:
		generate_sling_x_loads (m0, m1, tq);
		break;

	case tilt_support_mode_glued:
		generate_glued_x_loads (m0);
		break;
	}
	
	calculate_z88_nodal_moments (m0, m1, tq);

	generate_z_loading (m0, tq);

	calculate_z88_nodal_moments (m0, m1, tq);

	/* constrain center in x,y,z and top of center in xy is 5 extra */
	fprintf (ofile, "%d\n", n_z88_points * n_deg_free + 5);
	fprintf (ofile, "1 1 2 0\n");
	fprintf (ofile, "1 2 2 0\n");
	fprintf (ofile, "1 3 2 0\n");
	fprintf (ofile, "%d 1 2 0\n", total_mesh_points * (n_mesh_depth - 1) + 1);
	fprintf (ofile, "%d 2 2 0\n", total_mesh_points * (n_mesh_depth - 1) + 1);
	for (inode = 0; inode < n_z88_points; inode++) {
		for (idim = 0; idim < n_deg_free; idim++) {
			fprintf (ofile, "%d %d 1 %g\n", inode + 1, idim + 1, z88_node_loads [inode] [idim]);
		}
	}

	fclose (ofile);



}

void gen_z88_input (void) {
	int itri;
	int iquad;
	char *tmpdir;
	char tmpname [1000];

	if (!generate_z88_input_flag) {
		return;
	}
	if (n_triangles == 0) {
		return;
	}
	for (itri = 0, iquad = 0; itri < n_triangles; itri += 2, iquad++) {
		generate_quad (iquad, itri);
	}
	n_quads = n_triangles / 2;
	n_tetras = n_quads * (n_mesh_depth - 1);
	n_z88_points = total_mesh_points * n_mesh_depth;

	if ((tmpdir = getenv ("TEMP")) == NULL && (tmpdir = getenv ("TMP")) == NULL) {
		tmpname [0] = '\0';
	} else {
		sprintf (tmpname, "%s\\", tmpdir);
	}
	sprintf (z88_struct_file_name, "%s%s", tmpname, z88_struct_file_name_base);
	sprintf (z88_load_file_name, "%s%s", tmpname, z88_load_file_name_base);
	sprintf (z88_deflection_file_name, "%s%s", tmpname, z88_deflection_file_name_base);
	//DeleteFile (z88_struct_file_name);
	//DeleteFile (z88_load_file_name);
	//DeleteFile (z88_deflection_file_name);

	write_z88_structure ();
	write_z88_loads ();

	z88_input_files_valid = 1;
}

#ifndef standalone_grid_gen

void exec_plate (void)
{	int i;
	int j;
	int irhs;
	int p_temp [tri_pts];
	int pmin;

	double x;
	double y;

	plate_n_points = total_mesh_points;
	plate_n_triangles = n_triangles;

	plate_basis_n_forces = basis_n_supports;

	plate_modulus = modulus;
	plate_poisson = poisson;

	for (i = 0; i < total_mesh_points; i++)
	{	j = point_sorted_order [i];
		polar_to_euc (point_radius [j], point_angle [j], &x, &y);
		plate_point_x [i] = x;
		plate_point_y [i] = y;
	}

	for (i = 0; i < n_triangles; i++)
	{	/* sort the point set because plate uses banded matrices */
		/* doesnt' do anything useful with sparse matrices but harmless */
		pmin = 0;
		for (j = 1; j < tri_pts; j++)
		{	if (point_reordered_num [triangle_points [i] [j]] < point_reordered_num [triangle_points [i] [pmin]])
				pmin = j;
		}
		for (j = 0; j < tri_pts; j++)
		{	p_temp [j] = point_reordered_num [triangle_points [i] [(j + pmin) % tri_pts]];
		}
		for (j = 0; j < tri_pts; j++)
		{	plate_triangle_points [i] [j] = p_temp [j];
		}
		plate_thickness [i] = triangle_thickness [i];
		plate_tri_pressure [i] = triangle_pressure [i];
	}

	/* if there is no hole then constrain the center in z, theta_x, and theta_y */
	if (!hole_diameter_found && !rel_hole_diameter_found)
    {	/* fix center point */
    	plate_n_constraints = n_deg_free;


        plate_constraint_point [0] = point_reordered_num [0];
        plate_constraint_code [0] = constraint_z;
        plate_constraint_point [1] = point_reordered_num [0];
        plate_constraint_code [1] = constraint_theta_x;
        plate_constraint_point [2] = point_reordered_num [0];
        plate_constraint_code [2] = constraint_theta_y;

	}
    else
	{   /* need to make sure that inner mesh ring divides all basis rings */

		if (grid_gen_using_basis)
       	{	plate_n_constraints = basis_ring [0];
        	for (i = 1; i < n_basis_ring_found; i++)
            {	plate_n_constraints = gcd (plate_n_constraints, basis_ring [i]);
            }
            if (plate_n_constraints < n_deg_free)
            {	err_msg ("Gcd of basis_ring_size is too small, can't constrain the mesh\n");
                prompt_exit ();
            }
        }
        else
       	{	plate_n_constraints = num_support [0];
        	for (i = 1; i < n_num_support_rings; i++)
            {	plate_n_constraints = gcd (plate_n_constraints, num_support [i]);
            }
            if (plate_n_constraints < n_deg_free)
            {	err_msg ("Gcd of num_supports is too small, can't constrain the mesh\n");
                prompt_exit ();
            }
        }

      	for (i = 0; i < plate_n_constraints; i++)
        {	plate_constraint_point [i] = point_reordered_num [mesh_first_point [n_mesh_rings - 1] +
                i * n_mesh_points [n_mesh_rings - 1] / plate_n_constraints];
            plate_constraint_code [i] = constraint_z;
        }
        plate_n_constraints = plate_n_constraints;
	}

	for (irhs = 0; irhs < plate_n_basis_tests; irhs++)
	{	for (i = 0; i < basis_n_supports [irhs]; i++)
		{	plate_basis_forces [irhs] [i] = - basis_support_force [irhs] [i];
			plate_basis_force_points [irhs] [i] = point_reordered_num [basis_support_points [irhs] [i]];
		}
	}

	if (debug_fem)
    {	fprintf (debug_file, "n_points: %d\n", total_mesh_points);
    	fprintf (debug_file, "n_triangles: %d\n", plate_n_triangles);
        for (i = 0; i < total_mesh_points; i++)
        {	fprintf (debug_file, "%d: %g %g\n", i, plate_point_x [i], plate_point_y [i]);
        }

        for (i = 0; i < plate_n_triangles; i++)
        {	fprintf (debug_file, "%d: %d %d %d %g %g\n",
        		i, plate_triangle_points [i] [0],
                plate_triangle_points [i] [1] ,plate_triangle_points [i] [2],
                plate_thickness [i], plate_tri_pressure [i]);
        }
    }

    plate_setup ();

	pload ();

	formk ();

	solve ();

	out ();

}

/* ring is the loc support is tested on.
 * test_rad is the rad the support is on.
 * sum rad is the ring to be summed.
 * rot is rotation, weight..
 */
void sum_components (
	int *ring,
	int *test_rad,
	int *sum_rad,
	int *rot,
	double *weight,
	int n)
{	int i;
	int test_index;
	int m_ring;
	int m_a;
	int p_from;
	int p_to;
	double w_i;
	int rot_i;

	for (i = 0; i < plate_n_points; i++)
	{	plate_z_displacement [i] = 0;
	}
	for (i = 0; i < n; i++)
	{	rot_i = rot [i];
		test_index = basis_test_index [ring [i]] [test_rad [i]];
		if (test_index < 0)
		{	err_msg ("Sum_components: bug in mapping supports to basis\n");
			prompt_exit ();
		}
		w_i = weight [i];
		m_ring = sum_rad [i];
		for (m_a = 0; m_a < n_mesh_points [m_ring]; m_a++)
		{	/* note must be careful to remember reordering in interpolation, as rhs
			 * and plate_z_displacement is in terms of reordered points
			 */
			p_from = point_reordered_num [mesh_first_point [m_ring] + m_a];
            
            /* add of extra n_mesh_points [m_ring] before mod in case rot_i is -ve */

			p_to = point_reordered_num [mesh_first_point [m_ring] + (m_a + rot_i + n_mesh_points [m_ring]) % n_mesh_points [m_ring]];
			plate_z_displacement [p_to] += plate_basis_rhs [test_index] [mat_index (p_from, 0)] * w_i;
		}
	}
}

/* add a component using basis support ring: ring, at radius: rad with
 * rotation of rot points, and weight w
 */

void add_component (
	int *ring_a,
	int ring,
	int *test_rad_a,
	int test_rad,
	int *sum_rad_a,
	int sum_rad,
	int *rot_a,
	int rot,
	double *w_a,
	double w,
	int *nc)
{	if (*nc < max_components)
	{	ring_a [*nc] = ring;
		test_rad_a [*nc] = test_rad;
		sum_rad_a [*nc] = sum_rad;
		rot_a [*nc] = rot;
		w_a [*nc] = w;
		if (debug_interpolate)
		{	fprintf (debug_file, "component ring: %d test_rad: %d sum_rad: %d rot: %d w: %g\n",
				ring_a [*nc], test_rad_a [*nc], sum_rad_a [*nc],
				rot_a [*nc], w_a [*nc]);
		}
		(*nc)++;
	}
	else
	{	err_msg ("Interpolate into too many basis components\n");
		prompt_exit ();
	}
}

/* Interpolate the basis vectors. Break each support into 4 components,
 * corresponding to inner and outer ring, and 2 nearest points. This is
 * a bit bogus, but reasonable time complexity and easy to compute.
 */

void interpolate_basis (void)
{	int i;
	int j;

	int best_basis_ring;
	int n_best_basis_ring;

	int r_inner, r_outer;
	int r_sum;
	double w_inner, w_outer;
	double angle;
	int a_first, a_next;
	double wa_first, wa_next;
	double total_rel_force;
	double force;

	int basis_mesh_ring [max_components];		/* which basis ring the point uses */
	int basis_mesh_radius [max_components];		/* which radius of basis ring */
	int basis_mesh_sum_radius [max_components];		/* which radius of basis ring to sum */
	int basis_mesh_rot [max_components];		/* number of points to rotate the ring */
	double basis_mesh_weight [max_components];	/* weight of this component */
	int n_components;

	if (n_rel_force == 0)
	{	for (i = 0; i < n_support_radii; i++)
			rel_force [i] = 1.0;
	}
	total_rel_force = 0.0;
	for (i = 0; i < n_support_radii; i++)
		total_rel_force += num_support [i] * rel_force [i];
	n_supports = 0;
	n_components = 0;
	for (i = 0; i < n_support_radii; i++)
	{	/* find the best basis ring to use for the support ring.
		 * This is one with minimum points, and usable outside the support ring.
		 */
		force = num_support [i] * rel_force [i] / total_rel_force;
		r_inner = floor ((support_radii [i] - hole_radius) * (n_mesh_rings - 1) / (radius - hole_radius));
		if (r_inner >= n_mesh_rings - 1)
            r_inner = n_mesh_rings - 2;
        best_basis_ring  = -1;
		n_best_basis_ring = 2 * num_support [i];		/* init best count so far */
		for (j = 0; j < n_basis_ring_found; j++)
		{	if ((num_support [i] % basis_ring [j] == 0 ||
                r_inner == 0 && support_radii [0] == 0.0)
                 && basis_min [j] <= r_inner
                 && num_support [i] / basis_ring [j] < n_best_basis_ring)
			{	best_basis_ring = j;
				if (r_inner == 0 && support_radii [0] == 0.0)
                    n_best_basis_ring = 1;
                else
                    n_best_basis_ring = num_support [i] / basis_ring [j];
			}
		}
		if (best_basis_ring == -1)
		{	err_msg_gd ("Basis ring failed in interpolate_basis: evaluating outside the range specified\n"
                "A support at radius %g is outside of all of the basis rings\n"
                "A basis ring at %d is required\n", support_radii [i], r_inner);
			prompt_exit ();
		}
		r_outer = r_inner + 1;
		if (r_outer == n_mesh_rings)
		{	w_inner = 1;
			w_outer = 0;
			r_outer = n_mesh_rings - 1;
		}
		/* weighting is done on polar basis, should actually do the trig to do
		 * in cartesian
		 */
		else
		{	w_outer = (support_radii [i] - hole_radius) * (n_mesh_rings - 1) /
        		(radius - hole_radius) - r_inner;
			w_inner = 1 - w_outer;
		}
		w_inner *= force / n_best_basis_ring;
		w_outer *= force / n_best_basis_ring;
		if (debug_interpolate)
			fprintf (debug_file, "support %d radius %g on basis %d r_inner %d w_inner %g r_outer %d w_outer %g\n",
					i, support_radii [i], best_basis_ring, r_inner, w_inner, r_outer, w_outer);
		for (j = 0; j < n_best_basis_ring; j++)
		{	angle = fmod (support_angle [i] + j * (double) degrees / num_support [i] + (double) degrees, (double) degrees);
			for (r_sum = 0; r_sum < n_mesh_rings; r_sum++)
			{	if (n_mesh_points [r_sum] == 1)
				{	a_first = 0;
					a_next = 0;
					wa_first = 1.0;
					wa_next = 0.0;
				}
				else
				{	a_first = floor (angle / mesh_delta_angle [r_sum]); 
					a_next = (a_first + 1) % n_mesh_points [r_sum];
					wa_next = (angle / mesh_delta_angle [r_sum]) - a_first;
					wa_first = 1.0 - wa_next;
				}
				if (wa_first != 0.0)
				{	add_component (basis_mesh_ring, best_basis_ring, basis_mesh_radius, r_inner,
						basis_mesh_sum_radius, r_sum,
						basis_mesh_rot, a_first,
						basis_mesh_weight, w_inner * wa_first, &n_components);
				}
				if (wa_next != 0.0)
				{	add_component (basis_mesh_ring, best_basis_ring, basis_mesh_radius, r_inner,
						basis_mesh_sum_radius, r_sum,
						basis_mesh_rot, a_next,
						basis_mesh_weight, w_inner * wa_next, &n_components);
				}
				if (wa_first != 0.0)
				{	add_component (basis_mesh_ring, best_basis_ring, basis_mesh_radius, r_outer,
						basis_mesh_sum_radius, r_sum,
						basis_mesh_rot, a_first,
						basis_mesh_weight, w_outer * wa_first, &n_components);
				}
				if (wa_next != 0.0)
				{	add_component (basis_mesh_ring, best_basis_ring, basis_mesh_radius, r_outer,
						basis_mesh_sum_radius, r_sum,
						basis_mesh_rot, a_next,
						basis_mesh_weight, w_outer * wa_next, &n_components);
				}
			}
		}
	}
	sum_components (basis_mesh_ring, basis_mesh_radius, basis_mesh_sum_radius,
			basis_mesh_rot, basis_mesh_weight, n_components);
}

#endif

