/*******************************************************************************
Copyright (c) 2010, Jonathan Hiller (Cornell University)
If used in publication cite "J. Hiller and H. Lipson "Dynamic Simulation of Soft Heterogeneous Objects" In press. (2011)"

This file is part of Voxelyze.
Voxelyze is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
Voxelyze is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
See <http://www.opensource.org/licenses/lgpl-3.0.html> for license details.
*******************************************************************************/

#include "VXS_Bond.h"
#include "VXS_Voxel.h"
#include "VX_Sim.h"
//#include "RelaxMaterial.h"

CVXS_Bond::CVXS_Bond(CVX_Sim* p_SimIn)
{
	p_Sim = p_SimIn;
	ThisBondType = B_LINEAR;
	Perm = false;

	ResetBond(); //Zeroes out all state variables

	//Set independent variables
	Vox1SInd = -1;
	Vox2SInd = -1;
	pVox1 = NULL;
	pVox2 = NULL;
	OrigDist = Vec3D<>(0,0,0);
	HomogenousBond = false;
	ThisBondDir = BD_X;
	E=0; u=0; CTE=0;
	L = Vec3D<>(0, 0, 0);
	
	UpdateConstants(); //updates all the dependent variables based on zeros above.
}

CVXS_Bond::~CVXS_Bond(void)
{
}

CVXS_Bond& CVXS_Bond::operator=(const CVXS_Bond& Bond)
{
	//Bond definition
	p_Sim = Bond.p_Sim;
	ThisBondType = Bond.ThisBondType;
	Perm = Bond.Perm;

	//State variables
	Force1 = Bond.Force1;
	Force2 = Bond.Force2;
	Moment1 = Bond.Moment1;
	Moment2 = Bond.Moment2;
	StrainEnergy = Bond.StrainEnergy;
	SmallAngle = Bond.SmallAngle;
	_Pos2=Bond._Pos2; 
	_Angle1=Bond._Angle1;
	_Angle2=Bond._Angle2;
	_LastPos2=Bond._LastPos2; 
	_LastAngle1=Bond._LastAngle1;
	_LastAngle2=Bond._LastAngle2; 
	CurStrain = Bond.CurStrain;
	CurStress = Bond.CurStress;
	MaxStrain = Bond.MaxStrain;
	Yielded = Bond.Yielded;
	Broken = Bond.Broken;
	RestDist = Bond.RestDist;

	Vox1SInd = Bond.Vox1SInd;
	Vox2SInd = Bond.Vox2SInd;
	if (!UpdateVox1Ptr()){Vox1SInd=-1;}
	if (!UpdateVox2Ptr()){Vox2SInd=-1;}
	OrigDist = Bond.OrigDist;
	HomogenousBond = Bond.HomogenousBond;
	ThisBondDir = Bond.ThisBondDir;
	L = Bond.L;
	E = Bond.E;
	u = Bond.u;
	CTE = Bond.CTE;

	UpdateConstants();

	return *this;
}

bool CVXS_Bond::DefineBond(BondType BondTypeIn, int Vox1SIndIn, int Vox2SIndIn, bool PermIn) //usually only called whenn creating bond...
{
	// std::cout << "DEBUG MALLOC: GOT HERE -- BOND." <<  std::endl;
	ThisBondType = BondTypeIn;
	Perm = PermIn; 
	if (!SetVoxels(Vox1SIndIn, Vox2SIndIn)) return false;
	if (!UpdateConstants()) return false;
	ResetBond(); //??

	return true;
}

void CVXS_Bond::ResetBond(void) //resets this voxel to its default (imported) state.
{
	Force1 = Vec3D<>(0,0,0);
	Force2 = Vec3D<>(0,0,0);
	Moment1 = Vec3D<>(0,0,0);
	Moment2 = Vec3D<>(0,0,0);
	StrainEnergy = 0; 
	SmallAngle = true;

	CurStrain = 0;
	CurStress = 0;
	MaxStrain = 0;

	Yielded = false;
	Broken = false;
	RestDist = OrigDist;

	_Pos2 = Vec3D<>(0,0,0);
	_Angle1 = Vec3D<>(0,0,0);
	_Angle2 = Vec3D<>(0,0,0);
	_LastPos2 = Vec3D<>(0,0,0);
	_LastAngle1 = Vec3D<>(0,0,0);
	_LastAngle2 = Vec3D<>(0,0,0);

}


