/*******************************************************************************
Copyright (c) 2010, Jonathan Hiller
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * The name if its contributors may not be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/


#include "Dlg_Physics.h"

//#include <qwt_plot.h>
#include <qwt_plot_curve.h>
//#include <qwt_plot_directpainter.h>
//#include <qwt_plot_layout.h>


//TEMP
//#include "VXS_Voxel.h"

Dlg_Physics::Dlg_Physics(QVX_Sim* pSimIn, QWidget *parent)
	: QWidget(parent)
{
	pSim = pSimIn;
	ui.setupUi(this);

	//these should be in the order of the stop condition enum
	ui.StopSelectCombo->addItem("None");
	ui.StopSelectCombo->addItem("Time Steps");
	ui.StopSelectCombo->addItem("Simulation Time");
	ui.StopSelectCombo->addItem("Temperature cycles");
	ui.StopSelectCombo->addItem("Constant Energy");
	ui.StopSelectCombo->addItem("Kinetic Floor");
	ui.StopSelectCombo->addItem("Motion Floor");

	//these should match the order of the PlotType enum
	ui.VariableCombo->addItem("Displacement");
//	ui.VariableCombo->addItem("Force");
	ui.VariableCombo->addItem("Kinetic Energy");
	ui.VariableCombo->addItem("Potential Energy");
	ui.VariableCombo->addItem("Total Energy");

	//these should match the order of the PlotDir enum
	ui.DirectionCombo->addItem("Maximum");
	ui.DirectionCombo->addItem("X Direction");
	ui.DirectionCombo->addItem("Y Direction");
	ui.DirectionCombo->addItem("Z Direction");

	//enforce type-ins
	const QValidator* DEval = new QDoubleValidator(this);
	ui.dtEdit->setValidator(DEval);

	CurPlotType = PL_DISP;
	CurPlotDir = MAXDIR;

	connect(ui.PauseButton, SIGNAL(clicked()), this, SLOT(ClickedPause()));
	connect(ui.ResetButton, SIGNAL(clicked()), this, SLOT(ClickedReset()));
	connect(ui.RecordButton, SIGNAL(clicked(bool)), this, SLOT(ClickedRecord(bool)));

	
	connect(ui.UseEquilibriumCheck, SIGNAL(clicked(bool)), this, SLOT(UseEquilibriumCheckChanged(bool)));
	connect(ui.StopSelectCombo, SIGNAL(activated(int)), this, SLOT(StopSelectChanged(int)));
	connect(ui.StopValueEdit, SIGNAL(editingFinished()), this, SLOT(StopValueEditChanged()));

	connect(ui.dtSlider, SIGNAL(valueChanged(int)), this, SLOT(dtSliderChanged(int)));
	connect(ui.dtEdit, SIGNAL(editingFinished()), this, SLOT(dtEditChanged()));
	connect(ui.BondDampSlider, SIGNAL(valueChanged(int)), this, SLOT(BondDampSliderChanged(int)));
	connect(ui.BondDampEdit, SIGNAL(editingFinished()), this, SLOT(BondDampEditChanged()));
	connect(ui.GNDDampSlider, SIGNAL(valueChanged(int)), this, SLOT(GNDDampSliderChanged(int)));
	connect(ui.GNDDampEdit, SIGNAL(editingFinished()), this, SLOT(GNDDampEditChanged()));
	connect(ui.ColDampSlider, SIGNAL(valueChanged(int)), this, SLOT(ColDampSliderChanged(int)));
	connect(ui.ColDampEdit, SIGNAL(editingFinished()), this, SLOT(ColDampEditChanged()));
	connect(ui.MaxVelLimitSlider, SIGNAL(valueChanged(int)), this, SLOT(MaxVelLimitSliderChanged(int)));
	connect(ui.UseSelfColCheck, SIGNAL(clicked(bool)), this, SLOT(UseSelfColCheckChanged(bool)));
	connect(ui.UseMaxVelLimitCheck, SIGNAL(clicked(bool)), this, SLOT(UseMaxVelLimitCheckChanged(bool)));


	connect(ui.UseTempCheck, SIGNAL(clicked(bool)), this, SLOT(UseTempCheckChanged(bool)));
	connect(ui.TempSlider, SIGNAL(valueChanged(int)), this, SLOT(TempSliderChanged(int)));
	connect(ui.TempEdit, SIGNAL(editingFinished()), this, SLOT(TempEditChanged()));
	connect(ui.VaryTempCheck, SIGNAL(clicked(bool)), this, SLOT(VaryTempCheckChanged(bool)));
	connect(ui.TempPerSlider, SIGNAL(valueChanged(int)), this, SLOT(TempPerSliderChanged(int)));
	connect(ui.TempPerEdit, SIGNAL(editingFinished()), this, SLOT(TempPerEditChanged()));
	//connect(ui.TempPerEdit, SIGNAL(textChanged(QString)), this, SLOT(TempPerEditTextChanged(QString)));
	connect(ui.UseGravCheck, SIGNAL(clicked(bool)), this, SLOT(UseGravCheckChanged(bool)));
	connect(ui.GravSlider, SIGNAL(valueChanged(int)), this, SLOT(GravSliderChanged(int)));
	connect(ui.GravEdit, SIGNAL(editingFinished()), this, SLOT(GravEditChanged()));

	connect(ui.UseFloorCheck, SIGNAL(clicked(bool)), this, SLOT(UseFloorCheckChanged(bool)));

	connect(ui.DispDisableRadio, SIGNAL(clicked(bool)), this, SLOT(DisplayDisableChanged(bool)));
	connect(ui.DispVoxelsRadio, SIGNAL(clicked(bool)), this, SLOT(DisplayVoxChanged(bool)));
	connect(ui.DispConnRadio, SIGNAL(clicked(bool)), this, SLOT(DisplayConChanged(bool)));


	connect(ui.ViewDiscreteRadio, SIGNAL(clicked(bool)), this, SLOT(VoxDiscreteChanged(bool)));
	connect(ui.ViewDeformedRadio, SIGNAL(clicked(bool)), this, SLOT(VoxDeformedChanged(bool)));
	connect(ui.ViewSmoothRadio, SIGNAL(clicked(bool)), this, SLOT(VoxSmoothChanged(bool)));
//	connect(ui.BondsCheck, SIGNAL(clicked(bool)), this, SLOT(BondsCheckChanged(bool)));
	connect(ui.ForcesCheck, SIGNAL(clicked(bool)), this, SLOT(ForcesCheckChanged(bool)));
	connect(ui.LocalCoordCheck, SIGNAL(clicked(bool)), this, SLOT(LCsCheckChanged(bool)));

	connect(ui.TypeRadio, SIGNAL(clicked(bool)), this, SLOT(CTypeChanged(bool)));
	connect(ui.KineticERadio, SIGNAL(clicked(bool)), this, SLOT(CKinEChanged(bool)));
	connect(ui.DisplacementRadio, SIGNAL(clicked(bool)), this, SLOT(CDispChanged(bool)));
	connect(ui.StateRadio, SIGNAL(clicked(bool)), this, SLOT(CStateChanged(bool)));
	connect(ui.StrainERadio, SIGNAL(clicked(bool)), this, SLOT(CStrainEChanged(bool)));
	connect(ui.StrainRadio, SIGNAL(clicked(bool)), this, SLOT(CStrainChanged(bool)));
	connect(ui.StressRadio, SIGNAL(clicked(bool)), this, SLOT(CStressChanged(bool)));

	connect(ui.CoMCheck, SIGNAL(clicked(bool)), this, SLOT(CmMCheckChanged(bool)));
//	connect(ui.BbCheck, SIGNAL(clicked(bool)), this, SLOT(BbCheckChanged(bool)));

	connect(ui.VariableCombo, SIGNAL(activated(int)), this, SLOT(VarComboChanged(int)));
	connect(ui.DirectionCombo, SIGNAL(activated(int)), this, SLOT(DirComboChanged(int)));
	connect(ui.LogEachCheck, SIGNAL(clicked(bool)), this, SLOT(LogEachCheckChanged(bool)));
	connect(ui.SaveDataButton, SIGNAL(clicked()), this, SLOT(ClickedSaveData()));

	//set up plot for real-time display...
//	d_directPainter = new QwtPlotDirectPainter(ui.DataPlot);
	d_data = NULL;
	d_curve = NULL;

	ui.DataPlot->setAxisMaxMinor(QwtPlot::xBottom, 1);
	ui.DataPlot->setAxisMaxMinor(QwtPlot::yLeft, 1);
//	ui.DataPlot->setAutoReplot(false);
	/*


	d_curve = new QwtPlotCurve("MyData");
	
	double x[3] = {0};
	double y[3] = {0};
	x[1] = 1;
	x[2] = 2;
	y[1] = 1;

	d_curve->attach(ui.DataPlot);
	d_curve->setSamples(x, y, 3);
	ui.DataPlot->setAxisMaxMinor(QwtPlot::xBottom, 1);
	ui.DataPlot->setAxisMaxMinor(QwtPlot::yLeft, 1);
	//ui.DataPlot->setMargin(0);
	//ui.DataPlot->plotLayout()->setMargin(0);
	//ui.DataPlot->plotLayout()->setSpacing(0);
*/

