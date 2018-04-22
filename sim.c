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
#include <string.h>
#include <math.h>
#include <time.h>
#include <semaphore.h> 
#include <sys/time.h>
#include <stdlib.h>

// Vector struct
struct vec {
	double x;
	double y;
	double z;
};


// Planetary struct using vectors 
struct planet {
	double mass; // Intrinsic property

	struct vec p; // Position in vector
	struct vec v; // Velocity in vector
	struct vec a; // Acceleration in vector
};

// Total number of steps to perform
int totalSteps = 1000;

// Create pointers to functions
void forkSoln();
void threadSoln();
struct planet updatePlanet(struct planet* planets[], int active);
struct vec vecAdd(struct vec* v_1, struct vec* v_2);
struct vec delta(struct vec* v_1, struct vec* v_2);
void readCSV(char filename[]);
void *updater(int* planet);
double grav(double m, double r);


// If "stepSize" is 1, then each step is 1 second. Scale as appropriate
int stepSize = 60;
// Ensure all planets are on the same step every syncStep number of steps
int syncStep = 500;

// Storage for solar system
// 0 is sun, 1 is mercury, etc
// Pluto IS a planet. Ignore the NASA illuminati propoganda!
struct planet solarSystem[9];

// Planets ready to go?
sem_t sync; 

int main (int argc, char *argv[]) {

	// Add in timing monitoring TODO

	// Read in starting data at given time
	printf("Reading in data\n");
	readCSV("startData.csv");
	printf("Finished reading data\n");

	// Save state TODO (hard to copy arrays in C)
	// struct planet startData[9] = solarSystem;


	// Run one solution at a time to compare

	// Fork based solution
	// forkSoln();

	// Reset global storage of solarSystem TODO see above
	// solarSystem = startData;

	// Thread based solution
	threadSoln();

	// Test cases TODO



	// Do various timing comparisons to get the "interesting" data for the project TODO


}

// Add two vectors (AKA sets of polar coordinates) together.
struct vec vecAdd(struct vec* v_1, struct vec* v_2) {
	struct vec sum;
	sum.x = v_1->x + v_2->x;
	sum.y = v_1->y + v_2->y;
	sum.z = v_1->z + v_2->z;

	return sum;
}

// Difference between two vectors
struct vec delta(struct vec* v_1, struct vec* v_2) {
	struct vec diff;
	diff.x = v_1->x - v_2->x;
	diff.y = v_1->y - v_2->y;
	diff.z = v_1->z - v_2->z;

	return diff;
}


