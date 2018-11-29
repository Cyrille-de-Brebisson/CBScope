#include "global.h"

int total_mem_alloced;

int max_points;
int max_triangles;

int max_quads;

int max_sparse_els;

double *plate_point_x;        /* [max_points] */
double *plate_point_y;        /* [max_points] */

double *plate_forces;         /* [max_points] */
int *plate_force_points;      /* [max_points] */

double *plate_basis_forces [max_basis_tests];
int *plate_basis_force_points [max_basis_tests];

tri_int_corners *plate_triangle_points;   /*[max_triangles]  [tri_pts]; */

int plate_n_points;
int plate_n_triangles;
int plate_n_constraints;
int plate_n_forces;

int z88_plot_n_points;
int z88_plot_n_triangles;
int z88_plot_n_constraints;
int z88_plot_n_forces;

int *plate_basis_n_forces;

int plate_matrix_size;

double plate_modulus;
double plate_poisson;

double *plate_thickness;		    /* [max_triangle] */
double *plate_tri_pressure;	    /* [max_triangles */
double *plate_area;				/* [max_triangles] */
double *plate_mx;				    /* [max_triangles] */
double *plate_my;				    /* [max_triangles] */
double *plate_mxy;				/* [max_triangles] */
tri_dbl_corners *plate_tri_x_cg;  /* [max_triangles] [tri_pts];	*/ 	/* distance from x to x_cg */
tri_dbl_corners *plate_tri_y_cg;  /* [max_triangles] [tri_pts];  */
double *plate_xcg;				/* [max_triangles] */		/* distance from x to x_cg */
double *plate_ycg;				/* [max_triangles] */

double *plate_rhs;			    /* [max_eqn] */
double *plate_result;			    /* [max_eqn] */

double *plate_basis_rhs  [max_basis_tests];

double *plate_z_displacement;	/* [max_points] */

double plate_shear;

int plate_constraint_point [max_constraint];
int plate_constraint_code [max_constraint];

double plate_refocus_a0;
double plate_refocus_a2;

double plate_parab_a2;
double plate_focal_length;
double plate_refocus_focal_length;

double plate_err_rms;
double plate_err_vis_rms;
double plate_err_p_v;
double plate_err_vis_p_v;

double z88_plot_err_rms;
double z88_plot_err_vis_rms;
double z88_plot_err_p_v;
double z88_plot_err_vis_p_v;

int plate_n_basis_tests;

int warp_sphere_flag;
int refocus_flag;
int refocus_xyr_flag;
int calculate_zernike_flag;
int zernike_order;
int generate_z88_input_flag;

double zernike_coeff [max_refocus_distortion_order];

double fact_table [max_refocus_distortion_order];

int pic_size;
int n_colors;
int n_contours;

double pic_z_range;
double contour_space;


double radius;

double obstruction_radius;
double obstruction_diam;
double rel_obstruction_radius;

// copy of data for plotting in gui version
// the solver will update the plate_ version and gui accesses the plate_plot_
// version. need to get the lock on plop data to update plate_plot_ version.

double *tmp_function_vals [max_refocus_distortion_order];

int plate_plot_n_points;
int plate_plot_n_triangles;

double *plate_plot_point_x;        /* [max_points] */
double *plate_plot_point_y;        /* [max_points] */

double *plate_plot_z_displacement;	/* [max_points] */

double plate_plot_z_min;
double plate_plot_z_max;

tri_int_corners *plate_plot_triangle_points;   /*[max_triangles]  [tri_pts]; */

double tri_fract_inv;
double tri_fract_sq_inv;

// copy of z88 data for plotting in gui version

int z88_input_files_valid;

double *z88_plot_point_x;        /* [max_points] */
double *z88_plot_point_y;        /* [max_points] */

double *z88_plot_z_displacement;	/* [max_points] */

double z88_plot_z_min;
double z88_plot_z_max;

tri_int_corners *z88_plot_triangle_points;   /*[max_triangles]  [tri_pts]; */
