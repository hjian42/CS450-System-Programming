# HW4 Shared Memory and Signals 

For this assignment, you will write 3 related programs to manage, report and compute results stored in shared memory. You should hand in files manage.c, report.c and compute.c to implement the functions described below. 

Compute's job is to compute perfect numbers. It takes one command line argument, which is the first number to test. It tests all numbers starting at this point, subject to the constraints below. There may be more than one copy of compute running simultaneously. 

Manage's job is to maintain the shared memory segment. The shared segment is where the compute processes post their results. It also keeps track of the active "compute" processes, so that it can signal them to terminate. 

Report's job is to read the shared memory segement and report on the perfect numbers found. It also for each processe currently computing the number tested, skipped, and found. Finally it reports a total of these three numbers for the processes currently running. If invoked with the "-k" switch, it also is used to inform the Manage process to shut down computation. 

The shared memory segment should contain the following data:
  - a bit map large enough to contain 2\*\*25 bits. If a bit is off it inficates that the corresponding integer has not been tested. 
  - an array of integers of length 20 to contain the perfect numbers found.
  - an array of process structures of length 20, to summarize data on the currently active compute processes. This structure soudl contain the pid, the number of perfect number found, the number of candiates tested, and the number of candiateds not tested (ie. skipped). Compute should never test a number already marked in the bitmap. 
  
Compute processes are responsible for updating the bitmap, as well as their own process statistics. However, because of the possible conflicts, manager must initialize their process entry for each compute process. You may use your favorite IPC sheme for compute registering itself with manager. Similary, compute must request manager update the array of perfect numbers, when it finds one. 

Processes that hit the end should wrap around, but stop at their starting point. All processes should terminate cleanly on INTR, QUIT, and HANGUP signals. For compute processes this means they delete their process entry from the shared memory segement and then terminate. For manager it means he sends an INTR signal to all the running computes, sleep 5s, and then deallocates the shared memory segment and terminates. 

When -k flag is used on report, report sends an INTR to manager to force the same shutdown procedure.