void CVXS_Bond::UpdateBond() //calculates force, positive for tension, negative for compression
{
	switch (ThisBondType){
	case B_LINEAR: CalcLinForce(); break;
	case B_LINEAR_CONTACT: CalcContactForce(); break;
	case B_INPUT_LINEAR_NOROT: if (p_Sim->Dragging) CalcSimpleSpringForce(); else {Force1 = Vec3D<>(0,0,0); Force2 = Vec3D<>(0,0,0); Moment1 = Vec3D<>(0,0,0); Moment2 = Vec3D<>(0,0,0);} break;
	default: {}
	}
}

vfloat CVXS_Bond::GetMaxVoxKinE(){
	vfloat Ke1 = pVox1->GetCurKineticE(), Ke2 = pVox2->GetCurKineticE();
	return Ke1>Ke2?Ke1:Ke2;
}

vfloat CVXS_Bond::GetMaxVoxDisp(){
	vfloat D1 = pVox1->GetCurAbsDisp(), D2 = pVox2->GetCurAbsDisp();
	return D1>D2?D1:D2;
}

template <typename T> void CVXS_Bond::ToXDirBond(Vec3D<T>* const pVec) const //transforms a vec3D in the original orientation of the bond to that as if the bond was in +X direction
{
	switch (ThisBondDir){
	case BD_X: return;
	case BD_Y: {T tmp = pVec->x; pVec->x=pVec->y; pVec->y = -tmp; return;}
	case BD_Z: {T tmp = pVec->x; pVec->x=pVec->z; pVec->z = -tmp; return;}
	case BD_ARB:
		//ElRot quaternion transform
		return;
	default: return;
	}
}

template <typename T> void CVXS_Bond::ToXDirBond(CQuat<T>* const pQuat) const
{
	switch (ThisBondDir){
	case BD_X: return;
	case BD_Y: {T tmp = pQuat->x; pQuat->x=pQuat->y; pQuat->y = -tmp; return;}
	case BD_Z: {T tmp = pQuat->x; pQuat->x=pQuat->z; pQuat->z = -tmp; return;}
	case BD_ARB:
		//ElRot quaternion transform
		return;
	default: return;
	}
}

template <typename T> void CVXS_Bond::ToOrigDirBond(Vec3D<T>* const pVec) const
{
	switch (ThisBondDir){
	case BD_X: return;
	case BD_Y: {vfloat tmp = pVec->y; pVec->y=pVec->x; pVec->x = -tmp; return;}
	case BD_Z: {vfloat tmp = pVec->z; pVec->z=pVec->x; pVec->x = -tmp; return;}
	case BD_ARB:
		//ElRot quaternion transform
		return;
	default: return;
	}
}

template <typename T> void CVXS_Bond::ToOrigDirBond(CQuat<T>* const pVec) const
{
	switch (ThisBondDir){
	case BD_X: return;
	case BD_Y: {vfloat tmp = pVec->y; pVec->y=pVec->x; pVec->x = -tmp; return;}
	case BD_Z: {vfloat tmp = pVec->z; pVec->z=pVec->x; pVec->x = -tmp; return;}
	case BD_ARB:
		//ElRot quaternion transform
		return;
	default: return;
	}
}

void CVXS_Bond::SetYielded(void)
{
	Yielded = true;
	pVox1->SetYielded(true);
	pVox2->SetYielded(true);
}

void CVXS_Bond::SetBroken(void)
{
	if (p_Sim->IsFailureEnabled()){
		Broken = true; 
		pVox1->SetBroken(true);
		pVox2->SetBroken(true);
	}
}

