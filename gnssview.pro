HEADERS       = GLText.h \
								GNSSView.h \
								GNSSViewWidget.h \
								GNSSViewApp.h \
								GNSSSV.h \
								Sun.h \
								Colour.h \
								PowerManager.h \
								SkyModel.h
SOURCES       = GLText.cpp \
								GNSSView.cpp \
								GNSSViewWidget.cpp \
								GNSSViewApp.cpp \
								GNSSSV.cpp \
								Sun.cpp \
								Colour.cpp \
								PowerManager.cpp \
								SkyModel.cpp \
                Main.cpp
QT           += network opengl xml
CONFIG       += 
DEFINES      += QT_NO_DEBUG_OUTPUT
LIBS	       += -lGLU