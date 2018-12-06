#include "model.h"
#include <QPainter>
#include <QColor>
#include <QDirIterator>

CBSModel *CBSModel::singleton= nullptr;
double getScopeDiameter(CBSModelScope *scope) { return scope->getDiametre(); }
double getScopeFocal(CBSModelScope *scope) { return scope->getFocal(); }
double getScopeToHog(CBSModelScope *scope) { return scope->getToHog(); }
double getScopeSpherometerLegDistances(CBSModelScope *scope) { return scope->getSpherometerLegDistances(); }

void CBSModelEP::setScope(CBSModelScope *v)
{
	if (v==_scope) return; _scope= v; 
	connect(this, SIGNAL(nameChanged()),  v, SLOT(emitEpsChanged()));     // scope needs to know to redraw illumination
	connect(this, SIGNAL(fieldChanged()), v, SLOT(emitEpsChanged()));    // scope needs to know to redraw illumination
	connect(v, SIGNAL(focalChanged()),    this, SLOT(ScopeFocalChanged()));
	connect(v, SIGNAL(diametreChanged()), this, SLOT(ScopeFocalChanged()));
	emit scopeChanged();
}
void CBSModelHoggingWork::setScope(CBSModelScope *v)
{
	if (v==_scope) return; _scope= v; 
	connect(this, SIGNAL(gritChanged()), v, SLOT(emitHogTimeWithGritChanged()));
	connect(this, SIGNAL(endSphereChanged()), v, SLOT(emitHogTimeWithGritChanged()));
	connect(this, SIGNAL(hogSpeedChanged()), v, SLOT(emitHogTimeWithGritChanged())); 
}

