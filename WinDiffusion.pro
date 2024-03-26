QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += force_debug_info

CONFIG += c++20
QMAKE_CXXFLAGS += /await:strict
win32:LIBS += -lwindowsapp -lOLEAUT32 -lOle32 -lgdi32 -lUser32

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ESRGAN.cpp \
    canvas/drawgraphicsview.cpp \
    canvas/drawingscene.cpp \
    canvas/drawpixmapitem.cpp \
    canvas/layerwidget.cpp \
    canvasinferer.cpp \
    canvastab.cpp \
    clickableimagelabel.cpp \
    draggablelistwidget.cpp \
    ext/ColorUtils.cpp \
    ext/qfloodfill.cpp \
    horizontalscrollarea.cpp \
    inferer.cpp \
    main.cpp \
    mainwindow.cpp \
    paintablecanvas.cpp \
    pathwidgetitem.cpp \
    renderconfigform.cpp \
    stablediffusionmodel.cpp \
    topbarimg.cpp \
    upscaler.cpp

HEADERS += \
    ESRGAN.h \
    QtAxodoxInteropCommon.hpp \
    canvas/drawgraphicsview.h \
    canvas/drawingscene.h \
    canvas/drawpixmapitem.h \
    canvas/layerwidget.h \
    canvasinferer.h \
    canvastab.h \
    clickableimagelabel.h \
    draggablelistwidget.h \
    ext/ColorUtils.hpp \
    ext/qfloodfill.h \
    horizontalscrollarea.h \
    inferer.h \
    mainwindow.h \
    paintablecanvas.h \
    pathwidgetitem.h \
    renderconfigform.h \
    stablediffusionmodel.h \
    threadsafequeue.hpp \
    topbarimg.h \
    upscaler.h

FORMS += \
    canvas/layerwidget.ui \
    canvastab.ui \
    mainwindow.ui \
    renderconfigform.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


INCLUDEPATH += $$PWD/deps/include
INCLUDEPATH += $$PWD/deps/include-axcommon
INCLUDEPATH += $$PWD/deps/include-axml
INCLUDEPATH += $$PWD/deps/include-goodwindow
win32: LIBS += -L$$PWD/deps/lib/ QGoodWindow.lib Axodox.Common.lib DirectML.lib onnxruntime.lib Axodox.MachineLearning.lib QtColorWidgets2.lib
#win32: LIBS += Advapi32.lib User32.lib Psapi.lib

RESOURCES += \
    stdres.qrc


