#include "MarchCube.h"
#include <iomanip> 
#include "Experiments/HCUBE_SoftbotsExperiment.h"
#include <stdlib.h>
#include <time.h>
//#include "DigitalMatter.h"
//#include "DM_FEA.h" 
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctype.h>
//ACHEN: Included for temporary hack
#include <tinyxml.h> // #include “tinyxml.h"?
#include <string>
#ifdef VISUALIZESHAPES
//#include <SFML/Graphics.hpp>
#endif
#include <sstream>
#include <algorithm> 
#include <sys/wait.h>


#define SHAPES_EXPERIMENT_DEBUG (0)
#define PI 3.14159265

namespace HCUBE
{
    using namespace NEAT;

    SoftbotsExperiment::SoftbotsExperiment(string _experimentName)
    :   Experiment(_experimentName)
    {
        
        cout << "Constructing experiment named SoftbotsExperiment.\n";

		addDistanceFromCenter = true;
		addDistanceFromCenterXY = false;
		addDistanceFromCenterYZ = false;
		addDistanceFromCenterXZ = false;

		addAngularPositionXY = false;
		addAngularPositionYZ = false;
		addAngularPositionXZ = false;
			
		convergence_step = int(NEAT::Globals::getSingleton()->getParameterValue("ConvergenceStep"));
		if(convergence_step < 0)
			convergence_step = INT_MAX; // never switch

		num_x_voxels = (int)NEAT::Globals::getSingleton()->getParameterValue("BoundingBoxLength");
		num_y_voxels = num_x_voxels;
		num_z_voxels = num_x_voxels;
		cout << "NUM X VOXELS: " << num_x_voxels << endl;

		int FixedBoundingBoxSize = (int)NEAT::Globals::getSingleton()->getParameterValue("FixedBoundingBoxSize");
		if (FixedBoundingBoxSize > 0)
		{
			VoxelSize = FixedBoundingBoxSize*0.001/num_x_voxels;
		} else {
			VoxelSize = 0.001;
		}

		cout << "Voxel Size: " << VoxelSize << endl;
		cout << "Total Bounding Box Size: " << VoxelSize * num_x_voxels << endl;

		#ifndef INTERACTIVELYEVOLVINGSHAPES		
		//TargetContinuousArray = CArray3Df(num_x_voxels, num_y_voxels, num_z_voxels);	//because calling the consrucor on TargetContinuousArray only initialized it locally (!?!)u
		//generateTarget3DObject(); // fill array with target shape
		#endif
        
    }

    NEAT::GeneticPopulation* SoftbotsExperiment::createInitialPopulation(int populationSize)
    {

        NEAT::GeneticPopulation* population = new NEAT::GeneticPopulation();

        vector<GeneticNodeGene> genes;
	// cout << "JMC HERE" << endl;

        genes.push_back(GeneticNodeGene("Bias","NetworkSensor",0,false));
        genes.push_back(GeneticNodeGene("x","NetworkSensor",0,false));
        genes.push_back(GeneticNodeGene("y","NetworkSensor",0,false));
        genes.push_back(GeneticNodeGene("z","NetworkSensor",0,false));
		if(addDistanceFromCenter)	genes.push_back(GeneticNodeGene("d","NetworkSensor",0,false));
		if(addDistanceFromCenterXY) genes.push_back(GeneticNodeGene("dxy","NetworkSensor",0,false));
		if(addDistanceFromCenterYZ) genes.push_back(GeneticNodeGene("dyz","NetworkSensor",0,false));
		if(addDistanceFromCenterXZ) genes.push_back(GeneticNodeGene("dxz","NetworkSensor",0,false));
		if(addAngularPositionXY) genes.push_back(GeneticNodeGene("rxy","NetworkSensor",0,false));
		if(addAngularPositionYZ) genes.push_back(GeneticNodeGene("ryz","NetworkSensor",0,false));
		if(addAngularPositionXZ) genes.push_back(GeneticNodeGene("rxz","NetworkSensor",0,false));
		genes.push_back(GeneticNodeGene("OutputEmpty","NetworkOutputNode",1,false,ACTIVATION_FUNCTION_SIGMOID));
		genes.push_back(GeneticNodeGene("OutputConductive","NetworkOutputNode",1,false,ACTIVATION_FUNCTION_SIGMOID));
		genes.push_back(GeneticNodeGene("OutputActuated","NetworkOutputNode",1,false,ACTIVATION_FUNCTION_SIGMOID));
        for (size_t a=0;a<populationSize;a++)
        {
            shared_ptr<GeneticIndividual> individual(new GeneticIndividual(genes,true,1.0));

            population->addIndividual(individual);
        }

        return population;
    }


    double SoftbotsExperiment::mapXYvalToNormalizedGridCoord(const int & r_xyVal, const int & r_numVoxelsXorY) 
    {
        // turn the xth or yth node into its coordinates on a grid from -1 to 1, e.g. x values (1,2,3,4,5) become (-1, -.5 , 0, .5, 1)
        // this works with even numbers, and for x or y grids only 1 wide/tall, which was not the case for the original
        // e.g. see findCluster for the orignal versions where it was not a funciton and did not work with odd or 1 tall/wide #s
		
        double coord;
                
        if(r_numVoxelsXorY==1) coord = 0;
        else                  coord = -1 + ( r_xyVal * 2.0/(r_numVoxelsXorY-1) );

        return(coord);    
    }
	

