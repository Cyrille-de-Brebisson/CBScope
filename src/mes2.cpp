#include <QDebug>
#include "mes2.h"

static double const M_PI= 3.14159265358979323846264338327;
void MAINV1(double *_A, int N1)
{
    int INDEX[100+1];
#define A(a,b) _A[((a)-1)*N1+(b)]
    int PIVR;
//    double DET = 1.0;
    int N = N1;
    for(int K=1; K<=N; K++)
    {
        double PIV = 0.0;
        PIVR = 0;
        for(int I=K; I<=N; I++)
        {
            double W = A(I, K); if (W<0) W= -W;
            if (PIV >= W) continue;
            PIV = W;
            PIVR = I;
        }
        PIV = A(PIVR, K);
        INDEX[K] = PIVR;
        if (PIVR != K)
        {
            for(int J=1; J<=N; J++)
            {
                double W = A(PIVR, J);
                A(PIVR, J) = A(K, J);
                A(K, J) = W;
            }
//            DET = -DET;
        }
//        DET = DET*PIV;
        A(K, K) = 1.0;
        for(int J=1; J<=N; J++) A(K, J) = A(K, J)/PIV;
        for(int I=1; I<=N; I++)
        {
            if (I == K) continue;
            double W = A(I, K);
            A(I, K) = 0.0;
            for(int J=1; J<=N; J++) A(I, J) = A(I, J) - A(K, J)*W;
        }

    }
    for(int K = 1; K<=N - 1; K++)
    {
        int K1 = N - K;
        int INDEXK = INDEX[K1];
        if (INDEXK == K1) continue;
        for(int I=1; I<=N; I++)
        {
            double W = A(I, INDEXK);
            A(I, INDEXK) = A(I, K1);
            A(I, K1) = W;
        }
    }
#undef A
};

