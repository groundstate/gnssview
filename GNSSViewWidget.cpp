//
// gnssview - a program for displaying GNSS satellite paths
//
// The MIT License (MIT)
//
// Copyright (c)  2014  Michael J. Wouters
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.


#include <cmath>
#include <iostream>

#include <GL/glu.h> 

#include <QDebug>
#include <QTimer>
#include <Qt/QtXml>

#include "GLText.h"
#include "GNSSSV.h"
#include "GNSSViewApp.h"
#include "GNSSViewWidget.h"
#include "Sun.h"
#include "SkyModel.h"

#define CHECK_GLERROR() \
{ \
	GLenum err = glGetError(); \
	if (err != GL_NO_ERROR) \
	{ \
		std::cerr << "[" << __FILE__ << " " << __FUNCTION__<< " " << __LINE__<< "] GL error: " << gluErrorString(err) << std::endl;\
	}\
}\

#define EL1  90

#define HORIZON_OFFSET 0.1

ConstellationProperties::ConstellationProperties(int c)
{
	id=c;
	svcnt=0;
	x0=0;
	active=false;
	switch(id){
		case GNSSSV::Beidou:
			maxsv=12;
			histColour[0]=1;histColour[1]=0.753;histColour[2]=0.796;histColour[3]=0.5;
			label="Beidou";
			break;
		case GNSSSV::GPS:
			maxsv=12;
			histColour[0]=1;histColour[1]=0.549;histColour[2]=0.0;histColour[3]=0.5;
			label="GPS";
			break;
		case GNSSSV::Galileo:
			maxsv=6;
			histColour[0]=0.5;histColour[1]=1.0;histColour[2]=0.0;histColour[3]=0.5;
			label="Galileo";
			break;
		case GNSSSV::GLONASS:
			maxsv=12;
			histColour[0]=1;histColour[1]=0.843;histColour[2]=0.0;histColour[3]=0.5; 
			label="GLONASS";
			break;
		case GNSSSV::QZSS:
			maxsv=4;
			histColour[0]=0.867;histColour[1]=0.627;histColour[2]=0.867;histColour[3]=0.5; 
			label="QZSS";
			break;
		case GNSSSV::SBAS:
			maxsv=4;
			histColour[0]=0.98;histColour[1]=0.941;histColour[2]=0.901;histColour[3]=0.5; 
			label="SBAS";
			break;
	}
}

GNSSViewWidget::GNSSViewWidget(QWidget *parent,QList<GNSSSV *> *b):QGLWidget(parent)
{
	gridOn=true;
	animatedSky=true;
	showForeground=true;
	signalLevels=true;
	smooth=true;
	
	minElevation=-10.0;
	maxElevation=30.0;
	
	sunModel = new Sun(-33.87,151.21);
	skyModel = new SkyModel();
	gammaCorrection_=2.0;
	birds=b;
	
	tOffset=0;
	
	lastSkyUpdate = QDateTime::currentDateTime();
	lastSkyUpdate = lastSkyUpdate.addSecs(-999);
	skyUpdateInterval=60;
	
	receiverLabel=NULL;
	
	for (int i=GNSSSV::Beidou;i<=GNSSSV::SBAS;i++){
		ConstellationProperties *cprop= new ConstellationProperties(i);
		constellations.push_back(cprop);
	}
	
	naz=90;
	nel=90;
	skyColour = new Colour*[(nel+1)*(naz+1)];
	for (int i=0;i<(nel+1)*(naz+1);i++)
		skyColour[i]=new Colour();
	
	sideMargin=0.03; // margin at the sides
	barMargin=0.003; // separation between bars
	constellationNameSpc=0.015; // 
	constellationMargin=0.015;
	
	nrot=0;
	fps=24;
	trot=40;
	fov=1920.0*(90-minElevation)/1080.0;
	dphi=360.0/(fps*trot);
	phi0=0.0;
	phi1=fov;
	
	animationTimer=new QTimer(this);
	connect(animationTimer,SIGNAL(timeout()),this,SLOT(animate()));
	QDateTime now = QDateTime::currentDateTime();
	animationTimer->start(1000.0/fps); 
	
}

GNSSViewWidget::~GNSSViewWidget()
{
	delete sunModel;
	delete skyModel;
}

