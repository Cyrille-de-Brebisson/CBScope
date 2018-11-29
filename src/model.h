//   tab navigation
//   indicate ROC offset and allow to fix other parabolization on that offset!
//   find stroke that helped last time...
//   add ideal readings in parabolizing/zones...
//   Put camera on loader if mobile complains...
//   current items on list views...
//   save pictures in scopes?

#ifndef CBSMODEL_H
#define CBSMODEL_H

#include <cmath>
#define M_PI 3.14159265358979323846264338327
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
#include <QAbstractVideoFilter>
#include <QBasicTimer>

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
        bool ret= loadProperties(&o); // Load the json object into our system
        loadProperties(nullptr); // re-enable all signals!
        return ret;
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

    virtual bool subLoad(QJsonObject *o) { if (o==nullptr) blockSignals(false); return true; } // override these if you have more to load/save than read/write Q_PROPERTIES
    virtual bool subSave(QJsonObject *o) const { (void)o; return true; } // override these if you have more to load/save than read/write Q_PROPERTIES
    virtual QList<QString> Ignored() const { return QList<QString>(); }  // override to provide a list of properties to ignore!

    bool loadProperties(QJsonObject *j) // Load all the properties from j into this.
    {
        if (j!=nullptr)
        {
            blockSignals(true);
            const QMetaObject *metaobject = metaObject();
            int count = metaobject->propertyCount();                   // For each property in this
            QList<QString> ignore(Ignored());
            for (int i=1; i<count; ++i) {                              // skip object name (which is the first property)
                QMetaProperty metaproperty = metaobject->property(i);  // get the property accessor
                if (!metaproperty.isWritable()) continue;              // only works on read/write properties
                if (ignore.indexOf(metaproperty.name())!=-1) continue; // to ignore
                QJsonValue v= j->take(metaproperty.name());            // and and remove the associated object from json
                if (v.isUndefined()) { qDebug() << "could not find" << metaproperty.name() << "in" << *j; continue; } // if not in jason, do nothing
                //qDebug() << "loading " << metaproperty.name() << "=" << v.toVariant();
                bool ok= metaproperty.write(this, v.toVariant());      // save json value into this property
                (void)ok;
            }
        }
        return subLoad(j);          // load all the stuff that can not be done automatically
    }
    // helper function to load lists of SaveLoadObjects
    // assumes that j.property is an array of object of type T. load said array in list d
    template <typename T> bool loadList(QQmlObjectListModel<T> *d, char const *property, QJsonObject *j)
    {
        d->clear();        // clear current list content
        //qDebug() << "loading list " << property;
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


// adjusts offset for field diameter by using the diagonal to eye distance;
// since diagToEyeDistance > diagToFocalPlaneDistance, the offset will shrink as the diagonal appears closer to the primary mirror;
// from http://www.telescope-optics.net/newtonian.htm
static double inline forwardOffsetForSecondary(double diagMinorAxis, double diagToEyeDistance) { return -diagMinorAxis * diagMinorAxis / 4.0 / diagToEyeDistance; }

// macros to simplify property definitions...
// Each time you define a property, you need to enter at least 5 lines of code...
// with these macros, you only need 2, which is much better!
// To use the macro, you do: CBSProp???(property_type, name, Name, ?)
// Unfortunately, you have to enter the name twice, once with a lowercase first letter and once with an uppercase first letter...
// they are 2 versions of the macro.
// The normal version and the E version. The E version allows you to add code when the variable is changed
// Talking about signals! Since ALL the signals HAVE to be together in Qt, you will still have to type the signal by hand :-(
template <typename T> bool inline doubleEq(T d1, T d2) { return d1==d2; } // return true if the 2 numbers are equal with 10 significant digits
static bool inline doubleEq(double d1, double d2) { return std::abs(d1-d2)<=std::abs((d1+d2)/2.0e10); } // return true if the 2 numbers are equal with 10 significant digits
#define CBSPropE(type, name, Name, emits) \
    Q_PROPERTY(type name READ get##Name WRITE set##Name NOTIFY name##Changed) \
    type _##name; \
public:\
    type get##Name() const { return _##name; } \
    void set##Name(type v) { if (doubleEq(_##name,v)) return; _##name= v; emit name##Changed(); if (!signalsBlocked()) { emits; } }
#define CBSProp(type, name, Name) CBSPropE(type, name, Name, {})



//***************************************************
// Forwards
class CBSModelScope;
double getScopeDiameter(CBSModelScope *scope); // deported to .cpp file to access inner stuff...
double getScopeSpherometerLegDistances(CBSModelScope *scope); // deported to .cpp file to access inner stuff...
double getScopeFocal(CBSModelScope *scope); // deported to .cpp file to access inner stuff...
double getScopeToHog(CBSModelScope *scope); // deported to .cpp file to access inner stuff...
static double inline volumeSphereCap(double r, double sagita) { return M_PI*sagita*sagita/3.0*(3.0*r-sagita); } // return the volume of a part of a sphere
static double inline sagita(double radiusSphere, double radiusDisc) { return radiusSphere-sqrt(radiusSphere*radiusSphere-radiusDisc*radiusDisc/4.0); } // calculate the sagita of disk

//***************************************************
// Represents time spend hogging the mirror (digging the curve)
class CBSModelHoggingWork : public CBSSaveLoadObject {
    Q_OBJECT
public:
    CBSProp(QString, comments, Comments)
    CBSPropE(double, time, Time, emit hogSpeedChanged())                   // time to do this work.
    CBSPropE(double, startSphere, StartSphere, emit hogSpeedChanged(); emit percentDoneChanged();)     // sphere radius at begining of the work
    CBSPropE(double, endSphere, EndSphere, emit hogSpeedChanged(); emit endSagitaChanged(); emit percentDoneChanged(); emit endSphereSpherometerChanged(); ) // sphere radius at end of the work
    CBSProp(double, grit, Grit)                                            // grit that was used
    Q_PROPERTY(double hogSpeed READ getHogSpeed NOTIFY hogSpeedChanged)    // calculated property. Speed of hoggin in mm^3/mn
    Q_PROPERTY(double endSagita READ getEndSagita NOTIFY endSagitaChanged) // based on endSphere
    Q_PROPERTY(double percentDone READ getPercentDone NOTIFY percentDoneChanged) // based on endSphere. But will not get notified if scope data changes. Probably not a big issue at this point in time? Else, add notify in the connect bellow
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
    void percentDoneChanged();
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
        return (v2-v1)/_time; // end-start divided by time!
    }
    double getPercentDone()
    {
        if (_scope==nullptr) return 0.0;
        double v1= 0.0, v2= 0.0;
        if (!doubleEq(_startSphere, 0.0)) v1= volumeSphereCap(_startSphere, sagita(_startSphere, getScopeDiameter(_scope)));
        if (!doubleEq(_endSphere, 0.0)) v2= volumeSphereCap(_endSphere, sagita(_endSphere, getScopeDiameter(_scope)));
        return (v2-v1)/getScopeToHog(_scope)*100.0;
    }
    Q_INVOKABLE void setEndSphereSpherometer(double v) // set the sphere end using spherometer reading...
    {
        if (_scope==nullptr) return;
        double s= v/2.0 + getScopeSpherometerLegDistances(_scope)*getScopeSpherometerLegDistances(_scope)/(6.0*v); // radius = v/2+spherometer_leg_distance²/(6*v). From wikipedia
        QString out;
        QStringList l= _comments.split("\n"); // update comments....
        for (int i=0; i<l.count(); i++) if (!l.at(i).startsWith("value from spherometer reading")) out= out+l.at(i)+"\n";
        setComments(out+"value from spherometer reading of: "+QString::number(v));
        setEndSphere(floor(s));
    }
};

//**********************************************************
// a class that embedds a double and can be used in a model
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
    Q_PROPERTY(QString val READ getVal WRITE setVal NOTIFY valChanged)
    CBSProp(QString, comment, Comment)
    Q_PROPERTY(bool error READ getError NOTIFY errorChanged) // true if the string is not valid!
Q_SIGNALS:
  void valChanged();
  void errorChanged();
  void commentChanged();
public:
  QString _val;
  bool _error;
  CBSQString(QObject *parent=nullptr): CBSSaveLoadObject(parent), _val("0"), _error(false), _doubleVal(0.0) { }
  ~CBSQString() { } // Will this work?
  double _doubleVal;
  QString getVal() { return _val; }
  void setVal(QString v)
  {
    if (v==_val) return;
    _val= v; _doubleVal= nan(""); asDouble();
    emit valChanged();
  }
  bool getError() { return _error; }
  double asDouble() // calcualte the average of the numbers in the string. set error if/as needed
  {
    if (!std::isnan(_doubleVal)) return _doubleVal;
    _error= false;
    QStringList l(_val.split(" ", QString::SkipEmptyParts)); // split string in tokens and look at each token separately
    _doubleVal= 0.0; int nb= 0;
    foreach(QString const &s, l) { 
      bool ok; double t= s.toDouble(&ok); _error|= !ok; 
      if (ok) { _doubleVal+= t; nb++; } 
    }
    if (nb==0) { _error= true; emit errorChanged(); return 0.0; }
    emit errorChanged();
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
    QML_OBJMODEL_PROPERTY(CBSDouble, zones)
public:
    CBSPropE(double, scopeFocale, ScopeFocale, doCalculations())
    CBSPropE(double, scopeDiametre, ScopeDiametre, doCalculations())
    CBSPropE(double, scopeConical, ScopeConical, doCalculations())
    CBSPropE(double, scopeSlitIsMoving, ScopeSlitIsMoving, doCalculations())
Q_SIGNALS:
    void commentsChanged();
    void timeChanged();
    void mesuresChanged();
    void scopeChanged();
    void scopeFocaleChanged();
    void scopeDiametreChanged();
    void scopeConicalChanged();
    void scopeSlitIsMovingChanged();
public:
    CBSModelParabolizingWork(QObject *parent=nullptr): CBSSaveLoadObject(parent), _time(0.0),
      _scopeFocale(0.0), _scopeDiametre(0.0), _scopeConical(-1.0), _scopeSlitIsMoving(true),
      _lf1000(nullptr), _mesc(nullptr), _Hm4F(nullptr), _RelativeSurface(nullptr),
      _surf(nullptr), _profil(nullptr), _Hz(nullptr),
      _Std(0.0), _Lambda(0.0), _GlassMax(0.0), _WeightedLambdaRms(0.0), _WeightedStrehl(0.0), _LfRoMax(0.0), _glassToRemove(0.0), _focale(0.0)
    { 
      m_mesures= new QQmlObjectListModel<CBSQString>(this); 
      m_zones= new QQmlObjectListModel<CBSDouble>(this);
    }
    ~CBSModelParabolizingWork()
    {
        delete m_mesures;
        delete m_zones;
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
        if (o==nullptr) 
        { 
          for (int i=0; i<m_mesures->count(); i++) m_mesures->at(i)->subLoad(nullptr);
          for (int i=0; i<m_zones->count(); i++) m_zones->at(i)->subLoad(nullptr);
          blockSignals(false); return true; 
        }
        bool ret1= loadList(m_mesures, "mesures", o);
        bool ret2= loadList(m_zones, "zones", o);
        for (int i=0; i<m_mesures->count(); i++) connect(m_mesures->at(i), SIGNAL(valChanged()), this, SLOT(doCalculations()));
        for (int i=0; i<m_zones->count(); i++) connect(m_zones->at(i), SIGNAL(valChanged()), this, SLOT(doCalculations()));
        doCalculations();
        return ret1&&ret2;
    }
    bool subSave(QJsonObject *o) const { saveList(m_mesures, "mesures", o); saveList(m_zones, "zones", o); return true; }
    QList<QString> Ignored() const { QList<QString> l; l.append("scope"); return l; }
public slots:
    void doCalculations();
private:
    void checkNbMesures()
    {
        while (m_mesures->count()!=0 && m_mesures->count()>m_zones->count()-1) m_mesures->remove(m_mesures->count()-1);
        while (m_mesures->count()<m_zones->count()-1)
        {
          CBSQString *d= new CBSQString(m_mesures); m_mesures->append(d);
          connect(d, SIGNAL(valChanged()), this, SLOT(doCalculations()));
        }
    }
    double find_minimum(double a,double c,double res,double (*fcn)(CBSModelParabolizingWork *self,double h));
    static double calc_lf1000(CBSModelParabolizingWork *self, double h);
    static double calc_less_rms(CBSModelParabolizingWork* self,double curv);
    static double calc_less_ptv(CBSModelParabolizingWork* self,double curv);
    double *_lf1000;
    double *_mesc  ;
    double *_Hm4F  ;
    double *_RelativeSurface;
public:
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
    CBSPropE(double, focal, Focal, emit fieldChanged(); emit zoomChanged(); emit pupilleChanged(); emit viewAngleChanged())
    CBSPropE(double, angle, Angle, emit fieldChanged(); emit getViewAngle())
    Q_PROPERTY(double field READ getField NOTIFY fieldChanged)             
    Q_PROPERTY(double zoom READ getZoom NOTIFY zoomChanged)                // These do depend on the scope parameters
    Q_PROPERTY(double pupil READ getPupille NOTIFY pupilleChanged)         // These do depend on the scope parameters
    Q_PROPERTY(double viewAngle READ getViewAngle NOTIFY viewAngleChanged) // These do depend on the scope parameters
    CBSPropE(CBSModelScope*, scope, Scope, QObject *v2= (QObject*)v;
                                           connect(this, SIGNAL(nameChanged()), v2, SLOT(emitEpsChanged()));     // scope needs to know to redraw illumination
                                           connect(this, SIGNAL(fieldChanged()), v2, SLOT(emitEpsChanged()));    // scope needs to know to redraw illumination
                                           connect(v2, SIGNAL(focalChanged()), this, SLOT(ScopeFocalChanged()));
                                           connect(v2, SIGNAL(diametreChanged()), this, SLOT(ScopeFocalChanged())); )
Q_SIGNALS:
    void nameChanged();
    void focalChanged();
    void angleChanged();
    void fieldChanged();
    void viewAngleChanged();
    void zoomChanged();
    void scopeChanged();
    void pupilleChanged();
public slots:
    void ScopeFocalChanged() { emit viewAngleChanged(); emit zoomChanged(); emit pupilleChanged(); }
public:
    CBSModelEP(QObject *parent=nullptr): CBSSaveLoadObject(parent), _focal(20), _angle(60), _scope(nullptr) { }
    ~CBSModelEP() { }
    double getField() const { return 2*_focal*tan(_angle/2/180*M_PI); }
    double getZoom() { if (_scope==nullptr) return 0.0; return getScopeFocal(_scope)/_focal; }
    double getViewAngle() { if (_scope==nullptr) return 0.0; return getField()/getScopeFocal(_scope)*3456.0/60.0; } // { if (_scope==nullptr) return 0.0; return _angle/(getScopeFocal(_scope)/_focal); }  // 3456.0/60.0*fx/_scope->_focal
    double getPupille() { if (_scope==nullptr) return 0.0; return getScopeDiameter(_scope)/getZoom(); }
    QList<QString> Ignored() const { QList<QString> l; l.append("scope"); return l; }
};

//******************************************************
// Model object for one telescope. Mirror, zones, eyepeices, hoggin work....
class CBSModelScope : public CBSSaveLoadObject {
    Q_OBJECT
public:
    CBSProp(QString, name, Name)
    CBSProp(QString, comments, Comments)
    CBSProp(QString, secondariesToConcider, SecondariesToConcider) // for illumination  calculations
    CBSPropE(double, diametre, Diametre, emit nbZonesSuggestedChanged(); emit weightChanged(); emit leftToHogChanged(); emit toHogChanged(); emit sagitaChanged(); emit hogTimeWithGritChanged())
    CBSPropE(double, thickness, Thickness, emit weightChanged())
    CBSPropE(double, density, Density, emit weightChanged())
    Q_PROPERTY(double weight READ getWeight NOTIFY weightChanged)
    CBSProp(double, young, Young)
    CBSProp(double, poisson, Poisson)
    CBSPropE(double, focal, Focal, emit nbZonesSuggestedChanged(); emit leftToHogChanged(); emit toHogChanged(); emit sagitaChanged(); emit hogTimeWithGritChanged())
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

	// virtual Couder/Ronchi stuff
    CBSProp(double, couderx, Couderx)
    CBSProp(double, coudery, Coudery)
    CBSProp(double, couderz, Couderz)
	CBSProp(int, zone, Zone)                                           // zone being couder analyzed at the moment
	QML_OBJMODEL_PROPERTY(CBSQString, zoneModel)                       // list of the zones
	CBSProp(bool, pause, Pause)                                        // pause webcam streaming
	CBSProp(bool, ronchi, Ronchi)                                      // display ronchi
	CBSProp(double, ronchiOffset, RonchiOffset)                        // out of focus for Ronchi
	CBSProp(double, grading, Grading)                                  // 3.9 = 100 dpi or so

	// Couder screen
	CBSProp(bool, showCouderRed,  ShowCouderRed)   // normal zones style display
	CBSProp(bool, showCouderBlue, ShowCouderBlue) // my zone style display
    CBSProp(bool, showCouderOrange, ShowCouderOrange) // my zone style display
    CBSProp(int, virtualCouderType, VirtualCouderType) // 0: Grayscale, 1: RGB/3, 2:R, 3:G, 4:B
    // Properties for COG pages
    QML_OBJMODEL_PROPERTY(CBSQString, bottomWeights)
    QML_OBJMODEL_PROPERTY(CBSQString, topWeights)
    CBSPropE(double, cogTopLen, CogTopLen, emitcogWeightChanged())
    CBSPropE(double, cogBotLen, CogBotLen, emitcogWeightChanged())
    CBSPropE(double, cogWeight, CogWeight, emitcogWeightChanged())
    Q_PROPERTY(double cogTopWeights READ getCogTopWeights NOTIFY cogTopWeightsChanged)
    Q_PROPERTY(double cogBottomWeights READ getCogBottomWeights NOTIFY cogBottomWeightsChanged)
    Q_PROPERTY(double cog READ getCog NOTIFY cogChanged)
    CBSProp(QString, notes, Notes)
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
    void youngChanged();
    void poissonChanged();
    void weightChanged();
    void couderxChanged();
    void couderyChanged();
    void couderzChanged();
    void showCouderRedChanged();
    void showCouderBlueChanged();
    void showCouderOrangeChanged();
    void virtualCouderTypeChanged();
    void cogBotLenChanged();
    void cogTopLenChanged();
    void cogWeightChanged();
    void cogTopWeightsChanged();
    void cogBottomWeightsChanged();
    void cogChanged();
    void notesChanged();
	void zoneChanged();
	void scopeChanged();
	void pauseChanged();
	void ronchiChanged();
	void ronchiOffsetChanged();
	void gradingChanged();
public slots:
    void emitEpsChanged() { emit epsChanged(); }
    void emitHogTimeWithGritChanged() { emit hogTimeWithGritChanged(); }
    void emitcogWeightChanged() { emit cogChanged(); emit cogBottomWeightsChanged(); emit cogTopWeightsChanged(); }
    void emitZoneChanged() { emit nbZonesChanged(); }
	void rezone()
	{
		m_zoneModel->clear();
		for (int i=0; i<get_zones()->count()-1; i++)
		{
			CBSQString *s= new CBSQString(m_zoneModel); s->setVal(QString("Z")+QString::number(i+1)+QString(" ")+QString::number(get_zones()->at(i)->_val)+"-"+QString::number(get_zones()->at(i+1)->_val));
			m_zoneModel->append(s);
		}
	}
public:
    CBSModelScope(QObject *parent=nullptr): CBSSaveLoadObject(parent), _secondariesToConcider("19 25 35 50 63 70 80 88 100"),
                           _diametre(150), _thickness(25), _density(2.23), _young(6400.0), _poisson(0.2), _focal(750), _secondary(35), _secondaryToFocal(150/2+80), _spherometerLegDistances(56),
                           _excludedAngle(2.0), _slitIsMoving(true), _cellType(0),
                           _couderx(.5), _coudery(.5), _couderz(.80), _pause(false), _ronchi(false), _ronchiOffset(0.0), _grading(3.9), _zone(0),
						   _showCouderRed(true), _showCouderBlue(true),  _showCouderOrange(false), _virtualCouderType(0),
                           _cogTopLen(100.0), _cogBotLen(50.0), _cogWeight(1.0)
    {
        m_hoggings= new QQmlObjectListModel<CBSModelHoggingWork>(this);
        m_parabolizings= new QQmlObjectListModel<CBSModelParabolizingWork>(this);
        m_eps= new QQmlObjectListModel<CBSModelEP>(this);
        m_zones= new QQmlObjectListModel<CBSDouble>(this);
        m_bottomWeights= new QQmlObjectListModel<CBSQString>(this);
        m_topWeights= new QQmlObjectListModel<CBSQString>(this);
		m_zoneModel= new QQmlObjectListModel<CBSQString>(this);
		setNbZones(getNbZonesSuggested()); autoZones();
	}
    ~CBSModelScope() {
        delete m_hoggings;  
        delete m_parabolizings;
        delete m_zones;
        delete m_eps;
        delete m_bottomWeights;
        delete m_topWeights;
		delete m_zoneModel;
    }

    // Various small getters
    static int const _conical= -1.0;            // Used by parabolization hard set to -1 at the moment...
    double getSagita() { return ::sagita(_focal*2, _diametre); }                                      // get mirror sagita
    double getWeight() { return _diametre*_diametre/4.0*M_PI*_thickness*_density/1e6; }               // weight of the mirror...
    double getSecondaryOffset() { return -forwardOffsetForSecondary(_secondary, _secondaryToFocal); } // secondary offset toward primary
    double getToHog() { return volumeSphereCap(_focal*2, ::sagita(_focal*2, _diametre)); } // Total amount of material to hog for the mirror
    double getLeftToHog()                                                                  // amount of material left to hog to complete the mirror
    {
      if (m_hoggings->count()==0) return 0.0;
      double current= m_hoggings->at(m_hoggings->count()-1)->_endSphere;                 // assume last hogging is the deepest. get the sphere diameter
      double hogged= volumeSphereCap(current, ::sagita(current, _diametre));             // calculate how much was hogged
      return getToHog()-hogged;                                                          // substract from total to hog
    }
    int getNbZonesSuggested() // return suggested amount of zones. Found the formula online
    {
      //double n = (1-exc) * _diametre *_diametre / (16 * sqrt( Rmm*Rmm*Rmm * kmm ));
      double exclude= tan(_excludedAngle/180*M_PI)*(_focal-_secondaryToFocal)*2-_secondary;
      if (exclude<0.0) exclude= 0.0;
      double n = (_diametre-exclude) *_diametre / (16 * sqrt( _focal*_focal*_focal * 8.0 * 37e-6 )); //37e-6= 1/15 wavelength, in mm
      if (n-floor(n)<0.3) return int(n); else return int(n)+1;
    }
    int getNbZones() { int r= m_zones->count(); if (r==0) return 0; return r-1; } // return the number of zones...
    double getHogTimeWithGrit() // return the amount of time needed to finish the hogging with the latest used grit
    {
      if (m_hoggings->count()==0) return 0.0;
      double grit= m_hoggings->last()->_grit; // Get the last used grit
      int counts= 0; double sum= 0.0;         // summ the hogging speed of all the hog works that used the current grit
      for (int i=0; i<m_hoggings->count(); i++)
          if (doubleEq(grit, m_hoggings->at(i)->_grit)) { counts++; sum+= m_hoggings->at(i)->getHogSpeed(); }
      if (counts==0) return 0.0;              // no items, return 0
      return getLeftToHog()*counts/sum;       // estimate hog time using current average hog speed
    }


    ///////////////////////////////////////////////////////////
    Q_INVOKABLE int addEp(QString name, double focal, double angle) // new eye peice
    {
        CBSModelEP *e= new CBSModelEP(m_eps); e->setFocal(focal); e->setAngle(angle); e->setName(name); m_eps->append(e); e->setScope(this);
        emit epsChanged();
        return m_eps->count()-1;
    }
    Q_INVOKABLE int addHogging() // add a hogging work...
    {
        CBSModelHoggingWork *t= new CBSModelHoggingWork(m_hoggings);
        if (m_hoggings->count()!=0) { t->_startSphere= m_hoggings->last()->_endSphere; t->_grit= m_hoggings->last()->_grit; } // copy stuff from last hogging if present
        m_hoggings->append(t);
        t->setScope(this);
        emit hogTimeWithGritChanged();
        return m_hoggings->count()-1;
    }
    Q_INVOKABLE int addParabolizing() // create new parabolizing work
    {
      CBSModelParabolizingWork *p= new CBSModelParabolizingWork(m_parabolizings);
      p->setScopeConical(_conical); p->setScopeDiametre(_diametre); p->setScopeFocale(_focal); p->setScopeSlitIsMoving(_slitIsMoving);
      QList<CBSDouble*> l;
      for (int i=0; i<m_zones->count(); i++) { CBSDouble *s2= new CBSDouble(p->get_zones()); s2->setVal(m_zones->at(i)->getVal()); l.append(s2); } // Copy my zones to the parabolizing work.
      p->get_zones()->append(l); 
      m_parabolizings->append(p); p->doCalculations(); // force calcs. Might not even be needed...
      return m_parabolizings->count()-1;
    }



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
            if (e<3) { while (m_zones->count()!=3) m_zones->append(new CBSDouble(m_zones)); e= 3; } // make sure we have at least 3 zones
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
        while (v>m_zones->count()-1) 
        {
          CBSDouble *d= new CBSDouble(m_zones); connect(d, SIGNAL(valChanged()), this, SLOT(emitZoneChanged()));
          m_zones->append(d);
        }
        while (v<m_zones->count()-1) m_zones->remove(m_zones->count()-1);
        validateZones();
		rezone();
		emit nbZonesChanged();
    }
    Q_INVOKABLE void autoZones() // auto calc zones
    {
        while (m_zones->count()<4) setNbZones(4);
        m_zones->at(m_zones->count()-1)->setVal(_diametre/2.0);
        calcZones(0, m_zones->count()-1);
        emit nbZonesChanged();
    }
      void paintCouder(QPainter *painter, QPoint &c, double dpi, bool showRed, bool showBlue, bool showOrange); // Draw a couder mask
    Q_INVOKABLE void printCouder(); // Print the couder mask
	Q_INVOKABLE void email(); // send the scope by email...

    //////////////////////////////////////////////////////////
    // Center of Gravity stuff
    double secondaryThick(double sec)
    {
      if (sec<26.0) return 5;
      if (sec<37.0) return 6;
      if (sec<=79.0) return 12;
      if (sec<=110.0) return 18.7;
      return 25.0;
    }
    double getCogTopWeights() { 
        if (m_topWeights->count()==0) { CBSQString *s=new CBSQString(m_topWeights); s->setVal(QString::number(_secondary*_secondary*sqrt(2.0)*_density*1e-6*secondaryThick(_secondary))); s->setComment(("Secondary")); m_topWeights->append(s); }
        double a= 0.0;  for (int i=0; i<m_topWeights->count(); i++) a+= m_topWeights->at(i)->asDouble(); 
        return a; 
    }
    double getCogBottomWeights() { 
        if (m_bottomWeights->count()==0) { CBSQString *s=new CBSQString(m_bottomWeights); s->setVal(QString::number(getWeight())); s->setComment(("primary")); m_bottomWeights->append(s); }
        double a= 0.0;  for (int i=0; i<m_bottomWeights->count(); i++) a+= m_bottomWeights->at(i)->asDouble(); 
        return a; 
    }
    Q_INVOKABLE int addTopWeight() 
    { 
      CBSQString *s=new CBSQString(m_topWeights); m_topWeights->append(s); 
      connect(s, SIGNAL(valChanged()), this, SLOT(emitcogWeightChanged()));
      return m_topWeights->count()-1; 
    }
    Q_INVOKABLE int addBottomWeight() 
    { 
      CBSQString *s=new CBSQString(m_bottomWeights); 
      m_bottomWeights->append(s); 
      connect(s, SIGNAL(valChanged()), this, SLOT(emitcogWeightChanged()));
      return m_bottomWeights->count()-1; 
    }
    double getCog()
    {
      double t= getCogTopWeights(), b= getCogBottomWeights();
      // t*cog +((cog+topLen)/2*w)= (body-cog)*b + (body-cog+botLen)/2*w
      // cog= (2*b*body+body*w+botLen*w-topLen*w)/(2*b+2*t+2*w)
      double body= _focal-_secondaryToFocal;
      double w= _cogWeight;
      double cog= (2*b*body+body*w+_cogBotLen*w-_cogTopLen*w);
      double d2= (2*b+2*t+2*w);
      if (d2==0.0) return 0.0; else return body-(cog/d2);
    }

    //////////////////////////////////////////////////
    // Object load/save
    bool subLoad(QJsonObject *o)
    {
        if (o==nullptr) 
        { 
            for (int i=0; i<m_hoggings->count(); i++) m_hoggings->at(i)->subLoad(nullptr);
            for (int i=0; i<m_parabolizings->count(); i++) m_parabolizings->at(i)->subLoad(nullptr);
            for (int i=0; i<m_eps->count(); i++) m_eps->at(i)->subLoad(nullptr);
            for (int i=0; i<m_zones->count(); i++) m_zones->at(i)->subLoad(nullptr);
            for (int i=0; i<m_bottomWeights->count(); i++) m_bottomWeights->at(i)->subLoad(nullptr);
            for (int i=0; i<m_topWeights->count(); i++) m_topWeights->at(i)->subLoad(nullptr);
            blockSignals(false); return true; 
        }
        loadList(m_hoggings, "hoggings", o);
        loadList(m_zones, "zones", o);
        loadList(m_parabolizings, "parabolizings", o);
        loadList(m_eps, "eps", o);
        loadList(m_bottomWeights, "bottomWeights", o);
        loadList(m_topWeights, "topWeights", o);
        for (int i=0; i<m_bottomWeights->count(); i++) connect(m_bottomWeights->at(i), SIGNAL(valChanged()), this, SLOT(emitcogWeightChanged()));
        for (int i=0; i<m_topWeights->count();    i++) connect(m_topWeights->at(i),    SIGNAL(valChanged()), this, SLOT(emitcogWeightChanged()));
        for (int i=0; i<m_hoggings->count(); i++) m_hoggings->at(i)->setScope(this);
        for (int i=0; i<m_eps->count(); i++) m_eps->at(i)->setScope(this);
        for (int i=0; i<m_zones->count(); i++) connect(m_zones->at(i), SIGNAL(valChanged()), this, SLOT(emitZoneChanged()));
        return true;
    }
    bool subSave(QJsonObject *o) const
    {
        saveList(m_hoggings, "hoggings", o);
        saveList(m_parabolizings, "parabolizings", o);
        saveList(m_zones, "zones", o);
        saveList(m_eps, "eps", o);
        saveList(m_topWeights, "topWeights", o);
        saveList(m_bottomWeights, "bottomWeights", o);
        return true;
    }
	QList<QString> Ignored() const { QList<QString> l; l.append("zoneModel"); return l; }
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
	Q_PROPERTY(QString errMsg READ getErrMsg WRITE setErrMsg NOTIFY errMsgChanged)
	Q_PROPERTY(QString dbgMsg READ getDbgMsg WRITE setDbgMsg NOTIFY dbgMsgChanged)
	Q_PROPERTY(QString warMsg READ getWarMsg WRITE setWarMsg NOTIFY warMsgChanged)
	QML_OBJMODEL_PROPERTY(CBSModelScope, scopes)
Q_SIGNALS:
    void windowsPosXChanged();
    void windowsPosYChanged();
    void windowsWidthChanged();
    void windowsHeightChanged();
    void windowsFlagsChanged();
    void windowsFontChanged();
    void windowsFontBoldChanged();
	void errMsgChanged();
	void dbgMsgChanged();
	void warMsgChanged();
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
    Q_INVOKABLE int addScope(int copyEpIndex) 
    { 
        CBSModelScope *s= new CBSModelScope(m_scopes); s->setName("Default"); m_scopes->append(s);
        if (copyEpIndex<0 || copyEpIndex>=m_scopes->count()) copyEpIndex= 0;
        if (m_scopes->count()>copyEpIndex) // copy last scope EPs...
            for (int i=0; i<m_scopes->at(copyEpIndex)->get_eps()->count();i++)
            {
                CBSModelEP *ep= m_scopes->at(copyEpIndex)->get_eps()->at(i);
                s->addEp(ep->getName(), ep->getFocal(), ep->getAngle());
            }
        return m_scopes->count()-1; 
    }
    void saveFile() { CBSSaveLoadObject::saveFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)+"/CBScopes.scopes"); }
    bool subLoad(QJsonObject *o) 
    { 
      if (o==nullptr) 
      { 
        for (int i=0; i<m_scopes->count(); i++) m_scopes->at(i)->subLoad(nullptr);
        blockSignals(false); return true; 
      }
      return loadList(m_scopes, "scopes", o); 
    }
    bool subSave(QJsonObject *o) const { return saveList(m_scopes, "scopes", o); }
	QList<QString> Ignored() const { QList<QString> l; l.append("errMsg"); l.append("dbgMsg"); l.append("warMsg"); return l; }
    Q_INVOKABLE void help() {
        QDesktopServices::openUrl(QUrl::fromLocalFile(getAppPath()+"/SBScope_help.htm"));
    }
	Q_INVOKABLE int loadScope(QString scope); // load a scope from a definition. returns it's model id
    Q_INVOKABLE double materials(int index, int prop) // list properties of material. constants used by the UI
    {
      double const props[5][3]= {
        { 6400.0, 0.2 , 2.23}, // Pyrex 7740
        { 9100.0, 0.24, 2.48}, // zerodur       
        { 6000.0, 0.22, 2.45}, // plate glass   
        { 7400.0, 0.17, 2.22}, // fused scilica 
        { 6300.0, 0.2 , 2.23}}; // Duran         
      if (index<0 || index>4 || prop<0 || prop>2) return 0.0;
      return props[index][prop];
    }
	QMutex lock;
	QString _errMsg, _dbgMsg, _warMsg;
	QString getErrMsg() { lock.lock(); QString r(_errMsg); lock.unlock(); return r; }
	void setErrMsg(QString v) { lock.lock(); _errMsg= v; lock.unlock(); emit errMsgChanged(); }
	QString getDbgMsg() { lock.lock(); QString r(_dbgMsg); lock.unlock(); return r; }
	void setDbgMsg(QString v) { lock.lock(); _dbgMsg= v; lock.unlock(); emit dbgMsgChanged(); }
	QString getWarMsg() { lock.lock(); QString r(_warMsg); lock.unlock(); return r; }
	void setWarMsg(QString v) { lock.lock(); _warMsg= v; lock.unlock(); emit warMsgChanged(); }
};

