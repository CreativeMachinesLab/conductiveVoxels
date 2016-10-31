/*******************************************************************************
Copyright (c) 2010, Jonathan Hiller (Cornell University)
If used in publication cite "J. Hiller and H. Lipson "Dynamic Simulation of Soft Heterogeneous Objects" In press. (2011)"

This file is part of Voxelyze.
Voxelyze is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
Voxelyze is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
See <http://www.opensource.org/licenses/lgpl-3.0.html> for license details.
*******************************************************************************/

#ifndef VXS_BOND_H
#define VXS_BOND_H

#include "Utils/Vec3D.h"
class CVXS_Voxel;
class CVX_Sim;

#ifdef PREC_LOW
	static const vfloat SA_BOND_BEND_RAD = 0.1; //Amount for small angle bond calculations
#elif defined PREC_HIGH
	static const vfloat SA_BOND_BEND_RAD = 0.02; //Amount for small angle bond calculations
#elif defined PREC_MAX 
	static const vfloat SA_BOND_BEND_RAD = 0.002; //Amount for small angle bond calculations
#else //defined PREC_MED 
	static const vfloat SA_BOND_BEND_RAD = 0.075; //Amount for small angle bond calculations
#endif


//!Bond types between two voxels.
/*!These define the bahavior of two voxels that interact in the simulation*/
enum BondType {
	B_LINEAR, //!< Generates forces and moments between connected voxels. Generates both compressive and tensile forces to resist both pushing together and pulling apart.
	B_LINEAR_CONTACT, //!< Generates repulsive forces when the voxels overlap. No tensile forces are generated. Voxels resist penetration but nothing holds them together.
	B_INPUT_LINEAR_NOROT //!<Generates forces but not moments between connected voxels. This is equivalent to a simple spring force between voxels.
};

enum BondDirection {
	BD_X, BD_Y, BD_Z, BD_ARB
};

class CVXS_Bond
{
public:
	CVXS_Bond(CVX_Sim* p_SimIn);
public:
	~CVXS_Bond(void);

	CVXS_Bond& operator=(const CVXS_Bond& Bond); //overload "=" 
	CVXS_Bond(const CVXS_Bond& Bond) {*this = Bond;} //copy constructor

	//Bond setup
	bool DefineBond(BondType BondTypeIn, int Vox1SIndIn, int Vox2SIndIn, bool PermIn); //usually only called whenn creating bond...
	bool UpdateVoxPtrs() {return UpdateVox1Ptr() && UpdateVox2Ptr(); } //call whenever VoxArray may have been reallocated

	//Bond Calculation
	void UpdateBond(void); //calculates force, positive for tension, negative for compression
	void ResetBond(void); //resets this voxel to its default (imported) state.

	//Get information about this bond
	vfloat GetStrainEnergy(void) const {return StrainEnergy;}
	vfloat GetEngStrain(void) const {return CurStrain;}
	vfloat GetEngStress(void) const {return CurStress;}
	vfloat GetLinearStiffness(void) const {return a1;}
	Vec3D<> GetForce1(void) const {return Force1;}
	Vec3D<> GetForce2(void) const {return Force2;}
	Vec3D<> GetMoment1(void) const {return Moment1;}
	Vec3D<> GetMoment2(void) const {return Moment2;}
	int GetVox1SInd() const {return Vox1SInd;}
	int GetVox2SInd() const {return Vox2SInd;}
	BondType GetBondType() const {return ThisBondType;}
	vfloat GetDampingFactorM1() const {return _2xSqA1xM1;}
	vfloat GetDampingFactorM2() const {return _2xSqA1xM2;}
	vfloat GetMaxVoxKinE();
	vfloat GetMaxVoxDisp();

	bool const IsSmallAngle(void) const {return SmallAngle;}
	bool const IsYielded(void) const {return Yielded;}
	bool const IsBroken(void) const {return Broken;}
	bool const IsPermanent(void) const {return Perm;}

	CVXS_Voxel* GetpV1() const {return pVox1;}
	CVXS_Voxel* GetpV2() const {return pVox2;}


private:
	//Bond definition:
	CVX_Sim* p_Sim; //pointer back the the simulator
	BondType ThisBondType;
	bool Perm; //is this bond permanent, (i.e. not a temporary contact bond...)

