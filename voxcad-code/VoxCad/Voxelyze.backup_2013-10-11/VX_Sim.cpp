/*******************************************************************************
Copyright (c) 2010, Jonathan Hiller (Cornell University)
If used in publication cite "J. Hiller and H. Lipson "Dynamic Simulation of Soft Heterogeneous Objects" In press. (2011)"

This file is part of Voxelyze.
Voxelyze is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
Voxelyze is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
See <http://www.opensource.org/licenses/lgpl-3.0.html> for license details.
*******************************************************************************/

#include "VX_Sim.h"
#include "VXS_Voxel.h"
#include "VXS_Bond.h"
#include "Utils/MarchCube.h"
#include <math.h>
//#include "VX_SimGA.h"

#include <sstream>

#ifdef USE_OPEN_GL
#include "Utils/GL_Utils.h"
#endif


CVX_Sim::CVX_Sim(void)// : out("Logfile.txt", std::ios::ate)
{
	pEnv = NULL;

	CurColSystem = COL_SURFACE_HORIZON;
	CurIntegrator = I_EULER;

	PoissonKickBackEnabled = false;
	EnforceLatticeEnabled = false;

	SelfColEnabled = false;
	ColEnableChanged = true;
	CollisionHorizon = 3.0;

	MaxVelLimitEnabled = MemMaxVelEnabled = false;
	MaxVoxVelLimit = (vfloat)0.1;

	BlendingEnabled = false;
	MixRadius=0.0;
	BlendModel=MB_LINEAR;
	PolyExp = 1.0;

	FluidDampEnabled = false;

	PlasticityEnabled = true;
	FailureEnabled = true;

	InputVoxSInd = -1;
	InputBondInd = -1;

	BondDampingZ = MemBondDampZ = 0.1;
	ColDampingZ = 1;
	SlowDampingZ = MemSlowDampingZ = 0.001;

	EquilibriumModeEnabled = false;
//	EnableEquilibriumMode(false);
//	DisableEnergyHistory();
	KinEHistory.resize(HISTORY_SIZE, -1.0);
	TotEHistory.resize(HISTORY_SIZE, -1.0);
	MaxMoveHistory.resize(HISTORY_SIZE, -1.0);

	SetStopConditionType();
	SetStopConditionValue();



	DtFrac = (vfloat)0.9; //percent of maximum dt to use

	StatToCalc = CALCSTAT_ALL;

#ifdef USE_OPEN_GL
	NeedStatsUpdate=true;
	ViewForce = false;
	ViewAngles = false;
	CurViewMode = RVM_VOXELS;
	CurViewCol = RVC_TYPE;
	CurViewVox = RVV_DEFORMED;
#endif

	ClearAll();
	OptimalDt = 0; //remove when hack in ClearAll is dealt with
}

CVX_Sim::~CVX_Sim(void)
{
	ClearAll();

//	out.close();
}

CVX_Sim& CVX_Sim::operator=(const CVX_Sim& rSim) //overload "=" 
{
	//TODO: set everything sensible equal.

	return *this;
}

void CVX_Sim::SaveVXAFile(std::string filename)
{
	CXML_Rip XML;
	WriteVXA(&XML);
	XML.SaveFile(filename);
}

bool CVX_Sim::LoadVXAFile(std::string filename, std::string* pRetMsg)
{
	CXML_Rip XML;
	if (!XML.LoadFile(filename, pRetMsg)) return false;
	ReadVXA(&XML, pRetMsg);
	return true;
}

void CVX_Sim::WriteVXA(CXML_Rip* pXML)
{
	pXML->DownLevel("VXA");
	pXML->SetElAttribute("Version", "1.1");
	WriteXML(pXML);
	pEnv->WriteXML(pXML);
	pEnv->pObj->WriteXML(pXML);
	pXML->UpLevel();
}

bool CVX_Sim::ReadVXA(CXML_Rip* pXML, std::string* RetMessage) //pointer to VXA element
{
//	pObj->ClearMatter();
	std::string ThisVersion = "1.1";
	std::string Version;
	pXML->GetElAttribute("Version", &Version);
	if (atof(Version.c_str()) > atof(ThisVersion.c_str())) if (RetMessage) *RetMessage += "Attempting to open newer version of VXA file. Results may be unpredictable.\nUpgrade to newest version of VoxCAD.\n";

	if (pXML->FindElement("Simulator")){
		ReadXML(pXML);
		pXML->UpLevel();
	}

	//load environment
	if (pEnv && pXML->FindElement("Environment")){
		pEnv->ReadXML(pXML);
		pXML->UpLevel();
	}

	//Load VXC if pObj is valid...
	if (pEnv->pObj && (pXML->FindElement("VXC") || pXML->FindElement("DMF"))){
		pEnv->pObj->ReadXML(pXML, false, RetMessage);
		pXML->UpLevel();
	}
	return true;
}

void CVX_Sim::WriteXML(CXML_Rip* pXML)
{
	pXML->DownLevel("Simulator");
		pXML->DownLevel("Integration");
		pXML->Element("Integrator", (int)CurIntegrator);
		pXML->Element("DtFrac", DtFrac);
		pXML->UpLevel();

		pXML->DownLevel("Damping");
		pXML->Element("BondDampingZ", BondDampingZ);
		pXML->Element("ColDampingZ", ColDampingZ);
		pXML->Element("SlowDampingZ", SlowDampingZ);
		pXML->UpLevel();

		pXML->DownLevel("Collisions");
		pXML->Element("SelfColEnabled", SelfColEnabled);
		pXML->Element("ColSystem", CurColSystem);
		pXML->Element("CollisionHorizon", CollisionHorizon);
		pXML->UpLevel();

		pXML->DownLevel("Features");
		pXML->Element("MaxVelLimitEnabled", MaxVelLimitEnabled);
		pXML->Element("MaxVoxVelLimit", MaxVoxVelLimit);
		pXML->Element("BlendingEnabled", BlendingEnabled);
		pXML->Element("MixRadius", MixRadius);
		pXML->Element("BlendModel", BlendModel);
		pXML->Element("PolyExp", PolyExp);
		pXML->Element("FluidDampEnabled", FluidDampEnabled);
		pXML->Element("PoissonKickBackEnabled", PoissonKickBackEnabled);
		pXML->Element("EnforceLatticeEnabled", EnforceLatticeEnabled);
		pXML->UpLevel();

		pXML->DownLevel("StopCondition");
		pXML->Element("StopConditionType", (int)StopConditionType);
		pXML->Element("StopConditionValue", StopConditionValue);
		pXML->UpLevel();

		pXML->DownLevel("EquilibriumMode");
		pXML->Element("EquilibriumModeEnabled", EquilibriumModeEnabled);
	//	pXML->Element("StopConditionValue", StopConditionValue);
		pXML->UpLevel();

		pXML->DownLevel("SurfMesh");
		SurfMesh.DefMesh.WriteXML(pXML, true);
		pXML->UpLevel();

		WriteAdditionalSimXML(pXML);
	pXML->UpLevel();

}