//***********************************************
// display component for a couder screen mesure
class CBScopeMesure : public QQuickPaintedItem
{
    Q_OBJECT
public:
    CBSProp(CBSModelParabolizingWork*, mesure, Mesure)
Q_SIGNALS:
    void mesureChanged();
public:
    CBScopeMesure(QQuickItem *parent = nullptr): QQuickPaintedItem(parent), _mesure(nullptr) { }
    void paint(QPainter *painter);
    // members needed by the get functions that will be filled at paint time
        int w, h, addx, addy;
        double rw, rh;
    int getX(double x) { return int(x/rw*(w-2*addx))+addx; }
    int getY(double y) { return int((rh-y)/rh*(h-2*addy))+addy; }
    QPoint getP(double x, double y) { return QPoint(getX(x), getY(y)); }
};

//***********************************************
// display component for field ilumination depending on secondary size
class CBScopeIlumination : public QQuickPaintedItem
{
    Q_OBJECT
public:
    CBSPropE(CBSModelScope*, scope, Scope, update())
    CBSProp(bool, twoInches, TwoInches)
Q_SIGNALS:
    void scopeChanged();
    void twoInchesChanged();
public:
    CBScopeIlumination(QQuickItem *parent = nullptr): QQuickPaintedItem(parent), _scope(nullptr), _twoInches(true) { }
    void paint(QPainter *painter);

