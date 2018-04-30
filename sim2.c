/* EECS 338 Final Project
 * Jakob Rubin (jbr65)
 * Jaafar Bennani (jxb696)
 * Planetary simulation (simplified to two dimensions)
 * Base unit systems:
 * 		Time: s
 *		Distance: km
 *		Mass: kg 
 */

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <semaphore.h> 
#include <sys/time.h>
#include <stdlib.h>

// Vector struct
typedef struct Vecs {
	long double x;
	long double y;
	long double z;
} Vec;

Vec vecInit() {
	Vec blank;
	blank.x = 0;
	blank.y = 0;
	blank.z = 0;
	return blank;
}

// Planetary struct using vectors 
typedef struct Planets {
	long double mass; // Intrinsic property

	Vec p; // Position in vector
	Vec v; // Velocity in vector
	Vec a; // Acceleration in vector
} Planet;

Planet planetInit() {
	Planet blank;
	blank.mass = 0;
	blank.p = vecInit();
	blank.v = vecInit();
	blank.a = vecInit();
	return blank;
}

// Total number of steps to perform
int totalSteps = 2678400; // 1 month is 2678400. Currently results in huge error after 1 month of sim. I wonder if we need to take relativity into account. That'd be fun.

long double expVal = -98567773.36025174;

// Create pointers to functions
void forkSoln();
void threadSoln();
Vec newVec(int x, int y, int z);
void printVec(Vec v);
Planet newPlanet(long double mass, Vec p, Vec v, Vec a);
Vec vecAdd(Vec v1, Vec v2);
void vecAddi(Vec *res, Vec v1, Vec v2);
void vecAddii(Vec *v1, Vec v2);
void vecScale(Vec *v, long double scale);
Vec vecDir(Vec v);
void vecDiri(Vec *v);
Vec delta(Vec v1, Vec v2);
void vecSubi(Vec *res, Vec v1, Vec v2);
long double vecMag(Vec v);
long double grav(double m, long double r);
void updatePlanet(int active);
void readCSV(char filename[]);
void * updater(int *planet);


// If "stepSize" is 1, then each step is 1 second. Scale as appropriate
// Not currently implemented. TODO
int stepSize = 1;
// Ensure all planets are on the same step every syncStep number of steps
int syncStep = 1;

// Using a "brand new" thing I found, sync barriers!
// While these weren't covered in class, they fill our need perfectly.
pthread_barrier_t syncBarrier;

// Storage for solar system
// 0 is sun, 1 is mercury, etc
// Pluto IS a planet. Ignore the NASA illuminati propoganda!
Planet solarSystem[9];

int main (int argc, char *argv[]) {
    
    Vec v1 = newVec(1, 2, 3);
    vecAddi(&v1, v1, newVec(2,4,6));
    printVec(v1);
    printf("%Lf\n", vecMag(v1));

    printf("%Lf\n", 3 / vecMag(v1));

    vecScale(&v1, (long double) 1/3);
    printVec(v1);

    Vec e = vecDir(v1);
    printVec(e);
    printf("%Lf\n", vecMag(e));
    printVec(v1);

/*
	// Add in timing monitoring TODO

	// Read in starting data at given time
	printf("Reading in data\n");
	readCSV("startData.csv");
	printf("Earth's location (in x) is: %Lf \n", solarSystem[3].p.x);
	printf("Finished reading data\n");

	// Create sync barrier
	pthread_barrier_init(&syncBarrier, NULL, 10);

	// Save state TODO (hard to copy arrays in C)
	// Planet startData[9] = solarSystem;


	// Run one solution at a time to compare

	// Fork based solution
	// forkSoln();

	// Reset global storage of solarSystem TODO see above
	// solarSystem = startData;

	// Thread based solution
	threadSoln();

	// Test cases TODO



	// Do various timing comparisons to get the "interesting" data for the project TODO

*/
}

Vec newVec(int vx, int vy, int vz) {
    return (Vec){.x = vx, .y = vy, .z = vz};
}

void printVec(Vec v) {
    printf("(%Lf, %Lf, %Lf)\n", v.x, v.y, v.z);
}

Planet newPlanet(long double mass, Vec p, Vec v, Vec a){
    return (Planet){.mass = mass, .p = p, .v = v, .a = a};
}