void GNSSViewWidget::setForegroundImage(QString img,double min,double max)
{
	foreground=img;
	minElevation=min;
	maxElevation=max;
	fov=1920.0*(90-minElevation)/1080.0;
}

void GNSSViewWidget::setNightSkyImage(QString img)
{
	nightSky=img;
}

void GNSSViewWidget::setLocation(double lat,double lon)
{
	latitude=lat;
	longitude=lon;
	sunModel->setLocation(lat,lon);
}

void GNSSViewWidget::setReceiver(QString r){
	receiver=r;
}

void GNSSViewWidget::setAnimation(int framesPerSecond,double rotationalPeriod,int skyUpdate, bool smoothTracks){
	fps=framesPerSecond;
	trot=rotationalPeriod;
	dphi=360.0/(fps*trot);
	skyUpdateInterval=skyUpdate;
	smooth=smoothTracks;
}

void  GNSSViewWidget::addConstellation(int c)
{
	constellations[c]->active=true;
	qDebug() << "added constellation " << c;
}
//
//
//

void GNSSViewWidget::animate()
{
	phi0 += dphi;
	phi1 = phi0+fov;
	if (phi0 >= 360.0){
		phi0-=360.0;
		phi1=phi0+fov;
	}
	updateGL();
	animationTimer->start(1000.0/fps);
}

void GNSSViewWidget::update(QDateTime &)
{
	// update the animations as necessary
	//updateGL();
}

void GNSSViewWidget::toggleForeground()
{
	showForeground=!showForeground;
	updateGL();
}

void GNSSViewWidget::offsetTime(int hours)
{
	// need to trigger calculation of a new sky image
	lastSkyUpdate = QDateTime::currentDateTime();
	lastSkyUpdate = lastSkyUpdate.addSecs(-999);
	
	tOffset = hours;
	updateGL();
}


//
// Protected methods
//


void GNSSViewWidget::initializeGL()
{
	//glEnable(GL_DEPTH_TEST);
	
	glEnable(GL_TEXTURE_2D);
	
	QImage im(foreground);
	QImage glim = convertToGLFormat(im);
	fgWidth = glim.width();
	fgHeight = glim.height();
	
	glGenTextures(1,&fgtex);
	glBindTexture(GL_TEXTURE_2D,fgtex);
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, glim.width(), glim.height(), 0,
      GL_RGBA, GL_UNSIGNED_BYTE, glim.bits());
	
	QString r = app->locateResource("gpssat.png");
	QImage sim(r);
	glim = convertToGLFormat(sim);
	satWidth =glim.width();;
	satHeight=glim.height();
	
	glGenTextures(1,&sattex);
	glBindTexture(GL_TEXTURE_2D,sattex);
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, glim.width(), glim.height(), 0,
      GL_RGBA, GL_UNSIGNED_BYTE, glim.bits());
	
	r = app->locateResource("sun.png");
	QImage sun(r);
	glim = convertToGLFormat(sun);
	sunWidth =glim.width();;
	sunHeight=glim.height();
	
	glGenTextures(1,&suntex);
	glBindTexture(GL_TEXTURE_2D,suntex);
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, glim.width(), glim.height(), 0,
      GL_RGBA, GL_UNSIGNED_BYTE, glim.bits());
	
	QImage night(nightSky);
	glim = convertToGLFormat(night);
	nightWidth =glim.width();;
	nightHeight=glim.height();
	
	glGenTextures(1,&nighttex);
	glBindTexture(GL_TEXTURE_2D,nighttex);
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, glim.width(), glim.height(), 0,
      GL_RGBA, GL_UNSIGNED_BYTE, glim.bits());
			
	CHECK_GLERROR();
	
	glDisable(GL_TEXTURE_2D);
	
	initTextures();

}

