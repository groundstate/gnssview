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

#include "SkyModel.h"

// distribution coefficients for the luminance(Y) distribution function
static float YDC[5][2] = { { 0.1787, - 1.4630},
                            {-0.3554,   0.4275},
                            {-0.0227,   5.3251},
                            { 0.1206, - 2.5771},
                            {-0.0670,   0.3703} };

// distribution coefficients for the x distribution function
static float xDC[5][2] = { {-0.0193, -0.2592},
                            {-0.0665, 0.0008},
                            {-0.0004, 0.2125},
                            {-0.0641, -0.8989},
                            {-0.0033, 0.0452} };

// distribution coefficients for the y distribution function
static float yDC[5][2] = { {-0.0167, -0.2608},
                            {-0.0950, 0.0092},
                            {-0.0079, 0.2102},
                            {-0.0441, -1.6537},
                            {-0.0109, 0.0529} };

// xenith x value
static float xZC[3][4] = {  {0.00166, -0.00375, 0.00209, 0},
                            {-0.02903, 0.06377, -0.03203, 0.00394},
                            {0.11693, -0.21196, 0.06052, 0.25886} };
// xenith y value
static float yZC[3][4] = { { 0.00275, -0.00610, 0.00317, 0},
                            {-0.04214, 0.08970, -0.04153, 0.00516},
                            {0.15346, -0.26756, 0.06670, 0.26688} };
														
SkyModel::SkyModel()
{
	init();
}

SkyModel::~SkyModel()
{
}

void SkyModel::setSolarPosition(float azimuth,float elevation)
{
	// Input angles in degrees
	
	// Solar zenith angle (theta) is measured positive from the vertical
	// whereas elevation is measured wrt to horizontal
	sunTheta_=M_PI/2.0-elevation*M_PI/180.0;
	// Solar azimuth is measured anti-clockwise positive from South
	// whereas azimuth is measured in compass coordinates viz positive
	// clockwise from North
	sunPhi_= -azimuth*M_PI/180.0 + M_PI;
	
	// Absolute zenith luminance
	chi = (4.0/9.0 - turbidity_/120.0)*(M_PI - 2.0*sunTheta_);
	//cerr << "chi " << chi << endl;
	zenithY = 
		(4.0453*turbidity_ - 4.9710)*tan(chi) - 0.2155*turbidity_ + 2.4192;
  if (zenithY < 0) zenithY = - zenithY;
	
	for (int i=0;i<5;i++)
		YCoeff[i] = YDC[i][0]*turbidity_ + YDC[i][1];
	
	zenithx = chromaticity( xZC );
	for (int i=0;i<5;i++)
		xCoeff[i] = xDC[i][0]*turbidity_ + xDC[i][1];
		
	zenithy = chromaticity( yZC );
	for (int i=0;i<5;i++)
		yCoeff[i] = yDC[i][0]*turbidity_ + yDC[i][1];
	
}

void SkyModel::initialise()
{
	
	
}

Colour SkyModel::colour(float azimuth,float elevation)
{
	// Input angles in degrees 
	float d;
	Colour skycolour(0,0,0,Colour::xyY);
	
	float theta = M_PI/2.0-elevation*M_PI/180.0;
	float phi = -azimuth*M_PI/180.0 + M_PI;
	//float phi = azimuth*M_PI/180.0+M_PI;
	float gamma = angleBetween( theta, phi, sunTheta_, sunPhi_);
	
	d = distribution(YCoeff[0],YCoeff[1], YCoeff[2], YCoeff[3], YCoeff[4], theta, gamma);
  skycolour.z = zenithY * d;
	
	// Zenith x
	d = distribution(xCoeff[0], xCoeff[1], xCoeff[2], xCoeff[3], xCoeff[4], theta, gamma);
  skycolour.x = zenithx * d;
	
	// Zenith y
	d = distribution(yCoeff[0], yCoeff[1], yCoeff[2], yCoeff[3],yCoeff[4], theta, gamma);
  skycolour.y = zenithy * d;
	
	
	// FIXME scaling required here ....
	//skycolour.z = 1 - exp(-(1.0/10.0) * skycolour.z);
	//cerr << "xyY " << skycolour.x << " " << skycolour.y << " " << skycolour.z << endl;
	
	Colour finalcolour = skycolour.asRGB();
	//cerr << "rgb " << finalcolour.x << " " << finalcolour.y << " " << finalcolour.z <<
	//endl;
	finalcolour.x = 1.0 -exp(-1.0*finalcolour.x/scaling_);
	finalcolour.y = 1.0 -exp(-1.0*finalcolour.y/scaling_);
	finalcolour.z = 1.0 -exp(-1.0*finalcolour.z/scaling_);
	return finalcolour; // Convert xyY to RGB
}
		
//
// Private members
// 

void SkyModel::init()
{
	
	sunTheta_= 0.0;
	sunPhi_=0.0;
	
	turbidity_=3.0;
	scaling_=15.0;
	turbidity2_=turbidity_*turbidity_;
}

float SkyModel::angleBetween(float thetav, float phiv, 
	float theta, float phi) 
{
  float cospsi = sin(thetav) * sin(theta) * cos(phi-phiv) + 
		cos(thetav) * cos(theta);
  if (cospsi > 1)  return 0;
  if (cospsi < -1) return M_PI;
  return acos(cospsi);
}

float SkyModel::chromaticity(float c[3][4])
{
	float t1 = sunTheta_;
	float t2 = t1*t1;
	float t3 = t2*t1;
	
	return (c[0][0]*t3 + c[0][1]*t2 + c[0][2]*t1 + c[0][3])* turbidity2_ +
				 (c[1][0]*t3 + c[1][1]*t2 + c[1][2]*t1 + c[1][3])* turbidity_ +
				 (c[2][0]*t3 + c[2][1]*t2 + c[2][2]*t1 + c[2][3]);
}

float SkyModel::distribution( float A, float B, float C, float D, float E,
	 float theta, float gamma )
{
	
  float f0 = PerezModel( A,B,C,D,E,theta,gamma );
  float f1 = PerezModel( A,B,C,D,E,0,sunTheta_ );
	//if (theta == M_PI/2.0) return 0.0;
	//cerr << f0 << " " << f1 << endl;
  return(f0/f1);
}

float SkyModel::PerezModel( float A, float B, float C, float D, float E,
	 float theta, float gamma )
{
	float cosGamma = cos(gamma);
	return (1+ A * exp(B/cos(theta)))*(1+ C * exp(D*gamma) + 
		E * cosGamma*cosGamma );
}

