/*******************************************************************************
Copyright (c) 2010, Jonathan Hiller (Cornell University)
If used in publication cite "J. Hiller and H. Lipson "Dynamic Simulation of Soft Heterogeneous Objects" In press. (2011)"

This file is part of Voxelyze.
Voxelyze is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
Voxelyze is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
See <http://www.opensource.org/licenses/lgpl-3.0.html> for license details.
*******************************************************************************/

#include "VXS_Voxel.h"
#include "VXS_Bond.h"
#include "VX_Sim.h"


// CVXS_Voxel::CVXS_Voxel(CVX_Sim* pSimIn, int SIndexIn, int XIndexIn, int MatIndexIn, Vec3D<>& OriginalPosIn, Vec3D<>& OriginalScaleIn) 
CVXS_Voxel::CVXS_Voxel(CVX_Sim* pSimIn, int SIndexIn, int XIndexIn, int MatIndexIn, Vec3D<> OriginalPosIn, Vec3D<> OriginalScaleIn) 
{
	p_Sim = pSimIn;

	MyXIndex = XIndexIn;
	MySIndex = SIndexIn;
	OriginalPos = OriginalPosIn; //nominal position, if this is fixed
	OriginalSize = OriginalScaleIn;
	SetMaterial(MatIndexIn); //Sets a bunch of variables, depends on OriginalSize

	//constraints
	DofFixed = DOF_NONE;
	ExternalForce = Vec3D<>(0,0,0);
	ExternalDisp = Vec3D<>(0,0,0);
	ExternalTorque = Vec3D<>(0,0,0);
	ExternalTDisp = Vec3D<>(0,0,0);
	ExtInputScale = 1.0;

	ResetVoxel();

	SetColor(0,0,0,1);

	NumLocalBonds = 0;
	UnlinkAllBonds();

	BlendingEnabled = false;

}

CVXS_Voxel::~CVXS_Voxel(void)
{
}

CVXS_Voxel& CVXS_Voxel::operator=(const CVXS_Voxel& VIn)
{
	p_Sim = VIn.p_Sim;

	MyXIndex = VIn.MyXIndex;
	MySIndex = VIn.MySIndex;
	OriginalPos = VIn.OriginalPos;
	OriginalSize = VIn.OriginalSize;
	SetMaterial(VIn.MatIndex);

	DofFixed = VIn.DofFixed;
	ExternalForce = VIn.ExternalForce;
	ExternalDisp = VIn.ExternalDisp;
	ExternalTorque = VIn.ExternalTorque;
	ExternalTDisp = VIn.ExternalTDisp;
	ExtInputScale=VIn.ExtInputScale;

	S = VIn.S;
	PrevS = S;
	dS = VIn.dS;

//	MaxBondStrain = VIn.MaxBondStrain;
//	MaxBondStress = VIn.MaxBondStress;
	StaticFricFlag = VIn.StaticFricFlag;
	VYielded = VIn.VYielded;
	VBroken = VIn.VBroken;

//	ResidualDisp=VIn.ResidualDisp;
//	ResidualAng=VIn.ResidualAng;

	UnlinkAllBonds();
	for (int i=0; i<VIn.NumLocalBonds; i++)	LinkBond(VIn.GetBondIndex(i));

	NearbyVoxInds = VIn.NearbyVoxInds;

	BlendingEnabled = VIn.BlendingEnabled;
	BlendMix = VIn.BlendMix;

	m_Red = VIn.m_Red;
	m_Green = VIn.m_Green;
	m_Blue = VIn.m_Blue;
	m_Trans = VIn.m_Trans;

	return *this;
}

bool CVXS_Voxel::SetMaterial(int MatIndexIn) {
	MatIndex = MatIndexIn;

	if (MatIndexIn < 0 || MatIndexIn >= p_Sim->LocalVXC.Palette.size()){
		_pMat = NULL;
		Mass = Inertia = FirstMoment = ScaleMass = _massInv = _inertiaInv = _2xSqMxExS = _2xSqIxExSxSxS = 0;
		return false; //enforce index range
	}

	_pMat = p_Sim->LocalVXC.GetBaseMat(MatIndex);

	//Update material depended parameters
	vfloat Volume = OriginalSize.x*OriginalSize.y*OriginalSize.z;
	vfloat AvgDim = (OriginalSize.x + OriginalSize.y + OriginalSize.z) / 3;

	Mass = Volume * _pMat->GetDensity(); 
	Inertia = Mass * (AvgDim*AvgDim)/6; //simple 1D apprix
	FirstMoment = Mass*AvgDim/2;
	ScaleMass = Mass; //Temporary placeholder

	if (Volume==0 || Mass==0 || Inertia==0){_massInv=_inertiaInv=_2xSqMxExS=_2xSqIxExSxSxS=0; return false;}

	_massInv = 1/Mass; //cache inverses for FAST division
	_inertiaInv = 1/Inertia;
	_2xSqMxExS = 2*sqrt(Mass*GetEMod()*OriginalSize.x);
	_2xSqIxExSxSxS = 2*sqrt(Inertia*GetEMod()*OriginalSize.x*OriginalSize.x*OriginalSize.x);

	return true;
}
void CVXS_Voxel::ResetVoxel(void) //resets this voxel to its defualt (imported) state.
{
	S = VoxState(); //defaults everything to zero
	dS = dVoxState();

	S.Pos = OriginalPos; //only position and size need to be set
	S.Scale = OriginalSize;
	PrevS = S;

//	ResidualDisp=Vec3D<>(0,0,0);
//	ResidualAng=Vec3D<>(0,0,0);

	//varying parameters (during the sim)
//	MaxBondStrain = 0;
//	MaxBondStress = 0;
	StaticFricFlag = false;
	VYielded = false;
	VBroken = false;
}

