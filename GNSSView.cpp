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

#include <stdlib.h>
#include <sys/timex.h>

#include <iostream>

#include <QtGui>
#include <Qt/QtNetwork>
#include <Qt/QtXml>
#include <QRegExp>
#include <QUdpSocket>

#include <sys/types.h> // flimflummery
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "GNSSView.h"
#include "GNSSViewWidget.h"
#include "PowerManager.h"

#define VERSION_INFO  "v1.0"
#define TRACKING_TIMEOUT 120

GNSSView::GNSSView(QStringList & args)
{
	fullScreen=true;
	
	for (int i=1;i<args.size();i++){ // skip the first
		if (args.at(i) == "--nofullscreen")
			fullScreen=false;
		else if (args.at(i) == "--help"){
			std::cout << "gnssview " << std::endl;
			std::cout << "Usage: gnssview [options]" << std::endl;
			std::cout << std::endl;
			std::cout << "--help         print this help" << std::endl;
			std::cout << "--license      print this help" << std::endl;
			std::cout << "--nofullscreen run in a window" << std::endl;
			std::cout << "--version      display version" << std::endl;
			
			exit(EXIT_SUCCESS);
		}
		else if (args.at(i) == "--license"){
			std::cout << " gnssview - a program for displaying GNSS satellite paths" << std::endl;
			std::cout <<  std::endl;
			std::cout << " The MIT License (MIT)" << std::endl;
			std::cout <<  std::endl;
			std::cout << " Copyright (c)  2014  Michael J. Wouters" << std::endl;
			std::cout <<  std::endl; 
			std::cout << " Permission is hereby granted, free of charge, to any person obtaining a copy" << std::endl;
			std::cout << " of this software and associated documentation files (the \"Software\"), to deal" << std::endl;
			std::cout << " in the Software without restriction, including without limitation the rights" << std::endl;
			std::cout << " to use, copy, modify, merge, publish, distribute, sublicense, and/or sell" << std::endl;
			std::cout << " copies of the Software, and to permit persons to whom the Software is" << std::endl;
			std::cout << " furnished to do so, subject to the following conditions:" << std::endl;
			std::cout << std::endl; 
			std::cout << " The above copyright notice and this permission notice shall be included in" << std::endl;
			std::cout << " all copies or substantial portions of the Software." << std::endl;
			std::cout << std::endl;
			std::cout << " THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR" << std::endl;
			std::cout << " IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY," << std::endl;
			std::cout << " FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE" << std::endl;
			std::cout << " AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER" << std::endl;
			std::cout << " LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM," << std::endl;
			std::cout << " OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN" << std::endl;
			std::cout << " THE SOFTWARE." << std::endl;
			
			exit(EXIT_SUCCESS);
		}
		else if (args.at(i) == "--version"){
			std::cout << "gnssview " << VERSION_INFO << std::endl;
			std::cout << std::endl;
			std::cout << "This ain't no stinkin' Perl script!" << std::endl;
			
			exit(EXIT_SUCCESS);
		}
		else{
			std::cout << "gnssview: Unknown option '"<< args.at(i).toStdString() << "'" << std::endl;
			std::cout << "gnssview: Use --help to get a list of available command line options"<< std::endl;
			
			exit(EXIT_SUCCESS);
		}
	}
	
	setWindowTitle(tr("gnssview"));
	setMinimumSize(QSize(1920,1080));
	if (fullScreen)
		setWindowState(windowState() ^ Qt::WindowFullScreen);
	
	QTime on(7,0,0);
	QTime off(19,0,0);
	
	powerManager=new PowerManager(on,off);
	powerManager->enable(false);
	
	latitude=-33.87;
	longitude=151.21;
	address="";
	port=-1;
	
	QVBoxLayout * vb = new QVBoxLayout(this);
	vb->setContentsMargins(0,0,0,0);
	view = new GNSSViewWidget(NULL,&birds);
	vb->addWidget(view);
	
	// Look for a configuration file
	// The search path is ./:~/gnssview:~/.gnssview:/usr/local/etc:/etc
	
	QFileInfo fi;
	QString config;
	QString s("./gnssview.xml");
	fi.setFile(s);
	if (fi.isReadable())
		config=s;
	
	if (config.isNull()){
		char *eptr = getenv("HOME");
		QString home("./");
		if (eptr)
			home=eptr;
		s=home+"/gnssview/gnssview.xml";
		fi.setFile(s);
		if (fi.isReadable())
			config=s;
		if (config.isNull()){
			s=home+"/.gnssview/gnssview.xml";
			fi.setFile(s);
			if (fi.isReadable())
				config=s;
		}
	}
	
	if (config.isNull()){
		s="/usr/local/etc/gnssview.xml";
		fi.setFile(s);
		if (fi.isReadable())
			config=s;
	}
	
	if (config.isNull()){
		s="/etc/gnssview.xml";
		fi.setFile(s);
		if (fi.isReadable())
			config=s;
	}
	
	if (!config.isNull())
		readConfig(config);
	
	view->setLocation(latitude,longitude);
	
	createActions();
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this,SIGNAL(customContextMenuRequested ( const QPoint & )),this,SLOT(createContextMenu(const QPoint &)));

	udpSocket = new QUdpSocket(this);
  udpSocket->bind(port,QUdpSocket::ShareAddress);
	
	// Start of flimflummery
	// Newer versions of Qt support this but for the moment we'll do it the Unix way 
	int socketfd = udpSocket->socketDescriptor();
	if (socketfd != -1){
		ip_mreq mreq;
		memset(&mreq,0,sizeof(ip_mreq));
		mreq.imr_multiaddr.s_addr = inet_addr(address.toStdString().c_str()); // group addr
		mreq.imr_interface.s_addr = htons(INADDR_ANY); // use default

		//Make this a member of the Multicast Group
		if(setsockopt(socketfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const void *)&mreq,sizeof(mreq)) < 0){
			qDebug("Failed to add to multicast group");
		}
		
		// set TTL (Time To Live)
		unsigned int ttl = 38; // restricted to 38 hops
		if (setsockopt(socketfd, IPPROTO_IP, IP_MULTICAST_TTL, (const char *)&ttl, sizeof(ttl) ) < 0){
			qDebug("Failed to set TTL");
		}
	}
	else{
		qDebug() << "Bad socket fd!";
	}
	// End of flimflummery
	
  connect(udpSocket, SIGNAL(readyRead()),this, SLOT(readPendingDatagrams()));
				 
	updateTimer = new QTimer(this);
	connect(updateTimer,SIGNAL(timeout()),this,SLOT(updateView()));
	
	QDateTime now = QDateTime::currentDateTime();
	updateTimer->start(1000-now.time().msec()); 
	
}

