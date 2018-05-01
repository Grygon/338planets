# 338planets
Spring 2018 final project for EECS 338

Using data for Jan 1, 2000, 00:00, simulate the solar system for one month (by default). Determines the runtime for two different solution methods, the difference in states between these solutions, and prints it to the console.

Three options can be passed (as flags) to this program. These parameters are only minimally checked for validity. Options are as follows:



--ticks \[int]
  Can change the total number of ticks. One tick is equivalent to one second, so 2678400 is approximately one month. Theoretically, any positive integer should be valid. 



--finalState \[int]
  Prints the final state of one of the bodies simulated



--images \[int]
  Creates the specified number of images, evenly spaced, for the duration of the simulation. One image would display the final state, two the state at the middle and end, etc. If used, makes timing comparisons useless--only one method creates images, which means that valid comparisons cannot be drawn using this method.



Currently in its (likely) final state. 