bool CVX_Sim::ReadXML(CXML_Rip* pXML, std::string* RetMessage)
{
	int tmpInt;
	vfloat tmpVFloat;

	if (pXML->FindElement("Integration")){
		if (pXML->FindLoadElement("Integrator", &tmpInt)) CurIntegrator = (IntegrationType)tmpInt; else CurIntegrator = I_EULER;
		if (!pXML->FindLoadElement("DtFrac", &DtFrac)) DtFrac = (vfloat)0.9;
		pXML->UpLevel();
	}
		
	if (pXML->FindElement("Damping")){
		if (!pXML->FindLoadElement("BondDampingZ", &BondDampingZ)) BondDampingZ = 0.1;
		if (!pXML->FindLoadElement("ColDampingZ", &ColDampingZ)) ColDampingZ = 1.0;
		if (!pXML->FindLoadElement("SlowDampingZ", &SlowDampingZ)) SlowDampingZ = 1.0;
		pXML->UpLevel();
	}

	if (pXML->FindElement("Collisions")){
		if (!pXML->FindLoadElement("SelfColEnabled", &SelfColEnabled)) SelfColEnabled = false;
		if (pXML->FindLoadElement("ColSystem", &tmpInt)) CurColSystem = (ColSystem)tmpInt; else CurColSystem = COL_SURFACE_HORIZON;
		if (!pXML->FindLoadElement("CollisionHorizon", &CollisionHorizon)) CollisionHorizon = (vfloat)2.0;
		pXML->UpLevel();
	}

	if (pXML->FindElement("Features")){
		if (!pXML->FindLoadElement("MaxVelLimitEnabled", &MaxVelLimitEnabled)) MaxVelLimitEnabled = false;
		if (!pXML->FindLoadElement("MaxVoxVelLimit", &MaxVoxVelLimit)) MaxVoxVelLimit = (vfloat)0.1;
		if (!pXML->FindLoadElement("BlendingEnabled", &BlendingEnabled)) BlendingEnabled = false;
		if (!pXML->FindLoadElement("MixRadius", &MixRadius)) MixRadius = 0.0;
		if (pXML->FindLoadElement("BlendModel", &tmpInt)) BlendModel = (MatBlendModel)tmpInt; else BlendModel = MB_LINEAR;
		if (!pXML->FindLoadElement("PolyExp", &PolyExp)) PolyExp = 1.0;

		if (!pXML->FindLoadElement("FluidDampEnabled", &FluidDampEnabled)) FluidDampEnabled = false;
		if (!pXML->FindLoadElement("PoissonKickBackEnabled", &PoissonKickBackEnabled)) PoissonKickBackEnabled = false;
		if (!pXML->FindLoadElement("EnforceLatticeEnabled", &EnforceLatticeEnabled)) EnforceLatticeEnabled = false;
		pXML->UpLevel();
	}

	if (pXML->FindElement("StopCondition")){
		if (pXML->FindLoadElement("StopConditionType", &tmpInt)) SetStopConditionType((StopCondition)tmpInt); else SetStopConditionType();
		if (pXML->FindLoadElement("StopConditionValue", &tmpVFloat)) SetStopConditionValue(tmpVFloat); else SetStopConditionValue();
		pXML->UpLevel();
	}

	if (pXML->FindElement("EquilibriumMode")){
		if (!pXML->FindLoadElement("EquilibriumModeEnabled", &EquilibriumModeEnabled)) EquilibriumModeEnabled = false;
		if (EquilibriumModeEnabled) EnableEquilibriumMode(true); //so it can set up energy history if necessary
		pXML->UpLevel();
	}
	

	if (pXML->FindElement("SurfMesh")){
		if (pXML->FindElement("CMesh")){
			SurfMesh.DefMesh.ReadXML(pXML);
			pXML->UpLevel();
		}
		pXML->UpLevel();
	}

	
	return ReadAdditionalSimXML(pXML, RetMessage);
}


void CVX_Sim::ClearAll(void) //Reset all initialized variables
{
	Initalized = false;
	LocalVXC.ClearMatter();

	//This should be all the stuff set by "Import()"
	VoxArray.clear();
	BondArray.clear();
	XtoSIndexMap.clear();
	StoXIndexMap.clear();
	SurfVoxels.clear();

	OrigVoxArray.clear();
	A.clear();
	B.clear();
	C.clear();
	D.clear();
	OrigBondArray.clear();

	MaxDispSinceLastBondUpdate = (vfloat)FLT_MAX; //arbitrarily high as a flag to populate bonds

	ClearHistories();

	dt = (vfloat)0.0; //calculated per-step
	CurTime = (vfloat)0.0;
	TimeOfLastNeuralUpdate = (vfloat) -9999.0;
	CurStepCount = 0;
	DtFrozen = false;

	SS.Clear();
	IniCM = Vec3D<>(0,0,0);
	CurXSel = -1;
	Dragging = false;
}

bool CVX_Sim::UpdateAllVoxPointers() //updates all pointers into the VoxArray (call if reallocated!)
{
	for (std::vector<CVXS_Bond>::iterator it = BondArray.begin(); it != BondArray.end(); it++){
		if (!it->UpdateVoxPtrs()) return false;
	}
	return true;
}

int CVX_Sim::NumVox(void) const
{
	return (int)VoxArray.size()-1; //-1 is because we always have an input voxel shadow...
}

bool CVX_Sim::UpdateAllBondPointers() //updates all pointers into the VoxArray (call if reallocated!)
{
	for (std::vector<CVXS_Voxel>::iterator it = VoxArray.begin(); it != VoxArray.end(); it++){
		if (!it->UpdateBondLinks()) return false;
	}
	return true;
}

int CVX_Sim::NumBond(void) const
{
	return (int)BondArray.size();
}

void CVX_Sim::DeleteTmpBonds(void)
{
	int iT = NumVox();
	for (int i=0; i<iT; i++){
		VoxArray[i].UnlinkTmpBonds();
	}
	int NumBonds = NumBond();
	while (!BondArray[NumBonds-1].IsPermanent()){
		BondArray.pop_back();
		NumBonds--;
	}
}

CVXS_Voxel* CVX_Sim::InputVoxel(void)
{
	return &VoxArray[InputVoxSInd];
}

CVXS_Bond* CVX_Sim::InputBond(void)
{
	return &BondArray[InputBondInd];
}

/*! The environment should have been previously initialized and linked with a single voxel object. 
This function sets or resets the entire simulation with the new environment.
@param[in] pEnvIn A pointer to initialized CVX_Environment to import into the simulator.
@param[out] RetMessage A pointer to initialized string. Output information from the Import function is appended to this string.
*/
bool CVX_Sim::Import(CVX_Environment* pEnvIn, CMesh* pSurfMeshIn, std::string* RetMessage)
{
	ClearAll(); //clears out all arrays and stuff

	if (pEnvIn != NULL) pEnv = pEnvIn;
	if (pEnv == NULL) {if (RetMessage) *RetMessage += "Invalid Environment pointer"; return false;}

	LocalVXC = *pEnv->pObj; //make a copy of the reference digital object!
	if (LocalVXC.GetNumVox() == 0) {if (RetMessage) *RetMessage += "No voxels in object"; return false;}

	int SIndexIt = 0; //keep track of how many voxel we've added (for storing reverse lookup array...)
	int NumBCs = pEnv->GetNumBCs();
	CVX_FRegion* pCurBc;
	
	//initialize XtoSIndexMap & StoXIndexMap
	XtoSIndexMap.resize(LocalVXC.GetStArraySize(), -1); // = new int[LocalVXC.GetStArraySize()];
	StoXIndexMap.resize(LocalVXC.GetNumVox(), -1); // = new int [m_NumVox];

	std::vector<int> Sizes(NumBCs, 0);
	for (int i=0; i<NumBCs; i++) Sizes[i] = pEnv->GetNumTouching(i);
//	pEnv->GetNumVoxTouchingForced(&Sizes); //get the number of voxels in each region (to apply equal force to each voxel within this region!)

//	Vec3D BCpoint;
	Vec3D<> BCsize = pEnv->pObj->GetLatDimEnv()/2.0;
	Vec3D<> WSSize = pEnv->pObj->GetWorkSpace();

	//Add all Voxels:
	bool HasPlasticMaterial = false;
	Vec3D<> ThisPos;
	Vec3D<> ThisScale = LocalVXC.GetLatDimEnv();
	//Build voxel list
	for (int i=0; i<LocalVXC.GetStArraySize(); i++){ //for each voxel in the array
		XtoSIndexMap[i] = -1; //assume there is not a voxel here...

		if(LocalVXC.Structure[i] != 0 ){ //if there's material here
			int ThisMatIndex = LocalVXC.GetLeafMatIndex(i); 
			int ThisMatModel = LocalVXC.Palette[ThisMatIndex].GetMatModel();
			if (ThisMatModel == MDL_BILINEAR || ThisMatModel == MDL_DATA) HasPlasticMaterial = true; //enable plasticity in the sim

			LocalVXC.GetXYZ(&ThisPos, i, false);//Get XYZ location

			CVXS_Voxel CurVox(this, SIndexIt, i, ThisMatIndex, ThisPos, ThisScale);

			XtoSIndexMap[i] = SIndexIt; //so we can find this voxel based on it's original index
			StoXIndexMap[SIndexIt] = i; //so we can find the original index based on its simulator position
			
			for (int j = 0; j<NumBCs; j++){ //go through each primitive defined as a constraint!
				pCurBc = pEnv->GetBC(j);
				if (pCurBc->GetRegion()->IsTouching(&ThisPos, &BCsize, &WSSize)){ //if this point is within
					CurVox.FixDof(pCurBc->DofFixed);
					CurVox.AddExternalForce(pCurBc->Force/Sizes[j]);
					CurVox.AddExternalTorque(pCurBc->Torque/Sizes[j]);
					CurVox.SetExternalDisp(pCurBc->Displace);
					CurVox.SetExternalTDisp(pCurBc->AngDisplace);
				}
			}

			if(BlendingEnabled) CurVox.CalcMyBlendMix(); //needs to be done basically last.
			VoxArray.push_back(CurVox);
			SIndexIt++;
		}
	}

	//add input voxel so that NumVox() works!
	InputVoxSInd = (int)VoxArray.size();
	CVXS_Voxel TmpVox(this, 0, 0, 0, Vec3D<>(0,0,0), Vec3D<>(0,0,0));
//	TmpVox.LinkToVXSim(this);
	VoxArray.push_back(TmpVox);

	//SET UP ALL PERMANENT BONDS (in between materials and the input bond)
	Vec3D<> RelDist;

	//exhaustive (slower, but still OK)
	for (int i=0; i<NumVox(); i++){ //for each voxel in our newly-made array
		for (int j = i+1; j<NumVox(); j++){
			if (!LocalVXC.IsAdjacent(StoXIndexMap[i], StoXIndexMap[j], false, &RelDist)) {continue;}
			if (!CreateBond(B_LINEAR, i, j, true) && RetMessage) { *RetMessage += "At least one bond creation failed during import";}
			
	
		}
	}

	//Create input bond
	CreateBond(B_INPUT_LINEAR_NOROT, InputVoxSInd, InputVoxSInd, true, &InputBondInd, false); //create input bond, but initialize it to meaningless connection to self

	UpdateAllBondPointers(); //necessary since we probably reallocated the bond array when adding pbonds the first time

	//Set up our surface list...
	for (int i=0; i<NumVox(); i++){ //for each voxel in our newly-made array
		if (VoxArray[i].GetNumLocalBonds() != 6){
			SurfVoxels.push_back(i);
		}

		//todo: only do for those on surfaces, I think.
		VoxArray[i].CalcNearby((int)(CollisionHorizon*1.5)); //populate the nearby array

	}




#ifdef USE_OPEN_GL
	VoxMesh.ImportLinkSim(this);
	VoxMesh.DefMesh.DrawSmooth = false;

	//if the input mesh is not valid, use marching cubes to create one
	if (!pSurfMeshIn){
		CMesh GeneratedSmoothMesh;

		CArray3Df OccupancyArray(pEnv->pObj->GetVXDim(), pEnv->pObj->GetVYDim(), pEnv->pObj->GetVZDim()); 
		int NumPossibleVox = pEnv->pObj->GetStArraySize();
		for (int g=0; g<NumPossibleVox; g++){
			if (pEnv->pObj->Structure.GetData(g)>0) OccupancyArray[g] = 1.0;
		}
		CMarchCube::SingleMaterial(&GeneratedSmoothMesh, &OccupancyArray, 0.5, pEnv->pObj->GetLatticeDim());
		SurfMesh.ImportSimWithMesh(this, &GeneratedSmoothMesh);
	}
	else SurfMesh.ImportSimWithMesh(this, pSurfMeshIn);


#endif


	ResetSimulation();
	OptimalDt = CalcMaxDt(); //to set up dialogs parameter ranges, we need this before the first iteration.
	EnablePlasticity(HasPlasticMaterial); //turn off plasticity if we don't need it...

	Initalized = true;
//	std::string tmpString;

	std::ostringstream os;
	os << "Completed Simulation Import: " << NumVox() << " Voxels, " << NumBond() << "Bonds.\n";
	*RetMessage += os.str();

	return true;
}