    double SoftbotsExperiment::processEvaluation(shared_ptr<NEAT::GeneticIndividual> individual, string individualID, int genNum, bool saveVxaOnly, double bestFit)
    {
    	// return NEAT::Globals::getSingleton()->getRandom().getRandomDouble();

		cout << "Processing Evaluation" << endl;
		
		//initializes continuous space array with zeros. +1 is because we need to sample
		// these points at all corners of each voxel, leading to n+1 points in any dimension
		CArray3Df ContinuousArray(num_x_voxels, num_y_voxels, num_z_voxels); //evolved array
		CArray3Df ConductiveArray(num_x_voxels, num_y_voxels, num_z_voxels);
		CArray3Df ActuatedArray(num_x_voxels, num_y_voxels, num_z_voxels);


		NEAT::FastNetwork <double> network = individual->spawnFastPhenotypeStack<double>();        //JMC: this is the CPPN network

		int px, py, pz; //temporary variable to store locations as we iterate
		float xNormalized;
		float yNormalized;
		float zNormalized;
		float distanceFromCenter;
		float distanceFromCenterXY;
		float distanceFromCenterYZ;
		float distanceFromCenterXZ;
		float angularPositionXY;
		float angularPositionYZ;
		float angularPositionXZ;
		int ai;
	
		for (int j=0; j<ContinuousArray.GetFullSize(); j++)  //iterate through each location of the continuous matrix
		{ 
			ContinuousArray.GetXYZ(&px, &py, &pz, j); //gets XYZ location of this element in the array
			xNormalized = mapXYvalToNormalizedGridCoord(px, num_x_voxels);
			yNormalized = mapXYvalToNormalizedGridCoord(py, num_y_voxels);
			zNormalized = mapXYvalToNormalizedGridCoord(pz, num_z_voxels);
			
			//calculate input vars
			if(addDistanceFromCenter)   distanceFromCenter   = sqrt(pow(double(xNormalized),2.0)+pow(double(yNormalized),2.0)+pow(double(zNormalized),2.0));			
			if(addDistanceFromCenterXY) distanceFromCenterXY = sqrt(pow(double(xNormalized),2.0)+pow(double(yNormalized),2.0));
			if(addDistanceFromCenterYZ) distanceFromCenterYZ = sqrt(pow(double(yNormalized),2.0)+pow(double(zNormalized),2.0));
			if(addDistanceFromCenterXZ) distanceFromCenterXZ = sqrt(pow(double(xNormalized),2.0)+pow(double(zNormalized),2.0));	
			if(addAngularPositionXY) { if (yNormalized==0.0 and xNormalized==0.0) { angularPositionXY = 0.0; }	else { angularPositionXY = atan2(double(yNormalized),double(xNormalized))/PI; } }
			if(addAngularPositionYZ) { if (yNormalized==0.0 and zNormalized==0.0) { angularPositionYZ = 0.0; }	else { angularPositionYZ = atan2(double(zNormalized),double(yNormalized))/PI; } }
			if(addAngularPositionXZ) { if (zNormalized==0.0 and xNormalized==0.0) { angularPositionXZ = 0.0; }	else { angularPositionXZ = atan2(double(zNormalized),double(xNormalized))/PI; } }			
	
			network.reinitialize();								//reset CPPN
			network.setValue("x",xNormalized);					//set the input numbers
			network.setValue("y",yNormalized);
			network.setValue("z",zNormalized);
			if(addDistanceFromCenter) network.setValue("d",distanceFromCenter);
			if(addDistanceFromCenterXY) network.setValue("dxy",distanceFromCenterXY);
			if(addDistanceFromCenterYZ) network.setValue("dyz",distanceFromCenterYZ);
			if(addDistanceFromCenterXZ) network.setValue("dxz",distanceFromCenterXZ);
			if(addAngularPositionXY) network.setValue("rxy",angularPositionXY);
			if(addAngularPositionYZ) network.setValue("ryz",angularPositionYZ);
			if(addAngularPositionXZ) network.setValue("rxz",angularPositionXZ);
			//if(addDistanceFromShell) network.setValue("ds",e); //ACHEN: Shell distance
			//if(addInsideOrOutside) network.setValue("inout",e);
			network.setValue("Bias",0.3);                       
							
			network.update();                                   //JMC: on this line we run the CPPN network...  
			
			ContinuousArray[j] = network.getValue("OutputEmpty");        //JMC: and here we get the CPPN output (which is the weight of the connection between the two)
			ConductiveArray[j] = network.getValue("OutputConductive");
			ActuatedArray[j] = network.getValue("OutputActuated");
		}

		// cout<<"here1"<<endl;

		CArray3Df ArrayForVoxelyze = createArrayForVoxelyze(ContinuousArray, ConductiveArray, ActuatedArray);
	
		float fitness;
		float origFitness = 0.00001;
		int voxelPenalty;
		int numTotalVoxels = 0;
		int numConductiveVoxels = 0;
		int numActuatedVoxels = 0;
		for (int j=0; j<ContinuousArray.GetFullSize(); j++)  //iterate through each location of the continuous matrix
		{
			if ( ArrayForVoxelyze[j]>0 ) { numTotalVoxels++; }
			if ( ArrayForVoxelyze[j]==1 or ArrayForVoxelyze[j]==2 ) { numConductiveVoxels++; }
			if ( ArrayForVoxelyze[j]==1 or ArrayForVoxelyze[j]==3 ) { numActuatedVoxels++; }
		}

		if (numTotalVoxels<2 or numConductiveVoxels<2 or numActuatedVoxels<2) {cout << "No Voxels Filled... setting fitness to 0.00001" << endl; return 0.00001;}

		if (int(NEAT::Globals::getSingleton()->getParameterValue("PenaltyType")) == 3) 
		{
			cout << endl <<"ERROR: Connection Penalty Not Yet Implemented" << endl;
			exit(0);
			// voxelPenalty = numConnections/2;
		} else {
			if (int(NEAT::Globals::getSingleton()->getParameterValue("PenaltyType")) == 2) 
			{
				voxelPenalty = numActuatedVoxels;
			} else {
				voxelPenalty = numTotalVoxels;
			}
			if (int(NEAT::Globals::getSingleton()->getParameterValue("PenaltyType")) == 5) 
			{
				voxelPenalty = numConductiveVoxels;
			}

		}

		writeVoxelyzeFile(ArrayForVoxelyze, ConductiveArray, ActuatedArray, individualID, individual);

		if (true)
		{
			//nac: check md5sum of vxa file	
			//std::ostringstream md5sumCmd;
			//md5sumCmd << "md5sum " << individualID << "_genome.vxa";
			//FILE* pipe = popen(md5sumCmd.str().c_str(), "r");

			FILE* pipe = popen("md5sum md5sumTMP.txt", "r");			
			if (!pipe) {cout << "ERROR 1, exiting." << endl << endl; return 0.00001;}
			char buffer[128];
			std::string result = "";
			while(!feof(pipe)) 
			{
				if(fgets(buffer, 128, pipe) != NULL)
					result += buffer;
			}
			pclose(pipe);

			std::string md5sumString = result.substr(0,32);
			// cout << "md5sum: " << md5sumString << endl;
			individual->setThismd5(md5sumString);

			cout << "got md5 for: " << individual->getThismd5() << " (born from: " << individual->getParent1md5() << " and " << individual->getParent2md5() << ")" << endl; 
			
			if (fitnessLookup.find(md5sumString) != fitnessLookup.end())
			{
				//fitness = fitnessLookup[md5sumString];
				//FitnessRecord fits = fitnessLookup[md5sumString];
				pair<double, double> fits = fitnessLookup[md5sumString];
				fitness = fits.second;
				//origFitness = fits.second;
				//cout << "fitness from hash: " << fitness << endl;
				//cout << "orig fitness: " << origFitness << endl;
				cout << "This individual was already evaluated!  Its original fitness look-up is: " << fitness << endl;
			} 
			else 
			{
				clock_t start;
				clock_t end;
				start = clock();
				// cout << "starting voxelyze evaluation now" << endl;

				std::ostringstream callVoxleyzeCmd;
				callVoxleyzeCmd << "./voxelize -f " << individualID << "_genome.vxa";

				std::ostringstream outFileName;
				outFileName << individualID << "_fitness.xml";  

				fitness = 0.00001;

				int exitCode;
		
				FILE* input = popen(callVoxleyzeCmd.str().c_str(),"r");
	
double t2;
bool doneEval = false;

while (not doneEval)
{

	end = clock();
	std::ifstream infile(outFileName.str().c_str());
	if (infile.is_open())
	{
		// cout << "done" << endl;
		// printf("voxelyze took %.6lf seconds\n", double(end-start)/CLOCKS_PER_SEC);
		printf("voxelyze took %.6lf seconds\n", float(end-start)/CLOCKS_PER_SEC);
		sleep(0.1);
		doneEval = true;

		// exit(0);
	}
	else
	{
		sleep(0.01);
		// std::cout << ".";
		if ( double(end-start)/CLOCKS_PER_SEC > 130.0)//num_x_voxels*num_y_voxels*num_z_voxels/10)
		{
			// cout << "voxelyze hung after " << double(end-start)/CLOCKS_PER_SEC  << " seconds... assigning fitness of 0.00001"<<endl<<endl;
			cout << "voxelyze hung after 120 seconds... assigning fitness of 0.00001"<<endl;
			// kill(0, SIGTERM);

			int exitCode3 = std::system("ps axu > /tmp/HnPsFile.txt");
			std::ifstream psfile("/tmp/HnPsFile.txt");
			std::string thisLine;
			if (psfile.is_open())
  			{
				while (std::getline(psfile, thisLine))
				{
					std::size_t foundvox = thisLine.find("./voxelize");
					if (foundvox!=std::string::npos)
					{
						if (atoi(thisLine.substr(foundvox-8,4).c_str()) >= 2)
						{
							std::size_t foundsp = thisLine.find(" ");
							cout << ("kill "+thisLine.substr(foundsp,11)).c_str() << endl;
							int exitCode4 = std::system(("kill "+thisLine.substr(foundsp,11)).c_str());
							int exitCode5 = std::system("killall <defunct>");
						}
					}
				}
			}

			cout << endl;
			return 0.00001;
			// exit(0);
		}
	}
	infile.close();
	// cout << "t2" << t2 << endl;
}
pclose(input);
				std::ifstream infile(outFileName.str().c_str());

				std::string line;
				float FinalCOM_DistX;
				float FinalCOM_DistY;
				float FinalCOM_DistZ;
				if (infile.is_open())
  				{
					while (std::getline(infile, line))
					{
					    std::size_t foundx = line.find("<FinalCOM_DistX>");
					    if (foundx!=std::string::npos)
					    {
					    	FinalCOM_DistX = atof(line.substr(foundx+strlen("<FinalCOM_DistX>"),line.find("</")-(foundx+strlen("<FinalCOM_DistX>"))).c_str());
					    }
					    std::size_t foundy = line.find("<FinalCOM_DistY>");
					    if (foundy!=std::string::npos)
					    {
					    	FinalCOM_DistY = atof(line.substr(foundy+strlen("<FinalCOM_DistY>"),line.find("</")-(foundy+strlen("<FinalCOM_DistY>"))).c_str());
					    }
					    std::size_t foundz = line.find("<FinalCOM_DistZ>");
					    if (foundz!=std::string::npos)
					    {
					    	FinalCOM_DistZ = atof(line.substr(foundz+strlen("<FinalCOM_DistZ>"),line.find("</")-(foundz+strlen("<FinalCOM_DistZ>"))).c_str());
					    }
					}

					fitness = pow(pow(FinalCOM_DistX,2)+pow(FinalCOM_DistY,2),0.5);
					// cout << "Return from voxelyze: " << fitness << endl;
					fitness = fitness/(max(num_x_voxels,max(num_y_voxels,num_z_voxels))*VoxelSize);
					cout << "*** Fitness from voxelyze: " << fitness << endl;

					infile.close();
				}
			}
			
			if (fitness < 0.00001) fitness = 0.00001;
			if (fitness > 1000) fitness = 0.00001;

			int outOf;
			if (int(NEAT::Globals::getSingleton()->getParameterValue("PenaltyType")) == 3) 
			{
				// note: only works for cubes, foil out everything to get expression for rectangles
				outOf = (num_x_voxels*num_x_voxels*num_x_voxels - num_x_voxels*num_x_voxels)*3;
			} else {
				outOf = num_x_voxels*num_x_voxels*num_x_voxels;
			}
			cout << "Voxels (or Connections) per Possible Maximum: " << voxelPenalty << " / " << outOf << endl;  //nac: put in connection cost variant
			double voxelPenaltyf = 1.0 - pow(1.0*voxelPenalty/(outOf*1.0),NEAT::Globals::getSingleton()->getParameterValue("PenaltyExp"));
			//cout << "PenaltyExp: " << NEAT::Globals::getSingleton()->getParameterValue("PenaltyExp") << endl;
			//cout << "base: " << 1.0*voxelPenalty/(outOf*1.0) << endl;
			//cout << "pow: " << pow(1.0*voxelPenalty/(outOf*1.0),NEAT::Globals::getSingleton()->getParameterValue("PenaltyExp")) << endl;
			origFitness = fitness;
			if (int(NEAT::Globals::getSingleton()->getParameterValue("PenaltyType")) != 0) {fitness = fitness * (voxelPenaltyf);}
			//cout << "Fitness Penalty Adjustment: " << voxelPenaltyf << endl;
			if (int(NEAT::Globals::getSingleton()->getParameterValue("PenaltyType")) != 0) {printf("Fitness Penalty Multiplier: %f\n", voxelPenaltyf);}
			// else {cout << "No penalty funtion, using original fitness" << endl;}

			// nac: update fitness lookup table		
			// FitnessRecord fitness, origFitness;
			// std::map<double, FitnessRecord> fits;

			if (fitness < 0.00001) fitness = 0.00001;
			if (fitness > 1000) fitness = 0.00001;

			pair <double, double> fits (fitness, origFitness);	
			fitnessLookup[md5sumString]=fits;
	
		}
		if (int(NEAT::Globals::getSingleton()->getParameterValue("PenaltyType")) != 0) {cout << "ADJUSTED FITNESS VALUE: " << fitness << endl;}

		std::ostringstream addFitnessToInputFileCmd;
		std::ostringstream addFitnessToOutputFileCmd;

		if (genNum<51 or genNum % (int)NEAT::Globals::getSingleton()->getParameterValue("RecordEntireGenEvery")==0) // || bestFit<fitness) // <-- nac: individuals processed one at a time, not as generation group
		{		
			addFitnessToInputFileCmd << "mv " << individualID << "_genome.vxa Gen_"; 

			char buffer1 [100];
			sprintf(buffer1, "%04i", genNum);
			addFitnessToInputFileCmd << buffer1;

			addFitnessToInputFileCmd << "/" << individual->getThismd5() << "--adjFit_";

			char adjFitBuffer[100];
			sprintf(adjFitBuffer, "%.8lf", fitness);
			addFitnessToInputFileCmd << adjFitBuffer;

			addFitnessToInputFileCmd << "--numFilled_";

			char NumFilledBuffer[100];
			sprintf(NumFilledBuffer, "%04i", voxelPenalty);
			addFitnessToInputFileCmd << NumFilledBuffer;

			addFitnessToInputFileCmd << "--origFit_";

			char origFitBuffer[100];
			sprintf(origFitBuffer, "%.8lf", origFitness);
			addFitnessToInputFileCmd << origFitBuffer;

			addFitnessToInputFileCmd << "_genome.vxa";

		} else
		{
			addFitnessToInputFileCmd << "rm " << individualID << "_genome.vxa";
		}

		addFitnessToOutputFileCmd << "rm -f " << individualID << "_fitness.xml";// << individualID << "--Fit_"<< fitness <<"_fitness.xml";

		int exitCode3 = std::system(addFitnessToInputFileCmd.str().c_str());
		int exitCode4 = std::system(addFitnessToOutputFileCmd.str().c_str());  
		
		if (fitness < 0.00001) fitness = 0.00001;	
		individual->setOrigFitness(origFitness);
		
		return fitness; 
    }
	

