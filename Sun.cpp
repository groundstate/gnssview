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

#include <QDebug>
#include <QDateTime>

#include "Sun.h"

#define EPSILONG 278.833540
#define WG       282.596403
#define ECCENTRICITY 0.016718
#define OBLIQUITY 23.441884


static int days_noleap[12]=
        {0,30,59,90,120,151,181,212,243,273,304,334}; // FIXME ???
        
static int days_leap[12]=
        {0,31,60,91,121,152,182,213,244,274,305,335};

				
Sun::Sun(double lat,double lon)
{
	setLocation(lat,lon);
}

Sun::~Sun()
{
}

void Sun::setLocation(double lat,double lon)
{
	lat_=lat;
	if (lat >=0) hemi_=North; 
	else{
		lat_=-lat_;
		hemi_=South;
	}
	lon_=lon;
	if (lon>=0) 
		londir_=East; 
	else{
		lon_=-lon;
		londir_=West;
	}
	QDateTime now=QDateTime::currentDateTime();
	now=now.toUTC();
	update(now.date().year(),now.date().month(),now.date().day(),
				 now.time().hour(),now.time().minute(),now.time().second());
}

void Sun::position(double *az,double *el)
{
	*el=alt_;
	*az=az_;
}

void Sun::update(int year,int month,int mday,int hour,int min, int sec)
{
 	// Supply local time converted to GMT, and latitude and longitude in
  // degrees. Returns azimuth and elevation
	// Hmm GMT == UT1 ?? 
  double ra,dec;
  SolarRaDec(year,month,mday,&ra,&dec);
  double ha = RA2HourAngle(ra,sec,min,hour,mday,month,year,lon_,londir_);
	double lsgn=1.0;
	if (hemi_ == South) lsgn=-1.0;
  Equatorial2Horizon(ha,dec,lsgn*lat_,&alt_,&az_);
}


bool Sun::isLeapYear(int yr)
{
	if (yr % 4) return false;
	if (yr % 100) return true;
	return !(yr % 400);
}

void Sun::EclipticToEquatorial(double l,double b,double *ra,double *dec)
{
	double sind = sin(b*M_PI/180.0) * cos(OBLIQUITY*M_PI/180.0) +
		cos(b*M_PI/180.0)*sin(OBLIQUITY*M_PI/180.0)*sin(l*M_PI/180.0);
	*dec = 180.0*asin(sind)/M_PI;
	double y = sin(l*M_PI/180.0) * cos(OBLIQUITY*M_PI/180.0) -
		tan(b*M_PI/180.0)*sin(OBLIQUITY*M_PI/180.0);
	double x = cos(l*M_PI/180.0);
	double ap = atan2(y,x);
	ap *= 180.0/M_PI;
	if (ap < 0) ap += 360.0;
	*ra = ap/15.0;
}

double Sun::JulianDate(double d,int m,int y)
{
/* Checked OK */
	double yp=y;
	double mp=m;
	if (m==1 || m ==2){
		yp =y - 1;
		mp = m+12;
	}
	int A,B=0;
	if (y > 1582 || (y == 1582 && m > 10) || (y == 1582 && m == 10 && d > 15)){
		A = (int) (yp/100);
		B = 2 -A + A/4;
	}
	int C=(int)(365.25*yp);
	int D=(int)(30.6001*(mp+1));
	return B + C + D + d + 1720994.5;
}

double Sun::BConstant(int yr)
{
        /* Checked OK */
        double JD =JulianDate(0.0,1,yr);
        double S = JD - 2415020.0;
        double T = S/36525.0;
        double R= 6.6460656+2400.051262*T + 0.00002581*T*T;
        double U = R-24.0*(yr-1900);
        return 24.0 - U;
}

double Sun::GMT2GST(double sec,int min,int hr,int mday,int mon,int yr)
{
        /* Checked OK */
        
        double days;
        if (isLeapYear(yr))
                days = days_leap[mon-1] + mday;
        else
                days = days_noleap[mon-1] + mday;
  days*=0.0657098;
        double T0 = days - BConstant(yr);
        double GMT = hr + min/60.0+sec/3600.0;
        GMT *= 1.002738;
        GMT+=T0;
        if (GMT > 24)
                GMT -= 24;
        else if (GMT < 0)
                GMT += 24;
        return GMT;
}

double Sun::GST2LST(double gst,double lon,int londir)
{
        /* longitude in degrees */
        double lst=gst;
        double tl = lon/15.0;
        if (londir == West)
                lst -= tl;
        else if (londir==East)
                lst += tl;
				if (lst >24)
                lst -= 24;
        if (lst < 0)
                lst += 24;
        return lst;
}

double Sun::RA2HourAngle(double ra,double sec,int min,int hr,int mday,int mon,int yr,
        double lon,int londir)
{
        double GST = GMT2GST(sec,min,hr,mday,mon,yr);
        double LST = GST2LST(GST,lon,londir);
        double ha =LST-ra;
        if (ha<0) ha += 24.0;
        return ha;
}

void Sun::Equatorial2Horizon(double ha,double decl,double lat,double *alt,double *az)
{
        double dha = ha*15.0;
        double sina = sin(decl*M_PI/180.0)*sin(lat*M_PI/180.0)+
                cos(decl*M_PI/180.0)*cos(lat*M_PI/180.0)*cos(dha*M_PI/180.0);
        *alt = asin(sina);
        
        double cosA = (sin(decl*M_PI/180.0) - sin(lat*M_PI/180.0)*sina)/
                (cos(lat*M_PI/180.0)*cos(*alt));
        *az = acos(cosA);
        
        if (sin(dha*M_PI/180.0) > 0)
                *az = 2.0*M_PI - *az;
                
        *az *= 180.0/M_PI;
        *alt *= 180.0/M_PI;;
}
        
void Sun::SolarRaDec(int year,int month,int mday,double *ra,double *dec)
{
        
         
        /* Calculate the number of days since January 0.0, 1980.
         * This is the midnight between 30/12/1979 and 31/12/1979 */
        double days=0.0;
        
        if ( year< 1979)
        {
                for (int i=year;i<=1979;i++)
                {
                        if (isLeapYear(i))
                                days-=366.0;
                        else
                                days-=365.0;
                }
        }
        else if (year > 1980)
        {
                for (int i=1981;i<=year;i++)
                {
                        if (isLeapYear(i))
                                days+=366.0;
                        else
                                days+=365.0;
                }
        }
        
        if (isLeapYear(year))
                days += days_leap[month-1] + mday;
        else
                days += days_noleap[month-1] + mday;
        
        double N = 360.0/365.2422*days;
        if (N<0)
        {
                while (N < 360.0 && N > 0.0)
                        N+=360.0;
        }
        else if (N>0)
        {
                while (N < 360.0 && N > 0.0)
                        N-=360.0;
        }
        double M= N + EPSILONG - WG;
        if (M<0) M+=360.0;
        double Ec = 360.0/M_PI*ECCENTRICITY*sin(M*M_PI/180.0);
        double lsolar=N+Ec+EPSILONG;
        if (lsolar>360.0) lsolar -=360.0;
        
        EclipticToEquatorial(lsolar,0.0,ra,dec);
        
}