/*! This bond is appended to the master bond array (BondArray). 
The behavior of the bond is determined by BondType. If the bond is permanent and should persist throughout the simulation PermIn should be to true.
@param[in] BondTypeIn The physical behavior of the bond being added.
@param[in] SIndex1 One simulation voxel index to be joined.
@param[in] SIndex2 The other simulation voxel index to be joined.
@param[in] PermIn Denotes whether this bond should persist throughout the simulation (true) or is temporary (false).
*/
bool CVX_Sim::CreateBond(BondType BondTypeIn, int SIndex1In, int SIndex2In, bool PermIn, int* pBondIndexOut, bool LinkBond) //take care of all dynamic memory stuff...
{
	if(IS_ALL_FIXED(VoxArray[SIndex1In].GetDofFixed()) && IS_ALL_FIXED(VoxArray[SIndex2In].GetDofFixed())) return true; //if both voxels are fixed don't bother making a bond. (unnecessary)

	CVXS_Bond tmp(this);
	// std::cout << "DEBUG MALLOC: GOT HERE -- STARTSTART." <<  std::endl;
	if (!tmp.DefineBond(BondTypeIn, SIndex1In, SIndex2In, PermIn)) return false;
//	tmp.ThisBondType = BondTypeIn;
//	tmp.SetVox1SInd(SIndex1In);
//	tmp.SetVox2SInd(SIndex2In);
//	tmp.OrigDist = VoxArray[SIndex2In].GetOrigPos() - VoxArray[SIndex1In].GetOrigPos(); //was S.Pos
//	tmp.Perm = PermIn; 

//	tmp.UpdateConstants(); //do this here, cause we can...
	// std::cout << "DEBUG MALLOC: GOT HERE -- END3." <<  std::endl;

	BondArray.push_back(tmp);
	
	int MyBondIndex = NumBond()-1;

	if(LinkBond){
		VoxArray[SIndex1In].LinkBond(MyBondIndex); // .BondInds.push_back(NumBond());
		VoxArray[SIndex2In].LinkBond(MyBondIndex); // BondInds.push_back(NumBond());
	}
	if (pBondIndexOut) *pBondIndexOut = MyBondIndex;
	return true;
}

bool CVX_Sim::UpdateBond(int BondIndex, int NewSIndex1In, int NewSIndex2In, bool LinkBond)
{
	CVXS_Bond* pThisBond = &BondArray[BondIndex];

	if(LinkBond){ //unlink pointers to this bond from other voxels
		VoxArray[pThisBond->GetVox1SInd()].UnlinkBond(BondIndex);
		VoxArray[pThisBond->GetVox2SInd()].UnlinkBond(BondIndex);
	}

	pThisBond->DefineBond(pThisBond->GetBondType(), NewSIndex1In, NewSIndex2In, pThisBond->IsPermanent());
	//set and linknew indices
//	if (!pThisBond->SetVox1SInd(NewSIndex1In)) return false;
//	if (!pThisBond->SetVox2SInd(NewSIndex2In)) return false;

//	pThisBond->OrigDist = VoxArray[NewSIndex1In].GetOrigPos() - VoxArray[NewSIndex2In].GetOrigPos(); //Was S.Pos
//	pThisBond->UpdateConstants(); //material may have changed with switch
//	pThisBond->ResetBond(); //??

	if(LinkBond){ //link the involved voxels to this bond
		VoxArray[NewSIndex1In].LinkBond(BondIndex);
		VoxArray[NewSIndex2In].LinkBond(BondIndex);
	}

	return true;
}


void CVX_Sim::ResetSimulation(void)
{
	int iT = NumVox();
	for (int i=0; i<iT; i++) VoxArray[i].ResetVoxel();

	iT = NumBond();
	for (int j=0; j<iT; j++) BondArray[j].ResetBond();

	CurTime = (vfloat)0.0;
	TimeOfLastNeuralUpdate = (vfloat) -9999.0;
	CurStepCount = 0;

	ClearHistories();

	SS.Clear();
}

/*! Given the current state of the simulation (Voxel positions and velocities) and information about the current environment, advances the simulation by the maximum stable timestep. 
The integration scheme denoted by the CurIntegrator member variable is used.
Calculates some relevant system statistics such as maximum displacements and velocities and total force.
Returns true if the time step was successful, false otherwise.
@param[out] pRetMessage Pointer to an initialized string. Messages generated in this function will be appended to the string.
*/
bool CVX_Sim::TimeStep(std::string* pRetMessage)
{
	if(SelfColEnabled) UpdateCollisions(); //update self intersection lists if necessary
	else if (!SelfColEnabled && ColEnableChanged){ColEnableChanged=false; DeleteTmpBonds();}

	UpdateMatTemps(); //updates the temperatures

	//update information to calculate
	switch (GetStopConditionType()){ //may need to calculate certain items depending on stop condition
	case SC_CONST_MAXENERGY: StatToCalc |= CALCSTAT_KINE; StatToCalc |= CALCSTAT_STRAINE; break;
	case SC_MIN_KE: StatToCalc |= CALCSTAT_KINE; break;
	case SC_MIN_MAXMOVE: StatToCalc |= CALCSTAT_VEL; break;
	}
	if (IsEquilibriumEnabled()) StatToCalc |= CALCSTAT_KINE;

	if (!Integrate(CurIntegrator)){
		if (pRetMessage) *pRetMessage = "Simulation Diverged. Please reduce forces or accelerations.\n";	
		return false;
	}
	
	if (IsEquilibriumEnabled() && KineticEDecreasing()) ZeroAllMotion();
	UpdateStats(pRetMessage);
	return true;
}

