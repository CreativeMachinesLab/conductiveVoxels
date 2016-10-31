#ifndef HCUBE_SOFTBOTS_H_INCLUDED
#define HCUBE_SOFTBOTS_H_INCLUDED

#include "HCUBE_Experiment.h"

#include "HCUBE_Vector2.h"

#include <fstream>
#include "Mesh.h"
#include "Array3D.h"
#include <map>
#include <string>


namespace HCUBE
{
    class SoftbotsExperiment : public Experiment
    {
    public:
    protected:
		
		//struct FitnessRecord { double fits[2]; };

        int numVoxelsX,numVoxelsY,numVoxelsZ;

        map<Node,string> nameLookup;

        int sizeMultiplier;
		int convergence_step;
		float voxelExistsThreshold;
		int allTimeHighCounter; 
		CArray3Df TargetContinuousArray;
		CMesh TargetMesh;
		
		Vec3D WorkSpace; //reasonable workspace (meters)
		double VoxelSize; //Size of voxel in meters (IE the lattice dimension);
		int num_x_voxels;
		int num_y_voxels;
		int num_z_voxels;

		//map<std::string, double> fitnessLookup;
		map<std::string, pair<double, double> > fitnessLookup;
		int numVoxelsFilled;
		int numVoxelsActuated;
		int numConnections;
		int numMaterials;

		int totalEvals;
		int skippedEvals;

		bool addDistanceFromCenter;
		bool addDistanceFromCenterXY;
		bool addDistanceFromCenterYZ;
		bool addDistanceFromCenterXZ;

		bool addAngularPositionXY;
		bool addAngularPositionYZ;
		bool addAngularPositionXZ;

		int numPacemaker;
		
		
		void generateTarget3DObject();
		int compareEvolvedArrayToTargetArray(CArray3Df &ContinuousArray);  
		double drawShape(bool m_EvolvingInteractively, CMesh& meshToDraw);
		vector <int> drawShapes(vector <CMesh> m_meshesToDraw, int _generationNumber);

    public:
        SoftbotsExperiment(string _experimentName);

        virtual ~SoftbotsExperiment() {}

        virtual NEAT::GeneticPopulation* createInitialPopulation(int populationSize);

        double mapXYvalToNormalizedGridCoord(const int & r_xyVal, const int & r_numVoxelsXorY);
			
        double processEvaluation(shared_ptr<NEAT::GeneticIndividual> individual, string individualID, int genNum, bool saveVXA, double bestFit);

        virtual void processGroup(shared_ptr<NEAT::GeneticGeneration> generation);

        virtual void processIndividualPostHoc(shared_ptr<NEAT::GeneticIndividual> individual);

        virtual void printNetworkCPPN(shared_ptr<const NEAT::GeneticIndividual> individual);

        virtual Experiment* clone();
				
		bool converged(int generation);

		void writeAndTeeUpParentsOrderFile(); 

		// int writeVoxelyzeFile( CArray3Df ContinuousArray, CArray3Df PassiveStiffArray, CArray3Df PassiveSoftArray, CArray3Df Phase1Array, CArray3Df Phase2Array, string individualID);
		int writeVoxelyzeFile( CArray3Df ArrayForVoxelyze, CArray3Df ConductiveArray, CArray3Df ActuatedArray, string individualID, shared_ptr<NEAT::GeneticIndividual> individual);

		CArray3Df createArrayForVoxelyze(CArray3Df ContinuousArray,CArray3Df ConductiveArray, CArray3Df ActuatedArray);
		
		CArray3Df makeOneShapeOnly(CArray3Df ArrayForVoxelyze);

		int leftExists(CArray3Df ArrayForVoxelyze, int currentPosition);

		int rightExists(CArray3Df ArrayForVoxelyze, int currentPosition);

		int forwardExists(CArray3Df ArrayForVoxelyze, int currentPosition);

		int backExists(CArray3Df ArrayForVoxelyze, int currentPosition);

		int upExists(CArray3Df ArrayForVoxelyze, int currentPosition);

		int downExists(CArray3Df ArrayForVoxelyze, int currentPosition);

		pair< queue<int>, list<int> > circleOnce(CArray3Df ArrayForVoxelyze, queue<int> queueToCheck, list<int> alreadyChecked);

		bool isNextTo(int posX1, int posX2, int posY1, int posY2, int posZ1, int posZ2);

		vector<int> indexToCoordinates(int v);

		int coordinatesToIndex(int x, int y, int z);

    };
}

#endif // HCUBE_SHAPES_H_INCLUDED
