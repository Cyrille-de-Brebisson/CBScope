#include "model.h"
#include <QPainter>
#include <QColor>

void CBSModelParabolizingWork::checkNbMesures()
{
    if (scope==nullptr) return;
    while (m_mesures->count()>scope->getNbZones()) m_mesures->remove(m_mesures->count()-1);
    while (m_mesures->count()<scope->getNbZones())
    {
        CBSDouble *d= new CBSDouble(m_mesures); m_mesures->append(d);
        connect(d, SIGNAL(valChanged()), this, SLOT(doCalculations()));
    }
}

CBSModel *CBSModel::singleton= nullptr;

double getScopeDiameter(CBSModelScope *scope) { return scope->getDiametre(); }
double getScopeFocal(CBSModelScope *scope) { return scope->getFocal(); }

// adjusts offset for field diameter by using the diagonal to eye distance;
// since diagToEyeDistance > diagToFocalPlaneDistance, the offset will shrink as the diagonal appears closer to the primary mirror;
// from http://www.telescope-optics.net/newtonian.htm
double forwardOffsetForSecondary(double diagMinorAxis, double diagToEyeDistance)
{
    return -diagMinorAxis * diagMinorAxis / 4.0 / diagToEyeDistance;
}

// diagonal off-axis illumination; sources are mid-70's Sky and Telescope article on diagonal size and Telescope Making #9, pg. 11 on diagonal offset.

static double calcOffAxisIllumination(double mirrorDia, double focalLen, double diagSize, double diagToFocalPlaneDistance, double offAxisDistance)
{
    double r = diagSize * focalLen / (diagToFocalPlaneDistance * mirrorDia);
    double x = 2.0 * offAxisDistance * (focalLen - diagToFocalPlaneDistance) / (diagToFocalPlaneDistance * mirrorDia);
    double a = (x * x + 1.0 - r * r) / (2.0 * x);
    if (a < -1.0) return 1.0;
    if (a > 1.0) return 0.0;
    double c = (x * x + r * r - 1.0) / (2.0 * x * r);
    double ret= (acos(a) - x * sqrt(1.0 - a * a) + r * r * acos(c)) / M_PI;
    return ret;
};

static double diagObstructionArea(double mirrorDia, double diagSize)
{
    return diagSize / mirrorDia * diagSize / mirrorDia;
};