void CVX_Sim::EnableEquilibriumMode(bool Enabled)
{
	EquilibriumModeEnabled = Enabled;
	if (EquilibriumModeEnabled){
		MemBondDampZ = BondDampingZ;
		MemSlowDampingZ = SlowDampingZ;
		MemMaxVelEnabled = MaxVelLimitEnabled;

		BondDampingZ = 0.1;
		SlowDampingZ = 0;
		MaxVelLimitEnabled = false;
	}
	else {
		BondDampingZ = MemBondDampZ;
		SlowDampingZ = MemSlowDampingZ;
		MaxVelLimitEnabled = MemMaxVelEnabled;
	}
}

void CVX_Sim::ZeroAllMotion(void)
{
	int NumVoxLoc = NumVox();
	for (int i=0; i<NumVoxLoc; i++){
		VoxArray[i].ZeroMotion();
	}
}


//void CVX_Sim::SetStopConditionType(StopCondition StopConditionTypeIn)
//{
//	//Type
//	if (StopConditionType != StopConditionTypeIn){
//		StopConditionType = StopConditionTypeIn;
//
//		if (StopCondRqsEnergy()){ //enable energy history at 100 (or more)
//			EnableEnergyHistory(501);
//		}
//		else { //disable energy history if eq mode doesn't need it
//			if (!EquilibriumModeEnabled) DisableEnergyHistory();
//		}
//	}
//}

bool CVX_Sim::StopConditionMet(void) //have we met the stop condition yet?
{
	int numJump; //how many timesteps to look back in order to have 10 data points within the history length
	vfloat fNumVoxInv;
	if (StopConditionType==SC_CONST_MAXENERGY || StopConditionType==SC_MIN_KE || StopConditionType==SC_MIN_MAXMOVE){
		fNumVoxInv = 1.0/(float)NumVox();
		numJump = HISTORY_SIZE/10;
	}

	switch(StopConditionType){
		case SC_NONE: return false;
		case SC_MAX_TIME_STEPS: return (CurStepCount>(int)(StopConditionValue+0.5))?true:false;
		case SC_MAX_SIM_TIME: return CurTime>StopConditionValue?true:false;
		case SC_TEMP_CYCLES:  return CurTime>pEnv->GetTempPeriod()*StopConditionValue?true:false;
		case SC_CONST_MAXENERGY:{
			vfloat IniTotVal = TotEHistory[0];
			for (int i=numJump; i<HISTORY_SIZE; i+=numJump){
				if (TotEHistory[i] == -1) return false;
				if (abs(TotEHistory[i]-IniTotVal)*fNumVoxInv > 0.001*StopConditionValue) return false;
			}
			return true;
		  }
		case SC_MIN_KE:{
			for (int i=0; i<HISTORY_SIZE; i+=numJump){
				if (KinEHistory[i] == -1) return false;
				if (KinEHistory[i]*fNumVoxInv > 0.001*StopConditionValue) return false;
			}
			return true;
		  }
		case SC_MIN_MAXMOVE:{
			for (int i=0; i<HISTORY_SIZE; i+=numJump){
				if (MaxMoveHistory[i] == -1) return false;
				if (MaxMoveHistory[i] > 0.001*StopConditionValue) return false;
			}
			return true;
		}

		default: return false;
	}
}

bool CVX_Sim::UpdateStats(std::string* pRetMessage) //updates simulation state (SS)
{
	if (SelfColEnabled) StatToCalc |= CALCSTAT_VEL; //always need velocities if self collisition is enabled
	if (StatToCalc == CALCSTAT_NONE) return true;
	bool CCom=StatToCalc&CALCSTAT_COM, CDisp=StatToCalc&CALCSTAT_DISP, CVel=StatToCalc & CALCSTAT_VEL, CKinE=StatToCalc&CALCSTAT_KINE, CStrE=StatToCalc&CALCSTAT_STRAINE, CEStrn=StatToCalc&CALCSTAT_ENGSTRAIN, CEStrs=StatToCalc&CALCSTAT_ENGSTRESS;

	if (CCom) SS.CurCM = GetCM(); //calculate center of mass

	//update the overall statisics (can't do this within threaded loops and be safe without mutexes...
	vfloat tmpMaxVoxDisp2 = 0, tmpMaxVoxVel2 = 0, tmpMaxVoxKineticE = 0, tmpMaxVoxStrainE = 0; //need to sqrt these before we set them
	vfloat tmpMaxBondStrain=0, tmpMaxBondStress=0, tmpTotalObjKineticE = 0, tmpTotalObjStrainE=0;
	Vec3D<> tmpTotalObjDisp(0,0,0);

	if (CDisp || CVel || CKinE){
		int nVox = NumVox();
		for (int i=0; i<nVox; i++){ //for each voxel
			if (i == InputVoxSInd) continue;
			const CVXS_Voxel* it = &VoxArray[i]; //pointer to this voxel

			if (CDisp) { //Displacements
				tmpTotalObjDisp += it->GetCurVel().Abs()*dt; //keep track of displacements on global object
				const vfloat ThisMaxVoxDisp2 = (it->GetCurPos()-it->GetOrigPos()).Length2();
				if (ThisMaxVoxDisp2 > tmpMaxVoxDisp2) tmpMaxVoxDisp2 = ThisMaxVoxDisp2;
			}

			if (CVel) { //Velocities
				const vfloat ThisMaxVoxVel2 = it->GetCurVel().Length2();
				if (ThisMaxVoxVel2 > tmpMaxVoxVel2) tmpMaxVoxVel2 = ThisMaxVoxVel2;
			}
			if (CKinE) { // kinetic energy
				const vfloat ThisMaxKineticE =  it->GetCurKineticE();
				if (ThisMaxKineticE > tmpMaxVoxKineticE) tmpMaxVoxKineticE = ThisMaxKineticE;
				tmpTotalObjKineticE += ThisMaxKineticE; //keep track of total kinetic energy
			}
		}

		if (CDisp){ //Update SimState (SS)
			tmpTotalObjDisp /= nVox;
			SS.TotalObjDisp = tmpTotalObjDisp;
			SS.NormObjDisp = tmpTotalObjDisp.Length();
			SS.MaxVoxDisp = sqrt(tmpMaxVoxDisp2);
		}

		if (CVel) SS.MaxVoxVel = sqrt(tmpMaxVoxVel2);
		if (CKinE) {
			SS.MaxVoxKinE = tmpMaxVoxKineticE;
			SS.TotalObjKineticE = tmpTotalObjKineticE;
		}

	}

	if (CStrE || CEStrn || CEStrs){
		for (std::vector<CVXS_Bond>::iterator it = BondArray.begin(); it != BondArray.end(); it++){
			if (CStrE){
				const vfloat ThisMaxStrainE =  it->GetStrainEnergy();
				if (ThisMaxStrainE > tmpMaxVoxStrainE) tmpMaxVoxStrainE = ThisMaxStrainE;
				tmpTotalObjStrainE += ThisMaxStrainE;
			}

			if (CEStrn && it->GetEngStrain() > tmpMaxBondStrain) tmpMaxBondStrain = it->GetEngStrain(); //shouldn't these pull from bonds? would make more sense...
			if (CEStrs && it->GetEngStress() > tmpMaxBondStress) tmpMaxBondStress = it->GetEngStress();
		}
	
		//Updata SimState (SS)
		if (CStrE){
			SS.MaxBondStrainE = tmpMaxVoxStrainE;
			SS.TotalObjStrainE = tmpTotalObjStrainE;
		}

		if (CEStrn) SS.MaxBondStrain = tmpMaxBondStrain;
		if (CEStrs) SS.MaxBondStress = tmpMaxBondStress;

	}


	//update histories
	MaxMoveHistory.push_front(CVel ? SS.MaxVoxVel*dt : -1.0); MaxMoveHistory.pop_back();
	KinEHistory.push_front(CKinE ? SS.TotalObjKineticE : -1.0); KinEHistory.pop_back();
	TotEHistory.push_front((CStrE && CKinE) ? SS.TotalObjKineticE + SS.TotalObjStrainE : -1.0); TotEHistory.pop_back();

	return true;
}


