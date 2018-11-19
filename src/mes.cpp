#include "mes.h"
#include <math.h>
#include <QDebug>

#define k(i, j) K[i*dof_no+j]

void CMes::populate_equation()
{
    // init F, K
    for (unsigned int i=0; i<dof_no*dof_no; i++) K[i]= 0.0;
    for (unsigned int i=0; i<dof_no; i++) F[i]= 0.0;
    for (unsigned int i = 0; i < point_no; i++)
    {
        const Bool3D& fix = fix_list[i];
        Point3D& force = force_list[i];
        unsigned int px = 3 * i + 0;
        unsigned int py = 3 * i + 1;
        unsigned int pz = 3 * i + 2;

        // reactions in K (in place of fixed displacement)
        if(fix.x) k(px, px) = -1;
        else F[px] = force.x;

        if(fix.y) k(py, py) = -1;
        else F[py] = force.y;

        if(fix.z) k(pz, pz) = -1;
        else F[pz] = force.z;
    }

    // compose K - stiffness matrix
    for (unsigned int i = 0; i < element_no; i++)
    {
        const double EA = young; // Young * Area
        const Element& element = element_list[i];
        const Point3D& point1 = point_list[element.p1];
        const Point3D& point2 = point_list[element.p2];
        const Bool3D& fix1 = fix_list[element.p1];
        const Bool3D& fix2 = fix_list[element.p2];
        unsigned int p1x = 3 * element.p1 + 0;
        unsigned int p1y = 3 * element.p1 + 1;
        unsigned int p1z = 3 * element.p1 + 2;
        unsigned int p2x = 3 * element.p2 + 0;
        unsigned int p2y = 3 * element.p2 + 1;
        unsigned int p2z = 3 * element.p2 + 2;
        double dx = point2.x - point1.x;
        double dy = point2.y - point1.y;
        double dz = point2.z - point1.z;
        double l = sqrt(dx * dx + dy * dy + dz * dz);
        double cx = dx / l;
        double cy = dy / l;
        double cz = dz / l;
        double cxxEAl = cx * cx * EA / l;
        double cyyEAl = cy * cy * EA / l;
        double czzEAl = cz * cz * EA / l;
        double cxyEAl = cx * cy * EA / l;
        double cxzEAl = cx * cz * EA / l;
        double cyzEAl = cy * cz * EA / l;

        if(!fix1.x)
        {
            k(p1x, p1x) += cxxEAl;
            k(p1y, p1x) += cxyEAl;
            k(p1z, p1x) += cxzEAl;
            k(p2x, p1x) -= cxxEAl;
            k(p2y, p1x) -= cxyEAl;
            k(p2z, p1x) -= cxzEAl;
        }
        if(!fix1.y)
        {
            k(p1x, p1y) += cxyEAl;
            k(p1y, p1y) += cyyEAl;
            k(p1z, p1y) += cyzEAl;
            k(p2x, p1y) -= cxyEAl;
            k(p2y, p1y) -= cyyEAl;
            k(p2z, p1y) -= cyzEAl;
        }
        if(!fix1.z)
        {
            k(p1x, p1z) += cxzEAl;
            k(p1y, p1z) += cyzEAl;
            k(p1z, p1z) += czzEAl;
            k(p2x, p1z) -= cxzEAl;
            k(p2y, p1z) -= cyzEAl;
            k(p2z, p1z) -= czzEAl;
        }
        if(!fix2.x)
        {
            k(p1x, p2x) -= cxxEAl;
            k(p1y, p2x) -= cxyEAl;
            k(p1z, p2x) -= cxzEAl;
            k(p2x, p2x) += cxxEAl;
            k(p2y, p2x) += cxyEAl;
            k(p2z, p2x) += cxzEAl;
        }
        if(!fix2.y)
        {
            k(p1x, p2y) -= cxyEAl;
            k(p1y, p2y) -= cyyEAl;
            k(p1z, p2y) -= cyzEAl;
            k(p2x, p2y) += cxyEAl;
            k(p2y, p2y) += cyyEAl;
            k(p2z, p2y) += cyzEAl;
        }
        if(!fix2.z)
        {
            k(p1x, p2z) -= cxzEAl;
            k(p1y, p2z) -= cyzEAl;
            k(p1z, p2z) -= czzEAl;
            k(p2x, p2z) += cxzEAl;
            k(p2y, p2z) += cyzEAl;
            k(p2z, p2z) += czzEAl;
        }
    }
}