void 	GNSSView::keyPressEvent (QKeyEvent *ev)
{
	QWidget::keyPressEvent(ev);
	powerManager->deviceEvent();
}

void 	GNSSView::mouseMoveEvent (QMouseEvent *ev )
{
	QWidget::mouseMoveEvent(ev);
	powerManager->deviceEvent();
}

void 	GNSSView::mousePressEvent (QMouseEvent *ev )
{
	QWidget::mousePressEvent(ev);
	powerManager->deviceEvent();
}

void GNSSView::updateView()
{
	powerManager->update();
	
	QDateTime now = QDateTime::currentDateTime();

	QList<GNSSSV *>::iterator it=birds.begin();
	while ( it != birds.end() ){
		if ((*it)->lastUpdate.secsTo(now) > TRACKING_TIMEOUT){
				qDebug() << "dead bird";
				GNSSSV *ptr = *it;
				it = birds.erase(it); // iterator 'it' is next item 
				delete ptr;
		}
		else{
		 ++it;
		}
	}
	view->update(now);
	
	updateTimer->start(1000-now.time().msec());
}

void GNSSView::toggleFullScreen()
{
	fullScreen=!fullScreen;
	setWindowState(windowState() ^ Qt::WindowFullScreen);
}

void GNSSView::togglePowerManagement()
{
	powerManager->enable(!powerManager->isEnabled());
}

void GNSSView::offsetTime()
{
	int offset = QInputDialog::getInt(this,"Time offset","Enter time offset in hours (0 to 24)",0,0,24);
	view->offsetTime(offset);
}

void GNSSView::quit()
{
	exit(1);
}