bool CVXS_Voxel::LinkBond(int SBondIndex) //simulation bond index...
{
	if (!p_Sim || SBondIndex >= p_Sim->BondArray.size()) return false;

	BondInds.push_back(SBondIndex);
	BondPointers.push_back(&(p_Sim->BondArray[SBondIndex]));
	NumLocalBonds++;
	return true;
}

void CVXS_Voxel::UnlinkBond(int SBondIndex) //removes this bond from the pointers (if it exists), even if permanent bond
{
	for (int i=0; i<NumLocalBonds; i++){
		if (BondInds[i] == SBondIndex){ //if a match
			BondInds.erase(BondInds.begin()+i);
			BondPointers.erase(BondPointers.begin()+i);
			NumLocalBonds--;
			i--;
		}
	}
}

bool CVXS_Voxel::UpdateBondLinks(void) //updates all links (pointers) to bonds!
{
	int NumBondsInSim = p_Sim->BondArray.size();
	for (int i=0; i<NumLocalBonds; i++){
		int ThisBondInd = BondInds[i];
		if (ThisBondInd >= NumBondsInSim) return false;
		BondPointers[i] = &(p_Sim->BondArray[ThisBondInd]);
	}
	return true;
}


CVXS_Voxel* CVXS_Voxel::pNearbyVox(int LocalNearbyInd)
{
	return &(p_Sim->VoxArray[NearbyVoxInds[LocalNearbyInd]]);
}

void CVXS_Voxel::UnlinkTmpBonds(void) //(assumes all perm at beginning...) removes any temporary bonds from our local list...
{
	while (NumLocalBonds != 0 && !GetBond(NumLocalBonds-1)->IsPermanent()){
		BondInds.pop_back();
		BondPointers.pop_back();
		NumLocalBonds--;
	}
} 


//void CVXS_Voxel::UpdateDState() //calculates the derivative of our current state "S". All bonds should be updates before this!
//{
//	dS.LinMom = S.LinMom;
//	dS.AngMom = S.AngMom;
////	dS.ScaleMom = S.ScaleMom; //this isn't quite the right mass... need some sort of scaling inertia... Mass/6?
//
//	dS.Force = CalcTotalForce();
//	dS.Moment = CalcTotalMoment();
////	dS.ScaleForce = CalcTotalScaleForce();
//
//}

//void CVXS_Voxel::UpdateState(vfloat dtIn, dVoxState& Derivative) //advances this state "S" with previously calculated dS by dt
//{
//	/*
//	//DISPLACEMENT
//	S.LinMom = S.LinMom + Derivative.Force*dtIn;
////	S.Pos = S.Pos + dS.LinMom/Mass*dt; //dS.Vel = dS.LinMom/Mass
//	S.Pos = S.Pos + Derivative.LinMom/Mass*dtIn; //dS.Vel = dS.LinMom/Mass
//
//	//ANGLE
//	S.AngMom = S.AngMom + Derivative.Moment*dtIn;
//
//	//convert Angular velocity to quaternion form ("Spin")
////	Vec3D dSAngVel = dS.AngMom / Inertia;
//	Vec3D dSAngVel = Derivative.AngMom / Inertia;
//	if (abs(S.Angle.Length2()-1)>1e-6) S.Angle.Normalize(); //don't have to do this every time. Just if it drifts too much...
//	CQuat Spin = 0.5 * CQuat(0, dSAngVel.x, dSAngVel.y, dSAngVel.z) * S.Angle; //current "angular velocity"
//
//
//	S.Angle = S.Angle + Spin*dtIn; //see above
//
//	//SCALE
////	S.ScaleMom = S.ScaleMom + Derivative.ScaleForce*dtIn;
////	S.Scale = S.Scale + dS.ScaleMom/ScaleMass*dtIn; //dS.ScaleVel = dS.ScaleMom/ScaleMass;
////	S.Scale = S.Scale + S.ScaleMom/ScaleMass*dtIn; //dS.ScaleVel = dS.ScaleMom/ScaleMass;
//
////temporary Temp update...
//	//vfloat TempFact = 1.0;
//	//if(p_Sim->pEnv->TempEnabled) TempFact = (1+(p_Sim->pEnv->CurTemp-p_Sim->pEnv->TempBase)*GetCTE()); //LocalVXC.GetBaseMat(VoxArray[i].MatIndex)->GetCTE());
//	//if (TempFact < MIN_TEMP_FACTOR) TempFact = MIN_TEMP_FACTOR;
//	//S.Scale = TempFact*OriginalSize;
//
//
//	//Recalculate secondary:
//	S.Vel = S.LinMom/Mass;
//	S.AngVel = S.AngMom / Inertia;
////	S.ScaleVel = S.ScaleMom/ScaleMass;
//*/
//}