vfloat CVX_Sim::CalcMaxDt(void)
{
	vfloat MaxFreq2 = 0; //maximum frequency in the simulation
	vfloat MaxRFreq2 = 0; //maximum frequency in the simulation

	int iT = NumBond();
	for (int i=0; i<iT; i++){
		if (i==InputBondInd) continue; //zero mass of input voxel causes problems
		if (BondArray[i].GetLinearStiffness()/BondArray[i].GetpV1()->GetMass() > MaxFreq2) MaxFreq2 = BondArray[i].GetLinearStiffness()/BondArray[i].GetpV1()->GetMass();
		if (BondArray[i].GetLinearStiffness()/BondArray[i].GetpV2()->GetMass() > MaxFreq2) MaxFreq2 = BondArray[i].GetLinearStiffness()/BondArray[i].GetpV2()->GetMass();
	}

	//calculate dt: (as large as possible...)
	vfloat MaxFreq = sqrt(MaxFreq2);
	return 1.0/(MaxFreq*2*(vfloat)3.1415926); //convert to time... (seconds)

}

void CVX_Sim::UpdateCollisions(void) // Called every timestep to watch for collisions
{
	//self intersection accumulator
	MaxDispSinceLastBondUpdate += abs(SS.MaxVoxVel*dt/LocalVXC.GetLatticeDim());

	if (CurColSystem == COL_BASIC_HORIZON || CurColSystem == COL_SURFACE_HORIZON){
		if (MaxDispSinceLastBondUpdate > (CollisionHorizon-1.0)/2 || ColEnableChanged){ //if we want to check for self intersection (slow!)
			ColEnableChanged = false;
			CalcL1Bonds(CollisionHorizon); //doesn't need to be done every time...
			MaxDispSinceLastBondUpdate = 0.0;
		}
	}
	else { //if COL_BASIC || COL_SURFACE
		CalcL1Bonds(CollisionHorizon);
	}
}

void CVX_Sim::UpdateMatTemps(void) //updates expansions for each material
{
	pEnv->UpdateCurTemp(CurTime);
}

bool CVX_Sim::Integrate(IntegrationType Integrator)
{
	switch (Integrator){
		case I_EULER: 
		{
			//Update Forces...
			int iT = NumBond();

			bool Diverged = false;
#pragma omp parallel for
			for (int i=0; i<iT; i++)
			{
				BondArray[i].UpdateBond();
				if (BondArray[i].GetEngStrain() > 100) Diverged = true; //catch divergent condition! (if any thread sets true we will fail, so don't need mutex...
			}
			if (Diverged) return false;

			//if (!DtFrozen){ //for now, dt cannot change within the simulation (and this is a cycle hog)
			//	OptimalDt = CalcMaxDt();
				dt = DtFrac*OptimalDt;
			//}

			//Update positions... need to do this seperately if we're going to do damping in the summing forces stage.
			iT = NumVox();

#pragma omp parallel for

			if (CurTime - TimeOfLastNeuralUpdate > pEnv->GetNeuralUpdatePeriod())
			{
				TimeOfLastNeuralUpdate = CurTime;
				// UPDATE NEURAL NETWORK
				SetNeuralOutputOfIndex(0,GetSensorValueOfIndex(0));
			}


			for (int i=0; i<iT; i++) { VoxArray[i].EulerStep();}
			
			if (true) // Save time by running selectively???
			{	
				for (int i=0; i<iT; i++) 
				{
					// if (GetBaseMat(VoxArray[i].GetMaterial())
					// VoxArray[i].EulerStep();
					if ( LocalVXC.GetBaseMat(VoxArray[i].GetMaterial())->GetIsConductive() )
					{
						// if (CurTime - VoxArray[i].GetRepolarizationStartTime() > pEnv->GetElectricityUpdatePeriod() )
						// {
							VoxArray[i].SetElectricallyActiveOld( VoxArray[i].GetElectricallyActiveNew() );
							VoxArray[i].SetMembranePotentialOld( VoxArray[i].GetMembranePotentialNew() ); 
						// }
					}
				}
			}
		}
		break;

		case I_VERLET: { //Experimental
			vfloat MaxFreq2 = 0; //maximum frequency in the simulation
			vfloat MaxRFreq2 = 0; //maximum frequency in the simulation

			//Update Forces...
			int iT = NumBond();
			for (int i=0; i<iT; i++){
				BondArray[i].UpdateBond();
				if (BondArray[i].GetLinearStiffness()/BondArray[i].GetpV1()->GetMass() > MaxFreq2) MaxFreq2 = BondArray[i].GetLinearStiffness()/BondArray[i].GetpV1()->GetMass();
				if (BondArray[i].GetLinearStiffness()/BondArray[i].GetpV2()->GetMass() > MaxFreq2) MaxFreq2 = BondArray[i].GetLinearStiffness()/BondArray[i].GetpV2()->GetMass();
			}

			//calculate dt: (as large as possible...)
			if (!DtFrozen){
				vfloat MaxFreq = sqrt(MaxFreq2);
				dt = DtFrac/(MaxFreq*2*(vfloat)3.1415926); //safety factor of 10% on the timestep...
			}

			//Update positions... need to do thises seperately if we're going to do damping in the summing forces stage.
			iT = NumVox();
			for (int i=0; i<iT; i++){
				VoxArray[i].VerletStep();
			}
			
		}
		break;
		case I_RK4: { //Experimental
			////resize the array if needed
			//int VoxArraySize = VoxArray.size();
			//CVXS_Voxel defaultVoxel = CVXS_Voxel(this, 0, 0, 0, Vec3D(0,0,0), Vec3D(0,0,0));
			//if (OrigVoxArray.size() != VoxArraySize){ //if sizes have changed...
			//	OrigVoxArray.resize(VoxArraySize, defaultVoxel);
			//	A.resize(VoxArraySize, defaultVoxel);
			//	B.resize(VoxArraySize, defaultVoxel);
			//	C.resize(VoxArraySize, defaultVoxel);
			//	D.resize(VoxArraySize, defaultVoxel);
			//}
			//int BondArraySize = BondArray.size();
			//if (OrigBondArray.size() != BondArraySize)	OrigBondArray.resize(BondArraySize);


			//dt = (vfloat)0.00014*DtFrac;
			//int i, iT;
			//int NV = NumVox(); //num voxels constant, num bonds not necessarily
			////save the initial state.
			//for (i=0; i<VoxArraySize; i++) OrigVoxArray[i] = VoxArray[i];
			//for (i=0; i<BondArraySize; i++) OrigBondArray[i] = BondArray[i];

			////get the derivative at the current state:
			//iT = NumBond();
			//for (i=0; i<iT; i++) BondArray[i].CalcForce(); //CalcForce() uses whatever is in VoxArray to calculate forces...
			//for (i=0; i<VoxArraySize; i++) A[i] = VoxArray[i]; //set A = to original voxel array
			//for (i=0; i<NV; i++) A[i].UpdateDState(); //updates derivative of state A based on the S we have in VoxArray at the end of last time step

			//for (i=0; i<NV; i++) VoxArray[i].UpdateState(dt/2, A[i].dS); //Advances VoxArray according to derivative of A
			//iT = NumBond();
			//for (i=0; i<iT; i++)	BondArray[i].CalcForce(); //CalcForce() uses whatever is in VoxArray to calculate forces...
			//for (i=0; i<VoxArraySize; i++) B[i] = VoxArray[i]; //set B = to original voxel array
			//for (i=0; i<NV; i++) B[i].UpdateDState(); //updates derivate of B at 1/2 dt
			//VoxArray = OrigVoxArray; //reset VoxArray

			//for (i=0; i<NV; i++) VoxArray[i].UpdateState(dt/2, B[i].dS);
			//iT = NumBond();
			//for (i=0; i<iT; i++) BondArray[i].CalcForce(); //CalcForce() uses whatever is in VoxArray to calculate forces...
			//for (i=0; i<VoxArraySize; i++) C[i] = VoxArray[i]; //set C = to original voxel array
			//for (i=0; i<NV; i++) C[i].UpdateDState();
			//VoxArray = OrigVoxArray; //reset VoxArray

			//for (int i=0; i<NV; i++) VoxArray[i].UpdateState(dt, C[i].dS);
			//iT = NumBond();
			//for (int i=0; i<iT; i++)	BondArray[i].CalcForce(); //CalcForce() uses whatever is in VoxArray to calculate forces...
			//for (int i=0; i<VoxArraySize; i++) D[i] = VoxArray[i]; //set D = to original voxel array
			//for (int i=0; i<NV; i++) D[i].UpdateDState();
			//VoxArray = OrigVoxArray; //reset VoxArray

			////do the RK4 magic!
			//for (int i=0; i<NV; i++){
			//	VoxArray[i].dS.Force = (A[i].dS.Force + 2.0*(B[i].dS.Force + C[i].dS.Force) + D[i].dS.Force)/6.0;
			//	VoxArray[i].dS.LinMom = (A[i].dS.LinMom + 2.0*(B[i].dS.LinMom + C[i].dS.LinMom) + D[i].dS.LinMom)/6.0;
			//	VoxArray[i].dS.Moment = (A[i].dS.Moment + 2.0*(B[i].dS.Moment + C[i].dS.Moment) + D[i].dS.Moment)/6.0;
			//	VoxArray[i].dS.AngMom = (A[i].dS.AngMom + 2.0*(B[i].dS.AngMom + C[i].dS.AngMom) + D[i].dS.AngMom)/6.0;
			//	VoxArray[i].dS.ScaleForce = (A[i].dS.ScaleForce + 2.0*(B[i].dS.ScaleForce + C[i].dS.ScaleForce) + D[i].dS.ScaleForce)/6.0;
			//	VoxArray[i].dS.ScaleMom = (A[i].dS.ScaleMom + 2.0*(B[i].dS.ScaleMom + C[i].dS.ScaleMom) + D[i].dS.ScaleMom)/6.0;
			//}
			//for (int i=0; i<NV; i++) VoxArray[i].UpdateState(dt, VoxArray[i].dS);


		}
		break;

	}
	CurTime += dt; //keep track of time!
	CurStepCount++; //keep track of current step...

	return true;
}