void 	GNSSViewWidget::paintGL()
{
	if (animatedSky){
		QDateTime now=QDateTime::currentDateTime();
		if (lastSkyUpdate.secsTo(now) > skyUpdateInterval){ // calculate a new sky model
			QDateTime utc = now.toUTC();
			utc=utc.addSecs(tOffset*3600);
			sunModel->update(utc.date().year(),utc.date().month(),utc.date().day()
				,utc.time().hour(),utc.time().minute(),utc.time().second());
			double az,el;
			sunModel->position(&az,&el);
			if (el>-7){
				skyModel->setSolarPosition(az,el);
				for (int j=0;j<=nel;j++){
					for (int i=0;i<=naz;i++){
						az=((double) i/ (double) naz)*360.0;						
						Colour c = skyModel->colour(az,90.0*j/nel);
						Colour cg = c.gammaCorrect(gammaCorrection_);
						*skyColour[j*naz+i]=cg;
					}
				}
			}
			lastSkyUpdate=now;
		}
	}
	
	glClearColor(0.75,0.75,0.1,0);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	// Need to do this because we buggerize around elsewhere with the viewport
	glViewport(0,0,width(),height()); // set physical size
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	gluOrtho2D(phi0,phi1,minElevation,EL1);
	
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
	glPushMatrix();
	
	CHECK_GLERROR();
	if (animatedSky) {
		drawSky();
		drawSun();
	}
  drawBirds();
	if (showForeground) drawForeground();
	if (signalLevels) drawSignalBars();
	drawInfo();
	if (gridOn) drawGrid();
	
	glPopMatrix();
	
}

void 	GNSSViewWidget::resizeGL( int w, int h )
{
	glViewport(0,0,w,h); // set physical size
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(phi0,phi1,minElevation,EL1);
	initLayout();
}

//
//
//

void 	GNSSViewWidget::drawGrid()
{
	glColor3f(1,1,1);
	glBegin(GL_LINES);
	double ht;
	for (int i=0;i<36;++i){ 
		if (i % 9 == 0)
			ht=0.03;
		else
			ht=0.01;
		double x0=i*10.0;
		if (phi1>360 && x0+360<phi1){
			x0+=360;
		}
		glVertex2f(x0,(1.0-ht)*90);
		glVertex2f(x0,90);
		
	}
	
	for (int i=0;i<9;++i){
		double alt = i*10;
		glVertex2f(phi0,alt);
		glVertex2f(phi0+0.01*fov,alt);
		glVertex2f(phi1-0.01*fov,alt);
		glVertex2f(phi1,alt);
	}
	glEnd();
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0,width()-1,0,height()-1);
	
	glMatrixMode(GL_MODELVIEW);
	
	glEnable (GL_BLEND); 
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	
	for (int i=0;i<4;i++){
		glPushMatrix();
		double x0=i*90;
		if (phi1>360 && x0+360<phi1)
			x0+=360;
		glTranslatef( (x0-phi0)/fov*(width()-1)- compassLabels[i]->w/2.0,(1.0-0.03)*height()-1-compassLabels[i]->h,0);
		compassLabels[i]->paint();
		glPopMatrix();
	}
	
	CHECK_GLERROR();
	
	glBindTexture(GL_TEXTURE_2D,0);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	
	CHECK_GLERROR();
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(phi0,phi1,minElevation,EL1);
	
	glMatrixMode(GL_MODELVIEW);
}

void GNSSViewWidget::drawInfo()
{
	if (receiverLabel==NULL){
		QFont f;
		f.setPointSize(18);
		receiverLabel=new GLText(this,receiver,f);
	}
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0,width()-1,0,height()-1);
	
	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_TEXTURE_2D);
	
	
	glBindTexture(GL_TEXTURE_2D,0);
	glDisable(GL_TEXTURE_2D);
	
	CHECK_GLERROR();
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(phi0,phi1,minElevation,EL1);
	
	glMatrixMode(GL_MODELVIEW);

}

void GNSSViewWidget::drawSky()
{
	double az,el;
	
	sunModel->position(&az,&el);
	
	if (animatedSky){
		
		glPushAttrib(GL_POLYGON_BIT);
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
		
		if (el <= -7) // nighttime
		{
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D,nighttex);
	
			glBegin(GL_QUADS);
			double imsf=1.0/360.0;

			glTexCoord2f(phi0*imsf+0.5,0);
			glVertex2f(phi0,0);
	
			glTexCoord2f(phi1*imsf+0.5,0);
			glVertex2f(phi1,0);
	
			glTexCoord2f(phi1*imsf+0.5,1.0);
			glVertex2f(phi1,EL1);
	
			glTexCoord2f(phi0*imsf+0.5,1.0);
			glVertex2f(phi0,EL1);
			glEnd();
			
			glDisable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D,0);
			
		}
		else{
		
			double dd = rint(360.0/naz);

			int irphi0=floor(phi0/dd);	
			int irphi1=ceil(phi1/dd);
			
			for (int j=0;j<nel;j++){
				el = (double) j/ (double) nel;
				double elp1 = (j+1.0)/nel;
				glBegin(GL_TRIANGLE_STRIP);
				for (int i=irphi0;i<=irphi1;i++)
				{
					double phi=i*dd;
					int indx = i % naz; 
					Colour *c= skyColour[(j+1)*naz+indx];
					glColor3f(c->x,c->y,c->z);
					glVertex2f(phi,elp1*90);
					c= skyColour[j*naz+indx];
					glColor3f(c->x,c->y,c->z);
					glVertex2f(phi,el*90);
				}
				glEnd();
			}
		}
		glPopAttrib();
	}
	else{ // flat colour
		glBegin(GL_QUADS);
		glColor3f(0.1,0.1,0.8);
		glVertex2f(phi1,0);
		glVertex2f(phi0,0);
		glColor3f(0.0,0.0,0.2);
		glVertex2f(phi0,EL1);
		glVertex2f(phi1,EL1);
		glEnd();
	}
	CHECK_GLERROR();
}

