// EECS 338 Final Project
// Jakob Rubin jbr65
// Jaafar Bennani jxb696
// Planetary simulation (simplified to two dimensions)

#include <stdio.h>
#include <time.h>

// Create pointers to functions
void forkSoln();
void *threadSoln();
struct planet updatePlanet(struct planet system[]);
struct c polarAdd(struct c c_1, struct c c_2);

// Polar struct
struct c {
	double deg;
	double r;
}


// Planetary struct using polar coordinates
struct planet {
	double mass; // Intrinsic property

	struct c x; // Position in polar
	struct c v; // Velocity in polar
	struct c a; // Acceleration in polar
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
struct c polarAdd(struct c c_1, struct c c_2) {
	struct c final;
	final.r = sqrt(c_1.r^2 + c_2.r^2 + 2 * c_1.r * c_2.r * cos(c_2.deg - c_1.deg));
	final.deg = c_1.deg + arctan(c_2.r * sin(c_2.deg - c_1.deg) / (c_1.r + c_2.r * cos(c_2.deg - c_1.deg)));
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