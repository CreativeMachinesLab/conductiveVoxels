/*******************************************************************************
Copyright (c) 2010, Jonathan Hiller
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * The name if its contributors may not be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#ifndef DLG_PHYSICS_H
#define DLG_PHYSICS_H

#include <QWidget>
#include <QTimer>
#include "ui_vPhysics.h"
#include "QVX_Interfaces.h"

#include <qwt_plot.h>
class QwtPlotCurve;
//class QwtPlotDirectPainter;

class CurveData;

class Dlg_Physics : public QWidget
{
	Q_OBJECT

public:
	Dlg_Physics(QVX_Sim* pRelaxIn, QWidget *parent = 0);
	~Dlg_Physics();

	QVX_Sim* pRelax;

	CurveData *d_data;
	QwtPlotCurve *d_curve;
//	QwtPlotDirectPainter *d_directPainter;

	enum PlotType {PL_DISP, PL_FORCE};
	enum PlotDir {MAXDIR, XDIR, YDIR, ZDIR};

	PlotType CurPlotType;
	PlotDir CurPlotDir;


public slots:
	void SetStatusText(QString Text) {ui.OutText->setText(Text);};
	void ApplyVoxSelection(int NewSel);
	void AddPlotPoint(double Time);
	void DeletePlotPoints(void);

	void UpdateUI(void);
	void UpdatePlot(void);

	//UI stuff:
	void dtSliderChanged(int NewVal) {pRelax->DtFrac = NewVal/500.0;};
	void BondDampSliderChanged(int NewVal) {pRelax->SetBondDampZ(NewVal/100.0);} //range 0 to 2
	void GNDDampSliderChanged(int NewVal) {pRelax->SetSlowDampZ(pow(10, NewVal/25.0-5));} //range 0 to 2
	void ColDampSliderChanged(int NewVal) {pRelax->SetCollisionDampZ(NewVal/50.0);} //range 0 to 2

	void UseSelfColCheckChanged(bool State) {pRelax->EnableSelfCollision(State);}

	void UseTempCheckChanged(bool State) {pRelax->pEnv->TempEnabled = State;};
	void TempSliderChanged(int NewVal) {pRelax->pEnv->TempAmp = NewVal;}; //range 0 to 50

	void VaryTempCheckChanged(bool State) {pRelax->pEnv->VaryTempEnabled = State;};
	void TempPerSliderChanged(int NewVal) {pRelax->pEnv->TempPeriod = NewVal*10.0*pRelax->dt;}; //range 0 to 50

	void UseGravCheckChanged(bool State) {pRelax->pEnv->GravEnabled = State;};
	void GravSliderChanged(int NewVal) {pRelax->pEnv->GravAcc = -0.981*NewVal;}; //range 0 to 10g

	void UseFloorCheckChanged(bool State) {pRelax->pEnv->FloorEnabled = State;};

	void MeshCheckChanged(bool State) {pRelax->ViewGeoMesh = State; if (State){pRelax->ViewGeo = false; ui.VoxelsCheck->setChecked(false);}}
	void VoxelsCheckChanged(bool State) {pRelax->ViewGeo = State; if (State){pRelax->ViewGeoMesh = false; ui.MeshCheck->setChecked(false);}}
	void BondsCheckChanged(bool State) {pRelax->ViewBonds = State;}
	void ForcesCheckChanged(bool State) {pRelax->ViewForce = State;}
	void LCsCheckChanged(bool State) {pRelax->ViewAngles = State;}

	void VDispChanged(bool State) {if (State) pRelax->CurViewCol = RVC_DISP;}
	void VStrainChanged(bool State) {if (State) pRelax->CurViewCol = RVC_STRAIN;}
	void VStressChanged(bool State) {if (State) pRelax->CurViewCol = RVC_STRESS;}
	void VMaterialChanged(bool State) {if (State) pRelax->CurViewCol = RVC_MATERIAL;}
	void VStateChanged(bool State) {if (State) pRelax->CurViewCol = RVC_STATE;}




	void VarComboChanged(int NewIndex) {DeletePlotPoints(); CurPlotType = (PlotType)NewIndex;}
	void DirComboChanged(int NewIndex) {DeletePlotPoints(); CurPlotDir = (PlotDir)NewIndex;}
	void LogEachCheckChanged(bool NewVal) {pRelax->LogEvery = NewVal;}

	void ClickedPause(void) {pRelax->PauseSim(); UpdateUI();};
	void ClickedReset(void);

	void ClickedSaveData(void);

protected:
	QTimer* PlotUpdateTimer;
	double PlotUpdateRate; //in ms

	void ValidPlot(void);

private:
	Ui::PhysicsDialog ui;
};


class CurveData // A container class for growing data
{
public:
	CurveData(void) {d_count = 0; MaxToShow = 200;};
	void append(double x, double y) {d_x.push_back(x); d_y.push_back(y); d_count++;};
	void SetMaxToShow(int NewVal) {MaxToShow = NewVal;};
	void OutputData();

	int count() const {if (d_count < MaxToShow) return d_count; else return MaxToShow;};
	int size() const {return d_x.size();};
	const double *x() const {if (d_x.size() == 0) return NULL; if (d_count < MaxToShow) return &d_x[0]; else return &d_x[d_count-MaxToShow];};
	const double *y() const {if (d_y.size() == 0) return NULL; if (d_count < MaxToShow) return &(d_y[0]); else return &(d_y[d_count-MaxToShow]);};

private:
    int d_count, MaxToShow;
	std::vector<double> d_x;
    std::vector<double> d_y;
};

#endif // DLG_PHYSICS_H