void GNSSViewWidget::drawSun()
{
	double az,alt;
	
	sunModel->position(&az,&alt);
	
	if (alt < 0) return;
	if (phi1 > 360 && az + 360 < phi1) // fix up coordinates
			az+= 360.0;
	
	// Change coordinate system for drawing the sun pixmap
	// since this shouldn't be scaled
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0,width()-1,0,height()-1);
	
	glMatrixMode(GL_MODELVIEW);
	
	glEnable (GL_BLEND); 
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,suntex);
	
	GLfloat x=(az-phi0)/fov*(width()-1)-sunWidth/2.0;
	GLfloat y=(alt-minElevation)/(EL1-minElevation)*(height()-1)-sunHeight/2.0;
		
	glBegin(GL_QUADS);
	glTexCoord2f(0,0);
	glVertex2f(x,y);
	
	glTexCoord2f(1.0,0);
	glVertex2f(x+sunWidth-1,y);
	
	glTexCoord2f(1.0,1.0);
	glVertex2f(x+sunWidth-1,y+sunHeight-1);
	
	glTexCoord2f(0,1.0);
	glVertex2f(x,y+sunHeight-1);
	glEnd();
	
	glBindTexture(GL_TEXTURE_2D,0);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(phi0,phi1,minElevation,EL1);
	
	glMatrixMode(GL_MODELVIEW);
	
}

void GNSSViewWidget::drawForeground()
{
	glEnable (GL_BLEND); 
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,fgtex);
	
	// We see fov, starting at phi0, ending at phi1
	double imsf=1.0/360.0;
	glColor3f(1,1,1);
	
	glBegin(GL_QUADS);
	
	glTexCoord2f(phi0*imsf+0.5,0);
	glVertex2f(phi0,minElevation);
	
	glTexCoord2f(phi1*imsf+0.5,0);
	glVertex2f(phi1,minElevation);
	
	glTexCoord2f(phi1*imsf+0.5,1.0);
	glVertex2f(phi1,maxElevation);
	
	glTexCoord2f(phi0*imsf+0.5,1.0);
	glVertex2f(phi0,maxElevation);
	
	glEnd();
	
	glBindTexture(GL_TEXTURE_2D,0);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}

