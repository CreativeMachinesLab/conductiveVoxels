/*******************************************************************************
Copyright (c) 2010, Jonathan Hiller
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * The name if its contributors may not be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#include "QVX_Interfaces.h"
#include <QMessageBox>
#include <QTime>
#include "VXS_Voxel.h"
#include "VXS_Bond.h"
#include "VX_MeshUtil.h"



#ifdef DMU_ENABLED
#include "DMU.h"
#endif


//QVX_Object:
void QVX_Object::Save(int Compression, bool NewLoc) //saves the file, or prompts if not yet been saved
{
	QString tmpPath = QString(Path.c_str());
	if (tmpPath == "" || NewLoc){ //if file path is empty string (IE not been saved yet)
		tmpPath = QFileDialog::getSaveFileName(NULL, "Save VXC", "", "Voxel CAD Files (*.vxc)");
		if (tmpPath == "") return; //if we canceled the dialog...
	}

	if (!tmpPath.isNull()){
		Path = tmpPath.toStdString();
		SaveVXCFile(Path, Compression);
	}
	else Path = "";
}

bool QVX_Object::Open(void) //Brings up file dialog to open VXC file
{
#ifdef DMU_ENABLED
	QString tmpPath = QFileDialog::getOpenFileName(NULL, "Open VXC", "", "Voxel CAD Files (*.vxc *.dmf);;DMUnit Files (*.dmu)");
#else
	QString tmpPath = QFileDialog::getOpenFileName(NULL, "Open VXC", "", "Voxel CAD Files (*.vxc *.dmf)");
#endif
	
	if (!tmpPath.isNull()){
		Close();
		#ifdef DMU_ENABLED
		if (tmpPath.right(3).compare("dmu", Qt::CaseInsensitive) == 0) ImportDMU(tmpPath.toStdString(), this);
		else
		#endif
		LoadVXCFile(tmpPath.toStdString());
		return true;
	}
	return false;
}

bool QVX_Object::OpenPal(void) //Open a palette
{
	QString TmpPath = QFileDialog::getOpenFileName(NULL, "Open Palette", "", "VoxCad Palette Files (*.vxp *.pal)");;
	
	if(!TmpPath.isNull()){
		LoadVXPFile(TmpPath.toStdString());
		return true;
	}
	return false;
}


void QVX_Object::SavePal(void) //save a palette
{
	QString TmpPath = QFileDialog::getSaveFileName(NULL, "Save Palette", "", "VoxCad Palette Files (*.vxp)");
	SaveVXPFile(TmpPath.toStdString()); //store only the palette
}

void QVX_Object::ExportSTL(void)
{
	QString TmpPath = QFileDialog::getSaveFileName(NULL, "Export STL", "", "Stereolithography Files (*.stl)");
	CVX_MeshUtil::VXCToSTL(TmpPath.toStdString(), this);
}

//QIcon QVX_Object::GenerateMatIcon(int MatIndex)
//{
//
//}


//QVX_FEA:

void QVX_FEA::RequestSolveSystem(void)
{
	bool Success = true;
	QString RetMessage = "";

	Thread MyThread(&RetMessage);
	MyThread.LinkProgress(&CurProgTick, &CurProgMaxTick, &CurProgMsg, &CancelFlag);

	connect(&MyThread, SIGNAL(CallFunc(QString*)), this, SLOT(ExecuteSolveSystem(QString*)), Qt::DirectConnection);
	MyThread.Execute();

	//broadcast if we succesfully completed the simulation
	if (RetMessage != ""){
		QMessageBox::warning(NULL, "Warning", RetMessage);
		Success = false; //check if we were successful, first! (any return string is an error...)
	}
	emit SolveResult(Success);
}

void QVX_FEA::ExecuteSolveSystem(QString* Param)
{
	bool KeepGoing = true;
	std::string ParamString = Param->toStdString();

	if (!ImportObj(NULL, &ParamString)) KeepGoing = false; //import the linked VXC oject
	if (KeepGoing) Solve(&ParamString); //Solve the system
	*Param = ParamString.c_str();
}


//Environment
bool QVX_Environment::OpenBCs(void)
{
	QString TmpPath = QFileDialog::getOpenFileName(NULL, "Open Boundary Conditions", "", "VoxCad Boundary Condition Files (*.bcx)");;
	
	if(!TmpPath.isNull()){
		LoadBCXFile(TmpPath.toStdString());
		emit BCsChanged();
		return true;
	}
	return false;
}

void QVX_Environment::SaveBCs(void)
{
	QString TmpPath = QFileDialog::getSaveFileName(NULL, "Save Boundary Conditions", "", "DM Boundary Condition Files (*.bcx)");
	SaveBCXFile(TmpPath.toStdString()); //store only the palette
}

QVX_Sim::QVX_Sim(QWidget *parent)
{
	SimMessage = ""; 
	LogEvery = false; 
	ApproxMSperLog = 1.0f; 
	
	connect(&SimThread, SIGNAL(CallFunc(QString*)), this, SLOT(SimLoop(QString*)), Qt::DirectConnection);

	Running = false;
	Paused = false;
	StopSim = false;
}

bool QVX_Sim::OpenVXA(void)
{
	QString tmpPath = QFileDialog::getOpenFileName(NULL, "Open VoxCad Analysis", "", "VoxCad Analysis Files (*.vxa *.dmfea)");
	if (!tmpPath.isNull()){
		LoadVXAFile(tmpPath.toStdString());
		emit BCsChanged();
		return true;
	}
	return false;
}

void QVX_Sim::SaveVXA(void)
{
	QString TmpPath = QFileDialog::getSaveFileName(NULL, "Save VoxCad Analysis", "", "VoxCad Analysis Files (*.vxa)");
	SaveVXAFile(TmpPath.toStdString()); //store only the palette
}
void QVX_Sim::RequestBeginSim()
{

	SimThread.SArg1 = &SimMessage;
	SimThread.Execute(false);
}

void QVX_Sim::SimLoop(QString* pSimMessage)
{
	std::string tmp; //need to link this up to get info back...
	Import(NULL, &tmp);

	int Count = 0;
	int LastCount = 0;
	double Time = 0.0; //in seconds
	QString Message;
	std::string RetMsg;
	QTime tLastPlot;
	QTime tLastStatus;
	tLastPlot.start();
	tLastStatus.start();

	emit StartExternalGLUpdate();
	StopSim = false;
	Running = true;
	Paused = false;


	while (TimeStep(&RetMsg)){ //do this step...
		if (StopSim) break;
		while(Paused){
			if (StopSim) break; //kick out of the loop if we've stopped...
			Sleep(100);
		}

		Time += dt;
		pEnv->UpdateCurTemp(Time);

		double UpStatEv = 500; //in ms
		if(tLastStatus.elapsed() > UpStatEv){
			Message = "Step " + QString::number(Count) + "\nTime Step = " + QString::number(dt)+"\nRate = "+QString::number((Count-LastCount)/(UpStatEv/1000.0))+" Steps/sec\n";
			
			if (LogEvery) ApproxMSperLog = (double)tLastStatus.elapsed()/(Count-LastCount); //only update ms per step if its not fixed by the timer
			LastCount = Count;

			emit UpdateText(Message);
			tLastStatus.restart();
		}
			
		if (LogEvery){
			emit ReqAddPlotPoint(Time);
		}
		else if(tLastPlot.elapsed() > 30){
			ApproxMSperLog = 30;
			emit ReqAddPlotPoint(Time);
			tLastPlot.restart();
		}


		Count++;
	}

	emit StopExternalGLUpdate();

//	emit SetExternalGLUpdate(false);
	Running = false;
	Paused = false;
	RetMsg += "Simulation Stopped\n";


	Message = RetMsg.c_str();
	Message = Message + "\nStep = " + QString::number(Count) + "\nTime Step = " + QString::number(dt)+"\n";
	emit UpdateText(Message);
//	emit ReqGLUpdate();

	Running = false;
}

bool QVX_Sim::TimeStepMT(std::string* pRetMessage)
{
	if(SelfColEnabled) UpdateCollisions(); //update self intersection lists if necessary
	IntegrateMT(CurIntegrator);
	return UpdateStats(pRetMessage);
}

void QVX_Sim::IntegrateMT(IntegrationType Integrator)
{
	switch (Integrator){
		case I_EULER: {
			QFuture<void> res = QtConcurrent::map(BondArray.begin(), BondArray.end(), QVX_Sim::BondCalcForce);
			res.waitForFinished();
	

			////Update Forces...
			//int iT = NumBond();
			//for (int i=0; i<iT; i++){
			//	QFuture<void> future = QtConcurrent::run(BondCalcForce, &BondArray[i]);
			//	//BondArray[i].CalcForce();
			//}

			dt = CalcMaxDt();

			res = QtConcurrent::map(VoxArray.begin(), VoxArray.end(), QVX_Sim::VoxEulerStep);
			res.waitForFinished();

			//Update positions... need to do thises seperately if we're going to do damping in the summing forces stage.
	//		int iT = NumVox();
	//		for (int i=0; i<iT; i++) VoxArray[i].EulerStep();

		}
		break;
	}
}


void QVX_Sim::PauseSim()
{
	Paused = !Paused;
}

void QVX_Sim::EndSim()
{
	StopSim = true;
	SimThread.wait(); //wait until it actually exits...
	GeoMesh.Clear();
}

void QVX_Sim::ResetSim()
{
	ResetSimulation();
}

void QVX_Sim::LMouseDown(Vec3D P)
{
	if (CurXSel != -1){
//		if (InputVoxSInd == -1) //if the input voxel has not yet been added to the voxel array...
//			CVoxelArray

		CVXS_Voxel& CurVox = VoxArray[XtoSIndexMap[CurXSel]];

		InputVoxel()->S.Pos = CurVox.S.Pos;
		InputVoxel()->MatIndex = CurVox.MatIndex;

		InputBond()->Vox1SInd = XtoSIndexMap[CurXSel];
		InputBond()->Vox2SInd = InputVoxSInd;
		InputBond()->CalcConsts();



		PointerOffset = P - CurVox.S.Pos;
		Dragging = true;
	}
	else {
		Dragging = false;
	}
}

void QVX_Sim::LMouseUp(Vec3D P)
{
	Dragging = false;

}

void QVX_Sim::LMouseDownMove(Vec3D P)
{
	InputVoxel()->S.Pos = P-PointerOffset;
}