    void SoftbotsExperiment::processGroup(shared_ptr<NEAT::GeneticGeneration> generation)
    {
		double bestFit = 0.0;

		for(int z=0;z< group.size();z++)
		{
			numVoxelsFilled   = 0;
			numVoxelsActuated = 0;
			numConnections    = 0;	

			//totalEvals++;

			//cout << "indivudial num??? ('z' value): " << z <<endl;

			numMaterials = int(NEAT::Globals::getSingleton()->getParameterValue("NumMaterials"));

			int genNum = generation->getGenerationNumber() + 1;

			char buffer2 [50];
			sprintf(buffer2, "%04i", genNum);

			std::ostringstream mkGenDir;
			if (genNum<51 or genNum % (int)NEAT::Globals::getSingleton()->getParameterValue("RecordEntireGenEvery")==0)
			{
				mkGenDir << "mkdir -p Gen_" << buffer2;
			}			
	
			int exitCode5 = std::system(mkGenDir.str().c_str());

			shared_ptr<NEAT::GeneticIndividual> individual = group[z];

			//cout<< "RUN NAME: " << NEAT::Globals::getSingleton()->getOutputFilePrefix() << endl;

			std::ostringstream tmp;
			tmp << "Softbots--" << NEAT::Globals::getSingleton()->getOutputFilePrefix() << "--Gen_" ;
			char buffer [50];
			sprintf(buffer, "%04i", genNum);
			tmp << buffer;
			tmp << "--Ind_" << individual;
	  		string individualID = tmp.str();
			cout << endl << individualID << endl;
			

			#if SHAPES_EXPERIMENT_DEBUG
			  cout << "in SoftbotsExperiment.cpp::processIndividual, processing individual:  " << individual << endl;
			#endif
			double fitness = 0;
			
			
			fitness = processEvaluation(individual, individualID, genNum, false, bestFit);
			
			if (bestFit<fitness) bestFit = fitness;
					
			if(fitness > std::numeric_limits<double>::max())
			{
				cout << "error: the max fitness is greater than the size of the double. " << endl;
				cout << "max double size is: : " << std::numeric_limits<double>::max() << endl;
				exit(88);
			} 
			if (fitness < 0)
			{
				cout << "Fitness Less Than Zero!!!, it is: " << fitness << "\n";  
				exit(10);
			}
			
			#if SHAPES_EXPERIMENT_DEBUG        
				cout << "Individual Evaluation complete!\n";
				printf("fitness: %f\n", fitness);	
			#endif

			// cout << "Individual Evaluation complete!\n";
			cout << "fitness: " << fitness << endl;
	  
			individual->reward(fitness);

			if (false)	//to print cppn
			{
				printNetworkCPPN(individual);    
			}
			
			//cout << "Total Evaluations: " << totalEvals << endl;
			//cout << "Skipped Evaluations: " << skippedEvals << endl;
			//cout << "(Total for run) PercentOfEvaluationsSkipped: " << double(totalEvals)/double(skippedEvals) << endl;
		}
    }