// ================================
//  CALCULATE INTEGRAL IN TRIANGLE
// ================================
static void TRI_INT(double &AREA, double &X, double &Y, double &XY, double X1, double Y1, double X2, double Y2, double X3, double Y3)
{
    double XCG = (X1 + X2 + X3)/3.0;
    double YCG = (Y1 + Y2 + Y3)/3.0;
    double X1A = X1 - XCG;
    double Y1A = Y1 - YCG;
    double X2A = X2 - XCG;
    double Y2A = Y2 - YCG;
    double X3A = X3 - XCG;
    double Y3A = Y3 - YCG;
    AREA = 0.5*(X2A*Y3A + X3A*Y1A + X1A*Y2A - X1A*Y3A - X2A*Y1A - X3A*Y2A);
    X = AREA/12.0*(X1A*X1A + X2A*X2A + X3A*X3A);
    Y = AREA/12.0*(Y1A*Y1A + Y2A*Y2A + Y3A*Y3A);
    XY = AREA/12.0*(X1A*Y1A + X2A*Y2A + X3A*Y3A);
    return;
};
// =============================
//  ADD LOAD VECTOR OF PRESSURE
// =============================
void CMes2::PLOAD()
{
    double ainv[9*9];
#define AINV(a,b) ainv[((a)-1)*9+(b)-1]
    double XM[9+1], FP[9+1];
    // ----------------------------------- READ ELEMENT NUMBER AND PRESSURE
    double PTOTAL = 0.0;
    for (int NPRESS= 1; NPRESS<=NE; NPRESS++)
    {
        // for each NPRES point with pressure do:
        double PRESS=density*mesh(NPRESS-1)->thick; //*density*thickness_here; // pressure per unit of surface

        double X1 = pnt(mesh(NPRESS-1)->p1)->x;
        double Y1 = pnt(mesh(NPRESS-1)->p1)->y;
        double X2 = pnt(mesh(NPRESS-1)->p2)->x;
        double Y2 = pnt(mesh(NPRESS-1)->p2)->y;
        double X3 = pnt(mesh(NPRESS-1)->p3)->x;
        double Y3 = pnt(mesh(NPRESS-1)->p3)->y;
        double AREA, X, Y, XY;
        TRI_INT(AREA, X, Y, XY, X1, Y1, X2, Y2, X3, Y3);
        PTOTAL = PTOTAL + PRESS*AREA;
        // =======================
        //  INVERSE MATRIX OF [A]
        // =======================
        double XCG = (X1 + X2 + X3)/3.0;
        double YCG = (Y1 + Y2 + Y3)/3.0;
        double X1A = X1 - XCG;
        double Y1A = Y1 - YCG;
        double X2A = X2 - XCG;
        double Y2A = Y2 - YCG;
        double X3A = X3 - XCG;
        double Y3A = Y3 - YCG;
        AINV(1, 1) = 1.0; AINV(1, 2) = X1A; AINV(1, 3) = Y1A; AINV(1, 4) = 0.0; AINV(1, 5) = -0.5*X1A*X1A; AINV(1, 6) = -0.5*X1A*Y1A; AINV(1, 7) = 0.0; AINV(1, 8) = -0.5*X1A*Y1A; AINV(1, 9) = -0.5*Y1A*Y1A;
        AINV(2, 1) = 1.0; AINV(2, 2) = X2A; AINV(2, 3) = Y2A; AINV(2, 4) = 0.0; AINV(2, 5) = -0.5*X2A*X2A; AINV(2, 6) = -0.5*X2A*Y2A; AINV(2, 7) = 0.0; AINV(2, 8) = -0.5*X2A*Y2A; AINV(2, 9) = -0.5*Y2A*Y2A;
        AINV(3, 1) = 1.0; AINV(3, 2) = X3A; AINV(3, 3) = Y3A; AINV(3, 4) = 0.0; AINV(3, 5) = -0.5*X3A*X3A; AINV(3, 6) = -0.5*X3A*Y3A; AINV(3, 7) = 0.0; AINV(3, 8) = -0.5*X3A*Y3A; AINV(3, 9) = -0.5*Y3A*Y3A;
        AINV(4, 1) = 0.0; AINV(4, 2) = 0.0; AINV(4, 3) = 0.0; AINV(4, 4) = 1.0; AINV(4, 5) = X1A; AINV(4, 6) = Y1A; AINV(4, 7) = 0.0; AINV(4, 8) = 0.0; AINV(4, 9) = 0.0;
        AINV(5, 1) = 0.0; AINV(5, 2) = 0.0; AINV(5, 3) = 0.0; AINV(5, 4) = 1.0; AINV(5, 5) = X2A; AINV(5, 6) = Y2A; AINV(5, 7) = 0.0; AINV(5, 8) = 0.0; AINV(5, 9) = 0.0;
        AINV(6, 1) = 0.0; AINV(6, 2) = 0.0; AINV(6, 3) = 0.0; AINV(6, 4) = 1.0; AINV(6, 5) = X3A; AINV(6, 6) = Y3A; AINV(6, 7) = 0.0; AINV(6, 8) = 0.0; AINV(6, 9) = 0.0;
        AINV(7, 1) = 0.0; AINV(7, 2) = 0.0; AINV(7, 3) = 0.0; AINV(7, 4) = 0.0; AINV(7, 5) = 0.0; AINV(7, 6) = 0.0; AINV(7, 7) = 1.0; AINV(7, 8) = X1A; AINV(7, 9) = Y1A;
        AINV(8, 1) = 0.0; AINV(8, 2) = 0.0; AINV(8, 3) = 0.0; AINV(8, 4) = 0.0; AINV(8, 5) = 0.0; AINV(8, 6) = 0.0; AINV(8, 7) = 1.0; AINV(8, 8) = X2A; AINV(8, 9) = Y2A;
        AINV(9, 1) = 0.0; AINV(9, 2) = 0.0; AINV(9, 3) = 0.0; AINV(9, 4) = 0.0; AINV(9, 5) = 0.0; AINV(9, 6) = 0.0; AINV(9, 7) = 1.0; AINV(9, 8) = X3A; AINV(9, 9) = Y3A;
        MAINV1(ainv, 9);
        XM[1] = AREA*PRESS;
        XM[2] = 0.0;
        XM[3] = 0.0;
        XM[4] = 0.0;
        XM[5] = -0.5*X*PRESS;
        XM[6] = -0.5*XY*PRESS;
        XM[7] = 0.0;
        XM[8] = -0.5*XY*PRESS;
        XM[9] = -0.5*Y*PRESS;
        for(int I=1; I<=9; I++) FP[I] = 0.0;
        for(int I=1; I<=9; I++)
            for(int J=1; J<=9; J++)
                FP[I] = FP[I] + AINV(J, I)*XM[J];
        pnt(mesh(NPRESS-1)->p1)->forces[0]+= FP[1];
        pnt(mesh(NPRESS-1)->p1)->forces[1]+= FP[2];
        pnt(mesh(NPRESS-1)->p1)->forces[2]+= FP[3];
        pnt(mesh(NPRESS-1)->p2)->forces[0]+= FP[4];
        pnt(mesh(NPRESS-1)->p2)->forces[1]+= FP[5];
        pnt(mesh(NPRESS-1)->p2)->forces[2]+= FP[6];
        pnt(mesh(NPRESS-1)->p3)->forces[0]+= FP[7];
        pnt(mesh(NPRESS-1)->p3)->forces[1]+= FP[8];
        pnt(mesh(NPRESS-1)->p3)->forces[2]+= FP[9];
    }
#undef AINV
};

