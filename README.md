charm
=====

chARM is a Real-Time Operating System developed from scratch that runs on ARM platform. 
chARM implements Earliest Deadline First (EDF) scheduling algorithm. Its periodic 
threading model allows creating tasks with arbitrary period, budget, deadline and phase 
having granularity of few microseconds. It attempts to reduce the context switch time to 
absolute minimum when periodic tasks finish before their deadline by not storing all 
registers. chARM currently runs on TQ2440 development board that has Samsung's 
S3C2440 SOC. Eventually I intend to add support for MPU and MMU.