    void SoftbotsExperiment::processIndividualPostHoc(shared_ptr<NEAT::GeneticIndividual> individual)
    {
		
		
        /*{
            mutex::scoped_lock scoped_lock(*Globals::ioMutex);
            cout << "Starting Evaluation on object:" << this << endl;
            cout << "Running on individual " << individual << endl;
        }

        cout << "Sorry, this was never coded up for ShapeRecognitioV1. You'll have to do that now." << endl;
        exit(6);
        
        //in jason's code, the got 10 points just for entering the game, wahooo!
        individual->setFitness(0);

        double fitness=0;

        double maxFitness = 0;

        //bool solved=true; @jmc: was uncommented in the original and produced an error forn not being used

        //cout << "Individual Evaluation complete!\n";

        //cout << maxFitness << endl;

        individual->reward(fitness);

        if (fitness >= maxFitness*.95)
        {
            cout << "PROBLEM DOMAIN SOLVED!!!\n";
        }
		*/
    }

    
    void SoftbotsExperiment::printNetworkCPPN(shared_ptr<const NEAT::GeneticIndividual> individual)
    {
      cout << "Printing cppn network" << endl;
      ofstream network_file;        
      network_file.open ("networkCPPN-ThatSolvedTheProblem.txt", ios::trunc );
      
      NEAT::FastNetwork <double> network = individual->spawnFastPhenotypeStack <double>();        //JMC: this creates the network CPPN associated with this individual used to produce the substrate (neural network)

      network_file << "num links:" << network.getLinkCount() << endl;
      network_file << "num nodes:" << network.getNodeCount() << endl;
 
      int numLinks = network.getLinkCount();
      int numNodes = network.getNodeCount();
      ActivationFunction activationFunction;

      //print out which node corresponds to which integer (e.g. so you can translate a fromNode of 1 to "x1"  
      map<string,int> localNodeNameToIndex = *network.getNodeNameToIndex();
      for( map<string,int>::iterator iter = localNodeNameToIndex.begin(); iter != localNodeNameToIndex.end(); iter++ ) {
        network_file << (*iter).first << " is node number: " << (*iter).second << endl;
      }
      
      for (size_t a=0;a<numLinks;a++)
      {
          
          NetworkIndexedLink <double> link = *network.getLink(a);
           
          network_file << link.fromNode << "->" << link.toNode << " : " << link.weight << endl;
      }
      for (size_t a=0;a<numNodes;a++)
      {          
          activationFunction = *network.getActivationFunction(a);           
          network_file << " activation function " << a << ": ";
          if(activationFunction == ACTIVATION_FUNCTION_SIGMOID) network_file << "ACTIVATION_FUNCTION_SIGMOID";
          if(activationFunction == ACTIVATION_FUNCTION_SIN) network_file << "ACTIVATION_FUNCTION_SIN";
          if(activationFunction == ACTIVATION_FUNCTION_COS) network_file << "ACTIVATION_FUNCTION_COS";
          if(activationFunction == ACTIVATION_FUNCTION_GAUSSIAN) network_file << "ACTIVATION_FUNCTION_GAUSSIAN";
          if(activationFunction == ACTIVATION_FUNCTION_SQUARE) network_file << "ACTIVATION_FUNCTION_SQUARE";
          if(activationFunction == ACTIVATION_FUNCTION_ABS_ROOT) network_file << "ACTIVATION_FUNCTION_ABS_ROOT";
          if(activationFunction == ACTIVATION_FUNCTION_LINEAR) network_file << "ACTIVATION_FUNCTION_LINEAR";
          if(activationFunction == ACTIVATION_FUNCTION_ONES_COMPLIMENT) network_file << "ACTIVATION_FUNCTION_ONES_COMPLIMENT";
          if(activationFunction == ACTIVATION_FUNCTION_END) network_file << "ACTIVATION_FUNCTION_END";
          network_file << endl;
      }

      network_file.close();

      return;
    }

