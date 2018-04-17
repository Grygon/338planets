// EECS 338 Final Project
// Jakob Rubin jbr65
// Jaafar Bennani jxb696
// Planetary simulation

#include <stdio.h>
#include <time.h>

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

int main (int argc, char *argv[]) {
	// If "stepSize" is 1, then each step is 1 second. Scale as appropriate
	int stepSize = 60;

	// Read in starting data at given time



}