    // Members for cartesian to pixel conversions
        int w, h, addx, addy; // populated by paint...
        double rw;
    int getX(double x) { x+=rw/2.0; return addx+int(x/rw*0.8*w); }
    int getY(double y) { y= 1-y; return int(y*2.0*0.8*h)+addy; }
    QPoint getP(double x, double y) { return QPoint(getX(x), getY(y)); }
};

//***************************************
// display component for Couder mask
class CBScopeCouder : public QQuickPaintedItem
{
    Q_OBJECT
public:
    CBSProp(CBSModelScope*, scope, Scope)
    CBSProp(bool, zoom, Zoom)
Q_SIGNALS:
    void scopeChanged();
    void zoomChanged();
public:
    CBScopeCouder(QQuickItem *parent = nullptr): QQuickPaintedItem(parent), _scope(nullptr), _zoom(false) { }
    void paint(QPainter *painter);
};

//***************************************
// display component that draws Cell designs info
// For the moment, displays finite element method mesh
class CBScopeMes : public QQuickPaintedItem
{
    Q_OBJECT
public:
    Q_PROPERTY(CBSModelScope *scope READ getScope WRITE setScope NOTIFY scopeChanged);
    CBSModelScope *_scope;
	CBSPropE(bool, showForces, ShowForces, update())
	CBSPropE(bool, showMesh, ShowMesh, update())
	CBSPropE(bool, showParts, ShowParts, update())
	CBSPropE(bool, showSupports, ShowSupports, update())
	CBSPropE(bool, showSecondary, ShowSecondary, update())
	CBSProp(QString, parts, Parts)
	CBSPropE(double, zoom, Zoom, update())
	CBSProp(double, matrixProgresses, MatrixProgresses)
	CBSProp(double, errRms, ErrRms)
	CBSProp(double, errPV, ErrPv)
	CBSProp(double, refocusFL, RefocusFL)
	CBSProp(int, nbEvals, NbEvals)
	CBSProp(double, stepSize, StepSize)
	Q_PROPERTY(bool calc READ getCalc NOTIFY calcChanged) // true if calculating!
Q_SIGNALS:
    void scopeChanged();
	void showForcesChanged();
	void showMeshChanged();
	void showPartsChanged();
	void zoomChanged();
	void nbEvalsChanged();
	void stepSizeChanged();
	void showSupportsChanged();
	void matrixProgressesChanged();
	void errRmsChanged();
	void errPVChanged();
	void refocusFLChanged();
	void calcChanged();
	void showSecondaryChanged();
	void partsChanged();
private:
	bool hasPlopInit;
	bool hasCalculated;
	double _radius, _secondary;
	int _cellType;
public:
	CBScopeMes(QQuickItem *parent = nullptr): QQuickPaintedItem(parent), _scope(nullptr), _showForces(true), _showMesh(true), 
		_showParts(true), _showSupports(true), _showSecondary(true), _zoom(1.0), _matrixProgresses(0.0), _errRms(0.0), _errPV(0.0), _refocusFL(0.0),
		_nbEvals(-1), _stepSize(0), hasPlopInit(false), hasCalculated(false), _radius(0.0), _secondary(0.0), _cellType(0)
	{ }
    void paint(QPainter *painter);
    CBSModelScope *getScope() { return _scope; }
    void setScope(CBSModelScope *s)
    {
      if (s==_scope) return; _scope= s;
      emit scopeChanged();
      emit update();
    }
	void createMirror(double diametre, double secondary, double thickness, double young, double poisson, double focale, double density, int _cellType);
	Q_INVOKABLE void doMesSolve() 
	{ 
		if (_scope==nullptr) return;
		createMirror(_scope->getDiametre(), _scope->getSecondary(), _scope->getThickness(), _scope->getYoung(), _scope->getPoisson(), _scope->getFocal(), _scope->getDensity(), _scope->getCellType()); 
	}
	Q_INVOKABLE void doMesStop();
	bool getCalc();
	QBasicTimer timer;
	void timerEvent(QTimerEvent *event) 
	{ 
		update(); 
		if (!getCalc()) timer.stop();
	}
};

