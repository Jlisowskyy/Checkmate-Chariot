# -------------------------------------------------
# Project created by QtCreator 2010-05-16T16:01:09
# -------------------------------------------------
TARGET = clop-gui
TEMPLATE = app
VERSION = 0.0.9
INCLUDEPATH += ../../src/real \
    ../../src/qt-gui \
    ../../src/math \
    ../../src \
    ../../../rclib/src/util
SOURCES += ../../src/qt-gui/clop-gui.cpp \
    ../../src/qt-gui/CPlotWidget.cpp \
    ../../src/qt-gui/MainWindow.cpp \
    ../../src/qt-gui/AboutDialog.cpp \
    ../../src/qt-gui/CRAIICursor.cpp \
    ../../src/qt-gui/CCLOPSyntaxHighlighter.cpp \
    ../../src/real/CRealExperiment.cpp \
    ../../src/real/CLinearParameter.cpp \
    ../../src/real/CExperimentSettings.cpp \
    ../../src/real/CExperimentFromSettings.cpp \
    ../../src/real/CProblemRunner.cpp \
    ../../src/CResults.cpp \
    ../../src/CObserver.cpp \
    ../../src/CRegression.cpp \
    ../../src/CPFQuadratic.cpp \
    ../../src/CPFIndependentQuadratic.cpp \
    ../../src/math/CMatrixOperations.cpp \
    ../../src/math/CDiffFunction.cpp \
    ../../src/CParametricFunction.cpp \
    ../../src/CDFConfidence.cpp \
    ../../src/real/CExperimentLogFile.cpp \
    ../../../rclib/src/util/userflag.cpp \
    ../../src/CSPWeight.cpp \
    ../../src/CDFVariance.cpp \
    ../../src/real/CIntegerParameter.cpp \
    ../../src/real/CIntegerGammaParameter.cpp \
    ../../src/real/CParameterCollection.cpp \
    ../../src/CMESampleMean.cpp \
    ../../src/CEigenSystem.cpp \
    ../../src/real/CGammaParameter.cpp
HEADERS += ../../src/qt-gui/CPlotWidget.h \
    ../../src/qt-gui/MainWindow.h \
    ../../src/qt-gui/AboutDialog.h \
    ../../src/qt-gui/CCLOPSyntaxHighlighter.h \
    ../../src/qt-gui/CRAIICursor.h \
    ../../src/real/CRealProblem.h \
    ../../src/real/CRealExperiment.h \
    ../../src/real/CParameterCollection.h \
    ../../src/real/CParameter.h \
    ../../src/real/CLinearParameter.h \
    ../../src/real/CExperimentSettings.h \
    ../../src/real/CExperimentFromSettings.h \
    ../../src/real/CProblemRunner.h \
    ../../src/CResults.h \
    ../../src/COutcome.h \
    ../../src/CObserver.h \
    ../../src/CRegression.h \
    ../../src/CMERegressionMAPMax.h \
    ../../src/CPFQuadratic.h \
    ../../src/CPFIndependentQuadratic.h \
    ../../src/CDFVariance.h \
    ../../src/math/CMatrixOperations.h \
    ../../src/math/CLogistic.h \
    ../../src/math/CDiffFunction.h \
    ../../src/CSPVOptimal.h \
    ../../src/CParametricFunction.h \
    ../../src/CDFConfidence.h \
    ../../src/real/CExperimentLogFile.h \
    ../../../rclib/src/util/userflag.h \
    ../../src/CSPWeight.h \
    ../../src/real/CIntegerParameter.h \
    ../../src/real/CIntegerGammaParameter.h \
    ../../src/CMESampleMean.h \
    ../../src/CEigenSystem.h
FORMS += ../../src/qt-gui/MainWindow.ui \
    ../../src/qt-gui/AboutDialog.ui
DEFINES += APP_VERSION=\\\"$$VERSION\\\"
DEFINES += restrict=__restrict__
QMAKE_CXXFLAGS_WARN_ON = -Wall \
    -pedantic \
    -Wextra \
    -Wno-unused-parameter \
    -Wno-long-long
linux-g++:QMAKE_CXX = 'ccache g++'
win32:RC_FILE = ../../src/qt-gui/clop-gui.rc