//sub force calculation types...
void CVXS_Bond::CalcLinForce() //get bond forces given positions, angles, and stiffnesses...
{
	Vec3D<double> CurXRelPos(pVox2->GetCurPosHighAccuracy() - pVox1->GetCurPosHighAccuracy()); //digit truncation happens here...
	CQuat<double> CurXAng1(pVox1->GetCurAngleHighAccuracy());
	CQuat<double> CurXAng2(pVox2->GetCurAngleHighAccuracy()); 
	ToXDirBond(&CurXRelPos);
	ToXDirBond(&CurXAng1);
	ToXDirBond(&CurXAng2);
	
	Vec3D<double> Ang1AlignedRelPos(CurXAng1.RotateVec3DInv(CurXRelPos)); //undo current voxel rotation to put in line with original bond according to Angle 1
	CQuat<double> NewAng2(CurXAng1.Conjugate()*CurXAng2); 

	bool ChangedSaState = false;
	vfloat SmallTurn = (abs(Ang1AlignedRelPos.z)+abs(Ang1AlignedRelPos.y))/Ang1AlignedRelPos.x;
	if (!SmallAngle && NewAng2.IsSmallAngle() && SmallTurn < SA_BOND_BEND_RAD){ SmallAngle = true; ChangedSaState=true;}
	else if (SmallAngle && !NewAng2.IsSmallishAngle() && SmallTurn > VEC3D_HYSTERESIS_FACTOR*SA_BOND_BEND_RAD){SmallAngle = false; ChangedSaState=true;}

	double NomDistance = (pVox1->GetCurScale().x + pVox2->GetCurScale().x)*0.5; //nominal distance between voxels
	CQuat<> TotalRot;

	if (SmallAngle)	{ //Align so Angle1 is all zeros
		_Angle1 = Vec3D<>(0,0,0);
		_Angle2 = NewAng2.ToRotationVector();
		Ang1AlignedRelPos.x -= NomDistance; //only valid for small angles
		_Pos2 = Ang1AlignedRelPos;
		TotalRot = CurXAng1.Conjugate();

	}
	else { //Large angle. Align so that Pos2.y, Pos2.z are zero.
		CQuat<double> Pos2AlignedRotAng;
		Pos2AlignedRotAng.FromAngleToPosX(Ang1AlignedRelPos); //get the angle to align this with the X axis
		TotalRot = Pos2AlignedRotAng * CurXAng1.Conjugate();

		vfloat Length = CurXRelPos.Length(); //Ang1AlignedRelPos.x<0 ? -Ang1AlignedRelPos.Length() : Ang1AlignedRelPos.Length();
		_Pos2 = Vec3D<>(Length - NomDistance, 0, 0); //Small angle optimization target!!
//		Vec3D<> Pos2a = Pos2AlignedRotAng.RotateVec3D(Ang1AlignedRelPos); //high performance (but slow version) for special cases. should never crop up. (i.e. 1dof sim dragging voxel past each other)
//		_Pos2 = Vec3D<>(Pos2a.x - NomDistance, 0, 0); 
		
		_Angle1 = Pos2AlignedRotAng.ToRotationVector(); //these are currently a source of error of up to 1e-6
		_Angle2 = (TotalRot * CurXAng2).ToRotationVector();
	}

	UpdateBondStrain(_Pos2.x/L.x); //updates the bond parameters (yielded, broken, stress...) based on the current Strain

	//Beam equations! (all terms here, even though some are sero for small angle and large angle (negligible perfoprmance penalty)
	Force1 = Vec3D<> (	-CurStress*L.y*L.z,				-b1z*_Pos2.y + b2z*(_Angle1.z + _Angle2.z),		-b1y*_Pos2.z - b2y*(_Angle1.y + _Angle2.y)); //Use Curstress instead of -a1*Pos2.x to account for non-linear deformation 
	Force2 = -Force1;
	Moment1 = Vec3D<> (	a2*(_Angle1.x - _Angle2.x),		b2z*_Pos2.z + b3y*(2*_Angle1.y + _Angle2.y),	-b2y*_Pos2.y + b3z*(2*_Angle1.z + _Angle2.z));
	Moment2 = Vec3D<> (	a2*(_Angle2.x - _Angle1.x),		b2z*_Pos2.z + b3y*(_Angle1.y + 2*_Angle2.y),	-b2y*_Pos2.y + b3z*(_Angle1.z + 2*_Angle2.z));

	if (p_Sim->StatToCalc & CALCSTAT_STRAINE) StrainEnergy = CalcStrainEnergy(); //depends on Force1, Force2, Moment1, Moment2 being set!
	if (!ChangedSaState) AddDampForces();

	//Unrotate back to global coordinate system
	Force1 = TotalRot.RotateVec3DInv(Force1);
	if (!HomogenousBond) Force2 = TotalRot.RotateVec3DInv(Force2); //could reduce for homogenous...
	Moment1 = TotalRot.RotateVec3DInv(Moment1);
	Moment2 = TotalRot.RotateVec3DInv(Moment2);

	ToOrigDirBond(&Force1);
	if (HomogenousBond) Force2 = -Force1;
	else ToOrigDirBond(&Force2); //Added
	ToOrigDirBond(&Moment1);
	ToOrigDirBond(&Moment2);
}