//***************************************
// filter component for webcam for virtual couder
  class CBVirtualCouderOverlayInternal
  {
	  uint32_t *ronchi; int ronchisize;
	  double diam, roc, grad, off;
	  int imagew, imageh; // these get set in the CBScopeVirtualCouderRunnable::run function...
	  bool inverted;
  public:
	  CBVirtualCouderOverlayInternal(bool inverted): imagew(-1), imageh(-1), ronchi(nullptr), ronchisize(0), inverted(inverted) {}
	  ~CBVirtualCouderOverlayInternal() { if (ronchi!=nullptr) delete[] ronchi; }
	  void userclick(CBSModelScope *_scope, double x, double y) // find which zone the used clicked in and select it...
	  {
		  if (_scope==nullptr || imagew==-1 || imageh==-1) return;
		  QPoint c(int(imagew*_scope->_couderx),int(imageh*(1.0-_scope->_coudery)));
		  QPoint p1(int(imagew*x),int(imageh*(1.0-y)));
		  c= c-p1; double r= sqrt(c.x()*c.x()+c.y()*c.y());
		  double dpi= 100*_scope->_couderz/25.4;
		  r= r/dpi;
		  for (int i=0; i<_scope->get_zones()->count(); i++)
			  if (r<=_scope->get_zones()->at(i)->_val) { _scope->setZone(i-1); return; }
	  }
	  void button(CBSModelScope *_scope, int b)
	  {
		  if (_scope==nullptr || imagew==-1 || imageh==-1) return;
		  double dx= 1./double(imagew), dy= 1./double(imageh);
		  if (b==0) _scope->setCoudery(_scope->getCoudery()-dy);
		  if (b==1) _scope->setCoudery(_scope->getCoudery()+dy);
		  if (b==2) _scope->setCouderx(_scope->getCouderx()-dx);
		  if (b==3) _scope->setCouderx(_scope->getCouderx()+dx);
		  if (dy<dx) dx= dy;
		  if (b==4) _scope->setCouderz(_scope->getCouderz()-dx);
		  if (b==5) _scope->setCouderz(_scope->getCouderz()+dx);
	  }
	  void draw(QImage &tempImage, CBSModelScope *_scope, double &dpi, QPoint &c);
  };
