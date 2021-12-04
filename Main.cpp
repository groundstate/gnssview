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

#include <QtDebug>
#include <QFile>
#include <QTextStream>

#include "GNSSViewApp.h"
#include "GNSSView.h"

GNSSViewApp *app;

void myMessageHandler(QtMsgType type, const QMessageLogContext &, const QString & msg)
{
    QString txt;
    switch (type) {
    case QtDebugMsg:
        txt = QString("Debug: %1").arg(msg);
        break;
    case QtWarningMsg:
        txt = QString("Warning: %1").arg(msg);
    break;
    case QtCriticalMsg:
        txt = QString("Critical: %1").arg(msg);
    break;
    case QtFatalMsg:
        txt = QString("Fatal: %1").arg(msg);
    break;
    case QtInfoMsg:
        txt = QString("Fatal: %1").arg(msg);
    }
    QFile outFile("/tmp/gnssview.log");
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << txt << endl;
}



int main(int argc, char **argv)
{
 
	GNSSViewApp a(argc, argv);
	qInstallMessageHandler(myMessageHandler);   
	QStringList args = a.arguments(); 
	GNSSView disp(args);
	disp.show();
	return a.exec();
	
}
