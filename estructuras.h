#include <pthread.h>

// Máximo número de procesos permitidos en la cola
#define MAXPROCESSES 50

// Estructura de control de procesos (PCB)
typedef struct PCB {
    int pid;            // Identificador único del proceso
    int lifetime;       // Tiempo de ejecución del proceso
    int arrival_time;   // Tiempo de llegada del proceso
    int quantum;        // Determina si el proceso se ejecuta por Round Robin y el quantum es menor que el tiempo de vida
    int quantumTime;    // Tiempo de ejecución del proceso con quantum en Round Robin
    char state[10];     // Estado del proceso (READY, RUNNING, FINISHED, STOPPED)
} PCB; 

// Cola de procesos
typedef struct ProcessQueue {
    PCB* processes[MAXPROCESSES]; // Array de punteros a procesos
    int numProcesses;             // Número de procesos actualmente en la cola
} ProcessQueue;

// Estructura para describir la configuración de la máquina
typedef struct Machine {
    int numHilos; // Número de hilos
    int numCores; // Número de núcleos
} Machine;

extern Machine machine;         // Variable global para la configuración de la máquina
extern int numHilosTotales;     // Número total de hilos en la máquina
extern int numHilosDisponibles; // Número de hilos disponibles en la máquina

// Declaraciones para la sincronización entre hilos
extern pthread_mutex_t timer_mutex;         // Mutex para el Timer
extern pthread_cond_t timer_cond;           // Condicional para el Timer
extern pthread_mutex_t process_queue_mutex; // Mutex para la cola de procesos
extern pthread_mutex_t scheduler_mutex;     // Mutex para el Scheduler
extern pthread_cond_t scheduler_cond;       // Condicional para el Scheduler
extern pthread_mutex_t clock_mutex;         // Mutex para el Clock
extern pthread_cond_t clock_cond;           // Condicional para el Clock

// Cola de procesos
extern ProcessQueue queue;

// Variables globales para la configuración de la máquina
extern int intervalTimer, intervalProcessGen, politicaScheduler;
