// tab navigation
// indicate ROC offset and allow to fix other parabolization on that offset!
// Couder mask print
// Plop
// find stroke that helped last time...
// zone changes signals...
// couder meshing problem with redistributing zones after the support ring.

#ifndef CBSMODEL_H
#define CBSMODEL_H

#include <math.h>
#include <QObject>
#include <QMutex>
#include <QMutexLocker>
#include <QFile>
#include <QList>
#include <QString>
#include <QDebug>
#include <QDateTime>
#include <QQmlEngine>
#include <QJSEngine>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QStandardPaths>
#include <QQuickPaintedItem>
#include <QDesktopServices>
#include "QQmlObjectListModel.h"
#include "mes.h"

// ********************************************************
// Most of our objects will derive from this savable object
// This provides the load and saveProperties functions that auto load/save QObject properties from a JSon object
class CBSSaveLoadObject : public QObject
{
    Q_OBJECT
public:
    CBSSaveLoadObject(QObject *parent=nullptr): QObject(parent) { }
    bool loadFile(QString const file)
    {
        QFile file_obj(file);
        if (!file_obj.open(QIODevice::ReadOnly)) { qDebug() << "Failed to open " << file_obj.fileName(); return false; }
        QTextStream file_text(&file_obj); // Load data
        QString json_string;
        json_string = file_text.readAll();
        file_obj.close();
        QByteArray json_bytes = json_string.toLocal8Bit();
        auto json_doc = QJsonDocument::fromJson(json_bytes); // Transform into json system
        if (json_doc.isNull()) { qDebug() << "Failed to create JSON doc."; return false; }
        if (!json_doc.isObject()) { qDebug() << "JSON is not an object."; return false; }
        QJsonObject o = json_doc.object();
        if (o.isEmpty()) { qDebug() << "JSON object is empty."; return false; }
        return loadProperties(&o); // Load the json object into our system
    }

    bool saveFile(QString const file) const
    {
        QFile file_obj(file);
        QJsonObject json_obj;
        saveProperties(&json_obj);

        QJsonDocument json_doc(json_obj);
        QString json_string = json_doc.toJson();

        if (!file_obj.open(QIODevice::WriteOnly)) { qDebug() << "failed to open save file"; return false; }
        file_obj.write(json_string.toLocal8Bit());
        file_obj.close();
        return true;
    }

    virtual bool subLoad(QJsonObject *o) { (void)o; return true; } // override these if you have more to load/save than read/write Q_PROPERTIES
    virtual bool subSave(QJsonObject *o) const { (void)o; return true; } // override these if you have more to load/save than read/write Q_PROPERTIES
    virtual QList<QString> Ignored() const { return QList<QString>(); }  // override to provide a list of properties to ignore!

    bool loadProperties(QJsonObject *j) // Load all the properties from j into this.
    {
        const QMetaObject *metaobject = metaObject();
        int count = metaobject->propertyCount();                   // For each property in this
        QList<QString> ignore(Ignored());
        for (int i=1; i<count; ++i) {                              // skip object name (which is the first property)
            QMetaProperty metaproperty = metaobject->property(i);  // get the property accessor
            if (!metaproperty.isWritable()) continue;              // only works on read/write properties
            if (ignore.indexOf(metaproperty.name())!=-1) continue; // to ignore
            QJsonValue v= j->take(metaproperty.name());            // and and remove the associated object from json
            if (v.isUndefined()) { qDebug() << "could not find" << metaproperty.name() << "in" << *j; continue; } // if not in jason, do nothing
            bool ok= metaproperty.write(this, v.toVariant());      // save json value into this property
            (void)ok;
        }
        return subLoad(j);          // load all the stuff that can not be done automatically
    }
    // helper function to load lists of SaveLoadObjects
    // assumes that j.property is an array of object of type T. load said array in list d
    template <typename T> bool loadList(QQmlObjectListModel<T> *d, char const *property, QJsonObject *j)
    {
        d->clear();        // clear current list content
        QJsonValue v= j->take(property); if (v.isUndefined()) return false; // find data in json
        QJsonArray a= v.toArray();
        for (int i=0; i<a.count(); i++)          // for all object in the json property
        {
            T *o2= new T(d);                     // create new C++ object
            QJsonObject jo= a.at(i).toObject();
            o2->loadProperties(&jo);             // load from json
            d->append(o2);                       // add to list
        }
        return true;
    }
    // save this object in a Json object
    void saveProperties(QJsonObject *j) const
    {
        const QMetaObject *metaobject = metaObject();
        int count = metaobject->propertyCount();            // for all my properties
        QList<QString> ignore(Ignored());
        for (int i=1; i<count; ++i)                         // skip object name (first property)
        {
            QMetaProperty p= metaobject->property(i);
            if (!p.isWritable()) continue;                  // skip read only stuff!
            if (ignore.indexOf(p.name())!=-1) continue; // to ignore
            j->insert(p.name(), QJsonValue::fromVariant(property(p.name()))); // add property in Json object
        }
        subSave(j); // save all the object specific stuff
    }
    // helper function to save a list of object in a Json object
    // saves all objects form d in j.property
    template <typename T> bool saveList(QQmlObjectListModel<T> * d, char const *property, QJsonObject *j) const
    {
        QJsonArray a;
        for (int i=0; i<d->count(); i++)
        {
            QJsonObject jo; d->at(i)->saveProperties(&jo); // save each object to Json
            a.append(jo);                           // add object to Json array
        }
        j->insert(property, QJsonValue(a));         // add array in j
        return true;
    }
};


