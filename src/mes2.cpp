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
        QTime s= QTime::currentTime();
		init_default_basis();
		plate_init();
		run_plate();
		mesThread= nullptr;
        mesToUse->setMatrixProgresses(s.elapsed());
        emit mesToUse->calcChanged();
	}
};

bool CBScopeMes::getCalc() { return mesThread!=nullptr; }
void CBScopeMes::doMesStop() { please_stop_plop= 1; }

void CBScopeMes::createMirror(double diametre, double secondary, double thickness, double young, double poisson, double focale, double density, int _cellType)
{ 
	if (mesThread!=nullptr) return;
	_radius= diametre/2.0; _secondary= secondary/2.0; this->_cellType= _cellType;
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
	struct { int basis_ring, n_mesh_rings, n_num_support_rings; struct { double v; int boud_to_var, nb; } sup_radiis[9];
             int n_parts; struct { int part_type, part_quantity, part_point_type[12], part_ring_num[12], part_point_num[12]; } parts[5];
             int n_support_angle; struct { double v; int boud_to_var; } support_angles[9];
             int n_basis_ring_found, basis_ring_found[9];
             int n_optimize_vars; struct { int opt_var_which, opt_var_index, opt_var_is_var; double opt_var_step; } optimize_vars[10];
             int n_variables, nbv; var_def vars[17];
	       }
        const cellDefs[]={
              // 3 points
              { 0, 8, 1, {{0.400000, -1, 3}, }, // mesh rings and support rings defs
                0, {}, // parts
                1, {{0.000000, -1}, }, //support-angle (19)
                1, {3, }, // basis-ring-size (21)
                1, {{14, 0, 0, 0.010000},}, // vars to optimize
                0, 0, {}, // vars
              },
              // 6 points
              { 0, 12, 1, {{0.600000, -1, 6}, }, // mesh rings and support rings defs
                1, {{1, 3, {0,0,0,},{0,0,0,},{0,1,0,}},}, // parts
                1, {{0.000000, -1}, }, //support-angle (19)
                1, {3, }, // basis-ring-size (21)
                1, {{14, 0, 0, 0.010000},}, // vars to optimize
                0, 0, {}, // vars
              },
              // 9: variable_angles
              { 0, 12, 3, {{0.000000, 0, 3}, {0.000000, 1, 3}, {0.000000, 1, 3}, }, // mesh rings and support rings defs
                1, {{0, 3, {0,0,0,},{0,1,2,},{0,0,0,}},}, // parts
                3, {{0.000000, -1}, {0.000000, 3}, {0.000000, 4}, }, //support-angle (19)
                1, {3, }, // basis-ring-size (21)
                3, {{0, -1, 1, 0.010000},{1, -1, 1, 0.010000},{2, -1, 1, 1.000000},}, // vars to optimize
                5, 5, {{"r_inner", 0.303403, 0, 1, {0, 0.303403, 0}, {0, 0.000000, 0}},{"r_outer", 0.508024, 0, 1, {0, 0.508024, 0}, {0, 0.000000, 0}},{"angle", 34.060700, 0, 1, {0, 34.060700, 0}, {0, 0.000000, 0}},{"da1", 0.000000, 0, 2, {0, 0.000000, 0}, {1, 0.000000, 2}},{"da2", 0.000000, 0, 3, {0, 0.000000, 0}, {1, 0.000000, 2}},}, // vars
              },
              // 9: fixed_angles
              { 0, 12, 3, {{0.000000, 0, 3}, {0.000000, 1, 3}, {0.000000, 1, 3}, }, // mesh rings and support rings defs
                1, {{0, 3, {0,0,0,},{0,1,2,},{0,0,2,}},}, // parts
                3, {{0.000000, -1}, {30.000000, -1}, {90.000000, -1}, }, //support-angle (19)
                1, {3, }, // basis-ring-size (21)
                2, {{0, -1, 1, 0.010000},{1, -1, 1, 0.010000},}, // vars to optimize
                2, 5, {{"r_inner", 0.253672, 0, 1, {0, 0.253672, 0}, {0, 0.000000, 0}},{"r_outer", 0.559738, 0, 1, {0, 0.559738, 0}, {0, 0.000000, 0}},{"angle", 40.213209, 0, 1, {0, 34.060700, 0}, {0, 0.000000, 0}},{"da1", 40.213043, 0, 2, {0, 0.000000, 0}, {1, 40.213043, 2}},{"da2", -40.213043, 0, 3, {0, 0.000000, 0}, {1, 40.213043, 2}},}, // vars
              },
              // 12
              { 3, 20, 4, {{0.000000, 0, 3}, {0.000000, 1, 3}, {0.000000, 1, 3}, {0.000000, 1, 3}, }, // mesh rings and support rings defs
                3, {{1, 3, {0,0,0,},{0,2,0,},{0,0,0,}},{1, 3, {0,0,0,},{1,3,0,},{0,0,0,}},{1, 3, {1,1,0,},{0,1,0,},{0,0,0,}},}, // parts
                4, {{120.000000, -1}, {60.000000, -1}, {100.000000, -1}, {20.000000, -1}, }, //support-angle (19)
                1, {3, }, // basis-ring-size (21)
                3, {{0, -1, 1, 0.010000},{1, -1, 1, 0.010000},{16, 0, 0, 0.010000},}, // vars to optimize
                2, 2, {{"r0", 0.191692, 0, 1, {0, 0.191692, 0}, {0, 0.000000, 0}},{"r1", 0.713727, 0, 1, {0, 0.713727, 0}, {0, 0.000000, 0}},}, // vars
              },
              // 18: variable_angles
              { 0, 20, 3, {{0.000000, 0, 6}, {0.000000, 1, 6}, {0.000000, 1, 6}, }, // mesh rings and support rings defs
                2, {{0, 6, {0,0,0,1,1,0,},{0,1,2,0,0,0,},{0,0,0,0,1,0,}},{1, 3, {1,1,0,},{0,0,0,},{0,1,0,}},}, // parts
                3, {{0.000000, -1}, {0.000000, 3}, {0.000000, 4}, }, //support-angle (19)
                1, {6, }, // basis-ring-size (21)
                3, {{0, -1, 1, 0.010000},{1, -1, 1, 0.010000},{2, -1, 1, 1.000000},}, // vars to optimize
                5, 5, {{"r_inner", 0.360449, 0, 1, {0, 0.360449, 0}, {0, 0.000000, 0}},{"r_outer", 0.795996, 0, 1, {0, 0.795996, 0}, {0, 0.000000, 0}},{"angle", 15.740600, 0, 1, {0, 15.740600, 0}, {0, 0.000000, 0}},{"a0p", 0.000000, 0, 2, {0, 0.000000, 0}, {1, 40.213043, 2}},{"a0m", 0.000000, 0, 3, {0, 0.000000, 0}, {1, 40.213043, 2}},}, // vars
              },
              // 18: fixed_angles
              { 0, 20, 2, {{0.371456, -1, 6}, {0.799131, -1, 12}, }, // mesh rings and support rings defs
                2, {{0, 6, {0,0,0,1,1,0,},{0,1,1,0,0,0,},{0,0,11,0,1,0,}},{1, 3, {1,1,0,},{0,0,0,},{0,1,0,}},}, // parts
                2, {{0.000000, -1}, {15.000000, -1}, }, //support-angle (19)
                1, {6, }, // basis-ring-size (21)
                2, {{14, 0, 0, 0.010000},{14, 1, 0, 0.010000},}, // vars to optimize
                0, 5, {{"r_inner", 0.396028, 0, 1, {0, 0.360449, 0}, {0, 0.000000, 0}},{"r_outer", 0.794392, 0, 1, {0, 0.795996, 0}, {0, 0.000000, 0}},{"angle", 15.329860, 0, 1, {0, 15.740600, 0}, {0, 0.000000, 0}},{"a0p", 15.329860, 0, 2, {0, 0.000000, 0}, {1, 15.329860, 2}},{"a0m", -15.329860, 0, 3, {0, 0.000000, 0}, {1, 15.329860, 2}},}, // vars
              },
              // 27: variable_angles
              { 0, 20, 9, {{0.000000, 0, 3}, {0.000000, 0, 3}, {0.000000, 1, 3}, {0.000000, 1, 3}, {0.000000, 2, 3}, {0.000000, 2, 3}, {0.000000, 3, 3}, {0.000000, 4, 3}, {0.000000, 4, 3}, }, // mesh rings and support rings defs
                4, {{0, 3, {0,0,0,},{6,2,3,},{0,0,0,}},{0, 3, {0,0,0,},{0,4,7,},{0,0,0,}},{0, 3, {0,0,0,},{1,5,8,},{0,0,0,}},{0, 3, {1,1,1,},{0,1,2,},{0,0,0,}},}, // parts
                9, {{0.000000, 9}, {0.000000, 10}, {0.000000, 11}, {0.000000, 12}, {0.000000, 13}, {0.000000, 14}, {60.000000, -1}, {0.000000, 15}, {0.000000, 16}, }, //support-angle (19)
                1, {3, }, // basis-ring-size (21)
                9, {{0, -1, 1, 0.010000},{1, -1, 1, 0.010000},{2, -1, 1, 0.010000},{3, -1, 1, 0.010000},{4, -1, 1, 0.010000},{5, -1, 1, 1.000000},{6, -1, 1, 1.000000},{7, -1, 1, 1.000000},{8, -1, 1, 1.000000},}, // vars to optimize
                17, 17, {{"r0", 0.343039, 0, 1, {0, 0.343039, 0}, {0, 0.000000, 0}},{"r1", 0.818302, 0, 1, {0, 0.818302, 0}, {0, 0.000000, 0}},{"r2", 0.775977, 0, 1, {0, 0.775977, 0}, {0, 0.000000, 0}},{"r3", 0.622112, 0, 1, {0, 0.622112, 0}, {0, 0.000000, 0}},{"r4", 0.767846, 0, 1, {0, 0.767846, 0}, {0, 0.000000, 0}},{"da0", 36.629300, 0, 1, {0, 36.629300, 0}, {0, 0.000000, 0}},{"da1", 14.148800, 0, 1, {0, 14.148800, 0}, {0, 0.000000, 0}},{"da2", 50.457300, 0, 1, {0, 50.457300, 0}, {0, 0.000000, 0}},{"da4", 34.313700, 0, 1, {0, 34.313700, 0}, {0, 0.000000, 0}},{"a0p", 60.000000, 0, 2, {0, 60.000000, 0}, {1, 0.000000, 5}},{"a0m", 60.000000, 0, 3, {0, 60.000000, 0}, {1, 0.000000, 5}},{"a1p", 60.000000, 0, 2, {0, 60.000000, 0}, {1, 0.000000, 6}},{"a1m", 60.000000, 0, 3, {0, 60.000000, 0}, {1, 0.000000, 6}},{"a2p", 60.000000, 0, 2, {0, 60.000000, 0}, {1, 0.000000, 7}},{"a2m", 60.000000, 0, 3, {0, 60.000000, 0}, {1, 0.000000, 7}},{"a4p", 60.000000, 0, 2, {0, 60.000000, 0}, {1, 0.000000, 8}},{"a4m", 60.000000, 0, 3, {0, 60.000000, 0}, {1, 0.000000, 8}},}, // vars
              },
              // 27: fixed_angles
              { 0, 20, 3, {{0.000000, 0, 6}, {0.000000, 1, 9}, {0.000000, 2, 12}, }, // mesh rings and support rings defs
                4, {{0, 3, {0,0,0,},{1,2,2,},{0,0,1,}},{0, 3, {0,0,0,},{1,2,2,},{2,2,3,}},{0, 3, {0,0,0,},{0,0,1,},{0,1,1,}},{0, 3, {1,1,1,},{0,1,2,},{0,0,0,}},}, // parts
                3, {{30.000000, -1}, {20.000000, -1}, {15.000000, -1}, }, //support-angle (19)
                1, {3, }, // basis-ring-size (21)
                3, {{0, -1, 1, 0.010000},{1, -1, 1, 0.010000},{2, -1, 1, 0.010000},}, // vars to optimize
                3, 17, {{"radius0", 0.331522, 0, 1, {0, 0.331522, 0}, {0, 0.000000, 0}},{"radius1", 0.666744, 0, 1, {0, 0.666744, 0}, {0, 0.000000, 0}},{"radius2", 0.874839, 0, 1, {0, 0.874839, 0}, {0, 0.000000, 0}},{"r3", 0.537722, 0, 1, {0, 0.622112, 0}, {0, 0.000000, 0}},{"r4", 0.764024, 0, 1, {0, 0.767846, 0}, {0, 0.000000, 0}},{"da0", 37.278965, 0, 1, {0, 36.629300, 0}, {0, 0.000000, 0}},{"da1", 12.758612, 0, 1, {0, 14.148800, 0}, {0, 0.000000, 0}},{"da2", 50.648333, 0, 1, {0, 50.457300, 0}, {0, 0.000000, 0}},{"da4", 33.240835, 0, 1, {0, 34.313700, 0}, {0, 0.000000, 0}},{"a0p", 97.278965, 0, 2, {0, 60.000000, 0}, {1, 37.278965, 5}},{"a0m", 22.721035, 0, 3, {0, 60.000000, 0}, {1, 37.278965, 5}},{"a1p", 72.758612, 0, 2, {0, 60.000000, 0}, {1, 12.758612, 6}},{"a1m", 47.241388, 0, 3, {0, 60.000000, 0}, {1, 12.758612, 6}},{"a2p", 110.648333, 0, 2, {0, 60.000000, 0}, {1, 50.648333, 7}},{"a2m", 9.351667, 0, 3, {0, 60.000000, 0}, {1, 50.648333, 7}},{"a4p", 93.240835, 0, 2, {0, 60.000000, 0}, {1, 33.240835, 8}},{"a4m", 26.759165, 0, 3, {0, 60.000000, 0}, {1, 33.240835, 8}},}, // vars
              },
              // 36: variable_angles
              { 6, 24, 6, {{0.000000, 0, 6}, {0.000000, 1, 6}, {0.000000, 2, 6}, {0.000000, 4, 6}, {0.000000, 3, 6}, {0.000000, 3, 6}, }, // mesh rings and support rings defs
                4, {{0, 6, {0,0,0,0,0,0,},{0,2,4,1,5,3,},{0,0,0,1,1,0,}},{0, 6, {0,0,0,1,1,0,},{1,5,3,0,1,0,},{1,1,0,0,0,0,}},{1, 6, {1,1,0,1,1,0,},{0,1,0,2,2,0,},{0,0,0,0,1,0,}},{1, 3, {1,1,0,},{2,2,0,},{0,1,0,}},}, // parts
                6, {{0.000000, -1}, {0.000000, -1}, {30.000000, -1}, {30.000000, -1}, {0.000000, 7}, {0.000000, 8}, }, //support-angle (19)
                1, {6, }, // basis-ring-size (21)
                6, {{0, -1, 1, 0.010000},{1, -1, 1, 0.010000},{2, -1, 1, 0.010000},{3, -1, 1, 0.010000},{4, -1, 1, 0.010000},{7, -1, 1, 1.000000},}, // vars to optimize
                9, 17, {{"r1", 0.279529, 0, 1, {0, 0.279529, 0}, {0, 0.000000, 0}},{"r2", 0.623028, 0, 1, {0, 0.623028, 0}, {0, 0.000000, 0}},{"r2b", 0.584620, 0, 1, {0, 0.584620, 0}, {0, 0.000000, 0}},{"r3", 0.860814, 0, 1, {0, 0.860814, 0}, {0, 0.000000, 0}},{"r3b", 0.850884, 0, 1, {0, 0.850884, 0}, {0, 0.000000, 0}},{"a2", 16.242500, 0, 1, {0, 16.242500, 0}, {0, 0.000000, 0}},{"m2", 0.000000, 0, 3, {0, 0.000000, 0}, {1, 16.242500, 5}},{"a3", 10.593900, 0, 1, {0, 10.593900, 0}, {0, 0.000000, 0}},{"m3", 0.000000, 0, 3, {0, 0.000000, 0}, {1, 10.585153, 7}},{"a0p", 97.278965, 0, 2, {0, 60.000000, 0}, {1, 37.278965, 5}},{"a0m", 22.721035, 0, 3, {0, 60.000000, 0}, {1, 37.278965, 5}},{"a1p", 72.758612, 0, 2, {0, 60.000000, 0}, {1, 12.758612, 6}},{"a1m", 47.241388, 0, 3, {0, 60.000000, 0}, {1, 12.758612, 6}},{"a2p", 110.648333, 0, 2, {0, 60.000000, 0}, {1, 50.648333, 7}},{"a2m", 9.351667, 0, 3, {0, 60.000000, 0}, {1, 50.648333, 7}},{"a4p", 93.240835, 0, 2, {0, 60.000000, 0}, {1, 33.240835, 8}},{"a4m", 26.759165, 0, 3, {0, 60.000000, 0}, {1, 33.240835, 8}},}, // vars
              },
              // 36: fixed_angles
              { 6, 24, 3, {{0.000000, 0, 6}, {0.000000, 1, 12}, {0.000000, 2, 18}, }, // mesh rings and support rings defs
                5, {{1, 6, {0,0,0,0,0,0,},{0,1,0,2,2,0,},{0,0,0,0,1,0,}},{1, 6, {0,0,0,0,0,0,},{2,2,0,1,2,0,},{0,1,0,1,2,0,}},{1, 6, {0,0,0,1,1,1,},{1,2,0,0,1,2,},{1,2,0,0,0,0,}},{0, 6, {1,1,1,1,1,0,},{0,1,2,3,3,0,},{0,0,0,0,1,0,}},{1, 3, {1,1,0,},{3,3,0,},{0,1,0,}},}, // parts
                0, {}, //support-angle (19)
                1, {6, }, // basis-ring-size (21)
                3, {{0, -1, 1, 0.010000},{1, -1, 1, 0.010000},{2, -1, 1, 0.010000},}, // vars to optimize
                3, 17, {{"r1", 0.270178, 0, 1, {0, 0.270178, 0}, {0, 0.000000, 0}},{"r2", 0.581761, 0, 1, {0, 0.581761, 0}, {0, 0.000000, 0}},{"r3", 0.842222, 0, 1, {0, 0.842222, 0}, {0, 0.000000, 0}},{"r3", 0.860814, 0, 1, {0, 0.860814, 0}, {0, 0.000000, 0}},{"r3b", 0.850884, 0, 1, {0, 0.850884, 0}, {0, 0.000000, 0}},{"a2", 16.242500, 0, 1, {0, 16.242500, 0}, {0, 0.000000, 0}},{"m2", 0.000000, 0, 3, {0, 0.000000, 0}, {1, 16.242500, 5}},{"a3", 10.593900, 0, 1, {0, 10.593900, 0}, {0, 0.000000, 0}},{"m3", 0.000000, 0, 3, {0, 0.000000, 0}, {1, 10.585153, 7}},{"a0p", 97.278965, 0, 2, {0, 60.000000, 0}, {1, 37.278965, 5}},{"a0m", 22.721035, 0, 3, {0, 60.000000, 0}, {1, 37.278965, 5}},{"a1p", 72.758612, 0, 2, {0, 60.000000, 0}, {1, 12.758612, 6}},{"a1m", 47.241388, 0, 3, {0, 60.000000, 0}, {1, 12.758612, 6}},{"a2p", 110.648333, 0, 2, {0, 60.000000, 0}, {1, 50.648333, 7}},{"a2m", 9.351667, 0, 3, {0, 60.000000, 0}, {1, 50.648333, 7}},{"a4p", 93.240835, 0, 2, {0, 60.000000, 0}, {1, 33.240835, 8}},{"a4m", 26.759165, 0, 3, {0, 60.000000, 0}, {1, 33.240835, 8}},}, // vars
              },
              // 54: variable_angles
              { 6, 24, 4, {{0.000000, 0, 6}, {0.000000, 1, 12}, {0.000000, 2, 12}, {0.000000, 3, 24}, }, // mesh rings and support rings defs
                4, {{0, 6, {0,0,0,0,0,0,},{0,1,1,2,3,3,},{0,0,11,0,1,0,}},{0, 12, {0,0,0,1,1,1,1,1,0,1,1,0,},{2,3,3,0,1,1,2,2,0,3,3,0,},{0,1,0,0,0,11,0,1,0,0,1,0,}},{0, 6, {1,1,1,1,1,0,},{0,1,1,2,2,0,},{0,0,11,0,1,0,}},{1, 3, {1,1,0,},{2,2,0,},{0,1,0,}},}, // parts
                4, {{0.000000, -1}, {15.000000, -1}, {15.000000, -1}, {7.500000, -1}, }, //support-angle (19)
                1, {6, }, // basis-ring-size (21)
                4, {{0, -1, 1, 0.010000},{1, -1, 1, 0.010000},{2, -1, 1, 0.010000},{3, -1, 1, 0.010000},}, // vars to optimize
                4, 17, {{"r0", 0.216033, 0, 1, {0, 0.216033, 0}, {0, 0.000000, 0}},{"r1", 0.478200, 0, 1, {0, 0.478200, 0}, {0, 0.000000, 0}},{"r2", 0.672383, 0, 1, {0, 0.672383, 0}, {0, 0.000000, 0}},{"r3", 0.886919, 0, 1, {0, 0.886919, 0}, {1, 15.329860, 2}},{"r3b", 0.841457, 0, 1, {0, 0.850884, 0}, {1, 15.329860, 2}},{"a2", 16.242500, 0, 1, {0, 16.242500, 0}, {0, 0.000000, 0}},{"m2", -16.242500, 0, 3, {0, 0.000000, 0}, {1, 16.242500, 5}},{"a3", 10.585153, 0, 1, {0, 10.593900, 0}, {0, 0.000000, 0}},{"m3", -10.585153, 0, 3, {0, 0.000000, 0}, {1, 10.585153, 7}},{"a0p", 97.002877, 0, 2, {0, 60.000000, 0}, {1, 37.002877, 5}},{"a0m", 22.997123, 0, 3, {0, 60.000000, 0}, {1, 37.002877, 5}},{"a1p", 72.851547, 0, 2, {0, 60.000000, 0}, {1, 12.851547, 6}},{"a1m", 47.148453, 0, 3, {0, 60.000000, 0}, {1, 12.851547, 6}},{"a2p", 110.707862, 0, 2, {0, 60.000000, 0}, {1, 50.707862, 7}},{"a2m", 9.292138, 0, 3, {0, 60.000000, 0}, {1, 50.707862, 7}},{"a4p", 93.322782, 0, 2, {0, 60.000000, 0}, {1, 33.322782, 8}},{"a4m", 26.677218, 0, 3, {0, 60.000000, 0}, {1, 33.322782, 8}},}, // vars
              },
              // 54: fixed_angles
              { 6, 24, 4, {{0.000000, 0, 6}, {0.000000, 1, 12}, {0.000000, 2, 12}, {0.000000, 3, 24}, }, // mesh rings and support rings defs
                4, {{0, 6, {0,0,0,0,0,0,},{0,1,1,2,3,3,},{0,0,11,0,1,0,}},{0, 12, {0,0,0,1,1,1,1,1,0,1,1,0,},{2,3,3,0,1,1,2,2,0,3,3,0,},{0,1,0,0,0,11,0,1,0,0,1,0,}},{0, 6, {1,1,1,1,1,0,},{0,1,1,2,2,0,},{0,0,11,0,1,0,}},{1, 3, {1,1,0,},{2,2,0,},{0,1,0,}},}, // parts
                0, {}, //support-angle (19)
                1, {6, }, // basis-ring-size (21)
                4, {{0, -1, 1, 0.010000},{1, -1, 1, 0.010000},{2, -1, 1, 0.010000},{3, -1, 1, 0.010000},}, // vars to optimize
                4, 17, {{"r0", 0.216033, 0, 1, {0, 0.216033, 0}, {0, 0.000000, 0}},{"r1", 0.478200, 0, 1, {0, 0.478200, 0}, {0, 0.000000, 0}},{"r2", 0.672383, 0, 1, {0, 0.672383, 0}, {0, 0.000000, 0}},{"r3", 0.886919, 0, 1, {0, 0.886919, 0}, {1, 15.329860, 2}},{"r3b", 0.841457, 0, 1, {0, 0.850884, 0}, {1, 15.329860, 2}},{"a2", 16.242500, 0, 1, {0, 16.242500, 0}, {0, 0.000000, 0}},{"m2", -16.242500, 0, 3, {0, 0.000000, 0}, {1, 16.242500, 5}},{"a3", 10.585153, 0, 1, {0, 10.593900, 0}, {0, 0.000000, 0}},{"m3", -10.585153, 0, 3, {0, 0.000000, 0}, {1, 10.585153, 7}},{"a0p", 97.002877, 0, 2, {0, 60.000000, 0}, {1, 37.002877, 5}},{"a0m", 22.997123, 0, 3, {0, 60.000000, 0}, {1, 37.002877, 5}},{"a1p", 72.851547, 0, 2, {0, 60.000000, 0}, {1, 12.851547, 6}},{"a1m", 47.148453, 0, 3, {0, 60.000000, 0}, {1, 12.851547, 6}},{"a2p", 110.707862, 0, 2, {0, 60.000000, 0}, {1, 50.707862, 7}},{"a2m", 9.292138, 0, 3, {0, 60.000000, 0}, {1, 50.707862, 7}},{"a4p", 93.322782, 0, 2, {0, 60.000000, 0}, {1, 33.322782, 8}},{"a4m", 26.677218, 0, 3, {0, 60.000000, 0}, {1, 33.322782, 8}},}, // vars
              },
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
		n_rel_force= 0;
		n_support_angle= 0; // can be set to n_num_support_rings if we want the angles to vary...
		n_basis_min_found = 1;
		n_mesh_rings_found = 1;
		n_mesh_depth = 5;
		n_mesh_depth_found = 1;
		tilt_support_mode = 1;

		n_parts = cellDefs[_cellType].n_parts;
		for (int i = 0; i < n_parts; i++)
		{
			part_type[i] = cellDefs[_cellType].parts[i].part_type;
			part_quantity[i] = cellDefs[_cellType].parts[i].part_quantity;
			for (int j = 0; j < part_quantity[i]; j++) part_point_type[i][j] = cellDefs[_cellType].parts[i].part_point_type[j];
			for (int j = 0; j < part_quantity[i]; j++) part_ring_num[i][j] = cellDefs[_cellType].parts[i].part_ring_num[j];
			for (int j = 0; j < part_quantity[i]; j++) part_point_num[i][j] = cellDefs[_cellType].parts[i].part_point_num[j];
		}

		basis_ring[0] = cellDefs[_cellType].basis_ring;
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
			parm_list[14].dptr[i] = cellDefs[_cellType].sup_radiis[i].v;
            parm_list[14].bound_to_var[i] = cellDefs[_cellType].sup_radiis[i].boud_to_var;
        }
		// num-support (17)
		*parm_list[17].num_found = cellDefs[_cellType].n_num_support_rings;
		parm_list[17].affects_basis = 0;
		for (int i = 0; i < cellDefs[_cellType].n_num_support_rings; i++) parm_list[17].bound_to_var[i] = -1;
		for (int i = 0; i < cellDefs[_cellType].n_num_support_rings; i++) parm_list[17].iptr[i] = cellDefs[_cellType].sup_radiis[i].nb;
		// support-angle (19)
		*parm_list[19].num_found = cellDefs[_cellType].n_support_angle;
		parm_list[19].affects_basis = 0;
		for (int i = 0; i < cellDefs[_cellType].n_support_angle; i++) parm_list[19].bound_to_var[i] = cellDefs[_cellType].support_angles[i].boud_to_var;
		for (int i = 0; i < cellDefs[_cellType].n_support_angle; i++) parm_list[19].dptr[i] = cellDefs[_cellType].support_angles[i].v;
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

        n_optimize_vars = cellDefs[_cellType].n_optimize_vars;
        for (int i = 0; i < cellDefs[_cellType].n_optimize_vars; i++)
		{
			opt_var_which[i] =  cellDefs[_cellType].optimize_vars[i].opt_var_which;
			opt_var_index[i] =  cellDefs[_cellType].optimize_vars[i].opt_var_index;
			opt_var_is_var[i] = cellDefs[_cellType].optimize_vars[i].opt_var_is_var;
			opt_var_step[i] = cellDefs[_cellType].optimize_vars[i].opt_var_step;
		}
        n_variables = cellDefs[_cellType].n_variables;
        for (int i=0; i<cellDefs[_cellType].nbv; i++) var_table[i]= cellDefs[_cellType].vars[i];

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
		char buf[2000];
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
        sprintf(buf, "n_variables= %d;", n_variables); qDebug() << buf;

        for (int i=0; i<n_optimize_vars; i++)
		{
			sprintf(buf, "opt_var_which[%d]= %d;", i, opt_var_which[i]); qDebug() << buf;
			sprintf(buf, "opt_var_index[%d]= %d;", i, opt_var_index[i]); qDebug() << buf;
			sprintf(buf, "opt_var_is_var[%d]= %d;", i, opt_var_is_var[i]); qDebug() << buf;
			sprintf(buf, "opt_var_step[%d]= %f;", i, opt_var_step[i]); qDebug() << buf;
		}
		int i=0; while (var_table[i].name[0]!=0)
		{
            sprintf(buf, "var_table[%d]= {\"%s\", %f, %d, %d, {%d, %f, %d}, {%d, %f, %d}};", i, var_table[i].name, var_table[i].value, var_table[i].affects_basis, var_table[i].var_def,
                var_table[i].op1.opnd_type, var_table[i].op1.opnd_value, var_table[i].op1.opnd_which_var,
                var_table[i].op2.opnd_type, var_table[i].op2.opnd_value, var_table[i].op2.opnd_which_var
            ); qDebug() << buf;
            i++;
		}
		sprintf(buf, "n_monte_vars= %d;", n_monte_vars); qDebug() << buf;
		sprintf(buf, "n_scan_vars= %d;", n_scan_vars); qDebug() << buf;
		sprintf(buf, "n_scan_set_vars= %d;", n_scan_set_vars); qDebug() << buf;

        //////////////////////////////////////
        // part 2
        char buf2[200];
        sprintf(buf, "  { %d, %d, %d, {", basis_ring[0], parm_list[11].iptr[0], *parm_list[14].num_found); // n_mesh_rings n_num_support_rings
        for (int i = 0; i < *parm_list[14].num_found; i++)
        {
            sprintf(buf2, "{%f, %d, %d}, ", parm_list[14].dptr[i], parm_list[14].bound_to_var[i], parm_list[17].iptr[i]); strcat(buf, buf2);
        }
        strcat(buf, "}, // mesh rings and support rings defs"); qDebug() << buf;

        // parts
        sprintf(buf, "    %d, {", n_parts);
        for (int i = 0; i < n_parts; i++)
        {
            sprintf(buf2, "{%d, %d, ", part_type[i], part_quantity[i]); strcat(buf, buf2);
            strcat(buf, "{");
            for (int j = 0; j < part_quantity[i]; j++) { sprintf(buf2, "%d,", part_point_type[i][j]); strcat(buf, buf2); }
            strcat(buf, "},{");
            for (int j = 0; j < part_quantity[i]; j++) { sprintf(buf2, "%d,", part_ring_num[i][j]); strcat(buf, buf2); }
            strcat(buf, "},{");
            for (int j = 0; j < part_quantity[i]; j++) { sprintf(buf2, "%d,", part_point_num[i][j]); strcat(buf, buf2); }
            strcat(buf, "}},");
        }
        strcat(buf, "}, // parts"); qDebug() << buf;

        // support-angle (19)
        sprintf(buf, "    %d, {", *parm_list[19].num_found);
        for (int i = 0; i < *parm_list[19].num_found; i++)
        {
            sprintf(buf2, "{%f, %d}, ", parm_list[19].dptr[i], parm_list[19].bound_to_var[i]); strcat(buf, buf2);
        }
        strcat(buf, "}, //support-angle (19)"); qDebug() << buf;

        // basis-ring-size (21)
        sprintf(buf, "    %d, {", *parm_list[21].num_found);
        for (int i = 0; i < *parm_list[21].num_found; i++)
        {
            sprintf(buf2, "%d, ", parm_list[21].iptr[i]); strcat(buf, buf2);
        }
        strcat(buf, "}, // basis-ring-size (21)"); qDebug() << buf;

        // vars to optimize
        sprintf(buf, "    %d, {", n_optimize_vars);
        for (int i = 0; i < n_optimize_vars; i++)
        {
            sprintf(buf2, "{%d, %d, %d, %f},", opt_var_which[i], opt_var_index[i], opt_var_is_var[i], opt_var_step[i]); strcat(buf, buf2);
        }
        strcat(buf, "}, // vars to optimize"); qDebug() << buf;

        // vars
        int nbv = 0; while (var_table[nbv].name[0] != 0) nbv++;
        sprintf(buf, "    %d, %d, {", n_variables, nbv);
        for (int i = 0; i < nbv; i++)
        {
            sprintf(buf2, "{\"%s\", %f, %d, %d, {%d, %f, %d}, {%d, %f, %d}},", var_table[i].name, var_table[i].value, var_table[i].affects_basis, var_table[i].var_def,
                var_table[i].op1.opnd_type, var_table[i].op1.opnd_value, var_table[i].op1.opnd_which_var,
                var_table[i].op2.opnd_type, var_table[i].op2.opnd_value, var_table[i].op2.opnd_which_var
            ); strcat(buf, buf2);
        }
        strcat(buf, "}, // vars"); qDebug() << buf;
        qDebug() << "  },";
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

void CBScopeMes::printPLOP()
{
#if CanPrint
	QPrinter printer;//(QPrinter::HighResolution);
	QPrintDialog dialog(&printer, nullptr);
	dialog.setWindowTitle(tr("Print support"));
	if (dialog.exec() != QDialog::Accepted) return;
	QRect area(printer.pageRect());
	QPainter painter;
	painter.begin(&printer);
	painter.beginNativePainting();

	int dpi= printer.resolution();
	QRect r(printer.pageRect());
	QPoint c= r.center();

	paint(&painter, true, c, dpi);
	// add numerical data...
	painter.setPen(QPen(QColor(0, 0, 0)));
	QSize s= r.size()/20;
	c= r.topLeft()+QPoint(s.width(),s.height());
	if (mesToUse!=nullptr)
	{
		QFontMetrics fm(painter.font());                            
		painter.drawText(c, _scope->getName()); c.setY(c.y()+fm.height());
		QString t; t.sprintf("Diameter:%d Focal:%d Thickness:%d Secondary:%d", int(_scope->getDiametre()), int(_scope->getFocal()), int(_scope->getThickness()), int(_scope->getSecondary()), _scope->getDensity(), int(_scope->getYoung()), _scope->getPoisson());
		painter.drawText(c, t); c.setY(c.y()+fm.height());
		t.sprintf("%.2fg/cm^3 young:%d Poisson:%.2f", _scope->getDensity(), int(_scope->getYoung()), _scope->getPoisson());
		painter.drawText(c, t); c.setY(c.y()+fm.height());
		painter.drawText(c, "P-V err: "+QString::number(floor(mesToUse->getErrPv()*1e8)/100)+"nm lam/"+QString::number(floor(555e-6/mesToUse->getErrPv())));
		c.setY(c.y()+fm.height());
		painter.drawText(c, "RMS err: "+QString::number(floor(mesToUse->getErrRms()*1e8)/100)+"nm lam/"+QString::number(floor(555e-6/mesToUse->getErrRms())));
		c.setY(c.y()+fm.height());

		if (_cellType!=0)
			for (int i = 0; i < n_parts; i++)
			{	
				QString parts;
				int ncorners = part_type_num_corners [part_type [i]];
				double x[3], y[3], l[3];
				for (int j = 0; j < ncorners; j++) polar_to_euc (part_corner_radius [i] [j], part_corner_angle [i] [j], &x[j], &y[j]);
				for (int j=0; j<ncorners; j++) l[j]= sqrt((x[(j+1)%ncorners]-x[j])*(x[(j+1)%ncorners]-x[j])+(y[(j+1)%ncorners]-y[j])*(y[(j+1)%ncorners]-y[j]));
				if (part_type [i]==0) parts+= "triangle: cog_radius "+QString::number(part_cg_radius[i], 'f', 1)+" length1 "+QString::number(l[0], 'f', 1)+" length2 "+QString::number(l[1], 'f', 1)+" length3 "+QString::number(l[2], 'f', 1);
				else parts+= "bar: cog_radius "+QString::number(part_cg_radius[i], 'f', 1)+" length1 "+QString::number(l[0], 'f', 1);
				painter.drawText(c, parts);
				c.setY(c.y()+fm.height());
			}
		else painter.drawText(c, "3 points at "+QString::number(support_radii[0], 'f', 1)+"mm radius");
	}
	painter.endNativePainting();
	painter.end();
#endif
}

void CBScopeMes::paint(QPainter *painter, bool print, QPoint c, double dpi)
{
	QBrush brush(QColor(200, 200, 200));
	painter->setBrush(brush);
	painter->setPen(QPen(QColor(0, 0, 0)));
	painter->setRenderHint(QPainter::Antialiasing);

	if (!print) painter->drawRect(0, 0, c.x()*2+1, c.y()*2+1);
	if (plate_plot_n_triangles==0) return;
	mutex.lock();

	double const zs[]= {100.0, 75.0, 50.0, 25.0, 10.0, 5.0};
	dpi= dpi/25.4*zs[int(_zoom)]/100;
	if (_showMesh || _showForces)
	{
		if (_showMesh) painter->setPen(QPen(QColor(0,0,0))); else painter->setPen(QPen(QColor(0,0,0,0)));
		painter->setBrush(QBrush(QColor(0, 0, 0, 255)));
		double off= 0.0, fact= 1.0;
		if (_showForces)
		{
			double zmin= 1e300, zmax= -1e300; int mi, Mi;
			for (int i=0; i<plate_plot_n_points; i++)
			{
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
	QString parts;
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
				polar_to_euc (part_cg_radius[i],part_cg_angle [i] + isubpart * (double) degrees / part_quantity [i],&xs, &ys);
				painter->setPen(QPen(QColor(255,0,0)));
				painter->drawEllipse(QPoint(xs*dpi+c.x(), ys*dpi+c.y()), 10, 10);
			}
			double x[3], y[3], l[3];
			for (int j = 0; j < ncorners; j++) polar_to_euc (part_corner_radius [i] [j], part_corner_angle [i] [j], &x[j], &y[j]);
			for (int j=0; j<ncorners; j++) l[j]= sqrt((x[(j+1)%ncorners]-x[j])*(x[(j+1)%ncorners]-x[j])+(y[(j+1)%ncorners]-y[j])*(y[(j+1)%ncorners]-y[j]));
			if (i!=0) parts+= +"\n";
			if (part_type [i]==0) parts+= "triangle: cog_radius "+QString::number(part_cg_radius[i], 'f', 1)+" length1 "+QString::number(l[0], 'f', 1)+" length2 "+QString::number(l[1], 'f', 1)+" length3 "+QString::number(l[2], 'f', 1);
			else parts+= "bar: cog_radius "+QString::number(part_cg_radius[i], 'f', 1)+" length1 "+QString::number(l[0], 'f', 1);
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

	if (_cellType!=0) setParts(parts);
	else setParts("3 points at "+QString::number(support_radii[0], 'f', 1)+"mm radius");

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