static void rotateText(QPainter *painter, double x, double y, QString const &t)
{
    painter->save();
    painter->translate(x, y);
    painter->rotate(90);
    painter->drawText(0, 0, t);
    painter->restore();
}
void CBScopeIlumination::paint(QPainter *painter)
{
    QBrush brush(QColor("#ffffff"));
    painter->setBrush(brush);
    painter->setPen(QPen(QColor(0, 0, 0)));
    painter->setRenderHint(QPainter::Antialiasing);

    QSizeF itemSize = size();
    w= int(itemSize.width());
    h= int(itemSize.height());
    addx= w/10;
    addy= h/10;
    rw= 53.0;

    painter->drawRect(0, 0, w, h);
    painter->drawRect(addx, addy, w-2*addx, h-2*addy);
    painter->setPen(QPen(QColor(128, 128, 128)));
    for (double fx= -rw/2.0+0.5; fx<rw/2; fx+= 2) painter->drawLine(getX(fx), addy, getX(fx), h-addy);
    for (double fy= 1; fy>=0.5; fy-= 0.1)
    {
        painter->drawLine(addx, getY(fy), w-addx, getY(fy));
        painter->drawText(w-addx, getY(fy), QString::number(fy*100)+"%");
        painter->drawText(0, getY(fy), QString::number(-2.5*log(fy)/log(10.0),'f',2)+"mag");
    }

    painter->setPen(QPen(QColor(0, 0, 0)));
    for (double fx= 0; fx<rw/2; fx+= 2)
        rotateText(painter, getX(fx)-painter->font().pixelSize()/2, h-addy+5, QString::number(2*fx)+"mm");
    if (scope==nullptr) return;

    for (double fx= 2; fx<rw/2; fx+= 2)
        rotateText(painter, getX(-fx)-painter->font().pixelSize()/2, h-addy+5, QString::number(2* 3456.0/60.0*fx/scope->_focal, 'f', 2)+QString("°"));


    QColor colors[5]= {QColor(255, 0, 0), QColor(0, 255, 0), QColor(0, 0, 255), /*QColor(255, 255, 0),*/ QColor(255, 0, 255), QColor(0, 255, 255) };

    QStringList l= scope->_secondariesToConcider.split(" ");
    double alwaysmax= 1e300;
    int count= 0;
    int secX= addx;
    QFont f(painter->font()); f.setPointSize(int(f.pointSize()*1.5)); painter->setFont(f);
    foreach (QString const &n, l)
    {
        bool ok; double sec= n.toDouble(&ok); if (!ok) continue;
        if (sec>alwaysmax) continue; // smaller secondary are always at max, so there is no need to use this one
        double v= calcOffAxisIllumination(scope->_diametre, scope->_focal, sec, scope->_secondaryToFocal, 0);
        double loss= diagObstructionArea(scope->_diametre, sec);
        if (v==0.0) continue; // 0% at central point, no need to continue!
        QPen p(colors[count%5]); p.setWidth(2);
        painter->setPen(p);
        QPoint pts[53]; int cnt= 0;
        for (double fx= -rw/2.0; fx<rw/2.0; fx+= rw/53.0)
        {
            double v= calcOffAxisIllumination(scope->_diametre, scope->_focal, sec, scope->_secondaryToFocal, std::abs(fx))-loss;
            int y= getY(v); //1-v);
            pts[cnt].setY(y);
            pts[cnt++].setX(getX(fx));
        }
        if (pts[0].y()==pts[26].y()) alwaysmax= sec;
        count++;
        painter->save();
        painter->setClipRect(addx, addy, w-2*addx, h-2*addy);
        painter->drawPolyline(pts, 53);
        painter->restore();
        painter->drawText(secX, addy/2, n);
        QFontMetrics fm(painter->font());
        secX+= fm.width(n+" ");
    }

    int epsX= w, epsY= addy+1;
    for (int i=0; i<scope->get_eps()->count(); i++)
    {
        CBSModelEP *ep= scope->get_eps()->at(i);
        QPen p(colors[count%5]); p.setWidth(2);
        painter->setPen(p);
        double f= ep->getField()/2.0; int x1= getX(-f), x2= getX(f);
        painter->drawLine(x1, epsY, x2, epsY); epsY+= 3;
        QFontMetrics fm(painter->font());
        epsX-= fm.width(ep->getName()+" ");
        painter->drawText(epsX, addy/2, ep->getName());
        count++;
    }
}

