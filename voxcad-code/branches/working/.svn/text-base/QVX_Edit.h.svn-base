#ifndef QDM_EDIT_H
#define QDM_EDIT_H

#include "QVX_Interfaces.h"

enum Axis {XAXIS, YAXIS, ZAXIS}; //which axis do we refer to?
enum DrawTool {DT_PEN, DT_BOX, DT_ELLIPSE}; //current drawing tool for modifying the 2D view

class CQDM_Edit : public QVX_Object
{
	Q_OBJECT

public:
	CQDM_Edit();
	~CQDM_Edit();

signals:
	void UpdateGLWindows(void);
	void GetCurMaterial(int* MatIndex);
	void GetCurGLSelected(int* CurSel);
	void ModelChanged(void); //whenever the model (voxels) get edited

public slots:
	void DrawSceneView() {Draw3D();};
	void DrawSceneEdit() {Draw2D();};

//	void SelectedNew(int index);

	//mouse handlers:
	void HoverMove(Vec3D P);
	void LMouseDown(Vec3D P);
	void LMouseUp(Vec3D P);
	void LMouseDownMove(Vec3D P);
	void PressedEscape(void);
	void CtrlMouseRoll(bool Positive) {if (Positive) LayerForward(); else LayerBack();};

	void SetV2DTop(void) {CurSecAxis = ZAXIS; CurSecFromNeg = false; EnforceLayer();};
	void SetV2DBottom(void) {CurSecAxis = ZAXIS; CurSecFromNeg = true; EnforceLayer();};
	void SetV2DLeft(void) {CurSecAxis = YAXIS; CurSecFromNeg = true; EnforceLayer();};
	void SetV2DRight(void) {CurSecAxis = YAXIS; CurSecFromNeg = false; EnforceLayer();};
	void SetV2DFront(void) {CurSecAxis = XAXIS; CurSecFromNeg = false; EnforceLayer();};
	void SetV2DBack(void) {CurSecAxis = XAXIS; CurSecFromNeg = true; EnforceLayer();};

	void LayerBack(void);
	void LayerForward(void);

	void SetDrawPencil(void) {CurDrawTool = DT_PEN;};
	void SetDrawRectangle(void) {CurDrawTool = DT_BOX;};
	void SetDrawCircle(void) {CurDrawTool = DT_ELLIPSE;};

//	void GetCurIndex(int* CurIndex) {*CurIndex = CurSelected;};


public:
	void DrawMe(bool ShowSelected = true, int NumBehindSection = LAYERMAX);

	void Draw3D();
	void Draw2D();


	void Draw2DTransLayer(int LayerOffset, float Opacity = 0.7f); //opacity zero to 1 range
	void Draw2DOverlay();

	void SetSectionView(bool State = true);
	bool ViewSection; //flag that determines whether we are viewing a section as defined by Axis, Layer, FromNeg
	Axis CurSecAxis;
	int CurSecLayer;
	bool CurSecFromNeg;

	void EnforceLayer(); //makes sure current layer is valid given the axis...
	int GetCurSel(void){int tmp; emit GetCurGLSelected(&tmp); return tmp;};
	DrawTool GetCurDrawTool(void) {return CurDrawTool;};

	void ExtractCurLayer(CVXC_Structure* pOutput); //returns structure with ZDim of 1 of current slice...
	void ImposeLayerCur(CVXC_Structure* pInput); //pastes the z=0 layer of this structure onto current slice

private:
//	int CurSelected; //index of currently selected voxel
	std::vector<int> CurHighlighted; //index of voxel we are hovering over (currently used in 2D view only...)
	int V2DFindVoxelUnder(Vec3D Coord); //returns the index of a voxel under the coordinates


	bool V2DHighlightMe(int index); //highlight this voxel in 2D overlay layer?
	bool LMBDown; //flag for whether out left mouse button is down or not...
	Vec3D CurCoord; //current mouse coords in real units (not pixels)
	Vec3D DownCoord; //mouse coordinates in real numbers where mouse was pressed down

//	float CurMCX, CurMCY; //current mouse coords in real units (not pixels)
//	float DownMCX, DownMCY; //mouse coordinates in real numbers where mouse was pressed down

	DrawTool CurDrawTool;
	void FillHighlighted(void); //fill the CurHighlighted array based on current drawing tool and coodrinates
};

#endif // QDM_EDIT_H
