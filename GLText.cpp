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


#include <iostream>

#include <QDebug>
#include <QFontMetrics>
#include <QImage>
#include <QPainter>
#include <QGLWidget>

#include "GLText.h"

GLText::GLText(QGLWidget *glx,QString s,QFont f,QColor color)
{
	QFontMetrics fm(f);
	w = fm.width(s);
	h = fm.height();
	descent = fm.descent(); // in OpenGL window coordinates
	ascent  = fm.ascent();
	
	QImage im(w,h,QImage::Format_ARGB32);
	im.fill(0);
	QPainter p;
	p.begin(&im);
	p.setPen(color);
	p.setFont(f);
	p.drawText(0,h-fm.descent(),s);
	
	
	QImage glim = glx->convertToGLFormat(im);

	glEnable(GL_TEXTURE_2D);
	glGenTextures(1,&texture);
	glBindTexture(GL_TEXTURE_2D,texture);
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, glim.width(), glim.height(), 0,
      GL_RGBA, GL_UNSIGNED_BYTE, glim.bits());
			
	glDisable(GL_TEXTURE_2D);
	//CHECK_GLERROR();
}

GLText::~GLText()
{
	if (texture) {glDeleteTextures(1,&texture);}
}



void GLText::paint()
{
	glBindTexture(GL_TEXTURE_2D,texture);
	
	glBegin(GL_QUADS);
	
	glTexCoord2f(0.0, 0.0);
	glVertex3f(0,-descent,0);
	
	glTexCoord2f(1.0,0.0);
	glVertex3f(w,-descent,0);
	
	glTexCoord2f(1.0, 1.0);
	glVertex3f(w,ascent,0);
	
	glTexCoord2f(0, 1.0);
	glVertex3f(0,ascent,0);
	
	glEnd();
	
}