void CVXS_Bond::CalcContactForce() 
{
	//just basic sphere envelope, repel with the stiffness of the material... (assumes UpdateConstants has been called)
	Vec3D<> Pos2 = pVox2->GetCurPos() - pVox1->GetCurPos();
	vfloat NomDist = (pVox1->GetCurScale().x + pVox2->GetCurScale().x)*0.75; //effective diameter of 1.5 voxels...
	vfloat RelDist = NomDist - Pos2.Length(); //positive for overlap!
	if (RelDist > 0){ //if we're overlapping
		Force1 = Pos2/Pos2.Length() *a1*(RelDist); 
		Force2 = -Force1; 
	}
	else {
		Force1 = Vec3D<>(0,0,0);
		Force2 = Vec3D<>(0,0,0);
	}

	Moment1 = Vec3D<>(0,0,0);
	Moment2 = Vec3D<>(0,0,0);

}

void CVXS_Bond::CalcSimpleSpringForce()
{
	Vec3D<> CDist = pVox2->GetCurPos() - pVox1->GetCurPos();
	Force1 = a1*CDist;
	Force2 = -a1*CDist;
	Moment1 = Vec3D<>(0,0,0);
	Moment2 = Vec3D<>(0,0,0);
}

bool CVXS_Bond::SetVoxels(const int V1SIndIn, const int V2SIndIn)
{
	Vox1SInd=V1SIndIn;
	Vox2SInd=V2SIndIn;
	if (Vox1SInd == Vox2SInd) return true; //Equale voxel indices is a flag to disable a bond
	if (!UpdateVox1Ptr()){Vox1SInd=-1; return false;}
	if (!UpdateVox2Ptr()){Vox2SInd=-1; return false;}

	OrigDist = pVox2->GetOrigPos() - pVox1->GetOrigPos(); //original distance (world coords)
	HomogenousBond = (pVox1->GetMaterial() == pVox2->GetMaterial());

	if (OrigDist.x == 0 && OrigDist.y == 0) ThisBondDir = BD_Z;
	else if (OrigDist.x == 0 && OrigDist.z == 0) ThisBondDir = BD_Y;
	else if (OrigDist.y == 0 && OrigDist.z == 0) ThisBondDir = BD_X;
	else ThisBondDir = BD_ARB;

	vfloat E1 = pVox1->GetEMod(), E2 = pVox2->GetEMod();
	vfloat u1 = pVox1->GetPoisson(), u2 = pVox2->GetPoisson();
	vfloat CTE1 = pVox1->GetCTE(), CTE2 = pVox2->GetCTE();

	if (E1 == 0 || E2 == 0) {return false;}
	if (u1 < 0 || u1 > 0.5 || u2 < 0 || u2 > 0.5 ) {return false;} //AfxMessageBox("bad poissons ratio");

	E = (E1*E2/(E1+E2))*2; //x2 derived from case of equal stiffness: E1*E1/(E1+E1) = 0.5*E1 
	u = (u1*u2/(u1+u2))*2; //Poissons ratio
	CTE = (CTE1/2+CTE2/2); //thermal expansion

	//for now we are only using the nominal size of the voxel, although we could change this later if needed
	//rotate the box dimensions into the correct refference frame of x being direction of bond
	switch (ThisBondType){
		case B_LINEAR:
		L = (pVox1->GetOrigSize() + pVox2->GetOrigSize())*0.5;
		ToXDirBond(&L); //in X direction, so L.X is beam length, L.y, L.z are transverse dimensions
		L = L.Abs(); //distances strictly positive!
		break;
		case B_LINEAR_CONTACT: //X direction of L is in line with the contact...
		default:		
			L.x = p_Sim->LocalVXC.GetLatticeDim();
			L.y = L.x;
			L.z = L.x;
		break;
	}

	if (!UpdateConstants()) return false;
	ResetBond();

	return true;
}