	//STATE VARIABLES (change during simulation) All updated by UpdateBond(), all reset by ResetBond()
	Vec3D<> Force1, Force2, Moment1, Moment2; //The variables we really care about
	vfloat StrainEnergy; //closely related to Forces and moments for this bond
	bool SmallAngle; //based on compiled precision setting
	Vec3D<> _Pos2, _Angle1, _Angle2;
	Vec3D<> _LastPos2, _LastAngle1, _LastAngle2;
	//Everything below updated by UpdateBondStrain()
	vfloat CurStrain, CurStress;
	vfloat MaxStrain; //the most strain this bond has undergone (needed for plastic deformation...)
	bool Yielded, Broken; //has the bond yielded or broken? (only update these after relaxation cycle!
	Vec3D<> RestDist; //From Voxel 1 to Voxel 2 (in global coordinate system... ie points in different directions for different bonds)


	//INDEPENDENT VARIABLES that will never change during a simulation: 
	//Everything below updated using SetVoxels())
	int Vox1SInd, Vox2SInd; //USE get/set functions
	CVXS_Voxel *pVox1, *pVox2; //pointers to the two voxels that make up this bond (call UpdateVoxPtrs whenever voxels added to simulation)
	Vec3D<> OrigDist; //original distance from Vox1 to Vox2 (in global CS). Does not change for non-linear stretched materials.
	bool HomogenousBond; //true if both materials are the same...
	BondDirection ThisBondDir;
	vfloat E, u, CTE;
	Vec3D<> L;
	
	//DEPENDENT VARIABLES that will never change during a simulation
	//Everything below updated by UpdateConstants. To be called after SetVoxels().
	vfloat G, A, Iy, Iz, J, a1, a2, b1y, b2y, b3y, b1z, b2z, b3z;
	vfloat _2xA1Inv, _2xA2Inv, _3xB3yInv, _3xB3zInv; 
	vfloat _2xSqA1xM1, _2xSqA1xM2, _2xSqA2xI1, _2xSqA2xI2; //a1, a2
	vfloat _2xSqB1YxM1, _2xSqB1YxM2, _2xSqB1ZxM1, _2xSqB1ZxM2; //b1
	vfloat _2xSqB2YxFM1, _2xSqB2YxFM2, _2xSqB2ZxFM1, _2xSqB2ZxFM2;
	vfloat _2xSqB2YxI1, _2xSqB2YxI2, _2xSqB2ZxI1, _2xSqB2ZxI2, _2xSqB2YxM1, _2xSqB2YxM2, _2xSqB2ZxM1, _2xSqB2ZxM2; //b2
	vfloat _2xSqB3YxI1, _2xSqB3YxI2, _2xSqB3ZxI1, _2xSqB3ZxI2; //b3

	//Specific force calculation types...
	void CalcLinForce();
	void CalcContactForce();
	void CalcSimpleSpringForce();

	bool UpdateBondStrain(vfloat CurStrainIn); //Updates yielded, brokem, CurStrain, and CurStress RestDist based on CurStrainIn
	inline vfloat CalcStrainEnergy() const; //calculates the strain energy in the bond according to current forces and moments.
	void AddDampForces(); //Adds damping forces IN LOCAL BOND COORDINATES (with bond pointing in +x direction, pos1 = 0,0,0
	bool SetVoxels(const int V1SIndIn, const int V2SIndIn); //assumes valid p_sim
	bool UpdateConstants(void); //fills in the constant parameters for the bond... returns false if unsensible material properties

	template <typename T> void ToXDirBond(Vec3D<T>* const pVec) const; //transforms a vec3D in the original orientation of the bond to that as if the bond was in +X direction
	template <typename T> void ToXDirBond(CQuat<T>* const pVec) const;
	template <typename T> void ToOrigDirBond(Vec3D<T>* const pVec) const;
	template <typename T> void ToOrigDirBond(CQuat<T>* const pVec) const;

	bool UpdateVox1Ptr();
	bool UpdateVox2Ptr();

	void SetYielded(void);
	void SetBroken(void);
	
};

//RESOURCES