void GNSSView::readConfig(QString s)
{
	QDomDocument doc;
	
	qDebug() << "Using configuration file " << s;
	
	QFile f(s);
	if ( !f.open( QIODevice::ReadOnly) ){
		qWarning() << "Can't open " << s;
		return;
	}
	
	QString err;
	int errlineno,errcolno;
	if ( !doc.setContent( &f,true,&err,&errlineno,&errcolno ) ){	
		qWarning() << "PARSE ERROR " << err << " line=" << errlineno;
		f.close();
		return ;
	}
	f.close();
	
	QDomElement elem = doc.documentElement().firstChildElement();
	QString lc;

	while (!elem.isNull())
	{
		qDebug() << elem.tagName() << " " << elem.text();
		lc=elem.text().toLower();
		lc=lc.simplified();
		lc=lc.remove('"');
		if (elem.tagName()=="constellations"){
			// comma separated list
			QStringList sl=lc.split(",");
			for (int i=0;i<sl.length();i++){
				if (sl.at(i)=="beidou"){
					view->addConstellation(GNSSSV::Beidou);
					constellations.push_back(GNSSSV::Beidou);
				}
				else if (sl.at(i)=="gps"){
					view->addConstellation(GNSSSV::GPS);
					constellations.push_back(GNSSSV::GPS);
				}
				else if (sl.at(i)=="glonass"){
					view->addConstellation(GNSSSV::GLONASS);
					constellations.push_back(GNSSSV::GLONASS);
				}
				else if (sl.at(i)=="galileo"){
					view->addConstellation(GNSSSV::Galileo);
					constellations.push_back(GNSSSV::Galileo);
				}
				else if (sl.at(i)=="qzss"){
					view->addConstellation(GNSSSV::QZSS);
					constellations.push_back(GNSSSV::QZSS);
				}
				else if (sl.at(i)=="sbas"){
					view->addConstellation(GNSSSV::SBAS);
					constellations.push_back(GNSSSV::SBAS);
				}
			}
		}
		else if (elem.tagName()=="receiver"){
			view->setReceiver(elem.text());
		}
		else if (elem.tagName()=="location"){
			QDomElement cel=elem.firstChildElement();
			while(!cel.isNull()){
				if (cel.tagName() == "latitude")
					latitude=cel.text().toDouble();
				else if (cel.tagName() == "longitude")
					longitude=cel.text().toDouble();
				cel=cel.nextSiblingElement();
			}
		}
		else if (elem.tagName()=="network"){
			QDomElement cel=elem.firstChildElement();
			while(!cel.isNull()){
				if (cel.tagName() == "address")
					address=cel.text().trimmed();
				else if (cel.tagName() == "port")
					port=cel.text().toInt();
				cel=cel.nextSiblingElement();
			}
		}
		else if (elem.tagName()=="animation"){
			QDomElement cel=elem.firstChildElement();
			int fps=10;
			double period=60;
			int skyupdate=60;
			bool smooth=true;
			snMax=255.0;
			while(!cel.isNull()){
				if (cel.tagName() == "period")
					period=cel.text().toDouble();
				else if (cel.tagName() == "fps")
					fps=cel.text().toInt();
				else if (cel.tagName() == "skyupdate")
					skyupdate=cel.text().toInt();
				else if (cel.tagName() == "snmax")
					snMax=cel.text().toDouble();
				else if (cel.tagName() == "smooth"){
					QString txt=cel.text().toLower();
					txt=txt.trimmed();
					smooth = (txt=="yes");
				}
				cel=cel.nextSiblingElement();
			}
			view->setAnimation(fps,period,skyupdate,smooth);
		}
		else if (elem.tagName()=="images"){
			QDomElement cel=elem.firstChildElement();
			while(!cel.isNull()){
				if (cel.tagName() == "nightsky")
					view->setNightSkyImage(cel.text().trimmed());
				else if (cel.tagName() == "foreground"){
					double minel=-10.0;
					double maxel=30.0;
					QString fg="";
					QDomElement ccel = cel.firstChildElement();
					while (!ccel.isNull()){
						if (ccel.tagName() == "file")
							fg=ccel.text().trimmed();
						else if (ccel.tagName() == "minelevation")
							minel = ccel.text().toDouble();
						else if (ccel.tagName() == "maxelevation")
							maxel = ccel.text().toDouble();
						ccel=ccel.nextSiblingElement();
					}
					view->setForegroundImage(fg,minel,maxel);
				}
				cel=cel.nextSiblingElement();
			}
		}
		else if (elem.tagName()=="power")
		{
			QDomElement cel=elem.firstChildElement();
			while(!cel.isNull()){
				lc=cel.text().toLower();
				lc=lc.simplified();
				lc=lc.remove('"');
				if (cel.tagName() == "conserve"){
					powerManager->enable(lc == "yes");
					qDebug() << "power::conserve=" << lc;
				}
				else if (cel.tagName() == "weekends"){
					if (lc=="yes") 
						powerManager->setPolicy(PowerManager::NightTime | PowerManager::Weekends);
					else
						powerManager->setPolicy(PowerManager::NightTime);
					qDebug() << "power::weekends=" << lc;
				}
				else if (cel.tagName() == "on"){
					QTime t=QTime::fromString(lc,"hh:mm:ss");
					if (t.isValid())
						powerManager->setOnTime(t);
					else
						qWarning() << "Invalid power on time: " << lc;
				}
				else if (cel.tagName() == "off"){
					QTime t=QTime::fromString(lc,"hh:mm:ss");
					if (t.isValid())
						powerManager->setOffTime(t);
					else
						qWarning() << "Invalid power off time: " << lc;
				}
				else if (cel.tagName() == "overridetime"){
					powerManager->setOverrideTime(cel.text().toInt());
					qDebug() << "power::overridetime=" << lc;
				}
				cel=cel.nextSiblingElement();
			}
		}
		elem=elem.nextSiblingElement();
	}
	
}