bool CVXS_Bond::UpdateVox1Ptr() 
{
	if (!p_Sim || Vox1SInd<0 || Vox1SInd >= p_Sim->VoxArray.size()) return false;
	pVox1 = &(p_Sim->VoxArray[Vox1SInd]);
	return true;
}

bool CVXS_Bond::UpdateVox2Ptr() 
{
	if (!p_Sim || Vox2SInd<0 || Vox2SInd >= p_Sim->VoxArray.size()) return false;
	pVox2 = &(p_Sim->VoxArray[Vox2SInd]);
	return true;
}

bool CVXS_Bond::UpdateConstants(void) //fills in the constant parameters for the bond...
{
	G = E/(2*(1+u)); //Shear modulus is related to
	A = L.y * L.z;
	Iy = L.z*L.y*L.y*L.y / 12; //BHHH/12
	Iz = L.y*L.z*L.z*L.z / 12;
	J = L.y*L.z*(L.y*L.y + L.z*L.z)/12; //torsional MOI: BH/12*(BB+HH)

	if (L.x == 0){a1=0; a2=0; b1y=0; b1z=0; b2y=0; b2z=0; b3y=0; b3z=0;}
	else {
		a1 = E * A / L.x; //Units of N/m
		a2 = G * J / L.x; //Units of N-m
		b1y = 12 * E * Iy / (L.x*L.x*L.x); // + G * A / L; //Units of N/m
		b1z = 12 * E * Iz / (L.x*L.x*L.x); // + G * A / L; //Units of N/m
		b2y = 6 * E * Iy / (L.x*L.x); //Units of N (or N-m/m: torque related to linear distance)
		b2z = 6 * E * Iz / (L.x*L.x); //Units of N (or N-m/m: torque related to linear distance)
		b3y = 2 * E * Iy / L.x; //Units of N-m
		b3z = 2 * E * Iz / L.x; //Units of N-m
	}

	//for strain energy calculations
	_2xA1Inv = 1.0/(a1*2.0);
	_2xA2Inv = 1.0/(a2*2.0);
	_3xB3yInv = 1.0/(b3y*3.0);
	_3xB3zInv = 1.0/(b3z*3.0);

	//cached pre-multiplied values
	vfloat M1=0, M2=0;
	vfloat FM1=0, FM2=0;
	vfloat I1=0, I2=0;
	if (pVox1 && pVox2){	
		M1 = pVox1->GetMass(), M2 = pVox2->GetMass();
		FM1 = pVox1->GetFirstMoment(), FM2 = pVox2->GetFirstMoment(), 
		I1 = pVox1->GetInertia(), I2 = pVox2->GetInertia();
	}

	_2xSqA1xM1 = 2.0*sqrt(a1*M1); _2xSqA1xM2 = 2.0*sqrt(a1*M2);
	_2xSqA2xI1 = 2.0*sqrt(a2*I1); _2xSqA2xI2 = 2.0*sqrt(a2*I2);
	_2xSqB1YxM1 = 2.0*sqrt(b1y*M1); _2xSqB1YxM2 = 2.0*sqrt(b1y*M2);
	_2xSqB1ZxM1 = 2.0*sqrt(b1z*M1); _2xSqB1ZxM2 = 2.0*sqrt(b1z*M2);
	_2xSqB2YxFM1 = 2.0*sqrt(b2y*FM1); _2xSqB2YxFM2 = 2.0*sqrt(b2y*FM2);
	_2xSqB2ZxFM1 = 2.0*sqrt(b2z*FM1); _2xSqB2ZxFM2 = 2.0*sqrt(b2z*FM2);
	_2xSqB2YxI1 = 2.0*sqrt(b2y*I1); _2xSqB2YxI2 = 2.0*sqrt(b2y*I2);
	_2xSqB2ZxI1 = 2.0*sqrt(b2z*I1); _2xSqB2ZxI2 = 2.0*sqrt(b2z*I2);
	_2xSqB2YxM1 = 2.0*sqrt(b2y*M1); _2xSqB2YxM2 = 2.0*sqrt(b2y*M2);
	_2xSqB2ZxM1 = 2.0*sqrt(b2z*M1); _2xSqB2ZxM2 = 2.0*sqrt(b2z*M2);
	_2xSqB3YxI1 = 2.0*sqrt(b3y*I1); _2xSqB3YxI2 = 2.0*sqrt(b3y*I2);
	_2xSqB3ZxI1 = 2.0*sqrt(b3z*I1); _2xSqB3ZxI2 = 2.0*sqrt(b3z*I2);

	return true;
}