//http://klas-physics.googlecode.com/svn/trunk/src/general/Integrator.cpp (reference)
void CVXS_Voxel::EulerStep(void)
{
	double dt = p_Sim->dt;

	//bool EqMode = p_Sim->IsEquilibriumEnabled();
	if (IS_ALL_FIXED(DofFixed)){ //if fixed, just update the position and forces acting on it (for correct simulation-wide summing
		S.LinMom = Vec3D<double>(0,0,0);
		S.Pos = OriginalPos + ExtInputScale*ExternalDisp;
		S.AngMom = Vec3D<double>(0,0,0);
		S.Angle.FromRotationVector(Vec3D<double>(ExtInputScale*ExternalTDisp));
		dS.Force = CalcTotalForce();
	}
	else {
		Vec3D<> ForceTot = CalcTotalForce(); //TotVoxForce;

		//DISPLACEMENT
		S.LinMom = S.LinMom + ForceTot*dt;
		Vec3D<double> Disp(S.LinMom*(dt*_massInv)); //vector of what the voxel moves

		if(p_Sim->IsMaxVelLimitEnabled()){ //check to make sure we're not going over the speed limit!
			vfloat DispMag = Disp.Length();
			vfloat MaxDisp = p_Sim->GetMaxVoxVelLimit()*p_Sim->pEnv->pObj->GetLatticeDim();
			if (DispMag>MaxDisp) Disp *= (MaxDisp/DispMag);
		}
		S.Pos += Disp; //update position (source of noise in float mode???

		if (IS_FIXED(DOF_X, DofFixed)){S.Pos.x = OriginalPos.x + ExtInputScale*ExternalDisp.x; S.LinMom.x = 0;}
		if (IS_FIXED(DOF_Y, DofFixed)){S.Pos.y = OriginalPos.y + ExtInputScale*ExternalDisp.y; S.LinMom.y = 0;}
		if (IS_FIXED(DOF_Z, DofFixed)){S.Pos.z = OriginalPos.z + ExtInputScale*ExternalDisp.z; S.LinMom.z = 0;}

		//ANGLE
		Vec3D<> TotVoxMoment = CalcTotalMoment(); //debug
		S.AngMom = S.AngMom + TotVoxMoment*dt;

		//convert Angular velocity to quaternion form ("Spin")
		Vec3D<double> dSAngVel(S.AngMom * _inertiaInv);
		CQuat<double> Spin = 0.5 * CQuat<double>(0, dSAngVel.x, dSAngVel.y, dSAngVel.z) * S.Angle; //current "angular velocity"

		S.Angle += CQuat<double>(Spin*dt); //see above
		S.Angle.NormalizeFast(); //Through profiling, quicker to normalize every time than check to see if needed then do it...

	//	TODO: Only constrain fixed angles if one is non-zero! (support symmetry boundary conditions while still only doing this calculation) (only works if all angles are constrained for now...)
		if (IS_FIXED(DOF_TX, DofFixed) && IS_FIXED(DOF_TY, DofFixed) && IS_FIXED(DOF_TZ, DofFixed)){
			S.Angle.FromRotationVector(Vec3D<double>(ExtInputScale*ExternalTDisp));
			S.AngMom = Vec3D<>(0,0,0);
		}
	}

	//SCALE
	//	S.ScaleMom = S.ScaleMom + CalcTotalScaleForce()*p_Sim->dt;
	vfloat TempFact = 1.0;
	if(p_Sim->pEnv->IsTempEnabled()){ 
		//TempFact = (1+(p_Sim->pEnv->CurTemp-p_Sim->pEnv->TempBase)*GetCTE()); //LocalVXC.GetBaseMat(VoxArray[i].MatIndex)->GetCTE());
		
		// std::cout << "TempEnabled" << std::endl;
		double ThisTemp = p_Sim->pEnv->pObj->GetBaseMat(GetMaterial())->GetCurMatTemp();
		// std::cout << "ThisTemp: " << ThisTemp << std::endl;
		double ThisCTE = GetCTE();
		double TempBase =  p_Sim->pEnv->GetTempBase();


		TempFact = (1+(ThisTemp - TempBase)*ThisCTE);	//To allow selective temperature actuation for each different material
	}
	
	// Caclulate Electrical Activation:

	if ( /*_pMat->GetNeuralActivatedMuscleType()*/ GetMuscleType() )
	{	
		if (p_Sim->GetNeuralOutputOfIndex( /*_pMat->GetNeuralActivatedMuscleType()*/GetMuscleType()-1) > 0 ) //p_Sim->pEnv->GetActivationThreshold() )  // nac: add this as a parameter?
		{
			S.ElectricallyActiveNew = true;
			// S.RepolarizationStartTime = p_Sim->CurTime + _pMat->GetDepolarizationTime();
			if ((p_Sim->CurTime - S.RepolarizationStartTime) > p_Sim->pEnv->GetTempPeriod())
			{
				S.RepolarizationStartTime = p_Sim->CurTime;
			}
		}
		else
		{
			S.ElectricallyActiveNew = false;
			// S.RepolarizationStartTime = p_Sim->CurTime - _pMat->GetRepolarizationTime();
		}
	}
	else
	{
		if ( /*_pMat->GetNeuralSensorType()*/GetSensorType() )
		{
			p_Sim->SetSensorValueOfIndex( /*_pMat->GetNeuralSensorType()*/ GetSensorType() -1, GetCurGroundPenetration()>0);
		}
	}

	if ( _pMat->GetIsConductive() )
	{
		if ( _pMat->GetIsPacemaker() )
		{
			// std::cout << "TempFact: " << TempFact << std::endl;
			S.ElectricallyActiveNew = p_Sim->pEnv->GetCurTemp() > p_Sim->pEnv->GetTempBase();
			if (p_Sim->pEnv->GetCurTemp() > p_Sim->pEnv->GetTempBase())
			{		
				if ((p_Sim->CurTime - S.RepolarizationStartTime) > p_Sim->pEnv->GetTempPeriod())
				{
					S.RepolarizationStartTime = p_Sim->CurTime;
				}
			
			}

		} 
		// Touch Sensor
		else 
		{ 
			if (/*_pMat->GetIsGroundTouchSensor()*/GetSensorType())
			{
				if (GetCurGroundPenetration()>0)
				{
					S.ElectricallyActiveNew = true;
					// S.RepolarizationStartTime = p_Sim->CurTime + _pMat->GetDepolarizationTime();
					if ((p_Sim->CurTime - S.RepolarizationStartTime) > p_Sim->pEnv->GetTempPeriod())
					{
						S.RepolarizationStartTime = p_Sim->CurTime;
					}
				}
				else
				{
					S.ElectricallyActiveNew = false;
					// S.RepolarizationStartTime = p_Sim->CurTime - _pMat->GetRepolarizationTime();
				}
			}	
			else 
			{
				if ((p_Sim->CurTime - S.RepolarizationStartTime) < /*_pMat->GetDepolarizationTime()*/ _pMat->GetRepolarizationTime()/2)
				{
					S.ElectricallyActiveNew = true;
				}
				else
				{
					if ((p_Sim->CurTime - S.RepolarizationStartTime) < _pMat->GetRepolarizationTime()*2)
					{
						S.ElectricallyActiveNew = false;
					}
					else
					{
						// std::cout << "Updating Neighbors" << std::endl;		
						UpdateElectricalActivationFromNeighbors();
					}
				}
			}
		}

	}

	if (TempFact < MIN_TEMP_FACTOR) TempFact = MIN_TEMP_FACTOR;
	if (_pMat->GetIsConductive() or /*_pMat->GetNeuralActivatedMuscleType()*/ GetMuscleType() )
	{
		// if (S.ElectricallyActiveNew and p_Sim->CurTime - S.RepolarizationStartTime > p_Sim->pEnv->GetTempPeriod())
		// {
		// 	S.RepolarizationStartTime = p_Sim->CurTime;
		// }

		if (p_Sim->CurTime - S.RepolarizationStartTime <= p_Sim->pEnv->GetTempPeriod() )
		{
			S.Scale = std::max(MIN_TEMP_FACTOR, 1+ p_Sim->pEnv->GetTempAmplitude()*GetCTE()*sin(2*3.1415926/ p_Sim->pEnv->GetTempPeriod()/*_pMat->GetRepolarizationTime()*/ * (p_Sim->CurTime - S.RepolarizationStartTime) ))*OriginalSize;
			// std::max( MIN_TEMP_FACTOR, p_Sim->pEnv->GetTempBase() + p_Sim->pEnv->GetTempAmplitude()*sin((2*3.1415926f/p_Sim->pEnv->GetTempPeriod()) * p_Sim->CurTime ))*OriginalSize;
		}
		else 
		{
			S.Scale = OriginalSize; //get rid of this assumption?
		}
	} 
	else
	{
		S.Scale = TempFact*OriginalSize;
		// if (S.Scale.x != 0.001)
		// {
		// 	std::cout << "Scale:" << S.Scale.x << " " << S.Scale.y << " " <<  S.Scale.z << std::endl;
		// }
	}



	//Recalculate secondary:
	S.AngVel = S.AngMom * _inertiaInv;
	S.Vel = S.LinMom * _massInv;
	//vfloat LastKinE = S.KineticEnergy;
	if(p_Sim->StatToCalc & CALCSTAT_KINE) S.KineticEnergy = 0.5*Mass*S.Vel.Length2() + 0.5*Inertia*S.AngVel.Length2(); //1/2 m v^2

	////Testing:
	//if (EqMode && S.KineticEnergy < LastKinE){
	//	S.LinMom *= 0.99; //Vec3D<double>(0,0,0);
	//	S.AngMom *= 0.99; //= Vec3D<double>(0,0,0);
	//	S.Vel *= 0.99; //= Vec3D<>(0,0,0);
	//	S.AngVel*= 0.99; // = Vec3D<>(0,0,0);
	//	//S.KineticEnergy = 0;
	//	S.KineticEnergy = 0.5*Mass*S.Vel.Length2() + 0.5*Inertia*S.AngVel.Length2(); //1/2 m v^2
	//}
}