void CBScopeMesure::paint(QPainter *painter)
{
    QBrush brush(QColor(200, 200, 200));
    painter->setBrush(brush);
    painter->setPen(QPen(QColor(0, 0, 0)));
    painter->setRenderHint(QPainter::Antialiasing);

    QSizeF itemSize = size();
    w= int(itemSize.width());
    h= int(itemSize.height());
    painter->drawRect(0, 0, w, h);
    addx= w/10;
    QFontMetrics fm(painter->font());
    addy= fm.height()+4;
//    painter->drawRect(addx, addy, w-2*addx, h-2*addy);
    if (_scope==nullptr || _mesure==nullptr || _mesure->iNbZone==0) return;
    rw= _scope->_diametre/2.0;

    // find top y. Take the lowest 2^n*65nm/2 that works if it exists...
    rh=65.0/2.0; // at least 65/2nm onthe display
    for (int i=0;i<_mesure->iNbZone+1;i++) if (_mesure->_surf[i]>rh) rh= _mesure->_surf[i];
    double d= 65.0/2.0;
    for (int i=0; i<10; i++) if (rh<d) { rh= d; break; } else d*=2.0;
    // calculate y marks. max of one every 20 pixels. use a number that is a multiple of 5.
    int nb= int(h*0.8/20.0);
    double ystep= int(rh/nb/5)*5.0, ypos= 0.0;
    // draw horizontals
    while (true)
    {
        if (ypos>rh) break;
        QColor c;
        if (ypos<65.0) c= QColor(0, 255, 0);
        else if (ypos<65.0) c= QColor(255, 255, 0);
        else c= QColor(255, 0, 0);
        painter->setPen(QPen(c));
        int y= getY(ypos); painter->drawLine(addx, y, w-addx, y);
        painter->drawText(w-addx, y, " "+QString::number(ypos)+"nm");
        ypos+= ystep;
    }

    // get curve points and draw verticals...
    painter->setPen(QPen(QColor(0, 0, 0)));
    QPoint pts[_mesure->iNbZone+1];
    for (int iZ=0;iZ<_mesure->iNbZone+1;iZ++)
    {
        pts[iZ]= getP(_mesure->_Hz[iZ], _mesure->_surf[iZ]);
        painter->drawLine(pts[iZ].x(), addy, pts[iZ].x(), h-addy);
    }
    QPen p(QColor(0, 0, 0)); p.setWidth(2); painter->setPen(p);
    painter->drawPolyline(pts, _mesure->iNbZone+1);

    painter->drawText(0, fm.height(), " Lambda="+QString::number(_mesure->_Lambda, 'f', 2)+
    //                  " Glass="+QString::number(_mesure->_GlassMax, 'f', 2)+
                      " LambdaRms="+QString::number(_mesure->_WeightedLambdaRms, 'f', 2)+
                      " Lf/Ro="+QString::number(_mesure->_LfRoMax, 'f', 2)+
                      " Strehl="+QString::number(_mesure->_WeightedStrehl, 'f', 2)+
                      " glass to remove="+QString::number(_mesure->_glassToRemove, 'f', 2)+"mm^3");

}