bool CBSModel::help()
{
    QString temp(QStandardPaths::writableLocation(QStandardPaths::TempLocation)+"/CBScope");
    QDir(temp).mkdir(".");
    QDir(temp+"/CBScope_help_files").mkdir(".");
    if (!QFile::copy(":/Resources/help/CBScope_help.htm", temp+"/CBScope_help.htm")) goto er;
    {
        QDirIterator it(":/Resources/help/CBScope_help_files");
        while (it.hasNext()) {
            QString s = it.next();
            if (it.fileInfo().isDir()) continue;
            if (!QFile::copy(s, temp+"/CBScope_help_files/"+it.fileName())) goto er;
        }
        QDesktopServices::openUrl(QUrl(temp+"/CBScope_help.htm"));
        return true;
    }
    er:
    QDesktopServices::openUrl(QUrl("https://github.com/Cyrille-de-Brebisson/CBScope/wiki/Help"));
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Couder painting functions...

// Draw blue type zones. ie: arc of disks from angle a1 to a2 of radiis z1 to z2 centered at c. yflip specifies if the zone is on top or bottom...
// zones with a 0 radii is treated differently (see output)
// angles are in degree. I do not remember where the 0 point is!
// arcs are drawn using lines...
static void zone2(QPainter *p, double z1, double z2, QPoint &c, double a1, double a2, int dpi, int yflip) 
{
  int const steps= 20;
  a1= a1*M_PI/180.0;
  a2= a2*M_PI/180.0;
  double asteps= (a2-a1)/steps;
  double a= a1;
  QPoint pts[(steps+1)*2+1]; int pc= 0;
  for (int i=0; i<steps; i++) { pts[pc++]= QPoint(c.x()+int(z1*sin(a)/25.4*dpi), c.y()+yflip*int(z1*cos(a)/25.4*dpi)); a+= asteps; } // draw first arc going up
  for (int i=0; i<steps; i++) { a-= asteps; pts[pc++]= QPoint(c.x()+int(z2*sin(a)/25.4*dpi), c.y()+yflip*int(z2*cos(a)/25.4*dpi)); } // draw second arc going down
  pts[pc++]= pts[0]; // join the ends
  p->drawPolyline(pts, pc); // draw
}

// Draw orange type zones. not used anymore.
static void zone2(QPainter *p, double z1, double z2, QPoint &c, double a1, double a2, double a3, double a4, int dpi, int flip)
{
  // x²+y²=r² -> y=sqrt(r²-x²), x=sqrt(r²-y²)
  int const steps= 20;
  a1= a1*M_PI/180.0;
  a2= a2*M_PI/180.0;
  a3= a3*M_PI/180.0;
  a4= a4*M_PI/180.0;
  double asteps= (a2-a1)/steps;
  double a= a1;
  QPoint pts[(steps+1)*2+1]; int pc= 0;
  for (int i=0; i<steps+1; i++) { pts[pc++]= QPoint(c.x()+flip*int(z1*sin(a)/25.4*dpi), c.y()+int(z1*cos(a)/25.4*dpi)); a+= asteps; } // draw first arc going up
  a= a4; asteps= (a4-a3)/steps;
  for (int i=0; i<steps+1; i++) { pts[pc++]= QPoint(c.x()+flip*int(z2*sin(a)/25.4*dpi), c.y()+int(z2*cos(a)/25.4*dpi)); a-= asteps; } // draw second arc going down
  pts[pc++]= pts[0]; // join the ends
  p->drawPolyline(pts, pc); // draw
}

// Draw red type zones 
// z1 and z2 are the inner/outter diameters. y1 and y2 the top/bottom of the zones. c is the center. yflip indicate top/bottom drawing
static void zone(QPainter *p, double z1, double z2, QPoint &c, double y1, double y2, int dpi, int yflip)
{
    if (z1>1.0 && z1<y2) // special case when y2 is too large
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
void CBSModelScope::paintCouder(QPainter *painter, QPoint &c, double dpi, bool showRed, bool showBlue, bool showOrange)
{
    QBrush brush(QColor(0, 0, 0, 0));
    painter->setBrush(brush);
    // Paint in black the circles that correspond to the zone edges and full mirror (which should match zone(n)...)
#define circle(r) painter->drawEllipse(c.x()-int(r/25.4*dpi), c.y()-int(r/25.4*dpi), int(2.0*r/25.4*dpi), int(2.0*r/25.4*dpi))
    painter->setPen(QPen(QColor(0, 0, 0)));
    for (int i=0; i<m_zones->count(); i++) circle(m_zones->at(i)->_val);
    if (m_zones->count()==0 || !doubleEq(_diametre, m_zones->at(m_zones->count()-1)->_val))
        circle(_diametre/2.0); // full mirror, if needed
#undef circle
    if (m_zones->count()==0) return; // no zones to paint
    double bot= 10; // find the top "horizontal" cut line for the zones... depends if zone 0 starts at mirror center or not...
    if (m_zones->at(0)->_val<1) bot= m_zones->at(1)->_val*0.9;
    else bot= m_zones->at(2)->_val*0.9;
    if (showRed)
    {  // Paint in red the "normal" couder zones
        painter->setPen(QPen(QColor(255, 0, 0)));
        for (int i=0; i<m_zones->count()-1; i++) zone(painter, m_zones->at(i)->_val, m_zones->at(i+1)->_val, c, 2.5, bot, int(dpi), (i&1)==0?1:-1);
    }
    if (showBlue)
    {  // paint my zones style in blue (top/bottom)
        painter->setPen(QPen(QColor(0, 0, 255)));
        bot*= 0.8;
        zone(painter, m_zones->at(0)->_val, m_zones->at(1)->_val, c, 2.5, bot, int(dpi), 1);
        for (int i=1; i<m_zones->count()-1; i++) zone2(painter, m_zones->at(i)->_val, m_zones->at(i+1)->_val, c, -30.0, 30.0, int(dpi), (i&1)==0?1:-1);
    }
    if (showOrange)
    {  // Paint in Orange last zone style... not used at the moment
        painter->setPen(QPen(QColor(255,165,0)));
        bot*= 0.8;
        zone(painter, m_zones->at(0)->_val, m_zones->at(1)->_val, c, 2.5, bot, int(dpi), 1);
        for (int i=1; i<m_zones->count()-1; i++)
        {
          double a1= asin(bot/m_zones->at(i)->_val)*180.0/M_PI;
          double a2= asin(bot/m_zones->at(i+1)->_val)*180.0/M_PI;
          zone2(painter, m_zones->at(i)->_val, m_zones->at(i+1)->_val, c, 90.0-a1, 90.0+a1, 90.0-a2, 90.0+a2, int(dpi), 1);
          zone2(painter, m_zones->at(i)->_val, m_zones->at(i+1)->_val, c, 90.0-a1, 90.0+a1, 90.0-a2, 90.0+a2, int(dpi), -1);
        }
    }
}

// Print a couder screen on the printer. Pop up a print dialog box...
void CBSModelScope::printCouder()
{
#if CanPrint
    QPrinter printer;//(QPrinter::HighResolution);
    QPrintDialog dialog(&printer, nullptr);
    dialog.setWindowTitle(tr("Print couder mask"));
    if (dialog.exec() != QDialog::Accepted) return;
    QRect area(printer.pageRect());
    int dpi= printer.resolution();
    QPoint c= area.center();
    QPainter painter;
    painter.begin(&printer);
    painter.beginNativePainting();
    paintCouder(&painter, c, dpi, _showCouderRed, _showCouderBlue, _showCouderOrange);
    painter.endNativePainting();
    painter.end();
#endif
}

#define scopeEmailStartString "__SCOPE__START__\n"
#define scopeEmailEndString "\n__SCOPE__ENDS__"
void CBSModelScope::email()
{
	QJsonObject json_obj;
	saveProperties(&json_obj);
	QJsonDocument json_doc(json_obj);
	QString json_string = json_doc.toJson();
	QString startText("Sending scope "+_name+"\n"+scopeEmailStartString);
	QString endText(scopeEmailEndString); endText+= "\n";
	QDesktopServices::openUrl(QUrl::fromEncoded("mailto:email@somewhere.com?subject=Sending%20scope&body="+QUrl::toPercentEncoding(startText+json_string+endText)));
}

int CBSModel::loadScope(QString scope) // load a scope from a definition. returns it's model id
{
	int pos1= scope.indexOf(scopeEmailStartString);
	if (pos1==-1) return -1; pos1+= int(strlen(scopeEmailStartString));
	int pos2= scope.indexOf(scopeEmailEndString);
	if (pos2==-1) return -1;
	QString json_string(scope.mid(pos1, pos2-pos1));
	QByteArray json_bytes = json_string.toLocal8Bit();
	auto json_doc = QJsonDocument::fromJson(json_bytes); // Transform into json system
	if (json_doc.isNull()) { qDebug() << "Failed to create JSON doc."; return -1; }
	if (!json_doc.isObject()) { qDebug() << "JSON is not an object."; return -1; }
	QJsonObject o = json_doc.object();
	if (o.isEmpty()) { qDebug() << "JSON object is empty."; return -1; }
	CBSModelScope *s= new CBSModelScope(m_scopes); 
	s->loadProperties(&o); // Load the json object into our system
	s->loadProperties(nullptr); // re-enable all signals!
	m_scopes->append(s);
	return m_scopes->count()-1;
}

///////////////////////////////////////////////////////////////////////////
// Couder screen widget
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
  _scope->paintCouder(painter, c, _zoom?96/2:96, _scope->getShowCouderRed(), _scope->getShowCouderBlue(), _scope->getShowCouderOrange());
}

//////////////////////////////////////////////////////////////
//          Illumination field drawing functions are here...
// 
// diagonal off-axis illumination calculator
// pinched it from Mel Bartels at BBAstroDesigns
// original sources are mid-70's Sky and Telescope article on diagonal size and Telescope Making #9, pg. 11 on diagonal offset.
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
  static double diagObstructionArea(double mirrorDia, double diagSize) { return diagSize / mirrorDia * diagSize / mirrorDia; };
  static void rotateText(QPainter *painter, double x, double y, QString const &t) // Print rotated text
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
    // Size computations
    QSizeF itemSize = size();
    w= int(itemSize.width());
    h= int(itemSize.height());
    addx= w/10;
    addy= h/10;
    rw= _twoInches ? 53.0 : 33.0; // width of the graph in "field" coordinates

    painter->drawRect(0, 0, w, h); // blank out all
    painter->drawRect(addx, addy, w-2*addx, h-2*addy); // draw graph area borders
    painter->setPen(QPen(QColor(128, 128, 128)));
    for (double fx= -rw/2.0+0.5; fx<rw/2; fx+= 2) painter->drawLine(getX(fx), addy, getX(fx), h-addy); // vertical lines
    for (double fy= 1; fy>=0.5; fy-= 0.1) // horizontal lines and text
    {
        painter->drawLine(addx, getY(fy), w-addx, getY(fy));
        painter->drawText(w-addx, getY(fy), QString::number(fy*100)+"%");
        painter->drawText(0, getY(fy), QString::number(-2.5*log(fy)/log(10.0),'f',2)+"mag");
    }

    // vertical text on right and left of vertical optical axis
    painter->setPen(QPen(QColor(0, 0, 0)));
    for (double fx= 0; fx<rw/2; fx+= 2)
        rotateText(painter, getX(fx)-painter->font().pixelSize()/2, h-addy+5, QString::number(2*fx)+"mm");
    if (_scope==nullptr) return;

    for (double fx= 2; fx<rw/2; fx+= 2)
        rotateText(painter, getX(-fx)-painter->font().pixelSize()/2, h-addy+5, QString::number(2*3456.0/60.0*fx/_scope->_focal, 'f', 2)+QString("°"));

    QColor colors[5]= {QColor(255, 0, 0), QColor(0, 255, 0), QColor(0, 0, 255), QColor(255, 0, 255), QColor(0, 255, 255) };

    QStringList l= _scope->_secondariesToConcider.split(" "); // l is the list of secondary sizes to work with
    double alwaysmax= 1e300; // assuming that secondaries are in assending order. As soon as a secondary with a size that is maximum on the whole graph is found, its size will be palced here.
                             // any secondary larger than this will then be ignored/not drawn
    int count= 0;
    int secX= addx;          // X for writing of secondary sizes on the top left
    QFont f(painter->font()); f.setPointSize(int(f.pointSize()*1.5)); painter->setFont(f);
    foreach (QString const &n, l) // For all secondaries
    {
        bool ok; double sec= n.toDouble(&ok); if (!ok) continue;    // get the size as a float
        if (sec>alwaysmax) continue;                                // is this secondary known to be at max illumination in the whole graph? do not draw...
        double v= calcOffAxisIllumination(_scope->_diametre, _scope->_focal, sec, _scope->_secondaryToFocal, 0); // illumination at center
        if (v==0.0) continue;                                       // 0% at central point, no need to continue!
        double loss= diagObstructionArea(_scope->_diametre, sec);   // loss of ilumination due to the secondary size!
        QPen p(colors[count%5]); p.setWidth(2); painter->setPen(p); // pick a color
        QPoint pts[53*2+10]; int cnt= 0;                            // array of points that will be populated
        for (double fx= -rw/2.0; fx<=rw/2.0; fx+= 0.5)              // every 5mm of feild, calculate ilumination
        {
            double v= calcOffAxisIllumination(_scope->_diametre, _scope->_focal, sec, _scope->_secondaryToFocal, std::abs(fx))-loss;
            int y= getY(v); pts[cnt].setY(y); pts[cnt++].setX(getX(fx)); // generate point
            if (abs(fx)<0.001 && pts[0].y()==y) alwaysmax= sec;          // test if all the numbers are at the same value (ie: no need to get bigger secondary)
        }
        count++; // next secondary
        painter->save(); // save clip
        painter->setClipRect(addx, addy, w-2*addx, h-2*addy); // clip around graph area
        painter->drawPolyline(pts, cnt);    // draw curve
        painter->restore();                 // restore clip
        painter->drawText(secX, addy/2, n); // draw secondary size
        QFontMetrics fm(painter->font()); secX+= fm.width(n+" "); // spot for next secondary size drawing
    }

    // Now, display eyepeices field of view
    int epsX= w, epsY= addy+1;
    for (int i=0; i<_scope->get_eps()->count(); i++)          // for all EP
    {
        CBSModelEP *ep= _scope->get_eps()->at(i);
        QPen p(colors[count%5]); p.setWidth(2);
        painter->setPen(p);
        double f= ep->getField()/2.0; int x1= getX(-f), x2= getX(f); // calculate field
        painter->drawLine(x1, epsY, x2, epsY); epsY+= 3;             // draw it
        QFontMetrics fm(painter->font());                            // draw EP name
        epsX-= fm.width(ep->getName()+" ");
        painter->drawText(epsX, addy/2, ep->getName());
        count++;
    }
}

