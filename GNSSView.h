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

#ifndef __GNSSVIEW_H_
#define __GNSSVIEW_H_

#include <QList>
#include <QWidget>
#include <QDateTime>
#include <QList>

#include "GNSSSV.h"

class QAction;
class QLabel;
class QTimer;
class QUdpSocket;

class GNSSViewWidget;
class PowerManager;

class GNSSView : public QWidget
{
    Q_OBJECT

	public:

		GNSSView(QStringList &);

	protected slots:

		virtual void 	keyPressEvent (QKeyEvent *);
		virtual void 	mouseMoveEvent (QMouseEvent * );
		virtual void 	mousePressEvent (QMouseEvent * );
	
	private slots:
	
		void updateView();
	
		void toggleFullScreen();
		void togglePowerManagement();
		void offsetTime();
		void quit();
		
		void createContextMenu(const QPoint &);
		
		void readPendingDatagrams();
		
	private:
  	
		void readConfig(QString s);
		void createActions();
		
		bool fullScreen;

		GNSSViewWidget *view;
		PowerManager   *powerManager;
		
		QTimer  *updateTimer;
		QAction *toggleFullScreenAction;
		QAction *powerManAction;
		QAction *quitAction;
		
		QAction *toggleForegroundAction;
		QAction *offsetTimeAction;
		
		QUdpSocket *udpSocket;
		QString address;
		int     port;
		
		double latitude,longitude;
		
		double snMax;
		
		QList<GNSSSV *> birds;
		QList<int> constellations;
};

#endif
