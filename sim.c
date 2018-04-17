// EECS 338 Final Project
// Jakob Rubin jbr65
// Jaafar Bennani jxb696
// Planetary simulation

#include <stdio.h>
#include <time.h>

// Create pointers to functions
void forkSoln();
void *threadSoln();
struct planet updatePlanet(struct planet system[]);


// Planetary struct using polar coordinates
struct planet {
	double mass; // Intrinsic property

	double deg; // Angle 
	double r; // Radius
	double v_deg; // Degree component of velocity
	double v_r; // Degree component of velocity
	double a_deg; // Degree component of acceleration
	double a_r; // Degree component of acceleration

}

// If "stepSize" is 1, then each step is 1 second. Scale as appropriate
int stepSize = 60;

// Storage for solar system
// 0 is sun, 1 is mercury, etc
// Pluto IS a planet. Ignore the NASA illuminati propoganda!
struct planet solarSystem[9];

int main (int argc, char *argv[]) {

	// Add in timing monitoring TODO

	// Read in starting data at given time TODO
	struct planet startData[9];


	solarSystem = startData;

	// Run one solution at a time to compare

	// Fork based solution
	forkSoln();

	// Reset global storage of solarSystem
	solarSystem = startData;

	// Thread based solution
	threadSoln();

	// Test cases TODO



	// Do various timing comparisons to get the "interesting" data for the project TODO


}


// Updates the properties of the "active" planet using the rest of the system
// System is passed in as an array of planets, the active planet is determined based on its index in that array
// NOTE: Synchronization does not happen here. The system may occasionally be out-of-sync, but this function doesn't care
struct planet updatePlanet(struct planet system[], int active) { 

}


// Solution using fork
void forkSoln() {

}


// Solution using POSIX threads
void threadSoln() {

}