///////////////////////////////////////////////////////////////////////////
// Couder mesure printing function
void CBScopeMesure::paint(QPainter *painter)
{
    QBrush brush(QColor(220, 220, 220));
    painter->setBrush(brush);
    painter->setPen(QPen(QColor(0, 0, 0)));
    painter->setRenderHint(QPainter::Antialiasing);

    QSizeF itemSize = size();   // get sizes
    w= int(itemSize.width());
    h= int(itemSize.height());
    painter->drawRect(0, 0, w, h);
    addx= w/10;
    QFontMetrics fm(painter->font());
    addy= fm.height()+4;
//    painter->drawRect(addx, addy, w-2*addx, h-2*addy);
    if (_mesure!=nullptr && _mesure->_surf==nullptr) _mesure->doCalculations(); // make sure we have data!
    if (_mesure==nullptr || _mesure->_surf==nullptr) return;
    rw= _mesure->getScopeDiametre()/2.0;
    int iNbZone= _mesure->get_zones()->count()-1;

    // find top y. Take the lowest 2^n*65nm/2 that works if it exists...
    rh=65.0/2.0; // at least 65/2nm onthe display
    for (int i=0;i<iNbZone+1;i++) if (_mesure->_surf[i]>rh) rh= _mesure->_surf[i];
    double d= 65.0/2.0;
    for (int i=0; i<10; i++) if (rh<d) { rh= d; break; } else d*=2.0;
    // calculate y marks. max of one every 20 pixels. use a number that is a multiple of 5.
    int nb= int(h*0.8/20.0);
    double ystep= int(rh/nb/5)*5.0, ypos= 0.0;
    // draw horizontals in various colors
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
    QPoint *pts= new QPoint[iNbZone+1];
    for (int iZ=0; iZ<iNbZone+1; iZ++)
    {
        pts[iZ]= getP(_mesure->_Hz[iZ], _mesure->_surf[iZ]);
        painter->drawLine(pts[iZ].x(), addy, pts[iZ].x(), h-addy);
    }
    QPen p(QColor(0, 0, 0)); p.setWidth(2); painter->setPen(p);
    painter->drawPolyline(pts, iNbZone+1);
    delete[] pts;

    // Draw stats
    painter->drawText(0, fm.height(), " Lambda="+QString::number(_mesure->_Lambda, 'f', 2)+
                                      " LambdaRms="+QString::number(_mesure->_WeightedLambdaRms, 'f', 2)+
                                      " Lf/Ro="+QString::number(_mesure->_LfRoMax, 'f', 2)+
                                      " Strehl="+QString::number(_mesure->_WeightedStrehl, 'f', 2)+
                                      " dFocale="+QString::number(_mesure->_focale*10.0, 'f', 2)+
                                      " glass to remove="+QString::number(_mesure->_glassToRemove, 'f', 2)+"mm^3");

    QString s(" diametre:"+QString::number(_mesure->_scopeDiametre, 'f', 2)+" focale:"+QString::number(_mesure->_scopeFocale, 'f', 2)+(_mesure->_scopeSlitIsMoving?" mobile slit":" fixed slit")+" zones: ");
    for (int i=0; i<iNbZone; i++) s+= QString::number(_mesure->_Hz[i], 'f', 2)+" ("+QString::number(_mesure->_idealReadings[i], 'f', 2)+") "; s+= QString::number(_mesure->_Hz[iNbZone], 'f', 2);
	painter->drawText(0, h-4, s);

}