#ifdef USE_OPEN_GL

void CVX_Sim::Draw(int Selected, bool ViewSection, int SectionLayer)
{
	if (!Initalized) return;

	if (CurViewMode == RVM_NONE) return;
	else if (CurViewMode == RVM_VOXELS){ 
		switch (CurViewVox){
		case RVV_DISCRETE: DrawGeometry(Selected, ViewSection, SectionLayer); break; //section view only currently enabled in voxel view mode
		case RVV_DEFORMED: DrawVoxMesh(Selected); break;
		case RVV_SMOOTH: DrawSurfMesh(); break;
		}
	}
	else { //CurViewMode == RVT_BONDS
		DrawBonds();
		DrawStaticFric();
	}
	if (ViewAngles)	DrawAngles();
	if (ViewForce) DrawForce();
	if (pEnv->IsFloorEnabled()) DrawFloor(); //draw the floor if its in use

	NeedStatsUpdate=true;
}

void CVX_Sim::DrawForce(void)
{
	//TODO
}

void CVX_Sim::DrawFloor(void)
{

	//TODO: build an openGL list 
	vfloat Size = LocalVXC.GetLatticeDim()*4;
	vfloat sX = 1.5*Size;
	vfloat sY = .866*Size;

	glEnable(GL_LIGHTING);

	glLoadName (-1); //never want to pick floor

	glNormal3d(0.0, 0.0, 1.0);
	for (int i=-20; i <=30; i++){
		for (int j=-40; j <=60; j++){
			glColor4d(0.6, 0.7+0.2*((int)(1000*sin((float)(i+110)*(j+106)*(j+302)))%10)/10.0, 0.6, 1.0);
			glBegin(GL_TRIANGLE_FAN);
			glVertex3d(i*sX, j*sY, 0.0);
			glVertex3d(i*sX+0.5*Size, j*sY, 0.0);
			glVertex3d(i*sX+0.25*Size, j*sY+0.433*Size, 0.0);
			glVertex3d(i*sX-0.25*Size, j*sY+0.433*Size, 0.0);
			glVertex3d(i*sX-0.5*Size, j*sY, 0.0);
			glVertex3d(i*sX-0.25*Size, j*sY-0.433*Size, 0.0);
			glVertex3d(i*sX+0.25*Size, j*sY-0.433*Size, 0.0);
			glVertex3d(i*sX+0.5*Size, j*sY, 0.0);
			glEnd();

			glColor4d(0.6, 0.7+0.2*((int)(1000*sin((float)(i+100)*(j+103)*(j+369)))%10)/10.0, 0.6, 1.0);
			
			glBegin(GL_TRIANGLE_FAN);
			glVertex3d(i*sX+.75*Size, j*sY+0.433*Size, 0.0);
			glVertex3d(i*sX+1.25*Size, j*sY+0.433*Size, 0.0);
			glVertex3d(i*sX+Size, j*sY+0.866*Size, 0.0);
			glVertex3d(i*sX+0.5*Size, j*sY+0.866*Size, 0.0);
			glVertex3d(i*sX+0.25*Size, j*sY+0.433*Size, 0.0);
			glVertex3d(i*sX+0.5*Size, j*sY, 0.0);
			glVertex3d(i*sX+Size, j*sY, 0.0);
			glVertex3d(i*sX+1.25*Size, j*sY+0.433*Size, 0.0);
			glEnd();
		}
	}
}

void CVX_Sim::DrawGeometry(int Selected, bool ViewSection, int SectionLayer)
{
	bool DrawInputVoxel = false;
	Vec3D<> Center;
	Vec3D<> tmp(0,0,0);

	int iT = NumVox();
	int x, y, z;
	CColor ThisColor;
	for (int i = 0; i<iT; i++) //go through all the voxels...
	{
		pEnv->pObj->GetXYZNom(&x, &y, &z, StoXIndexMap[i]);
		if (ViewSection && z>SectionLayer) continue; //exit if obscured in a section view!


		Center = VoxArray[i].GetCurPos();

		ThisColor = GetCurVoxColor(i, Selected);
		glColor4d(ThisColor.r, ThisColor.g, ThisColor.b, ThisColor.a);
		

		glPushMatrix();
		glTranslated(Center.x, Center.y, Center.z);

		glLoadName (StoXIndexMap[i]); //to enable picking

		//generate rotation matrix here!!! (from quaternion)
		Vec3D<> Axis;
		vfloat AngleAmt;
		CQuat<>(VoxArray[i].GetCurAngle()).AngleAxis(AngleAmt, Axis);
		glRotated(AngleAmt*180/3.1415926, Axis.x, Axis.y, Axis.z);
	
		Vec3D<> Scale = VoxArray[i].GetCurScale(); //show deformed voxel size
		glScaled(Scale.x, Scale.y, Scale.z);

		//LocalVXC.Voxel.DrawVoxel(&tmp, LocalVXC.Lattice.Lattice_Dim*(1+0.5*CurTemp * pMaterials[CVoxelArray[i].MatIndex].CTE), LocalVXC.Lattice.Z_Dim_Adj);
		LocalVXC.Voxel.DrawVoxel(&tmp, 1); //LocalVXC.GetLatticeDim()); //[i].CurSize.x); //, LocalVXC.Lattice.Z_Dim_Adj);
		
		glPopMatrix();
	}

	if (DrawInputVoxel){
		Vec3D<> tmp(0,0,0);
		Center = VoxArray[NumVox()].GetCurPos();
		glColor4d(1.0, 0.2, 0.2, 1.0);
		glPushMatrix();
		glTranslated(Center.x, Center.y, Center.z);	
		Vec3D<> Scale = LocalVXC.GetLatDimEnv();
		glScaled(Scale.x, Scale.y, Scale.z);

		LocalVXC.Voxel.DrawVoxel(&tmp, 1); //LocalVXC.GetLatticeDim()); //[i].CurSize.x); //, LocalVXC.Lattice.Z_Dim_Adj);
		
		glPopMatrix();
	}
}

