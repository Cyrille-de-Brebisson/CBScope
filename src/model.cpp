#include "model.h"
#include <QPainter>
#include <QColor>
#include <QPrinter>
#include <QPrintDialog>
#include "mes.h"

CBSModel *CBSModel::singleton= nullptr;

double getScopeDiameter(CBSModelScope *scope) { return scope->getDiametre(); }
double getScopeFocal(CBSModelScope *scope) { return scope->getFocal(); }
double getScopeSpherometerLegDistances(CBSModelScope *scope) { return scope->getSpherometerLegDistances(); }

void zone2(QPainter *p, double z1, double z2, QPoint &c, double a1, double a2, int dpi, int yflip)
{
    // x²+y²=r² -> y=sqrt(r²-x²), x=sqrt(r²-y²)
    int const steps= 20;
    a1= a1*M_PI/180.0;
    a2= a2*M_PI/180.0;
    double asteps= (a2-a1)/steps;
    double a= a1;
    QPoint pts[(steps+1)*2+1]; int pc= 0;
    for (int i=0; i<steps; i++) // draw first arc going up
    {
        pts[pc++]= QPoint(c.x()+int(z1*sin(a)/25.4*dpi), c.y()+yflip*int(z1*cos(a)/25.4*dpi));
        a+= asteps;
    }
    for (int i=0; i<steps; i++) // draw second arc going down
    {
        a-= asteps;
        pts[pc++]= QPoint(c.x()+int(z2*sin(a)/25.4*dpi), c.y()+yflip*int(z2*cos(a)/25.4*dpi));
    }
    pts[pc++]= pts[0]; // join the ends
    p->drawPolyline(pts, pc); // draw
}

void zone(QPainter *p, double z1, double z2, QPoint &c, double y1, double y2, int dpi, int yflip)
{
    if (z1>1.0 && z1<y2)
    {
        double a1= acos(y1/z2)*180.0/M_PI, a2= acos(y2/z2)*180.0/M_PI;
        if (std::isnan(a2)) a2= 30.0;
        zone2(p, z1, z2, c, a1, a2, dpi, yflip);
        zone2(p, z1, z2, c, a1+180.0, a2+180.0, dpi, yflip);
        return;
    }
    // x²+y²=r² -> y=sqrt(r²-x²), x=sqrt(r²-y²)
    int const steps= 20;
    double ystep= (y2-y1)/steps;
    double y= y1;
    QPoint pts[(steps+1)*2+1]; int pc= 0;
    if (z1>1)
    {   // normal zones
        for (int i=0; i<steps; i++) // draw first arc going up
        {
            double x= sqrt(z1*z1-y*y);
            pts[pc++]= QPoint(c.x()+int(x/25.4*dpi), c.y()+yflip*int(y/25.4*dpi));
            y+= ystep;
        }
        for (int i=0; i<steps; i++) // draw second arc going down
        {
            y-= ystep;
            double x= sqrt(z2*z2-y*y);
            pts[pc++]= QPoint(c.x()+int(x/25.4*dpi), c.y()+yflip*int(y/25.4*dpi));
        }
        pts[pc++]= pts[0]; // join the ends
        p->drawPolyline(pts, pc); // draw
        for (int i=0; i<pc; i++) pts[i]= QPoint(c.x()-(pts[i].x()-c.x()), c.y()-(pts[i].y()-c.y())); // reverse
        p->drawPolyline(pts, pc); // draw the 2nd part
    } else { // center zone!
        for (int i=0; i<steps; i++) // draw first arc going up
        {
            double x= sqrt(z2*z2-y*y);
            pts[pc++]= QPoint(c.x()+int(x/25.4*dpi), c.y()+int(y/25.4*dpi));
            y+= ystep;
        }
        y= y1;
        for (int i=0; i<steps; i++) // draw first arc going up, but inverted
        {
            double x= sqrt(z2*z2-y*y);
            pts[pc++]= QPoint(c.x()-int(x/25.4*dpi), c.y()-int(y/25.4*dpi));
            y+= ystep;
        }
        pts[pc++]= pts[0]; // join the ends
        p->drawPolyline(pts, pc); // draw
    }
}