///////////////////////////////////////////////////////////////////////////
// Couder mesure calculations function
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
    checkNbMesures();
    int iNbZone= m_zones->count()-1;
    if (iNbZone<=1) return;

    if (_surf  !=nullptr) delete[] _surf  ; _surf  = new double[10+  iNbZone+1]; //10+ because I think that I have an overflow somewhere...
    if (_profil!=nullptr) delete[] _profil; _profil= new double[10+  iNbZone];
    if (_lf1000!=nullptr) delete[] _lf1000; _lf1000= new double[10+  iNbZone];
    if (_mesc  !=nullptr) delete[] _mesc  ; _mesc  = new double[10+  iNbZone];
    if (_Hz    !=nullptr) delete[] _Hz    ; _Hz    = new double[10+  iNbZone+1];
    if (_Hm4F  !=nullptr) delete[] _Hm4F  ; _Hm4F  = new double[10+  iNbZone];
    if (_RelativeSurface  !=nullptr) delete[] _RelativeSurface  ; _RelativeSurface  = new double[iNbZone];
	if (_idealReadings!=nullptr) delete[] _idealReadings; _idealReadings= new double[iNbZone];
    _Std=0.;

    double dRay=2.*_scopeFocale;

    double greenWaveNm=greenWave*1.e-9;
    double _dRoDif=1.22*greenWaveNm*dRay/_scopeDiametre/2.; //unit? TODO

	// Hz: zone edges
	// Hm: zone_middle
	// Hm4f: Hm/focale
	// Hm2R: Hm*Hm/focal.. Mesure that we are suposed to get here!
	// mesc: (mesure-Hm2R)*2(if moving)

    // get Hz 
    for(int i=0;i<iNbZone+1;i++) _Hz[i]= get_zones()->at(int(i))->_val;

    //compute Hm2R, Hm4F
    double a= 1e300, b= -1e300; // min and max
    for(int i=0;i<iNbZone;i++)
    {
        double _Hm=(_Hz[i+1]+_Hz[i])/2.0;
        _Hm4F[i]=_Hm/dRay/2.;
        double _Hm2R;
        if (_scopeSlitIsMoving)
             _Hm2R=-_scopeConical*sqr(_Hm)/2./dRay;
        else
            _Hm2R=-_scopeConical*(sqr(_Hm)/dRay + sqr(sqr(_Hm)) /2. /dRay/sqr(dRay));
		if (i==0) _idealReadings[0]= _Hm2R; else _idealReadings[i]= _Hm2R-_idealReadings[0];
        _mesc[i]=(m_mesures->at(int(i))->asDouble()-_Hm2R)*(_scopeSlitIsMoving?2.0:1.0);
        if (a>_mesc[i]) a= _mesc[i]; if (b<_mesc[i]) b= _mesc[i];
    }
	_idealReadings[0]= 0.0;

    //calcule les surfaces relatives: _RelativeSurface=% of total surface taken by a zone.
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
        double t=-_lf1000[i]/_scopeFocale/2.0*2000.;
        if (a>t) a= t; if (b<t) b= t;
        _profil[i+1]=_profil[i]+(_Hz[i+1]-_Hz[i])*t;
    }

    //on calcule le max et le min de la conique a ajuster
    a=a/_Hz[iNbZone]*2.0;
    b=b/_Hz[iNbZone]*2.0;
    double dReso=1./(_Hz[iNbZone]*_Hz[iNbZone]);

    // compute conique qui minimise le rms
	find_minimum(a,b,dReso,calc_less_rms);

    // compute conique qui minimise le ptv
	_focale= find_minimum(a,b,dReso,calc_less_ptv);
	if (!std::isnan(_adjustFocal)) calc_less_ptv(this, _adjustFocal); // override of the above, but we still want _focale...

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

