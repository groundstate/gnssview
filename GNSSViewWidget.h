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


#ifndef __GNSS_VIEW_WIDGET_H_
#define __GNSS_VIEW_WIDGET_H_

#include <QDateTime>
#include <QGLWidget>
#include <QString>

class Colour;
class Sun;
class SkyModel;
class GLText;
class GNSSSV;

class QTimer;

class ConstellationProperties{
	public:
		ConstellationProperties(int);
		int id;
		int svcnt;
		int maxsv;
		double x0;
		bool active;
		GLfloat histColour[4];
		QString label;
		GLText *GLlabel;
		int svIDmin,svIDmax;
		QString idLabel;
		QList<GLText *> svLabels;
};

class GNSSViewWidget: public QGLWidget
{
	Q_OBJECT

	public:
		
		GNSSViewWidget(QWidget *parent=0,QList<GNSSSV *> *b=NULL);
		~GNSSViewWidget();
		
		void setForegroundImage(QString,double,double);
		void setNightSkyImage(QString);
		void setLocation(double,double);
		void setReceiver(QString);
		void setAnimation(int,double,int,bool);
		void addConstellation(int);
		
	public slots:
		
		void update(QDateTime &);
		
		void toggleForeground();
		void offsetTime(int);
		
	protected:

		virtual void 	initializeGL ();
		virtual void 	paintGL ();
		virtual void 	resizeGL ( int w, int h );
	
	private slots:
		
		void animate();
		
	private:
		
		void initTextures();
		void initLayout();
		
		void drawGrid();
		void drawSky();
		void drawSun();
		void drawForeground();
		void drawBirds();
		void drawInfo();
		void drawSignalBars();
		
		bool gridOn;
		bool rotate;
		bool animatedSky;
		bool showForeground;
		bool signalLevels;
		
		double latitude,longitude;
		
		int nrot;
		double phi,dphi,fov,phi0,phi1,trot;
		int fps;
		QTimer *animationTimer;
		
		Sun *sunModel;
		double gammaCorrection_;
		
		QString receiver;
		GLText *receiverLabel;
		
		SkyModel *skyModel;
		QString foreground;
		double minElevation,maxElevation;
		QString nightSky;
		QDateTime lastSkyUpdate;
		int skyUpdateInterval;
		bool smooth;
		
		Colour** skyColour;
		int naz,nel;
		
		GLuint fgtex;
		int fgWidth,fgHeight; 
		GLuint sattex;
		int satWidth,satHeight;
		GLuint suntex;
		int sunWidth,sunHeight;
		GLuint nighttex;
		int nightWidth,nightHeight;
		
		double barWidth;
		double sideMargin; // margin at the sides
		double barMargin; // separation between bars
		double constellationNameSpc; // 
		double constellationMargin;
	
		QList<GNSSSV *> *birds;
		QList<ConstellationProperties *> constellations;
		
		QList<GLText *> compassLabels;
		
		// debugging stuff
		int tOffset; // in hours
};

#endif