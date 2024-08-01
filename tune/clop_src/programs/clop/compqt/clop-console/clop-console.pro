# -------------------------------------------------
# Project created by QtCreator 2010-05-14T18:25:44
# -------------------------------------------------
linux-g++:QMAKE_CXX = 'ccache g++'
DEFINES += restrict=__restrict__
INCLUDEPATH += ../../../rclib/src/util \
    ../../src \
    ../../src/artificial \
    ../../src/real \
    ../../src/math \
    ../../../plot/src \
    ../../../math/src
QMAKE_CXXFLAGS_WARN_ON = -Wall \
    -pedantic \
    -Wextra \
    -Wno-unused-parameter \
    -Wno-long-long
QT -= gui
TARGET = clop-console
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += ../../src/real/clop-console.cpp \
    ../../src/real/CRealObserver.cpp \
    ../../src/real/CRealExperiment.cpp \
    ../../src/real/CLinearParameter.cpp \
    ../../src/CResults.cpp \
    ../../src/CObserver.cpp \
    ../../src/real/CProblemRunner.cpp \
    ../../src/CSPUniform.cpp \
    ../../../rclib/src/util/userflag.cpp \
    ../../src/CSPWeight.cpp \
    ../../src/CRegression.cpp \
    ../../src/CPFQuadratic.cpp \
    ../../src/CPFIndependentQuadratic.cpp \
    ../../src/CDFVariance.cpp \
    ../../src/math/CMatrixOperations.cpp \
    ../../src/math/CDiffFunction.cpp \
    ../../src/CDFConfidence.cpp \
    ../../src/CParametricFunction.cpp \
    ../../src/real/CExperimentSettings.cpp \
    ../../src/real/CExperimentFromSettings.cpp \
    ../../src/real/CExperimentLogFile.cpp \
    ../../src/real/CIntegerParameter.cpp \
    ../../src/real/CIntegerGammaParameter.cpp \
    ../../src/CPFConstant.cpp \
    ../../src/real/CParameterCollection.cpp \
    ../../src/CMESampleMean.cpp \
    ../../src/CEigenSystem.cpp \
    ../../src/real/CGammaParameter.cpp
HEADERS += ../../src/real/CRealObserver.h \
    ../../src/real/CRealExperiment.h \
    ../../src/real/CParameterCollection.h \
    ../../src/real/CParameter.h \
    ../../src/real/CLinearParameter.h \
    ../../src/CResults.h \
    ../../src/CObserver.h \
    ../../src/real/CRealProblem.h \
    ../../src/CSPUniform.h \
    ../../src/CMaxEstimator.h \
    ../../src/CSPWeight.h \
    ../../src/CRegression.h \
    ../../src/CMERegressionMAPMax.h \
    ../../src/CPFQuadratic.h \
    ../../src/CPFIndependentQuadratic.h \
    ../../src/CDFVariance.h \
    ../../src/math/CMatrixOperations.h \
    ../../src/math/CLogistic.h \
    ../../src/math/CDiffFunction.h \
    ../../src/CDFConfidence.h \
    ../../src/CParametricFunction.h \
    ../../src/real/CExperimentSettings.h \
    ../../src/real/CExperimentFromSettings.h \
    ../../src/real/CExperimentLogFile.h \
    ../../src/real/CIntegerParameter.h \
    ../../src/real/CIntegerGammaParameter.h \
    ../../src/CPFConstant.h \
    ../../src/real/CProblemRunner.h \
    ../../src/CMESampleMean.h \
    ../../src/CEigenSystem.h