// Dichotomy search for lowest point
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
    int iNbZone= pMes->m_zones->count()-1;
    for (int i=0;i<iNbZone+1;i++)
    {
        double dtemp;
        double denom= 1.-(pMes->_scopeConical+1.)*sqr(curv*pMes->_Hz[i]);

        if (denom>=0.) dtemp=( curv*sqr(pMes->_Hz[i]) ) / ( 1.+sqrt(denom) );
        else dtemp=curv*sqr(pMes->_Hz[i]); // cas degrade

        pMes->_surf[i]=(pMes->_profil[i]-dtemp)/2.;
        if (pMes->_surf[i]<min) min= pMes->_surf[i];
    }

    // shift between 0 and max-min
    double max= -1e300;
    for (int i=0;i<iNbZone+1;i++) { pMes->_surf[i]-=min; if (pMes->_surf[i]>max) max= pMes->_surf[i]; }
    return max; // return PTV
}

double CBSModelParabolizingWork::calc_less_rms(CBSModelParabolizingWork* pMes,double curv)
{
    double dtemp;
    //compute surface
    int iNbZone= pMes->m_zones->count()-1;
    for (int i=0;i<iNbZone+1;i++)
    {
        double denom= 1.-(pMes->_scopeConical+1.)*sqr(curv*pMes->_Hz[i]);
        if (denom>=0.) dtemp=( curv*sqr(pMes->_Hz[i]) ) / ( 1.+sqrt(denom) );
        else dtemp=curv*sqr(pMes->_Hz[i]); // bad case
        pMes->_surf[i]=(pMes->_profil[i]-dtemp)/2.;
    }
    //compute mean
    double dM=0.;
    for(int i=0;i<iNbZone;i++) dM+=(pMes->_surf[i]+pMes->_surf[i+1])/2.*pMes->_RelativeSurface[i];
    //compute var and stddev
    double dVar=0.;
    for(int i=0;i<iNbZone;i++) dVar+=sqr((pMes->_surf[i]+pMes->_surf[i+1])/2.-dM)*pMes->_RelativeSurface[i];
    double dStd=sqrt(dVar);
    return pMes->_Std=dStd;
}