// Draw the Couder screen on the painter at DPI. c is the center of the mirror
void CBSModelScope::paintCouder(QPainter *painter, QPoint &c, double dpi, bool showRed, bool showBlue)
{
    QBrush brush(QColor(0, 0, 0, 0));
    painter->setBrush(brush);
    painter->setRenderHint(QPainter::Antialiasing);
    // Paint in black the circles that correspond to the zone edges and full mirror (which should match zone(n)...)
#define circle(r) painter->drawEllipse(c.x()-int(r/25.4*dpi), c.y()-int(r/25.4*dpi), int(2.0*r/25.4*dpi), int(2.0*r/25.4*dpi))
    painter->setPen(QPen(QColor(0, 0, 0)));
    for (int i=0; i<m_zones->count(); i++) circle(m_zones->at(i)->_val);
    if (m_zones->count()==0 || !doubleEq(_diametre, m_zones->at(m_zones->count()-1)->_val))
        circle(_diametre/2.0); // full mirror, if needed
#undef circle
    if (m_zones->count()==0) return;
    double bot= 10; // find the "horizontal" cut line for the zones...
    if (m_zones->at(0)->_val<1) bot= m_zones->at(1)->_val*0.9;
    else bot= m_zones->at(2)->_val*0.9;
    if (showRed)
    {
        // Paint in red the normal zones
        painter->setPen(QPen(QColor(255, 0, 0)));
        for (int i=0; i<m_zones->count()-1; i++)
          zone(painter, m_zones->at(i)->_val, m_zones->at(i+1)->_val, c, 2.5, bot, int(dpi), (i&1)==0?1:-1);
    }
    if (showBlue)
    {
        // paint my zones in blue
        painter->setPen(QPen(QColor(0, 0, 255)));
        bot*= 0.8;
        zone(painter, m_zones->at(0)->_val, m_zones->at(1)->_val, c, 2.5, bot, int(dpi), 1);
        for (int i=1; i<m_zones->count()-1; i++)
          zone2(painter, m_zones->at(i)->_val, m_zones->at(i+1)->_val, c, -30.0, 30.0, int(dpi), (i&1)==0?1:-1);
    }
}

