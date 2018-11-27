#include "model.h"
#include "plop/global.h"
#include "plop/plate_global.h"
#include "plop/plop_parser.h"
#include "plop/plop_parms.h"
#include "plop/grid_gen.h"
#include <QObject>
#include <QPainter>
#include <QColor>
#include <QThread>
#include <QSemaphore>


static CBScopeMes *mesToUse= nullptr;
static QMutex mutex;
class MesThread;
static MesThread *mesThread= nullptr;
class MesThread : public QThread {
public:
	MesThread(QObject *parent=nullptr): QThread(parent) { mesThread= this; }
private:
	void run() override
	{
		init_default_basis();
		plate_init();
		run_plate();
		mesThread= nullptr;
		emit mesToUse->calcChanged();
	}
};

bool CBScopeMes::getCalc() { return mesThread!=nullptr; }
void CBScopeMes::doMesStop() { please_stop_plop= 1; }

void CBScopeMes::createMirror(double diametre, double secondary, double thickness, double young, double poisson, double focale, double density, int _cellType)
{ 
	if (mesThread!=nullptr) return;
	_radius= diameter/2.0; _secondary= secondary/2.0;
	mutex.lock();
	mesToUse= this;
	if (!hasPlopInit)
	{
		hasPlopInit= true;
		init_plop ();
		set_max_points(2371*2); // 2371 is the number for 54 point cell... doubeling it to be sure...
		init_graphics (); /* used to store some error calculation info */
		alloc_globals ();
		alloc_plate_globals ();
	}
	int const maxSupportRings= 4;
	struct { int nbPoints, n_mesh_rings;
		     char nb_vars_to_opt; char optimize_vars[4][3]; // nb, which, index, isvar;
			 int basis_ring;
	         int n_num_support_rings; char num_support[maxSupportRings]; double rel_support_radii[maxSupportRings]; char rel_support_boud_vars[maxSupportRings];
			 int n_basis_ring_found, basis_ring_found[6];
			 int n_support_angle;  double support_angle[maxSupportRings];
			 int n_rel_support_radii;
			 int n_parts; struct { char type, quantity, point_type[3], ring_num[3], point_num[3]; } parts[5];
	       }
        const cellDefs[]={
			//       opt vars                                     sup_rings nb, pts & diam                       basis_rng               angles          sup_radii   parts
			{3,  8,  1,{{14, 0,0},          },                    3, 1, {3         }, {0.4          },{-1,0,0,0},    1, {3,      },      1, {0,  0,  0}, 1,          0, {},                              },
			{6,  12, 1,{{14, 0,0},          },                    3, 1, {6         }, {0.6          },{-1,0,0,0},    1, {3,      },      1, {0,  0,  0}, 1,          1, {{ 1, 3, {0, 0}, {0, 0}, {0, 1} } }},
			{9,  12, 2,{{ 0,-1,1}, { 1,-1,1}},                    3, 3, {3,3,3     }, {0.4, 0.6, 0.8},{ 0,1,1,0},    1, {3,      },      3, {0, 30, 90}, 3,          1, {{ 0, 3, {0, 0, 0}, {0, 1, 2}, {0, 0, 2}} }},
		    {18, 20, 2,{{14, 0,0}, {14, 1,0}},                    6, 2, {6,12      }, {0.37, 0.799  },{-1,-1,0,0},   1, {6,      },      2, {0, 15,  0}, 1,          2, {{ 0, 6, {0, 0, 0}, {0, 1, 1}, {0, 0, 11}}, { 1, 3, {1, 1}, {0, 0}, {0, 1} } }},
		    {27, 20, 3,{{ 0,-1,1}, {1,-1,1}, {2,-1,1}},           3, 3, {6,9,12    }, {0.4, 0.6, 0.8},{ 0,1,2,0},    1, {3,      },      3, {30,20,15 }, 1,          4, {{ 0, 3, {0, 0, 0}, {1, 2, 2}, {0, 0, 1}}, { 0, 3, {0, 0, 0}, {1, 2, 2}, {2, 2, 3} }, { 0, 3, {0, 0, 0}, {0, 0, 1}, {0, 1, 1} }, { 0, 3, {1, 1, 1}, {0, 1, 2}, {0, 0, 0} } } },
			{36, 24, 3,{{ 0,-1,1}, {1,-1,1}, {2,-1,1}},           3, 3, {6,12,18   }, {0, 0, 0,     },{ 0,1,2,0},    1, {6,      },      0, {30,20,15 }, 1,          5, {{ 1, 6, {0, 0}, {0, 1}, {0, 0}}, { 1, 6, {0, 0}, {2, 2}, {0, 1} }, { 1, 6, {0, 0}, {1, 2}, {1, 2} }, { 0, 6, {1, 1, 1}, {0, 1, 2}, {0, 0, 0} }, { 1, 3, {1, 1}, {3, 3}, {0, 1} } } },
			{54, 24, 4,{{ 0,-1,1}, {1,-1,1}, {2,-1,1}, {3,-1,1}}, 3, 4, {6,12,12,24}, {0, 0, 0,     },{ 0,1,2,3},    1, {6,      },      0, {30,20,15 }, 1,          4, {{ 0, 6, {0, 0,0}, {0, 1,1}, {0, 0,11}}, { 0, 12, {0,0,0}, {2,3,3}, {0,1,0} }, { 0, 6, {1,1,1}, {0,1,1}, {0,0,11} }, { 1, 3, {1, 1}, {2, 2}, {0, 1} } } },
		};

		CBSModel::singleton->setErrMsg("");
		CBSModel::singleton->setDbgMsg("");
		CBSModel::singleton->setWarMsg("");

		init_gr_info(); // init plate parser variables...

		// diameter (0)
		*parm_list[0].num_found = 1;
		parm_list[0].affects_basis = 1;
		*parm_list[0].bound_to_var = -1;
		parm_list[0].dptr[0] = diametre;
		// thickness (3)
		*parm_list[3].num_found = 1;
		parm_list[3].affects_basis = 1;
		*parm_list[3].bound_to_var = -1;
		parm_list[3].dptr[0] = thickness;
		// focal-length (7)
		*parm_list[7].num_found = 1;
		parm_list[7].affects_basis = 1;
		*parm_list[7].bound_to_var = -1;
		parm_list[7].dptr[0] = focale;
		// obstruction-diam (24)
		*parm_list[24].num_found = 1;
		parm_list[24].affects_basis = 0;
		*parm_list[24].bound_to_var = -1;
		parm_list[24].dptr[0] = secondary;
		// density (4)
		*parm_list[4].num_found = 1;
		parm_list[4].affects_basis = 1;
		*parm_list[4].bound_to_var = -1;
		parm_list[4].dptr[0] = density*1e-6;
		// modulus (5)
		*parm_list[5].num_found = 1;
		parm_list[5].affects_basis = 1;
		*parm_list[5].bound_to_var = -1;
		parm_list[5].dptr[0] = young;
		// poisson (6)
		*parm_list[6].num_found = 1;
		parm_list[6].affects_basis = 1;
		*parm_list[6].bound_to_var = -1;
		parm_list[6].dptr[0] = poisson;

		n_abs_support_radii = 0;
		n_basis_min_found = 1;
		n_mesh_rings_found = 1;
		n_mesh_depth = 5;
		n_mesh_depth_found = 1;
		tilt_support_mode = 1;

		n_parts = cellDefs[_cellType].n_parts;
		for (int i = 0; i < n_parts; i++)
		{
			part_type[i] = cellDefs[_cellType].parts[i].type;
			part_quantity[i] = cellDefs[_cellType].parts[i].quantity;
			for (int j = 0; j < part_quantity[i]; j++) part_point_type[i][j] = cellDefs[_cellType].parts[i].point_type[j];
			for (int j = 0; j < part_quantity[i]; j++) part_ring_num[i][j] = cellDefs[_cellType].parts[i].ring_num[j];
			for (int j = 0; j < part_quantity[i]; j++) part_point_num[i][j] = cellDefs[_cellType].parts[i].point_num[j];
		}

		basis_ring[0] = 3;
		basis_min[0] = 0;

		// n-mesh-rings (11)
		*parm_list[11].num_found = 1;
		parm_list[11].affects_basis = 0;
		*parm_list[11].bound_to_var = -1;
		parm_list[11].iptr[0] = cellDefs[_cellType].n_mesh_rings;
		// n-mesh-depth (12)
		*parm_list[12].num_found = 1;
		parm_list[12].affects_basis = 0;
		*parm_list[12].bound_to_var = -1;
		parm_list[12].iptr[0] = 5;
		// rel-support-radii (14)
		*parm_list[14].num_found = cellDefs[_cellType].n_num_support_rings;
		parm_list[14].affects_basis = 0;
		*parm_list[14].bound_to_var = -1;
		for (int i=0; i< cellDefs[_cellType].n_num_support_rings; i++) 
		{
			parm_list[14].dptr[i] = cellDefs[_cellType].rel_support_radii[i];
			parm_list[14].bound_to_var[i] = cellDefs[_cellType].rel_support_boud_vars[i];
		}
		// num-support (17)
		*parm_list[17].num_found = cellDefs[_cellType].n_num_support_rings;
		parm_list[17].affects_basis = 0;
		for (int i = 0; i < cellDefs[_cellType].n_num_support_rings; i++) parm_list[17].bound_to_var[i] = -1;
		for (int i = 0; i < cellDefs[_cellType].n_num_support_rings; i++) parm_list[17].iptr[i] = cellDefs[_cellType].num_support[i];
		// support-angle (19)
		*parm_list[19].num_found = cellDefs[_cellType].n_support_angle;
		parm_list[19].affects_basis = 0;
		for (int i = 0; i < cellDefs[_cellType].n_support_angle; i++) parm_list[19].bound_to_var[i] = -1;
		for (int i = 0; i < cellDefs[_cellType].n_support_angle; i++) parm_list[19].dptr[i] = cellDefs[_cellType].support_angle[i];
		// basis-ring-size (21)
		*parm_list[21].num_found = cellDefs[_cellType].n_basis_ring_found;
		parm_list[21].affects_basis = 0;
		for (int i=0; i< cellDefs[_cellType].n_basis_ring_found; i++) parm_list[21].bound_to_var[i] = -1;
		for (int i=0; i< cellDefs[_cellType].n_basis_ring_found; i++) parm_list[21].iptr[i] = cellDefs[_cellType].basis_ring_found[i];
		// basis-ring-min (22)
		*parm_list[22].num_found = 1;
		parm_list[22].affects_basis = 0;
		*parm_list[22].bound_to_var = -1;
		parm_list[22].iptr[0] = 0;

		n_monte_vars = 0;
		n_scan_vars = 0;
		n_scan_set_vars = 0;

		n_optimize_vars = cellDefs[_cellType].nb_vars_to_opt;
		for (int i = 0; i < n_optimize_vars; i++)
		{
			opt_var_which[i] =  cellDefs[_cellType].optimize_vars[i][0];
			opt_var_index[i] =  cellDefs[_cellType].optimize_vars[i][1];
			opt_var_is_var[i] = cellDefs[_cellType].optimize_vars[i][2];
			opt_var_step[i] = 0.010000;
		}

		use_p_v_error = false;
        refocus_flag = true;
		refocus_xyr_flag = false;
		generate_z88_input_flag = false;
		please_stop_plop = 0;
		n_monte_tests = 0;
		calculate_zernike_flag = false;
		trace_opt = false; // print optimization outputs... might not be plate
		output_file = stdout;

		/////////////////////////////////////////////////////////////////////
	#if 0
		char buf[200];
		sprintf(buf, "n_num_support_rings = %d;", n_num_support_rings); qDebug() << buf;
		sprintf(buf, "n_mesh_rings = %d;", n_mesh_rings);              qDebug() << buf;

		sprintf(buf, "n_parts = %d;", n_parts); qDebug() << buf;
		for (int i = 0; i < n_parts; i++)
		{
			sprintf(buf, "part_type[%d]= %d;", i, part_type[i]);         qDebug() << buf;
			sprintf(buf, "part_quantity[%d]= %d;", i, part_quantity[i]); qDebug() << buf;
			int jmax = part_type_num_corners[part_type[i]]; // hard codded to 3 and 2
			for (int j = 0; j < jmax; j++)    // Do not know what this is exacty or how to set it. It seems to indicate where the various points are for each part
			{
				sprintf(buf, "part_point_type[%d][%d]= %d;", i, j, part_point_type[i][j]);  qDebug() << buf;
				sprintf(buf, "part_ring_num[%d][%d]= %d;", i, j, part_ring_num[i][j]);      qDebug() << buf;
				sprintf(buf, "part_point_num[%d][%d]= %d;", i, j, part_point_num[i][j]);    qDebug() << buf;
			}
		}
		qDebug() << "";

		sprintf(buf, "n_abs_support_radii = %d;", n_abs_support_radii); qDebug() << buf;
		sprintf(buf, "n_basis_ring_found = %d;", n_basis_ring_found);   qDebug() << buf;
		sprintf(buf, "n_basis_min_found = %d;", n_basis_min_found);     qDebug() << buf;
		sprintf(buf, "n_mesh_rings_found = %d;", n_mesh_rings_found);  qDebug() << buf;
		sprintf(buf, "n_mesh_depth = %d;", n_mesh_depth);              qDebug() << buf;
		sprintf(buf, "n_mesh_depth_found = %d;", n_mesh_depth_found);  qDebug() << buf;
		sprintf(buf, "tilt_support_mode = %d;", tilt_support_mode);    qDebug() << buf;

		for (int i = 0; i < n_basis_ring_found; i++)
		{
			sprintf(buf, "basis_ring[%d]= %d;", i, basis_ring[i]);         qDebug() << buf;
			sprintf(buf, "basis_min[%d]= %d;", i, basis_min[i]);         qDebug() << buf;
		}
		for (int i = 0; parm_list[i].name[0]; i++)
		{
			if (*parm_list[i].num_found == 0) continue;
			sprintf(buf, "// %s (%d)", parm_list[i].name, i); qDebug() << buf;
			sprintf(buf, "  parm_list[%d].num_found= %d;", i, *parm_list[i].num_found);         qDebug() << buf;
			sprintf(buf, "  parm_list[%d].affects_basis= %d;", i, parm_list[i].affects_basis);         qDebug() << buf;
			for (int j = 0; j < *parm_list[i].num_found; j++)
			{
				sprintf(buf, "  parm_list[%d].bound_to_var[%d]= %d;", i, j, parm_list[i].bound_to_var[j]);         qDebug() << buf;
				if (parm_list[i].type_flag == parm_type_double)
					sprintf(buf, "  parm_list[%d].dptr[%d]= %f;", i, j, parm_list[i].dptr[j]);
				else
					sprintf(buf, "  parm_list[%d].iptr[%d]= %d;", i, j, parm_list[i].iptr[j]);
				qDebug() << buf;
			} 
		}

		sprintf(buf, "n_optimize_vars= %d;", n_optimize_vars); qDebug() << buf;
		for (int i=0; i<n_optimize_vars; i++)
		{
			sprintf(buf, "opt_var_which[%d]= %d;", i, opt_var_which[i]); qDebug() << buf;
			sprintf(buf, "opt_var_index[%d]= %d;", i, opt_var_index[i]); qDebug() << buf;
			sprintf(buf, "opt_var_is_var[%d]= %d;", i, opt_var_is_var[i]); qDebug() << buf;
			sprintf(buf, "opt_var_step[%d]= %f;", i, opt_var_step[i]); qDebug() << buf;
		}
		sprintf(buf, "n_monte_vars= %d;", n_monte_vars); qDebug() << buf;
		sprintf(buf, "n_scan_vars= %d;", n_scan_vars); qDebug() << buf;
		sprintf(buf, "n_scan_set_vars= %d;", n_scan_set_vars); qDebug() << buf;
	#endif

		/////////////////////////////////////////////////////////////////////
		mutex.unlock();
		mesToUse->timer.start(100, mesToUse);
		mesThread= new MesThread(nullptr); mesThread->start();
		emit calcChanged();
}

