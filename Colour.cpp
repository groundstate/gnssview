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

#include <math.h>
#include <iostream>

#include "Colour.h"

using namespace std;

Colour::Colour(Colour *c)
{
	x=c->x;
	y=c->y;
	z=c->z;
	
}

Colour::Colour(float cx,float cy,float cz,int spc)
{
	x=cx;
	y=cy;
	z=cz;
	space=spc;
}

		
Colour  Colour::asRGB()
{
	Colour c(this);
	
	switch (space)
	{
		case RGB:
			c.space = RGB;
			break;
		case XYZ:
		
			c.space = RGB;
		
			c.x =    3.240479 * x    - 1.537150 * y  - 0.498535 * z;
    	c.y = -  0.969256 * x    + 1.875991 * y  + 0.041556 * z;
    	c.z =    0.055648 * x    - 0.204043 * y  + 1.057311 * z;
			
			break;
		case HSV:
		
			c.space = HSV;
			
			// H is given on [0, 6] or UNDEFINED. S and V are given on [0, 1].
  		// RGB are each returned on [0, 1].

  		float m, n, f;
  		int i;

  		if(x == -1 ) {c.x=c.y=c.z=z;}

  		i = (int)floorf(x);
  		f = x - i;

  		if(!(i & 1)) f = 1 - f; // if i is even


  		m = z * (1 - y);
  		n = z * (1 - y * f);
  		switch (i)
			{
    		case 6:
    		case 0: c.x=z; c.y=n;  c.z=m;
     			break;
   		 	case 1: c.x=n; c.y=z;  c.z=m;
      		break;
    		case 2: c.x=m, c.y=z;  c.z=n;
     		 break;
    		case 3: c.x=m, c.y=n;  c.z=z;
     		 break;    
    		case 4: c.x=n, c.y=m;  c.z=z;
      		break;    
    		case 5: c.x=z, c.y=m;  c.z=n;
      		break;    
  		}
		
			break;
			
		case xyY:
			// FIXME do this directly
			Colour xyz = asXYZ();
			c = xyz.asRGB();
			break;
	}
	return c;
}

Colour  Colour::asXYZ()
{
	Colour c(this);
	switch (space)
	{
		case RGB:
			c.space = RGB;
			c.x = 0.412453 * x + 0.357580 * y + 0.180423 * z;
    	c.y = 0.212671 * x + 0.715160 * y + 0.072169 * z;
    	c.z = 0.019334 * x + 0.119193 * y + 0.950227 * z;
			break;
		case XYZ:
			break;
		case HSV:
			break;
		case xyY:
			c.space = XYZ;
			c.x = x * (z / y);
  		c.y = z;
  		c.z = (1.0 - x - y)* (z/y);
			//cerr << "asXYZ " << c.x << " " << c.y << " " << c.z << endl;
			break;
	}
	return c;
}

Colour  Colour::asHSV()
{
	Colour c(this);
	float delta,mn,mx;
	switch (space)
	{
		case RGB:
			c=space = RGB;
			mn = mx = x;
  		int maxVal;
			maxVal=0;
			
  		if (y > mx){ mx = y;maxVal=1;}
  		if (z > mx){ mx = z;maxVal=2;}
  		if (y < mn) mn = y;
  		if (z < mn) mn= z;

  		delta = mx - mn;

  		c.z = mx;
  		if( mx != 0 )
    		c.y = delta / mx;
  		else {
    		c.y = 0;
    		c.x = 0;
    		return c;
  		}
  		if (c.y==0.0f) {
    		// unknown color
    		c.x=-1;
    		return c;
  		} else {
    		switch (maxVal) {
      		case 0:{c.x = ( y - z ) / delta; break;} // yel < h < mag
      		case 1:{c.x = 2 + ( z - x) / delta;break;} // cyan < h < yel
      		case 2:{c.x = 4 + ( x - y ) / delta;break;} // mag < h < cyan
    		}
  		}

  		c.x *= 60;
  		if (c.x < 0) c.x += 360;
			break;
		case XYZ:
			break;
		case HSV:
			break;
		case xyY:
			break;
	}
	return c;
}

Colour  Colour::asxyY()
{
	Colour c(this);
	switch (space)
	{
		case RGB:
			return c;
			break;
		case XYZ:
			break;
		case HSV:
			break;
		case xyY:
			return c;
			break;
	}
	return c;
}
		
Colour Colour::gammaCorrect(float gamma)
{
	Colour c(*this);
	c.x=pow(c.x,1.0/gamma);
	c.y=pow(c.y,1.0/gamma);
	c.z=pow(c.z,1.0/gamma);
	return c;
}