void GNSSViewWidget::drawBirds()
{
	
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(5.0);
	
	for (int i=0;i<birds->size();++i){
		//if (birds->at(i)->PRN !=10)
		//	continue;
		//qDebug()<< "###";
		int c = birds->at(i)->constellation;
		// Could filter out birds which are not visible but this looks a bit icky visually because they and their trails
		// will pop in and out. So we won't do that.
	
		int npts=birds->at(i)->az.size(); // guaranteed non-zero
		double deltaAlpha=0.9;
		if (npts != 1)
			deltaAlpha /= (npts-1.0);
		
		int jdrop=npts; // need to track dropped points so that they're not included ?
		bool dropping=true; // to catch the first point
		
		for (int j=npts-1;j>=0;j--){
			
			double el;
			double az =  birds->at(i)->az[j];
			
			if (phi1 > 360 &&  az+360 < phi1)
				az+=360.0;
			if (!(az >=phi0 && az<=phi1)){ // point not visible so skip it
				jdrop=j;
				if (!dropping){
					dropping=true;
					glEnd(); // of the GL_LINE_STRIP
				}
				continue;
			}
			
			glColor4f(constellations[c]->histColour[0],constellations[c]->histColour[1],constellations[c]->histColour[2],0.1+j*deltaAlpha);
			
			if (dropping){ // resume drawing
				dropping=false;
				glBegin(GL_LINE_STRIP);
				//qDebug() << az << " " << birds->at(i)->elev[j] << " " << phi0 << " " << phi1 << "1";
				glVertex2f(az,birds->at(i)->elev[j]);
				continue;
			}
			
			el=birds->at(i)->elev[j];
			
			if (smooth && j<= jdrop-3){ // can smooth ...
				double az1 =  birds->at(i)->az[j+1];
				if (phi1 > 360 &&  az1+360 < phi1)
					az1+=360.0;
				double az2 =  birds->at(i)->az[j+2];
				if (phi1 > 360 &&  az2+360 < phi1)
					az2+=360.0;
				az= (az+az1+az2)/3.0;
				el=(birds->at(i)->elev[j]+ birds->at(i)->elev[j+1]+ birds->at(i)->elev[j+2])/3.0;
				//qDebug() << az << " " << birds->at(i)->elev[j] << " " << phi0 << " " << phi1 << "2";
			}
			//else
				//qDebug() << az << " " << birds->at(i)->elev[j] << " " << phi0 << " " << phi1 << "3";
			glVertex2f(az,el);
		
		}
		if(!dropping) glEnd(); // finish the line
	}
	
	glDisable(GL_LINE_SMOOTH);
	glLineWidth(1.0);
	
	CHECK_GLERROR();
	
	// Change coordinate system for drawing text since we are using pixmaps for text and these
	// shouldn't be scaled
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0,width()-1,0,height()-1);
	
	glMatrixMode(GL_MODELVIEW);
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,sattex);
	CHECK_GLERROR();
	glPushMatrix();
	glBegin(GL_QUADS);
	for (int i=0;i<birds->size();++i){
		int sz = birds->at(i)->az.size() -1 ;
		GLfloat x0 =  birds->at(i)->az[sz];
		if (phi1 > 360 &&  x0+360 < phi1)
			x0+=360.0;
		GLfloat x=(x0-phi0)/fov*(width()-1)-satWidth/2.0;
		GLfloat y=(birds->at(i)->elev[sz]-minElevation)/(EL1-minElevation)*(height()-1)-satHeight/2.0;
		
		glTexCoord2f(0,0);
		glVertex2f(x,y);

		glTexCoord2f(1.0,0);
		glVertex2f(x+satWidth-1,y);

		glTexCoord2f(1.0,1.0);
		glVertex2f(x+satWidth-1,y+satHeight-1);

		glTexCoord2f(0,1.0);
		glVertex2f(x,y+satHeight-1);
		
	}
	glEnd();
	glPopMatrix();
	
	CHECK_GLERROR();
	
	glBindTexture(GL_TEXTURE_2D,0);
	
	for (int i=0;i<birds->size();++i){
		int sz = birds->at(i)->az.size() -1 ;
		GLfloat phi =  birds->at(i)->az[sz];
		if (phi1 > 360 &&  phi+360 < phi1)
			phi+=360.0;
		int x=(phi-phi0)/fov*(width()-1)+satWidth/2.0;
		//if (x > width()-1 -usiLabel[birds->at(i)->PRN]->w)
		//	x=(birds->at(i)->az[sz]-phi0)/fov*(width()-1)-satWidth/2.0-usiLabel[birds->at(i)->PRN]->w;
		int y=(birds->at(i)->elev[sz]-minElevation)/(EL1-minElevation)*(height()-1)-usiLabel[birds->at(i)->PRN]->h/2.0;
		if (y>height()-1 -usiLabel[birds->at(i)->PRN]->h)
			y-=usiLabel[birds->at(i)->PRN]->h/2.0;
		
		glPushMatrix();
		glTranslatef(x,y,0);
		usiLabel[birds->at(i)->PRN]->paint();
		glPopMatrix();
	}
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	
	CHECK_GLERROR();
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(phi0,phi1,minElevation,EL1);
	
	glMatrixMode(GL_MODELVIEW);
}

