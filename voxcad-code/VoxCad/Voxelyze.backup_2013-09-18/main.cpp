#include <iostream>
#include "VX_Object.h"
#include "VX_Environment.h"
#include "VX_Sim.h"
#include "VX_SimGA.h"


int main(int argc, char *argv[]){

	char* InputFile;

	//create the three main objects
	CVX_Object Object;
	CVX_Environment Environment;
	CVX_Sim Simulator;
	//CVX_SimGA SimulatorGA;
	long int Step = 0;
	vfloat Time = 0.0; //in seconds
	Vec3D<> Vox0Pos;	//for reporting the position of the first voxel
	bool print_scrn;

	//first, parse inputs. Use as: -f followed by the filename of the .vxa file that describes the simulation. Can also follow this with -p to cause console output to occur
	if (argc < 3) { // Check the value of argc. If not enough parameters have been passed, inform user and exit.
		std::cout << "\nInput file required. Quitting.\n";
		return(0);	//return, indicating via code (0) that we did not complete the simulation
		} 
	else { // if we got enough parameters...
		for (int i = 1; i < argc; i++) 
			{ 
			if (strcmp(argv[i],"-f") == 0) {
				InputFile = argv[i + 1];	// We know the next argument *should* be the filename:
				} 
			else if (strcmp(argv[i],"-p") == 0) {
				// Simulator.print_scrn=true;	//decide if output to the console is desired
				/*Simulator.*/print_scrn=true;	//decide if output to the console is desired
				}
			}
		}

	//setup main object
	Simulator.pEnv = &Environment;	//connect Simulation to environment
	Environment.pObj = &Object;		//connect environment to object
	
	//import the configuration file
	if (!Simulator.LoadVXAFile(InputFile)){
		if (/*Simulator.*/print_scrn) std::cout << "\nProblem importing VXA file. Quitting\n";
		return(0);	//return, indicating via code (0) that we did not complete the simulation
		}

	std::string ReturnMessage;
	if (/*Simulator.*/print_scrn) std::cout << "\nImporting Environment into simulator...\n";
	Simulator.Import(&Environment, 0, &ReturnMessage);
	if (/*Simulator.*/print_scrn) std::cout << "Simulation import return message:\n" << ReturnMessage << "\n";

	// Simulator.pEnv->UpdateCurTemp(Time, &Simulator.LocalVXC);	//set the starting temperature 
	Simulator.pEnv->UpdateCurTemp(Time);	//set the starting temperature (nac: pointer removed for debugging)
	Simulator.IniCM=Simulator.GetCM();	//set the starting CM position

	while (Step<100000 && Time<1) //nac: removed for DEBUGGING!!!: while (Step<Simulator.SimStopStep && Time<Simulator.SimStopTime)
		{
		//do some reporting via the stdoutput if required
		if (Step%100 == 0 && /*Simulator.*/print_scrn){ //Only output every n time steps
			//std::cout << "Step: " << i << " \tCoM X: " << Simulator.GetCM().x*1000 << "mm" << "\tCoM Y: " << Simulator.GetCM().y*1000 << "mm" << "\tCoM Z: " << Simulator.GetCM().z*1000 << "mm\n"; //note that computing the COM requires looping through the entire voxel array, and is therefore not very efficient
			Vox0Pos=Simulator.VoxArray[0].GetCurPos()*1000; //*1000 //position in mm
			std::cout << "Time: " << Time << " \tVox 0 X: " << Vox0Pos.x << "mm" << "\tVox 0 Y: " << Vox0Pos.y << "mm" << "\tVox 0 Z: " << Vox0Pos.z << "mm\n";	//just display the position of the first voxel in the voxelarray
			}

		//do the actual simulation step
		Simulator.TimeStep(&ReturnMessage);
		//TODO: add in error reporting if there is some problem with the timestep
		Step += 1;	//increment the step counter
		Time += Simulator.dt;	//update the sim tim after the step
		// Simulator.pEnv->UpdateCurTemp(Time, &Simulator.LocalVXC);	//pass in the global time, and a pointer to the local object so its material temps can be modified
		Simulator.pEnv->UpdateCurTemp(Time);	//pass in the global time, and a pointer to the local object so its material temps can be modified (nac: pointer removed for debugging)
		// nac: not needed for non-cyclic actuations:  Simulator.pEnv->UpdateCurTime(Time);	//just pass in the Simulation time to the Environment (currently used only for calculating phase of actuations)
		}

	//generate results file, if required
	//if (SimulatorGA.WriteFitnessFile) SimulatorGA.SaveResultFile(SimulatorGA.FitnessFileName);

	return 1;	//code for successful completion
	}