// solving K * dP = F by LU method
void CMes::calculate_equation()
{
//    qDebug() << "const int point_no =" << point_no <<";";
//    qDebug() << "const int element_no = "<< element_no << ";";
//    qDebug() << "const Point3D point_list[point_no] = {";
//    for (int i=0; i<point_no; i++)
//        qDebug()<< ("{" + QString::number(point_list[i].x)+", "+QString::number(point_list[i].y)+", "+QString::number(point_list[i].z)+"}, ").toStdString().c_str();
//    qDebug() << "};";
//    qDebug() << "Point3D force_list[point_no] = {";
//    for (int i=0; i<point_no; i++)
//        qDebug()<< ("{" + QString::number(force_list[i].x)+", "+QString::number(force_list[i].y)+", "+QString::number(force_list[i].z)+"}, ").toStdString().c_str();
//    qDebug() << "};";
//    qDebug() << "const Element element_list[element_no] = {";
//    for (int i=0; i<element_no; i++)
//        qDebug()<< ("{" + QString::number(element_list[i].p1)+", "+QString::number(element_list[i].p2)+"}, ").toStdString().c_str();
//    qDebug() << "};";
//    qDebug() << "Point3D fix_list[point_no] = {";
//    for (int i=0; i<point_no; i++)
//        qDebug()<< ("{" + QString(fix_list[i].x?"1":"0")+", "+QString(fix_list[i].y?"1":"0")+", "+QString(fix_list[i].z?"1":"0")+"}, ").toStdString().c_str();
//    qDebug() << "};";
//
//    qDebug() << "Forces";
//    for (int i=0; i<point_no; i++)
//    {
//        qDebug()<< QString::number(force_list[i].x,'f',2)+" "+QString::number(force_list[i].y,'f',2)+" "+QString::number(force_list[i].z,'f',2)+" ";
//        qDebug()<< (fix_list[i].x?"T ":"  ")+QString(fix_list[i].y?"T ":"  ")+(fix_list[i].z?"T ":"  ");
//    }
//
//    qDebug() << "K";
//    for (int i=0; i<dof_no; i++)
//    {
//        QString s;
//        for (int j=0; j<dof_no; j++) s= s+QString::number(k(i,j),'f',2)+" ";
//        qDebug() << s;
//    }
//    qDebug() << "F";
//    QString s;
//    for (int j=0; j<dof_no; j++) s= s+QString::number(F[j],'f',2)+" ";
//    qDebug() << s;
//	double L[dof_no][dof_no] = { { 0.0 } };
//	double U[dof_no][dof_no] = { { 0.0 } }; // upper matrix
//	double Z[dof_no] = { 0.0 }; // auxiliary vector
    double *Z= new double[(dof_no*dof_no)*2+dof_no*2]; // auxiliary vector
    double *L= Z+dof_no;          // lower matrix
    double *U= L+dof_no*dof_no;   // Upper matrix
    double *dP= U+dof_no*dof_no; // K * dP = F, delta P - solution vector
#define l(a,b) L[a*dof_no+b]
#define u(a,b) U[int(a)*int(dof_no)+int(b)]
    for (int i=int((dof_no*dof_no)*2+dof_no*2); --i>=0; ) Z[i]= 0.0; // init

    // init L := I
    for(unsigned int i = 0; i < dof_no; i++)
    {
        l(i,i) = 1.0;
    }

    // find L, U where L * U = K
    for(unsigned int i1 = 0; i1 < dof_no; i1++)
    {
//        qDebug() << i1 << " out of " << dof_no;
        double acc = 0.0;
        for(unsigned int i2 = 0; i2 < dof_no; i2++)
            acc += l(i1,i2) * u(i2,i1);
        u(i1,i1) = k(i1,i1) - acc;

        for(unsigned int i2 = i1 + 1; i2 < dof_no; i2++)
        {
            acc = 0.0;
            for(unsigned int i3 = 0; i3 < i1; i3++)
                acc += l(i1,i3) * u(i3,i2);
            u(i1,i2) = k(i1,i2) - acc;

            acc = 0.0;
            for(unsigned int i3 = 0; i3 < i1; i3++)
                acc += l(i2,i3) * u(i3,i1);
            l(i2,i1) = (k(i2,i1) - acc) / u(i1,i1);
        }
    }

    // finally find result
    for(unsigned int i1 = 0; i1 < dof_no; i1++)
    {
        // find Z where L * Z = F
        double acc = 0.0;
        for(unsigned int i2 = 0; i2 < i1; i2++)
            acc += l(i1,i2) * Z[i2];
        Z[i1] = F[i1] - acc;
    }
    for(int i1 = int(dof_no) - 1; i1 >= 0; i1--)
    {
        // find dP where U * dP = Z
        double acc = 0.0;
        for(unsigned int i2 = static_cast<unsigned int>(i1); i2 < dof_no; i2++)
            acc += u(i1,i2) * dP[i2];
        dP[i1] = (Z[i1] - acc) / u(i1,i1);
    }

    // copy to global output
    for(unsigned int i = 0; i < point_no; i++)
    {
        const Point3D& point = point_list[i];
        const Bool3D& fix = fix_list[i];
        Point3D& force = force_list[i];
        Point3D& output = output_list[i];

        unsigned int px = 3 * i + 0;
        unsigned int py = 3 * i + 1;
        unsigned int pz = 3 * i + 2;
        output = point;

        if(fix.x) force.x = dP[px];
        else output.x += dP[px];

        if(fix.y) force.y = dP[py];
        else output.y += dP[py];

        if(fix.z) force.z = dP[pz];
        else output.z += dP[pz];
    }
    qDebug() << "Point3D output_list[point_no] = {";
    for (int i=0; i<point_no; i++)
        qDebug()<< ("{" + QString::number(output_list[i].x)+", "+QString::number(output_list[i].y)+", "+QString::number(output_list[i].z)+"}, ").toStdString().c_str();
    qDebug() << "};";
    qDebug() << "Point3D force[point_no] = {";
    for (int i=0; i<point_no; i++)
        qDebug()<< ("{" + QString::number(force_list[i].x)+", "+QString::number(force_list[i].y)+", "+QString::number(force_list[i].z)+"}, ").toStdString().c_str();
    qDebug() << "};";
}