void err_msg(char const *s)  { if (CBSModel::singleton==nullptr) return; CBSModel::singleton->setErrMsg(CBSModel::singleton->getErrMsg()+s); }
void DebugMsg(char const *s) { if (CBSModel::singleton==nullptr) return; CBSModel::singleton->setErrMsg(CBSModel::singleton->getErrMsg()+s); }
void warn_msg(char const *s) { if (CBSModel::singleton==nullptr) return; CBSModel::singleton->setErrMsg(CBSModel::singleton->getErrMsg()+s); }
void prompt_exit() {}
void UpdateCellEditOptVars() // vars to optimized have been changed...
{
	mutex.lock();
	if (mesToUse==nullptr) { mutex.unlock(); return; }
	copy_plot_data();
	if (!std::isnan(plate_err_vis_rms)) mesToUse->setErrRms(plate_err_vis_rms);
	if (!std::isnan(plate_err_vis_p_v)) mesToUse->setErrPv(plate_err_vis_p_v);
	mesToUse->setRefocusFL(plate_refocus_focal_length);
	mutex.unlock();
} 
void UpdateRunOptStatus (int n_ev, double rel_step)
{
	mutex.lock();
	if (mesToUse==nullptr) { mutex.unlock(); return; }
	copy_plot_data();
	mesToUse->setNbEvals(n_ev);
	mesToUse->setStepSize(rel_step);
	mutex.unlock();
}
void UpdateMonteDisplay(int ntests, double avgerr, double maxerr) { (void)ntests; (void)avgerr; (void)maxerr; } // not used
void terminate_plop() {} // throw exception!
static QMutex lock;
void LockPlopData(int) { lock.lock(); }
void UnlockPlopData() { lock.unlock(); }
void UpdateSparseStat(int n) { if (mesToUse==nullptr) return; mesToUse->setMatrixProgresses(double(n)/100); } // track bar for solving matrix max = SparseStateSteps


