#ifndef MES_H
#define MES_H
#include <stdlib.h>
class CMes
{
public:
    bool hasCalculated;
    CMes(): hasCalculated(false), young(64000), point_list(nullptr), force_list(nullptr), output_list(nullptr), fix_list(nullptr), element_list(nullptr), point_no(0), element_no(0), dof_no(0), F(nullptr), K(nullptr) { }
    ~CMes() { setNbPoints(0); setNbelement(0); }
    double young;                             // young elasticity number
    double density;
    struct Point3D { double x, y, z; };       // x,y,z coordinates, used for a number of things
    Point3D pts(double x, double y, double z) { Point3D r= {x, y, z}; return r; } // helper function
    struct Bool3D { bool x, y, z; };          // 3 booleans. Used to fix points...
    struct Element { unsigned int p1, p2; void set(int i, int j) { p1= i; p2= j; } };  // element. Used to create edges by linking points
    Element el(int i, int j) { Element e= {static_cast<unsigned int>(i), static_cast<unsigned int>(j)}; return e; }
    Point3D *point_list;                      // x,y,z positions of each points
    Point3D *force_list;                      // force vectors on each point
    Point3D *output_list;                     // End position
    Bool3D *fix_list;                         // put a true on x,y,z if the point is fixed in this direction
    Element *element_list;                    // edge list
    void calc() { populate_equation(); calculate_equation(); hasCalculated= true; } // Does the calculations. Get you results in output_list

    // Number of points. Use setNbPoints to set it!!!!
    unsigned int point_no;
    void setNbPoints(int i, bool reset= false)
    {
        point_no= static_cast<unsigned int>(i);
        dof_no= 3*point_no; // degrees of freedom
        if (i==0)
        {
            if (point_list!=nullptr) free(point_list);
            point_list= force_list= output_list= nullptr;
            if (fix_list!=nullptr) free(fix_list);
            fix_list= nullptr;
            if (F==nullptr) free(F);
            K= F= nullptr;
            return;
        }
        if (point_list==nullptr) point_list= static_cast<Point3D*>(malloc(point_no*sizeof(Point3D)*3));
        else point_list= static_cast<Point3D*>(realloc(point_list, point_no*sizeof(Point3D)*3));
        force_list= point_list+point_no;
        output_list= force_list+point_no;
        if (fix_list==nullptr) fix_list= static_cast<Bool3D*>(malloc(point_no*sizeof(Bool3D)));
        else fix_list= static_cast<Bool3D*>(realloc(fix_list, point_no*sizeof(Bool3D)));
        if (F==nullptr) F= static_cast<double*>(malloc(sizeof(double)*dof_no*(dof_no+1)));
        else F= static_cast<double*>(realloc(F, sizeof(double)*dof_no*(dof_no+1)));
        K= F+dof_no;
        if (!reset) return;
        Point3D const p= {0.0, 0.0, 0.0}; for (unsigned int i=0; i<point_no*3; i++) point_list[i]= p;
        Bool3D const b= {false, false, false}; for (unsigned int i=0; i<point_no; i++) fix_list[i]= b;
    }

    // Number of edges. Use setNbelement to set it!!!!
    unsigned int element_no;
    void setNbelement(int i, bool reset= false)
    {
        element_no= static_cast<unsigned int>(i);
        if (i==0)
        {
            if(element_list!=nullptr) free(element_list);
            return;
        }
        if (element_list==nullptr) element_list= static_cast<Element*>(malloc(element_no*sizeof(Element)));
        else element_list= static_cast<Element*>(realloc(element_list, element_no*sizeof(Element)));
        if (!reset) return;
        Element const p= {0,0}; for (unsigned int i=0; i<element_no; i++) element_list[i]= p;
    }

private:
    void calculate_equation();
    void populate_equation();
    unsigned int dof_no; // degrees of freedom
    double *F;           // force vector
    double *K;           // stiffness matrix
};

#endif // MES_H
