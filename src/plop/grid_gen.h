#ifndef grid_gen_h
#define grid_gen_h

#include "global.h"


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
void order_nodes (void);
void interpolate_basis (void);
void interpolate_tris (void);


#endif


void generate_quad (int iquad, int itri);
void gen_z88_input (void);
void set_max_points (int);
void exec_plate (void);
void compute_geom (void);
void gen_basis_mesh (void);
void gen_mesh (int);

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

extern void DebugMsg (char const *);

#endif

extern double deg_to_rad;

extern int diameter_found;
extern double diameter;

extern int please_stop_plop;

#ifdef gui_plop
extern void terminate_plop (void);
#else
void terminate_plop (void)
{}
#endif


extern double parab_a0;			/* coefficients of parabola for thickness vs. radius */
extern double parab_a2;
extern double parab_a4;

/*
 * We'll automatically generate radii of mesh, and which supports are on
 * which ring, but this can be overridden by direct input of the data.
 */


extern int basis_data_alloced;								/* 1 if have allocated data structures for basis gen */


/*
 * plate uses banded matrices. we reorder the
 * the nodes to minimize band width.
 */

extern int *point_reordered_num;	/*[max_points] */		/* number of this point after reordering */
extern int *point_sorted_order;	/* [max_points] */				/* sequence to print points */
extern int *point_sorted_order_temp;	/* [max_points] */		/* temp  for sorting */

extern tri_int_corners *triangle_points;	/* int [max_triangles] [tri_pts] */
extern double *triangle_thickness;			/* [max_triangles] */
extern double *triangle_pressure;			/* [max_triangles] */
extern double *triangle_mass;				/* [max_triangles] */
extern double total_mass;

extern dvec *z88_node_loads;

extern int n_supports;
extern int *support_point;				/* [max_points] */
extern double *support_force;			/* [max_points] */

extern int basis_changed;			/* set to 1 if need to regenerate basis */

extern int basis_test_index [max_basis_size] [max_mesh_radii];

/* basis_n_supports is aliased to plate_basis_n_forces */

extern int basis_n_supports [max_basis_tests];
extern int *basis_support_points [max_basis_tests];
extern double *basis_support_force [max_basis_tests];

extern char const *var_def_strings [];
extern var_def var_table [max_variables];

extern int n_variables;

extern int gr_info_alloced;


extern char const *part_type_names [];
extern char const *part_point_type_names [];


extern int verbose;
extern int trace_opt;
extern int quiet;

extern int use_p_v_error;

extern int n_optimize_steps;
extern int max_optimize_evals;
extern double optimize_delta_f_too_small;
extern double optimize_min_delta; 

/*
 * Changing map of supports to mesh rings can cause discontinuities.
 * Optionally disallow this, but user must be careful not to span a large range.
 */

extern int grid_gen_using_basis;
extern int have_initial_grid;
extern int GridMeshFlags [];

extern char *grid_gen_names [];


extern int n_monte_tests;

extern int reuse_best_opt;

#ifndef standalone_grid_gen
extern char picfile [max_line_len];
extern char datfile [max_line_len];
extern char grfile [max_line_len];
extern char plotfile [max_line_len];
extern char contourplotfile [max_line_len];
extern char opt_gr_file [max_line_len];
extern char opt_phys_gr_file [max_line_len];
extern int have_plot;
extern int have_contour_plot;
extern int have_pic;
extern int have_pic_mesh;
extern int have_dat;
extern int have_gr;
extern int have_opt_gr;
extern int have_opt_phys_gr;
#endif

extern FILE *output_file;
extern FILE *debug_file;


void alloc_plate_globals (void);

void *malloc_or_die (int, char const *);

extern char z88_struct_file_name [];
extern char z88_load_file_name [];
extern char z88_deflection_file_name [];

void init_default_basis();
void run_plate();
void init_plop();
void alloc_globals();

#endif