double CBSModelParabolizingWork::calc_lf1000(CBSModelParabolizingWork* pMes,double h)
{
    double minl=1e300, maxl= -1e300;
    int iNbZone= pMes->m_zones->count()-1;
    for (int i=0;i<iNbZone;i++)
    {
        pMes->_lf1000[i]=1000.*(pMes->_mesc[i]-h)*pMes->_Hm4F[i];
        if (minl>pMes->_lf1000[i]) minl= pMes->_lf1000[i];
        if (maxl<pMes->_lf1000[i]) maxl= pMes->_lf1000[i];
    }
    return(maxl+minl);
}

//////////////////////////////////////////////////////////////////////////////
// Webcam filter to add couder screen and ilumination bar graph
extern QImage qt_imageFromVideoFrame(const QVideoFrame &f);
struct TZoneData { // structure that does the counting and image modification...
                   // pass it an image, a zone definition and it will count, for the right and left, the
                   // it will also modify the zones color to show what was counted
  static int const bucket= 4;
  int Z1[256/bucket], Z2[256/bucket];
  void count2(QImage *i, int x1, int x2, int y, int *zs, int type)
  {
    if (x2<0 || x1>=i->width()) return;
    if (x1<0) x1= 0; if (x2>=i->width()) x2= i->width()-1;
    if (y<0 || y>=i->height()) return;
    uchar *s= i->bits()+y*i->bytesPerLine()+4*x1;
    switch (type)
    {
    case 0: // Grayscale
      while (x1<=x2)
      {
        int g= (76*s[2]+150*s[1]+29*s[0])/256; // s[0]:b s[1]:g s[2]:r
        zs[g/bucket]++; 
        s[0]= s[1]= s[2]= g;
        s+= 4; x1++;
      }
      break;
    case 1: // (B+G+R)/3
      while (x1<=x2)
      {
        int g= (s[2]+s[1]+s[0])/3; // s[0]:b s[1]:g s[2]:r
        zs[g/bucket]++; 
        s[0]= s[1]= s[2]= g;
        s+= 4; x1++;
      }
      break;
    case 2: // B only
      while (x1<=x2) { zs[s[0]/bucket]++; s[1]= s[2]= 0; s+= 4; x1++; }
      break;
    case 3: // G only
      while (x1<=x2) { zs[s[1]/bucket]++; s[0]= s[2]= 0; s+= 4; x1++; }
      break;
    case 4: // R only
      while (x1<=x2) { zs[s[2]/bucket]++; s[0]= s[1]= 0; s+= 4; x1++; }
      break;
    }
  }
  void count(QImage *i, QPoint &c, double z1, double z2, int h, int type)
  {
    memset(Z1, 0, sizeof(Z1));
    memset(Z2, 0, sizeof(Z2));
    for (int y=h; --y>=0;)
    {
      // x= sqrt(d²-h²)
      int x1= 0;
      if (z1>=y) x1= int(sqrt(z1*z1-y*y));
      int x2= int(sqrt(z2*z2-y*y));
      count2(i, c.x()-x2, c.x()-x1, c.y()-y, Z1, type);
      count2(i, c.x()+x1, c.x()+x2, c.y()-y, Z2, type);
      if (y==0) break; // no double counting!!!!
      count2(i, c.x()-x2, c.x()-x1, c.y()+y, Z1, type);
      count2(i, c.x()+x1, c.x()+x2, c.y()+y, Z2, type);
    }
  }
};

static void ronchiCalcWithAllowableDeviation(double mirrorDia, double radiusOfCurvature, double gratingFreq, double gratingOffset, double scalingFactor, 
	uint32_t *imageData, int lineWidth, double allowableParabolicDeviation, bool includeDeviation, bool invertBands, double zoneSqrt);
	
