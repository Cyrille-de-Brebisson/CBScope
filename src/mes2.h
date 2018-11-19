#ifndef MES2_H
#define MES2_H

#include <stdlib.h>
#include <cassert>
class CMes2
{
    constexpr static double const shear= 0.8333333;
    static int const NDF= 3;  // Number of degree of freedom
    static int const NBAND= 90;
    struct TPTS { double x, y, forces[3]; char fix; TPTS(): x(0.0), y(0.0), fix(0) { forces[0]= forces[1]= forces[2]= 0.0; } };
    TPTS *_PTS;
    struct TELEM { int p1, p2, p3; double thick; TELEM(): p1(0), p2(0), p3(0), thick(0.0) {} };
    TELEM *_ELEM;
    double *_SK;
    double inline &SK(int i, int j)
    {
      assert(i>0 && i<=NP*3);
      assert(j>0 && j<=NBAND);
      return _SK[(i-1)*NBAND+j-1];
    }
    int NP;                         // Number of Points
    int NE;                         // Number of elements
    double inline &R1(int i) 
    { 
      assert(i>0 && i<=NP*3);
      return _PTS[(i-1)/3].forces[(i-1)%3];
    }
    int inline NOP(int i, int j) 
    { 
      assert(i>0 && i<=NE);
      if (j==1) return _ELEM[i-1].p1+1; else if (j==2) return _ELEM[i-1].p2+1; else if (j==3) return _ELEM[i-1].p3+1; Q_ASSERT("ERROR HERE 6"); 
    }
    void inline pnt(int i, char fix, double x, double y) { _PTS[i].fix= fix; _PTS[i].x= x; _PTS[i].y= y; _PTS[i].forces[0]= _PTS[i].forces[1]= _PTS[i].forces[2]= 0.0; }
    void inline mesh(int i, double h, int p1, int p2, int p3) { _ELEM[i].p1= p1; _ELEM[i].p2= p2; _ELEM[i].p3= p3; _ELEM[i].thick= h; }

    double young;
    double poisson;
    double density;

    void PLOAD();
    void FORMK();
    void SOLVE();

public:
    bool hasCalculated;
    CMes2(): _SK(nullptr), hasCalculated(false), NP(0), NE(0), _PTS(nullptr), _ELEM(nullptr), young(6700), poisson(0.2), density(2.32e-6) { }
    ~CMes2() { if (_SK!=nullptr) delete[] _SK; if (_PTS!=nullptr) delete[] _PTS; if (_ELEM!=nullptr) delete[] _ELEM; }
    TPTS inline *pnt(int i) { return _PTS+i; }
    TELEM inline *mesh(int i) { return _ELEM+i; }
    int inline getNbPoints() { return NP; }
    int inline getNbElmements() { return NE; }
    void setNbPoints(int nb) 
    { 
      if (_PTS!=nullptr) delete[] _PTS;
      _PTS= new TPTS[NP= nb];
      hasCalculated= false;
    }
    void setNbElmements(int nb) 
    { 
      if (_ELEM!=nullptr) delete[] _ELEM;
      _ELEM= new TELEM[NE= nb];
      hasCalculated= false;
    }
    void doIt()
    {
        // Input are; points: NP, CORD, NFIX, R1    elements: NE, NOP, THICK    consts: young, poisson, density
        _SK= new double[NP*NDF*NBAND];
        PLOAD(); // add pressure to R1
        FORMK(); // stiffness matrix calculation
        SOLVE(); // solve
        //    for(int I = 1; I<=NP; I++) printf(I, R1[I*3 - 2], R1[I*3 - 1], R1[I*3])  Z, THETA-X, THETA-Y for point N
        hasCalculated= true;
        delete[] _SK; _SK= nullptr;
    }

    void createMirror(double radius, double thickness, double young, double poisson, double roc, double density, int cellType, double *supports);
};

#endif // MES2_H