// =======================
//  FORM STIFFNESS MATRIX
// =======================
void CMes2::FORMK()
{
    double bdb[9*9];
#define BDB(a,b) bdb[((a)-1)*9+(b)-1]
    int IXX[9+1]= { 0, 1, 4, 7, 2, 5, 8, 3, 6, 9 };
    double ainv[9*9];
#define AINV(a,b) ainv[((a)-1)*9+(b)-1]
    double _DUMMY[9*9];
#define DUMMY(a,b) _DUMMY[((a)-1)*9+(b)-1]
    double _XK1[9*9];
#define XK1(a,b) _XK1[((a)-1)*9+(b)-1]
    double _XK[9*9];
#define XK(a,b) _XK[((a)-1)*9+(b)-1]
    // --------------------------- ZERO STIFFNESS MATRIX
    for(int N = 1; N<=NP*NDF; N++) for(int M=1; M<=NBAND; M++) SK(N, M) = 0.0;
    // --------------------------- SCAN ELEMENTS
    for(int N = 1; N<=NE; N++)
    {
        // --------------------------- ELEMENT STIFFNESS MATRIX
        double X1 = pnt(mesh(N-1)->p1)->x;
        double Y1 = pnt(mesh(N-1)->p1)->y;
        double X2 = pnt(mesh(N-1)->p2)->x;
        double Y2 = pnt(mesh(N-1)->p2)->y;
        double X3 = pnt(mesh(N-1)->p3)->x;
        double Y3 = pnt(mesh(N-1)->p3)->y;
        double TH = mesh(N-1)->thick;
        double E = young;
        double V = poisson;
        double SH = shear;
        //     TRIANGULAR ELEMENT BENDING WITH TRANSVERSE SHEAR
        double G = 0.5*E/(1. + V);
        double D = E*TH*TH*TH/12.0/(1. - V*V); // was E*TH**3. Precedence?
        double AREA, X, Y, XY;
        TRI_INT(AREA, X, Y, XY, X1, Y1, X2, Y2, X3, Y3);
        //  INVERSE MATRIX OF [A]
        double XCG = (X1 + X2 + X3)/3.0;
        double YCG = (Y1 + Y2 + Y3)/3.0;
        double X1A = X1 - XCG;
        double Y1A = Y1 - YCG;
        double X2A = X2 - XCG;
        double Y2A = Y2 - YCG;
        double X3A = X3 - XCG;
        double Y3A = Y3 - YCG;
        AINV(1, 1) = 1.0; AINV(1, 2) = X1A; AINV(1, 3) = Y1A; AINV(1, 4) = 0.0; AINV(1, 5) = -0.5*X1A*X1A; AINV(1, 6) = -0.5*X1A*Y1A; AINV(1, 7) = 0.0; AINV(1, 8) = -0.5*X1A*Y1A; AINV(1, 9) = -0.5*Y1A*Y1A;
        AINV(2, 1) = 1.0; AINV(2, 2) = X2A; AINV(2, 3) = Y2A; AINV(2, 4) = 0.0; AINV(2, 5) = -0.5*X2A*X2A; AINV(2, 6) = -0.5*X2A*Y2A; AINV(2, 7) = 0.0; AINV(2, 8) = -0.5*X2A*Y2A; AINV(2, 9) = -0.5*Y2A*Y2A;
        AINV(3, 1) = 1.0; AINV(3, 2) = X3A; AINV(3, 3) = Y3A; AINV(3, 4) = 0.0; AINV(3, 5) = -0.5*X3A*X3A; AINV(3, 6) = -0.5*X3A*Y3A; AINV(3, 7) = 0.0; AINV(3, 8) = -0.5*X3A*Y3A; AINV(3, 9) = -0.5*Y3A*Y3A;
        AINV(4, 1) = 0.0; AINV(4, 2) = 0.0; AINV(4, 3) = 0.0; AINV(4, 4) = 1.0; AINV(4, 5) = X1A; AINV(4, 6) = Y1A; AINV(4, 7) = 0.0; AINV(4, 8) = 0.0; AINV(4, 9) = 0.0;
        AINV(5, 1) = 0.0; AINV(5, 2) = 0.0; AINV(5, 3) = 0.0; AINV(5, 4) = 1.0; AINV(5, 5) = X2A; AINV(5, 6) = Y2A; AINV(5, 7) = 0.0; AINV(5, 8) = 0.0; AINV(5, 9) = 0.0;
        AINV(6, 1) = 0.0; AINV(6, 2) = 0.0; AINV(6, 3) = 0.0; AINV(6, 4) = 1.0; AINV(6, 5) = X3A; AINV(6, 6) = Y3A; AINV(6, 7) = 0.0; AINV(6, 8) = 0.0; AINV(6, 9) = 0.0;
        AINV(7, 1) = 0.0; AINV(7, 2) = 0.0; AINV(7, 3) = 0.0; AINV(7, 4) = 0.0; AINV(7, 5) = 0.0; AINV(7, 6) = 0.0; AINV(7, 7) = 1.0; AINV(7, 8) = X1A; AINV(7, 9) = Y1A;
        AINV(8, 1) = 0.0; AINV(8, 2) = 0.0; AINV(8, 3) = 0.0; AINV(8, 4) = 0.0; AINV(8, 5) = 0.0; AINV(8, 6) = 0.0; AINV(8, 7) = 1.0; AINV(8, 8) = X2A; AINV(8, 9) = Y2A;
        AINV(9, 1) = 0.0; AINV(9, 2) = 0.0; AINV(9, 3) = 0.0; AINV(9, 4) = 0.0; AINV(9, 5) = 0.0; AINV(9, 6) = 0.0; AINV(9, 7) = 1.0; AINV(9, 8) = X3A; AINV(9, 9) = Y3A;
        MAINV1(ainv, 9);
        for(int IX=1; IX<=9; IX++)
            for(int JX=1; JX<=9; JX++)
            {
                BDB(IX, JX) = 0.0;
                XK1(IX, JX) = 0.0;
                XK(IX, JX) = 0.0;
                DUMMY(IX, JX) = 0.0;
            }
        BDB(2, 2) = SH*TH*G*AREA;
        BDB(2, 4) = BDB(2, 2);
        BDB(3, 3) = BDB(2, 2);
        BDB(3, 7) = BDB(2, 2);
        BDB(4, 2) = BDB(2, 2);
        BDB(4, 4) = BDB(2, 2);
        BDB(5, 5) = D*AREA;
        BDB(5, 9) = V*D*AREA;
        BDB(6, 6) = 0.5*(1. - V)*D*AREA + 0.25*SH*TH*G*(X + Y);
        BDB(6, 8) = 0.5*(1. - V)*D*AREA - 0.25*SH*TH*G*(X + Y);
        BDB(7, 3) = BDB(2, 2);
        BDB(7, 7) = BDB(2, 2);
        BDB(8, 6) = 0.5*(1. - V)*D*AREA - 0.25*SH*TH*G*(X + Y);
        BDB(8, 8) = 0.5*(1. - V)*D*AREA + 0.25*SH*TH*G*(X + Y);
        BDB(9, 5) = V*D*AREA;
        BDB(9, 9) = D*AREA;
        for(int IX=1; IX<=9; IX++) // Dummy=BDB*AINV ?
            for(int JX=1; JX<=9; JX++)
                for(int KX=1; KX<=9; KX++)
                    DUMMY(IX, JX)+= BDB(IX, KX)*AINV(KX, JX);
        for(int IX=1; IX<=9; IX++) // XK1=AINV*Dummy? = AINV * (BDB*AINV)
            for(int JX=1; JX<=9; JX++)
                for(int KX=1; KX<=9; KX++)
                    XK1(IX, JX)+= AINV(KX, IX)*DUMMY(KX, JX);
        for(int IX=1; IX<=9; IX++) // feels like a transpose....
            for(int JX=1; JX<=9; JX++)
                XK(IX, JX) = XK1(IXX[IX], IXX[JX]);
        // --------------------------- RETURNS XK(9,9) AS STIFFNESS MATRIX
        // --------------------------- STORE XK IN SK
        // --------------------------- FIRST, ROWS
        for(int JJ = 1; JJ<=NDF; JJ++) // for each point of element
        {
            int NROWB = (NOP(N, JJ) - 1)*NDF;
            for(int J = 1; J<=NDF; J++) // 
            {
                NROWB = NROWB + 1;
                int I = (JJ - 1)*NDF + J;
                // --------------------------- THEN, COLUMNS
                for(int KK = 1; KK<=NDF; KK++)
                {
                    int NCOLB = (NOP(N, KK) - 1)*NDF;
                    for(int K = 1; K<=NDF; K++)
                    {
                        int L = (KK - 1)*NDF + K;
                        int NCOL = NCOLB + K + 1 - NROWB;
                        if (NCOL>0 && NCOL<=NBAND) SK(NROWB, NCOL)+= XK(I, L); // only store in band!
                    }
                }
            }
        }
    }
    // --------------------------- INSERT BOUNDARY CONDITION
    for(int I = 1; I<=NP; I++)
    {
        int NROWB = (I - 1)*NDF;
        int ICON = pnt(I-1)->fix;
        for(int M = 1; M<=NDF; M++)
        {
            if ((ICON&4)!=0)
            {
                SK(NROWB+M, 1)= 1.0;
                R1(NROWB+M)= 0.0;
                for(int J=2; J<=NBAND; J++)
                {
                    SK(NROWB+M, J) = 0.0;
                    int NR = NROWB+M + 1 - J;
                    if (NR>0) SK(NR, J) = 0.0;
                }
            }
            ICON<<=1;
        }
    }
#undef BDB
#undef AINV
#undef DUMMY
#undef XK1
#undef XK
};

