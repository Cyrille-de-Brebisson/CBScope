// Move input with round to a component
// same with output
// add delete in row with comments and time in parabolizing
// indicate ROC offset and allow to fix other parabolization on that offset!

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

    bool saveFile(QString const file)
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
    virtual bool subSave(QJsonObject *o) { (void)o; return true; } // override these if you have more to load/save than read/write Q_PROPERTIES
    virtual QList<QString> Ignored() { return QList<QString>(); }  // override to provide a list of properties to ignore!

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
//            qDebug() << "writing" << v.toVariant() << "to" << metaproperty.name() << (ok?"went OK":"failled");
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
    void saveProperties(QJsonObject *j)
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
    template <typename T> bool saveList(QQmlObjectListModel<T> *d, char const *property, QJsonObject *j)
    {
        QJsonArray a;
        for (int i=0; i<d->count(); i++) // for all object in the list
        {
            QJsonObject jo; d->at(i)->saveProperties(&jo); // save each object to Json
            a.append(jo);                                  // add object to Json array
        }
        j->insert(property, QJsonValue(a));                // add array in j
        return true;
    }
};


static bool inline doubleEq(double d1, double d2) { return std::abs(d1-d2)<=std::abs((d1+d2)/2.0e10); }
static double inline volumeSphereCap(double r, double sagita) { return M_PI*sagita*sagita/3.0*(3.0*r-sagita); }
static double inline sagita(double radiusSphere, double radiusDisc) { return radiusSphere-sqrt(radiusSphere*radiusSphere-radiusDisc*radiusDisc/4.0); }
double forwardOffsetForSecondary(double diagMinorAxis, double diagToEyeDistance);

