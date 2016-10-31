#ifndef QVX_INTERFACES_H
#define QVX_INTERFACES_H

//wraps classes with QT slots, dialog interfaces, etc:

#include <QWidget>
#include "VX_Object.h"
#include "VX_FEA.h"
#include "VX_Environment.h"
#include "VX_Sim.h"
#include <QFileDialog>
#include "QThreadWrap.h"

//for multithreading
#include <QtConcurrentMap>



class QVX_Object : public QWidget, public CVX_Object
{
	Q_OBJECT

public:
	
	QVX_Object(QWidget *parent = 0){Path = "";};
	~QVX_Object(){};
	QVX_Object& operator=(const QVX_Object& RefObj); //overload "=" 

//	Path = RefObj.Path;


public slots:
	void GetDim(Vec3D* pVec) {GetWorkSpace(pVec);};

	//high level file I/O functions
	void New(void) {Close(); InitializeMatter(0.001, 10, 10, 10);};
	void Save(int Compression = CP_ZLIB, bool NewLoc = false);
	void SaveZLib(void) {Save(CP_ZLIB);};
	void SaveAsZLib(void) {Save(CP_ZLIB, true);};
	void SaveAsBase64(void) {Path = ""; Save(CP_BASE64);};
	void SaveAsAsciiReadable(void) {Path = ""; Save(CP_ASCIIREADABLE);};
	void ExportSTL(void);
	bool Open(void);
	void Close(void) {ClearMatter(); Path = "";};
	
	bool OpenPal(void); //open material palette
	void SavePal(void); //save material palette

	void GetVXCInfoString(QString* pString) {std::string tmp; GetVXCInfoStr(&tmp); *pString = QString(tmp.c_str());};
	void GetVoxInfoString(int Index, QString* pString) {std::string tmp; GetVoxInfoStr(Index, &tmp); *pString = QString(tmp.c_str());};

//	QIcon GenerateMatIcon(int MatIndex);

protected:
	std::string Path; //current file path

private:
	
};

class QVX_FEA : public QWidget, public CVX_FEA
{
	Q_OBJECT

public:
	QVX_FEA(QWidget *parent = 0){};
	~QVX_FEA(){};

	int GetCurSel(void) {int tmp; emit GetCurGLSelected(&tmp); return tmp;};
signals:
	void SolveResult(bool);
	void GetCurGLSelected(int* CurSel);

public slots:

	void RequestSolveSystem(void);
	void ExecuteSolveSystem(QString* Param = NULL);
	void DrawScene(void) {DrawFEA(GetCurSel());}
//	void SetViewConstraints(bool View) {ViewConstraints = View;}
	void SetViewDisplaced(bool View) {if (View) ViewDefPerc = 2.0; else ViewDefPerc = 0.0;}
	void SetViewModeDisplacement(void) {ViewMode = VIEW_DISP;}
	void SetViewModeForce(void) {ViewMode = VIEW_FORCE;}
	void SetViewModeStrain(void) {ViewMode = VIEW_STRAIN;}
	void SetViewModeReaction(void) {ViewMode = VIEW_REACTION;}



	void GetFEAInfoString(QString* pString) {std::string tmp; GetFEAInfoStr(&tmp); *pString = QString(tmp.c_str());};
	void GetFEAInfoString(int VoxIndex, QString* pString) {std::string tmp; GetFEAInfoStr(VoxIndex, &tmp); *pString = QString(tmp.c_str());};

private:
	
};


class QVX_Environment : public QWidget, public CVX_Environment
{
	Q_OBJECT

public:
	QVX_Environment(QWidget *parent = 0){};
	~QVX_Environment(){};

signals:
	void BCsChanged();

public slots:
	bool OpenBCs(void);
	void SaveBCs(void);
};

class QVX_Sim : public QWidget, public CVX_Sim
{
	Q_OBJECT

public:
	
	QVX_Sim(QWidget *parent = 0);
	~QVX_Sim(){EndSim();};

	Thread SimThread; //the simulation
	QString SimMessage; //simulation has access to change this...

	bool LogEvery; //do we want to log every data point, or at fixed (real) time intervals?
	float ApproxMSperLog;

	
	bool Running; //simulation running?
	bool Paused; //sim paused?
	bool StopSim; //Stop Simulation?

	//reimplemented timestep and integration functions to multithread this baby with QT threadpool!
	bool TimeStepMT(std::string* pRetMessage = NULL);
	void IntegrateMT(IntegrationType Integrator = I_EULER);

signals:
	void BCsChanged();
	void ReqAddPlotPoint(double time); //adds a point to the plot...
	void UpdateText(QString);
//	void ReqGLUpdate(void);
	void StartExternalGLUpdate();
	void StopExternalGLUpdate();


public slots:
	void SaveVXA(void);
	bool OpenVXA(void);

	void RequestBeginSim();
	void SimLoop(QString* pSimMessage = NULL);
	void PauseSim();
	void EndSim();
	void ResetSim();

	void LMouseDown(Vec3D P);
	void LMouseUp(Vec3D P);
	void LMouseDownMove(Vec3D P);

public:
//	void BondCalcForce(std::vector<CVXS_Bond>::iterator Bond) {(*Bond).CalcForce();}

static void BondCalcForce(CVXS_Bond &Bond) {Bond.CalcForce();}
static void VoxEulerStep(CVXS_Voxel &Voxel) {Voxel.EulerStep();}



};



#endif // QVX_INTERFACES_H
