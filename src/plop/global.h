#ifndef globalH
#define globalH

#define gui_plop
#define use_sparse
#define use_gd_graphics

//#define DEBUGGING_ENVARS
//#define log_malloc
//#define obsolete
//#define solve_by_col
//#define ppm_p3_format
//#define standalone
//#define foo
//#define foooo
//#define debug
//#define debug_ainv_middle
//#define debug_detailed
//#define standalone_plate
//#define DebugLocale
//#define richardgetfiledir

#define M_PI 3.14159265358979323846264338327

#define z88_struct_file_name_base "z88i1.txt"
#define z88_load_file_name_base "z88i2.txt"
#define z88_deflection_file_name_base "z88o2.txt"

#define z88i1_bin_name "\\z88i1plop.exe"
#define z88i2_bin_name "\\z88i2plop.exe"


#define max_line_len	200

#define max_plop_file_len   30000

#define max_comments        100
#define max_comments_len    15000

#define default_max_points		5000

#define max_constraint	60

#define degrees				360		/* process angles in degrees to avoid roundoff */

#define tri_pts			3
#define n_deg_free		3			/* number of degrees of freedom in this universe */
#define dim_x			0
#define dim_y			1
#define dim_z			2

#define quad_pts		4			/* numer of points in quadrilateral */
#define tetra_pts		8			/* number of points in a tetra */


#define constraint_z		100
#define constraint_theta_x	10
#define constraint_theta_y	1

typedef int tri_int_corners [tri_pts];
typedef double tri_dbl_corners [tri_pts];

typedef int quad_int_corners [quad_pts];

typedef double dvec [n_deg_free];

#define max_eqn			(max_points * n_deg_free)

#define mat_index(point,deg)	((point) * n_deg_free + (deg))

#define tri_deg_free	(tri_pts * n_deg_free)		/* number of degrees of freedom of a triangle */

#define band_size			250


#define tri_fract		1			/* num linear pieces to fracture triangle for rms error */
#define tri_fract_sq	(tri_fract * tri_fract)

#define max_basis_size	20		/* max number of independent support configurations used as basis */
#define max_mesh_radii	100		/* maximum number of radii of points */
#define max_mesh_depth	20

#define max_points_per_ring	30	/* not presently checked; we could croak on this */


#define max_basis_tests	(max_basis_size * max_mesh_radii)   /* should actually use and check max supports on a mesh ring.*/
#define max_components	(max_basis_tests * 4)		/* upper bound, not guaranteed unless above is valid */

#define max_parm_names		50
                                                   
#define max_variables		100

#define max_parts           50

#define max_mesh_perim		1000	/* maximum n of points around each circle */

#define max_refocus_distortion_order    100


#define maxwords			100
#define wordlen				100

#define parm_type_int		0
#define parm_type_double	1
#define parm_type_flag		2

#define max_support_radii	50

#define max_opt_vars		100		/* max number of parms to optimize */
#define max_scan_vars		100		/* max number of parms to scan */
#define max_scan_set_vars	100		/* max number of scan-set vars */
#define max_monte_vars		100
#define max_scan_set		50		/* max number in set of values to scan; must be <= maxwords */


#define grid_gen_never		0			/* only once, actually, but never again! */
#define grid_gen_basis		1			/* generate a basis and use it */

#define part_type_triangle        0
#define part_type_bar             1

#define part_point_ring           0
#define part_point_part           1

#endif
