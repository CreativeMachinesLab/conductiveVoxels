#ifndef QVX_TENSILETEST_H
#define QVX_TENSILETEST_H

#include "../Voxelyze/VX_Sim.h"
#include "../QTUtils/QThreadWrap.h"


class QVX_Environment;


class QVX_TensileTest : public QWidget, public CVX_Sim
{
	Q_OBJECT

public:
	QVX_TensileTest();
	~QVX_TensileTest();

	Thread TensileThread;
	QString OutFilePath;

	bool DoBCChecks(void); //returns true if passes all checks
	bool IsBasicTensile; //true if two BC, one fixed, one displaced axially at two ends.
	double CSArea; //cross sectional area of the tensile specimen.
	double IniLength; //initial length of the tensile specimen.
	Axis TensileAxis;
	bool TestRunning;

public slots:
	void BeginTensileTest(QVX_Environment* pEnvIn, int NumStepIn, double ConvThreshIn, double MixRadiusIn = 0.0, MatBlendModel ModelIn = MB_LINEAR, double PolyExpIn = 1.0);
	void RunTensileTest(QString* pMessage = NULL);

signals:
	void StartExternalGLUpdate(int);
	void StopExternalGLUpdate();

private:
	void RenderMixedObject(CVX_Object* pSrcObj, CVX_Object* pDestObj, double MixRadius);

	int NumStep;
	double ConvThresh;
//	double MixRadius;

	//for threading:
	int CurTick, TotalTick;
	bool CancelFlag;
	std::string ProgressMessage;

	CVX_Environment LocEnv; //local objects to run the test on so we don't modify the main object..
	CVX_Object LocObj;
};

#endif // QVX_TENSILETEST_H