// Add two vectors (AKA sets of polar coordinates) together.
Vec vecAdd(Vec v1, Vec v2) {
	return newVec(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

void vecAddi(Vec *res, Vec v1, Vec v2) {
	res->x = v1.x + v2.x;
    res->y = v1.y + v2.y;
    res->z = v1.z + v2.z;
}

void vecAddii(Vec *v1, Vec v2) {
	v1->x += v2.x;
    v1->y += v2.y;
    v1->z += v2.z;
}

// Difference between two vectors
Vec delta(Vec v1, Vec v2) {
	return newVec(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

void vecSubi(Vec *res, Vec v1, Vec v2) {
	res->x = v1.x - v2.x;
    res->y = v1.y - v2.y;
    res->z = v1.z - v2.z;
}

void vecScale(Vec *v, long double scale){
    v->x *= scale;
    v->y *= scale;
    v->z *= scale;
}

Vec vecDir(Vec v){
    Vec temp = v;
    vecDiri(&temp);
    return temp;
}

void vecDiri(Vec *v){
    vecScale(v, 1 / vecMag(*v));
}

long double vecMag(Vec v) {
    return sqrtl(v.x * v.x + v.y * v.y + v.z * v.z);
}

long double grav(double m, long double r) {
    return -1 * 6.6740831 * (long double)(pow(10, -20)) * m / (r * r);
}

// Updates the properties of the "active" planet using the rest of the 1
// System is passed in as an array of planets, the active planet is determined based on its index in that array
// NOTE: Synchronization does not happen here. The system may occasionally be out-of-sync, but this function doesn't care
void updatePlanet(int active) { 
	int i;
	Planet *activePlanet = &solarSystem[active];
	
	for(i = 0; i <= 9;i++) {
		if(!(i==active)) {
			Vec dist = delta(solarSystem[active].p, solarSystem[i].p);
            long double distMag = vecMag(dist);
			activePlanet->a = dist;
            vecDiri(&activePlanet->a);
            vecScale(&activePlanet->a, grav(solarSystem[i].mass, distMag));
			}
	}

	// Update velocity
	vecAddii(&activePlanet->v, activePlanet->a);

	// Update positions
	vecAddii(&activePlanet->p, activePlanet->v);
}


// Adapted from http://c-for-dummies.com/blog/?p=2355
// Reads the given file and places it in the solarSystem
// All data thanks to https://ssd.jpl.nasa.gov/horizons.cgi
void readCSV(char filename[]) {

	// Temporary hardcoded implementation for the sake of the beta
	solarSystem[0] = (Planet){.p = (Vec){.x=-1.068000648301820 * pow(10,6),.y=-4.176802125684930 * pow(10,5),.z=3.084467020687090 * pow(10,4)},.v = (Vec){.x=9.305300847631915 * pow(10,-3),.y=-1.283176670344807 * pow(10,-2),.z=-1.631528028381386 * pow(10,-4)},.mass=1.988544*pow(10,30)};	
	solarSystem[1] = (Planet){.p = (Vec){.x=-2.212062175862221 * pow(10,7),.y=-6.682431829610253 * pow(10,7),.z=-3.461601353176080 * pow(10,6)},.v = (Vec){.x=3.666229236478603 * pow(10,1),.y=-1.230266986781422 * pow(10,1),.z=-4.368336051784789 * pow(10,0)},.mass = 0.330103* pow(10,24)};
	solarSystem[2] = (Planet){.p = (Vec){.x=-1.085735509178141 * pow(10,8),.y=-3.784200933160055 * pow(10,6),.z=6.190064472977990 * pow(10,6)},.v = (Vec){.x=8.984651054838754 * pow(10,-1),.y=-3.517203950794635 * pow(10,1),.z=-5.320225582712421 * pow(10,-1)},.mass = 4.13804 * pow(10,24)};
	solarSystem[3] = (Planet){.p = (Vec){.x=-2.627892928682480 * pow(10,7),.y=1.445102393586391 * pow(10,8),.z=3.022818135935813 * pow(10,4)},.v = (Vec){.x=-2.983052803283506 * pow(10,1),.y=-5.220465685407924 * pow(10,0),.z=-1.014621798034465 * pow(10,-4)},.mass = 5.97226 * pow(10,24)};
	solarSystem[4] = (Planet){.p = (Vec){.x=2.069270543147017 * pow(10,8),.y=-3.560689745239088 * pow(10,6),.z=-5.147936537447235 * pow(10,6)},.v = (Vec){.x=1.304308833322233 * pow(10,0),.y=2.628158890420931 * pow(10,1),.z=5.188465740839767 * pow(10,-1)},.mass = 0.642736 * pow(10,24)};
	solarSystem[5] = (Planet){.p = (Vec){.x=5.978411588543636 * pow(10,8),.y=4.387049129308410 * pow(10,8),.z=-1.520170148446965 * pow(10,7)},.v = (Vec){.x=-7.892632330758821 * pow(10,0),.y=1.115034520921672 * pow(10,1),.z=1.305097532924950 * pow(10,-1)},.mass = 1898.5219 * pow(10,24)};
	solarSystem[6] = (Planet){.p = (Vec){.x=9.576383363062742 * pow(10,8),.y=9.821475305205020 * pow(10,8),.z=-5.518981313640279 * pow(10,7)},.v = (Vec){.x=-7.419580380567519 * pow(10,0),.y=6.725982472131615 * pow(10,0),.z=1.775012122796475 * pow(10,-1)},.mass = 568.466 * pow(10,24)};
	solarSystem[7] = (Planet){.p = (Vec){.x=2.157706702828831 * pow(10,9),.y=-2.055242911807622 * pow(10,9),.z=-3.559264256520975 * pow(10,7)},.v = (Vec){.x=4.646953712646178 * pow(10,0),.y=4.614361110490073 * pow(10,0),.z=-4.301340943493193 * pow(10,-2)},.mass = 86.8199 * pow(10,24)};
	solarSystem[8] = (Planet){.p = (Vec){.x=2.513785419503203 * pow(10,9),.y=-3.739265092576820 * pow(10,9),.z=1.907031792232442 * pow(10,7)},.v = (Vec){.x=4.475105284920682 * pow(10,0),.y=3.062849397248741 * pow(10,0),.z=-1.667285646337855 * pow(10,-1)},.mass = 102.4311 * pow(10,24)};
	solarSystem[9] = (Planet){.p = (Vec){.x=-1.478626340724678 * pow(10,9),.y=-4.182878118662979 * pow(10,9),.z=8.753002592760717 * pow(10,8)},.v = (Vec){.x=5.271230989790396 * pow(10,0),.y=-2.661751411789654 * pow(10,0),.z=-1.242036206632027 * pow(10,0)},.mass = 0.0146 * pow(10,24)};
/*
	int i = 0;

	for (i = 0;i <= 9;i++) {
		solarSystem[i].a = vecInit();
	}

	return;

	// Post-beta implementation of CSV reading

	int BSIZE = 80;
	char buffer[BSIZE];
	FILE *f;
	char *field;

	// open the CSV file 
	f = fopen(filename,"r");
	if( f == NULL)
	{
		printf("Unable to open file '%s'\n",filename);
		exit(1);
	}

	// Index for read body
	int i = 0;

	// process the data 
	// the file contains 8 fields 
	// TODO implement parsing for exponents
	printf("Reading initialized, accessing values\n");
	fflush(stdout);
	while(fgets(buffer,BSIZE,f))
	{
		// Skip name
		field=strtok(buffer,",");
		// get x position 
		field=strtok(NULL,",");
		printf(field);
		fflush(stdout);
		solarSystem[i].p.x=atoi(field); // Seg fault here. Field is X(km)-1.068000648301820E+06. 
		// Why does it include 1st and 2nd rows? That's problem a, problem b is parsing
		// get y position 
		field=strtok(NULL,",");
		solarSystem[i].p.y=atoi(field);
		// get z position 
		field=strtok(NULL,",");
		solarSystem[i].p.z=atoi(field);
		// get x vel 
		field=strtok(NULL,",");
		solarSystem[i].v.x=atoi(field);
		// get y vel 
		field=strtok(NULL,",");
		solarSystem[i].v.y=atoi(field);
		// get z vel 
		field=strtok(NULL,",");
		solarSystem[i].v.z=atoi(field);
		// get mass 
		field=strtok(NULL,",");
		solarSystem[i].mass=atoi(field);

		i++;
	}
	printf("Reading finished");
	fflush(stdout);

	// close file 
	fclose(f);
*/

}





// Solution using fork
void forkSoln() {
	// Implemented post-beta. TODO
    
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
	pthread_t tid[10]; // the thread identifiers 
	for(i = 0; i <= 9;i++) {
		printf("Starting body %d\n", i); 
		fflush(stdout);
		pthread_create(&tid[i], NULL, updater, &planet[i]);
	}	


	// Wait for threads to terminate
	for(i = 0; i <= 9;i++) {
		pthread_join(tid[i], NULL);	
		printf("Terminating body %d\n", i);
	}

	// Print a result:
	printf("Earth's location is %Lf%% off  \n", (expVal/solarSystem[3].p.x - 1) * 100);
	// Expected result after 1mo is -9.856777336025174E+07

}


// Takes a planet and handles updates (on the solarSystem) for it while running
void * updater(int *planet) {
    
	int i = 0;
	while(i < totalSteps) {
		// Sync on 0th tick and then every $syncStep after
		if (i % syncStep == 0) {
			pthread_barrier_wait(&syncBarrier);
			fflush(stdout); 
		}

		fflush(stdout);
		// Handle updating here to minimize conflicts where velocity/position changes halfway through reading it.
		updatePlanet(*planet);
		i++;
	}
    
}