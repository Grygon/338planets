// EECS 338 Final Project
// Jakob Rubin jbr65
// Jaafar Bennani jxb696
// Planetary simulation (simplified to two dimensions)
// Base unit systems:
// 		Time: s
//		Distance: km
//		Mass: kg

#include <pthread.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <semaphore.h> 
#include <sys/time.h>
#include <stdlib.h>

// Total number of steps to perform
int totalSteps = 1000;

// Create pointers to functions
void forkSoln();
void *threadSoln();
struct planet updatePlanet(planet planetSystem[]);
struct vec vecAdd(struct vec v_1, struct vec v_2);
void readCSV(planet planetSystem[]);
void updater(int planet);

// Vector struct
typedef struct {
	double x;
	double y;
	double z;
} vec;


// Planetary struct using vectors 
typedef struct {
	double mass; // Intrinsic property

	struct vec p; // Position in vector
	struct vec v; // Velocity in vector
	struct vec a; // Acceleration in vector
} planet ;

// If "stepSize" is 1, then each step is 1 second. Scale as appropriate
int stepSize = 60;

// Storage for solar system
// 0 is sun, 1 is mercury, etc
// Pluto IS a planet. Ignore the NASA illuminati propoganda!
struct planet solarSystem[9];

int main (int argc, char *argv[]) {

	// Add in timing monitoring TODO

	// Read in starting data at given time
	readCSV("startData.csv");


	// Run one solution at a time to compare

	// Fork based solution
	// forkSoln();

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


// Adapted from http://c-for-dummies.com/blog/?p=2355
// Reads the given file and places it in the solarSystem
void readCSV(char filename[]) {
	int BSIZE = 80;
	char buffer[BSIZE];
	FILE *f;
	char *field;

	/* open the CSV file */
	f = fopen(filename,"r");
	if( f == NULL)
	{
		printf("Unable to open file '%s'\n",filename);
		exit(1);
	}

	// Index for read body
	int i = 0;

	/* process the data */
	/* the file contains 8 fields */
	// TODO implement parsing for exponents
	while(fgets(buffer,BSIZE,f))
	{
		/* skip name */
		field=strtok(buffer,",");
		/* get x position */
		field=strtok(NULL,",");
		solarSystem[i]->p->x=atoi(field);
		/* get y position */
		field=strtok(NULL,",");
		solarSystem[i]->p->y=atoi(field);
		/* get z position */
		field=strtok(NULL,",");
		solarSystem[i]->p->z=atoi(field);
		/* get x vel */
		field=strtok(NULL,",");
		solarSystem[i]->v->x=atoi(field);
		/* get y vel */
		field=strtok(NULL,",");
		solarSystem[i]->v->y=atoi(field);
		/* get z vel */
		field=strtok(NULL,",");
		solarSystem[i]->v->z=atoi(field);
		/* get mass */
		field=strtok(NULL,",");
		solarSystem[i]->mass=atoi(field);

		i++;
	}

	/* close file */
	fclose(f);

	return(0);
}



// Updates the properties of the "active" planet using the rest of the 1
// System is passed in as an array of planets, the active planet is determined based on its index in that array
// NOTE: Synchronization does not happen here. The system may occasionally be out-of-sync, but this function doesn't care
struct planet updatePlanet(struct planet planets[], int active) { 
	int i;
	struct planet activePlanet;
	// Unfortunately need to hardcode in 10 elements
	for(i = 0; i < 10;i++) {
		if(!(i==active)) {
			dist = delta(planets[active]->p, planets[i]->p);
			activePlanet->a->x += copysign(1.0,dist->x) * grav(planets[i]->mass, dist->x); // Copysign to ensure it's the right direction
			activePlanet->a->y += copysign(1.0,dist->y) * grav(planets[i]->mass, dist->y); 
			activePlanet->a->z += copysign(1.0,dist->z) * grav(planets[i]->mass, dist->z); 
		}
	}

	// Update velocity
	vecAdd(activePlanet->a, activePlanet->v);

	// Update positions
	vecAdd(activePlanet->p, activePlanet->v);

	return activePlanet;
}

// Calculates the acceleration due to an object of mass m at distance r
// Returns in km/s
double grav(double m, double r) {
	return 6.674 * pow(10, -20) * m / (r * r);
}

// Solution using fork
void forkSoln() {

}


// Solution using POSIX threads
void threadSoln() {

	// Array for planet values (for ease of passing, it's an array)
	double planet[10];

	// Set start values
	for(i = 0; i < 10;i++) {
		 planet[i] = i;
	}

	// Prepare the child threads
	pthread_t tid[10]; /* the thread identifiers */
	for(i = 0; i < 10;i++) {
		pthread_create(&tid[i], NULL, updater, &planet[i]);
	}


}


// Takes a planet and handles updates (on the solarSystem) for it while running
void updater(int planet) {
	int i;
	while(i < totalSteps) {
		// Handle updating here to minimize conflicts where velocity/position changes halfway through reading it.
		solarSystem[planet] = updatePlanet(solarSystem, planet);
	}

}