bool CVXS_Bond::UpdateBondStrain(vfloat CurStrainIn)
{
	CurStrain = CurStrainIn;

	//Single material optimize possibilities
	if (!p_Sim->IsPlasticityEnabled() || CurStrainIn >= MaxStrain){ //if we're in new territory on the stress-strain curve or plasticity is not enabled...
		MaxStrain = CurStrainIn; //set the high-water mark on the strains

		if (HomogenousBond){
			bool TmpYielded, TmpBroken;
			CurStress = pVox1->CalcVoxMatStress(CurStrainIn, &TmpYielded, &TmpBroken);
			if (!Yielded && TmpYielded) SetYielded(); //if any part of the bond yielded, its all yielded
			if (!Broken && TmpBroken) SetBroken(); //if any part of the bond broke, its all broke

			//Update the rest distance for the voxels...
			if (p_Sim->IsPlasticityEnabled())
				RestDist = OrigDist*(1+(MaxStrain - CurStress/E));

		}
		else {
			//All this to set CurStrain, bond yield/broken		
			vfloat Stress1, Stress2;
			bool Yielded1, Yielded2, Broken1, Broken2;
		

			vfloat Strain1 = CurStrainIn; //initial guesses at strains
			vfloat Strain2 = CurStrainIn;

			//get stiffness of each voxel in question...
			Stress1 = pVox1->CalcVoxMatStress(Strain1, &Yielded1, &Broken1);
			Stress2 = pVox2->CalcVoxMatStress(Strain2, &Yielded2, &Broken2);

			int MaxCount = 3;
			int count = 0;
			vfloat StressDiff = (Stress1 >= Stress2) ? Stress1-Stress2 : Stress2-Stress1;
			vfloat StressSum = Stress1+Stress2;
			if (StressSum<0) StressSum = -StressSum;

			while (StressDiff > StressSum*.0005 && count<MaxCount){ //refine guesses at the strains until difference is less than .05% of average stress
				Strain1 = 2*Stress2/(Stress1+Stress2)*Strain1;
				Strain2 = 2*Stress1/(Stress1+Stress2)*Strain2;

				Stress1 = pVox1->CalcVoxMatStress(Strain1, &Yielded1, &Broken1);
				Stress2 = pVox2->CalcVoxMatStress(Strain2, &Yielded2, &Broken2);
				StressDiff = (Stress1 >= Stress2) ? Stress1-Stress2 : Stress2-Stress1; //recalc stressDiff
				StressSum = Stress1+Stress2;
				if (StressSum<0) StressSum = -StressSum;
				count++;
			}

			CurStress = (Stress1+Stress2)/2; //average just in case we didn't quite converge to the exact same stress

			if (!Yielded && (Yielded1 || Yielded2)) SetYielded(); //if any part of the bond yielded, its all yielded
			if (!Broken && (Broken1 || Broken2)) SetBroken(); //if any part of the bond broke, its all broke

			//Update the rest distance for the voxels...
			if (p_Sim->IsPlasticityEnabled())
				RestDist = OrigDist*(1+(MaxStrain - CurStress/E));

		}
	}
	else { //if we've backed off a max strain and linearly drop back down at the elastic modulus
		CurStress = E*(CurStrainIn-(sqrt(RestDist.Length2()/OrigDist.Length2())-1.0));
	}

	return true;
}