    Experiment* SoftbotsExperiment::clone()
    {
        SoftbotsExperiment* experiment = new SoftbotsExperiment(*this);

        return experiment;
    }
    
	
	bool SoftbotsExperiment::converged(int generation) {
		if(generation == convergence_step)
			return true;
		return false;
	}		

	void SoftbotsExperiment::writeAndTeeUpParentsOrderFile() {		//this lets django know what the order in the vector was of each parent for each org in the current gen
		string outFilePrefix = NEAT::Globals::getSingleton()->getOutputFilePrefix();
		std::ostringstream parentFilename;
		std::ostringstream parentFilenameTemp;
		std::ostringstream parentFilenameCmd;

		parentFilename << outFilePrefix << "-parents";
		parentFilenameTemp << parentFilename.str() << ".tmp";

		FILE* file;
		file = fopen(parentFilenameTemp.str().c_str(), "w");		
		if (!file) 
		{
			cout << "could not open parent order file" << endl;
			exit(33);
		}

        bool parentFound = false;
		for(int s=0;s< group.size();s++)
		{
			shared_ptr<NEAT::GeneticIndividual> individual = group[s];
			individual->setOrder(s); //tells each org what order in the vector they were in when the were evaluated (tees up the info for the next time)
			int parent1Order = individual->getParent1Order();
			int parent2Order = individual->getParent2Order();
			
			PRINT(parent1Order);
			PRINT(parent2Order);
			
			//write info 
			if(parent1Order > -1)  fprintf(file, "%i ", parent1Order);
			if(parent2Order > -1)  fprintf(file, "%i ", parent2Order);
		    fprintf(file, "\n");				
		
		    if(parent1Order > -1 || parent2Order > -1) parentFound = true;
		    PRINT(parentFound);
						
		}
		fclose(file);
		//mv the temp file to the non-temp name
		if(parentFound) //only print the file if we found a parent (i.e. it is not the first gen)
        {
				cout << "CREATING PARENTS FILE" << endl;
				parentFilenameCmd << "mv " << parentFilenameTemp.str() << " " << parentFilename.str();
				int result = ::system(parentFilenameCmd.str().c_str());
				(void) result;
			
        }
		
	}

    
    int SoftbotsExperiment::writeVoxelyzeFile( CArray3Df ArrayForVoxelyze, CArray3Df ConductiveArray, CArray3Df ActuatedArray, string individualID, shared_ptr<NEAT::GeneticIndividual> individual)
	{
		ofstream md5file;
		md5file.open ("md5sumTMP.txt");		

		std::ostringstream stiffness;
		stiffness << (int(NEAT::Globals::getSingleton()->getParameterValue("MaxStiffness")));
		
		//add in some stuff to allow the addition of obstacles (and an area around the creature for these obstacles)
		int DoObs = (int(NEAT::Globals::getSingleton()->getParameterValue("HasObstacles")));
		int ObsBuf;
		if (DoObs > 0) ObsBuf = 50; else ObsBuf = 0;	//we'll add 50 voxels around the object to allow space (should really make this value a parameter) 

  		ofstream myfile;
		std::ostringstream myFileName;
		myFileName << individualID << "_genome.vxa";
  		myfile.open (myFileName.str().c_str());

  		myfile << "\
<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n\
<VXA Version=\"1.0\">\n\
<Simulator>\n\
<Integration>\n\
<Integrator>0</Integrator>\n\
<DtFrac>0.9</DtFrac>\n\
</Integration>\n\
<Damping>\n\
<BondDampingZ>1</BondDampingZ>\n\
<ColDampingZ>0.8</ColDampingZ>\n\
<SlowDampingZ>0.01</SlowDampingZ>\n\
</Damping>\n\
<Collisions>\n\
<SelfColEnabled>1</SelfColEnabled>\n\
<ColSystem>3</ColSystem>\n\
<CollisionHorizon>2</CollisionHorizon>\n\
</Collisions>\n\
<Features>\n\
<FluidDampEnabled>0</FluidDampEnabled>\n\
<PoissonKickBackEnabled>0</PoissonKickBackEnabled>\n\
<EnforceLatticeEnabled>0</EnforceLatticeEnabled>\n\
</Features>\n\
<SurfMesh>\n\
<CMesh>\n\
<DrawSmooth>1</DrawSmooth>\n\
<Vertices/>\n\
<Facets/>\n\
<Lines/>\n\
</CMesh>\n\
</SurfMesh>\n\
<StopCondition>\n\
<StopConditionType>2</StopConditionType>\n\
<StopConditionValue>" << (float(NEAT::Globals::getSingleton()->getParameterValue("SimulationTime"))) << "</StopConditionValue>\n\
</StopCondition>\n\
<GA>\n\
<WriteFitnessFile>1</WriteFitnessFile>\n\
<FitnessFileName>" << individualID << "_fitness.xml</FitnessFileName>\n\
</GA>\n\
</Simulator>\n\
<Environment>\n\
<Fixed_Regions>\n\
<NumFixed>0</NumFixed>\n\
</Fixed_Regions>\n\
<Forced_Regions>\n\
<NumForced>0</NumForced>\n\
</Forced_Regions>\n\
<Gravity>\n\
<GravEnabled>1</GravEnabled>\n\
<GravAcc>-27.468</GravAcc>\n\
<FloorEnabled>1</FloorEnabled>\n\
</Gravity>\n\
<Thermal>\n\
<TempEnabled>1</TempEnabled>\n\
<TempAmp>39</TempAmp>\n\
<TempBase>25</TempBase>\n\
<VaryTempEnabled>1</VaryTempEnabled>\n\
<TempPeriod>0.025</TempPeriod>\n\
</Thermal>\n\
</Environment>\n\
<VXC Version=\"0.93\">\n\
<Lattice>\n\
<Lattice_Dim>" << VoxelSize << "</Lattice_Dim>\n\
<X_Dim_Adj>1</X_Dim_Adj>\n\
<Y_Dim_Adj>1</Y_Dim_Adj>\n\
<Z_Dim_Adj>1</Z_Dim_Adj>\n\
<X_Line_Offset>0</X_Line_Offset>\n\
<Y_Line_Offset>0</Y_Line_Offset>\n\
<X_Layer_Offset>0</X_Layer_Offset>\n\
<Y_Layer_Offset>0</Y_Layer_Offset>\n\
</Lattice>\n\
<Voxel>\n\
<Vox_Name>BOX</Vox_Name>\n\
<X_Squeeze>1</X_Squeeze>\n\
<Y_Squeeze>1</Y_Squeeze>\n\
<Z_Squeeze>1</Z_Squeeze>\n\
</Voxel>\n\
<Palette>\n";
		myfile << "\
<Material ID=\"1\">\n\
<MatType>0</MatType>\n\
<Name>ActuatedConductive</Name>\n\
<Display>\n\
<Red>1</Red>\n\
<Green>0.5</Green>\n\
<Blue>0</Blue>\n\
<Alpha>1</Alpha>\n\
</Display>\n\
<Mechanical>\n\
<MatModel>0</MatModel>\n\
<Elastic_Mod>1e+007</Elastic_Mod>\n\
<Plastic_Mod>0</Plastic_Mod>\n\
<Yield_Stress>0</Yield_Stress>\n\
<FailModel>0</FailModel>\n\
<Fail_Stress>0</Fail_Stress>\n\
<Fail_Strain>0</Fail_Strain>\n\
<Density>1e+006</Density>\n\
<Poissons_Ratio>0.35</Poissons_Ratio>\n\
<CTE>0.01</CTE>\n\
<uStatic>1</uStatic>\n\
<uDynamic>0.5</uDynamic>\n\
<IsConductive>1</IsConductive>\n\
<RepolarizationTime>0.125</RepolarizationTime>\n\
<DepolarizationTime>0.0625</DepolarizationTime>\n\
</Mechanical>\n\
</Material>\n";
		myfile << "\
<Material ID=\"2\">\n\
<MatType>0</MatType>\n\
<Name>PassiveConductive</Name>\n\
<Display>\n\
<Red>1</Red>\n\
<Green>0.7</Green>\n\
<Blue>0</Blue>\n\
<Alpha>0.5</Alpha>\n\
</Display>\n\
<Mechanical>\n\
<MatModel>0</MatModel>\n\
<Elastic_Mod>1e+007</Elastic_Mod>\n\
<Plastic_Mod>0</Plastic_Mod>\n\
<Yield_Stress>0</Yield_Stress>\n\
<FailModel>0</FailModel>\n\
<Fail_Stress>0</Fail_Stress>\n\
<Fail_Strain>0</Fail_Strain>\n\
<Density>1e+006</Density>\n\
<Poissons_Ratio>0.35</Poissons_Ratio>\n\
<CTE>0.00</CTE>\n\
<uStatic>1</uStatic>\n\
<uDynamic>0.5</uDynamic>\n\
<IsConductive>1</IsConductive>\n\
<RepolarizationTime>0.125</RepolarizationTime>\n\
<DepolarizationTime>0.0625</DepolarizationTime>\n\
</Mechanical>\n\
</Material>\n";
		myfile << "\
<Material ID=\"3\">\n\
<MatType>0</MatType>\n\
<Name>ActuatedInsulator</Name>\n\
<Display>\n\
<Red>0</Red>\n\
<Green>0</Green>\n\
<Blue>1</Blue>\n\
<Alpha>1</Alpha>\n\
</Display>\n\
<Mechanical>\n\
<MatModel>0</MatModel>\n\
<Elastic_Mod>1e+007</Elastic_Mod>\n\
<Plastic_Mod>0</Plastic_Mod>\n\
<Yield_Stress>0</Yield_Stress>\n\
<FailModel>0</FailModel>\n\
<Fail_Stress>0</Fail_Stress>\n\
<Fail_Strain>0</Fail_Strain>\n\
<Density>1e+006</Density>\n\
<Poissons_Ratio>0.35</Poissons_Ratio>\n\
<CTE>0.01</CTE>\n\
<uStatic>1</uStatic>\n\
<uDynamic>0.5</uDynamic>\n\
<IsConductive>0</IsConductive>\n\
<RepolarizationTime>0.125</RepolarizationTime>\n\
<DepolarizationTime>0.0625</DepolarizationTime>\n\
</Mechanical>\n\
</Material>\n";
		myfile << "\
<Material ID=\"4\">\n\
<MatType>0</MatType>\n\
<Name>PassiveInsulator</Name>\n\
<Display>\n\
<Red>0.5</Red>\n\
<Green>1</Green>\n\
<Blue>1</Blue>\n\
<Alpha>1</Alpha>\n\
</Display>\n\
<Mechanical>\n\
<MatModel>0</MatModel>\n\
<Elastic_Mod>1e+007</Elastic_Mod>\n\
<Plastic_Mod>0</Plastic_Mod>\n\
<Yield_Stress>0</Yield_Stress>\n\
<FailModel>0</FailModel>\n\
<Fail_Stress>0</Fail_Stress>\n\
<Fail_Strain>0</Fail_Strain>\n\
<Density>1e+006</Density>\n\
<Poissons_Ratio>0.35</Poissons_Ratio>\n\
<CTE>0.00</CTE>\n\
<uStatic>1</uStatic>\n\
<uDynamic>0.5</uDynamic>\n\
<IsConductive>0</IsConductive>\n\
<RepolarizationTime>0.125</RepolarizationTime>\n\
<DepolarizationTime>0.0625</DepolarizationTime>\n\
</Mechanical>\n\
</Material>\n";
		myfile << "\
<Material ID=\"5\">\n\
<MatType>0</MatType>\n\
<Name>PassivePacemaker</Name>\n\
<Display>\n\
<Red>1</Red>\n\
<Green>0</Green>\n\
<Blue>0</Blue>\n\
<Alpha>1</Alpha>\n\
</Display>\n\
<Mechanical>\n\
<MatModel>0</MatModel>\n\
<Elastic_Mod>1e+007</Elastic_Mod>\n\
<Plastic_Mod>0</Plastic_Mod>\n\
<Yield_Stress>0</Yield_Stress>\n\
<FailModel>0</FailModel>\n\
<Fail_Stress>0</Fail_Stress>\n\
<Fail_Strain>0</Fail_Strain>\n\
<Density>1e+006</Density>\n\
<Poissons_Ratio>0.35</Poissons_Ratio>\n\
<CTE>0.00</CTE>\n\
<uStatic>1</uStatic>\n\
<uDynamic>0.5</uDynamic>\n\
<IsConductive>1</IsConductive>\n\
<IsPacemaker>1</IsPacemaker>\n\
<RepolarizationTime>0.125</RepolarizationTime>\n\
<DepolarizationTime>0.0625</DepolarizationTime>\n\
</Mechanical>\n\
</Material>\n";
		myfile << "\
</Palette>\n\
<Structure Compression=\"LONG_READABLE\">\n\
<X_Voxels>" << num_x_voxels+(2*ObsBuf) << "</X_Voxels>\n\
<Y_Voxels>" << num_y_voxels+(2*ObsBuf) << "</Y_Voxels>\n\
<Z_Voxels>" << num_z_voxels << "</Z_Voxels>\n\
<Data>\n\
<Layer><![CDATA[";


//some stuff for circular obstacles
float centerX, centerY, ObsDistDes, CurDist;
centerX=(num_x_voxels+(2*ObsBuf))/2;
centerY=(num_y_voxels+(2*ObsBuf))/2;
float ObsClearance = 1.5;	//the number of voxels away from the creature we want the obstacle
float ObsThickness = 1; //how many voxels thick is it?
ObsDistDes=sqrt(pow(num_x_voxels/2,2)+pow(num_y_voxels/2,2))+ObsClearance;	

int v=0;
for (int z=0; z<num_z_voxels; z++)
{
	for (int y=0; y<num_y_voxels+(2*ObsBuf); y++)
	{
		for (int x=0; x<num_x_voxels+(2*ObsBuf); x++)
		{

			// NOTE: this uses material 0 as passive, material 1 as active phase 1, and material 2 as active phase 2
			/*
			if(ContinuousArray[v] >= Phase1Array[v] && ContinuousArray[v] >= Phase2Array[v] && ContinuousArray[v] >= PassiveArray[v]) myfile << 0;
			else if(PassiveArray[v] >= ContinuousArray[v] && PassiveArray[v] >= Phase2Array[v] && PassiveArray[v] >= Phase1Array[v]) myfile << 1;
			else if(Phase1Array[v] >= ContinuousArray[v] && Phase1Array[v] >= Phase2Array[v] && Phase1Array[v] >= PassiveArray[v]) myfile << 2;
			else if(Phase2Array[v] >= ContinuousArray[v] && Phase2Array[v] >= Phase1Array[v] && Phase2Array[v] >= PassiveArray[v]) myfile << 3;
			*/
			// change "=" to random assignment? currently biases passive, then phase 1, then phase 2 if all equal

			CurDist=sqrt(pow(x-centerX,2)+pow(y-centerY,2));
			
			//if we're within the area occupied by the creature, format the voxels appropriately, else add in zeros
			if ((y>=ObsBuf) && (y<ObsBuf+num_y_voxels) && (x>=ObsBuf) && (x<ObsBuf+num_x_voxels))
			{
				myfile << ArrayForVoxelyze[v] << ",";
				md5file << ArrayForVoxelyze[v];
				v++;
			}
			//if this is a place where we want an obstacle, place it
			else if ((DoObs > 0) && (z==0) && (
				((CurDist>ObsDistDes) && (CurDist<ObsDistDes+ObsThickness))	/*a first concentric circle*/
				|| ((CurDist>ObsDistDes*1.5) && (CurDist<ObsDistDes*1.5+ObsThickness))	/*a second concentric circle*/
				|| ((CurDist>ObsDistDes*2) && (CurDist<ObsDistDes*2+ObsThickness))	/*a third concentric circle*/
				))
			{
				myfile << 5;
				md5file << 5;
			}
			//o.w., no voxel is placed 
			else 
			{
				myfile << 0;
				md5file << 0;
			}

		}
	}
	if (z<num_z_voxels-1) {myfile << "]]></Layer>\n<Layer><![CDATA["; md5file << "\n";}
}
myfile << "]]></Layer>\n\
</Data>\n\
</Structure>\n\
</VXC>\n\
</VXA>\n\
<!-- speciesID: " <<  individual->getSpeciesID() << " -->\n\
<!-- parent1md5: " <<  individual->getParent1md5() << " -->\n\
<!-- parent2md5: " <<  individual->getParent2md5() << " -->\n\
<!-- nodesAdded: " <<  individual->getNodesAdded() << " -->\n\
<!-- linksAdded: " <<  individual->getLinksAdded() << " -->\n\
<!-- linkWeightsMutated: " <<  individual->getLinkWeightsMutated() << " -->\n\
<!-- linkWeightsNotMutated: " <<  individual->getLinkWeightsNotMutated() << " -->\n\
<!-- linksDemolished: " <<  individual->getLinksDemolished() << " -->\n\
";

  		myfile.close();
		md5file.close();
		
		return 0;
	}		

