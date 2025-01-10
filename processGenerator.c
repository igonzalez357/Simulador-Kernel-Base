#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "processGenerator.h"
#include "estructuras.h"
#include <string.h>

// Función para crear un nuevo proceso con un PID y tiempo de llegada
PCB* create_process(int pid) {

    PCB* new_process = (PCB*)malloc(sizeof(PCB));

    if (new_process == NULL) {
        perror("Error al crear un nuevo proceso");
        exit(1);
    }

    new_process->pid = pid;                         // Asigna un identificador único al proceso
    new_process->arrival_time = queue.numProcesses; // Establece el tiempo de llegada
    new_process->lifetime = rand() % 20 + 1;        // Establece una duración de ejecución aleatoria entre 1 y 20
    strcpy(new_process->state, "READY");            // Establece el estado del proceso como READY
    new_process->quantum = 0;                       // Establece el quantum en 0. Solo se usa en Round Robin
    new_process->quantumTime = 0;                   // Establece el tiempo de ejecución con quantum en 0

    return new_process;
}

// Función que inicializa el generador de procesos
void* process_generator_thread() {

    int pid_counter = 1;       // Contador para asignar identificadores únicos a los procesos

    while (1) { // Bucle infinito para mantener el generador activo

        pthread_mutex_lock(&process_queue_mutex); // Bloquea el mutex para sincronización de la cola de procesos

        if (queue.numProcesses < MAXPROCESSES) {

            // Crear un nuevo proceso con su PID y tiempo de llegada
            PCB* new_process = create_process(pid_counter);
            printf("Process Generator: Creado proceso con PID %d y tiempo de vida %d s. ", new_process->pid, new_process->lifetime);

            // Añadir el proceso a la cola de procesos
            queue.processes[queue.numProcesses] = new_process;
            queue.numProcesses++;
            pid_counter++;

        } else {

            printf("Process Generator: No se pueden crear más procesos. Cola llena. ");
        }

        printf("Ocupación de la queue = (%d/%d)\n", queue.numProcesses, MAXPROCESSES);
        pthread_mutex_unlock(&process_queue_mutex); // Desbloquea el mutex

        // Espera un tiempo definido antes de generar el próximo proceso
        sleep(intervalProcessGen);
    }

    return NULL; // Aunque nunca se alcanza
}