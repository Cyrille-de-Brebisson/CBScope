#ifndef MES2_H
#define MES2_H

#include <stdlib.h>
#include <cassert>

static QString r2s2(double d)
{
  char b[30]= "                    ";
  char *s= b+20; 
  bool neg= d<0; d= ::abs(d);
  d*= 1e5; d= ::floor(d+0.4999999);
  for (int i=0; i<5;i++) { *--s= '0'+int(::fmod(d,10.0)); d= floor(d/10.0); }
  *--s= '.';
  *--s= '0'+int(::fmod(d,10.0)); d= floor(d/10.0);
  while (d>=1.0 && s>b+1) { *--s= '0'+int(::fmod(d,10.0)); d= floor(d/10.0); }
  if (neg) *--s= '-';
  return QString(b);
}
#define r2s(a) r2s2(a).toStdString().c_str()

class CMes2
{
    typedef double mesdouble;
    constexpr static mesdouble const shear= 0.8333333;
    static int const NDF= 3;  // Number of degree of freedom
    static int const NBAND= 90;
    struct TPTS { mesdouble x, y, forces[3]; char fix; TPTS(): x(0.0), y(0.0), fix(0) { forces[0]= forces[1]= forces[2]= 0.0; } };
    TPTS *_PTS;
    struct TELEM { int p1, p2, p3; mesdouble thick, meanDisp, press; TELEM(): p1(0), p2(0), p3(0), thick(0.0), meanDisp(0.0), press(0.0) {} };
    TELEM *_ELEM;
    mesdouble *_SK;
    mesdouble inline &SK(int i, int j)
    {
      assert(i>0 && i<=NP*3);
      assert(j>0 && j<=NBAND);
      return _SK[(i-1)*NBAND+j-1];
    }
    int NP;                         // Number of Points
    int NE;                         // Number of elements
    mesdouble inline &R1(int i) 
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
    void inline mesh(int i, double h, int p3, int p2, int p1, double press) { _ELEM[i].p1= p1; _ELEM[i].p2= p2; _ELEM[i].p3= p3; _ELEM[i].thick= h; _ELEM[i].press= press; }

    mesdouble young;
    mesdouble poisson;
    mesdouble density;

    void PLOAD();
    void FORMK();
    void SOLVE();
    void MAINV1(mesdouble *_A, int N1);
    void TRI_INT(mesdouble &AREA, mesdouble &X, mesdouble &Y, mesdouble &XY, double X1, double Y1, double X2, double Y2, double X3, double Y3);

public:
    bool hasCalculated;
    double meshMeanDispH, meshMeanDispL; // output, min max of mesh mean displacement
    CMes2(): _SK(nullptr), hasCalculated(false), meshMeanDispH(0.0), meshMeanDispL(0.0), NP(0), NE(0), _PTS(nullptr), _ELEM(nullptr), young(6700), poisson(0.2), density(2.32e-6) { }
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
        meshMeanDispH= 0.0; meshMeanDispL= 0.0;
        // Input are; points: NP, _PTS    elements: NE, _ELEM    consts: young, poisson, density
        _SK= new mesdouble[NP*NDF*NBAND];
        PLOAD(); // add pressure to R1
        FORMK(); // stiffness matrix calculation
        SOLVE(); // solve
//        qDebug() << " ============================================";
//        qDebug() << "  GRID NO.      W        THETA-X     THETA-Y ";
//        qDebug() << " ============================================";
//        for(int I = 0; I<NP; I++) qDebug()  << I+1 << pnt(I)->forces[0] << pnt(I)->forces[1] << pnt(I)->forces[2];
        meshMeanDispH= -1e300; meshMeanDispL= 1e300;
//        qDebug() << "=====================================================";
//        qDebug() << "  ELEMENT NO.   X           Y           AREA        W";
//        qDebug() << "=====================================================";

        for(int N = 0; N<NE; N++)
        {
            // for each element, calculate center of gravity (X/YCG) , AREA and WEIGHT
            //double X1 = pnt(mesh(N)->p1)->x;
            //double Y1 = pnt(mesh(N)->p1)->y;
            //double X2 = pnt(mesh(N)->p2)->x;
            //double Y2 = pnt(mesh(N)->p2)->y;
            //double X3 = pnt(mesh(N)->p3)->x;
            //double Y3 = pnt(mesh(N)->p3)->y;
            double WEIGHT = (pnt(mesh(N)->p1)->forces[0]+pnt(mesh(N)->p2)->forces[0]+pnt(mesh(N)->p3)->forces[0])/3.0;
            mesh(N)->meanDisp= WEIGHT;
            if (WEIGHT>meshMeanDispH) meshMeanDispH= WEIGHT;
            if (WEIGHT<meshMeanDispL) meshMeanDispL= WEIGHT;
//            double XCG = (X1 + X2 + X3)/3.0;
//            double YCG = (Y1 + Y2 + Y3)/3.0;
//            double X1A = X1 - XCG;
//            double Y1A = Y1 - YCG;
//            double X2A = X2 - XCG;
//            double Y2A = Y2 - YCG;
//            double X3A = X3 - XCG;
//            double Y3A = Y3 - YCG;
//            double AREA = 0.5*(X2A*Y3A + X3A*Y1A + X1A*Y2A - X1A*Y3A - X2A*Y1A - X3A*Y2A);
//            qDebug() << N+1 << XCG << YCG << AREA << WEIGHT;
        }
        qDebug() << meshMeanDispL << meshMeanDispH;
        hasCalculated= true;
        delete[] _SK; _SK= nullptr;
    }

    void createMirror(double radius, double thickness, double young, double poisson, double roc, double density, int cellType, double *supports);
};

#endif // MES2_H