//	ui.DataPlot->
	PlotUpdateTimer = new QTimer(this);
	connect(PlotUpdateTimer, SIGNAL(timeout()), this, SLOT(UpdatePlot()));

	PlotUpdateRate = 33;
	PlotUpdateTimer->start(PlotUpdateRate); //just run continuously...

	TimestepMag = 1.0;

	UpdateUI();
	

}



Dlg_Physics::~Dlg_Physics()
{
	PlotUpdateTimer->stop();
}

void Dlg_Physics::UpdateUI(void)
{
	CVX_Environment* ptEnv = pSim->pEnv; //pointer to current environment object
	bool EqMode = pSim->IsEquilibriumEnabled();
//	TimestepMag = pSim->dt/pSim->DtFrac;

	//Sim
	if (!pSim->Paused && pSim->Running) SetStatusButtonText(true);
	else SetStatusButtonText(false);
	ui.RecordButton->setChecked(pSim->Recording);

	ui.UseEquilibriumCheck->setChecked(pSim->IsEquilibriumEnabled());
	StopCondition CurStopCond = pSim->GetStopConditionType();
	ui.StopSelectCombo->setCurrentIndex(CurStopCond);
	ui.StopValueEdit->setEnabled(CurStopCond != SC_NONE);
	ui.StopValueEdit->setText(QString::number(pSim->GetStopConditionValue()));
	switch (CurStopCond){
	case SC_NONE: ui.StopValueLabel->setText(""); break;
	case SC_MAX_TIME_STEPS: ui.StopValueLabel->setText("#"); break;
	case SC_MAX_SIM_TIME: ui.StopValueLabel->setText("Sec"); break;
	case SC_TEMP_CYCLES: ui.StopValueLabel->setText("#"); break;
	case SC_CONST_MAXENERGY: ui.StopValueLabel->setText("Avg mJ/Vox/500 ts"); break;
	case SC_MIN_KE: ui.StopValueLabel->setText("Avg mJ/Vox/500 ts"); break;
	case SC_MIN_MAXMOVE: ui.StopValueLabel->setText("Max mm/timestep"); break;
	}

	ui.dtSlider->setRange(0, 1000);
	ui.dtSlider->setValue(qRound(pSim->DtFrac*500)); 
	ui.dtEdit->setText(QString::number(pSim->DtFrac, 'g', 3));

	ui.BondDampSlider->setEnabled(!EqMode);
	ui.BondDampEdit->setEnabled(!EqMode);
	ui.BondDampSlider->setRange(0, 100);
	ui.BondDampSlider->setValue(qRound(pSim->GetBondDampZ()*50)); 
	ui.BondDampEdit->setText(QString::number(pSim->GetBondDampZ(), 'g', 3));

	//from .00001 to .1
	ui.GNDDampSlider->setEnabled(!EqMode);
	ui.GNDDampEdit->setEnabled(!EqMode);
	ui.GNDDampSlider->setRange(0, 100);
	if (pSim->GetSlowDampZ() == 0) ui.GNDDampSlider->setValue(0);
	else ui.GNDDampSlider->setValue(qRound((log10(pSim->GetSlowDampZ())+5)*25.0)); 
	ui.GNDDampEdit->setText(QString::number(pSim->GetSlowDampZ(), 'g', 3));

	ui.UseSelfColCheck->setChecked(pSim->IsSelfColEnabled());
	ui.ColDampSlider->setEnabled(pSim->IsSelfColEnabled());
	ui.ColDampSlider->setRange(0, 100);
	ui.ColDampSlider->setValue(qRound(pSim->GetCollisionDampZ()*50)); 
	ui.ColDampEdit->setText(QString::number(pSim->GetCollisionDampZ(), 'g', 3));

	ui.UseMaxVelLimitCheck->setEnabled(!EqMode);
	ui.MaxVelLimitSlider->setEnabled(!EqMode);
	ui.UseMaxVelLimitCheck->setChecked(pSim->IsMaxVelLimitEnabled());
	ui.MaxVelLimitSlider->setEnabled(pSim->IsMaxVelLimitEnabled());
	ui.MaxVelLimitSlider->setRange(0, 100);
	ui.MaxVelLimitSlider->setValue(qRound(pSim->GetMaxVoxVelLimit()*400)); 

	//Env
	ui.UseTempCheck->setChecked(ptEnv->IsTempEnabled());
	ui.TempSlider->setRange(0, 50); //+/- 25 degrees from TempBase
	ui.TempSlider->setValue(qRound(25 + ptEnv->GetTempAmplitude())); 
	ui.TempEdit->setText(QString::number(ptEnv->GetTempBase() + ptEnv->GetTempAmplitude(), 'g', 3));

	ui.VaryTempCheck->setChecked(ptEnv->IsTempVaryEnabled());
	ui.TempPerSlider->setRange(0, 10000);
	ui.TempPerSlider->setValue(qRound(ptEnv->GetTempPeriod()/pSim->OptimalDt)); //slider range of 0-10,000 timesteps
	ui.TempPerEdit->setText(QString::number(ptEnv->GetTempPeriod(), 'g', 3));

	ui.UseGravCheck->setChecked(ptEnv->IsGravityEnabled());
	ui.GravSlider->setRange(0, 10000);
	ui.GravSlider->setValue(qRound(-ptEnv->GetGravityAccel()/0.00981)); //1e-5 takes care for float -> int rounding...
	ui.GravEdit->setText(QString::number(ptEnv->GetGravityAccel(), 'g', 3));

	ui.UseFloorCheck->setChecked(ptEnv->IsFloorEnabled());

//	ui.SurfMeshCheck->setEnabled(false);
//	if (pSim->SurfMesh.DefMesh.Exists()) ui.SurfMeshCheck->setEnabled(true);

	//View
	bool ViewEnabled = true;
	switch (pSim->GetCurViewMode()){
		case RVM_NONE: ui.DispDisableRadio->setChecked(true); ViewEnabled = false; break;
		case RVM_VOXELS: ui.DispVoxelsRadio->setChecked(true); break;
		case RVM_BONDS: ui.DispConnRadio->setChecked(true); break;
	}
	ui.ViewOptionsGroup->setEnabled(ViewEnabled);
	ui.ColorGroup->setEnabled(ViewEnabled);
	ui.CoMCheck->setEnabled(ViewEnabled);
//	ui.BbCheck->setEnabled(ViewEnabled);

	switch (pSim->GetCurViewVox()){
		case RVV_DISCRETE: ui.ViewDiscreteRadio->setChecked(true); break;
		case RVV_DEFORMED: ui.ViewDeformedRadio->setChecked(true); break;
		case RVV_SMOOTH: ui.ViewSmoothRadio->setChecked(true); break;

	}
	ui.ForcesCheck->setChecked(pSim->ViewForce);
	ui.LocalCoordCheck->setChecked(pSim->ViewAngles);

	switch (pSim->GetCurViewCol()){
		case RVC_TYPE: ui.TypeRadio->setChecked(true); break;
		case RVC_KINETIC_EN: ui.KineticERadio->setChecked(true); break;
		case RVC_DISP: ui.DisplacementRadio->setChecked(true); break;
		case RVC_STATE: ui.StateRadio->setChecked(true); break;
		case RVC_STRAIN_EN: ui.StrainERadio->setChecked(true); break;
		case RVC_STRAIN: ui.StrainRadio->setChecked(true); break;
		case RVC_STRESS: ui.StressRadio->setChecked(true); break;
	}

	ui.CoMCheck->setChecked(pSim->LockCoMToCenter);
//	ui.BbCheck->setChecked(pSim->HideBoundingBox);

	ui.VariableCombo->setCurrentIndex(CurPlotType);
	ui.DirectionCombo->setCurrentIndex(CurPlotDir);

}

