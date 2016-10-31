/*******************************************************************************
Copyright (c) 2010, Jonathan Hiller (Cornell University)
If used in publication cite "J. Hiller and H. Lipson "Dynamic Simulation of Soft Heterogeneous Objects" In press. (2011)"

This file is part of Voxelyze.
Voxelyze is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
Voxelyze is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
See <http://www.opensource.org/licenses/lgpl-3.0.html> for license details.
*******************************************************************************/

#ifndef VXS_VOXEL_H
#define VXS_VOXEL_H

#include "Utils/Vec3D.h"
#include <vector>

//http://gafferongames.com/game-physics/physics-in-3d/

//Maximum number of bonds to a given voxel (including temporary contact bonds...)
#define NUMVBOND 50

class CVXS_Bond;
class CVX_Sim;
class CVXC_Material;
//class CQuat;

struct VoxState {
	VoxState(void) {Pos = Vec3D<double>(0,0,0); LinMom = Vec3D<double>(0,0,0); Angle = CQuat<double>(1.0, 0, 0, 0); AngMom = Vec3D<double>(0,0,0); Scale = Vec3D<>(0,0,0); ScaleMom = Vec3D<>(0,0,0); Vel = Vec3D<>(0,0,0); KineticEnergy = 0; /*PotentialEnergy = 0;*/ AngVel = Vec3D<>(0,0,0); ScaleVel = Vec3D<>(0,0,0); ElectricallyActiveOld = false; MembranePotentialOld = 0.0; ElectricallyActiveNew = false; MembranePotentialNew = 0.0; RepolarizationStartTime = -99999.0;};
	VoxState(const VoxState& SIn) {*this = SIn;} //copy constructor
	inline VoxState& operator=(const VoxState& SIn) {Pos = SIn.Pos; LinMom = SIn.LinMom; Angle = SIn.Angle; AngMom = SIn.AngMom; Scale = SIn.Scale; ScaleMom = SIn.ScaleMom; Vel = SIn.Vel; KineticEnergy=SIn.KineticEnergy; /*PotentialEnergy=SIn.PotentialEnergy;*/ AngVel = SIn.AngVel; ScaleVel = SIn.ScaleVel; ElectricallyActiveOld = SIn.ElectricallyActiveOld; MembranePotentialOld = SIn.MembranePotentialOld; ElectricallyActiveNew = SIn.ElectricallyActiveNew; MembranePotentialNew = SIn.MembranePotentialNew; return *this; }; //overload equals

	Vec3D<double> Pos; //translation
	Vec3D<double> LinMom;

	CQuat<double> Angle; //rotation
	Vec3D<double> AngMom;
		
	Vec3D<> Scale; 
	Vec3D<> ScaleMom; //Scaling

	//convenience derived secondary quantities:
	Vec3D<> Vel;
	vfloat KineticEnergy;
//	vfloat PotentialEnergy;
	Vec3D<> AngVel;
	Vec3D<> ScaleVel;

	// Electrical Activity:
	bool ElectricallyActiveOld;
	float MembranePotentialOld;
	bool ElectricallyActiveNew;
	float MembranePotentialNew;
	float RepolarizationStartTime;

};

struct dVoxState { //derivative of voxel state
	dVoxState(void) {LinMom = Vec3D<>(0,0,0); Force = Vec3D<>(0,0,0); AngMom = Vec3D<>(0,0,0); Moment = Vec3D<>(0,0,0); ScaleMom = Vec3D<>(0,0,0); ScaleForce = Vec3D<>(0,0,0);};
	dVoxState(const dVoxState& dSIn) {*this = dSIn;} //copy constructor
	inline dVoxState& operator=(const dVoxState& dSIn) {LinMom = dSIn.LinMom; Force = dSIn.Force; AngMom = dSIn.AngMom; Moment = dSIn.Moment; ScaleMom = dSIn.ScaleMom; ScaleForce = dSIn.ScaleForce; return *this; }; //overload equals