CColor CVX_Sim::GetCurVoxColor(int SIndex, int Selected)
{
	if (StoXIndexMap[SIndex] == Selected) return CColor(1.0f, 0.0f, 1.0f, 1.0f); //highlight selected voxel (takes precedence...)

	switch (CurViewCol) {
		case RVC_TYPE:
			float R, G, B, A;
			LocalVXC.GetLeafMat(VoxArray[SIndex].GetVxcIndex())->GetColorf(&R, &G, &B, &A);
			return CColor(R, G, B, A);
			break;
		case RVC_KINETIC_EN:
			if (SS.MaxVoxKinE == 0) return GetJet(0);
			return GetJet(VoxArray[SIndex].GetCurKineticE() / SS.MaxVoxKinE);
			break;
		case RVC_DISP:
			if (SS.MaxVoxDisp == 0) return GetJet(0);
			return GetJet(VoxArray[SIndex].GetCurAbsDisp() / SS.MaxVoxDisp);
			break;
		case RVC_STATE:
			if (VoxArray[SIndex].GetBroken()) return CColor(1.0f, 0.0f, 0.0f, 1.0f);
			else if (VoxArray[SIndex].GetYielded()) return CColor(1.0f, 1.0f, 0.0f, 1.0f);
			else return CColor(1.0f, 1.0f, 1.0f, 1.0f);
			break;
		case RVC_STRAIN_EN:
			if (SS.MaxBondStrainE == 0) return GetJet(0);
			return GetJet(VoxArray[SIndex].GetMaxBondStrainE() / SS.MaxBondStrainE);
			break;
		case RVC_STRAIN:
			if (SS.MaxBondStrain == 0) return GetJet(0);
			return GetJet(VoxArray[SIndex].GetMaxBondStrain() / SS.MaxBondStrain);
			break;
		case RVC_STRESS:
			if (SS.MaxBondStress == 0) return GetJet(0);
			return GetJet(VoxArray[SIndex].GetMaxBondStress() / SS.MaxBondStress);
			break;

	
		default:
			return CColor(1.0f,1.0f,1.0f, 1.0f);
			break;
	}
}

CColor CVX_Sim::GetCurBondColor(int BondIndex)
{
	switch (BondArray[BondIndex].GetBondType()){
		case B_LINEAR:
			switch (CurViewCol) {
				case RVC_TYPE:
					if (BondArray[BondIndex].IsSmallAngle()) return CColor(0.3, 0.7, 0.3, 1.0);
					else return CColor(0.0, 0.0, 0.0, 1.0);
					break;
				case RVC_KINETIC_EN:
					if (SS.MaxVoxKinE == 0) return GetJet(0);
					return GetJet(BondArray[BondIndex].GetMaxVoxKinE() / SS.MaxVoxKinE);
					break;
				case RVC_DISP:
					if (SS.MaxVoxDisp == 0) return GetJet(0);
					return GetJet(BondArray[BondIndex].GetMaxVoxDisp() / SS.MaxVoxDisp);
					break;
				case RVC_STATE:
					if (BondArray[BondIndex].IsBroken()) return CColor(1.0f, 0.0f, 0.0f, 1.0f);
					else if (BondArray[BondIndex].IsYielded()) return CColor(1.0f, 1.0f, 0.0f, 1.0f);
					else return CColor(1.0f, 1.0f, 1.0f, 1.0f);
					break;
				case RVC_STRAIN_EN:
					if (SS.MaxBondStrainE == 0) return GetJet(0);
					return GetJet(BondArray[BondIndex].GetStrainEnergy() / SS.MaxBondStrainE);
					break;
				case RVC_STRAIN:
					if (SS.MaxBondStrain == 0) return GetJet(0);
					return GetJet(BondArray[BondIndex].GetEngStrain() / SS.MaxBondStrain);
					break;
				case RVC_STRESS:
					if (SS.MaxBondStress == 0) return GetJet(0);
					return GetJet(BondArray[BondIndex].GetEngStress() / SS.MaxBondStress);
					break;
	
				default:
					return CColor(0.0f,0.0f,0.0f,1.0f);
					break;
			}
			break;
		case B_LINEAR_CONTACT: {
			if (!SelfColEnabled) return CColor(0.0, 0.0, 0.0, 0.0); //Hide me
			vfloat Force = BondArray[BondIndex].GetForce1().Length(); //check which force to use!
			if (Force == 0.0) return CColor(0.3, 0.3,1.0, 1.0);
			else return CColor(1.0, 0.0, 0.0, 1.0);
			}
		break;

		case B_INPUT_LINEAR_NOROT:
			if (!Dragging) return CColor(0.0, 0.0, 0.0, 0.0); //Hide me
			return CColor(1.0, 0.0, 0.0, 1.0);
		break;
		default:
			return CColor(0.0, 0.0, 0.0, 0.0); //Hide me
			break;

	}
}

void CVX_Sim::DrawSurfMesh(int Selected)
{
	SurfMesh.UpdateMesh(Selected); //updates the generated mesh
	SurfMesh.Draw();
}

void CVX_Sim::DrawVoxMesh(int Selected)
{
	VoxMesh.UpdateMesh(Selected); //updates the generated mesh
	VoxMesh.Draw();
}


void CVX_Sim::DrawBonds(void)
{
	Vec3D<> P1, P2;
	CVXS_Voxel* pV1, *pV2;

	float PrevLineWidth;
	glGetFloatv(GL_LINE_WIDTH, &PrevLineWidth);
	glLineWidth(3.0);
	glDisable(GL_LIGHTING);

	int iT = NumBond();
	glBegin(GL_LINES);
	glLoadName (-1); //to disable picking
	for (int i = 0; i<iT; i++) //go through all the bonds...
	{
		pV1 = BondArray[i].GetpV1(); pV2 = BondArray[i].GetpV2();

		CColor ThisColor = GetCurBondColor(i);
		P1 = pV1->GetCurPos();
		P2 = pV2->GetCurPos();

		glColor4f(ThisColor.r, ThisColor.g, ThisColor.b, ThisColor.a);
		//TODO:sweet curved bonds!
		//if (CurViewVox == RVV_SMOOTH){
		//	CQuat A1 = pV1->GetCurAngle();
		//	CQuat A2 = pV1->GetCurAngle();
		//}
		//else {
			if (ThisColor.a != 0.0) {glVertex3f((float)P1.x, (float)P1.y, (float)P1.z); glVertex3f((float)P2.x, (float)P2.y, (float)P2.z);}
//		}
	}
	glEnd();


	Vec3D<> Center;
	iT = NumVox();
	glPointSize(5.0);
	Vec3D<> tmp(0,0,0);
	for (int i = 0; i<iT; i++) //go through all the voxels...
	{
		//mostly copied from Voxel drawing function!
		Center = VoxArray[i].GetCurPos();
		glColor4d(0.2, 0.2, 0.2, 1.0);
		glLoadName (StoXIndexMap[i]); //to enable picking

		glPushMatrix();
		glTranslated(Center.x, Center.y, Center.z);
		glLoadName (StoXIndexMap[i]); //to enable picking

		//generate rotation matrix here!!! (from quaternion)
		Vec3D<> Axis;
		vfloat AngleAmt;
		CQuat<>(VoxArray[i].GetCurAngle()).AngleAxis(AngleAmt, Axis);
		glRotated(AngleAmt*180/3.1415926, Axis.x, Axis.y, Axis.z);
	
		Vec3D<> Scale = VoxArray[i].GetCurScale(); //show deformed voxel size
		glScaled(Scale.x, Scale.y, Scale.z);

		//LocalVXC.Voxel.DrawVoxel(&tmp, LocalVXC.Lattice.Lattice_Dim*(1+0.5*CurTemp * pMaterials[CVoxelArray[i].MatIndex].CTE), LocalVXC.Lattice.Z_Dim_Adj);
		LocalVXC.Voxel.DrawVoxel(&tmp, 0.2); //LocalVXC.GetLatticeDim()); //[i].CurSize.x); //, LocalVXC.Lattice.Z_Dim_Adj);
		
		glPopMatrix();
	}


	glLineWidth(PrevLineWidth);
	glEnable(GL_LIGHTING);

}