// =========================================
//  SOLVE EQUATION USING BAND MATRIX METHOD
// =========================================
void CMes2::SOLVE()
{
    // ------------------------------ REDUCE MATRIX
    for(int N = 1; N<=NP*NDF; N++)
    {
        int I = N;
        for(int L=2; L<=NBAND; L++)
        {
            I = I + 1;
            if (SK(N, L)==0.0) continue;
            double C = SK(N, L)/SK(N, 1);
            int J = 0;
            for(int K=L; K<=NBAND; K++)
            {
                J = J + 1;
                if (SK(N, K)==0.0) continue;
                SK(I, J) = SK(I, J) - C*SK(N, K);
            }
            SK(N, L) = C;
            // ------------------------------ AND LOAD VECTOR FOR EACH EQUATION
            R1(I) = R1(I) - C*R1(N);
        }
        R1(N) = R1(N)/SK(N, 1);
    }
    // ------------------------------ BACK-SUBSTITUTION
    for (int N= NP*NDF; --N>0;)
    {
        int L = N;
        for(int K=2; K<=NBAND; K++)
        {
            L = L + 1;
            if (SK(N, K)==0.0) continue;
            R1(N) -= SK(N, K)*R1(L);
            // ------------------------------ R1(N) <= DISPLACEMENTS
        }
    }
};