void CVXS_Voxel::VerletStep()
{
	//vfloat dt = p_Sim->dt;

	//Vec3D<> newPos = 2*S.Pos - PrevS.Pos + CalcTotalForce()/Mass*dt*dt;
	//if (StaticFricFlag) {newPos.x = S.Pos.x; newPos.y = S.Pos.y;}

	//PrevS.Pos = S.Pos;

	//S.Vel = (newPos - S.Pos)/dt; //get Vel for other functions to use...
	//S.Pos = newPos;

	//if (IS_ALL_FIXED(DofFixed)) S.Pos = OriginalPos + ExtInputScale*ExternalDisp; //enforce fixed voxels...

}
//
//const inline vfloat CVXS_Voxel::GetCurAbsDisp(void)
//{
//	return (S.Pos-OriginalPos).Length();
//}

void CVXS_Voxel::SetColor(float r, float g, float b, float a)
{
	m_Red = r;
	m_Green = g;
	m_Blue = b;
	m_Trans = a;
}	

void CVXS_Voxel::FixDof(char DofFixedIn) //fixes any of the degrees of freedom indicated. Doesn't unfix any currently fixed ones
{
	if (IS_FIXED(DOF_X, DofFixedIn)) SET_FIXED(DOF_X, DofFixed, true);
	if (IS_FIXED(DOF_Y, DofFixedIn)) SET_FIXED(DOF_Y, DofFixed, true);
	if (IS_FIXED(DOF_Z, DofFixedIn)) SET_FIXED(DOF_Z, DofFixed, true);
	if (IS_FIXED(DOF_TX, DofFixedIn)) SET_FIXED(DOF_TX, DofFixed, true);
	if (IS_FIXED(DOF_TY, DofFixedIn)) SET_FIXED(DOF_TY, DofFixed, true);
	if (IS_FIXED(DOF_TZ, DofFixedIn)) SET_FIXED(DOF_TZ, DofFixed, true);

}