	CArray3Df SoftbotsExperiment::createArrayForVoxelyze(CArray3Df ContinuousArray, CArray3Df ConductiveArray, CArray3Df ActuatedArray)
	{	
		// cout << "got to createArrayForVoxelyze" << endl;
		CArray3Df ArrayForVoxelyze(num_x_voxels, num_y_voxels, num_z_voxels); //array denoting type of material

		numVoxelsFilled   = 0;
		numVoxelsActuated = 0;
		numConnections    = 0;
		int v=0;
		// std::string materialTypes = NEAT::Globals::getSingleton()->getMaterialTypes();
		// bool motorOn = materialTypes.find("Motor")!=std::string::npos;

		for (int z=0; z<num_z_voxels; z++)
		{
			for (int y=0; y<num_y_voxels; y++)
			{
				for (int x=0; x<num_x_voxels; x++)
				{
					if(ContinuousArray[v] < 0)
					{ 
						ArrayForVoxelyze[v] = 0;
					}
					else
					{
						if (ConductiveArray[v] > 0)
						{
							if (ActuatedArray[v] > 0)
							{
								ArrayForVoxelyze[v] = 1;
							}
							else
							{
								ArrayForVoxelyze[v] = 2;
							}
						}
						else
						{
							if (ActuatedArray[v] > 0)
							{
								ArrayForVoxelyze[v] = 3;
							}
							else
							{
								ArrayForVoxelyze[v] = 4;
							}
						}
					}
					v++;
				}
			}
		}

		ArrayForVoxelyze = makeOneShapeOnly(ArrayForVoxelyze);

		ArrayForVoxelyze[555] = 5;

		return ArrayForVoxelyze;
	}

