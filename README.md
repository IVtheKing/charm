chARM 
=====
is a Realtime Operating System developed from scratch that runs on ARM platform. chARM implements Earliest Deadline First (EDF) scheduling algorithm. Its periodic threading model allows creating tasks with arbitrary period, budget, deadline and phase having granularity of few microseconds. It attempts to reduce the context switch time to absolute minimum when periodic tasks finish before their deadline by not storing all registers. chARM currently runs on TQ2440 development board that has Samsung's S3C2440 SOC. Eventually I intend to add support for MPU and MMU.

Main Features of the RTOS
=========================

* From the beginning, one of the main goal was to keep the RTOS highly configurable. Most features of the OS can be enabled or disabled using compile time macros. This allows the developer to optimize the OS for smaller code size / lesser data / faster execution time / higher debug-ability or more features.

* The RTOS supports both periodic & aperiodic threads. This provides a great flexibility to write applications that make use of both periodic and priority based tasks.

* Periodic threads are scheduled using Earliest Deadline First (EDF) algorithm. The EDF algorithm is superior to Rate Monotonic Algorithm as it guarantees the schedulability under higher CPU utilization.

* Has built in support to accumulate run-time statistics about various Operating System parameters, CPU utilization etc.

* Keeps track of per task statistics such as accumulated run time, deadline miss count, Budget exceeded count etc.

* The RTOS makes use of a single hardware timer to control all EDF parameters (Period / Deadline / Budget / Phase). There is no single fixed timer interrupts in this RTOS. At each timer event, the RTOS computes when the next interrupt needs to be generated based on the current set of ready and waiting tasks.

* It configures the timer tick at a very high resolution (1.8963 uSec) to get good granularity. It smartly works around the resulting issue of smaller maximum interval while using 16 bit timers.

* Because the RTOS programs the main OS timer each time, the is a possibility of very slow drift. In order to eliminate this drift completely, it optionally makes use of a second fixed interval timer which triggers every 1 second. At each 1 second interval, the main OS clock re-synchronizes itself to make the drift zero.

* The OS context switch event interrupts are measured to take < 20 micro seconds. The actual time taken by the OS timer interrupt depends on the number of ready tasks at a given time.

* The RTOS supports zero context store for periodic tasks which means for those periodic tasks which complete before its deadline / budget expiry it stores minimal context information. This is possible because each periodic task does not need to retain its registers between two periods.