	Vec3D<> LinMom;
	Vec3D<> Force;

	Vec3D<> AngMom;
	Vec3D<> Moment;
	
	Vec3D<> ScaleMom;
	Vec3D<> ScaleForce;
};


class CVXS_Voxel
{
public:
	//CVXS_Voxel(CVX_Sim* pSimIn, int SIndexIn, int XIndexIn, int MatIndexIn, Vec3D<>& OriginalPosIn, Vec3D<>& OriginalScaleIn);
	CVXS_Voxel(CVX_Sim* pSimIn, int SIndexIn, int XIndexIn, int MatIndexIn, Vec3D<>/*(nac)*/ OriginalPosIn, Vec3D<>/*(nac)*/ OriginalScaleIn);
public:
	~CVXS_Voxel(void);

	CVXS_Voxel(const CVXS_Voxel& VIn) {*this = VIn;} //copy constructor
	CVXS_Voxel& operator=(const CVXS_Voxel& VIn);

	void RelinkToVXSim(CVX_Sim* pSimIn) {p_Sim = pSimIn;} //link this voxel back to the simulator
	void ResetVoxel(void); //resets this voxel to its defualt (imported) state.

	inline void AddExternalForce(const Vec3D<>& ForceIn) {ExternalForce += ForceIn;}
	inline void AddExternalTorque(const Vec3D<>& TorqueIn) {ExternalTorque += TorqueIn;}
	inline void SetExternalDisp(const Vec3D<>& DispIn) {ExternalDisp = DispIn;}
	inline void SetExternalTDisp(const Vec3D<>& TDispIn) {ExternalTDisp = TDispIn;}
	
	void EulerStep();
	void VerletStep(); //Experimental. Not yet ready.

	inline int GetVxcIndex(void) const {return MyXIndex;}
	inline int GetSimIndex(void) const {return MySIndex;}

	const inline bool GetElectricallyActiveOld(void) const {return S.ElectricallyActiveOld;}
	const inline float GetMembranePotentialOld(void) const {return S.MembranePotentialOld;}
	const inline bool GetElectricallyActiveNew(void) const {return S.ElectricallyActiveNew;}
	const inline float GetMembranePotentialNew(void) const {return S.MembranePotentialNew;}
	const inline float GetRepolarizationStartTime(void) const {return S.RepolarizationStartTime;}

	const inline void SetElectricallyActiveOld(const bool _ElectricallyActiveOld)  {S.ElectricallyActiveOld = _ElectricallyActiveOld;}
	const inline void SetMembranePotentialOld(const float _MembranePotentialOld) {S.MembranePotentialOld = _MembranePotentialOld;}
	const inline void SetElectricallyActiveNew(const bool _ElectricallyActiveNew) {S.ElectricallyActiveNew = _ElectricallyActiveNew;}
	const inline void SetMembranePotentialNew(const float _MembranePotentialNew) {S.MembranePotentialNew = _MembranePotentialNew;}
	const inline void SetRepolarizationStartTime(const float _RepolarizationStartTime)  {S.RepolarizationStartTime = _RepolarizationStartTime;}

	const inline Vec3D<> GetCurPos(void) const {return S.Pos;}
	const inline Vec3D<double> GetCurPosHighAccuracy(void) const {return S.Pos;}
	const inline CQuat<> GetCurAngle(void) const {return S.Angle;}
	const inline CQuat<double> GetCurAngleHighAccuracy(void) const {return S.Angle;}
	const inline Vec3D<> GetCurScale(void) const {return S.Scale;}
	const inline Vec3D<> GetCurVel(void) const {return S.Vel;}
	const inline Vec3D<> GetCurAngVel(void) const {return S.AngVel;}
	const inline Vec3D<> GetCurForce(void) const {return dS.Force;}
	const inline vfloat GetCurKineticE(void) const {return S.KineticEnergy;}
//	const inline vfloat GetCurPotentialE(void) const {return S.PotentialEnergy;}
	const inline bool GetCurStaticFric(void) const {return StaticFricFlag;}
	const inline vfloat GetCurAbsDisp(void) const {return (Vec3D<>(S.Pos)-OriginalPos).Length();}
	inline void ZeroMotion(void) {S.LinMom = Vec3D<double>(0,0,0); S.AngMom = Vec3D<double>(0,0,0); S.Vel = Vec3D<>(0,0,0); S.AngVel = Vec3D<>(0,0,0); S.KineticEnergy = 0;}

