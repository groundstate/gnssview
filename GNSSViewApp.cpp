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
#include <QFileInfo>

#include "GNSSViewApp.h"


GNSSViewApp::GNSSViewApp(int argc,char *argv[]):QApplication(argc,argv){
	app = this;
}

QString GNSSViewApp::locateResource(QString f){
	// Look for a resource
	// The search path is ./:~/gnssview:~/.gnssview:/usr/local/share/gnssview:/usr/share/gnssview
	
	QFileInfo fi;
	QString s;
	
	fi.setFile(f);
	if (fi.isReadable())
		return f;
	
	char *eptr = getenv("HOME");
	QString home("./");
	if (eptr)
		home=eptr;
	s=home+QString("/gnssview/")+f;
	fi.setFile(s);
	if (fi.isReadable())
		return s;
	
	s=home+QString("/.gnssview/")+f;
	fi.setFile(s);
	if (fi.isReadable())
		return s;
	
	s="/usr/local/share/gnssview/"+f;
	fi.setFile(s);
	if (fi.isReadable())
		return s;
	
	s="/usr/share/gnssview/"+f;
	fi.setFile(s);
	if (fi.isReadable())
		return s;
	
	return QString();
}