float CVXS_Voxel::UpdateElectricalActivationFromNeighbors(void)
{
	// std::cout<< "checking neighbors" << std::endl;
	int NumLocBond = GetNumLocalBonds();
	S.ElectricallyActiveNew = false;
	for (int i=0; i<NumLocBond; i++){
		CVXS_Voxel* pThisNeighborVoxel;
		CVXS_Bond* pThisBond = GetBond(i);
		bool IAmVox1 = IsMe(pThisBond->GetpV1()); //otherwise vox 2 of the bond
		if (IAmVox1) pThisNeighborVoxel = pThisBond->GetpV2(); //Vox 1 from this bond
		else {pThisNeighborVoxel = pThisBond->GetpV1();} //Vox 2 from this bond

		// // if ((p_Sim->CurTime - S.RepolarizationStartTime > 0 ) and (p_Sim->CurTime - S.RepolarizationStartTime < _pMat->GetRepolarizationTime()/2)) {S.ElectricallyActiveNew = true;}
		// if (p_Sim->pEnv->pObj->GetBaseMat(pThisNeighborVoxel->GetMaterial())->GetName() == "Active_+"){
		// std::cout<< "neighbor material:" <<p_Sim->pEnv->pObj->GetBaseMat(pThisNeighborVoxel->GetMaterial())->GetName() << std::endl;
		// if (p_Sim->CurTime - S.RepolarizationStartTime >= _pMat->GetRepolarizationTime()) {std::cout<< "polarization up" << std::endl;}
		// }
		// if (pThisNeighborVoxel->GetElectricallyActiveOld()) {std::cout<< "neighbor active" << std::endl;}
		// // if (p_Sim->CurTime - S.RepolarizationStartTime >= _pMat->GetRepolarizationTime()) {std::cout<< "polarization up" << std::endl;}

		// if (pThisNeighborVoxel->GetElectricallyActiveOld() and p_Sim->CurTime - S.RepolarizationStartTime >= _pMat->GetRepolarizationTime()) { S.ElectricallyActiveNew = true; S.RepolarizationStartTime = p_Sim->CurTime;}
		if (  ((p_Sim->CurTime - pThisNeighborVoxel->GetRepolarizationStartTime()) > _pMat->/*GetDepolarizationTime()/2*/GetRepolarizationTime()/4) and ((p_Sim->CurTime - pThisNeighborVoxel->GetRepolarizationStartTime()) < _pMat->/*GetDepolarizationTime()*/GetRepolarizationTime()/2 ))  { S.ElectricallyActiveNew = true; S.RepolarizationStartTime = p_Sim->CurTime;}
		if ( /*p_Sim->pEnv->pObj->GetBaseMat(*/pThisNeighborVoxel->/*GetMaterial())->GetIsGroundTouchSensor()*/GetSensorType() and pThisNeighborVoxel->GetElectricallyActiveOld())  { S.ElectricallyActiveNew = true; S.RepolarizationStartTime = p_Sim->CurTime;}
		// std::cout << "CurTime: "<< p_Sim->CurTime << std::endl;
		// std::cout << "_pMat->GetRepolarizationTime(): "<< _pMat->GetRepolarizationTime() << std::endl;
		// std::cout << "S.RepolarizationStartTime: "<< S.RepolarizationStartTime << std::endl;
	}
	return 0;
}

Vec3D<> CVXS_Voxel::CalcTotalForce(bool WithRestraint)
{
//	THE NEXT optimization target
	//INTERNAL forces
	Vec3D<> TotalForce = Vec3D<>(0,0,0);
//	Vec3D<> _NegSlowDampZxVel = -p_Sim->GetSlowDampZ() * S.Vel;
	TotalForce += -p_Sim->GetSlowDampZ() * S.Vel*_2xSqMxExS; //(2*sqrt(Mass*GetEMod()*S.Scale.x)); //TOOODOOO!!! GetEMod = slow, slow, slow!!

	//Forces from the bonds
	int NumLocBond = GetNumLocalBonds();
	for (int i=0; i<NumLocBond; i++){
		CVXS_Bond* pThisBond = GetBond(i);
		bool IAmVox1 = IsMe(pThisBond->GetpV1()); //otherwise vox 2 of the bond

		if (IAmVox1) TotalForce -= pThisBond->GetForce1(); //Force on Vox 1 from this bond
		else TotalForce -= pThisBond->GetForce2(); //Force on Vox 2 from this bond
	}

	//from interactive click/drag. TODO: This should just be another bond, and treated as such. ?
	if (p_Sim->Dragging && MyXIndex == p_Sim->CurXSel){ //if we're currently dragging this one!
		TotalForce += p_Sim->InputBond()->GetForce1();
		TotalForce -= 2*Mass*sqrt(p_Sim->InputBond()->GetLinearStiffness()/Mass)*S.Vel;  //critically damp force for this bond to ground
	}

	//From gravity
	if (p_Sim->pEnv->IsGravityEnabled())
		TotalForce.z += Mass*p_Sim->pEnv->GetGravityAccel();

	//EXTERNAL forces
	TotalForce += ExtInputScale*ExternalForce; //add in any external forces....

	if(p_Sim->pEnv->IsFloorEnabled()) TotalForce += CalcFloorEffect(Vec3D<vfloat>(TotalForce));
	else StaticFricFlag = false;

	if (StaticFricFlag) {TotalForce.x = 0; TotalForce.y = 0;}

	if (IS_FIXED(DOF_X, DofFixed) && WithRestraint) TotalForce.x=0;
	if (IS_FIXED(DOF_Y, DofFixed) && WithRestraint) TotalForce.y=0;
	if (IS_FIXED(DOF_Z, DofFixed) && WithRestraint) TotalForce.z=0;

	return TotalForce;
}