class CBScopeVirtualCouder;
class CBScopeVirtualCouderRunnable : public  QVideoFilterRunnable
{
public:
    CBScopeVirtualCouder *filter;
    CBScopeVirtualCouderRunnable(CBScopeVirtualCouder *p): QVideoFilterRunnable(), filter(p) {}
    QVideoFrame run(QVideoFrame *inputframe, const QVideoSurfaceFormat &surfaceFormat, RunFlags flags);
};
class CBScopeVirtualCouder : public QAbstractVideoFilter
{
    Q_OBJECT
    CBSProp(CBSModelScope*, scope, Scope);
Q_SIGNALS:
	void scopeChanged();
	void finished(QObject *result);
public:
    CBScopeVirtualCouder(QObject *p= nullptr): QAbstractVideoFilter(p), _scope(nullptr), pausedFrame(nullptr), vco(false) { }
    QVideoFilterRunnable *createFilterRunnable() { return new CBScopeVirtualCouderRunnable(this); }
	QImage *pausedFrame; // non null if we are paused...
	CBVirtualCouderOverlayInternal vco;
	Q_INVOKABLE void userclick(double x, double y) { vco.userclick(_scope, x, y); }
	Q_INVOKABLE void button(int b) { vco.button(_scope, b); }
};

class CBScopeCouderOverlay : public QQuickPaintedItem
{
	Q_OBJECT
public:
	CBSPropE(CBSModelScope*, scope, Scope, QObject *v2= (QObject*)(v);
										update();
										connect(v2, SIGNAL(couderxChanged()), this, SLOT(update()));
										connect(v2, SIGNAL(couderyChanged()), this, SLOT(update()));
										connect(v2, SIGNAL(couderzChanged()), this, SLOT(update()));
										connect(v2, SIGNAL(zoneChanged()), this, SLOT(update()));
										connect(v2, SIGNAL(ronchiChanged()), this, SLOT(update()));
										connect(v2, SIGNAL(ronchiOffsetChanged()), this, SLOT(update()));
										connect(v2, SIGNAL(gradingChanged()), this, SLOT(update())); )
	CBSPropE(QString, source, Source, load(); )
Q_SIGNALS:
	void scopeChanged();
	void sourceChanged();
public:
	CBScopeCouderOverlay(QQuickItem *parent = nullptr): QQuickPaintedItem(parent), _scope(nullptr), vco(true) { }
	void paint(QPainter *painter);
	CBVirtualCouderOverlayInternal vco;
	Q_INVOKABLE void userclick(double x, double y) { vco.userclick(_scope, x, y); update(); }
	Q_INVOKABLE void button(int b) { vco.button(_scope, b); update(); }
	QRect irect;
	Q_INVOKABLE QPointF mapPointToSourceNormalized(QPoint p)
	{
		if (irect.width()==0 || irect.height()==0) return QPointF(0,0);
		return QPointF((p.x()-irect.left())/double(irect.width()), (p.y()-irect.top())/double(irect.height()));
	}
	QImage img;
	void load()
	{
		if (_source=="") img= QImage();
		else {
			if (_source.startsWith("file:///")) _source= _source.mid(8);
			img= QImage(_source);
		}
		update();
	}
};

#endif
