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


#ifndef __GNSS_SV_H_
#define __GNSS_SV_H_

#include <QDateTime>

#include <QList>

class GNSSSV
{
	public:

		enum Constellation {Beidou,GPS,Galileo,GLONASS,QZSS,SBAS};

		GNSSSV();
		GNSSSV(int,double,double,double,int,QDateTime &);
		GNSSSV(GNSSSV &);

		void update(double,double,double,QDateTime &,double threshold=0.0);
		
		~GNSSSV();

		QDateTime lastUpdate;
		bool changed;
		
		int PRN;
		QList<double> az,elev;
		double sn;
		int constellation;

};

#endif