Vec3D<> CVXS_Voxel::CalcTotalMoment(void)
{
	Vec3D<> TotalMoment(0,0,0);
	for (int i=0; i<GetNumLocalBonds(); i++) {
		//add moments from bond
		if (MySIndex == GetBond(i)->GetpV1()->MySIndex){ TotalMoment -= GetBond(i)->GetMoment1(); } //if this is voxel 1
		else { TotalMoment -= GetBond(i)->GetMoment2(); } //if this is voxel 2
	}

	//EXTERNAL moments
	//ground damping????
	//Vec3D<> _NegSlowDampZxAngVel = -p_Sim->GetSlowDampZ() * S.AngVel;
//	TotalMoment += _NegSlowDampZxAngVel * (2*sqrt(Inertia*GetEMod()*S.Scale.x*S.Scale.x*S.Scale.x)); //TODO: cache!!!
	TotalMoment += -p_Sim->GetSlowDampZ() * S.AngVel *_2xSqIxExSxSxS; //(2*sqrt(Inertia*GetEMod()*S.Scale.x*S.Scale.x*S.Scale.x)); //TODO: cache!!!
	//	Inertia = Mass * (AvgDim*AvgDim)/6; //simple 1D apprix


	//TotalForce += CalcGndDampEffect();
	TotalMoment += ExtInputScale*ExternalTorque; //add in any external forces....

	if (IS_FIXED(DOF_TX, DofFixed)) TotalMoment.x=0;
	if (IS_FIXED(DOF_TY, DofFixed)) TotalMoment.y=0;
	if (IS_FIXED(DOF_TZ, DofFixed)) TotalMoment.z=0;

	return TotalMoment;
}

Vec3D<> CVXS_Voxel::CalcTotalScaleForce(void) //calculate Scaling force
{
	Vec3D<> TotalSForce(0,0,0);

	Vec3D<> TmpThisForce;

	for (int i=0; i<GetNumLocalBonds(); i++){
		TmpThisForce -= GetBond(i)->GetForce1(); //positive for tension, negative for compression
	}

	//add forces to enforce poissons ratio...
	vfloat MyStiff = p_Sim->LocalVXC.GetLeafMat(MatIndex)->GetElasticMod()*p_Sim->LocalVXC.GetLatticeDim();
	TotalSForce += Vec3D<>((S.Angle.Conjugate()*CQuat<double>(TmpThisForce)*S.Angle).ToVec()); //add in the force internal to the voxel...
	TotalSForce -= 2*MyStiff*(S.Scale-OriginalSize);
	vfloat poiss = 0.5;
	if (p_Sim->PoissonKickBackEnabled) {
		vfloat VolScale = OriginalSize.x*OriginalSize.y*OriginalSize.z/(S.Scale.x*S.Scale.y*S.Scale.z); //>1 if we're too small <1 if we're too large
		TotalSForce -= (2*poiss)*(1-VolScale) * MyStiff*(S.Scale); //arbitrary scaling factor...
	}

	TotalSForce -= 2*ScaleMass*sqrt(MyStiff/ScaleMass)*S.ScaleVel;  //Damping force for this bond:

	if (IS_ALL_FIXED(DofFixed)) TotalSForce = Vec3D<>(0,0,0);
	return TotalSForce;
}

vfloat CVXS_Voxel::GetCurGroundPenetration() //how far into the ground penetrating (penetration is positive, no penetration is zero)
{
	vfloat Penetration = (S.Scale.x+S.Scale.y+S.Scale.z)/6.0 - S.Pos.z;
	return Penetration <= 0 ? 0 : Penetration;
}

Vec3D<> CVXS_Voxel::CalcFloorEffect(Vec3D<> TotalVoxForce) //calculates the object's interaction with a floor. should be calculated AFTER all other forces for static friction to work right...
{
	Vec3D<> FloorForce(0,0,0); //the force added by floor interactions...

	StaticFricFlag = false; //assume not under static friction unless we decide otherwise
	vfloat LatDim = (OriginalSize.x+OriginalSize.y+OriginalSize.z)/3.0; //first order approximation!!! not accurate for oblong voxels!  //p_Sim->LocalVXC.GetLatticeDim(); //need to account for the change in volume elements...
	vfloat CurPenetration = GetCurGroundPenetration();

//	S.Scale.
//	if (S.Pos.z < LatDim/2){ 
	if (CurPenetration>0){ 
		vfloat LocA1 = p_Sim->LocalVXC.GetBaseMat(MatIndex)->GetElasticMod()*2*LatDim; 
		vfloat LocUDynamic = p_Sim->LocalVXC.GetBaseMat(MatIndex)->GetuDynamic();
		vfloat LocUStatic = p_Sim->LocalVXC.GetBaseMat(MatIndex)->GetuStatic();

		vfloat NormalForce = LocA1 * CurPenetration; //positive for penetration...
//		vfloat NormalForce = p_Sim->LocalVXC.GetBaseMat(MatIndex)->GetElasticMod() * LatDim/2 *(LatDim/2 - S.Pos.z); //positive for penetration...
		//TODO:																get id of this /2 ^  ?


		FloorForce.z += NormalForce; //force resisting penetration

	
		//do vertical damping here...
		FloorForce.z -= p_Sim->GetCollisionDampZ()*2*Mass*sqrt(LocA1/Mass)*S.Vel.z;  //critically damp force for this bond to ground


		vfloat SurfaceVel = sqrt(S.Vel.x*S.Vel.x + S.Vel.y*S.Vel.y); //velocity along the floor...
		vfloat SurfaceVelAngle = atan2(S.Vel.y, S.Vel.x); //angle of sliding along floor...
		vfloat SurfaceForce = sqrt(TotalVoxForce.x*TotalVoxForce.x + TotalVoxForce.y*TotalVoxForce.y);
		vfloat dFrictionForce = LocUDynamic*NormalForce; //TODO: make this a material property!
		Vec3D<> FricForceToAdd = -Vec3D<>(cos(SurfaceVelAngle)*dFrictionForce, sin(SurfaceVelAngle)*dFrictionForce, 0); //always acts in direction opposed to velocity in DYNAMIC friction mode
		//alwyas acts in direction opposite to force in STATIC friction mode

		if (S.Vel.x == 0 && S.Vel.y == 0){ //STATIC FRICTION: if this point is stopped and in the static friction mode...

			if (SurfaceForce < LocUStatic*NormalForce){ //if we don't have enough to break static friction
				StaticFricFlag = true;
			}
//			else //i don't think we need. this is when we've just broken static friction, so there's no velocity yet to have a counter force
//				CollisionForce += FricForceToAdd;

		}
		else { //DYNAMIC FRICTION
			if (dFrictionForce*p_Sim->dt < Mass*SurfaceVel){ // if we are sliding, coming to a stop. We know they are always in same angle, opposite directions.
				FloorForce += FricForceToAdd;
			}
			else {
				StaticFricFlag = true;
				S.LinMom.x = 0; //fully stop the voxel here! (caution...)
				S.LinMom.y = 0;
			}
		}
		
	}
	return FloorForce;

}