//void mainOutput()
//{
//    for(int I = 1; I<=NP; I++) printf(I, R1[I*3 - 2], R1[I*3 - 1], R1[I*3])  Z, THETA-X, THETA-Y for point N
//    for(int N = 1; N<=NE; N++)
//    {
//        // for each element, calculate center of gravity (X/YCG) , AREA and WEIGHT
//        double X1 = CORD(NOP(N, 1), 1);
//        double Y1 = CORD(NOP(N, 1), 2);
//        double X2 = CORD(NOP(N, 2), 1);
//        double Y2 = CORD(NOP(N, 2), 2);
//        double X3 = CORD(NOP(N, 3), 1);
//        double Y3 = CORD(NOP(N, 3), 2);
//        double WEIGHT = (R1[NOP(N, 1)*3 - 2] + R1[NOP(N, 2)*3 - 2] + R1[NOP(N, 3)*3 - 2])/3.;
//        double XCG = (X1 + X2 + X3)/3.0;
//        double YCG = (Y1 + Y2 + Y3)/3.0;
//        double X1A = X1 - XCG;
//        double Y1A = Y1 - YCG;
//        double X2A = X2 - XCG;
//        double Y2A = Y2 - YCG;
//        double X3A = X3 - XCG;
//        double Y3A = Y3 - YCG;
//        double AREA = 0.5*(X2A*Y3A + X3A*Y1A + X1A*Y2A - X1A*Y3A - X2A*Y1A - X3A*Y2A);
//    }
//};

    static double inline sagita(double radiusSphere, double radiusDisc) { return radiusSphere-sqrt(radiusSphere*radiusSphere-radiusDisc*radiusDisc/4.0); } // calculate the sagita of disk
    static double thicnknessAt(double radius, double thickness, double roc, double r) { return (thickness-sagita(roc, radius-r)); } // Thickness of the mirror at a given distance from center in metters!