void Dlg_Physics::ApplyVoxSelection(int NewSel)
{
	DeletePlotPoints();
	pSim->CurXSel = NewSel; //TMP!!! don't make cpoies of selection, or it will get out of sync
}

void Dlg_Physics::ValidPlot(void)
{
	if ( d_data == NULL ) d_data = new CurveData;

	if ( d_curve == NULL ){
		d_curve = new QwtPlotCurve("Total");
		//d_curve->setStyle(QwtPlotCurve::NoCurve);
		//const QColor &c = Qt::white;
		//d_curve->setSymbol(new QwtSymbol(QwtSymbol::XCross, QBrush(c), QPen(c), QSize(5, 5)) );
		d_curve->attach(ui.DataPlot);
	}
}

void Dlg_Physics::AddPlotPoint(double Time)
{
//	if (ui.DataPlot->isVisible()){ //only add points if the plot is visible
		ValidPlot();
		
		//big ugly list here....
			switch (CurPlotType){
			case PL_DISP:
				if (pSim->CurXSel == -1){ //if a voxel is not selected:
					switch (CurPlotDir){
					case MAXDIR: d_data->append(Time, pSim->SS.NormObjDisp); break;
					case XDIR: d_data->append(Time, pSim->SS.TotalObjDisp.x); break;
					case YDIR: d_data->append(Time, pSim->SS.TotalObjDisp.y); break;
					case ZDIR: d_data->append(Time, pSim->SS.TotalObjDisp.z); break;
					}
				}
				else {
					int ThisVox = pSim->CurXSel==-1?0:pSim->CurXSel; //solves weird threading issue
					CVXS_Voxel& CurVox = pSim->VoxArray[pSim->XtoSIndexMap[ThisVox]];
					switch (CurPlotDir){
					case MAXDIR: d_data->append(Time, 0); break; //NOT USED
					case XDIR: d_data->append(Time, CurVox.GetCurPos().x-CurVox.GetOrigPos().x); break;
					case YDIR: d_data->append(Time, CurVox.GetCurPos().y-CurVox.GetOrigPos().y); break;
					case ZDIR: d_data->append(Time, CurVox.GetCurPos().z-CurVox.GetOrigPos().z); break;
					}
				}
				break;
			case PL_KINETIC: 
				d_data->append(Time, pSim->SS.TotalObjKineticE);
				break;
			case PL_STRAINE: 
				d_data->append(Time, pSim->SS.TotalObjStrainE);
				break;
			case PL_TOTALENERGY: 
				d_data->append(Time, pSim->SS.TotalObjKineticE + pSim->SS.TotalObjStrainE);
				break;
			}

		//}
		//else { //single voxel selected
		//	
		//	switch (CurPlotType){
		//	case PL_DISP: 
		//		switch (CurPlotDir){
		//		case MAXDIR: d_data->append(Time, 0); break; //NOT USED
		//		case XDIR: d_data->append(Time, CurVox.GetCurPos().x-CurVox.GetOrigPos().x); break;
		//		case YDIR: d_data->append(Time, CurVox.GetCurPos().y-CurVox.GetOrigPos().y); break;
		//		case ZDIR: d_data->append(Time, CurVox.GetCurPos().z-CurVox.GetOrigPos().z); break;
		//		}
		//		break;
		//	case PL_KINETIC: 
		//		d_data->append(Time, CurVox.GetCurKineticE());
		//		break;
		//	case PL_STRAINE: 
		//		d_data->append(Time, CurVox.GetCurPotentialE());
		//		break;
		//	case PL_TOTALENERGY: 
		//		d_data->append(Time, CurVox.GetCurKineticE() + CurVox.GetCurPotentialE());
		//		break;
		//	}
		//}


//	}
}