Vec3D<> CVXS_Voxel::CalcGndDampEffect() //damps everything to ground as qucik as possible...
{
	vfloat tmp = 0;
	for (int i=0; i<GetNumLocalBonds(); i++) tmp+=sqrt(GetBond(i)->GetLinearStiffness()*Mass);
	return -p_Sim->GetSlowDampZ()*2*tmp*S.Vel;
}


void CVXS_Voxel::CalcNearby(int NumHops) //populates Nearby
{
	NearbyVoxInds.clear();
	int StartPoint = 0; //our enter and exit point (so we don't repeat for each iteration
	int StopPoint = 1;

	NearbyVoxInds.push_back(MySIndex);

	for (int i=0; i<NumHops; i++){
		for (int j=StartPoint; j<StopPoint; j++){ //go through the list from the most recent interation...
			for (int k=0; k<pNearbyVox(j)->GetNumLocalBonds(); k++){ //look at all the bonds of this voxel
				//get the other voxel in this bond...
				int OtherSIndex = pNearbyVox(j)->GetBond(k)->GetpV1()->MySIndex;
				if (pNearbyVox(j)->IsMe(pNearbyVox(j)->GetBond(k)->GetpV1()))  OtherSIndex = pNearbyVox(j)->GetBond(k)->GetpV2()->MySIndex; //if this voxel 1

				//Add it to the list if its not already on it.
				if (!IsNearbyVox(OtherSIndex)) NearbyVoxInds.push_back(OtherSIndex);
			}
		}
		StartPoint = StopPoint;
		StopPoint = NumNearbyVox();
	}
}

vfloat CVXS_Voxel::GetMaxBondStrain(void) const
{
	vfloat MxSt = 0;
	for (int i=0; i<NumLocalBonds; i++){
		vfloat TSt = BondPointers[i]->GetEngStrain();
		if (TSt>MxSt) MxSt = TSt; 
	}
	return MxSt;
}

vfloat CVXS_Voxel::GetMaxBondStrainE(void) const
{
	vfloat MxSt = 0;
	for (int i=0; i<NumLocalBonds; i++){
		vfloat TSt = BondPointers[i]->GetStrainEnergy();
		if (TSt>MxSt) MxSt = TSt; 
	}
	return MxSt;
}

vfloat CVXS_Voxel::GetMaxBondStress(void) const
{
	vfloat MxSt = 0;
	for (int i=0; i<NumLocalBonds; i++){
		vfloat TSt = BondPointers[i]->GetEngStress();
		if (TSt>MxSt) MxSt = TSt; 
	}
	return MxSt;

}

vfloat CVXS_Voxel::CalcVoxMatStress(const vfloat StrainIn, bool* const IsPastYielded, bool* const IsPastFail) const
{
//	CVXC_Material* ThisMat = GetpMat();
	if (!BlendingEnabled){ //if we're not doing any material blending
		return _pMat->GetModelStress(StrainIn, IsPastYielded, IsPastFail);
	}
	else { //MAJOR optimization target for blended materials!
		vfloat Sum = 0;
		vfloat tmpWeight = 0;
		vfloat tmpStress;
		bool tmpYield, tmpFail;
		*IsPastYielded = false;
		*IsPastFail = false;


		//ONLY WORKS WITH TWO MATERIALS OAT A TIME! MAJOR LIMITATION IN NEED OF WORK!
		vfloat MinStress= FLT_MAX;
		vfloat MaxStress = -FLT_MAX;
		vfloat PercStronger = 0;
		//get min and max
		for (int MatIndex = 1; MatIndex<p_Sim->LocalVXC.GetNumMaterials(); MatIndex++){
			if (BlendMix[MatIndex] != 0){
				tmpStress = p_Sim->LocalVXC.GetBaseMat(MatIndex)->GetModelStress(StrainIn, &tmpYield, &tmpFail);
				if(tmpYield) *IsPastYielded = true; //yielded if any material yielded
				if(tmpFail) *IsPastFail = true; //yielded if any material yielded


				if (tmpStress<MinStress){
					MinStress=tmpStress;
				}
				if (tmpStress>MaxStress){
					PercStronger=BlendMix[MatIndex];
					MaxStress=tmpStress;
				}
			}
		}
		if (MinStress == MaxStress){*IsPastYielded=tmpYield; *IsPastFail=tmpFail; return MinStress;}

		switch (p_Sim->BlendModel){
			case MB_LINEAR: tmpWeight = PercStronger; break;
			case MB_EXPONENTIAL: tmpWeight = pow(2, PercStronger)-1; break;
			case MB_POLYNOMIAL: tmpWeight = pow(PercStronger, p_Sim->PolyExp); break;
			}

		return tmpWeight*(MaxStress-MinStress)+MinStress;

	}
}