	CArray3Df SoftbotsExperiment::makeOneShapeOnly(CArray3Df ArrayForVoxelyze)
	{
			//find start value:
			queue<int> queueToCheck;
			list<int> alreadyChecked;
			pair<  queue<int>, list<int> > queueAndList;
			int startVoxel = int(ArrayForVoxelyze.GetFullSize()/2);
			queueToCheck.push(startVoxel);
			alreadyChecked.push_back(startVoxel);
			while (ArrayForVoxelyze[startVoxel] == 0 && !queueToCheck.empty())
				{
					startVoxel = queueToCheck.front();
					queueAndList = circleOnce(ArrayForVoxelyze, queueToCheck, alreadyChecked);
					queueToCheck = queueAndList.first;
					alreadyChecked = queueAndList.second;
				}

			// create only one shape:	
			alreadyChecked.clear();
			alreadyChecked.push_back(startVoxel);
			//queueToCheck.clear();
			while (!queueToCheck.empty()) {queueToCheck.pop();}
			queueToCheck.push(startVoxel);
			while (!queueToCheck.empty())		
			{
				startVoxel = queueToCheck.front();

				queueAndList = circleOnce(ArrayForVoxelyze, queueToCheck, alreadyChecked);

				queueToCheck = queueAndList.first;
				alreadyChecked = queueAndList.second;

			}


			for (int v=0; v<ArrayForVoxelyze.GetFullSize(); v++)
			{
				if (find(alreadyChecked.begin(),alreadyChecked.end(),v)==alreadyChecked.end())
				{
					ArrayForVoxelyze[v]=0;
				}
			}

		return ArrayForVoxelyze;
	}