// The set of macros bellow are there to simplify property definitions...
// Each time you define a property, you need to enter at least 5 lines of code...
// with these macros, you only need 2, which is much better!
// To use the macro, you do: CBSProp???(property_type, name, Name, ?, ?)
// Unfortunately, you have to enter the name twice, once with a lowercase first letter and once with an uppercase first letter...
// they are 3 versions of the macro.
// The G version will let you define the getter function (just type code in {} as if it was the function and return the value)
// The S version will let you define the setter function. There things are slightly different. Do a return if the value is not changed
// else do not do a return to have the time stamp updated (if appropriate) and the emit of the signal done...
// Talking about signals! Since ALL the signals HAVE to be together in Qt, you will have to also add a '2' version of the macro
// in the signal area. Just copy/paste what you entered in the property area and add the 2 after the macro name....
        #define CBSProp(type, name, Name) \
            Q_PROPERTY(type name READ get##Name WRITE set##Name NOTIFY name##Changed) \
            type _##name; \
            public: type get##Name() const { return _##name; } \
            void set##Name(type v) { if (_##name==v) return; _##name= v; emit name##Changed(); }


        #define CBSPropE(type, name, Name, emits) \
            Q_PROPERTY(type name READ get##Name WRITE set##Name NOTIFY name##Changed) \
            type _##name; \
            public: type get##Name() const { return _##name; } \
            void set##Name(type v) { if (_##name==v) return; _##name= v; emits; emit name##Changed(); }

        #define CBSPropD(type, name, Name) \
            Q_PROPERTY(type name READ get##Name WRITE set##Name NOTIFY name##Changed) \
            type _##name; \
            type get##Name() const { return _##name; } \
            void set##Name(type v) { if (doubleEq(_##name,v)) return; _##name= v; emit name##Changed(); }

        #define CBSPropDE(type, name, Name, emits) \
            Q_PROPERTY(type name READ get##Name WRITE set##Name NOTIFY name##Changed) \
            type _##name; \
            type get##Name() const { return _##name; } \
            void set##Name(type v) { if (doubleEq(_##name,v)) return; _##name= v; emit name##Changed(); emits; }


class CBSModelScope;
double getScopeDiameter(CBSModelScope *scope); // deported to .cpp file to access inner stuff...
double getScopeSpherometerLegDistances(CBSModelScope *scope); // deported to .cpp file to access inner stuff...
double getScopeFocal(CBSModelScope *scope); // deported to .cpp file to access inner stuff...
class CBSModelScope;
class CBSModelParabolizingWork;

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

class CBSModelHoggingWork : public CBSSaveLoadObject {
    Q_OBJECT
public:
    CBSProp(QString, comments, Comments)
    CBSPropDE(double, time, Time, emit hogSpeedChanged())
    CBSPropDE(double, startSphere, StartSphere, emit hogSpeedChanged())
    CBSPropDE(double, endSphere, EndSphere, emit hogSpeedChanged(); emit endSagitaChanged(); emit endSphereSpherometerChanged(); )
    CBSPropD(double, grit, Grit) // TODO: notify at higher level when average speed will change!!!
    Q_PROPERTY(double hogSpeed READ getHogSpeed NOTIFY hogSpeedChanged)
    Q_PROPERTY(double endSagita READ getEndSagita NOTIFY endSagitaChanged)
    CBSPropE(CBSModelScope*, scope, Scope, QObject *v2= (QObject*)v;
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
    double getEndSagita() { if (_scope==nullptr) return 0.0; return sagita(_endSphere, getScopeDiameter(_scope)); }
    double getHogSpeed()
    {
        if (doubleEq(_time, 0.0) || _scope==nullptr) return 0;
        double v1= 0.0, v2= 0.0;
        if (!doubleEq(_startSphere, 0.0)) v1= volumeSphereCap(_startSphere, sagita(_startSphere, getScopeDiameter(_scope)));
        if (!doubleEq(_endSphere, 0.0)) v2= volumeSphereCap(_endSphere, sagita(_endSphere, getScopeDiameter(_scope)));
        return (v2-v1)/_time;
    }
    QList<QString> Ignored() { QList<QString> l; l.append("scope"); return l; }
    Q_INVOKABLE void setEndSphereSpherometer(double v)
    {
        if (_scope==nullptr) return;
        double s= v/2.0 + getScopeSpherometerLegDistances(_scope)*getScopeSpherometerLegDistances(_scope)/(6.0*v);
        QString out;
        QStringList l= _comments.split("\n");
        for (int i=0; i<l.count(); i++) if (!l.at(i).startsWith("value from sperometer reading")) out= out+l.at(i)+"\n";
        setComments(out+"value from sperometer reading of: "+QString::number(v));
        setEndSphere(floor(s));
    }
};

class CBSDouble : public CBSSaveLoadObject {
    Q_OBJECT
public:
    CBSPropD(double, val, Val)
Q_SIGNALS:
    void valChanged();
public:
    CBSDouble(QObject *parent=nullptr): CBSSaveLoadObject(parent), _val(0.0) { }
};

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
        _doubleVal= 0.0; double nb= 0;
        foreach(QString const &s, l)
        {
            bool ok; double t= s.toDouble(&ok); if (ok) { _doubleVal+= t; nb++; }
        }
        return _doubleVal/= nb;
    }
};

class CBSModelParabolizingWork : public CBSSaveLoadObject {
    Q_OBJECT
public:
    CBSPropD(double, time, Time)
    CBSProp(QString, comments, Comments)
    QML_OBJMODEL_PROPERTY(CBSQString, mesures)
    CBSPropE(CBSModelScope*, scope, Scope, doCalculations(); QObject *v2= (QObject*)v;
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
      iNbZone(0), _surf(nullptr), _profil(nullptr), _deltaC(nullptr), _lf1000(nullptr), _mesc(nullptr), _Hz(nullptr), _Hm4F(nullptr), _RelativeSurface(nullptr),
      _Std(0.0), _Lambda(0.0), _GlassMax(0.0), _WeightedLambdaRms(0.0), _WeightedStrehl(0.0), _LfRoMax(0.0), _glassToRemove(0.0), _focale(0.0)
    { m_mesures= new QQmlObjectListModel<CBSQString>(this); }
    ~CBSModelParabolizingWork()
    {
        delete m_mesures;
        if (_surf  !=nullptr) delete[] _surf  ;
        if (_profil!=nullptr) delete[] _profil;
        if (_deltaC!=nullptr) delete[] _deltaC;
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
    bool subSave(QJsonObject *o) { return saveList(m_mesures, "mesures", o); }
    QList<QString> Ignored() { QList<QString> l; l.append("scope"); return l; }
public slots:
    void checkNbMesures();
    void doCalculations();
private:
    double find_minimum(double a,double c,double res,double (*fcn)(CBSModelParabolizingWork *self,double h));
    static double calc_lf1000(CBSModelParabolizingWork *self, double h);
    static double calc_less_rms(CBSModelParabolizingWork* self,double curv);
    static double calc_less_ptv(CBSModelParabolizingWork* self,double curv);
    friend void CBScopeMesure::paint(QPainter *painter);
    int iNbZone; // applies to _surf
    double *_surf  ;
    double *_profil;
    double *_deltaC;
    double *_lf1000;
    double *_mesc  ;
    double *_Hz  ;
    double *_Hm4F  ;
    double *_RelativeSurface;
    double _Std;
    double _Lambda;
    double _GlassMax;
    double _WeightedLambdaRms;
    double _WeightedStrehl;
    double _LfRoMax;
    double _glassToRemove;
    double _focale;
};

class CBSModelEP : public CBSSaveLoadObject {
    Q_OBJECT
public:
    CBSProp(QString, name, Name)
    CBSPropDE(double, focal, Focal, emit fieldChanged(); ) // TODO: add emit to higher level to repaint ilumination!
    CBSPropDE(double, angle, Angle, emit fieldChanged(); ) // TODO: add emit to higher level to repaint ilumination!
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
    QList<QString> Ignored() { QList<QString> l; l.append("scope"); return l; }
};

class CBSModelScope : public CBSSaveLoadObject {
    Q_OBJECT
public:
    CBSProp(QString, name, Name)
    CBSProp(QString, comments, Comments)
    CBSProp(QString, secondariesToConcider, SecondariesToConcider)
    CBSPropDE(double, diametre, Diametre, emit nbZonesSuggestedChanged(); emit leftToHogChanged(); emit toHogChanged(); emit sagitaChanged(); emit hogTimeWithGritChanged())
    CBSPropDE(double, focal, Focal, emit nbZonesSuggestedChanged(); emit leftToHogChanged(); emit toHogChanged(); emit sagitaChanged(); emit hogTimeWithGritChanged(); )
    CBSPropDE(double, secondary, Secondary, emit nbZonesSuggestedChanged(); emit secondaryOffsetChanged())
    CBSPropDE(double, secondaryToFocal, SecondaryToFocal, emit nbZonesSuggestedChanged(); emit secondaryOffsetChanged())
    CBSPropD(double, spherometerLegDistances, SpherometerLegDistances)
    Q_PROPERTY(int nbZonesSuggested READ getNbZonesSuggested NOTIFY nbZonesSuggestedChanged)
    CBSPropDE(double, excludedAngle, ExcludedAngle, emit nbZonesSuggestedChanged())
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
public slots:
    void emitEpsChanged() { emit epsChanged(); }
    void emitHogTimeWithGritChanged() { emit hogTimeWithGritChanged(); }
public:
    CBSModelScope(QObject *parent=nullptr): CBSSaveLoadObject(parent), _secondariesToConcider("19 25 35 50 63 70 80 88 100"),
                           _diametre(150), _focal(750), _secondary(35), _secondaryToFocal(150/2+80), _spherometerLegDistances(56),
                           _excludedAngle(2.0), _slitIsMoving(true)
    {
        m_hoggings= new QQmlObjectListModel<CBSModelHoggingWork>(this);
        m_parabolizings= new QQmlObjectListModel<CBSModelParabolizingWork>(this);
        m_eps= new QQmlObjectListModel<CBSModelEP>(this);
        m_zones= new QQmlObjectListModel<CBSDouble>(this);
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
    Q_INVOKABLE void removeSpherometer(int i)
    {
        if (i<0 || i>=m_hoggings->count()) return;
        m_hoggings->remove(i);
        emit hogTimeWithGritChanged();
    }
    Q_INVOKABLE CBSModelHoggingWork *getSpherometer(int i)
    {
        if (i<0 || i>=m_hoggings->count()) return nullptr;  return m_hoggings->at(i);
    }
    Q_INVOKABLE int addEp(QString name, double focal, double angle)
    {
        CBSModelEP *e= new CBSModelEP(m_eps); e->setFocal(focal); e->setAngle(angle); e->setName(name); m_eps->append(e); e->setScope(this);
        return m_eps->count()-1;
    }
    Q_INVOKABLE int deleteEp(int index) { if (index<0 || index>=m_eps->count()) return index; m_eps->remove(index); emit epsChanged(); return m_eps->count()-1; }
    Q_INVOKABLE double getHogTimeWithGrit()
    {
        if (m_hoggings->count()==0) return 0.0;
        double grit= m_hoggings->last()->_grit;
        int counts= 0; double sum= 0.0;
        for (int i=0; i<m_hoggings->count(); i++)
            if (doubleEq(grit, m_hoggings->at(i)->_grit))
            { counts++; sum+= m_hoggings->at(i)->getHogSpeed(); }
        if (counts==0) return 0.0;
        return getLeftToHog()*counts/sum;
    }
    double getToHog() { return volumeSphereCap(_focal*2, ::sagita(_focal*2, _diametre)); }
    double getSagita() { return ::sagita(_focal*2, _diametre); }
    double getLeftToHog()
    {
        if (m_hoggings->count()==0) return 0.0;
        double current= m_hoggings->at(m_hoggings->count()-1)->_endSphere;
        double hogged= volumeSphereCap(current, ::sagita(current, _diametre));
        return getToHog()-hogged;
    }
    double getSecondaryOffset() { return -forwardOffsetForSecondary(_secondary, _secondaryToFocal); }
    int getNbZonesSuggested()
    {
        //double n = (1-exc) * _diametre *_diametre / (16 * sqrt( Rmm*Rmm*Rmm * kmm ));
        double exclude= tan(_excludedAngle/180*M_PI)*(_focal-_secondaryToFocal)*2-_secondary;
        if (exclude<0.0) exclude= 0.0;
        double n = (_diametre-exclude) *_diametre / (16 * sqrt( _focal*_focal*_focal * 8.0 * 37e-6 )); //37e-6= 1/15 wavelength, in mm
        if (n-floor(n)<0.3) return int(n); else return int(n)+1;
    }
    int getNbZones() { int r= m_zones->count(); if (r==0) return 0; return r-1; }
    void calcZones(int d, int e)
    {
        int nb= e-d; if (nb<=1) return; // nothing to calcualte
        double rd= m_zones->at(d)->_val, re= m_zones->at(e)->_val;
        double surfaceStart= rd*rd*M_PI;
        double surfaceInc= (re*re*M_PI-surfaceStart)/nb;
        d++; while (d<e)
        {
            surfaceStart+= surfaceInc;
            m_zones->at(d++)->setVal(int(sqrt(surfaceStart/M_PI)*100)/100.0);
        }
    }
    void validateZones()
    {
        int d= 0, e= m_zones->count()-1;
        double fd= 0.0, fe= _diametre/2.0;
        if (e<4) { while (m_zones->count()!=4) m_zones->append(new CBSDouble(m_zones)); e= 4; }
        m_zones->at(e)->setVal(fe);
        for (int i= m_zones->count(); --i>=0;)
            if (m_zones->at(0)->_val<0.0) m_zones->at(i)->setVal(0.0);
            else if (m_zones->at(0)->_val>_diametre/2.0) m_zones->at(i)->setVal(_diametre/2.0);
        while (d+1<e && m_zones->at(d+1)->_val>fd && m_zones->at(d+1)->_val<fe) { fd= m_zones->at(d)->_val; d++; }
        while (d<e-1 && m_zones->at(e-1)->_val<fe && m_zones->at(e-1)->_val>fd) { fe= m_zones->at(e)->_val; e--; }
        if (d<e-1) calcZones(d, e);
    }
    void setNbZones(int v)
    {
        if (v==m_zones->count()-1 || v<3) return;
        while (v>m_zones->count()-1) m_zones->append(new CBSDouble(m_zones));
        while (v<m_zones->count()-1) m_zones->remove(m_zones->count()-1);
        validateZones();
        emit nbZonesChanged();
    }
    Q_INVOKABLE void autoZones()
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
    Q_INVOKABLE CBSModelParabolizingWork *getParabolizing(int index)
    { if (index<0 || index>=m_parabolizings->count()) return nullptr; return m_parabolizings->at(index); }
    Q_INVOKABLE void removeMesure(int i) { m_parabolizings->remove(i); }
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
    bool subSave(QJsonObject *o)
    {
        saveList(m_hoggings, "hoggings", o);
        saveList(m_parabolizings, "parabolizings", o);
        saveList(m_zones, "zones", o);
        saveList(m_eps, "eps", o);
        return true;
    }
    double getConical() { return -1.0; }
};

QString getAppPath();

class CBSModel : public CBSSaveLoadObject {
    Q_OBJECT
public:
    CBSProp(int, windowsPosX, WindowsPosX)
    CBSProp(int, windowsPosY, WindowsPosY)
    CBSProp(int, windowsWidth, WindowsWidth)
    CBSProp(int, windowsHeight, WindowsHeight)
    CBSProp(int, windowsFlags, WindowsFlags)
    QML_OBJMODEL_PROPERTY(CBSModelScope, scopes)
Q_SIGNALS:
    void windowsPosXChanged();
    void windowsPosYChanged();
    void windowsWidthChanged();
    void windowsHeightChanged();
    void windowsFlagsChanged();
public:
    CBSModel(QObject *parent=nullptr): CBSSaveLoadObject(parent)
    {
        m_scopes= new QQmlObjectListModel<CBSModelScope>(this);
        loadFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)+"/CBScopes.scopes"); // Create file
        qDebug() << "end of load";
        if (m_scopes->count()==0) // If nothing, create default scope
        { CBSModelScope *s= new CBSModelScope(m_scopes); s->setName("Default"); m_scopes->append(s); }
        singleton= this;
    }

    ~CBSModel() { delete m_scopes; }
      static CBSModel *singleton;
    static QObject *SingletonProvider(QQmlEngine *engine, QJSEngine *scriptEngine) // for Qml link...
    {
        (void)engine; (void)scriptEngine;
        if (singleton==nullptr) new CBSModel();
        return singleton;
    }
    Q_INVOKABLE CBSModelScope *getScope(int index) { return m_scopes->at(index); }
    Q_INVOKABLE int addScope() { CBSModelScope *s= new CBSModelScope(m_scopes); s->setName("Default"); m_scopes->append(s); return m_scopes->count()-1; }
    Q_INVOKABLE void removeScope(int index) { return m_scopes->remove(index); }
    void saveFile() { CBSSaveLoadObject::saveFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)+"/CBScopes.scopes"); }
    bool subLoad(QJsonObject *o) { return loadList(m_scopes, "scopes", o); }
    bool subSave(QJsonObject *o) { return saveList(m_scopes, "scopes", o); }
    Q_INVOKABLE void help()
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(getAppPath()+"/SBScope help.odt"));
    }
};

class CBScopeIlumination : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(CBSModelScope *scope READ getScope WRITE setScope)
    public:
    CBScopeIlumination(QQuickItem *parent = nullptr): QQuickPaintedItem(parent), scope(nullptr) { }
    void paint(QPainter *painter);
    CBSModelScope *scope;
    CBSModelScope *getScope() { return scope; }
    void setScope(CBSModelScope *scope) { this->scope= scope; }

    // members needed by the get functions that will be filled at paint time
        int w, h, addx, addy;
        double rw;
    int getX(double x) { x+=rw/2.0; return addx+int(x/rw*0.8*w); }
    int getY(double y) { y= 1-y; return int(y*2.0*0.8*h)+addy; }
    QPoint getP(double x, double y) { return QPoint(getX(x), getY(y)); }
};
#endif
