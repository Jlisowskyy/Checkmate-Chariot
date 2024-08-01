INCLUDES := $(INCLUDES)\
 -I../../clop/src\
 -I../../clop/src/real\
 -I../../clop/src/artificial\
 -I../../clop/src/math

vpath %cpp ../../clop/src
vpath %cpp ../../clop/src/real
vpath %cpp ../../clop/src/artificial
vpath %cpp ../../clop/src/math

OUTDIR := ../../clop/compgcc/bin
include $(DEPEND)

#LDFLAGS := $(LDFLAGS) -lgsl -lgslcblas

ifeq ($(OSTYPE),cygwin)
 LDFLAGS := $(LDFLAGS) -lboost_thread-gcc-mt -lboost_filesystem-gcc-mt
else
 LDFLAGS := $(LDFLAGS) -lboost_thread -I/usr/include/python3.12
endif

OBJECTS := $(OBJECTS) $(patsubst %.o, $(OUTDIR)/%.o,\
 C2SPSA.o\
 CArtificialExperiment.o\
 CArtificialProblem.o\
 CBAST.o\
 CCheckPointData.o\
 CCPLConsole.o\
 CCrossEntropy.o\
 CDiffFunction.o\
 CDFConfidence.o\
 CDFConfidenceBound.o\
 CDFExpectedMC.o\
 CDFRatingLCB.o\
 CDFVariance.o\
 CDFVarianceAlpha.o\
 CDFVarianceDelta.o\
 CMatrixOperations.o\
 CMatrixIO.o\
 CMEQLRLCB.o\
 CMESampleMean.o\
 CObserver.o\
 CParametricFunction.o\
 CPDonut.o\
 CPFlat.o\
 CPFConcave.o\
 CPFConstant.o\
 CPFCubic.o\
 CPFGaussian.o\
 CPFIndependentQuadratic.o\
 CPFQuadratic.o\
 CPMultiple.o\
 CPNonQuadratic2D.o\
 CPNonQuadraticND.o\
 CPQuadratic1D.o\
 CPQuadratic2D.o\
 CPQuadraticND.o\
 CPRosenbrock.o\
 CPVolcano.o\
 CQLRIO.o\
 CRegression.o\
 CRepeatThread.o\
 CRepeatThreads.o\
 CResults.o\
 CRSPSA.o\
 CSPComposite.o\
 CSPCOptimal.o\
 CSPCOptimalMAP.o\
 CSPDyadic.o\
 CSPQLRMax.o\
 CSPSA.o\
 CSPUCB.o\
 CSPUniform.o\
 CSPVOptimal.o\
 CSPWeight.o\
 CTikZ.o\
 CTikZOneD.o\
 CTikZTwoD.o\
)