	inline vfloat GetMass(void) const {return Mass;}
	inline vfloat GetFirstMoment(void) const {return FirstMoment;}
	inline vfloat GetInertia(void) const {return Inertia;}

	inline Vec3D<> GetOrigPos(void) const {return OriginalPos;}
	inline Vec3D<> GetOrigSize(void) const {return OriginalSize;}
	

	inline void OverridePos(const Vec3D<>& NewPos) {S.Pos = NewPos;}
	inline void ScaleExternalInputs(const vfloat ScaleFactor=1.0) {ExtInputScale=ScaleFactor;} //scales force, torque, etc. to some percentage of its set value

	bool SetMaterial(const int MatIndexIn); //sets the material according to VX_Object material index
	int GetMaterial(void) {return MatIndex;} //returns the material index for VX_Object
	inline void SetYielded(const bool Yielded) {VYielded = Yielded;}
	inline bool GetYielded(void) const {return VYielded;}
	inline void SetBroken(const bool Broken) {VBroken = Broken;}
	inline bool GetBroken(void) const {return VBroken;}

	float UpdateElectricalActivationFromNeighbors(void);

	Vec3D<> CalcTotalForce(bool WithRestraint = true); //calculates InternalForce. If WithRestraint, includes external (to ground) restraint force from a fixed voxel. THis allows calculating internal (structure) force on a constrained region
	Vec3D<> CalcTotalMoment(void); //Calculates TotalMoment
	Vec3D<> CalcTotalScaleForce(void); //calculate Scaling force


	//Display color stuff
	void SetColor(float r, float g, float b, float a);

	void FixDof(char DofFixedIn); //fixes any of the degrees of freedom indicated. Doesn't unfix any currently fixed ones
	inline char GetDofFixed(void) const {return DofFixed;}

	vfloat CalcVoxMatStress(const vfloat StrainIn, bool* const IsPastYielded, bool* const IsPastFail) const;
	vfloat GetEMod(void); //gets elastic modulus, taking blending into account
	vfloat GetPoisson(void); //gets poisson ratio, taking blending into account
	vfloat GetCTE(void); //gets CTE, taking blending into account

	//keep track of the bonds this voxel is participating in...
	bool LinkBond(int SBondIndex); //simulation bond index...
	void UnlinkBond(int SBondIndex); //removes this bond from the pointers (if it exists), even if permanent bond
	bool UpdateBondLinks(void); //updates all links (pointers) to bonds!
	void UnlinkAllBonds() {BondInds.clear(); BondPointers.clear(); NumLocalBonds = 0;} 
	inline CVXS_Bond* GetBond(const int LocalBondIndex) const {return BondPointers[LocalBondIndex];}
	inline int GetBondIndex(const int LocalBondIndex) const {return BondInds[LocalBondIndex];}
	inline int GetNumLocalBonds() const {return NumLocalBonds;}
	void UnlinkTmpBonds(void); //(assumes all perm at beginning...) removes any temprary bonds from our local list...

	void CalcMyBlendMix(); //requires p_sim, p_Sim->LocalVXC, and MyXIndex to already be in place...

	//collision information
	void CalcNearby(int NumHops); //populates how close of voxels to ignore in the lattice
	inline bool IsNearbyVox(int GlobalRVoxInd) {for (int i=0; i<NumNearbyVox(); i++) if (NearbyVoxInds[i] == GlobalRVoxInd) return true; return false;};
	inline bool IsMe(const CVXS_Voxel* pCompare) const {return pCompare == this;} //if (pCompare->MyXIndex == MyXIndex) return true; else return false;};

