# HW4 Shared Memory and Signals 

For this assignment, you will write 3 related programs to manage, report and compute results stored in shared memory. You should hand in files manage.c, report.c and compute.c to implement the functions described below. 

Compute's job is to compute perfect numbers. It takes one command line argument, which is the first number to test. It tests all numbers starting at this point, subject to the constraints below. There may be more than one copy of compute running simultaneously. 

Manage's job is to maintain the shared memory segment. The shared segment is where the compute processes post their results. It also keeps track of the active "compute" processes, so that it can signal them to terminate. 
