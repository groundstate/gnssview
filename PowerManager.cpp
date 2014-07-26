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
//
// On Debian systems, vbetool needs to run via sudo
// so to disable the password for just vbetool you need to edit
// sudoers:
// user_name ALL=(ALL) NOPASSWD: /usr/sbin/vbetool
//

#include <QDebug>
#include <QFileInfo>
#include <QProcess>

#include "PowerManager.h"


PowerManager::PowerManager(QTime &on,QTime &off):
	on(on),off(off)
{
	policy= NightTime | Weekends;
	enabled=true;
	powerState=PowerSaveInactive;
	overrideTime = 30;
	
	// Detect power management tool
	
	// Raspberry Pi
	QFileInfo vc = QFileInfo("/opt/vc/bin/tvservice");
	videoTool = Unknown;
	if (vc.exists()){
		videoTool=RaspberryPi;
	}
	else // Conventional Video BIOS
	{	
		vc.setFile("/usr/bin/xset");
		if (vc.exists()){
			videoTool=XSet;
		}
	}
	disableOSPowerManagment();
	
}

PowerManager::~PowerManager()
{
}

void PowerManager::update()
{
	if (!enabled) return;
	
	QDateTime now = QDateTime::currentDateTime();
	
	// Turn off on weekends
	// Turn off between configured times
	bool powerOn=true;
	
	if (policy & NightTime)
		powerOn = (now.time() > on) && (now.time() < off); // simple logic here - only works when on < off in 24 hr clock 
  if (policy & Weekends){
		if (now.date().dayOfWeek() > 5)
			powerOn=false;
	}
	
	// If no change then return
	if (powerOn && (powerState == PowerSaveInactive))
		return;
	if (!powerOn && (powerState == PowerSaveActive))
		return;
	
	if (powerOn && (powerState == PowerSaveActive))
	{
		displayOn();
		powerState=PowerSaveInactive;
	}
	else if (!powerOn && (powerState == PowerSaveInactive))
	{
		displayOff();
		powerState=PowerSaveActive;
	}
	else if (powerState == (PowerSaveOverridden | PowerSaveActive))
	{
		qDebug() << now.toString() << " " << overrideStop.toString();
		if (now >= overrideStop)
		{
			powerState=PowerSaveActive; // next bit of code takes care of powering back on
			if (powerOn)
			{
				displayOn();
				powerState=PowerSaveInactive;
			}
			else
			{
				displayOff();
			}
		}
	}
}

void PowerManager::enable(bool en)
{
	enabled=en;
}

bool PowerManager::isEnabled()
{
	return enabled;
}

void PowerManager::setOnTime(QTime &t)
{
	on=t;
	qDebug() << "Power on " << on.toString();
}

void PowerManager::setOffTime(QTime &t)
{
	off=t;
	qDebug() << "Power off " << off.toString();
}
		
void PowerManager::setPolicy(int pol)
{
	policy=pol;
}

void PowerManager::setOverrideTime(int t)
{
	overrideTime=t;
}

void PowerManager::deviceEvent()
{
	// Device events turn the power back on tenporarily if the power is off
	if (powerState==PowerSaveActive)
	{
		overrideStop = QDateTime::currentDateTime();
		overrideStop = overrideStop.addSecs(overrideTime*60);
		powerState |= PowerSaveOverridden;
		displayOn();
	}
}

//
//
//

void PowerManager::disableOSPowerManagment()
{
	qDebug() << "disableOSPowerManagment()";
	
	if (videoTool==Unknown) return;
	
	QProcess pwr;
	
	// jiggery pokery with the screensaver is required too
	pwr.start("xset", QStringList() << "-dpms");
	pwr.waitForStarted();
	pwr.waitForFinished();
	pwr.start("xset", QStringList() << "s" << "reset");
	pwr.waitForStarted();
	pwr.waitForFinished();
	pwr.start("xset", QStringList() << "s" << "off");
	pwr.waitForStarted();
	pwr.waitForFinished();
	
}

void PowerManager::displayOn()
{
	qDebug() << "displayOn()";
	
	if (videoTool==Unknown) return;

	QProcess pwr;
	switch (videoTool)
	{
		case RaspberryPi:
			pwr.start("/opt/vc/bin/tvservice -p");
			pwr.waitForStarted();
			pwr.waitForFinished();
			pwr.start("sudo chvt 1"); // this is black magic to kick the xserver back to life
			pwr.waitForStarted();
			pwr.waitForFinished();
			pwr.start("sudo chvt 2");
			pwr.waitForStarted();
			pwr.waitForFinished();
			break;
		case XSet:
			pwr.start("xset dpms force on");
			pwr.waitForStarted();
			pwr.waitForFinished();
			disableOSPowerManagment();
			break;
		case Unknown:
			break;
	}
	
}


//sleep 1; xset dpms force off; sleep 30; xset dpms force on; xset s reset; xset s off;
// this works ..

void PowerManager::displayOff()
{
	qDebug() << "displayOff()";
	
	if (videoTool==Unknown) return;
	
	qDebug() << "power off";
	QProcess pwr;
	switch (videoTool)
	{
		case RaspberryPi:
			pwr.start("/opt/vc/bin/tvservice -o");
			break;
		case XSet:
			pwr.start("xset dpms force off");
			break;
		case Unknown:
			return;
			break;
	}
	qDebug() << "starting" << QDateTime::currentDateTime().time().toString();
	pwr.waitForStarted();
	qDebug() << "finishing"<< QDateTime::currentDateTime().time().toString();
	pwr.waitForFinished();
	qDebug() << "finished"<< QDateTime::currentDateTime().time().toString();
}