//quaternion properties (for reference)
//1) To rotate a vector V, form a quaternion with w = 0; To rotate by Quaternion Q, do Q*V*Q.Conjugate() and trim off the w component.
//2) To do multiple rotations: To Rotate by Q1 THEN Q2, Q2*Q1*V*Q1.Conjugate*Q2.Conjugate(), or make a Qtot = Q2*Q1 and do Qtot*V*Qtot.Conjugate()
//3) Q1*Q1.Conjugate - Identity
//4) To do a reverse rotation Q1, just do Q1.conjugate*V*Q1
//5) An orientation quaternion is really just the relative rotation from the identity quaternion (1,0,0,0) to this orientation.
//6) If an orientation is represented by Q1, to rotate that orientation by Q2 the new orientation is Q2*Q1
//http://www.cprogramming.com/tutorial/3d/quaternions.html


//The giant stiffness matrix VoxCAD uses to model the connections between beams: (Forces, torques) = K*(displacements, angles)
//Sources:
//http://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=3&ved=0CDkQFjAC&url=http%3A%2F%2Fsteel.sejong.ac.kr%2Fdown%2Fpaper%2Fi-paper-13.pdf&ei=fmFuUP_kAeOGyQGIrIDYDQ&usg=AFQjCNG3YZI0bd9OO69VQqV7PTO3KIsEyQ&cad=rja
//http://www.colorado.edu/engineering/cas/courses.d/IFEM.d/
//http://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=2&ved=0CCcQFjAB&url=http%3A%2F%2Fwww.eng.fsu.edu%2F~chandra%2Fcourses%2Feml4536%2FChapter4.ppt&ei=919uUKOnDamryQGc_oEI&usg=AFQjCNER15oW2k4KtNrNR3FdvxNnUbnRVw&cad=rja
/*
F = |k| x
				1	2	3	4	5	6	7	8	9	10	11	12
|F1x|		|	a1						-a1						 |	|X1 |
|F1y|		|		b1y				b2z		-b1y			b2z	 | 	|Y1 |
|F1z|		|			b1z		-b2y			-b1z	-b2y	 | 	|Z1 |
|M1x|		|				a2						-a2			 | 	|TX1|
|M1y|		|			-b2z	2*b3y			b2z		b3y		 | 	|TY1|
|M1z| = 	|		b2y				2*b3z	-b2y			b3z	 |*	|TZ1|
|F2x|		|	-a1						a1						 | 	|X2 |
|F2y|		|		-b1y			-b2z	b1y				-b2z | 	|Y2 |
|F2z|		|			-b1z	b2y				b1z		b2y		 | 	|Z2 |
|M2x|		|				-a2						a2			 | 	|TX2|
|M2y|		|			-b2z	b3y				b2z		2*b3y	 | 	|TY2|
|M2z|		|		b2y				b3z		-b2y			2*b3z| 	|TZ2|

//Confirmed at http://www.eng-tips.com/viewthread.cfm?qid=14924&page=97
k[ 1][ 1] =  E*area/L;				k[ 1][ 7] = -k[1][1];
k[ 2][ 2] =  12.0*E*Iz/(L*L*L);		k[ 2][ 6] =  6.0*E*Iz/(L*L);	k[ 2][ 8] = -k[2][2];		k[ 2][12] =  k[2][6];		
k[ 3][ 3] =  12.0*E*Iy/(L*L*L);		k[ 3][ 5] = -6.0*E*Iy/(L*L);	k[ 3][ 9] = -k[3][3];		k[ 3][11] =  k[3][5];
k[ 4][ 4] =  G*J/L;					k[ 4][10] = -k[4][4];
k[ 5][ 5] =  4.0*E*Iy/L;			k[ 5][ 9] =  6.0*E*Iy/(L*L);	k[ 5][11] =  2.0*E*Iy/L;
k[ 6][ 6] =  4.0*E*Iz/L;			k[ 6][ 8] = -6.0*E*Iz/(L*L);	k[ 6][12] =  2.0*E*Iz/L;
k[ 7][ 7] =  k[1][1];
k[ 8][ 8] =  k[2][2];				k[ 8][12] = -k[2][6];
k[ 9][ 9] =  k[3][3];				k[ 9][11] =  k[5][9];
k[10][10] =  k[4][4];
k[11][11] =  k[5][5];
k[12][12] =  k[6][6];

Likewise, for caculating local damping forces
F=-zeta*2*s(MK)*v or -zeta*2*s(I*wk)*w

		||M1|				|
|M| =	|	|I1|			|	(|M1| is 3x3 identity * mass of voxel 1, etc.)
		|		|M2|		|
		|			|I2|	| (12x12)

					1		2		3		4		5		6		7		8		9		10		11		12
|F1x|			|	a1M1											-a1M1											|	|V1x|
|F1y|			|			b1yM1							b2zI1			-b1yM1							b2zI1	|	|V1y|
|F1z|			|					b1zM1			-b2yI1							-b1zM1			-b2yI1			|	|V1z|
|M1x|			|							a2I1											-a2I1					|	|w1x|
|M1y|			|					-b2zM1			2*b3yI1							b2zM1			b3yI1			|	|w1y|
|M1z| =	zeta *	|			b2yM1							2*b3zI1			-b2yM1							b3zI1	| *	|w1z|
|F2x|			|	-a1M2											a1M2											|	|V2x|
|F2y|			|			-b1yM2							-b2zI2			b1yM2							-b2zI2	|	|V2y|
|F2z|			|					-b1zM2			b2yM2							b1zM2			b2yM2			|	|V2z|
|M2x|			|							-a2I2											a2I2					|	|w2x|
|M2y|			|					-b2zM2			b3yI2							b2zM2			2*b3yI2			|	|w2y|
|M2z|			|			b2yM2							b3zI2			-b2yM2							2*b3zI2	|	|w2z|

a1M1 = 2*sqrt(a1*m1)
a1M2 = 2*sqrt(a1*m2)
b1yM1 = 2*sqrt(b1y*m2)
etc...

(extra negation added to make signs work out)
F1x = -zeta*a1M1*(V2x-V1x)
F1y = -zeta*b1yM1(V2y-V1y) + zeta*b2zI1(w1z+w2z)
F1z = -zeta*b1zM1(V2z-V1z) - zeta*b2yI1(w1y+w2y)
M1x = -zeta*a2I1(w2x-w1x)
M1y = zeta*b2zM1(V2z-V1z) + zeta*b3yI1(2*w1y+w2y)
M1z = -zeta*b2yM1(V2y-V1y) + zeta*b3zI1(2*w1z+w2z)
F2x = zeta*a1M2*(V2x-V1x)
F2y = zeta*b1yM2(V2y-V1y) - zeta*b2zI2(w1z+w2z)
F2z = zeta*b1zM2(V2z-V1z) + zeta*b2yI2(w1y+w2y)
M2x = zeta*a2I2(w2x-w1x)
M2y = zeta*b2zM2(V2z-V1z) + zeta*b3yI2(w1y+2*w2y)
M2z = -zeta*b2yM2(V2y-V1y) + zeta*b3zI2(w1z+2*w2z)


STRAIN ENERGY of the bond is the area under the force-displacement curve that can be recovered.

PURE TENSION AND COMPRESSION:
For our purposes, even with non-linear materials, it is assumed materials will rebound with their base elastic modulus.

Strain energy is the shaded area, slope is EA/L, therefore bottom edge length is FL/AE.
Area is then Fcurrent*(FL/AE)/2. (or, given EA/L is stiffness k, strain energy = 0.5*F^2/k (or F^2/(2k))
http://www.roymech.co.uk/Useful_Tables/Beams/Strain_Energy.html
http://www.freestudy.co.uk/statics/complex/t5.pdf

		|
Fcurrent|       ___
		|     /   /|
F (n)	|   /    /*|
		|  /    /**|
		| /    /***|
		|/    /****|
		------------------------
			Disp (m)

PURE TORSION
For rotational, the process is similar: T^2/(2k) where K is units of N-m

BENDING
The strain energy from bending = integral [M(x)^2 dx / (2EI)].
Because we only load the ends of the beam, M(x) = (M2-M1)*x/L+M1 (Linear interoplation, M1 and M2 are the moments at each end).
Plugging and chugging though the integral, we get:

Strain energy = L/(6EI) * (M1^2 + M1*M2 + M2^2)

Note that the sign of M2 must be reversed from that calculated by the stiffness matrix.
This is because a positive "Moment" on a beam (in the moment diagram) is actually two opposite-direction torques on the ends of the beam. ALA http://www.efunda.com/formulae/solid_mechanics/beams/sign_convention.cfm

This means Strain energy = L/(6EI) * (M1^2 - M1*M2' + M2'^2)

SHEAR
Not currently modeled in voxcad.

SUPERPOSITION:
We assume each of these modes is independent, thus total strain energy is simply the sum of them all.

*/




#endif //VXS_BOND_H