void CMes2::createMirror(double radius, double thickness, double young, double poisson, double roc, double density, int _cellType, double *supports)
{ 
    this->young= young; this->poisson= poisson; this->density= density;
    // cellType NbPoints NbSupportRings NbAnglularSegments NbMeshRings supportsPerRing
    // 0        3        1              3                  7           3 @ n*120°
    // 1        6        1              6                  11          6 @ n*60°
    // 2        9        2              9                  11          3,6 @ n*120° n*60°+30°
    // 3        18       2              18                 19          6,12 @ n*60°n*30°+15°
    // 4        27       3              24                 19          6,9,12 @ n*60° n*40°+10° n*30°+15°
    // 5        36       3              24                 23          6,12,18 @ n*60° n*30° n*20°
    struct { int nbSupportRing, nbMeshRings; struct { int nbSupports, astep, offset; } r[3]; }
        const cellDefs[6]={
            {1, 7,  {{3, 120, 0 }} },
            {1, 11, {{6, 60, 0 }} },
            {2, 11, {{3, 120, 0 }, {6, 60, 30 }} },
            {2, 19, {{6, 60, 0 }, {12, 30, 15 }} },
            {3, 19, {{6, 60, 0 }, {9, 40, 10 }, {12, 30, 15}} },
            {3, 23, {{6, 60, 0 }, {12, 30, 0 }, {18, 20, 0}} }
        };
    // Support ring radiis
    int nbSupRings= cellDefs[_cellType].nbSupportRing;
    double supRingRad[3]; 
    if (supports==nullptr)
    {
      for (int i=0; i<nbSupRings; i++) supRingRad[i]= radius/(nbSupRings+1.0)*(i+1);
      supports= supRingRad;
    }

    // Calc nb of points which will depend on cell type...
    int nbRings= cellDefs[_cellType].nbMeshRings;
    int const pointsPerRing[24]= {1, 6, 12, 24, 24, 48, 48, 48, 48, 96, 96, 96, 96, 96, 96, 96, 96, 192, 192, 192, 192, 192, 192, 192 };
    int nbPoints= 0; for (int i=0; i<=nbRings; i++) nbPoints+= pointsPerRing[i];
    setNbPoints(nbPoints);
    // Calc nb of elements...
    int nbElements= 6;
    for (int i=2; i<=nbRings; i++) 
      if (pointsPerRing[i-1]==pointsPerRing[i]) nbElements+= pointsPerRing[i]/2*4;
      else nbElements+= pointsPerRing[i]/2*3;
    setNbElmements(nbElements);

    // Calc the radiis of the mesh rings
    double meshRingRads[24]; for (int i=0; i<=nbRings; i++) meshRingRads[i]= i*radius/nbRings; // radiis of the various mesh rings

    int ptsCount= 0;
    int elCount= 0;
    pnt(ptsCount++, 0, 0.0, 0.0);
    // Nb triangles= if count change from previous to new row: 3*(nb_old_points-1), else 2*(nb_old_points-1)

    double h= thicnknessAt(radius, thickness, roc, (meshRingRads[0]+meshRingRads[1])/2);
    int firstPointLastRing= ptsCount;
    for (int j=0; j<6; j++) pnt(ptsCount++, 0, meshRingRads[1]*cos(j*M_PI/3.0), -meshRingRads[1]*sin(j*M_PI/3.0));
    mesh(elCount++, h, 0, 1, 2);
    mesh(elCount++, h, 0, 2, 3);
    mesh(elCount++, h, 0, 3, 4);
    mesh(elCount++, h, 0, 4, 5);
    mesh(elCount++, h, 0, 5, 6);
    mesh(elCount++, h, 0, 6, 1);

    int supportRing= 0;

    for (int i=2; i<=nbRings; i++)
    {
        int saveFirstPointLastRing= firstPointLastRing;
        int firstPointThisRing= ptsCount;
        double a= 0.0, astep= M_PI*2.0/pointsPerRing[i];                     // Angle counter and stepps...
        double h= thicnknessAt(radius, thickness, roc, (meshRingRads[i-1]+meshRingRads[i])/2);
        if (pointsPerRing[i]==pointsPerRing[i-1])
            for (int j=pointsPerRing[i]/2; --j>=0; ) // for each 2 points per point of the ring
            {
                pnt(ptsCount++, 0, meshRingRads[i]*cos(a), -meshRingRads[i]*sin(a)); a+= astep;
                pnt(ptsCount++, 0, meshRingRads[i]*cos(a), -meshRingRads[i]*sin(a)); a+= astep;
                mesh(elCount++, h, firstPointLastRing, ptsCount-2, ptsCount-1);
                mesh(elCount++, h, firstPointLastRing, ptsCount-1, firstPointLastRing+1); firstPointLastRing++;
                mesh(elCount++, h, firstPointLastRing, ptsCount-1, j!=0 ? firstPointLastRing+1 : saveFirstPointLastRing);
                mesh(elCount++, h, j!=0 ? firstPointLastRing+1 : saveFirstPointLastRing, ptsCount-1, j!=0 ? ptsCount : firstPointThisRing); firstPointLastRing++;
            }
        else
            for (int j=pointsPerRing[i]/2; --j>=0;) // for each point of the ring
            {
                pnt(ptsCount++, 0, meshRingRads[i]*cos(a), -meshRingRads[i]*sin(a)); a+= astep;
                pnt(ptsCount++, 0, meshRingRads[i]*cos(a), -meshRingRads[i]*sin(a)); a+= astep;
                mesh(elCount++, h, firstPointLastRing, ptsCount-2, ptsCount-1);
                mesh(elCount++, h, firstPointLastRing, ptsCount-1, j!=0 ? firstPointLastRing+1 : saveFirstPointLastRing);
                mesh(elCount++, h, j!=0 ? firstPointLastRing+1 : saveFirstPointLastRing, ptsCount-1, j!=0 ? ptsCount : firstPointThisRing); firstPointLastRing++;
            }
        // in the +/-20% zone around this ring. put supports directly under the points
        // else put support on 2 points
        double d=(meshRingRads[i]-meshRingRads[i-1])*0.2;
        if (supportRing<cellDefs[_cellType].nbSupportRing && supports[supportRing]>=meshRingRads[i-1]-d && supports[supportRing]<meshRingRads[i])
        {
            double degPerPoints= 360.0/pointsPerRing[i-1];
            for (int j=0; j<cellDefs[_cellType].r[supportRing].nbSupports; j++)
              pnt(saveFirstPointLastRing+int((cellDefs[_cellType].r[supportRing].astep*j+cellDefs[_cellType].r[supportRing].offset)/degPerPoints))->fix= 4;
            if (supports[supportRing]>meshRingRads[i-1]+d)
            {
              double degPerPoints= 360.0/pointsPerRing[i];
              for (int j=0; j<cellDefs[_cellType].r[supportRing].nbSupports; j++)
                pnt(firstPointThisRing+int((cellDefs[_cellType].r[supportRing].astep*j+cellDefs[_cellType].r[supportRing].offset)/degPerPoints))->fix= 4;
            }
            supportRing++;
        }
        firstPointLastRing= firstPointThisRing;
    }
    qDebug() << "nbEl nbPts " << getNbElmements() << getNbPoints();
}
