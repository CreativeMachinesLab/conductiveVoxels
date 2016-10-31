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
	Dlg_Physics(QVX_Sim* pSimIn, QWidget *parent = 0);
	~Dlg_Physics();

	QVX_Sim* pSim;

	CurveData *d_data;
	QwtPlotCurve *d_curve;
//	QwtPlotDirectPainter *d_directPainter;

	enum PlotType {PL_DISP, PL_KINETIC, PL_STRAINE, PL_TOTALENERGY};
	enum PlotDir {MAXDIR, XDIR, YDIR, ZDIR};

	PlotType CurPlotType;
	PlotDir CurPlotDir;

	void SetStatusButtonText(bool IsSimRunning) {if (IsSimRunning) ui.PauseButton->setText("Pause"); else ui.PauseButton->setText("Start");}
	double TimestepMag; //approx size of timestep (shouldn't change much... for scaling sliders


public slots:
	void SetStatusText(QString Text) {ui.OutText->setText(Text);};
	void ApplyVoxSelection(int NewSel);
	void AddPlotPoint(double Time);
	void DeletePlotPoints(void);

	void UpdateUI(void);
	void UpdatePlot(void);

	void UseEquilibriumCheckChanged(bool State) {pSim->EnableEquilibriumMode(State); UpdateUI();}
	void StopSelectChanged(int NewIndex) {pSim->SetStopConditionType((StopCondition)NewIndex); UpdateUI();}
	void StopValueEditChanged(void) {double Val = ui.StopValueEdit->text().toDouble(); Val=Val<0?0:Val; pSim->SetStopConditionValue(Val); UpdateUI();}

	//UI stuff:
	void dtSliderChanged(int NewVal) {pSim->DtFrac = NewVal/500.0; UpdateUI();}
	void dtEditChanged(void) {double Val = ui.dtEdit->text().toDouble(); Val=Val<0?0:Val; pSim->DtFrac=Val; UpdateUI();}
	void BondDampSliderChanged(int NewVal) {pSim->SetBondDampZ(NewVal/50.0); UpdateUI();} //range 0 to 2
	void BondDampEditChanged(void) {double Val = ui.BondDampEdit->text().toDouble(); Val=Val<0?0:Val; Val=Val>1?1:Val; pSim->SetBondDampZ(Val); UpdateUI();}
	void GNDDampSliderChanged(int NewVal) {pSim->SetSlowDampZ(NewVal==0 ? 0:pow(10, NewVal/25.0-5)); UpdateUI();} //range 0 to 2
	void GNDDampEditChanged(void) {double Val = ui.GNDDampEdit->text().toDouble(); Val=Val<0?0:Val; pSim->SetSlowDampZ(Val); UpdateUI();}
	void ColDampSliderChanged(int NewVal) {pSim->SetCollisionDampZ(NewVal/50.0); UpdateUI();} //range 0 to 2
	void ColDampEditChanged(void) {double Val = ui.ColDampEdit->text().toDouble(); Val=Val<0?0:Val; pSim->SetCollisionDampZ(Val); UpdateUI();}
	void MaxVelLimitSliderChanged(int NewVal) {pSim->SetMaxVoxVelLimit(NewVal/400.0);} //range 0 to .25

	void UseSelfColCheckChanged(bool State) {pSim->EnableSelfCollision(State); UpdateUI();}
	void UseMaxVelLimitCheckChanged(bool State) {pSim->EnableMaxVelLimit(State); UpdateUI();}

	void UseTempCheckChanged(bool State) {pSim->pEnv->EnableTemp(State);};
	void TempSliderChanged(int NewVal) {pSim->pEnv->SetTempAmplitude(NewVal-25); UpdateUI();} //range +/-25 from BaseTemp
	void TempEditChanged() {double Val = ui.TempEdit->text().toDouble(); pSim->pEnv->SetTempAmplitude(Val-pSim->pEnv->GetTempBase()); UpdateUI();}

	void VaryTempCheckChanged(bool State) {pSim->pEnv->EnableTempVary(State);};
	void TempPerSliderChanged(int NewVal) {pSim->pEnv->SetTempPeriod(NewVal*pSim->OptimalDt); UpdateUI();}; //range 0 to 50
	void TempPerEditChanged() {double Val = ui.TempPerEdit->text().toDouble(); pSim->pEnv->SetTempPeriod(Val); UpdateUI();}

	void UseGravCheckChanged(bool State) {pSim->pEnv->EnableGravity(State);}
	void GravSliderChanged(int NewVal) {pSim->pEnv->SetGravityAccel(-0.00981*NewVal); UpdateUI();} //range 0 to 10g
	void GravEditChanged() {double Val = ui.GravEdit->text().toDouble(); pSim->pEnv->SetGravityAccel(Val); UpdateUI();}

	void UseFloorCheckChanged(bool State) {pSim->pEnv->EnableFloor(State);}

	void DisplayDisableChanged(bool State) {if (State) pSim->SetCurViewMode(RVM_NONE); UpdateUI();}
	void DisplayVoxChanged(bool State) {if (State) pSim->SetCurViewMode(RVM_VOXELS); UpdateUI();}
	void DisplayConChanged(bool State) {if (State) pSim->SetCurViewMode(RVM_BONDS); UpdateUI();}

	void VoxDiscreteChanged(bool State) {if (State) pSim->SetCurViewVox(RVV_DISCRETE);}
	void VoxDeformedChanged(bool State) {if (State) pSim->SetCurViewVox(RVV_DEFORMED);}
	void VoxSmoothChanged(bool State) {if (State) pSim->SetCurViewVox(RVV_SMOOTH);}