void CBVirtualCouderOverlayInternal::draw(QImage &tempImage, CBSModelScope *_scope, double &dpi, QPoint &c)
{
	if (_scope==nullptr) return;
	TZoneData tt;
	// center
	imagew= tempImage.width(); imageh= tempImage.height(); // save data for later as they are needed by "user click"
	c= QPoint(int(imagew*_scope->_couderx),int(imageh*(inverted ? _scope->_coudery : (1.0-_scope->_coudery))));
	dpi= 100*_scope->_couderz/25.4; // scaling factor
	if (_scope->getDiametre()*dpi>2*imagew) _scope->_couderz= imagew*25.4/(100*_scope->getDiametre());
	if (_scope->_couderx<0 || _scope->_couderx>1) _scope->_couderx= 0.5;
	if (_scope->_coudery<0 || _scope->_coudery>1) _scope->_coudery= 0.5;
	if (_scope->get_zones()->count()!=0 && !_scope->_ronchi && imageh>256/tt.bucket) // no zones, no work....
	{
		double bot= 10; // find the "horizontal" cut line for the zones...
		if (_scope->get_zones()->at(0)->_val<1) bot= _scope->get_zones()->at(1)->_val*0.7;
		else bot= _scope->get_zones()->at(2)->_val*0.7;
		int z= _scope->getZone(); // current zone to "count"
		if (z<0 || z>_scope->get_zones()->count()-1) _scope->setZone(z=0); // sanity check
		int type= _scope->getVirtualCouderType(); // type of filtering to do...
		if (type<0 || type>4) _scope->setVirtualCouderType(type= 0); // more sanity check
		tt.count(&tempImage, c, _scope->get_zones()->at(z)->getVal()*dpi, _scope->get_zones()->at(z+1)->getVal()*dpi, bot*dpi, type); // count the pixels
		int maxz1= 1, maxz2= 1;
		for (int i=0; i<256/tt.bucket; i++) { if (tt.Z1[i]>maxz1) maxz1= tt.Z1[i]; if (tt.Z2[i]>maxz2) maxz2= tt.Z2[i]; } // max on each sides..
		if (maxz2>maxz1) maxz1= maxz2; // equalize We could have only one max as a mater of fact, but since I added this later on and was not sure if it was a good idea, I am leaving it here so far
		for (int i=0; i<256/tt.bucket; i++) // for each bucket fo ilumination counting...
		{
			uint32_t *s= (uint32_t*)(tempImage.bits()+(tempImage.height()-i-1)*tempImage.bytesPerLine()); // point in the picture to the correct line/pow
			int x= tt.Z1[i]*16/maxz1;                 // nb pixels to draw for left zone
			for (int j=16-x; --j>=0;) *s++= 0xffffffff; // draw white first
			while (--x>=0) *s++= 0xff000000;                   // then black
			*s++= 0xff0000ff;                          // and a blue vertical line
			int x2= tt.Z2[i]*16/maxz2;                // same for the right zone, but inverted
			x= x2; while (--x>=0) *s++= 0xff000000;
			for (int j=16-x2; --j>=0;) *s++= 0xffffffff;
		}
	}

	if (_scope->_ronchi)
	{
		int w= int(_scope->getDiametre()*dpi)/2*2;
		int line= (w+31)/32;
		if (ronchisize!=w || diam!=_scope->getDiametre() || roc!=_scope->getFocal()*2.0 || grad!= _scope->getGrading() || off!= _scope->getRonchiOffset())
		{
			if (ronchi!=nullptr) delete[] ronchi;
			ronchi= new uint32_t[line*w];
			memset(ronchi, 0, line*w*4);
			ronchisize= w;
			diam= _scope->getDiametre(); roc= _scope->getFocal()*2.0; grad= _scope->getGrading(); off= _scope->getRonchiOffset();
			ronchiCalcWithAllowableDeviation(diam, roc, grad, off, dpi, ronchi, line, 0.0, false, false, 0);
		}
		w/= 2;
		int dx1= c.x()-w, dx2= c.x()+w-1, dy1= c.y()-w, dy2= c.y()+w-1;
		int sx1= 0, sy1= 0;
		if (dx1<0) { sx1-= dx1; dx1= 0; }
		if (dy1<0) { sy1-= dy1; dy1= 0; }
		if (dx2>=imagew) dx2= imagew;
		if (dy2>=imageh) dy2= imageh;
		for (int y=dy1; y<dy2; y++)
		{
			uchar *s= tempImage.bits()+y*tempImage.bytesPerLine()+4*dx1;
			int sx= sx1;
			for (int x=dx1; x<dx2; x++)
			{
				if ((ronchi[sy1*line+(sx/32)]&(1<<(sx%32)))!=0) { s[0]>>=1; s[1]>>=1; s[2]>>=1; }
				s+= 4; sx++;
			}
			sy1++;
		}

	} else {
		ronchisize= 0;
		if (ronchi!=nullptr) { delete[] ronchi; ronchi= nullptr; }
	}

	dpi*= 25.4;
}

QVideoFrame CBScopeVirtualCouderRunnable::run(QVideoFrame *inputframe, const QVideoSurfaceFormat &surfaceFormat, RunFlags flags)
{
    (void)surfaceFormat; (void)flags;
    inputframe->map(QAbstractVideoBuffer::ReadOnly);
    QImage tempImage;
    // handle pause. Either start of pause or end of pause mode by taking a snapshot of the current frame.
    // then, set tempImage to current frame or paused frame
	bool pause= false;
	if (filter!=nullptr && filter->getScope()!=nullptr)  pause= filter->getScope()->getPause();
		
    if (filter!=nullptr && filter->pausedFrame!=nullptr) 
    {
      if (pause) tempImage= *filter->pausedFrame;
      else { delete filter->pausedFrame; filter->pausedFrame= nullptr; tempImage =qt_imageFromVideoFrame(*inputframe); }
    } else {
      tempImage =qt_imageFromVideoFrame(*inputframe);
      if (pause) filter->pausedFrame= new QImage(tempImage);
    }

    if (filter!=nullptr)
	{
		double dpi; QPoint c;
		filter->vco.draw(tempImage, filter->getScope(), dpi, c);
		if (filter->getScope()!=nullptr)
		{
  			QPainter p(&tempImage);
			filter->getScope()->paintCouder(&p, c, dpi, false, false, false); // Paint couder screen (ie: circles for each zones)
		}
	}
    inputframe->unmap();
    QVideoFrame outputFrame= QVideoFrame(tempImage);
    return outputFrame;
}