// Adapted from http://c-for-dummies.com/blog/?p=2355
// Reads the given file and places it in the solarSystem
void readCSV(char filename[]) {

	// Temporary hardcoded implementation for the sake of the beta
	solarSystem[0] = (struct planet){.p = (struct vec){.x=-1.068000648301820 * pow(10,6),.y=-4.176802125684930 * pow(10,5),.z=3.084467020687090 * pow(10,4)},.v = (struct vec){.x=9.305300847631915 * pow(10,-3),.y=-1.283176670344807 * pow(10,-2),.z=-1.631528028381386 * pow(10,-4)},.mass=1.989*pow(10,30)};	
	solarSystem[1] = (struct planet){.p = (struct vec){.x=-2.212062175862221 * pow(10,7),.y=-6.682431829610253 * pow(10,7),.z=-3.461601353176080 * pow(10,6)},.v = (struct vec){.x=3.666229236478603 * pow(10,1),.y=-1.230266986781422 * pow(10,1),.z=-4.368336051784789 * pow(10,0)},.mass = 0.33* pow(10,24)};
	solarSystem[2] = (struct planet){.p = (struct vec){.x=-1.085735509178141 * pow(10,8),.y=-3.784200933160055 * pow(10,6),.z=6.190064472977990 * pow(10,6)},.v = (struct vec){.x=8.984651054838754 * pow(10,-1),.y=-3.517203950794635 * pow(10,1),.z=-5.320225582712421 * pow(10,-1)},.mass =4.87 * pow(10,24)};
	solarSystem[3] = (struct planet){.p = (struct vec){.x=-2.627892928682480 * pow(10,7),.y=1.445102393586391 * pow(10,8),.z=3.022818135935813 * pow(10,4)},.v = (struct vec){.x=-2.983052803283506 * pow(10,1),.y=-5.220465685407924 * pow(10,0),.z=-1.014621798034465 * pow(10,-4)},.mass = 5.97 * pow(10,24)};
	solarSystem[4] = (struct planet){.p = (struct vec){.x=2.069270543147017 * pow(10,8),.y=-3.560689745239088 * pow(10,6),.z=-5.147936537447235 * pow(10,6)},.v = (struct vec){.x=1.304308833322233 * pow(10,0),.y=2.628158890420931 * pow(10,1),.z=5.188465740839767 * pow(10,-1)},.mass = 0.642 * pow(10,24)};
	solarSystem[5] = (struct planet){.p = (struct vec){.x=5.978411588543636 * pow(10,8),.y=4.387049129308410 * pow(10,8),.z=-1.520170148446965 * pow(10,7)},.v = (struct vec){.x=-7.892632330758821 * pow(10,0),.y=1.115034520921672 * pow(10,1),.z=1.305097532924950 * pow(10,-1)},.mass = 1898 * pow(10,24)};
	solarSystem[6] = (struct planet){.p = (struct vec){.x=9.576383363062742 * pow(10,8),.y=9.821475305205020 * pow(10,8),.z=-5.518981313640279 * pow(10,7)},.v = (struct vec){.x=-7.419580380567519 * pow(10,0),.y=6.725982472131615 * pow(10,0),.z=1.775012122796475 * pow(10,-1)},.mass = 568 * pow(10,24)};
	solarSystem[7] = (struct planet){.p = (struct vec){.x=2.157706702828831 * pow(10,9),.y=-2.055242911807622 * pow(10,9),.z=-3.559264256520975 * pow(10,7)},.v = (struct vec){.x=4.646953712646178 * pow(10,0),.y=4.614361110490073 * pow(10,0),.z=-4.301340943493193 * pow(10,-2)},.mass = 86.8 * pow(10,24)};
	solarSystem[8] = (struct planet){.p = (struct vec){.x=2.513785419503203 * pow(10,9),.y=-3.739265092576820 * pow(10,9),.z=1.907031792232442 * pow(10,7)},.v = (struct vec){.x=4.475105284920682 * pow(10,0),.y=3.062849397248741 * pow(10,0),.z=-1.667285646337855 * pow(10,-1)},.mass = 102 * pow(10,24)};
	solarSystem[9] = (struct planet){.p = (struct vec){.x=-1.478626340724678 * pow(10,9),.y=-4.182878118662979 * pow(10,9),.z=8.753002592760717 * pow(10,8)},.v = (struct vec){.x=5.271230989790396 * pow(10,0),.y=-2.661751411789654 * pow(10,0),.z=-1.242036206632027 * pow(10,0)},.mass = 0.0146 * pow(10,24)};


	return;

	// Come back to this post-beta
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
	printf("Reading initialized, accessing values\n");
	fflush(stdout);
	while(fgets(buffer,BSIZE,f))
	{
		/* skip name */
		field=strtok(buffer,",");
		/* get x position */
		field=strtok(NULL,",");
		printf(field);
		fflush(stdout);
		solarSystem[i].p.x=atoi(field); // Seg fault here. Field is X(km)-1.068000648301820E+06. 
		// Why does it include 1st and 2nd rows? That's problem a, problem b is parsing
		/* get y position */
		field=strtok(NULL,",");
		solarSystem[i].p.y=atoi(field);
		/* get z position */
		field=strtok(NULL,",");
		solarSystem[i].p.z=atoi(field);
		/* get x vel */
		field=strtok(NULL,",");
		solarSystem[i].v.x=atoi(field);
		/* get y vel */
		field=strtok(NULL,",");
		solarSystem[i].v.y=atoi(field);
		/* get z vel */
		field=strtok(NULL,",");
		solarSystem[i].v.z=atoi(field);
		/* get mass */
		field=strtok(NULL,",");
		solarSystem[i].mass=atoi(field);

		i++;
	}
	printf("Reading finished");
	fflush(stdout);

	/* close file */
	fclose(f);
}



// Updates the properties of the "active" planet using the rest of the 1
// System is passed in as an array of planets, the active planet is determined based on its index in that array
// NOTE: Synchronization does not happen here. The system may occasionally be out-of-sync, but this function doesn't care
struct planet updatePlanet(struct planet* planets[], int active) { 
	int i;
	struct planet activePlanet;
	// Unfortunately need to hardcode in 10 elements

	// TODO Implement sync here. 
	// Sync on 0th tick and then every $syncSteps after

	for(i = 0; i <= 9;i++) {
		if(!(i==active)) {
			struct vec dist = delta(&(planets[active]->p), &(planets[i]->p));
			activePlanet.a.x += copysign(1.0,dist.x) * grav(planets[i]->mass, dist.x); // Copysign to ensure it's the right direction
			activePlanet.a.y += copysign(1.0,dist.y) * grav(planets[i]->mass, dist.y); 
			activePlanet.a.z += copysign(1.0,dist.z) * grav(planets[i]->mass, dist.z); 
		}
	}

	// Update velocity
	activePlanet.v = vecAdd(&(activePlanet.a), &(activePlanet.v));

	// Update positions
	activePlanet.p = vecAdd(&(activePlanet.p), &(activePlanet.v));

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
	int planet[10];

	int i;	
	// Set start values
	for(i = 0; i <= 9;i++) {
		 planet[i] = i;
	}

	// Prepare the child threads
	pthread_t tid[10]; /* the thread identifiers */
	for(i = 0; i <= 9;i++) {
		printf("Starting planet %d\n", i); // WEEEE'VE GOT A SYNC ISSUE!
		// I'm pretty sure planets start trying to calculate deltas on other planets before they're created. Need to implement a sync to get even the first step off the ground.
		fflush(stdout);
		// Seg fault after reading in planet 2-5 
		pthread_create(&tid[i], NULL, updater, &planet[i]);
	}	


}


// Takes a planet and handles updates (on the solarSystem) for it while running
void *updater(int* planet) {
	int i;
	while(i < totalSteps) {
		// Handle updating here to minimize conflicts where velocity/position changes halfway through reading it.
		solarSystem[*planet] = updatePlanet(&solarSystem, *planet);
		i++;
	}
}