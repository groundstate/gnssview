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


#ifndef _SKY_MODEL_H_
#define _SKY_MODEL_H_

#include "Colour.h"


class SkyModel
{
	public:
	
		SkyModel();
		~SkyModel();
		
		void setSolarPosition(float,float);
		Colour colour(float,float);
		
	private:
	
		void init();
		void initialise();
		float angleBetween(float,float,float,float);
		float chromaticity(float c[3][4]);
		float distribution( float, float , float , float , float ,float , float );
		float PerezModel(float, float , float , float , float ,float , float);
		
		float turbidity_,turbidity2_;
		float sunTheta_,sunPhi_;
		
		float xCoeff[5],yCoeff[5],YCoeff[5];
		float chi,zenithx,zenithy,zenithY;
		
		float scaling_;
		
};

#endif
