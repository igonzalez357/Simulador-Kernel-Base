#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "clock.h"
#include "timer.h"
#include "processGenerator.h"
#include "scheduler.h"
#include "estructuras.h"

// Declaración de las variables globales
pthread_mutex_t timer_mutex;
pthread_cond_t timer_cond; 
pthread_mutex_t process_queue_mutex;
pthread_mutex_t scheduler_mutex;
pthread_cond_t scheduler_cond;
pthread_mutex_t clock_mutex;
pthread_cond_t clock_cond;
ProcessQueue queue; 
Machine machine;
int numHilosTotales;
int numHilosDisponibles;

int intervalTimer, intervalProcessGen, politicaScheduler;

int main(int argc, char *argv[]){

    // Verificar que no se hayan pasado argumentos
    if (argc > 1) {
        printf("Use: %s ", argv[0]);
        exit(1);
    }

    printf("Ingrese el número de núcleos de la máquina (Recomendado 2): ");
    if (scanf("%d", &machine.numCores) != 1) {
        printf("Error: entrada inválida para el número de núcleos.\n");
        exit(1);
    }
    
    // Solicitar al usuario el número de hilos por núcleo de la máquina
    printf("Ingrese el número de hilos por núcleo de la máquina (Recomendado 3): ");
    if (scanf("%d", &machine.numHilos) != 1) {
        printf("Error: entrada inválida para el número de hilos.\n");
        exit(1);
    }

    numHilosTotales = machine.numCores * machine.numHilos;
    numHilosDisponibles = numHilosTotales;
    printf("Número total de hilos en la máquina: %d\n", numHilosTotales);

    // Solicitar al usuario la configuración de la simulación por consola
    printf("Ingrese el intervalo de ticks del temporizador (Recomendado 4): ");
    if (scanf("%d", &intervalTimer) != 1) {
        printf("Error: entrada inválida para el intervalo de ticks del temporizador.\n");
        exit(1);
    }

    printf("Ingrese el intervalo de tiempo del generador de procesos en segundos (Recomendado 2): ");
    if (scanf("%d", &intervalProcessGen) != 1) {
        printf("Error: entrada inválida para el intervalo de tiempo del generador de procesos.\n");
        exit(1);
    }

    printf("Ingrese la política del scheduler: (0: FCFS, 1: SJF, 2: Round Robin): ");
    while (1) { // Bucle infinito hasta que se reciba una entrada válida

        if (scanf("%d", &politicaScheduler) == 1 && (politicaScheduler == 0 || politicaScheduler == 1 || politicaScheduler == 2)) {
            printf("\n----------------------------------------\n");
            printf("\n  SIMULACIÓN DEL KERNEL INICIADA\n");
            printf("\n----------------------------------------\n\n");
            break; // Salir del bucle si la entrada es válida

        } else {

            printf("Entrada inválida. Por favor, ingrese 0, 1 o 2: ");
            // Limpiar la entrada para evitar bucles infinitos
            while (getchar() != '\n'); 
        }
    }

    // Inicializar los mutex y condicionales
    if (pthread_mutex_init(&timer_mutex, NULL) != 0) {
        perror("Error al inicializar timer_mutex");
        exit(1);
    }
    if (pthread_cond_init(&timer_cond, NULL) != 0) {
        perror("Error al inicializar timer_cond");
        exit(1);
    }
    if (pthread_mutex_init(&process_queue_mutex, NULL) != 0) {
        perror("Error al inicializar process_queue_mutex");
        exit(1);
    }
    if (pthread_mutex_init(&scheduler_mutex, NULL) != 0) {
        perror("Error al inicializar scheduler_mutex");
        exit(1);
    }
    if (pthread_cond_init(&scheduler_cond, NULL) != 0) {
        perror("Error al inicializar scheduler_cond");
        exit(1);
    }
    if (pthread_mutex_init(&clock_mutex, NULL) != 0) {
        perror("Error al inicializar clock_mutex");
        exit(1);
    }
    if (pthread_cond_init(&clock_cond, NULL) != 0) {
        perror("Error al inicializar clock_cond");
        exit(1);
    }
    
    // Declaramos los hilos
    pthread_t threadClock, threadTimer, threadProcessGen, threadScheduler;

    queue.numProcesses = 0; // Inicializar el número de procesos en la cola

    //Creamos el primer hilo que ejecuta Clock
    if (pthread_create(&threadClock, NULL, clock_thread, NULL) != 0) {
        perror("Error al crear el hilo Clock");
        exit(1);
    }

    //Creamos el hilo del timer
    if (pthread_create(&threadTimer, NULL, timer_thread, NULL) != 0) {
        perror("Error al crear el hilo Timer");
        exit(1);
    }    

    //Creamos el hilo del process generator
    if (pthread_create(&threadProcessGen, NULL, process_generator_thread, NULL) != 0) {
        perror("Error al crear el hilo Process Generator");
        exit(1);
    }

    //Creamos el hilo del process generator
    if (pthread_create(&threadScheduler, NULL, scheduler_thread, NULL) != 0) {
        perror("Error al crear el hilo Scheduler");
        exit(1);
    }  

    // Esperamos a que los hilos terminen usando pthread_join (Aunque no es necesario porque los hilos no terminan)
    pthread_join(threadClock, NULL);
    pthread_join(threadTimer, NULL);
    pthread_join(threadProcessGen, NULL);
    pthread_join(threadScheduler, NULL);

    return 0;

}