static double sqr(double v) { return v*v; }
void CBSModelParabolizingWork::doCalculations()
{
    qDebug() << "Do Calculations (this, _scope)" << this << _scope;
    double const greenWave= 560.0; //555.0;
    _LfRoMax=-1e300;
    _Lambda=0.;
    _GlassMax=0.;
    _WeightedLambdaRms=0.;
    _WeightedStrehl=0.;
    if (_scope==nullptr) return;
    checkNbMesures();
    iNbZone=_scope->getNbZones();
    double *_lfro  = new double[iNbZone+1];
    double *_moinsu= new double[iNbZone+1];
    double *_Hm    = new double[iNbZone];
    double *_Hm2R  = new double[iNbZone];

    if (_surf  !=nullptr) delete[] _surf  ; _surf  = new double[iNbZone+1];
    if (_profil!=nullptr) delete[] _profil; _profil= new double[iNbZone];
    if (_deltaC!=nullptr) delete[] _deltaC; _deltaC= new double[iNbZone];
    if (_lf1000!=nullptr) delete[] _lf1000; _lf1000= new double[iNbZone];
    if (_mesc  !=nullptr) delete[] _mesc  ; _mesc  = new double[iNbZone];
    if (_Hz    !=nullptr) delete[] _Hz    ; _Hz    = new double[iNbZone+1];
    if (_Hm4F  !=nullptr) delete[] _Hm4F  ; _Hm4F  = new double[iNbZone];
    if (_RelativeSurface  !=nullptr) delete[] _RelativeSurface  ; _RelativeSurface  = new double[iNbZone];
    _Std=0.;

    double dRay=2.*_scope->_focal;

    double greenWaveNm=greenWave*1.e-9;
    double _dRoDif=1.22*greenWaveNm*dRay/_scope->_diametre/2.; //unit? TODO

    //compute Hz (not saved)
    for(int i=0;i<iNbZone+1;i++) _Hz[i]= _scope->get_zones()->at(i)->_val;

    //compute Hm
    for(int i=0;i<iNbZone;i++) _Hm[i]=(_Hz[i+1]+_Hz[i])/2.;

    //compute Hm2R
    double const _Conical= _scope->getConical();
    if (_scope->_slitIsMoving)
        for(int i=0;i<iNbZone;i++) _Hm2R[i]=-_Conical*sqr(_Hm[i])/2./dRay;
    else
        for(int i=0;i<iNbZone;i++) _Hm2R[i]=-_Conical*(sqr(_Hm[i])/dRay + sqr(sqr(_Hm[i])) /2. /dRay/sqr(dRay));

    //compute Hm4F
    for (int i=0;i<iNbZone;i++) _Hm4F[i]=_Hm[i]/dRay/2.;

    //calcule les surfaces relatives
    double dSum=0.;
    for(int i=0;i<iNbZone;i++)
    {
        _RelativeSurface[i]=sqr(_Hz[i+1])-sqr(_Hz[i]);
        dSum+=_RelativeSurface[i];
    }
    for(int i=0;i<iNbZone;i++) _RelativeSurface[i]=_RelativeSurface[i]/dSum/**iNbZone*/;

    double a= 1e300, b= -1e300; // min and max
    for (int i=0;i<iNbZone;i++)
    {
        _mesc[i]=(m_mesures->at(i)->_val-_Hm2R[i])*(_scope->getSlitIsMoving()?2.0:1.0);
        if (a>_mesc[i]) a= _mesc[i]; if (b<_mesc[i]) b= _mesc[i];
    }

    // search for lf/ro between a end b
    double const RESMES= 0.0001;
    find_minimum(a,b,RESMES,calc_lf1000);

    for(int i=0;i<iNbZone;i++)
    {
        _lfro[i]=_lf1000[i]/_dRoDif/1e6;
        if (_LfRoMax<_lfro[i]) _LfRoMax= _lfro[i];
    }

    a= 1e300; b= -1e300; // min and max
    for(int i=0;i<iNbZone;i++)
    {
        _moinsu[i]=-_lf1000[i]/_scope->getFocal()/2.0*2000.;
        if (a>_moinsu[i]) a= _moinsu[i]; if (b<_moinsu[i]) b= _moinsu[i];
    }

    // compute surface profile using slopes
    _profil[0]=0.;
    for(int i=1;i<iNbZone+1;i++) _profil[i]=_profil[i-1]+(_Hz[i]-_Hz[i-1])*_moinsu[i-1];

    //on calcule le max et le min de la conique a ajuster
    a=a/_Hz[iNbZone]*2.0;
    b=b/_Hz[iNbZone]*2.0;

    double dReso=1./(_Hz[iNbZone]*_Hz[iNbZone]);

    // compute conique qui minimise le rms
    find_minimum(a,b,dReso,&(calc_less_rms));

    // compute conique qui minimise le ptv
    find_minimum(a,b,dReso,&(calc_less_ptv));

    double dMax=_surf[0]; for (int i=1; i<iNbZone; i++) if (_surf[1]>dMax) dMax= _surf[1];
    if (dMax!=0.) _Lambda=greenWave/2./dMax;
    else _Lambda=9999.;

    _GlassMax= dMax;
    _WeightedLambdaRms=greenWave/2./_Std; //   1./(surface std in lambda unit) //compute best parabola (vs RMS)
    _WeightedStrehl=exp(-sqr(2.*M_PI*1./_WeightedLambdaRms)); //compute stddev rms

    delete[] _lfro  ;
    delete[] _moinsu;
    delete[] _Hm    ;
    delete[] _Hm2R  ;
    _glassToRemove= 0.0;
    for (int i=0; i<iNbZone; i++)
    {
        double a= _Hz[i], b= _Hz[i+1];
        double d=_surf[i]/1e6, f= _surf[i+1]/1e6;
        double v= (-2*a*a*d*M_PI-a*a*f*M_PI+a*b*d*M_PI-a*b*f*M_PI+b*b*d*M_PI+2*b*b*f*M_PI)/3;
        _glassToRemove+= v;
    }

    emit mesuresChanged();
//    qDebug() << "  done Do Calculations (this, _scope)" << this << _scope;
}