template <typename T> bool inline doubleEq(T d1, T d2) { return d1==d2; } // return true if the 2 numbers are equal with 10 significant digits
static bool inline doubleEq(double d1, double d2) { return std::abs(d1-d2)<=std::abs((d1+d2)/2.0e10); } // return true if the 2 numbers are equal with 10 significant digits
static double inline volumeSphereCap(double r, double sagita) { return M_PI*sagita*sagita/3.0*(3.0*r-sagita); } // return the volume of a part of a sphere
static double inline sagita(double radiusSphere, double radiusDisc) { return radiusSphere-sqrt(radiusSphere*radiusSphere-radiusDisc*radiusDisc/4.0); } // calculate the sagita of disk
// adjusts offset for field diameter by using the diagonal to eye distance;
// since diagToEyeDistance > diagToFocalPlaneDistance, the offset will shrink as the diagonal appears closer to the primary mirror;
// from http://www.telescope-optics.net/newtonian.htm
static double inline forwardOffsetForSecondary(double diagMinorAxis, double diagToEyeDistance) { return -diagMinorAxis * diagMinorAxis / 4.0 / diagToEyeDistance; }

// The set of macros bellow are there to simplify property definitions...
// Each time you define a property, you need to enter at least 5 lines of code...
// with these macros, you only need 2, which is much better!
// To use the macro, you do: CBSProp???(property_type, name, Name, ?, ?)
// Unfortunately, you have to enter the name twice, once with a lowercase first letter and once with an uppercase first letter...
// they are 2 versions of the macro.
// The normal version and the E version. The E version allows you to add code when the variable is changed
// Talking about signals! Since ALL the signals HAVE to be together in Qt, you will still have to type the signal by hand :-(
#define CBSPropE(type, name, Name, emits) \
    Q_PROPERTY(type name READ get##Name WRITE set##Name NOTIFY name##Changed) \
    type _##name; \
public:\
    type get##Name() const { return _##name; } \
    void set##Name(type v) { if (doubleEq(_##name,v)) return; _##name= v; emit name##Changed(); emits; }

#define CBSProp(type, name, Name) CBSPropE(type, name, Name, {})



//***************************************************
// Forwards
class CBSModelScope;
double getScopeDiameter(CBSModelScope *scope); // deported to .cpp file to access inner stuff...
double getScopeSpherometerLegDistances(CBSModelScope *scope); // deported to .cpp file to access inner stuff...
double getScopeFocal(CBSModelScope *scope); // deported to .cpp file to access inner stuff...

//***************************************************
// Represents time spend hogging the mirror (digging the curve)
class CBSModelHoggingWork : public CBSSaveLoadObject {
    Q_OBJECT
public:
    CBSProp(QString, comments, Comments)
    CBSPropE(double, time, Time, emit hogSpeedChanged())
    CBSPropE(double, startSphere, StartSphere, emit hogSpeedChanged())
    CBSPropE(double, endSphere, EndSphere, emit hogSpeedChanged(); emit endSagitaChanged(); emit endSphereSpherometerChanged(); )
    CBSProp(double, grit, Grit) // TODO: notify at higher level when average speed will change!!!
    Q_PROPERTY(double hogSpeed READ getHogSpeed NOTIFY hogSpeedChanged)
    Q_PROPERTY(double endSagita READ getEndSagita NOTIFY endSagitaChanged)
    CBSPropE(CBSModelScope*, scope, Scope, QObject *v2= (QObject*)(v);
                                           connect(this, SIGNAL(gritChanged()), v2, SLOT(emitHogTimeWithGritChanged()));
                                           connect(this, SIGNAL(hogSpeedChanged()), v2, SLOT(emitHogTimeWithGritChanged())); )
Q_SIGNALS:
    void commentsChanged();
    void timeChanged();
    void startSphereChanged();
    void endSphereChanged();
    void gritChanged();
    void hogSpeedChanged();
    void endSagitaChanged();
    void scopeChanged();
    void endSphereSpherometerChanged();
public:
    CBSModelHoggingWork(QObject *parent=nullptr): CBSSaveLoadObject(parent), _time(0.0), _startSphere(0.0), _endSphere(0.0), _grit(0.0), _scope(nullptr) { }
    QList<QString> Ignored() const { QList<QString> l; l.append("scope"); return l; }
    double getEndSagita() { if (_scope==nullptr) return 0.0; return sagita(_endSphere, getScopeDiameter(_scope)); }
    double getHogSpeed()
    {
        if (doubleEq(_time, 0.0) || _scope==nullptr) return 0;
        double v1= 0.0, v2= 0.0;
        if (!doubleEq(_startSphere, 0.0)) v1= volumeSphereCap(_startSphere, sagita(_startSphere, getScopeDiameter(_scope)));
        if (!doubleEq(_endSphere, 0.0)) v2= volumeSphereCap(_endSphere, sagita(_endSphere, getScopeDiameter(_scope)));
        return (v2-v1)/_time;
    }
    Q_INVOKABLE void setEndSphereSpherometer(double v)
    {
        if (_scope==nullptr) return;
        double s= v/2.0 + getScopeSpherometerLegDistances(_scope)*getScopeSpherometerLegDistances(_scope)/(6.0*v);
        QString out;
        QStringList l= _comments.split("\n");
        for (int i=0; i<l.count(); i++) if (!l.at(i).startsWith("value from spherometer reading")) out= out+l.at(i)+"\n";
        setComments(out+"value from spherometer reading of: "+QString::number(v));
        setEndSphere(floor(s));
    }
};

//**********************************************************
// a class the embedds a double and can be used in a model
class CBSDouble : public CBSSaveLoadObject {
    Q_OBJECT
public:
    CBSProp(double, val, Val)
Q_SIGNALS:
    void valChanged();
public:
    CBSDouble(QObject *parent=nullptr): CBSSaveLoadObject(parent), _val(0.0) { }
};

//***************************************************
// a class the embedds a QString property and allows to get the string value as a double.
// If the string has more than 1 numbers in it, returns the average of the numbers...
class CBSQString : public CBSSaveLoadObject {
    Q_OBJECT
public:
    CBSPropE(QString, val, Val, _doubleVal= NAN;)
Q_SIGNALS:
    void valChanged();
public:
    CBSQString(QObject *parent=nullptr): CBSSaveLoadObject(parent), _doubleVal(NAN) { }
    double _doubleVal;
    double asDouble()
    {
        if (!std::isnan(_doubleVal)) return _doubleVal;
        QStringList l(_val.split(" "));
        _doubleVal= 0.0; int nb= 0;
        foreach(QString const &s, l)
        {
            bool ok; double t= s.toDouble(&ok); if (ok) { _doubleVal+= t; nb++; }
        }
        if (nb==0) return 0.0;
        return _doubleVal/= nb;
    }
};

//**************************************************************
// model object for a couder mesure.
// Will calculate, based on the mesure, the mirror surface. CBScopeMesure can then display it...
class CBSModelParabolizingWork : public CBSSaveLoadObject {
    Q_OBJECT
public:
    CBSProp(double, time, Time)
    CBSProp(QString, comments, Comments)
    QML_OBJMODEL_PROPERTY(CBSQString, mesures) // The mesured positions for the zones.
    CBSPropE(CBSModelScope*, scope, Scope, doCalculations(); QObject *v2= (QObject*)v; // needed to create the connections... do not know why...
                                           connect(v2, SIGNAL(diametreChanged()), this, SLOT(doCalculations()));
                                           connect(v2, SIGNAL(slitIsMovingChanged()), this, SLOT(doCalculations()));
                                           connect(v2, SIGNAL(focalChanged()), this, SLOT(doCalculations())); // Should also connect on zone changes, but I have no signal on this yet...
                                           connect(v2, SIGNAL(nbZonesChanged()), this, SLOT(doCalculations())); ) // TODO: add connection here to get scope property changes!
Q_SIGNALS:
    void commentsChanged();
    void timeChanged();
    void mesuresChanged();
    void scopeChanged();
public:
    CBSModelScope *scope;
    CBSModelParabolizingWork(QObject *parent=nullptr): CBSSaveLoadObject(parent), _time(0.0), scope(nullptr),
      _lf1000(nullptr), _mesc(nullptr), _Hm4F(nullptr), _RelativeSurface(nullptr),
      iNbZone(0), _surf(nullptr), _profil(nullptr), _Hz(nullptr),
      _Std(0.0), _Lambda(0.0), _GlassMax(0.0), _WeightedLambdaRms(0.0), _WeightedStrehl(0.0), _LfRoMax(0.0), _glassToRemove(0.0), _focale(0.0)
    { m_mesures= new QQmlObjectListModel<CBSQString>(this); }
    ~CBSModelParabolizingWork()
    {
        delete m_mesures;
        if (_surf  !=nullptr) delete[] _surf  ;
        if (_profil!=nullptr) delete[] _profil;
        if (_lf1000!=nullptr) delete[] _lf1000;
        if (_mesc  !=nullptr) delete[] _mesc  ;
        if (_Hz    !=nullptr) delete[] _Hz    ;
        if (_Hm4F  !=nullptr) delete[] _Hm4F  ;
        if (_RelativeSurface!=nullptr) delete[] _RelativeSurface;
    }
    bool subLoad(QJsonObject *o)
    {
        bool ret= loadList(m_mesures, "mesures", o);
        for (int i=0; i<m_mesures->count(); i++) connect(m_mesures->at(i), SIGNAL(valChanged()), this, SLOT(doCalculations()));
        return ret;
    }
    bool subSave(QJsonObject *o) const { return saveList(m_mesures, "mesures", o); }
    QList<QString> Ignored() const { QList<QString> l; l.append("scope"); return l; }
public slots:
    void checkNbMesures();
    void doCalculations();
private:
    double find_minimum(double a,double c,double res,double (*fcn)(CBSModelParabolizingWork *self,double h));
    static double calc_lf1000(CBSModelParabolizingWork *self, double h);
    static double calc_less_rms(CBSModelParabolizingWork* self,double curv);
    static double calc_less_ptv(CBSModelParabolizingWork* self,double curv);
    double *_lf1000;
    double *_mesc  ;
    double *_Hm4F  ;
    double *_RelativeSurface;
public:
    int iNbZone; // applies to _surf
    double *_surf  ;
    double *_profil;
    double *_Hz  ;
    double _Std;
    double _Lambda;
    double _GlassMax;
    double _WeightedLambdaRms;
    double _WeightedStrehl;
    double _LfRoMax;
    double _glassToRemove;
    double _focale;
};

//*****************************************************
// Model object for an eye peice. Focal and angle are input. The rest is calcualted...
class CBSModelEP : public CBSSaveLoadObject {
    Q_OBJECT
public:
    CBSProp(QString, name, Name)
    CBSPropE(double, focal, Focal, emit fieldChanged(); ) // TODO: add emit to higher level to repaint ilumination!
    CBSPropE(double, angle, Angle, emit fieldChanged(); ) // TODO: add emit to higher level to repaint ilumination!
    Q_PROPERTY(double field READ getField NOTIFY fieldChanged)
    Q_PROPERTY(double zoom READ getZoom NOTIFY zoomChanged)
    Q_PROPERTY(double viewAngle READ getViewAngle NOTIFY viewAngleChanged)
    CBSPropE(CBSModelScope*, scope, Scope, QObject *v2= (QObject*)v;
                                           connect(this, SIGNAL(fieldChanged()), v2, SLOT(emitEpsChanged()));
                                           connect(v2, SIGNAL(focalChanged()), this, SLOT(ScopeFocalChanged())); )
Q_SIGNALS:
    void nameChanged();
    void focalChanged();
    void angleChanged();
    void fieldChanged();
    void viewAngleChanged();
    void zoomChanged();
    void scopeChanged();
public slots:
    void ScopeFocalChanged() { emit viewAngleChanged(); emit zoomChanged(); }
public:
    CBSModelEP(QObject *parent=nullptr): CBSSaveLoadObject(parent), _focal(20), _angle(60), _scope(nullptr) { }
    ~CBSModelEP() { }
    double getField() const { return 2*_focal*tan(_angle/2/180*M_PI); }
    double getZoom() { if (_scope==nullptr) return 0.0; return getScopeFocal(_scope)/_focal; }
    double getViewAngle() { if (_scope==nullptr) return 0.0; return _angle/(getScopeFocal(_scope)/_focal); }
    QList<QString> Ignored() const { QList<QString> l; l.append("scope"); return l; }
};

//******************************************************
// Model object for one telescope. Mirror, zones, eyepeices, hoggin work....
class CBSModelScope : public CBSSaveLoadObject {
    Q_OBJECT
public:
    CBSProp(QString, name, Name)
    CBSProp(QString, comments, Comments)
    CBSProp(QString, secondariesToConcider, SecondariesToConcider)
    CBSPropE(double, diametre, Diametre, emit nbZonesSuggestedChanged(); emit leftToHogChanged(); emit toHogChanged(); emit sagitaChanged(); emit hogTimeWithGritChanged())
    CBSProp(double, thickness, Thickness)
    CBSProp(double, density, Density)
    CBSPropE(double, focal, Focal, emit nbZonesSuggestedChanged(); emit leftToHogChanged(); emit toHogChanged(); emit sagitaChanged(); emit hogTimeWithGritChanged(); )
    CBSPropE(double, secondary, Secondary, emit nbZonesSuggestedChanged(); emit secondaryOffsetChanged())
    CBSPropE(double, secondaryToFocal, SecondaryToFocal, emit nbZonesSuggestedChanged(); emit secondaryOffsetChanged())
    CBSProp(double, spherometerLegDistances, SpherometerLegDistances)
    Q_PROPERTY(int nbZonesSuggested READ getNbZonesSuggested NOTIFY nbZonesSuggestedChanged)
    CBSPropE(double, excludedAngle, ExcludedAngle, emit nbZonesSuggestedChanged())
    Q_PROPERTY(double secondaryOffset READ getSecondaryOffset NOTIFY secondaryOffsetChanged)
    Q_PROPERTY(double leftToHog READ getLeftToHog NOTIFY leftToHogChanged)
    Q_PROPERTY(double toHog READ getToHog NOTIFY toHogChanged)
    Q_PROPERTY(double sagita READ getSagita NOTIFY sagitaChanged)
    Q_PROPERTY(double hogTimeWithGrit READ getHogTimeWithGrit NOTIFY hogTimeWithGritChanged)
    Q_PROPERTY(double nbZones READ getNbZones WRITE setNbZones NOTIFY nbZonesChanged)
    CBSProp(bool, slitIsMoving, SlitIsMoving)
    QML_OBJMODEL_PROPERTY(CBSModelHoggingWork, hoggings)
    QML_OBJMODEL_PROPERTY(CBSModelParabolizingWork, parabolizings)
    QML_OBJMODEL_PROPERTY(CBSDouble, zones)
    QML_OBJMODEL_PROPERTY(CBSModelEP, eps)
    CBSProp(int, cellType, CellType)
Q_SIGNALS:
    void nameChanged();
    void commentsChanged();
    void diametreChanged();
    void focalChanged();
    void secondaryChanged();
    void secondaryToFocalChanged();
    void secondariesToConciderChanged();
    void spherometerLegDistancesChanged();
    void excludedAngleChanged();
    void nbZonesSuggestedChanged();
    void secondaryOffsetChanged();
    void leftToHogChanged();
    void toHogChanged();
    void sagitaChanged();
    void epsChanged();
    void hogTimeWithGritChanged();
    void nbZonesChanged();
    void slitIsMovingChanged();
    void cellTypeChanged();
    void thicknessChanged();
    void densityChanged();
public slots:
    void emitEpsChanged() { emit epsChanged(); }
    void emitHogTimeWithGritChanged() { emit hogTimeWithGritChanged(); }
public:
    CBSModelScope(QObject *parent=nullptr): CBSSaveLoadObject(parent), _secondariesToConcider("19 25 35 50 63 70 80 88 100"),
                           _diametre(150), _thickness(25), _density(2.8), _focal(750), _secondary(35), _secondaryToFocal(150/2+80), _spherometerLegDistances(56),
                           _excludedAngle(2.0), _slitIsMoving(true), _cellType(0), _conical(-1)
    {
        m_hoggings= new QQmlObjectListModel<CBSModelHoggingWork>(this);
        m_parabolizings= new QQmlObjectListModel<CBSModelParabolizingWork>(this);
        m_eps= new QQmlObjectListModel<CBSModelEP>(this);
        m_zones= new QQmlObjectListModel<CBSDouble>(this);
        setNbZones(getNbZonesSuggested()); autoZones();
    }
    ~CBSModelScope() {
        delete m_hoggings;
        delete m_parabolizings;
        delete m_zones;
        delete m_eps;
    }
    Q_INVOKABLE int addSpherometer() //double time, double reading, bool realSphere, double grit)
    {
        CBSModelHoggingWork *t= new CBSModelHoggingWork(m_hoggings);
        if (m_hoggings->count()!=0) { t->_startSphere= m_hoggings->last()->_endSphere; t->_grit= m_hoggings->last()->_grit; }
        m_hoggings->append(t);
        t->setScope(this);
        emit hogTimeWithGritChanged();
        return m_hoggings->count()-1;
    }
    Q_INVOKABLE int addEp(QString name, double focal, double angle)
    {
        CBSModelEP *e= new CBSModelEP(m_eps); e->setFocal(focal); e->setAngle(angle); e->setName(name); m_eps->append(e); e->setScope(this);
        return m_eps->count()-1;
    }
    double getSagita() { return ::sagita(_focal*2, _diametre); }                           // get mirror sagita
    double getToHog() { return volumeSphereCap(_focal*2, ::sagita(_focal*2, _diametre)); } // Total amount of material to hog for the mirror
    double getLeftToHog()                                                                  // amount of material left to hog to complete the mirror
    {
        if (m_hoggings->count()==0) return 0.0;
        double current= m_hoggings->at(m_hoggings->count()-1)->_endSphere;                 // assume last hogging is the deepest. get the sphere diameter
        double hogged= volumeSphereCap(current, ::sagita(current, _diametre));             // calculate how much was hogged
        return getToHog()-hogged;                                                          // substract from total to hog
    }
    Q_INVOKABLE double getHogTimeWithGrit() // return the amount of time needed to finish the hogging with the latest used grit
    {
        if (m_hoggings->count()==0) return 0.0;
        double grit= m_hoggings->last()->_grit; // Get the last used grit
        int counts= 0; double sum= 0.0;         // summ the hogging speed of all the hog works that used the current grit
        for (int i=0; i<m_hoggings->count(); i++)
            if (doubleEq(grit, m_hoggings->at(i)->_grit))
            { counts++; sum+= m_hoggings->at(i)->getHogSpeed(); }
        if (counts==0) return 0.0;              // no items, return 0
        return getLeftToHog()*counts/sum;       // estimate hog time using current average hog speed
    }
    double getSecondaryOffset() { return -forwardOffsetForSecondary(_secondary, _secondaryToFocal); }
    int getNbZonesSuggested() // return suggested amount of zones. Found the formulat online
    {
        //double n = (1-exc) * _diametre *_diametre / (16 * sqrt( Rmm*Rmm*Rmm * kmm ));
        double exclude= tan(_excludedAngle/180*M_PI)*(_focal-_secondaryToFocal)*2-_secondary;
        if (exclude<0.0) exclude= 0.0;
        double n = (_diametre-exclude) *_diametre / (16 * sqrt( _focal*_focal*_focal * 8.0 * 37e-6 )); //37e-6= 1/15 wavelength, in mm
        if (n-floor(n)<0.3) return int(n); else return int(n)+1;
    }
    int getNbZones() { int r= m_zones->count(); if (r==0) return 0; return r-1; } // return the number of zones...
            void calcZones(int d, int e) // set equi-surface zones between boundaries d and e
            {
                int nb= e-d; if (nb<=1) return; // nothing to calcualte
                double rd= m_zones->at(d)->_val, re= m_zones->at(e)->_val; // radius at start and end
                double surfaceStart= rd*rd*M_PI;                 // start surface
                double surfaceInc= (re*re*M_PI-surfaceStart)/nb; // increment surface = (end_surface-start_surface)/nb
                d++; while (d<e)
                {
                    surfaceStart+= surfaceInc;
                    m_zones->at(d++)->setVal(int(sqrt(surfaceStart/M_PI)*100)/100.0); // set the zones at the radius for the desired surface...
                }
            }
        void validateZones()
        {
            int d= 0, e= m_zones->count()-1;
            double fd= 0.0, fe= _diametre/2.0;
            if (e<4) { while (m_zones->count()!=4) m_zones->append(new CBSDouble(m_zones)); e= 4; } // make sure we have at least 4 zones
            m_zones->at(e)->setVal(fe); // make sure zones are roughtly legit!
            for (int i= m_zones->count(); --i>=0;)
                if (m_zones->at(0)->_val<0.0) m_zones->at(i)->setVal(0.0);
                else if (m_zones->at(0)->_val>_diametre/2.0) m_zones->at(i)->setVal(_diametre/2.0);
            while (d+1<e && m_zones->at(d+1)->_val>fd && m_zones->at(d+1)->_val<fe) { fd= m_zones->at(d)->_val; d++; }
            while (d<e-1 && m_zones->at(e-1)->_val<fe && m_zones->at(e-1)->_val>fd) { fe= m_zones->at(e)->_val; e--; }
            if (d<e-1) calcZones(d, e);
        }
    void setNbZones(int v) // set the number of zones.
    {
        if (v==m_zones->count()-1 || v<3) return;
        while (v>m_zones->count()-1) m_zones->append(new CBSDouble(m_zones));
        while (v<m_zones->count()-1) m_zones->remove(m_zones->count()-1);
        validateZones();
        emit nbZonesChanged();
    }
    Q_INVOKABLE void autoZones() // auto calc zones
    {
        while (m_zones->count()<4) m_zones->append(new CBSDouble(m_zones));
        m_zones->at(m_zones->count()-1)->setVal(_diametre/2.0);
        calcZones(0, m_zones->count()-1);
    }
    Q_INVOKABLE int addMesure()
    {
        CBSModelParabolizingWork *p= new CBSModelParabolizingWork(m_parabolizings); m_parabolizings->append(p);
        p->setScope(this);
        return m_parabolizings->count()-1;
    }
    bool subLoad(QJsonObject *o)
    {
        loadList(m_hoggings, "hoggings", o);
        loadList(m_zones, "zones", o);
        loadList(m_parabolizings, "parabolizings", o);
        loadList(m_eps, "eps", o);
        for (int i=0; i<m_hoggings->count(); i++) m_hoggings->at(i)->setScope(this);
        for (int i=0; i<m_parabolizings->count(); i++) m_parabolizings->at(i)->setScope(this);
        for (int i=0; i<m_eps->count(); i++) m_eps->at(i)->setScope(this);
        return true;
    }
    bool subSave(QJsonObject *o) const
    {
        saveList(m_hoggings, "hoggings", o);
        saveList(m_parabolizings, "parabolizings", o);
        saveList(m_zones, "zones", o);
        saveList(m_eps, "eps", o);
        return true;
    }
    double _conical;
        void paintCouder(QPainter *painter, QPoint &c, double dpi, bool showRed=true, bool showBlue=true);
    Q_INVOKABLE void printCouder();
        double thicnknessAt(double r) { return _thickness-sagita(_focal*2.0, (_diametre/2.0)-r); } // Thickness of the mirror at a given distance from center
    Q_INVOKABLE void doMes();
    Q_INVOKABLE void doMesSolve();
    CMes mes;
};


//*******************************************************
// the main model. ie: the list of scopes...
QString getAppPath();
class CBSModel : public CBSSaveLoadObject {
    Q_OBJECT
public:
    CBSProp(int, windowsPosX, WindowsPosX)
    CBSProp(int, windowsPosY, WindowsPosY)
    CBSProp(int, windowsWidth, WindowsWidth)
    CBSProp(int, windowsHeight, WindowsHeight)
    CBSProp(int, windowsFlags, WindowsFlags)
    CBSProp(int, windowsFont, WindowsFont)
    CBSProp(bool, windowsFontBold, WindowsFontBold)
    QML_OBJMODEL_PROPERTY(CBSModelScope, scopes)
Q_SIGNALS:
    void windowsPosXChanged();
    void windowsPosYChanged();
    void windowsWidthChanged();
    void windowsHeightChanged();
    void windowsFlagsChanged();
    void windowsFontChanged();
    void windowsFontBoldChanged();
public:
    CBSModel(QObject *parent=nullptr): CBSSaveLoadObject(parent), _windowsPosX(-1), _windowsPosY(-1), _windowsWidth(-1), _windowsHeight(-1), _windowsFlags(-1), _windowsFont(12), _windowsFontBold(false)
    {
        m_scopes= new QQmlObjectListModel<CBSModelScope>(this);
        loadFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)+"/CBScopes.scopes"); // load file
        qDebug() << "end of load";
        if (m_scopes->count()==0) // If nothing, create default scope
        { CBSModelScope *s= new CBSModelScope(m_scopes); s->setName("Default"); m_scopes->append(s); }
    }
    ~CBSModel() { delete m_scopes; }
      static CBSModel *singleton;
    static QObject *SingletonProvider(QQmlEngine *engine, QJSEngine *scriptEngine) // for Qml link...
    {
        (void)engine; (void)scriptEngine;
        if (singleton==nullptr) singleton= new CBSModel();
        return singleton;
    }
    Q_INVOKABLE int addScope() { CBSModelScope *s= new CBSModelScope(m_scopes); s->setName("Default"); m_scopes->append(s); return m_scopes->count()-1; }
    void saveFile() { CBSSaveLoadObject::saveFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)+"/CBScopes.scopes"); }
    bool subLoad(QJsonObject *o) { return loadList(m_scopes, "scopes", o); }
    bool subSave(QJsonObject *o) const { return saveList(m_scopes, "scopes", o); }
    Q_INVOKABLE void help() { QDesktopServices::openUrl(QUrl::fromLocalFile(getAppPath()+"/SBScope help.odt")); }
};