void GNSSViewWidget::drawSignalBars()
{
	
	double signalBMargin=0.01;
	double signalVSpace=0.15;
	double signalHeight=(signalVSpace-signalBMargin)*(height()-1);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0,width()-1,0,height()-1);
	
	CHECK_GLERROR();
	
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	double voffset=signalBMargin;
	int cnt=0;
	GLfloat *col;
	
	for (int c=GNSSSV::Beidou;c<=GNSSSV::SBAS;c++)
		constellations[c]->svcnt=0;
	
	// signal bars
	for (int i=0;i<birds->size();++i){
		int c = birds->at(i)->constellation;
		double sn = signalHeight*birds->at(i)->sn; // prescaled [0,1]
		
		constellations[c]->svcnt++;
		cnt=constellations[c]->svcnt;
		if (cnt > constellations[c]->maxsv) continue;
		
		col = constellations[c]->histColour;
		double x0=(constellations[c]->x0+(cnt-1)*barWidth + (cnt-1)*barMargin)*(width()-1.0);
		double y0= voffset*(height()-1);
		
		glDisable(GL_BLEND);
		glBegin(GL_POLYGON);
		glColor4f(col[0]*0.5,col[1]*0.5,col[2]*0.5,col[3]);
		glVertex2f(x0,y0);
		glVertex2f(x0+barWidth*(width()-1),y0);
		glColor4fv(col);
		glVertex2f(x0+barWidth*(width()-1),y0+sn);
		glVertex2f(x0,y0+sn);
		glEnd();
		glEnable(GL_BLEND);
		
		glEnable(GL_TEXTURE_2D);
		glPushMatrix();
		glTranslatef(x0+(barWidth*(width()-1)+ usiLabel[birds->at(i)->PRN]->ascent)/2.0,y0+6,0);
		glRotatef(90,0,0,1);
		usiLabel[birds->at(i)->PRN]->paint();
		glPopMatrix();
		glDisable(GL_TEXTURE_2D);
	
	}
	
	// constellation names
	glEnable(GL_TEXTURE_2D);
	double y0=voffset*(height()-1);
	for (int c=GNSSSV::Beidou;c<=GNSSSV::SBAS;c++){
		if (constellations[c]->active){
			glPushMatrix();
			double x0=constellations[c]->x0*(width()-1)-2*constellations[c]->GLlabel->descent; // good enough
			glTranslatef(x0,y0,0);
			glRotatef(90,0,0,1);
			constellations[c]->GLlabel->paint();
			glPopMatrix();
		}
	}
	
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	
	CHECK_GLERROR();
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(phi0,phi1,minElevation,EL1);
	
	glMatrixMode(GL_MODELVIEW);
}

void GNSSViewWidget::initTextures(){
	QFont f;
	f.setPointSize(18);

	for (int i=0;i<128;i++){
		QString txt = QString::number(i);
		GLText *glt = new GLText(this,txt,f);
		usiLabel.append(glt);
	}

	f.setPointSize(24);
	
	for (int c=GNSSSV::Beidou;c<=GNSSSV::SBAS;c++){
		ConstellationProperties *cprop= constellations[c];
		cprop->GLlabel= new GLText(this,cprop->label,f);
	}	
	
	compassLabels.append(new GLText(this,"N",f));
	compassLabels.append(new GLText(this,"E",f));
	compassLabels.append(new GLText(this,"S",f));
	compassLabels.append(new GLText(this,"W",f));
}

void GNSSViewWidget::initLayout(){
	
	// For the moment, just laying out the signal bar regio
	
	double x0=sideMargin;
	int maxsats=0;
	
	// determine horizontal space for each signal bar
	int nconst=0;
	for (int c=GNSSSV::Beidou;c<=GNSSSV::SBAS;c++){
		if (constellations.at(c)->active){
			maxsats += constellations.at(c)->maxsv;
			nconst++;
		}
	}
	
	if (maxsats==0) return;
	
	barWidth=(1.0-2.0*sideMargin-nconst*constellationNameSpc-(maxsats-nconst)*barMargin-(nconst-1.0)*constellationMargin)/maxsats;
	if ( barWidth * (width()-1) > 48){ // just in case we are only looking at a few SVs
		barWidth = 48.0/width();
	}
	
	for (int c=GNSSSV::Beidou;c<=GNSSSV::SBAS;c++){
		if (constellations.at(c)->active){
			constellations.at(c)->x0=x0;
			x0+=constellationNameSpc+constellations.at(c)->maxsv*barWidth+(constellations.at(c)->maxsv-1)*barMargin+constellationMargin;
		}
	}
	
	
}