void CVX_Sim::DrawAngles(void)
{
	//draw directions
	float PrevLineWidth;
	glGetFloatv(GL_LINE_WIDTH, &PrevLineWidth);
	glLineWidth(2.0);
	glDisable(GL_LIGHTING);

	glBegin(GL_LINES);

	for (int i = 0; i<NumVox(); i++){ //go through all the voxels... (GOOD FOR ONLY SMALL DISPLACEMENTS, I THINK... think through transformations here!)
		glColor3f(1,0,0); //+X direction
		glVertex3d(VoxArray[i].GetCurPos().x, VoxArray[i].GetCurPos().y, VoxArray[i].GetCurPos().z);
		Vec3D<> Axis1(LocalVXC.GetLatticeDim()/4,0,0);
		Vec3D<> RotAxis1 = (VoxArray[i].GetCurAngle()*CQuat<>(Axis1)*VoxArray[i].GetCurAngle().Conjugate()).ToVec();
		glVertex3d(VoxArray[i].GetCurPos().x + RotAxis1.x, VoxArray[i].GetCurPos().y + RotAxis1.y, VoxArray[i].GetCurPos().z + RotAxis1.z);

		glColor3f(0,1,0); //+Y direction
		glVertex3d(VoxArray[i].GetCurPos().x, VoxArray[i].GetCurPos().y, VoxArray[i].GetCurPos().z);
		Axis1 = Vec3D<>(0, LocalVXC.GetLatticeDim()/4,0);
		RotAxis1 = (VoxArray[i].GetCurAngle()*CQuat<>(Axis1)*VoxArray[i].GetCurAngle().Conjugate()).ToVec();
		glVertex3d(VoxArray[i].GetCurPos().x + RotAxis1.x, VoxArray[i].GetCurPos().y + RotAxis1.y, VoxArray[i].GetCurPos().z + RotAxis1.z);

		glColor3f(0,0,1); //+Z direction
		glVertex3d(VoxArray[i].GetCurPos().x, VoxArray[i].GetCurPos().y, VoxArray[i].GetCurPos().z);
		Axis1 = Vec3D<>(0,0, LocalVXC.GetLatticeDim()/4);
		RotAxis1 = (VoxArray[i].GetCurAngle()*CQuat<>(Axis1)*VoxArray[i].GetCurAngle().Conjugate()).ToVec();
		glVertex3d(VoxArray[i].GetCurPos().x + RotAxis1.x, VoxArray[i].GetCurPos().y + RotAxis1.y, VoxArray[i].GetCurPos().z + RotAxis1.z);

	}
	glEnd();

	glLineWidth(PrevLineWidth);
	glEnable(GL_LIGHTING);
}

void CVX_Sim::DrawStaticFric(void)
{
	//draw triangle for points that are stuck via static friction
	glBegin(GL_TRIANGLES);
	glColor4f(255, 255, 0, 1.0);
	vfloat dist = VoxArray[0].GetOrigSize().x/3; //needs work!!
	int iT = NumVox();
	Vec3D<> P1;
	for (int i = 0; i<iT; i++){ //go through all the voxels...
		if (VoxArray[i].GetCurStaticFric()){ //draw point if static friction...
			P1 = VoxArray[i].GetCurPos();
			glVertex3f((float)P1.x, (float)P1.y, (float)P1.z); 
			glVertex3f((float)P1.x, (float)(P1.y - dist/2), (float)(P1.z + dist));
			glVertex3f((float)P1.x, (float)(P1.y + dist/2), (float)(P1.z + dist));
		}
	}
	glEnd();
}

char CVX_Sim::StatRqdToDraw() //returns the stats bitfield that we need to calculate to draw the current view.
{
	if (CurViewMode == RVM_NONE) return CALCSTAT_NONE;
	switch (CurViewCol){
	case RVC_KINETIC_EN: return CALCSTAT_KINE; break;
	case RVC_DISP: return CALCSTAT_DISP; break;
	case RVC_STRAIN_EN: return CALCSTAT_STRAINE; break;
	case RVC_STRAIN: return CALCSTAT_ENGSTRAIN; break;
	case RVC_STRESS: return CALCSTAT_ENGSTRESS; break;
	default: return CALCSTAT_NONE;
	}
}

#endif //OPENGL

void CVX_Sim::CalcL1Bonds(vfloat Dist) //creates contact bonds for all voxels within specified distance
{
//	Dist = Dist*LocalVXC.GetLatticeDim();
	vfloat Dist2;

	DeleteTmpBonds();

	if (CurColSystem == COL_SURFACE || COL_SURFACE_HORIZON){
		//clear bond array past the m_NumBonds position
		for (int i=0; i<NumSurfVoxels(); i++){ //go through each combination of surface voxels...
			int SIndex1 = SurfVoxels[i];

			for (int j=i+1; j<NumSurfVoxels(); j++){
				int SIndex2 = SurfVoxels[j];
				if (!VoxArray[SIndex1].IsNearbyVox(SIndex2)){
					vfloat ActDist = Dist*(VoxArray[SIndex1].GetCurScale().x + VoxArray[SIndex1].GetCurScale().x)/2; //ASSUMES ISOTROPIC!!

					Dist2 = (VoxArray[SIndex1].GetCurPos() - VoxArray[SIndex2].GetCurPos()).Length2();
					if (Dist2 < ActDist*ActDist){ //if within the threshold
						CreateBond(B_LINEAR_CONTACT, SIndex1, SIndex2); //create temporary bond...	
					}

				}
			}
		}
	}
	else { //check against all!
		for (int i=0; i<NumVox(); i++){ //go through each combination of voxels...
			int SIndex1 = i;

			for (int j=i+1; j<NumVox(); j++){
				int SIndex2 = j;
				if (!VoxArray[SIndex1].IsNearbyVox(SIndex2)){

					vfloat ActDist = Dist*(VoxArray[SIndex1].GetCurScale().x + VoxArray[SIndex1].GetCurScale().x)/2; //ASSUMES ISOTROPIC!!
					Dist2 = (VoxArray[SIndex1].GetCurPos() - VoxArray[SIndex2].GetCurPos()).Length2();
					if (Dist2 < ActDist*ActDist){ //if within the threshold
						CreateBond(B_LINEAR_CONTACT, SIndex1, SIndex2); //create temporary bond...	
					}

				}
			}
		}
	}

	//todo: check for reallocation
	UpdateAllBondPointers();
}

Vec3D<> CVX_Sim::GetCM(void)
{

	vfloat TotalMass = 0;
	Vec3D<> Sum(0,0,0);
	int nVox = NumVox();
	for (int i=0; i<nVox; i++){
		if (i==InputVoxSInd) continue;
		CVXS_Voxel* it = &VoxArray[i];
		vfloat ThisMass = it->GetMass();
		Sum += it->GetCurPos()*ThisMass;
		TotalMass += ThisMass;
	}

	return Sum/TotalMass;
}

int CVX_Sim::GetNumTouchingFloor()
{
	int NumTouching = 0;

	int LocNumVox = NumVox();
	for (int i=0; i<LocNumVox; i++){
		if (VoxArray[i].GetCurGroundPenetration() > 0) NumTouching++;
	}
	return NumTouching;
}

bool CVX_Sim::KineticEDecreasing(void)
{
	 if (KinEHistory[0]+KinEHistory[1]+KinEHistory[2] < KinEHistory[3]+KinEHistory[4]+KinEHistory[5]) return true;
	 else return false;
}

Vec3D<> CVX_Sim::GetSumForce(CVX_FRegion* pRegion)
{
	Vec3D<> ForceSum(0,0,0);

	Vec3D<> BCpoint;
	Vec3D<> BCsize = pEnv->pObj->GetLatDimEnv()/2.0;
	Vec3D<> WSSize = pEnv->pObj->GetWorkSpace();

	int NumVoxS = NumVox();
	for (int i=0; i<NumVoxS; i++){
		BCpoint = pEnv->pObj->GetXYZ(StoXIndexMap[i]);
		if (pRegion->GetRegion()->IsTouching(&BCpoint, &BCsize, &WSSize)){
			ForceSum += VoxArray[i].CalcTotalForce(false);
		}
	}

	return ForceSum;
}

vfloat CVX_Sim::GetSumForceDir(CVX_FRegion* pRegion)
{
	//right now only fixed regions... (forced regions should be zero!)
	//get force only in dircetion of pull!
	Vec3D<> Res = GetSumForce(pRegion);

	Vec3D<> Dir = pRegion->Displace;
	if (Dir.Length2() == 0) return Res.Length(); //return magnitude of no direction...
	else {
		Dir.Normalize();
		return Res.Dot(Dir);
	}

}

int CVX_Sim::NumYielded(void)
{
	int NumYieldRet = 0;
	int NumBondNow = (int)(BondArray.size());
	for (int i=0; i<NumBondNow; i++)
		if (BondArray[i].IsYielded()) NumYieldRet++;

	return NumYieldRet;
}

int CVX_Sim::NumBroken(void)
{
	int NumBrokenRet = 0;
	int NumBondNow = (int)(BondArray.size());
	for (int i=0; i<NumBondNow; i++)
		if (BondArray[i].IsBroken()) NumBrokenRet++;

	return NumBrokenRet;

}