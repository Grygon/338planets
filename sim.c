// EECS 338 Final Project
// Jakob Rubin jbr65
// Jaafar Bennani jxb696
// Planetary simulation (simplified to two dimensions)
// Base unit systems:
// 		Time: s
//		Distance: km
//		Mass: kg

#include <stdio.h>
#include <time.h>

// Create pointers to functions
void forkSoln();
void *threadSoln();
struct planet updatePlanet(struct planet system[]);
struct vec vecAdd(struct vec v_1, struct vec v_2);

// Vector struct
struct vec {
	double x;
	double y;
	double z;
}


// Planetary struct using vectors 
struct planet {
	double mass; // Intrinsic property

	struct vec x; // Position in vector
	struct vec v; // Velocity in vector
	struct vec a; // Acceleration in vector
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

// Add two vectors (AKA sets of polar coordinates) together.
struct vec vecAdd(struct vec v_1, struct vec v_2) {
	struct vec sum;
	sum.x = v_1.x + v_2.x;
	sum.y = v_1.y + v_2.y;
	sum.z = v_1.z + v_2.z;

	return sum;
}

// Difference between two vectors
struct vec delta(struct vec v_1, struct vec v_2) {
	struct vec diff;
	sum.x = v_1.x - v_2.x;
	sum.y = v_1.y - v_2.y;
	sum.z = v_1.z - v_2.z;

	return sum;
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