//	void MeshCheckChanged(bool State) {pSim->ViewGeoMesh = State; if (State){pSim->ViewSurfMesh = false; ui.SurfMeshCheck->setChecked(false); pSim->ViewGeo = false; ui.VoxelsCheck->setChecked(false);}}
//	void SurfMeshCheckChanged(bool State) {if(pSim->SurfMesh.DefMesh.Exists()) {pSim->ViewSurfMesh = State; if (State){pSim->ViewGeo = false; ui.VoxelsCheck->setChecked(false); pSim->ViewGeoMesh = false; ui.MeshCheck->setChecked(false);}} else {ui.MeshCheck->setChecked(true); MeshCheckChanged(true);}}
//	void VoxelsCheckChanged(bool State) {pSim->ViewGeo = State; if (State){pSim->ViewSurfMesh = false; ui.SurfMeshCheck->setChecked(false); pSim->ViewGeoMesh = false; ui.MeshCheck->setChecked(false);}}
//	void BondsCheckChanged(bool State) {pSim->ViewBonds = State;}

	void ForcesCheckChanged(bool State) {pSim->ViewForce = State;}
	void LCsCheckChanged(bool State) {pSim->ViewAngles = State;}

	void CTypeChanged(bool State) {if (State) pSim->SetCurViewCol(RVC_TYPE);}
	void CKinEChanged(bool State) {if (State) pSim->SetCurViewCol(RVC_KINETIC_EN);}
	void CDispChanged(bool State) {if (State) pSim->SetCurViewCol(RVC_DISP);}
	void CStateChanged(bool State) {if (State) pSim->SetCurViewCol(RVC_STATE);}
	void CStrainEChanged(bool State) {if (State) pSim->SetCurViewCol(RVC_STRAIN_EN);}
	void CStrainChanged(bool State) {if (State) pSim->SetCurViewCol(RVC_STRAIN);}
	void CStressChanged(bool State) {if (State) pSim->SetCurViewCol(RVC_STRESS);}

	void CmMCheckChanged(bool State){pSim->LockCoMToCenter = State;}
//	void BbCheckChanged(bool State){pSim->HideBoundingBox = State;}

	void VarComboChanged(int NewIndex) {DeletePlotPoints(); CurPlotType = (PlotType)NewIndex;}
	void DirComboChanged(int NewIndex) {DeletePlotPoints(); CurPlotDir = (PlotDir)NewIndex;}
	void LogEachCheckChanged(bool NewVal) {pSim->LogEvery = NewVal;}

	void ClickedPause(void) {if (!pSim->Running){ pSim->RequestBeginSim(); pSim->Running = true;} else pSim->SimPauseToggle(); UpdateUI();};
	void ClickedReset(void);
	void ClickedRecord(bool State);
//	void SimEndedInternally(QString) {UpdateUI();};

	void ClickedSaveData(void);
	void IsOutputVisible(bool* pVisible) {*pVisible =  ui.OutText->isVisible();}
	void IsPlotVisible(bool* pVisible) {*pVisible =  ui.DataPlot->isVisible();}

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