vfloat CVXS_Bond::CalcStrainEnergy() const
{
	return	_2xA1Inv*Force1.x*Force1.x + //Tensile strain
			_2xA2Inv*Moment1.x*Moment1.x + //Torsion strain
			_3xB3zInv*(Moment1.z*Moment1.z - Moment1.z*Moment2.z +Moment2.z*Moment2.z) + //Bending Z
			_3xB3yInv*(Moment1.y*Moment1.y - Moment1.y*Moment2.y +Moment2.y*Moment2.y); //Bending Y
}

void CVXS_Bond::AddDampForces() //Adds damping forces IN LOCAL BOND COORDINATES (with bond pointing in +x direction, pos1 = 0,0,0
{
	if (p_Sim->dt != 0){ //F = -cv, zeta = c/(2*sqrt(m*k)), c=zeta*2*sqrt(mk). Therefore, F = -zeta*2*sqrt(mk)*v. Or, in rotational, Moment = -zeta*2*sqrt(Inertia*angStiff)*w
		vfloat BondZ = 0.5*p_Sim->GetBondDampZ();
		vfloat _DtInv = 1.0/p_Sim->dt;
		Vec3D<> RelVel2((_Pos2-_LastPos2)*_DtInv);
		Vec3D<> RelAngVel1((_Angle1-_LastAngle1)*_DtInv);
		Vec3D<> RelAngVel2((_Angle2-_LastAngle2)*_DtInv);

		Force1 += BondZ*Vec3D<>(-_2xSqA1xM1*RelVel2.x,
			-_2xSqB1YxM1*RelVel2.y + _2xSqB2ZxFM1*(RelAngVel1.z+RelAngVel2.z),
			-_2xSqB1ZxM1*RelVel2.z - _2xSqB2YxFM1*(RelAngVel1.y+RelAngVel2.y));
		if (!HomogenousBond){ //otherwise this is just negative of F1
			Force2 += BondZ*Vec3D<>(_2xSqA1xM2*RelVel2.x,
				_2xSqB1YxM2*RelVel2.y - _2xSqB2ZxFM2*(RelAngVel1.z+RelAngVel2.z),
				_2xSqB1ZxM2*RelVel2.z + _2xSqB2YxFM2*(RelAngVel1.y+RelAngVel2.y)); 
		}
		Moment1 += 0.5*BondZ*Vec3D<>(	-_2xSqA2xI1*(RelAngVel2.x - RelAngVel1.x),
			_2xSqB2ZxFM1*RelVel2.z + _2xSqB3YxI1*(2*RelAngVel1.y + RelAngVel2.y),
			-_2xSqB2YxFM1*RelVel2.y + _2xSqB3ZxI1*(2*RelAngVel1.z + RelAngVel2.z));
		Moment2 += 0.5*BondZ*Vec3D<>(	_2xSqA2xI2*(RelAngVel2.x - RelAngVel1.x),
			_2xSqB2ZxFM2*RelVel2.z + _2xSqB3YxI2*(RelAngVel1.y + 2*RelAngVel2.y),
			-_2xSqB2YxFM2*RelVel2.y + _2xSqB3ZxI2*(RelAngVel1.z + 2*RelAngVel2.z));

	}
	_LastPos2 = _Pos2;
	_LastAngle1 = _Angle1;
	_LastAngle2 = _Angle2;
}