void CBScopeMes::paint(QPainter *painter)
{
	QBrush brush(QColor(200, 200, 200));
	painter->setBrush(brush);
	painter->setPen(QPen(QColor(0, 0, 0)));
	painter->setRenderHint(QPainter::Antialiasing);

	QSizeF itemSize = size();
	int w= int(itemSize.width());
	int h= int(itemSize.height());
	painter->drawRect(0, 0, w, h);
	if (plate_plot_n_triangles==0) return;
	mutex.lock();


	QPoint c(w/2, h/2);
	double const zs[]= {100.0, 75.0, 50.0, 25.0, 10.0, 5.0};
	double dpi= 96.0/25.4*zs[int(_zoom)]/100;
	if (_showMesh || _showForces)
	{
		if (_showMesh) painter->setPen(QPen(QColor(0,0,0))); else painter->setPen(QPen(QColor(0,0,0,0)));
		painter->setBrush(QBrush(QColor(0, 0, 0, 255)));
		double off= 0.0, fact= 1.0;
		if (_showForces)
		{
			double zmin= 1e300, zmax= -1e300; int mi, Mi;
			for (int i=0; i<plate_plot_n_triangles; i++)
			{
				if (abs(plate_plot_z_displacement[i])>1) continue;
				if (zmin>plate_plot_z_displacement[i]) zmin= plate_plot_z_displacement[i], mi= i;
				if (zmax<plate_plot_z_displacement[i]) zmax= plate_plot_z_displacement[i], Mi= i;
			}
			off= zmin;
			if (zmax-zmin>1e-20) fact= 255/(zmax-zmin);
		}
		painter->setBrush(QBrush(QColor(0, 0, 0, 0)));
		for (int i=0; i<plate_plot_n_triangles; i++)
		{
			QPoint p[3];
			p[0]= QPoint(plate_plot_point_x[plate_plot_triangle_points[i][0]]*dpi+c.x(), plate_plot_point_y[plate_plot_triangle_points[i][0]]*dpi+c.y());
			p[1]= QPoint(plate_plot_point_x[plate_plot_triangle_points[i][1]]*dpi+c.x(), plate_plot_point_y[plate_plot_triangle_points[i][1]]*dpi+c.y());
			p[2]= QPoint(plate_plot_point_x[plate_plot_triangle_points[i][2]]*dpi+c.x(), plate_plot_point_y[plate_plot_triangle_points[i][2]]*dpi+c.y());
			if (_showForces)
			{
				double prop= (plate_plot_z_displacement[plate_plot_triangle_points[i][0]] + plate_plot_z_displacement[plate_plot_triangle_points[i][1]] + plate_plot_z_displacement[plate_plot_triangle_points[i][2]])/3.0;
				prop= (prop-off)*fact;
				painter->setBrush(QBrush(QColor(0, int(prop), int(255-prop))));
			}
			painter->drawConvexPolygon(p, 3);
		}
	}
	if (_showParts)
	{
		painter->setBrush(QBrush(QColor(0, 0, 0, 0)));
		for (int i = 0; i < n_parts; i++)
		{	int ncorners = part_type_num_corners [part_type [i]];
			for (int isubpart = 0; isubpart < part_quantity [i]; isubpart++)
			{   
				QPen p(QColor(0,0,0)); p.setWidth(3); painter->setPen(p); // pick a color
				for (int j = 0; j < ncorners; j++)
				{	
					double xs, ys, xe, ye;
					polar_to_euc (part_corner_radius [i] [j], part_corner_angle [i] [j] + isubpart * (double) degrees / part_quantity [i], &xs, &ys);
					polar_to_euc (part_corner_radius [i] [(j + 1) % ncorners], part_corner_angle [i] [(j + 1) % ncorners] + isubpart * (double) degrees / part_quantity [i], &xe, &ye);
					painter->drawLine(QPoint(xs*dpi+c.x(), ys*dpi+c.y()), QPoint(xe*dpi+c.x(), ye*dpi+c.y()));
				}
				double xs, ys;
				polar_to_euc (part_cg_radius [i],part_cg_angle [i] + isubpart * (double) degrees / part_quantity [i],&xs, &ys);
				painter->setPen(QPen(QColor(255,0,0)));
				painter->drawEllipse(QPoint(xs*dpi+c.x(), ys*dpi+c.y()), 10, 10);
			}
		}
	}
	if (_showSupports)
	{
		painter->setBrush(QBrush(QColor(0, 0, 0, 0)));
		painter->setPen(QPen(QColor(255,0,0)));
		for (int i = 0; i < n_support_radii; i++)
		{	for (int j = 0; j < num_support [i]; j++)
			{	double angle = support_angle [i] + j * (double) degrees / num_support [i];
				double xs, ys;
				polar_to_euc (support_radii [i], angle, &xs, &ys);
				painter->drawEllipse(QPoint(xs*dpi+c.x(), ys*dpi+c.y()), 10, 10);
			}
		}
	}

	if (_radius!=0.0)
	{
		QPen p(QColor(0,0,0)); p.setWidth(3); painter->setPen(p); // pick a color
		painter->drawEllipse(c, int(_radius*dpi), int(_radius*dpi));
	}
	if (_secondary!=0.0 && _showSecondary)
	{
		QPen p(QColor(200,0,0)); p.setWidth(2); painter->setPen(p); // pick a color
		painter->drawEllipse(c, int(_secondary*dpi), int(_secondary*dpi));
	}

	mutex.unlock();
}