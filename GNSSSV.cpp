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


#include <QDebug>

#include "GNSSSV.h"

GNSSSV::GNSSSV(){}

GNSSSV::GNSSSV(int prn,double a,double e,double s,int c,QDateTime &u)
{
	PRN=prn;
	az.append(a);
	elev.append(e);
	sn=s;
	constellation=c;
	lastUpdate=u;
	qDebug() << "New PRN " << PRN;
}

GNSSSV::GNSSSV(GNSSSV & gnsssv)
{
	*this = gnsssv;
}

GNSSSV::~GNSSSV(){}

void GNSSSV::update(double a,double e,double s,QDateTime &u)
{
	// if this precedes the last update, just ignore it
	if (u<lastUpdate) return;
	lastUpdate=u;
	// if the position is unchanged ignore it
	if (!az.isEmpty()){
		if (az.last()==a && elev.last()==e) return;
		// a further refinement - drop points where we are moving in a straight line
		// this also cleans up the jaggies a bit 
// 		if ((az.size() > 1) && (az.last()==a || elev.last()==e)) // don't drop the first point otherwise we don't start a track until something changes
// 		{
// 			qDebug() << "Dropping point " << PRN << ":" << az.last() << " " << elev.last();
// 			az.pop_back();
// 			elev.pop_back();
// 			
// 		}
	}
	

	az.append(a);
	elev.append(e);
	sn = s;
	//qDebug() << "Updated " << PRN << ":" << a << " " << e;
}