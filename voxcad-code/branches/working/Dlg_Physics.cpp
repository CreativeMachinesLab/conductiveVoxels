#include "Dlg_Physics.h"
//#include <qwt_plot.h>
#include <qwt_plot_curve.h>
//#include <qwt_plot_directpainter.h>
//#include <qwt_plot_layout.h>


//TEMP
#include "VXS_Voxel.h"

Dlg_Physics::Dlg_Physics(QVX_Sim* pRelaxIn, QWidget *parent)
	: QWidget(parent)
{
	pRelax = pRelaxIn;
	ui.setupUi(this);

	//these should match the order of the PlotType enum
	ui.VariableCombo->addItem("Displacement");
	ui.VariableCombo->addItem("Force");
//	ui.VariableCombo->addItem("Total Force");

	//these should match the order of the PlotDir enum
	ui.DirectionCombo->addItem("Maximum");
	ui.DirectionCombo->addItem("X Direction");
	ui.DirectionCombo->addItem("Y Direction");
	ui.DirectionCombo->addItem("Z Direction");

	CurPlotType = PL_DISP;
	CurPlotDir = MAXDIR;

	connect(ui.PauseButton, SIGNAL(clicked()), this, SLOT(ClickedPause()));
	connect(ui.ResetButton, SIGNAL(clicked()), this, SLOT(ClickedReset()));
	connect(ui.dtSlider, SIGNAL(sliderMoved(int)), this, SLOT(dtSliderChanged(int)));
	connect(ui.BondDampSlider, SIGNAL(sliderMoved(int)), this, SLOT(BondDampSliderChanged(int)));
	connect(ui.GNDDampSlider, SIGNAL(sliderMoved(int)), this, SLOT(GNDDampSliderChanged(int)));
	connect(ui.ColDampSlider, SIGNAL(sliderMoved(int)), this, SLOT(ColDampSliderChanged(int)));
	connect(ui.UseSelfColCheck, SIGNAL(clicked(bool)), this, SLOT(UseSelfColCheckChanged(bool)));


	connect(ui.UseTempCheck, SIGNAL(clicked(bool)), this, SLOT(UseTempCheckChanged(bool)));
	connect(ui.TempSlider, SIGNAL(sliderMoved(int)), this, SLOT(TempSliderChanged(int)));
	connect(ui.VaryTempCheck, SIGNAL(clicked(bool)), this, SLOT(VaryTempCheckChanged(bool)));
	connect(ui.TempPerSlider, SIGNAL(sliderMoved(int)), this, SLOT(TempPerSliderChanged(int)));
	connect(ui.UseGravCheck, SIGNAL(clicked(bool)), this, SLOT(UseGravCheckChanged(bool)));
	connect(ui.GravSlider, SIGNAL(sliderMoved(int)), this, SLOT(GravSliderChanged(int)));

	connect(ui.UseFloorCheck, SIGNAL(clicked(bool)), this, SLOT(UseFloorCheckChanged(bool)));

	connect(ui.MeshCheck, SIGNAL(clicked(bool)), this, SLOT(MeshCheckChanged(bool)));
	connect(ui.VoxelsCheck, SIGNAL(clicked(bool)), this, SLOT(VoxelsCheckChanged(bool)));
	connect(ui.BondsCheck, SIGNAL(clicked(bool)), this, SLOT(BondsCheckChanged(bool)));
	connect(ui.ForcesCheck, SIGNAL(clicked(bool)), this, SLOT(ForcesCheckChanged(bool)));
	connect(ui.LocalCoordCheck, SIGNAL(clicked(bool)), this, SLOT(LCsCheckChanged(bool)));

	connect(ui.DisplacementRadio, SIGNAL(clicked(bool)), this, SLOT(VDispChanged(bool)));
	connect(ui.StrainRadio, SIGNAL(clicked(bool)), this, SLOT(VStrainChanged(bool)));
	connect(ui.StressRadio, SIGNAL(clicked(bool)), this, SLOT(VStressChanged(bool)));
	connect(ui.MaterialRadio, SIGNAL(clicked(bool)), this, SLOT(VMaterialChanged(bool)));
	connect(ui.StateRadio, SIGNAL(clicked(bool)), this, SLOT(VStateChanged(bool)));

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


	UpdateUI();
	

}
/*******************************************************************************
Copyright (c) 2010, Jonathan Hiller
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * The name if its contributors may not be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/


Dlg_Physics::~Dlg_Physics()
{
	PlotUpdateTimer->stop();
}

void Dlg_Physics::UpdateUI(void)
{
	//Sim
	if (!pRelax->Paused) ui.PauseButton->setText("Pause");
	else ui.PauseButton->setText("Start");


	ui.dtSlider->setRange(0, 1000);
	ui.dtSlider->setValue(pRelax->DtFrac*500); 

	ui.BondDampSlider->setRange(0, 100);
	ui.BondDampSlider->setValue(pRelax->GetBondDampZ()*100); 

	//from .00001 to .1
	ui.GNDDampSlider->setRange(0, 100);
	ui.GNDDampSlider->setValue((log10(pRelax->GetSlowDampZ())+5)*25.0); 

	ui.ColDampSlider->setRange(0, 100);
	ui.ColDampSlider->setValue(pRelax->GetCollisionDampZ()*50); 

	ui.UseSelfColCheck->setChecked(pRelax->IsSelfColEnabled());

	//Env
	ui.UseTempCheck->setChecked(pRelax->pEnv->TempEnabled);
	ui.TempSlider->setRange(0, 50);
	ui.TempSlider->setValue(pRelax->pEnv->TempAmp); 

	ui.VaryTempCheck->setChecked(pRelax->pEnv->VaryTempEnabled);
	ui.TempPerSlider->setRange(0, 1000);
	ui.TempPerSlider->setValue(pRelax->pEnv->TempPeriod/pRelax->dt/10.0); 

	ui.UseGravCheck->setChecked(pRelax->pEnv->GravEnabled);
	ui.GravSlider->setRange(0, 100);
	ui.GravSlider->setValue(-pRelax->pEnv->GravAcc/0.981); 

	ui.UseFloorCheck->setChecked(pRelax->pEnv->FloorEnabled);

	//View
	ui.MeshCheck->setChecked(pRelax->ViewGeoMesh);
	ui.VoxelsCheck->setChecked(pRelax->ViewGeo);
	ui.BondsCheck->setChecked(pRelax->ViewBonds);
	ui.ForcesCheck->setChecked(pRelax->ViewForce);
	ui.LocalCoordCheck->setChecked(pRelax->ViewAngles);

	switch (pRelax->CurViewCol){
		case RVC_DISP: ui.DisplacementRadio->setChecked(true); break;
		case RVC_STRAIN: ui.StrainRadio->setChecked(true); break;
		case RVC_STRESS: ui.StressRadio->setChecked(true); break;
		case RVC_MATERIAL: ui.MaterialRadio->setChecked(true); break;
		case RVC_STATE: ui.StateRadio->setChecked(true); break;
	}



	ui.VariableCombo->setCurrentIndex(CurPlotType);
	ui.DirectionCombo->setCurrentIndex(CurPlotDir);

}

void Dlg_Physics::ApplyVoxSelection(int NewSel)
{
	DeletePlotPoints();
	pRelax->CurXSel = NewSel; //TMP!!! don't make cpoies of selection, or it will get out of sync
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
	if (ui.DataPlot->isVisible()){ //only add points if the plot is visible
		ValidPlot();
		
		//big ugly list here....
		if (pRelax->CurXSel == -1){ //if a voxel is not selected:
			switch (CurPlotType){
			case PL_DISP:
				switch (CurPlotDir){
				case MAXDIR: d_data->append(Time, pRelax->NormObjDisp); break;
				case XDIR: d_data->append(Time, pRelax->TotalObjDisp.x); break;
				case YDIR: d_data->append(Time, pRelax->TotalObjDisp.y); break;
				case ZDIR: d_data->append(Time, pRelax->TotalObjDisp.z); break;
				}
				break;
			case PL_FORCE: 
				switch (CurPlotDir){
				case MAXDIR: d_data->append(Time, pRelax->NormObjForce); break;
				case XDIR: d_data->append(Time, pRelax->TotalObjForce.x); break;
				case YDIR: d_data->append(Time, pRelax->TotalObjForce.y); break;
				case ZDIR: d_data->append(Time, pRelax->TotalObjForce.z); break;
				}
				break;
			}
		}
		else { //single voxel selected
			CVXS_Voxel& CurVox = pRelax->VoxArray[pRelax->XtoSIndexMap[pRelax->CurXSel]];
			switch (CurPlotType){
			case PL_DISP: 
				switch (CurPlotDir){
				case MAXDIR: d_data->append(Time, 0); break; //NOT USED
				case XDIR: d_data->append(Time, CurVox.S.Pos.x-CurVox.NomPos.x); break;
				case YDIR: d_data->append(Time, CurVox.S.Pos.y-CurVox.NomPos.y); break;
				case ZDIR: d_data->append(Time, CurVox.S.Pos.z-CurVox.NomPos.z); break;
				}
				break;
			case PL_FORCE: 
				switch (CurPlotDir){
				case MAXDIR: d_data->append(Time, 0); break; //NOT USED
				case XDIR: d_data->append(Time, CurVox.dS.Force.x); break;
				case YDIR: d_data->append(Time, CurVox.dS.Force.y); break;
				case ZDIR: d_data->append(Time, CurVox.dS.Force.z); break;
				}
				break;
			}
		}


	}
}

void Dlg_Physics::UpdatePlot(void)
{
	if (ui.DataPlot->isVisible()){ //only add points if the plot is visible
		ValidPlot();
		const double* pX = d_data->x();
		const double* pY = d_data->y();
		if (pX == NULL || pY == NULL) return; //allows us to drop gracefully

		double dTStep = pRelax->ApproxMSperLog;
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
	pRelax->ResetSim(); 
	DeletePlotPoints();
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