	vfloat GetMaxBondStrain(void) const;
	vfloat GetMaxBondStrainE(void) const;
	vfloat GetMaxBondStress(void) const;
	
	vfloat GetCurGroundPenetration(); //how far into the ground penetrating (penetration is positive, no penetration is zero)

	int GetMuscleType();
	int GetSensorType();

	vfloat GetTimeOfLastMuscleUpdate(void) const {return timeOfLastMuscleUpdate;}
	vfloat SetTimeOfLastMuscleUpdate(const vfloat newTime) {timeOfLastMuscleUpdate = newTime;}


private:
	CVX_Sim* p_Sim;

	vfloat timeOfLastMuscleUpdate;

	//const these?
	//simulation info
	int MyXIndex; //index in the original VXC Array
	int MySIndex; //index in the Relaxation voxel Array
	Vec3D<> OriginalPos; //Original position upon import. Never change this, as it is needed for resetting/bond directions
	Vec3D<> OriginalSize; //Original size upon import. Never change this.
	int MatIndex; //Material associated with this voxel (pre blending)
	CVXC_Material* _pMat; //cached pointer to material.

	//Inputs affecting this local voxel
	char DofFixed;
	Vec3D<> ExternalForce; //External force applied to this voxel in N if relevant DOF are unfixed
	Vec3D<> ExternalDisp; //Prescribed displaced position in meters, if DOF is fixed
	Vec3D<> ExternalTorque; //External torque applied to this voxel in N-m if relevant DOF are unfixed
	Vec3D<> ExternalTDisp; //Prescribed displaced in radians, if DOF is fixed
	vfloat ExtInputScale; //Scales the external Force, displacement, torque (range: 0-1)

	//Current status of this voxel:
	VoxState S;
	VoxState PrevS; //needed for Verlet...
	dVoxState dS; //needed for RK4

//	vfloat MaxBondStrain, MaxBondStress;
	bool StaticFricFlag; //flag to set if this voxel shouldnot move in X/Y due to being in static friction regime
	bool VYielded, VBroken;

	//Other static info about this voxel: (set via SetMaterial())
	vfloat Mass; //The mass of this voxel
	vfloat FirstMoment; //1st moment "inertia" (needed for certain
	vfloat Inertia; //rotational mass
	vfloat ScaleMass; //scaling mass
	vfloat _massInv; //1/Mass
	vfloat _inertiaInv; //1/Inertia
	vfloat _2xSqMxExS;
	vfloat _2xSqIxExSxSxS;

//	void UpdateDState(); //calculates the derivative of our current state "S". All bonds should be updated before this!
//	void UpdateState(vfloat dt, dVoxState& Derivative); //advances this state "S" with previously calculated dS by dt

	int NumNearbyVox(void){return (int)NearbyVoxInds.size();};
	CVXS_Voxel* pNearbyVox(int LocalNearbyInd);

	std::vector<int> NearbyVoxInds;

	bool BlendingEnabled;
	std::vector<vfloat> BlendMix; //array with size equal to number of materials in Palette. Contains percentage of each material. All numbers should add up to 1.


	//Other effects that might have soemthing to do with force calculation
	Vec3D<> CalcFloorEffect(Vec3D<> TotalVoxForce); //calculates the object's interaction with a floor under gravity
	Vec3D<> CalcGndDampEffect(); //damps everything to ground as quick as possible...

	//cached info on any bonds involving this voxel
	std::vector<int> BondInds;
	std::vector<CVXS_Bond*> BondPointers;
	int NumLocalBonds; //tracks the number of bonds in BondInds[]. 

	//Current display color of this voxel.
	float m_Red, m_Green, m_Blue, m_Trans; //can update voxel color based on state, mode, etc.
//	vfloat _TwoxSqrtA1xMass;

};

#endif //VXS_VOXEL_H