	pair< queue<int>, list<int> > SoftbotsExperiment::circleOnce(CArray3Df ArrayForVoxelyze, queue<int> queueToCheck, list<int> alreadyChecked)
	{
				int currentPosition = queueToCheck.front();
				queueToCheck.pop();

				int index;

				index = leftExists(ArrayForVoxelyze, currentPosition);

				if (index>0 && find(alreadyChecked.begin(),alreadyChecked.end(),index)==alreadyChecked.end())
				{queueToCheck.push(index);}
				alreadyChecked.push_back(index);
				index = rightExists(ArrayForVoxelyze, currentPosition);

				if (index>0 && find(alreadyChecked.begin(),alreadyChecked.end(),index)==alreadyChecked.end())
				{queueToCheck.push(index);}
				alreadyChecked.push_back(index);
				index = forwardExists(ArrayForVoxelyze, currentPosition);

				if (index>0 && find(alreadyChecked.begin(),alreadyChecked.end(),index)==alreadyChecked.end())
				{queueToCheck.push(index);}
				alreadyChecked.push_back(index);
				index = backExists(ArrayForVoxelyze, currentPosition);

				if (index>0 && find(alreadyChecked.begin(),alreadyChecked.end(),index)==alreadyChecked.end())
				{queueToCheck.push(index);}
				alreadyChecked.push_back(index);
				index = upExists(ArrayForVoxelyze, currentPosition);

				if (index>0 && find(alreadyChecked.begin(),alreadyChecked.end(),index)==alreadyChecked.end())
				{queueToCheck.push(index);}
				alreadyChecked.push_back(index);
				index = downExists(ArrayForVoxelyze, currentPosition);

				if (index>0 && find(alreadyChecked.begin(),alreadyChecked.end(),index)==alreadyChecked.end())
				{queueToCheck.push(index);}
				alreadyChecked.push_back(index);

				return make_pair (queueToCheck, alreadyChecked);
	}

	int SoftbotsExperiment::leftExists(CArray3Df ArrayForVoxelyze, int currentPosition)
	{
//cout << "ArrayForVoxelyze.GetFullSize(): "<< ArrayForVoxelyze.GetFullSize() << endl;
				if (currentPosition % num_x_voxels == 0) 
				{
					return -999;	
				}
				else if (ArrayForVoxelyze[currentPosition - 1] == 0)
				{
					 return -999;
				}

				else
				{
				return (currentPosition - 1);
				}
	}

	int SoftbotsExperiment::rightExists(CArray3Df ArrayForVoxelyze, int currentPosition)
	{
				if (currentPosition % num_x_voxels == num_x_voxels-1) return -999;				
				else if (ArrayForVoxelyze[currentPosition + 1] == 0) return -999;
				else {numConnections++; return (currentPosition + 1);}
	}
			
	int SoftbotsExperiment::forwardExists(CArray3Df ArrayForVoxelyze, int currentPosition)
	{
				if (int(currentPosition/num_x_voxels) % num_y_voxels == num_y_voxels-1) return -999;				
				else if (ArrayForVoxelyze[currentPosition + num_x_voxels] == 0) return -999;
				else {numConnections++; return (currentPosition + num_x_voxels);}
	}

	int SoftbotsExperiment::backExists(CArray3Df ArrayForVoxelyze, int currentPosition)
	{
				if (int(currentPosition/num_x_voxels) % num_y_voxels == 0) return -999;				
				else if (ArrayForVoxelyze[currentPosition - num_x_voxels] == 0) return -999;
				else {numConnections++; return (currentPosition - num_x_voxels);}
	}

	int SoftbotsExperiment::upExists(CArray3Df ArrayForVoxelyze, int currentPosition)
	{
				if (int(currentPosition/(num_x_voxels*num_y_voxels)) % num_z_voxels == num_z_voxels-1) return -999;				
				else if (ArrayForVoxelyze[currentPosition + num_x_voxels*num_y_voxels] == 0) return -999;
				else {numConnections++; return (currentPosition + num_x_voxels*num_y_voxels);}
	}

	int SoftbotsExperiment::downExists(CArray3Df ArrayForVoxelyze, int currentPosition)
	{
				if (int(currentPosition/(num_x_voxels*num_y_voxels)) % num_z_voxels == 0) return -999;				
				else if (ArrayForVoxelyze[currentPosition - num_x_voxels*num_y_voxels] == 0) return -999;
				else {numConnections++; return (currentPosition - num_x_voxels*num_y_voxels);}
	}

	bool SoftbotsExperiment::isNextTo(int posX1, int posY1, int posZ1, int posX2, int posY2, int posZ2)
	{
		// bool nextTo = true;
		if (abs(posX1 - posX2) >1 ) {return false;}
		if (abs(posY1 - posY2) >1 ) {return false;}
		if (abs(posZ1 - posZ2) >1 ) {return false;}
		return true;
	}

	vector<int> SoftbotsExperiment::indexToCoordinates(int v)
	{
		int z = v/(num_x_voxels*num_y_voxels);
		int y = (v-z*num_x_voxels*num_y_voxels)/num_x_voxels;
		int x = v-z*num_x_voxels*num_y_voxels-y*num_x_voxels;
		int tempCoordinates[] = {x,y,z};
		vector<int> coordinates (tempCoordinates,tempCoordinates+3);
		return coordinates;
	}

	int SoftbotsExperiment::coordinatesToIndex(int x, int y, int z)
	{
		return x+y*num_x_voxels+z*num_x_voxels*num_y_voxels;
	}

}