//***********************************************
// display component for a couder screen mesure
class CBScopeMesure : public QQuickPaintedItem
{
    Q_OBJECT
public:
    CBSProp(CBSModelScope*, scope, Scope)
    CBSProp(CBSModelParabolizingWork*, mesure, Mesure)
Q_SIGNALS:
    void mesureChanged();
    void scopeChanged();
public:
    CBScopeMesure(QQuickItem *parent = nullptr): QQuickPaintedItem(parent), _scope(nullptr), _mesure(nullptr) { }
    void paint(QPainter *painter);
    // members needed by the get functions that will be filled at paint time
        int w, h, addx, addy;
        double rw, rh;
    int getX(double x) { return int(x/rw*(w-2*addx))+addx; }
    int getY(double y) { return int((rh-y)/rh*(h-2*addy))+addy; }
    QPoint getP(double x, double y) { return QPoint(getX(x), getY(y)); }
};

//***********************************************
// field ilumination depending on secondary size
class CBScopeIlumination : public QQuickPaintedItem
{
    Q_OBJECT
public:
    CBSProp(CBSModelScope*, scope, Scope)
Q_SIGNALS:
        void scopeChanged();
    public:
    CBScopeIlumination(QQuickItem *parent = nullptr): QQuickPaintedItem(parent), _scope(nullptr) { }
    void paint(QPainter *painter);