vfloat CVXS_Voxel::GetEMod(void) //gets elastic modulus, taking blending into account
{
	if (!p_Sim->BlendingEnabled){ //if we're not doing any material blending
		return p_Sim->LocalVXC.GetBaseMat(MatIndex)->GetElasticMod();
	}

	//cache this!!!

	else { //return combo of elastic moduli according to blending model
		vfloat Sum = 0;
		vfloat tmpWeight = 0;
		vfloat PercWeaker = 0;

		//ONLY WORKS WITH TWO MATERIALS OAT A TIME! MAJOR LIMITATION IN NEED OF WORK!
		vfloat MinEMod = FLT_MAX;
		vfloat MaxEMod = 0;
		//get min and max
		for (int MatIndex = 1; MatIndex<p_Sim->LocalVXC.GetNumMaterials(); MatIndex++){
			if (BlendMix[MatIndex] != 0){
				vfloat ThisEMod = p_Sim->LocalVXC.GetBaseMat(MatIndex)->GetElasticMod();
				if (ThisEMod<MinEMod){
					PercWeaker = BlendMix[MatIndex];
					MinEMod=ThisEMod;
				}
				if (ThisEMod>MaxEMod) MaxEMod=ThisEMod;
			}
		}
		if (MinEMod == MaxEMod) return MaxEMod;


		switch (p_Sim->BlendModel){
			case MB_LINEAR: tmpWeight = PercWeaker; break;
			case MB_EXPONENTIAL: tmpWeight = pow(2, PercWeaker)-1; break;
			case MB_POLYNOMIAL: tmpWeight = pow(PercWeaker, p_Sim->PolyExp); break;
			}

		return tmpWeight*(MaxEMod-MinEMod)+MinEMod;


	}
}

vfloat CVXS_Voxel::GetPoisson(void) //gets poisson ratio, taking blending into account
{
	if (!p_Sim->BlendingEnabled){ //if we're not doing any material blending
		return p_Sim->LocalVXC.GetBaseMat(MatIndex)->GetPoissonsRatio();
	}

	//cache this!!!
	else { //for now, return linear average Poissons ratio
		vfloat Sum = 0;
		for (int MatIndex = 1; MatIndex<p_Sim->LocalVXC.GetNumMaterials(); MatIndex++){
			if (BlendMix[MatIndex] != 0){ //if contains any of this material
				Sum += BlendMix[MatIndex]*p_Sim->LocalVXC.GetBaseMat(MatIndex)->GetPoissonsRatio();
			}
		}
		return Sum; //Don't have to normalize by weight since BlendMix is already normalized.
	}
}

vfloat CVXS_Voxel::GetCTE(void) //gets CTE, taking blending into account
{
	if (!p_Sim->BlendingEnabled){ //if we're not doing any material blending
		return p_Sim->LocalVXC.GetBaseMat(MatIndex)->GetCTE();
	}


	//cache this!!!
	else { //for now, return linear average CTE
		vfloat Sum = 0;
		for (int MatIndex = 1; MatIndex<p_Sim->LocalVXC.GetNumMaterials(); MatIndex++){
			if (BlendMix[MatIndex] != 0){ //if contains any of this material
				Sum += BlendMix[MatIndex]*p_Sim->LocalVXC.GetBaseMat(MatIndex)->GetCTE();
			}
		}
		return Sum; //Don't have to normalize by weight since BlendMix is already normalized.
	}
}






void CVXS_Voxel::CalcMyBlendMix() //requires MyXIndex, p_sim and p_Sim->LocalVXC to already be in place...
{
	//initialize blendmix to size of palette with zeros...
	BlendingEnabled = true;
	BlendMix.clear();
	BlendMix.resize(p_Sim->LocalVXC.GetNumMaterials(), 0);

	vfloat BlendRad=p_Sim->MixRadius; //make a local copy for convenience
	int x, y, z, ThisInd;
	Vec3D<> ThisPos;
	vfloat ThisDist, ThisWeight;
	p_Sim->LocalVXC.GetXYZNom(&x, &y, &z, MyXIndex);
	Vec3D<> BasePos = p_Sim->LocalVXC.GetXYZ(MyXIndex);
	Vec3D<> EnvMult = p_Sim->LocalVXC.Lattice.GetDimAdj();
	int xLook = 3*BlendRad/EnvMult.x+1;
	int yLook = 3*BlendRad/EnvMult.y+1;
	int zLook = 3*BlendRad/EnvMult.z+1;

	//BlendRad is the standard deviation of the gaussian mixing weighting . (multiply by GetLatticeDim to get the real-valued standard dev)
//	vfloat AvgEMod = 0.0;
	vfloat TotalWeight = 0.0;

	//up to three sigma...
	for (int ix = x-xLook; ix<=x+xLook; ix++){
		for (int jy = y-yLook; jy<=y+yLook; jy++){
			for (int kz = z-zLook; kz<=z+zLook; kz++){
				ThisInd = p_Sim->LocalVXC.GetIndex(ix, jy, kz);
				if (ThisInd == -1) continue; //if invalid location then skip
				if (p_Sim->LocalVXC.GetMat(ThisInd) == 0) continue; //skip this one if it is empty (no material...
					
				ThisPos = p_Sim->LocalVXC.GetXYZ(ThisInd);
				ThisDist = ((ThisPos-BasePos).Length())/p_Sim->LocalVXC.GetLatticeDim();

				ThisWeight = exp(-0.5*ThisDist*ThisDist/(BlendRad*BlendRad));
				BlendMix[p_Sim->LocalVXC.GetLeafMatIndex(ThisInd)] += ThisWeight;
				TotalWeight += ThisWeight;
			}
		}
	}

	//normalize to make sure BlendMix adds up to 1!
	for(std::vector<vfloat>::iterator it = BlendMix.begin(); it != BlendMix.end(); it++)
		*it /= TotalWeight;


}

int CVXS_Voxel::GetMuscleType() 
{
	return p_Sim->LocalVXC.GetMuscleTypeArray(MyXIndex);
}

int CVXS_Voxel::GetSensorType() 
{
	return p_Sim->LocalVXC.GetSensorTypeArray(MyXIndex);
}