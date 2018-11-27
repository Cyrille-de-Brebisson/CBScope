#ifndef plop_parmsH
#define plop_parmsH

#include "plot.h"
#include "parm_types.h"

/*
 * Data used to communicate between gui_plop and plop.
 */

extern char plop_text_file_buff [];
extern int plop_text_file_len;
extern char *plop_text_file_ptr;

extern char plop_comment_file_buff [];
extern int plop_comment_len;

extern parm_descr parm_list [];
extern var_def var_table [max_variables];

extern int n_triangles;
extern tri_int_corners *triangle_points;	/* int [max_triangles] [tri_pts] */

extern int n_quads;
extern quad_int_corners *quad_points;


extern double *point_radius;	/* [max_points] */
extern double *point_angle;		/* [max_points] */


extern double diameter;
extern int diameter_found;

extern int hole_diameter_found;
extern double hole_diameter;

extern int rel_hole_diameter_found;
extern double rel_hole_diameter;

extern int thickness_found;
extern double thickness;

extern int focal_length_found;
extern double focal_length;

extern int f_ratio_found;
extern double f_ratio;

extern int sagitta_found;
extern double sagitta;

extern int rel_sagitta_found;
extern double rel_sagitta;

extern double obstruction_radius;
extern double obstruction_diam;

extern int n_variables;

extern int n_mesh_rings;
extern int n_mesh_rings_found;
extern int n_mesh_depth;
extern int n_mesh_depth_found;

extern double mirror_tilt_angle;
extern int mirror_tilt_angle_found;

extern double edge_support_sling_angle;
extern int edge_support_sling_angle_found;
extern double edge_support_glued_angle;
extern int edge_support_glued_angle_found;


extern double sling_included_angle;
extern int sling_included_angle_found;
extern int tilt_support_mode;

#define tilt_support_mode_sling	0
#define tilt_support_mode_glued	1


extern int n_support_radii;

extern int n_abs_support_radii;
extern double support_radii [max_support_radii];

extern int n_rel_support_radii;
extern double rel_support_radii [max_support_radii];

extern int n_rel_force;
extern double rel_force [max_support_radii];

extern int n_num_support_rings;
extern int num_support [max_support_radii];				/* number of supports on each ring of supports */

extern int n_support_angle;
extern double support_angle [max_support_radii];			/* angle of first support */

extern int n_support_mesh_ring_found;
extern int support_mesh_ring [max_support_radii];			/* which mesh ring the support ring is on */

extern int n_mesh_points_found;
extern int n_mesh_points [max_mesh_radii];					/* number of points on each ring in mesh */
extern int mesh_first_point [max_mesh_radii];				/* index number of points on this mesh ring */
extern double mesh_delta_angle [max_mesh_radii];			/* spacing angle between mesh points */

extern double support_radii [max_support_radii];

extern int n_support_angle;

extern int n_rel_force;


extern int n_mesh_radii_found;
extern double mesh_radii [max_mesh_radii];

extern double modulus;
extern int modulus_found;

extern double poisson;
extern int poisson_found;

extern int density_found;
extern double density;

extern int obstruct_found;
extern int obstruct_diam_found;
extern int rel_obstruct_found;

extern int total_mesh_points;
extern double *point_radius;	/* [max_points] */
extern double *point_angle;		/* [max_points] */
/*
 * To use the decomposition, specify a basis consisting of the number of support points on the mesh rings,
 * and the minimum mesh ring that the support ring will be used on. Each support ring must be a
 * multiple of the basis size for some ring.
 */

extern int n_basis_ring_found;
extern int basis_ring [max_mesh_radii];

extern int n_basis_min_found;
extern int basis_min [max_mesh_radii];

extern int n_optimize_vars;
extern int n_scan_vars;
extern int n_scan_set_vars;
extern int n_monte_vars;

extern int opt_var_which [max_opt_vars];		/* which variable, either predefined or a var */
extern int opt_var_index [max_opt_vars];		/* index into set */
extern int opt_var_is_var [max_opt_vars];		/* 1 if optimization refers to var, 0 for predefined */
extern double opt_var_step [max_opt_vars];

extern int scan_var_which [max_scan_vars];
extern int scan_var_index [max_scan_vars];
extern double scan_var_start [max_scan_vars];
extern double scan_var_end [max_scan_vars];
extern int scan_var_nsteps [max_scan_vars];
extern int scan_var_is_var [max_scan_vars];

extern int scan_set_which [max_scan_set_vars];
extern int scan_set_index [max_scan_set_vars];
extern int scan_set_n_values [max_scan_set_vars];
extern double scan_set_values [max_scan_set_vars] [max_scan_set];
extern int scan_set_is_var [max_scan_set_vars];
             
extern int monte_var_which [max_scan_vars];
extern int monte_var_index [max_scan_vars];
extern double monte_var_delta [max_scan_vars];
extern int monte_var_is_var [max_scan_vars];

extern int part_type_num_corners [];


extern int n_parts;
extern int part_type [max_parts];
extern int part_quantity [max_parts];
extern int part_point_type [max_parts] [tri_pts];
extern int part_ring_num [max_parts] [tri_pts];
extern int part_point_num [max_parts] [tri_pts];
extern double part_corner_x [max_parts] [tri_pts];        /* absolute */
extern double part_corner_y [max_parts] [tri_pts];
extern double part_corner_radius [max_parts] [tri_pts];        /* absolute */
extern double part_corner_angle [max_parts] [tri_pts];
extern double part_cg_x [max_parts];
extern double part_cg_y [max_parts];
extern double part_cg_radius [max_parts];
extern double part_cg_angle [max_parts];
extern double part_total_force [max_parts];
extern double part_draw_x [max_parts] [tri_pts];
extern double part_draw_y [max_parts] [tri_pts];
extern double part_draw_cg_x [max_parts];
extern double part_draw_cg_y [max_parts];
extern int part_rotation [max_parts];

/* flags for controlling Plop */


extern int n_monte_tests;

extern int reuse_best_opt;

extern int use_p_v_error;

extern plop_cmap color_cmap;
extern plop_cmap contour_cmap;
extern plop_cmap bw_cmap;

extern int n_colors;
extern int n_contours;

extern int pic_size;

extern int use_p_v_error;
extern int refocus_flag;
extern int refocus_xyr_flag;
extern int trace_opt;
extern int generate_z88_input_flag;


extern int please_stop_plop;

extern FILE *output_file;

extern double pic_z_range;
extern double contour_space;

extern unsigned char picture [graph_xy_max] [graph_xy_max];

#endif


