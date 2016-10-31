/*******************************************************************************
Copyright (c) 2010, Jonathan Hiller
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * The name if its contributors may not be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#include "Dlg_BCs.h"

Dlg_BCs::Dlg_BCs(QVX_Environment* pEnvIn, QWidget *parent)
	: QWidget(parent)
{
	CurRegion = NULL;
	CurRow = -1;
	Snap = true;

	pEnv = pEnvIn;
	ui.setupUi(this);

	ui.BCPresetsCombo->addItem("None"); //should be in same order as BCPresetType enum
	ui.BCPresetsCombo->addItem("X Cantilever"); //should be in same order as BCPresetType enum
	ui.BCPresetsCombo->addItem("Y Cantilever");
	ui.BCPresetsCombo->addItem("X Axial");
	ui.BCPresetsCombo->addItem("Y Axial");
	ui.BCPresetsCombo->addItem("Z Axial");

	ui.SnapCheckBox->setChecked(Snap);

	const QValidator* DEval = new QDoubleValidator(this);
	ui.XForceEdit->setValidator(DEval);
	ui.YForceEdit->setValidator(DEval);
	ui.ZForceEdit->setValidator(DEval);

	ApplyPreset(BC_XCANT);

	connect(ui.BCPresetsCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(ApplyPreset(int)));

	//add or delete boundary conditions
	connect(ui.AddBCButton, SIGNAL(clicked(bool)), this, SLOT(AddBC()));
	connect(ui.DelBCButton, SIGNAL(clicked(bool)), this, SLOT(DelCurBC()));

	//Load or Save Boundary Conditions
	connect(ui.LoadBCButton, SIGNAL(clicked()), this, SLOT(LoadBCs()));
	connect(ui.SaveBCButton, SIGNAL(clicked()), this, SLOT(SaveBCs()));

	connect(ui.SnapCheckBox, SIGNAL(stateChanged(int)), this, SLOT(ChangedSnap(int)));

	connect(ui.ModeFixed, SIGNAL(toggled(bool)), this, SLOT(ChangedFixed(bool)));
	connect(ui.ModeForced, SIGNAL(toggled(bool)), this, SLOT(ChangedForced(bool)));

	connect(ui.XForceEdit, SIGNAL(textChanged(QString)), this, SLOT(ChangedXForce(QString)));
	connect(ui.YForceEdit, SIGNAL(textChanged(QString)), this, SLOT(ChangedYForce(QString)));
	connect(ui.ZForceEdit, SIGNAL(textChanged(QString)), this, SLOT(ChangedZForce(QString)));

	connect(ui.XSlider, SIGNAL(valueChanged(int)), this, SLOT(ChangedX(int)));
	connect(ui.XSpin, SIGNAL(valueChanged(double)), this, SLOT(ChangedX(double)));
	connect(ui.YSlider, SIGNAL(valueChanged(int)), this, SLOT(ChangedY(int)));
	connect(ui.YSpin, SIGNAL(valueChanged(double)), this, SLOT(ChangedY(double)));
	connect(ui.ZSlider, SIGNAL(valueChanged(int)), this, SLOT(ChangedZ(int)));
	connect(ui.ZSpin, SIGNAL(valueChanged(double)), this, SLOT(ChangedZ(double)));

	connect(ui.DXSlider, SIGNAL(valueChanged(int)), this, SLOT(ChangedDX(int)));
	connect(ui.DXSpin, SIGNAL(valueChanged(double)), this, SLOT(ChangedDX(double)));
	connect(ui.DYSlider, SIGNAL(valueChanged(int)), this, SLOT(ChangedDY(int)));
	connect(ui.DYSpin, SIGNAL(valueChanged(double)), this, SLOT(ChangedDY(double)));
	connect(ui.DZSlider, SIGNAL(valueChanged(int)), this, SLOT(ChangedDZ(int)));
	connect(ui.DZSpin, SIGNAL(valueChanged(double)), this, SLOT(ChangedDZ(double)));

	connect(ui.BCList, SIGNAL(currentRowChanged (int)), this, SLOT(BCrowChanged(int)));
	connect(ui.BCDonePushButton, SIGNAL(clicked()), this, SLOT(DoneButtonClicked())); 

}

Dlg_BCs::~Dlg_BCs()
{

}

void Dlg_BCs::UpdateUI(void)
{
	ui.SnapCheckBox->setChecked(Snap);

	if (!CurRegion){ //if nothing selected:
		ui.ModeFixed->setEnabled(false);
		ui.ModeForced->setEnabled(false);
		ui.XForceEdit->setText("");
		ui.XForceEdit->setEnabled(false);
		ui.YForceEdit->setText("");
		ui.YForceEdit->setEnabled(false);
		ui.ZForceEdit->setText("");
		ui.ZForceEdit->setEnabled(false);
		ui.XSpin->setValue(0);
		ui.XSpin->setEnabled(false);
		ui.XSlider->setEnabled(false);
		ui.YSpin->setValue(0);
		ui.YSpin->setEnabled(false);
		ui.YSlider->setEnabled(false);
		ui.ZSpin->setValue(0);
		ui.ZSpin->setEnabled(false);
		ui.ZSlider->setEnabled(false);
		ui.DXSpin->setValue(0);
		ui.DXSpin->setEnabled(false);
		ui.DXSlider->setEnabled(false);
		ui.DYSpin->setValue(0);
		ui.DYSpin->setEnabled(false);
		ui.DYSlider->setEnabled(false);
		ui.DZSpin->setValue(0);
		ui.DZSpin->setEnabled(false);
		ui.DZSlider->setEnabled(false);
		
	}
	else { //if something selected:
		ui.ModeFixed->setEnabled(true);
		ui.ModeForced->setEnabled(true);

		if (CurRegion->Fixed){ //if this region is fixed:
			ui.ModeFixed->setChecked(true);
			ui.XForceEdit->setText("");
			ui.XForceEdit->setEnabled(false);
			ui.YForceEdit->setText("");
			ui.YForceEdit->setEnabled(false);
			ui.ZForceEdit->setText("");
			ui.ZForceEdit->setEnabled(false);
		}
		else { //if this region is not fixed:
			ui.ModeForced->setChecked(true);
			ui.XForceEdit->setText(QString::number(CurRegion->Force.x, 'g', 3));
			ui.XForceEdit->setEnabled(true);
			ui.YForceEdit->setText(QString::number(CurRegion->Force.y, 'g', 3));
			ui.YForceEdit->setEnabled(true);
			ui.ZForceEdit->setText(QString::number(CurRegion->Force.z, 'g', 3));
			ui.ZForceEdit->setEnabled(true);
		}

		ui.XSpin->setValue(CurRegion->pRegion->X);
		ui.XSpin->setEnabled(true);
		ui.XSlider->setEnabled(true);
		ui.YSpin->setValue(CurRegion->pRegion->Y);
		ui.YSpin->setEnabled(true);
		ui.YSlider->setEnabled(true);
		ui.ZSpin->setValue(CurRegion->pRegion->Z);
		ui.ZSpin->setEnabled(true);
		ui.ZSlider->setEnabled(true);
		ui.DXSpin->setValue(CurRegion->pRegion->dX);
		ui.DXSpin->setEnabled(true);
		ui.DXSlider->setEnabled(true);
		ui.DYSpin->setValue(CurRegion->pRegion->dY);
		ui.DYSpin->setEnabled(true);
		ui.DYSlider->setEnabled(true);
		ui.DZSpin->setValue(CurRegion->pRegion->dZ);
		ui.DZSpin->setEnabled(true);
		ui.DZSlider->setEnabled(true);
	}
}

void Dlg_BCs::BCrowChanged(int NewRow) //changes current state variables and updates the window according to the selection
{
	CurRow = NewRow;

	if (CombToFixedIndex(NewRow) != -1){ //if this is fixed...
		CurRegion = pEnv->GetFixedRegion(CombToFixedIndex(NewRow));
	}
	else if (CombToForcedIndex(NewRow) != -1){
		CurRegion = pEnv->GetForcedRegion(CombToForcedIndex(NewRow));
	}
	else CurRegion = NULL;

	emit RequestGLSelect(CombToGLIndex(NewRow));
	UpdateUI();
}

void Dlg_BCs::ApplyExtSelection(int NewGLIndex) //called when we select a BC external to the window (IE GL picking)
{
	if (GLToCombIndex(NewGLIndex) != CurRow)
		ui.BCList->setCurrentRow(GLToCombIndex(NewGLIndex));
}


void Dlg_BCs::UpdateBCLists(void)
{
	int PrevCurRow = CurRow;
	ui.BCList->clear();
	for (int i=0; i<pEnv->GetNumFixed(); i++){
		new QListWidgetItem("Fixed" + QString::number(i), ui.BCList);
	}
	for (int i=0; i<pEnv->GetNumForced(); i++){
		new QListWidgetItem("Forced" + QString::number(i), ui.BCList);
	}
//	if (CurRow >= ui.BCList->count()) CurRow = ui.BCList->count()-1; //make sure 
	ui.BCList->setCurrentRow(PrevCurRow);

	UpdateUI();
}

void Dlg_BCs::ApplyPreset(int NewPreset)
{
	pEnv->ClearBCs();
	ChangedSnap(false);

	switch (NewPreset){
	case BC_XCANT:
		pEnv->AddFixedRegion(Vec3D(0,0,0), Vec3D(0.01, 1.0, 1.0));
		pEnv->AddForcedRegion(Vec3D(0.99,0,0), Vec3D(0.01, 1.0, 1.0), Vec3D(0,0,-1.0));
		break;
	case BC_YCANT:
		pEnv->AddFixedRegion(Vec3D(0,0,0), Vec3D(1.0, 0.01, 1.0));
		pEnv->AddForcedRegion(Vec3D(0,0.99,0), Vec3D(1.0, 0.01, 1.0), Vec3D(0,0,-1.0));
		break;
	case BC_XAXIAL:
		pEnv->AddFixedRegion(Vec3D(0,0,0), Vec3D(0.01, 1.0, 1.0));
		pEnv->AddForcedRegion(Vec3D(0.99,0,0), Vec3D(0.01, 1.0, 1.0), Vec3D(1.0,0,0));
		break;
	case BC_YAXIAL:
		pEnv->AddFixedRegion(Vec3D(0,0,0), Vec3D(1.0, 0.01, 1.0));
		pEnv->AddForcedRegion(Vec3D(0,0.99,0), Vec3D(1.0, 0.01, 1.0), Vec3D(0,1.0,0));
		break;
	case BC_ZAXIAL:
		pEnv->AddFixedRegion(Vec3D(0,0,0), Vec3D(1.0, 1.0, 0.01));
		pEnv->AddForcedRegion(Vec3D(0,0,0.99), Vec3D(1.0, 1.0, 0.01), Vec3D(0,0,1.0));
		break;

	}

	ui.BCList->setCurrentRow(0); //set the current elected to this one
	BCrowChanged(0); //need to call this explicitly sometimes, if new index happens to be the same as the old one...

	UpdateBCLists();
	emit RequestUpdateGL();
}

void Dlg_BCs::AddBC(void)
{
	pEnv->AddFixedRegion(Vec3D(0,0,0), Vec3D(0.1, 0.1, 0.1));
	int MyNewIndex = pEnv->GetNumFixed()-1;
	ui.BCList->setCurrentRow(MyNewIndex); //set the current elected to this one
	BCrowChanged(MyNewIndex); //need to call this explicitly sometimes, if new index happens to be the same as the old one...

	UpdateBCLists();
	emit RequestUpdateGL();
}

void Dlg_BCs::DelCurBC(void)
{
	if (CombToFixedIndex(CurRow) != -1) pEnv->DelFixedRegion(CombToFixedIndex(CurRow));
	else if (CombToForcedIndex(CurRow) != -1) pEnv->DelForcedRegion(CombToForcedIndex(CurRow));

	UpdateBCLists();
	emit RequestUpdateGL();
}

void Dlg_BCs::SaveBCs(void)
{
	pEnv->SaveBCs();
}

void Dlg_BCs::LoadBCs(void)
{
	pEnv->OpenBCs();

	ui.BCList->setCurrentRow(0); //set the current elected to this one
	BCrowChanged(0); //need to call this explicitly sometimes, if new index happens to be the same as the old one...

	UpdateBCLists();
	emit RequestUpdateGL();
}

void Dlg_BCs::ChangedX(double NewVal)
{
	ui.XSlider->setValue(NewVal*100);
	if (CurRegion){
		if (NewVal + CurRegion->pRegion->dX <= 1.0) CurRegion->pRegion->X = NewVal; //if we're not pushing against the far boundary
		else ui.XSpin->setValue(1.0-CurRegion->pRegion->dX); //if this BC would be outside the workspace, keep it within
		if (Snap) CurRegion->pRegion->X = GetSnapped(CurRegion->pRegion->X, 1.0/pEnv->pObj->GetVXDim());
	}
	emit RequestUpdateGL();
}

void Dlg_BCs::ChangedY(double NewVal)
{
	ui.YSlider->setValue(NewVal*100);
	if (CurRegion){
		if (NewVal + CurRegion->pRegion->dY <= 1.0) CurRegion->pRegion->Y = NewVal; //if we're not pushing against the far boundary
		else ui.YSpin->setValue(1.0-CurRegion->pRegion->dY); //if this BC would be outside the workspace, keep it within
		if (Snap) CurRegion->pRegion->Y = GetSnapped(CurRegion->pRegion->Y, 1.0/pEnv->pObj->GetVYDim());
	}
	emit RequestUpdateGL();
}

void Dlg_BCs::ChangedZ(double NewVal)
{
	ui.ZSlider->setValue(NewVal*100);
	if (CurRegion){
		if (NewVal + CurRegion->pRegion->dZ <= 1.0) CurRegion->pRegion->Z = NewVal; //if we're not pushing against the far boundary
		else ui.ZSpin->setValue(1.0-CurRegion->pRegion->dZ); //if this BC would be outside the workspace, keep it within
		if (Snap) CurRegion->pRegion->Z = GetSnapped(CurRegion->pRegion->Z, 1.0/pEnv->pObj->GetVZDim());
	}
	emit RequestUpdateGL();
}

void Dlg_BCs::ChangedDX(double NewVal)
{
	ui.DXSlider->setValue(NewVal*100); //keep the slider buddy-buddy
	if (CurRegion){ //if we've got a current region
		CurRegion->pRegion->dX = NewVal; //update the region
		if ((NewVal + CurRegion->pRegion->X > 1.0)) { ui.XSpin->setValue(1.0-NewVal);} //if increasing the size pushes out the end, push location back
		if (Snap) CurRegion->pRegion->dX = GetSnapped(CurRegion->pRegion->dX, 1.0/pEnv->pObj->GetVXDim()); //if snapping to voxel is enabled
	}
	emit RequestUpdateGL();
}

void Dlg_BCs::ChangedDY(double NewVal)
{
	ui.DYSlider->setValue(NewVal*100); //keep the slider buddy-buddy
	if (CurRegion){ //if we've got a current region
		CurRegion->pRegion->dY = NewVal; //update the region
		if ((NewVal + CurRegion->pRegion->Y > 1.0)) { ui.YSpin->setValue(1.0-NewVal);} //if increasing the size pushes out the end, push location back
		if (Snap) CurRegion->pRegion->dY = GetSnapped(CurRegion->pRegion->dY, 1.0/pEnv->pObj->GetVYDim()); //if snapping to voxel is enabled
	}
	emit RequestUpdateGL();
}

void Dlg_BCs::ChangedDZ(double NewVal)
{
	ui.DZSlider->setValue(NewVal*100); //keep the slider buddy-buddy
	if (CurRegion){ //if we've got a current region
		CurRegion->pRegion->dZ = NewVal; //update the region
		if ((NewVal + CurRegion->pRegion->Z > 1.0)) { ui.ZSpin->setValue(1.0-NewVal);} //if increasing the size pushes out the end, push location back
		if (Snap) CurRegion->pRegion->dZ = GetSnapped(CurRegion->pRegion->dZ, 1.0/pEnv->pObj->GetVZDim()); //if snapping to voxel is enabled
	}
	emit RequestUpdateGL();
}

void Dlg_BCs::ChangedFixed(bool State)
{
	if (CurRegion && State && !CurRegion->Fixed){
		CVX_FRegion tmp = *CurRegion;
		DelCurBC();
		pEnv->AddFixedRegion(&tmp);

		int MyNewIndex = pEnv->GetNumFixed()-1;
		ui.BCList->setCurrentRow(MyNewIndex); //set the current elected to this one
		BCrowChanged(MyNewIndex); //need to call this explicitly sometimes, if new index happens to be the same as the old one...

		UpdateBCLists();
	}
}

void Dlg_BCs::ChangedForced(bool State)
{
	if (CurRegion && State && CurRegion->Fixed){
		CVX_FRegion tmp = *CurRegion;
		DelCurBC();
		pEnv->AddForcedRegion(&tmp);

		int MyNewIndex = pEnv->GetNumFixed()+pEnv->GetNumForced()-1;
		ui.BCList->setCurrentRow(MyNewIndex); //set the current elected to this one
		BCrowChanged(MyNewIndex); //need to call this explicitly sometimes, if new index happens to be the same as the old one...

		UpdateBCLists();
	}
}

void Dlg_BCs::ChangedXForce(QString NewText)
{
	if (CurRegion) CurRegion->Force.x = NewText.toDouble();
	emit RequestUpdateGL();
}

void Dlg_BCs::ChangedYForce(QString NewText)
{
	if (CurRegion) CurRegion->Force.y = NewText.toDouble();
	emit RequestUpdateGL();
}

void Dlg_BCs::ChangedZForce(QString NewText)
{
	if (CurRegion) CurRegion->Force.z = NewText.toDouble();
	emit RequestUpdateGL();
}

////MODEL
//
//int CBCModel::rowCount(const QModelIndex &parent) const
//{
//     return pEnv->GetNumMaterials();
//}
//
//QVariant CBCModel::data(const QModelIndex &index, int role) const
// {
//	 if (!index.isValid())
//         return QVariant();
//
//	if (index.row() >= pObj->GetNumMaterials())
//         return QVariant();
//
//	if (role == Qt::DisplayRole || role ==  Qt::EditRole)
//		return QVariant((pObj->Palette[index.row()].GetName()).c_str());
//	else if (role == Qt::DecorationRole){
//		int R, G, B;
//		pObj->Palette[index.row()].GetColori(&R, &G, &B);
//		return QColor(R,G,B);
//	}
//	else
//		return QVariant();
//
// }
//
//Qt::ItemFlags CBCModel::flags(const QModelIndex &index) const
//{
//	if (!index.isValid()) return Qt::ItemIsEnabled;
//
//	return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
//}
//
//bool CBCModel::setData(const QModelIndex &index, const QVariant &value, int role)
//{
//	if (index.isValid() && role == Qt::EditRole) {
//
//		pObj->Palette[index.row()].SetName(value.toString().toStdString());
//		emit dataChanged(index, index);
//		return true;
//	}
//	return false;
//}
//
//bool CBCModel::insertRows(int row, int count, const QModelIndex & parent)
//{
//	beginInsertRows(parent, rowCount(), rowCount()); //always only add one to the end
//	pObj->AddMat("Default");
//	endInsertRows();
//	return true;
//}
//
//bool CBCModel::removeRows(int row, int count, const QModelIndex & parent)
//{
//	beginRemoveRows(parent, row, row); //removes just the one material
//	pObj->DeleteMat(row);
//	endRemoveRows();
//	return true;
//}
//
//
//
//void CBCModel::UpdateList(void)
//{
//	emit dataChanged(index(0,0), index(pObj->GetNumMaterials(), 0));
//}