void Dlg_Physics::UpdatePlot(void)
{
	if (ui.DataPlot->isVisible()){ //only add points if the plot is visible
		ValidPlot();
		const double* pX = d_data->x();
		const double* pY = d_data->y();
		if (pX == NULL || pY == NULL) return; //allows us to drop gracefully

		double dTStep = pSim->ApproxMSperLog;
		if (dTStep == 0) dTStep = 1;
		d_data->SetMaxToShow(5000/dTStep); //5000.0/30.0);//5 second window..., update every 30ms
		d_curve->setRawSamples(pX, pY, d_data->count());
		double AxisStart = *pX;
		double AxisEnd = *(pX + d_data->count()-1);
		ui.DataPlot->setAxisScale(QwtPlot::xBottom, AxisStart, AxisEnd, (AxisEnd-AxisStart)/2.0);

		ui.DataPlot->replot();
	}
}


void Dlg_Physics::DeletePlotPoints(void)
{
    delete d_curve;
    d_curve = NULL;

    delete d_data;
    d_data = NULL;

	ui.DataPlot->replot();
}



void Dlg_Physics::ClickedReset(void)
{
	pSim->ResetSim(); 
	DeletePlotPoints();
//	pSim->ReqGLUpdate();
	UpdateUI();
}

void Dlg_Physics::ClickedRecord(bool State)
{
	if (State) pSim->BeginRecording();
	else pSim->EndRecording();

	UpdateUI();
}



void Dlg_Physics::ClickedSaveData(void)
{
	if(d_data) d_data->OutputData();
}

void CurveData::OutputData()
{
	QFile data(QFileDialog::getSaveFileName(NULL, "Save trace data", "", "Text files (*.txt)"));
	if (data.open(QFile::WriteOnly | QFile::Truncate)) {
		QTextStream out(&data);
		for (int i=0; i<d_count; i++) {
			out << d_x[i] << "\t" << d_y[i] << "\n";
		}
	}
	data.close();
}