/////////////////////////////////////////
static bool calcBand(double scaledRadiusOfCurvature, double scaledMirrorZone, double deviationForScaledMirrorZone, double scaledGratingOffset, double x, double scaledLineWidth) 
{
	double zZone = scaledRadiusOfCurvature + (scaledMirrorZone - deviationForScaledMirrorZone) / scaledRadiusOfCurvature;
	double lZone = scaledRadiusOfCurvature + scaledGratingOffset * 2 - zZone;
	double uZone = abs(lZone * x / zZone);
	int tZone = int((uZone / scaledLineWidth) + 0.5);
	return tZone % 2 == 0;
};

static double calcDeviationForScaledMirrorZone(double scaledMirrorZone, double scaledMirrorRadiusSquared, double allowableParabolicDeviation, double maxScaledMirrorZone, double zoneSqrt) 
{
	if (scaledMirrorZone / scaledMirrorRadiusSquared < zoneSqrt) {
		// inside zone: shape gradually from perfect center to max allowableParabolicDeviation at zone
		return allowableParabolicDeviation * maxScaledMirrorZone * scaledMirrorZone / maxScaledMirrorZone / zoneSqrt;
	}
	// outside zone: shape gradually from perfect edge to max allowableParabolicDeviation at zone
	return allowableParabolicDeviation * maxScaledMirrorZone * (scaledMirrorRadiusSquared - scaledMirrorZone) / maxScaledMirrorZone / (1 - zoneSqrt);
};

// from algorithm in Sky and Telescope magazine: trace light rays through grating;
// correction factor of 1 = parabola
static void ronchiCalcWithAllowableDeviation(double mirrorDia, double radiusOfCurvature, double gratingFreq, double gratingOffset, double scalingFactor, 
			uint32_t *imageData, int lineWidth, double allowableParabolicDeviation, bool includeDeviation, bool invertBands, double zoneSqrt) 
{
	int scaledMirrorRadius = mirrorDia / 2 * scalingFactor;
	int scaledMirrorRadiusSquared = scaledMirrorRadius * scaledMirrorRadius;
	double scaledRadiusOfCurvature = radiusOfCurvature * scalingFactor;
	double scaledGratingOffset = gratingOffset * scalingFactor;
	double scaledLineWidth = scalingFactor / (2 * gratingFreq);

	for (int y = 0; y < scaledMirrorRadius; y++) 
	{
		int ySquared = y * y;
		int maxScaledMirrorZone = ySquared + scaledMirrorRadiusSquared;
		for (int x = 0; x < scaledMirrorRadius; x++) {
			int scaledMirrorZone = ySquared + x * x;
			if (scaledMirrorZone > scaledMirrorRadiusSquared) break;
			// for spherical mirror, Z=RC;
			double z = scaledRadiusOfCurvature + scaledMirrorZone / scaledRadiusOfCurvature;
			// offset*2 for light source that moves with Ronchi grating
			double l = scaledRadiusOfCurvature + scaledGratingOffset * 2 - z;
			// u = projection of ray at scaledMirrorRadius onto grating displaced from RC by gratingOffset
			double u = abs(l * x / z);
			// test for ray blockage by grating
			int t = int((u / scaledLineWidth) + 0.5);
			bool band = t % 2 == 0;

			if (includeDeviation) {
				double deviationForScaledMirrorZone = calcDeviationForScaledMirrorZone(scaledMirrorZone, scaledMirrorRadiusSquared, allowableParabolicDeviation, maxScaledMirrorZone, zoneSqrt);
				band = calcBand(scaledRadiusOfCurvature, scaledMirrorZone, deviationForScaledMirrorZone, scaledGratingOffset, x, scaledLineWidth);
			}
			if ((band && !invertBands) || (!band && invertBands))
			{
#define pixon(y, x) imageData[(y)*lineWidth+(x)/32]|= 1<<((x)%32)
				pixon((scaledMirrorRadius+y), scaledMirrorRadius+x);
				pixon((scaledMirrorRadius+y), scaledMirrorRadius-x);
				pixon((scaledMirrorRadius-y), scaledMirrorRadius+x);
				pixon((scaledMirrorRadius-y), scaledMirrorRadius-x);
#undef pixon
			}
		}
	}
};

void CBScopeCouderOverlay::paint(QPainter *painter)
{
	QSizeF itemSize = size();
	int w= int(itemSize.width());
	int h= int(itemSize.height());
	if (_scope==nullptr) return;
	int iw= img.width(), ih= img.height();
	if (_source=="" || ih==0 || iw==0 || w==0 || h==0) return;
	QImage i(iw, ih, QImage::Format_ARGB32);
	QPainter p(&i);
	p.drawImage(QPoint(0,0), img);
	double dpi; QPoint c;
	vco.draw(i, _scope, dpi, c);
	if (_scope!=nullptr) _scope->paintCouder(&p, c, dpi, false, false, false); // Paint couder screen (ie: circles for each zones)
	painter->setBrush(QBrush(QColor(255,255,255)));
	painter->setPen(QPen(QColor(255,255,255)));
	painter->drawRect(0, 0, w, h);
	double rd= w/double(h), rs= iw/double(ih);
	if (rd>rs)
	{
		int dw= (iw*h)/ih;
		painter->drawImage(irect= QRect((w-dw)/2, 0, dw, h), i);
	} else {
		int dh= (ih*w)/iw;
		painter->drawImage(irect= QRect(0, (h-dh)/2, w, dh), i);
	}
}
