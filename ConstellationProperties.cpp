//
// gnssview - a program for displaying GNSS satellite paths
//
// The MIT License (MIT)
//
// Copyright (c)  2014-2016  Michael J. Wouters
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


#include "ConstellationProperties.h"
#include "GNSSSV.h"

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
			idLabel="C";
			svIDmin=1;
			svIDmax=32;
			break;
		case GNSSSV::GPS:
			maxsv=14;
			histColour[0]=1;histColour[1]=0.549;histColour[2]=0.0;histColour[3]=0.5;
			label="GPS";
			idLabel="G";
			svIDmin=1;
			svIDmax=37;
			break;
		case GNSSSV::Galileo:
			maxsv=8;
			histColour[0]=0.5;histColour[1]=1.0;histColour[2]=0.0;histColour[3]=0.5;
			label="Galileo";
			idLabel="E";
			svIDmin=1;
			svIDmax=32;
			break;
		case GNSSSV::GLONASS:
			maxsv=12;
			histColour[0]=1;histColour[1]=0.843;histColour[2]=0.0;histColour[3]=0.5; 
			label="GLONASS";
			idLabel="R";
			svIDmin=1;
			svIDmax=25;
			break;
		case GNSSSV::QZSS:
			maxsv=4;
			histColour[0]=0.867;histColour[1]=0.627;histColour[2]=0.867;histColour[3]=0.5; 
			label="QZSS";
			idLabel="J";
			svIDmin=1;
			svIDmax=7;
			break;
		case GNSSSV::SBAS:
			maxsv=6;
			histColour[0]=0.98;histColour[1]=0.941;histColour[2]=0.901;histColour[3]=0.5; 
			label="SBAS";
			idLabel="S";
			svIDmin=20;
			svIDmax=40;
			break;
	}
}