    // Members for cartesian to pixel conversions
        int w, h, addx, addy; // populated by paint...
        double rw;
    int getX(double x) { x+=rw/2.0; return addx+int(x/rw*0.8*w); }
    int getY(double y) { y= 1-y; return int(y*2.0*0.8*h)+addy; }
    QPoint getP(double x, double y) { return QPoint(getX(x), getY(y)); }
};

//***************************************
// Couder mask display component
class CBScopeCouder : public QQuickPaintedItem
{
    Q_OBJECT
public:
    CBSProp(CBSModelScope*, scope, Scope)
    CBSProp(bool, showRed, ShowRed)   // normal zones style display
    CBSProp(bool, showBlue, ShowBlue) // my zone style display
    CBSProp(bool, zoom, Zoom)
Q_SIGNALS:
    void scopeChanged();
    void showRedChanged();
    void showBlueChanged();
    void zoomChanged();
public:
    CBScopeCouder(QQuickItem *parent = nullptr): QQuickPaintedItem(parent), _scope(nullptr), _showRed(true), _showBlue(true), _zoom(false) { }
    void paint(QPainter *painter);
};

//***************************************
// Cell design painter component
// For the moment, displays finite element method mesh
class CBScopeMes : public QQuickPaintedItem
{
    Q_OBJECT
public:
    CBSProp(CBSModelScope*, scope, Scope)
    CBSProp(bool, show3D, Show3D)
    CBSProp(bool, showForces, ShowForces)
Q_SIGNALS:
    void scopeChanged();
    void show3DChanged();
    void showForcesChanged();
public:
    CBScopeMes(QQuickItem *parent = nullptr): QQuickPaintedItem(parent), _scope(nullptr), _show3D(false), _showForces(true) { }
    void paint(QPainter *painter);
};
#endif
