### Choose Priority Utilities

The utility `chooseprio_fighter` accepts a command in argument, that is executed with all CPUs occupied. The time to run the program is reported. If the subprocess is CPU intensive, the time to execute it should be roughly 2x time as if the process is running without the `fighting` load. By using different priority levels for the subprocess, we can observe the effect on the execution time.

The utility `chooseprio_latency` works similarly. It accepts a command in argument, and while the subprocess executes, it measures the time it takes to sleep for 16ms (which is the delay for interactive 60 FPS). The sampling stops when the subprocess resumes and the average latency is shown. By varying the priority of the subprocess, then we can observe the effect it has on the responsiveness on other running programs. 

