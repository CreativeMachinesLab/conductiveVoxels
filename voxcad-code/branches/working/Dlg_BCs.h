/*******************************************************************************
Copyright (c) 2010, Jonathan Hiller
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * The name if its contributors may not be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#ifndef DLG_BCS_H
#define DLG_BCS_H

#include "ui_vBCs.h"
#include "QVX_Interfaces.h"
#include <QWidget>

class Dlg_BCs : public QWidget
{
	Q_OBJECT

public:
	Dlg_BCs(QVX_Environment* pEnvIn, QWidget *parent = 0);
	~Dlg_BCs();

	QVX_Environment* pEnv;
	enum BCPresetType {BC_NONE, BC_XCANT, BC_YCANT, BC_XAXIAL, BC_YAXIAL, BC_ZAXIAL};

	virtual QSize sizeHint () const {return QSize(100, 500);};

//	std::vector <CVX_FRegion> PercBCs; //holds the percentage values for the BC (independant of LatDim and Workspace size)

signals:
	void RequestUpdateGL(void);
	void RequestGLSelect(int NewGLIndex);
	void DoneEditing(void); //emitted to exit the boundary condition editting mode

public slots:
	void AddBC(void);
	void DelCurBC(void);
	void SaveBCs(void);
	void LoadBCs(void);
	void BCrowChanged(int NewRow);
	void ApplyExtSelection(int NewGLIndex); //called when we select a BC external to the window (IE GL picking)

	void ChangedFixed(bool State);
	void ChangedForced(bool State);

	void ChangedXForce(QString NewText);
	void ChangedYForce(QString NewText);
	void ChangedZForce(QString NewText);

	void ApplyPreset(int NewPreset);

	void ChangedSnap(int NewState) {if(NewState == Qt::Checked) Snap = true; else Snap = false;};
	void ChangedX(int NewVal) {ui.XSpin->setValue(NewVal/100.0);};
	void ChangedX(double NewVal);
	void ChangedY(int NewVal) {ui.YSpin->setValue(NewVal/100.0);};
	void ChangedY(double NewVal);
	void ChangedZ(int NewVal) {ui.ZSpin->setValue(NewVal/100.0);};
	void ChangedZ(double NewVal);

	void ChangedDX(int NewVal) {ui.DXSpin->setValue(NewVal/100.0);};
	void ChangedDX(double NewVal);
	void ChangedDY(int NewVal) {ui.DYSpin->setValue(NewVal/100.0);};
	void ChangedDY(double NewVal);
	void ChangedDZ(int NewVal) {ui.DZSpin->setValue(NewVal/100.0);};
	void ChangedDZ(double NewVal);

	void DoneButtonClicked(void){emit DoneEditing();};

	void UpdateBCLists(void);

private:
	Ui::BCDlg ui;

	void UpdateUI(void);

	bool Snap;
	int CurRow; //currently selected row number
	CVX_FRegion* CurRegion; //currently selected boundary condition

	int CombToFixedIndex(int CombInd) {if (CombInd >= 0 && CombInd < pEnv->GetNumFixed()) return CombInd; else return -1;}; //returns the index of the fixed array from the master index
	int CombToForcedIndex(int CombInd) {if (CombInd >= pEnv->GetNumFixed() && CombInd < pEnv->GetNumFixed()+pEnv->GetNumForced()) return CombInd-pEnv->GetNumFixed(); else return -1;}; //returns the index of the forced array from the master index
	int FixedToCombIndex(int FixedInd) {return FixedInd;};
	int ForcedToCombIndex(int ForcedInd) {return pEnv->GetNumFixed() + ForcedInd;};
	int CombToGLIndex(int CombInd) {
		int tmpInd = CombToFixedIndex(CombInd); 
		if (tmpInd != -1) return BCFIXED_GLIND_OFF+tmpInd;
		tmpInd = CombToForcedIndex(CombInd); 
		if (tmpInd != -1) return BCFORCE_GLIND_OFF+tmpInd;
		return -1;
	};
	int GLToCombIndex(int GLIndex){
		int tmpInd = GLIndex-BCFIXED_GLIND_OFF;
		if (tmpInd>=0 && tmpInd<pEnv->GetNumFixed()) return FixedToCombIndex(tmpInd);
		tmpInd = GLIndex-BCFORCE_GLIND_OFF;
		if (tmpInd>=0 && tmpInd<pEnv->GetNumForced()) return ForcedToCombIndex(tmpInd);
		return -1;
	}

//	int GLToForcedIndex(int CombInd) {int tmpInd = CombToForcedIndex(CombInd); if (tmpInd == -1) return -1; else return BCFORCE_GLIND_OFF+tmpInd;};
	

	double GetSnapped(double Dim, double Intvl){return Intvl * (((int)(Dim/Intvl+0.5)));} //returns
};

////THE LINK between qt's model framework and my CVX_Object palette object
//
//class CBCModel : public QAbstractListModel
//{
//	Q_OBJECT
//
//public:
//	CBCModel() {pObj = NULL;};
//	CBCModel(CVX_FEA* pFEAIn, QObject *parent = 0) : QAbstractListModel(parent), pEnv(pFEAIn) {} //calls default constructor, sets pObj to pVXC
//
//public slots:
//	void UpdateList(void);
//
//public:
//	CVX_FEA* pEnv; //VXC object we are analyzing
//
//	int rowCount(const QModelIndex &parent = QModelIndex()) const;
//	QVariant data(const QModelIndex &index, int role) const;
//
//	Qt::ItemFlags flags(const QModelIndex &index) const;
//	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
//
//	bool insertRows(int row, int count, const QModelIndex & parent = QModelIndex()); 
//	bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex()); 
//};
//
#endif // DLG_BCS_H
