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
#include <tgmath.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h> 
#include <sys/mman.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <ctype.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h" // Handy library for image writing. https://github.com/nothings/stb


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
int totalSteps = 267840; // 1 month is 2678400. Currently results in huge error after 1 month of sim. I wonder if we need to take relativity into account. That'd be fun.

long double expVal = -98567773.36025174;

// Create pointers to functions
void forkSoln();
void threadSoln();
Vec newVec(int x, int y, int z);
void printVec(Vec v);
void vecReset(Vec *v);
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
void readData(char filename[]);
void *updater(int *planet);
void updater2(int planet);
void createImage(char* name);

// Ensure all planets are on the same step every syncStep number of steps
int syncStep = 86400;

// Using a "brand new" thing I found, sync barriers!
// While these weren't covered in class, they fill our need perfectly.
//static pthread_barrier_t *syncBarrier;

// Storage for solar system
// 0 is sun, 1 is mercury, etc
// Pluto IS a planet. Ignore the NASA illuminati propoganda!
static Planet *solarSystem;
static int *syncPlanet;
static sem_t *sem, *sem2;

// For CLI flags
int numImages = 0;

int main (int argc, char *argv[]) {

	// CLI flag handling
	int i;

	// Possible arguments
	const char* possibleArgs[2] = {"--images", "--finalState"};

	// Final result printing. If negative, doesn't print
	int finalResult = -1;

	for (i=1;i<argc;i++) {
		if (strcmp(possibleArgs[0], argv[i]) == 0) { // Unfortunately can't swtich on a string
			// Image flag case
			numImages = atoi(argv[++i]);
		} else if (strcmp(possibleArgs[1], argv[i]) == 0) {
			// For final state flag, check it's within bounds
			if (atoi(argv[i+1]) < 10 && atoi(argv[i+1]) >= 0)
				finalResult = atoi(argv[++i]);
			else
				printf("Final state flag requires number between 0 and 9\n");
		} else {
			printf("Unknown argument passed\n");
			return 0;
		}
	}



	// Timers to time the two approaches
	struct timeval start_time, stop_time, elapsed_time;

    Planet planets[10];
    solarSystem = &planets;
    solarSystem = mmap(NULL, sizeof *solarSystem, PROT_READ | PROT_WRITE, 
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);


    syncPlanet = mmap(NULL, sizeof *syncPlanet, PROT_READ | PROT_WRITE, 
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    *syncPlanet = 0;

    const int SIZE = 1;
	const char *name = "planetSim1";
    const char *name2 = "planetSim2";

	printf("Using shared memory named '%s'.\n\n", name);
	int shm_fd, shm_fd2;

	// Create shared memory for semaphore
	shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
	ftruncate(shm_fd,SIZE);
	sem = mmap(0,SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (sem == MAP_FAILED) {
		printf("Map failed\n");
		exit(0);
	}
	// Set up semaphore
	if(sem_init(sem, 1, 0) < 0) { // 1 = multiprocess
		fprintf(stderr, "ERROR: could not initialize semaphore.\n");
		exit(0);
	}

    shm_fd2 = shm_open(name2, O_CREAT | O_RDWR, 0666);
	ftruncate(shm_fd2,SIZE);
    sem2 = mmap(0,SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd2, 0);
	if (sem2 == MAP_FAILED) {
		printf("Map failed\n");
		exit(0);
	}
	// Set up semaphore
	if(sem_init(sem2, 1, 1) < 0) { // 1 = multiprocess
		fprintf(stderr, "ERROR: could not initialize semaphore.\n");
		exit(0);
	}

	// Read in starting data at given time
	readData("startData.csv");

	// Run one solution at a time to compare


	gettimeofday(&start_time,NULL); // Start timer for fork
	printf("(Running fork...)\n");
	// Fork based solution
    forkSoln();

    // Time for fork:
	gettimeofday(&stop_time,NULL);
	timersub(&stop_time, &start_time, &elapsed_time); // Unix time subtract routine
	double forkRuntime = elapsed_time.tv_sec+elapsed_time.tv_usec/1000000.0;

	// Store results of fork
	Planet forkResults[10];
	memcpy(&forkResults, &planets, sizeof(planets));

	// Read in starting data to start thread at same point
	readData("startData.csv");


	gettimeofday(&start_time,NULL); // Start timer for thread
	printf("(Running thread...)\n");

	// Thread based solution
	threadSoln();

    // Time for thread:
	gettimeofday(&stop_time,NULL);
	timersub(&stop_time, &start_time, &elapsed_time); // Unix time subtract routine
	double threadRuntime = elapsed_time.tv_sec+elapsed_time.tv_usec/1000000.0;


	printf("Forked simulation took %f seconds\n", forkRuntime);
	printf("Threaded simulation took %f seconds\n", threadRuntime);

	// Compare final states TODO		
	double diff;
	for (i = 0;i<=9;i++) {
		diff = 0;
	}

	printf("The final states of the two simulations are %f%% different\n", diff);

	if (finalResult > 0) {
		printf("Final state of body %d:\n", finalResult);
		printVec(solarSystem[finalResult].p);
	}
}

Vec newVec(int vx, int vy, int vz) {
    return (Vec){.x = vx, .y = vy, .z = vz};
}

void printVec(Vec v) {
    printf("(%Lf, %Lf, %Lf)\n", v.x, v.y, v.z);
}

void vecReset(Vec *v) {
	v->x = 0;
    v->y = 0;
    v->z = 0;
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
    return -6.6740831 * (long double)(pow(10, -20)) * m / (r * r);
}

// Updates the properties of the "active" planet using the rest of the 1
// System is passed in as an array of planets, the active planet is determined based on its index in that array
// NOTE: Synchronization does not happen here. The system may occasionally be out-of-sync, but this function doesn't care
void updatePlanet(int active) { 
	int i;
    // store "init" acceleration and velocity
    Vec accel = solarSystem[active].a;
    Vec veloc = solarSystem[active].v;

    // reset net gravitational acceleration to 0
    vecReset(&solarSystem[active].a);

    // Update acceleration
	for(i = 0; i <= 9;i++) {
		if(!(i==active)) {
            Vec dist = delta(solarSystem[active].p, solarSystem[i].p);
            long double distMag = vecMag(dist);
            vecScale(&dist, grav(solarSystem[i].mass, distMag)/distMag);
            vecAddii(&solarSystem[active].a, dist);
		}
	}

    // averaging init and final acceleration
    vecAddii(&accel, solarSystem[active].a);
    vecScale(&accel, (long double) 0.5);

    // Updating Velocity
	vecAddii(&solarSystem[active].v, accel);

    // averaging init and final velocity
    vecAddii(&veloc, solarSystem[active].v);
    vecScale(&veloc, (long double) 0.5);
    
    // updating position
	vecAddii(&solarSystem[active].p, veloc);
}


// Reads the given data and places it in the solarSystem
// All data thanks to https://ssd.jpl.nasa.gov/horizons.cgi
void readData(char filename[]) {

	// Unfortunately have to hardcode because implementing proper reading was extremely difficult (and outside the necessary scope of the project)
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
}





// Solution using fork
void forkSoln() {
	   
    int i;
    pid_t pid = 0;
    for (i = 0; i < 10; i++) {
        printf("Starting body %d\n", i); 
		fflush(stdout);
        pid = fork();
        if (pid == 0){
            *updater(&i);
            exit(0);
        }
    } 
    for (i = 0; i < 10; i++){
        wait(NULL);
    }
    printf("Earth's location is %Lf%% off  \n", (expVal-solarSystem[3].p.x)/expVal * 100);
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
		fflush(stdout);
		pthread_create(&tid[i], NULL, updater, &planet[i]);
	}	


	// Wait for threads to terminate
	for(i = 0; i <= 9;i++) {
		pthread_join(tid[i], NULL);	
	}

	// Print a result:
    printf("Earth's location is %Lf%% off  \n", (expVal-solarSystem[3].p.x)/expVal * 100);
	// Expected result after 1mo is -9.856777336025174E+07

}

// Takes a planet and handles updates (on the solarSystem) for it while running
void *updater(int* planet) {

	int i = 0;
	while(i < totalSteps) {
		// Sync on 0th tick and then every $syncStep after
		if (i % syncStep == 0) {
            sem_wait(sem2);
            if(*syncPlanet < 9) {
                
                *syncPlanet += 1;
                sem_post(sem2);
                sem_wait(sem);
                *syncPlanet += 1;
                sem_post(sem);
            }
            else {
                *syncPlanet = 0;
                sem_post(sem);
                while (*syncPlanet < 9) {}
                *syncPlanet = 0;
                sem_wait(sem);

                sem_post(sem2);
            }
		}

		// Handle updating here to minimize conflicts where velocity/position changes halfway through reading it.
		updatePlanet(*planet);
		i++;
	}
}

// When called, creates an image of the system at the current state.
void createImage(char* name) {
	// Max distance pluto can be, which means scaling down by this factor will properly scale everything within bounds
	long double maxVal = 6 * pow(10, 9);

	// Create a square image, with this many pixels per side
	int size = 300;

	// Just take solar system, don't need to differentiate between implementations at the given scale
	unsigned char imageData[size][size];

	// Zero array
	memset(imageData, 0, size*size*sizeof(unsigned char));
	
	int i;
	// Loop through solar system, changing the 10 required pixels
	for (i = 0; i <= 9; ++i) {
		int x, y;
		x = solarSystem[i].p.x / maxVal * size / 2 + size/2;
		y = solarSystem[i].p.y / maxVal * size / 2 + size/2; 
		printf("SS%d: %Lf\n", i, solarSystem[i].p.x / maxVal);
		printf("x: %d, y: %d \n", x, y);
		imageData[x][y] = 255;
	}

	// Use a handy external library to write the data
	if(stbi_write_bmp(name, size, size, 1, &imageData) == 0)
		printf("Image writing failed\n");
}