void GNSSView::readPendingDatagrams()
{
	while (udpSocket->hasPendingDatagrams()) {
		QByteArray datagram;
		datagram.resize(udpSocket->pendingDatagramSize());
		QHostAddress sender;
		quint16 senderPort;

		udpSocket->readDatagram(datagram.data(), datagram.size(),&sender, &senderPort);
		//qDebug() << datagram.data();
		
		// Parse it
		// Data is of the form
		// timestamp,constellation,prn,az,el,sn
		 QString str(datagram.data());
		 QStringList svd = str.split("\n");
		 for (int i=0;i<svd.size();i++){
			 QStringList sv = svd.at(i).split(",");
			 if (sv.size() == 6)
			 {
				 // FIXME do some sanity checks on values
				 int c   = sv.at(1).toInt();
				 // ignore it if it's not a constellation were tracking
				 int k;
				 for (k=0;k<constellations.size();k++){
						if (c==constellations.at(k))
							break;
				 }
				 if (k<constellations.size()){
						int prn = sv.at(2).toInt();
						double az=sv.at(3).toDouble();
						//if (az > 180) az -= 360;
						//az /= 360.0;
						//double elev=sin(sv.at(4).toDouble()/10.0*M_PI/180.0); // precompute for plotting 
						double elev=sv.at(4).toDouble();
						double sn = sv.at(5).toDouble()/snMax; // prescale
						QDateTime u;
						u.setTime_t(sv.at(0).toInt());
						
						int idx=-1;
						for (int b=0;b<birds.size();b++){
							if (birds.at(b)->constellation == c && birds.at(b)->PRN == prn){
								idx =b;
								break;
							}
						}
						if (idx >= 0){
							birds.at(idx)->update(az,elev,sn,u);
						}
						else {// new bird
						
						//prn,double a,double e,double s,int c,QDateTime &u)
							birds.append(new GNSSSV(prn,az,elev ,sn,c,u));
						}
				 }
			 } // otherwise ignore it
		 }
	}
}

void GNSSView::createContextMenu(const QPoint &)
{

	toggleFullScreenAction->setChecked(fullScreen);

	QMenu *cm = new QMenu(this);
	cm->addAction(toggleFullScreenAction);
	cm->addAction(powerManAction);
	cm->addSeparator();
	cm->addAction(toggleForegroundAction);
	cm->addAction(offsetTimeAction);
	cm->addAction(quitAction);
	cm->exec(QCursor::pos());
	delete cm;
}

void GNSSView::createActions()
{
	toggleFullScreenAction = new QAction(QIcon(), tr("Full screen"), this);
	toggleFullScreenAction->setStatusTip(tr("Show full screen"));
	addAction(toggleFullScreenAction);
	connect(toggleFullScreenAction, SIGNAL(triggered()), this, SLOT(toggleFullScreen()));
	toggleFullScreenAction->setCheckable(true);
	
	powerManAction = new QAction(QIcon(), tr("Power management"), this);
	powerManAction->setStatusTip(tr("Power management"));
	addAction(powerManAction);
	connect(powerManAction, SIGNAL(triggered()), this, SLOT(togglePowerManagement()));
	powerManAction->setCheckable(true);
	powerManAction->setChecked(powerManager->isEnabled());
	
	toggleForegroundAction = new QAction(QIcon(), tr("Foreground"), this);
	toggleForegroundAction->setStatusTip(tr("Show the foreground"));
	addAction(toggleForegroundAction);
	connect(toggleForegroundAction, SIGNAL(triggered()), view, SLOT(toggleForeground()));
	toggleForegroundAction->setCheckable(true);
	toggleForegroundAction->setChecked(true);
	
	offsetTimeAction = new QAction(QIcon(), tr("Offset time"), this);
	offsetTimeAction->setStatusTip(tr("Offset the time by n hours"));
	addAction(offsetTimeAction);
	connect(offsetTimeAction, SIGNAL(triggered()), this, SLOT(offsetTime()));
	
	quitAction = new QAction(QIcon(), tr("Quit"), this);
	quitAction->setStatusTip(tr("Quit"));
	addAction(quitAction);
	connect(quitAction, SIGNAL(triggered()), this, SLOT(quit()));
	
}