double CBSModelParabolizingWork::find_minimum(double a,double c,double res,double (*fcn)(CBSModelParabolizingWork* self,double h))
{
    double b=(a+c)/2.;
    double fb=fcn(this,b);
    while (fabs(a-c)>res)   //dichotomy 1.5
    {
        double m1=(a+b)/2.;
        double fm1=fcn(this,m1);
        if (fabs(fm1)<fabs(fb)) { c=b; b=m1; fb=fm1; }
        else
        {
            double m2=(b+c)/2.;
            double fm2=fcn(this,m2);
            if (fabs(fm2)<fabs(fb)) { a=b; b=m2; fb=fm2; }
            else { a=m1; b=m2; fb=fm2; }
        }
    }
    return b;
}

double CBSModelParabolizingWork::calc_less_ptv(CBSModelParabolizingWork *pMes, double curv)
{
    double min= 1e300;
    //compute surface
    for (int i=0;i<pMes->iNbZone+1;i++)
    {
        double dtemp;
        double denom= 1.-(pMes->scope->getConical()+1.)*sqr(curv*pMes->_Hz[i]);

        if (denom>=0.) dtemp=( curv*sqr(pMes->_Hz[i]) ) / ( 1.+sqrt(denom) );
        else dtemp=curv*sqr(pMes->_Hz[i]); // cas degrade

        pMes->_surf[i]=(pMes->_profil[i]-dtemp)/2.;
        if (pMes->_surf[i]<min) min= pMes->_surf[i];
    }

    // shift between 0 and max-min
    double max= -1e300;
    for (int i=0;i<pMes->iNbZone+1;i++) { pMes->_surf[i]-=min; if (pMes->_surf[i]>max) max= pMes->_surf[i]; }
    return max; // return PTV
}

double CBSModelParabolizingWork::calc_less_rms(CBSModelParabolizingWork* pMes,double curv)
{
    double dtemp;
    //compute surface
    for (int i=0;i<pMes->iNbZone+1;i++)
    {
        double denom= 1.-(pMes->scope->getConical()+1.)*sqr(curv*pMes->_Hz[i]);
        if (denom>=0.) dtemp=( curv*sqr(pMes->_Hz[i]) ) / ( 1.+sqrt(denom) );
        else dtemp=curv*sqr(pMes->_Hz[i]); // bad case
        pMes->_surf[i]=(pMes->_profil[i]-dtemp)/2.;
    }
    //compute mean
    double dM=0.;
    for(int i=0;i<pMes->iNbZone;i++) dM+=(pMes->_surf[i]+pMes->_surf[i+1])/2.*pMes->_RelativeSurface[i];
    //compute var and stddev
    double dVar=0.;
    for(int i=0;i<pMes->iNbZone;i++) dVar+=sqr((pMes->_surf[i]+pMes->_surf[i+1])/2.-dM)*pMes->_RelativeSurface[i];
    double dStd=sqrt(dVar);
    return pMes->_Std=dStd;
}

double CBSModelParabolizingWork::calc_lf1000(CBSModelParabolizingWork* pMes,double h)
{
    double minl=1e300, maxl= -1e300;
    for (int i=0;i<pMes->iNbZone;i++)
    {
        pMes->_deltaC[i]=pMes->_mesc[i]-h;
        pMes->_lf1000[i]=1000.*pMes->_deltaC[i]*pMes->_Hm4F[i];
        if (minl>pMes->_lf1000[i]) minl= pMes->_lf1000[i];
        if (maxl<pMes->_lf1000[i]) maxl= pMes->_lf1000[i];
    }
    return(maxl+minl);
}
