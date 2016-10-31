/*******************************************************************************
Copyright (c) 2010, Jonathan Hiller
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * The name if its contributors may not be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#ifndef DLG_PALETTE_H
#define DLG_PALETTE_H

#include "ui_vPalette.h"
#include "QVX_Interfaces.h"
#include <QAbstractListModel>
#include "Dlg_StructureEditor.h"

//#include "QVX_Edit.h"
//#include "QOpenGL.h" //tmp!!!

class CPalModel;

class Dlg_Palette : public QWidget
{
	Q_OBJECT


public:
	Dlg_Palette(QVX_Object* pObjIn, QWidget *parent = 0);
	~Dlg_Palette();

	CPalModel* Palette_Model;
	QItemSelectionModel *selectionModel;

	QVX_Object* pObj; //pointer to the digital object this palette editor is looking at...
	CQDM_Edit TmpEdit; //to edit internal structures when needed
	Dlg_StructureEditor* TmpEditDlg;

	int GetCurMat(void){int CurMat; GetCurMat(&CurMat); return CurMat;};
	CVXC_Material* GetPCurMat(void) {return pObj->GetBaseMat(GetCurMat());}; // &(pObj->Palette[GetCurMat()]);};
	void SetCurMat(int NewMatIndex);

	virtual QSize sizeHint () const {return QSize(100, 500);};

signals:
	void RequestUpdateGL(void);


public slots:
	void ChangedVisible(bool State) {GetPCurMat()->SetVisible(State); emit RequestUpdateGL(); UpdateUI();};

	void ChangedBasic(bool State) {if (State) {GetPCurMat()->SetMatType(SINGLE); emit RequestUpdateGL(); UpdateUI();}};
	void ChangedDither(bool State) {if (State) {GetPCurMat()->SetMatType(DITHER); emit RequestUpdateGL(); UpdateUI();}};
	void ChangedStructure(bool State) {if (State) {GetPCurMat()->SetMatType(INTERNAL); emit RequestUpdateGL(); UpdateUI();}};

	
	void ChangedR(int RVal);
	void ChangedG(int GVal);
	void ChangedB(int BVal);
	void ChangedA(int AVal);

	void ChangedStiffness(QString str) {GetPCurMat()->SetElasticMod(str.toFloat()*1000000);};
	void ChangedPoisson(QString str) {GetPCurMat()->SetPoissonsRatio(str.toFloat());};
	void ChangedDens(QString str) {GetPCurMat()->SetDensity(str.toFloat());};
	void ChangedCTE(QString str) {GetPCurMat()->SetCTE(str.toFloat());};
	void ChangeduStatic(QString str) {GetPCurMat()->SetuStatic(str.toFloat());};
	void ChangeduDynamic(QString str) {GetPCurMat()->SetuDynamic(str.toFloat());};

	void Mat1IndexChanged(int NewMatIndex);
	void Mat2IndexChanged(int NewMatIndex);
	void ChangedPercRand(int RVal);

	void ClickedImport(void);
	void ClickedExport(void);

	void ChangedXDim(int XDimVal);
	void ChangedYDim(int YDimVal);
	void ChangedZDim(int ZDimVal);
	void ChangedXOff(int XOffVal);
	void ChangedYOff(int YOffVal);
	void ChangedZOff(int ZOffVal);

	void ClickedEditStructure(void);
	void DoneEditStructure(void);

	void UpdateUI(void) {UpdateVisibleTabs(); UpdateFields();};
	void UpdateFields(void);

	void AddMaterial(void);
	void DeleteCurMaterial(void);

	void LoadPalette(void);
	void SavePalette(void);


	void GetCurMat(int* MatIndex);

private:
	void UpdateVisibleTabs(void);
	bool MatIsEditable(int MatIndex, std::string* ModName = NULL); //check for self reference, recursion...
	Ui::PaletteDlg ui;
};


//THE LINK between qt's model framework and my CVX_Object palette object

class CPalModel : public QAbstractListModel
{
	Q_OBJECT

public:
	CPalModel() {pObj = NULL;};
	CPalModel(CVX_Object* pVXC, QObject *parent = 0) : QAbstractListModel(parent), pObj(pVXC) {} //calls default constructor, sets pObj to pVXC

public slots:
	void UpdateList(void);

public:
	CVX_Object* pObj; //VXC object we are analyzing

	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role) const;

	Qt::ItemFlags flags(const QModelIndex &index) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

	bool insertRows(int row, int count, const QModelIndex & parent = QModelIndex()); 
	bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex()); 
};

#endif // DLG_PALETTE_H