void CBSModelScope::printCouder()
{
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog dialog(&printer, nullptr);
    dialog.setWindowTitle(tr("Print couder mask"));
    if (dialog.exec() != QDialog::Accepted) return;
//    printer.setOutputFileName("couder.ps");
    QRect area(printer.pageRect());
    int dpi= printer.resolution();
    QPoint c= area.center();
    QPainter painter;
    painter.begin(&printer);
    paintCouder(&painter, c, dpi);
    painter.end();
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
    if (_scope==nullptr) return;

    for (double fx= 2; fx<rw/2; fx+= 2)
        rotateText(painter, getX(-fx)-painter->font().pixelSize()/2, h-addy+5, QString::number(2* 3456.0/60.0*fx/_scope->_focal, 'f', 2)+QString("°"));


    QColor colors[5]= {QColor(255, 0, 0), QColor(0, 255, 0), QColor(0, 0, 255), /*QColor(255, 255, 0),*/ QColor(255, 0, 255), QColor(0, 255, 255) };

    QStringList l= _scope->_secondariesToConcider.split(" ");
    double alwaysmax= 1e300;
    int count= 0;
    int secX= addx;
    QFont f(painter->font()); f.setPointSize(int(f.pointSize()*1.5)); painter->setFont(f);
    foreach (QString const &n, l)
    {
        bool ok; double sec= n.toDouble(&ok); if (!ok) continue;
        if (sec>alwaysmax) continue; // smaller secondary are always at max, so there is no need to use this one
        double v= calcOffAxisIllumination(_scope->_diametre, _scope->_focal, sec, _scope->_secondaryToFocal, 0);
        double loss= diagObstructionArea(_scope->_diametre, sec);
        if (v==0.0) continue; // 0% at central point, no need to continue!
        QPen p(colors[count%5]); p.setWidth(2); painter->setPen(p);
        QPoint pts[53]; int cnt= 0;
        for (double fx= -rw/2.0; fx<rw/2.0; fx+= rw/53.0)
        {
            double v= calcOffAxisIllumination(_scope->_diametre, _scope->_focal, sec, _scope->_secondaryToFocal, std::abs(fx))-loss;
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
    for (int i=0; i<_scope->get_eps()->count(); i++)
    {
        CBSModelEP *ep= _scope->get_eps()->at(i);
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
    QBrush brush(QColor(220, 220, 220));
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
        if (ypos<65.0) c= QColor(0, 176, 80);
        else if (ypos<65.0*2) c= QColor(255, 255, 0);
        else c= QColor(255, 0, 0);
        painter->setPen(QPen(c));
        int y= getY(ypos); painter->drawLine(addx, y, w-addx, y);
        painter->drawText(w-addx, y, " "+QString::number(ypos)+"nm");
        ypos+= ystep;
    }

    // get curve points and draw verticals...
    painter->setPen(QPen(QColor(0, 0, 0)));
    QPoint *pts= new QPoint[_mesure->iNbZone+1];
    for (int iZ=0;iZ<_mesure->iNbZone+1;iZ++)
    {
        pts[iZ]= getP(_mesure->_Hz[iZ], _mesure->_surf[iZ]);
        painter->drawLine(pts[iZ].x(), addy, pts[iZ].x(), h-addy);
    }
    QPen p(QColor(0, 0, 0)); p.setWidth(2); painter->setPen(p);
    painter->drawPolyline(pts, _mesure->iNbZone+1);
    delete[] pts;

    painter->drawText(0, fm.height(), " Lambda="+QString::number(_mesure->_Lambda, 'f', 2)+
                      " LambdaRms="+QString::number(_mesure->_WeightedLambdaRms, 'f', 2)+
                      " Lf/Ro="+QString::number(_mesure->_LfRoMax, 'f', 2)+
                      " Strehl="+QString::number(_mesure->_WeightedStrehl, 'f', 2)+
                      " focale="+QString::number(_mesure->_focale*10.0, 'f', 2)+
                      " glass to remove="+QString::number(_mesure->_glassToRemove, 'f', 2)+"mm^3");


}

void CBSModelParabolizingWork::checkNbMesures()
{
    if (_scope==nullptr) return;
    while (m_mesures->count()>_scope->getNbZones()) m_mesures->remove(m_mesures->count()-1);
    while (m_mesures->count()<_scope->getNbZones())
    {
        CBSQString *d= new CBSQString(m_mesures); m_mesures->append(d);
        connect(d, SIGNAL(valChanged()), this, SLOT(doCalculations()));
    }
}

// Volume of a "ring" of a cone from of innder radius r1 (height h1) and outer radius r2 (height h2). Thanks to HP Prime cas for that!
static double volumeRing(double r1, double r2, double h1, double h2)
{
    double a= r1, b= r2, d= h1, f= h2;
    return (-2*a*a*d*M_PI-a*a*f*M_PI+a*b*d*M_PI-a*b*f*M_PI+b*b*d*M_PI+2*b*b*f*M_PI)/3;
}

static double sqr(double v) { return v*v; }
// Most of this code was lifted from Etienne de Foras "Foucault" program (with some slight changes)...
// Thanks ETI... Hope you do not mind as I have not yet asked you for your permission :-(
void CBSModelParabolizingWork::doCalculations()
{
    QDateTime t1(QDateTime::currentDateTime());
    double const greenWave= 555.0; // used to be 560
    _LfRoMax=-1e300;
    _Lambda=0.;
    _GlassMax=0.;
    _WeightedLambdaRms=0.;
    _WeightedStrehl=0.;
    if (_scope==nullptr) return;
    checkNbMesures();
    iNbZone=_scope->getNbZones();

    if (_surf  !=nullptr) delete[] _surf  ; _surf  = new double[10+  iNbZone+1]; //10+ because I think that I have an overflow somewhere...
    if (_profil!=nullptr) delete[] _profil; _profil= new double[10+  iNbZone];
    if (_lf1000!=nullptr) delete[] _lf1000; _lf1000= new double[10+  iNbZone];
    if (_mesc  !=nullptr) delete[] _mesc  ; _mesc  = new double[10+  iNbZone];
    if (_Hz    !=nullptr) delete[] _Hz    ; _Hz    = new double[10+  iNbZone+1];
    if (_Hm4F  !=nullptr) delete[] _Hm4F  ; _Hm4F  = new double[10+  iNbZone];
    if (_RelativeSurface  !=nullptr) delete[] _RelativeSurface  ; _RelativeSurface  = new double[iNbZone];
    _Std=0.;

    double dRay=2.*_scope->_focal;

    double greenWaveNm=greenWave*1.e-9;
    double _dRoDif=1.22*greenWaveNm*dRay/_scope->_diametre/2.; //unit? TODO

    // get Hz
    for(int i=0;i<iNbZone+1;i++) _Hz[i]= _scope->get_zones()->at(i)->_val;

    //compute Hm2R, Hm4F
    double a= 1e300, b= -1e300; // min and max
    for(int i=0;i<iNbZone;i++)
    {
        double _Hm=(_Hz[i+1]+_Hz[i])/2.0;
        _Hm4F[i]=_Hm/dRay/2.;
        double _Hm2R;
        if (_scope->_slitIsMoving)
             _Hm2R=-_scope->_conical*sqr(_Hm)/2./dRay;
        else
            _Hm2R=-_scope->_conical*(sqr(_Hm)/dRay + sqr(sqr(_Hm)) /2. /dRay/sqr(dRay));
        _mesc[i]=(m_mesures->at(i)->asDouble()-_Hm2R)*(_scope->getSlitIsMoving()?2.0:1.0);
        if (a>_mesc[i]) a= _mesc[i]; if (b<_mesc[i]) b= _mesc[i];
    }

    //calcule les surfaces relatives
    double dSum=0.;
    for(int i=0;i<iNbZone;i++)
    {
        _RelativeSurface[i]=sqr(_Hz[i+1])-sqr(_Hz[i]);
        dSum+=_RelativeSurface[i];
    }
    for(int i=0;i<iNbZone;i++) _RelativeSurface[i]=_RelativeSurface[i]/dSum/**iNbZone*/;

    // search for lf/ro between a end b
    double const RESMES= 0.0001;
    find_minimum(a,b,RESMES,calc_lf1000);
    for(int i=0;i<iNbZone;i++) { double t=_lf1000[i]/_dRoDif/1e6; if (_LfRoMax<t) _LfRoMax= t; } // find max of LfRo

    // compute surface profile using slopes
    _profil[0]=0.;
    a= 1e300; b= -1e300; // min and max
    for(int i=0;i<iNbZone;i++)
    {
        double t=-_lf1000[i]/_scope->getFocal()/2.0*2000.;
        if (a>t) a= t; if (b<t) b= t;
        _profil[i+1]=_profil[i]+(_Hz[i+1]-_Hz[i])*t;
    }

    //on calcule le max et le min de la conique a ajuster
    a=a/_Hz[iNbZone]*2.0;
    b=b/_Hz[iNbZone]*2.0;
    double dReso=1./(_Hz[iNbZone]*_Hz[iNbZone]);

    // compute conique qui minimise le rms
    _focale= find_minimum(a,b,dReso,calc_less_rms);

    // compute conique qui minimise le ptv
    find_minimum(a,b,dReso,calc_less_ptv);

    double dMax=_surf[0]; for (int i=1; i<iNbZone; i++) if (_surf[1]>dMax) dMax= _surf[1];
    if (dMax!=0.) _Lambda=greenWave/2./dMax;
    else _Lambda=9999.;

    _GlassMax= dMax;
    _WeightedLambdaRms=greenWave/2./_Std; //   1./(surface std in lambda unit) //compute best parabola (vs RMS)
    _WeightedStrehl=exp(-sqr(2.*M_PI*1./_WeightedLambdaRms)); //compute stddev rms

    _glassToRemove= 0.0;
    for (int i=0; i<iNbZone; i++) _glassToRemove+= volumeRing(_Hz[i], _Hz[i+1], _surf[i]/1e6, _surf[i+1]/1e6);

    QDateTime t2(QDateTime::currentDateTime());
    emit mesuresChanged();
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
        double denom= 1.-(pMes->_scope->_conical+1.)*sqr(curv*pMes->_Hz[i]);

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
        double denom= 1.-(pMes->_scope->_conical+1.)*sqr(curv*pMes->_Hz[i]);
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
        pMes->_lf1000[i]=1000.*(pMes->_mesc[i]-h)*pMes->_Hm4F[i];
        if (minl>pMes->_lf1000[i]) minl= pMes->_lf1000[i];
        if (maxl<pMes->_lf1000[i]) maxl= pMes->_lf1000[i];
    }
    return(maxl+minl);
}

void CBScopeCouder::paint(QPainter *painter)
{
    QBrush brush(QColor(255, 255, 255));
    painter->setBrush(brush);
    painter->setPen(QPen(QColor(0, 0, 0)));
    painter->setRenderHint(QPainter::Antialiasing);

    QSizeF itemSize = size();
    int w= int(itemSize.width());
    int h= int(itemSize.height());
    painter->drawRect(0, 0, w, h);
    if (_scope==nullptr) return;
    QPoint c(w/2, h/2);
    _scope->paintCouder(painter, c, _zoom?96/2:96, _showRed, _showBlue);
}

void CBSModelScope::doMes()
{
    // cellType NbPoints NbSupportRings NbAnglularSegments NbMeshRings supportsPerRing
    // 0        3        1              3                  8           3 @ n*120°
    // 1        6        1              6                  8           6 @ n*60°
    // 2        9        2              9                  12          3,6 @ n*120° n*60°+30°
    // 3        18       2              18                 12          6,12 @ n*60°n*30°+15°
    // 4        27       3              24                 16          6,9,12 @ n*60° n*40°+10° n*30°+15°
    // 5        36       3              24                 16          6,12,18 @ n*60° n*30° n*20°
    struct TSupportRingEq { int nbSupports, astep, offset; };
    struct TCellTypeSupportDef { int nbRing; TSupportRingEq r[3]; };
    TCellTypeSupportDef const cellDefs[6]={
        {1, {{3, 120, 0 }} },
        {1, {{6, 60, 0 }} },
        {2, {{3, 120, 0 }, {6, 60, 30 }} },
        {2, {{6, 60, 0 }, {12, 30, 15 }} },
        {3, {{6, 60, 0 }, {9, 40, 10 }, {12, 30, 15}} },
        {3, {{6, 60, 0 }, {12, 30, 0 }, {18, 20, 0}} }
    };

    // Calc nb of 'rings' which will depend on cell type...
    int const nbRingsPerCell[6]= { 7, 11, 11, 19, 23};
    int nbRings= nbRingsPerCell[_cellType];
    int const pointsPerRing[24]= {1, 6, 12, 24, 24, 48, 48, 48, 48, 96, 96, 96, 96, 96, 96, 96, 96, 192, 192, 192, 192, 192, 192, 192 };
    int nbPoints= 0; for (int i=0; i<=nbRings; i++) nbPoints+= pointsPerRing[i]; nbPoints*= 2;
    mes.setNbPoints(nbPoints, true);
    int nbSupRings= cellDefs[_cellType].nbRing;
    double supRingRad[3]; for (int i=0; i<nbSupRings; i++) supRingRad[i]= _diametre/2.0/(nbSupRings+1.0)*(i+1);
    double meshRingRads[17]; // radiis of the various mesh rings
    for (int i=0; i<=nbRings; i++) meshRingRads[i]= i*_diametre/2.0/(nbRings);
    // now, find closest meshRing to a support ring and move meshRing to that ring. Then, re-adjust internal rings...
    int lastMeshRingDone= 0;
    for (int sr= 0; sr<nbSupRings; sr++)
    {
        int r= lastMeshRingDone;
        while (r<nbRings) if (supRingRad[sr]<(meshRingRads[r]+meshRingRads[r+1])/2) break; else r++;
        meshRingRads[r]= supRingRad[sr];
        for (int i= lastMeshRingDone+1; i<r; i++)
            meshRingRads[i]= ((r-i)*meshRingRads[lastMeshRingDone]+(i-lastMeshRingDone)*meshRingRads[r])/(r-lastMeshRingDone);
        lastMeshRingDone= r;
    }
    for (int i= lastMeshRingDone+1; i<nbSupRings; i++)
        meshRingRads[i]= ((nbSupRings-i)*meshRingRads[lastMeshRingDone]+(i-lastMeshRingDone)*meshRingRads[nbSupRings])/(nbSupRings-lastMeshRingDone);

    int ptsCount= 0;
    mes.point_list[ptsCount++]= mes.pts(0.0, 0.0, 0.0);
    mes.point_list[ptsCount++]= mes.pts(0.0, 0.0, thicnknessAt(0.0));
    double v= volumeRing(0, meshRingRads[1]/2.0, thicnknessAt(0.0), thicnknessAt(meshRingRads[1]/2.0))*_density/1000.0; // force
    mes.force_list[1]= mes.pts(0, 0, -v);

    // 1 elements for centre + 7 elements for each of the 6 triangles= 1+7*6=43!
    mes.setNbelement(43); int elCount= 0;
    // Centre "post"
    mes.element_list[elCount++].set(0,1);

    double h= thicnknessAt(meshRingRads[1]);
    int lastPointL= 12, lastPointH= 13;
    int firstPointLastRing= ptsCount;
    v= volumeRing(meshRingRads[1]/2.0, (meshRingRads[1]+meshRingRads[2])/2.0, thicnknessAt(meshRingRads[1]/2.0), thicnknessAt((meshRingRads[1]+meshRingRads[2])/2.0))*_density/1000.0/6.0; // force
    for (int j=0; j<6; j++)
    {
        mes.point_list[ptsCount]= mes.pts(meshRingRads[1]*cos(j*M_PI/3.0), meshRingRads[1]*sin(j*M_PI/3.0), 0);
        mes.force_list[ptsCount]= mes.pts(0, 0, -v);
        mes.point_list[ptsCount+1]= mes.pts(meshRingRads[1]*cos(j*M_PI/3.0), meshRingRads[1]*sin(j*M_PI/3.0), h);
        mes.element_list[elCount++]= mes.el(ptsCount,ptsCount+1);  // vertical between the points
        mes.element_list[elCount++]= mes.el(0,ptsCount);           // center to point 1
        mes.element_list[elCount++]= mes.el(1,ptsCount+1);         // center to point 2
        mes.element_list[elCount++]= mes.el(0,ptsCount+1);         // center to point 2 (diag radial face)
        mes.element_list[elCount++]= mes.el(lastPointL,ptsCount);  // lastPointL to point 1
        mes.element_list[elCount++]= mes.el(lastPointH,ptsCount+1);// lastPointH to point 2
        mes.element_list[elCount++]= mes.el(lastPointL,ptsCount+1);// lastPointL to point 2 (diag on outside face)
        lastPointL= ptsCount++; lastPointH= ptsCount++;
    }

    for (int i=2; i<=nbRings; i++)
    {
        int firstPointThisRing= ptsCount, savedFirstPointLastRing= firstPointLastRing;
        double a= 0.0, astep= M_PI*2.0/pointsPerRing[i];                     // Angle counter and stepps...
        lastPointL= ptsCount+pointsPerRing[i]*2-2; lastPointH= lastPointL+1; // Index of the last points of this ring to link the first point with the last and points with the previous point
        // add the appropriate number of elements for this ring (depends on if we increased the point count form last ring or not)
        if (pointsPerRing[i]==pointsPerRing[i-1]) mes.setNbelement(int(mes.element_no)+10*pointsPerRing[i]);
        else mes.setNbelement(int(mes.element_no)+17*pointsPerRing[i]/2);
        // calculate the weight, per point in this ring. take 1/2 of the previous and next rings.
        // except for last ring there only 1/2 of the previous ring is used...
        if (i!=nbRings) v= volumeRing((meshRingRads[i-1]+meshRingRads[i])/2.0, (meshRingRads[i]+meshRingRads[i+1])/2.0, thicnknessAt((meshRingRads[i-1]+meshRingRads[i])/2.0), thicnknessAt((meshRingRads[i]+meshRingRads[i+1])/2.0))*_density/1000.0/pointsPerRing[i]; // force
        else v= volumeRing((meshRingRads[i-1]+meshRingRads[i])/2.0, meshRingRads[i], thicnknessAt((meshRingRads[i-1]+meshRingRads[i])/2.0), thicnknessAt(meshRingRads[i]))*_density/1000.0/pointsPerRing[i]; // force
        // thickness of the disk on the points here...
        double h= thicnknessAt(meshRingRads[i]);
        // find fix points!
        for (int j=0; j<nbSupRings; j++)
        {
            if (doubleEq(supRingRad[j], meshRingRads[i]))
            {
                for (int k=0; k<cellDefs[_cellType].r[j].nbSupports; k++)
                {
                    int a= k*cellDefs[_cellType].r[j].astep+cellDefs[_cellType].r[j].offset;
                    a= a*pointsPerRing[i]/360;
                    mes.fix_list[ptsCount+a*2].x= mes.fix_list[ptsCount+a*2].y= mes.fix_list[ptsCount+a*2].z= true;
                }
            }
        }
        // now, calculate points in the mesh and create the mest itself!
        for (int j=0; j<pointsPerRing[i]; j++) // for each point of the ring
        {
            mes.point_list[ptsCount]= mes.pts(meshRingRads[i]*cos(a), meshRingRads[i]*sin(a), 0.0);
            mes.force_list[ptsCount]= mes.pts(0, 0, -v);
            mes.point_list[ptsCount+1]= mes.pts(meshRingRads[i]*cos(a), meshRingRads[i]*sin(a), h);
            if (pointsPerRing[i]==pointsPerRing[i-1])
            {
                mes.element_list[elCount++]= mes.el(ptsCount,ptsCount+1);  // vertical between the points
                mes.element_list[elCount++]= mes.el(firstPointLastRing+0,ptsCount);           // center to point 1
                mes.element_list[elCount++]= mes.el(firstPointLastRing+1,ptsCount+1);         // center to point 2
                mes.element_list[elCount++]= mes.el(firstPointLastRing+0,ptsCount+1);         // center to point 2 (diag radial face)
                mes.element_list[elCount++]= mes.el(lastPointL,ptsCount);  // lastPointL to point 1
                mes.element_list[elCount++]= mes.el(lastPointH,ptsCount+1);// lastPointH to point 2
                mes.element_list[elCount++]= mes.el(lastPointL,ptsCount+1);// lastPointL to point 2 (diag on outside face)
                firstPointLastRing+=2;
                if (firstPointLastRing==firstPointThisRing) firstPointLastRing= savedFirstPointLastRing;
                mes.element_list[elCount++]= mes.el(firstPointLastRing+0,ptsCount);           // center to point 1
                mes.element_list[elCount++]= mes.el(firstPointLastRing+1,ptsCount+1);         // center to point 2
                mes.element_list[elCount++]= mes.el(firstPointLastRing+0,ptsCount+1);         // center to point 2 (diag radial face)
            } else {
                mes.element_list[elCount++]= mes.el(ptsCount,ptsCount+1);  // vertical between the points
                mes.element_list[elCount++]= mes.el(firstPointLastRing+0,ptsCount);           // center to point 1
                mes.element_list[elCount++]= mes.el(firstPointLastRing+1,ptsCount+1);         // center to point 2
                mes.element_list[elCount++]= mes.el(firstPointLastRing+0,ptsCount+1);         // center to point 2 (diag radial face)
                mes.element_list[elCount++]= mes.el(lastPointL,ptsCount);  // lastPointL to point 1
                mes.element_list[elCount++]= mes.el(lastPointH,ptsCount+1);// lastPointH to point 2
                mes.element_list[elCount++]= mes.el(lastPointL,ptsCount+1);// lastPointL to point 2 (diag on outside face)
                if ((j&1)==1)
                {
                    firstPointLastRing+=2;
                    if (firstPointLastRing==firstPointThisRing) firstPointLastRing= savedFirstPointLastRing;
                    mes.element_list[elCount++]= mes.el(firstPointLastRing+0,ptsCount);           // center to point 1
                    mes.element_list[elCount++]= mes.el(firstPointLastRing+1,ptsCount+1);         // center to point 2
                    mes.element_list[elCount++]= mes.el(firstPointLastRing+0,ptsCount+1);         // center to point 2 (diag radial face)
                }
            }
            lastPointL= ptsCount++; lastPointH= ptsCount++;
            a+= astep;
        }
        firstPointLastRing= firstPointThisRing;
    }
}

void CBSModelScope::doMesSolve()
{
    qDebug() << mes.point_no << " points " << mes.element_no << " elements";
    QDateTime t(QDateTime::currentDateTime());
    mes.calc();
    QDateTime t2(QDateTime::currentDateTime());
    qDebug() << t2.toMSecsSinceEpoch()-t.toMSecsSinceEpoch() << " ms";
}

void CBScopeMes::paint(QPainter *painter)
{
    QBrush brush(QColor(200, 200, 200));
    painter->setBrush(brush);
    painter->setPen(QPen(QColor(0, 0, 0)));
    painter->setRenderHint(QPainter::Antialiasing);

    QSizeF itemSize = size();
    int w= int(itemSize.width());
    int h= int(itemSize.height());
    painter->drawRect(0, 0, w, h);
    if (_scope==nullptr) return;
    _scope->doMes();
    QPoint c(w/2, h/2);
    int dpi= 96;
    if (_show3D)
    {
        int const offset= 10;
        painter->setPen(QPen(QColor(255,0,0)));
        for (unsigned int i=0; i<_scope->mes.element_no; i++)
            if ((_scope->mes.element_list[i].p1&1)==1 && (_scope->mes.element_list[i].p2&1)==1)
                painter->drawLine(QPoint(c.x()+offset+int(_scope->mes.point_list[_scope->mes.element_list[i].p1].x/25.4*dpi), c.y()+offset+int(_scope->mes.point_list[_scope->mes.element_list[i].p1].y/25.4*dpi)),
                                  QPoint(c.x()+offset+int(_scope->mes.point_list[_scope->mes.element_list[i].p2].x/25.4*dpi), c.y()+offset+int(_scope->mes.point_list[_scope->mes.element_list[i].p2].y/25.4*dpi)));
        painter->setPen(QPen(QColor(0,0,255)));
        for (unsigned int i=0; i<_scope->mes.element_no; i++)
            if ((_scope->mes.element_list[i].p1&1)==0 && (_scope->mes.element_list[i].p2&1)==1)
                painter->drawLine(QPoint(c.x()+int(_scope->mes.point_list[_scope->mes.element_list[i].p1].x/25.4*dpi), c.y()+int(_scope->mes.point_list[_scope->mes.element_list[i].p1].y/25.4*dpi)),
                                  QPoint(c.x()+offset+int(_scope->mes.point_list[_scope->mes.element_list[i].p2].x/25.4*dpi), c.y()+offset+int(_scope->mes.point_list[_scope->mes.element_list[i].p2].y/25.4*dpi)));
        for (unsigned int i=0; i<_scope->mes.element_no; i++)
            if ((_scope->mes.element_list[i].p1&1)==1 && (_scope->mes.element_list[i].p2&1)==0)
                painter->drawLine(QPoint(c.x()+offset+int(_scope->mes.point_list[_scope->mes.element_list[i].p1].x/25.4*dpi), c.y()+offset+int(_scope->mes.point_list[_scope->mes.element_list[i].p1].y/25.4*dpi)),
                                  QPoint(c.x()+0+int(_scope->mes.point_list[_scope->mes.element_list[i].p2].x/25.4*dpi), c.y()+0+int(_scope->mes.point_list[_scope->mes.element_list[i].p2].y/25.4*dpi)));
    }
    painter->setPen(QPen(QColor(0,0,0)));
    for (unsigned int i=0; i<_scope->mes.element_no; i++)
        if ((_scope->mes.element_list[i].p1&1)==0 && (_scope->mes.element_list[i].p2&1)==0)
            painter->drawLine(QPoint(c.x()+int(_scope->mes.point_list[_scope->mes.element_list[i].p1].x/25.4*dpi), c.y()+int(_scope->mes.point_list[_scope->mes.element_list[i].p1].y/25.4*dpi)),
                              QPoint(c.x()+int(_scope->mes.point_list[_scope->mes.element_list[i].p2].x/25.4*dpi), c.y()+int(_scope->mes.point_list[_scope->mes.element_list[i].p2].y/25.4*dpi)));
    if (!_scope->mes.hasCalculated || !_showForces)
    {
        painter->setBrush(QBrush(QColor(0, 0, 0, 0)));
        for (unsigned int i=0; i<_scope->mes.point_no; i+= 2)
        {
            QPen p(QColor(255,0,0)); p.setWidth(3); painter->setPen(p);
            painter->drawPoint(QPoint(c.x()+int(_scope->mes.point_list[i].x/25.4*dpi), c.y()+int(_scope->mes.point_list[i].y/25.4*dpi)));
            if (_scope->mes.fix_list[i].z)
            {
              QPen p(QColor(0,255,255)); p.setWidth(1); painter->setPen(p);
              painter->drawEllipse(c.x()-5+int(_scope->mes.point_list[i].x/25.4*dpi), c.y()-5+int(_scope->mes.point_list[i].y/25.4*dpi), 10, 10);
            }
        }
    } else {
        double m= 1e300, M= -1e300;
        for (unsigned int i=0; i<_scope->mes.point_no; i+= 2)
        {
            if (_scope->mes.output_list[i].z<m) m= _scope->mes.output_list[i].z;
            if (_scope->mes.output_list[i].z>M) M= _scope->mes.output_list[i].z;
        }
        qDebug() << "min max " << m << M;
        painter->setBrush(QBrush(QColor(0, 0, 0, 0)));
        for (unsigned int i=0; i<_scope->mes.point_no; i+= 2)
        {
            QPen p(QColor(int((_scope->mes.output_list[i].z-m)/(M-m)*255),0,0)); p.setWidth((int(_scope->mes.output_list[i].z-m)/(M-m)*15+3)); painter->setPen(p);
            painter->drawPoint(QPoint(c.x()+int(_scope->mes.point_list[i].x/25.4*dpi), c.y()+int(_scope->mes.point_list[i].y/25.4*dpi)));
            if (_scope->mes.fix_list[i].z)
            {
              QPen p(QColor(0,255,255)); p.setWidth(1); painter->setPen(p);
              painter->drawEllipse(c.x()-5+int(_scope->mes.point_list[i].x/25.4*dpi), c.y()-5+int(_scope->mes.point_list[i].y/25.4*dpi), 10, 10);
            }
        }
    }
}
