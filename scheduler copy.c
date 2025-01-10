#include "estructuras.h"
#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"
#include <pthread.h>
#include <time.h>  // Para usar clock_gettime()
#include <unistd.h> // Para usar sleep()

int quantum = 5; // Quantum de tiempo para Round Robin

// Función para eliminar el proceso en la posición index de la cola
void remove_process_from_queue(int index) {

    queue.numProcesses--; // Reducir el número de procesos
    for (int i = index; i < queue.numProcesses; i++) {     
        // Desplazar los procesos restantes hacia adelante
        queue.processes[i] = queue.processes[i + 1];
        queue.processes[i]->arrival_time = i; // Actualizar el orden de llegada
    }    
}

// Función para ejecutar el proceso y medir el tiempo de ejecución real
void execute_process(int wait, int pid) {
    
    struct timespec start, end; // Estructuras para almacenar el tiempo

    clock_gettime(CLOCK_MONOTONIC, &start); // Tiempo de inicio
    sleep(wait); // Simular la ejecución del proceso
    clock_gettime(CLOCK_MONOTONIC, &end); // Tiempo de fin

    // Calcular el tiempo real en segundos
    double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    // Mostrar el mensaje solo si el tiempo de espera es mayor a 0 (Es decir, si había procesos en la cola)
    if (wait > 0 && politicaScheduler != 2) {
        printf("Scheduler: Proceso con PID %d finalizado, tiempo total = %.6fs. Ocupación de la queue = (%d/%d)\n", pid, elapsed_time, queue.numProcesses, MAXPROCESSES);
    }
}

// Función para ejecutar la política FCFS (First-Come, First-Served)
void scheduler_fcfs() {

    while (1) {

        // Esperar señal del Timer para activarse
        pthread_mutex_lock(&scheduler_mutex);
        pthread_cond_wait(&scheduler_cond, &scheduler_mutex);
        pthread_mutex_unlock(&scheduler_mutex);

        // Acceder a la cola de procesos
        pthread_mutex_lock(&process_queue_mutex);
        printf("Scheduler: (Activado por Timer) ");

        if (queue.numProcesses > 0) { // Si hay procesos en la cola

            // Obtener la información del primer proceso en la cola
            int wait = queue.processes[0]->lifetime;
            int pid = queue.processes[0]->pid;
            printf("Proceso en ejecución (PID %d), tiempo de ejecución = %ds\n", pid, wait);

            // Eliminar el proceso de la cola
            remove_process_from_queue(0);
            pthread_mutex_unlock(&process_queue_mutex);

            // Ejecutar el proceso y medir el tiempo
            execute_process(wait, pid);

        } else {

            // Si no hay procesos en la cola
            printf("No hay procesos en la cola\n");
            pthread_mutex_unlock(&process_queue_mutex);
        }
    }
}

// Función para ejecutar la política SJF (Shortest Job First)
void scheduler_sjf() {

    while (1) {
    
        // Esperar señal del Timer para activarse
        pthread_mutex_lock(&scheduler_mutex);
        pthread_cond_wait(&scheduler_cond, &scheduler_mutex);
        pthread_mutex_unlock(&scheduler_mutex);

        // Acceder a la cola de procesos
        pthread_mutex_lock(&process_queue_mutex);
        printf("Scheduler: (Activado por Timer) ");

        if (queue.numProcesses > 0) {

            // Encontrar el proceso con el menor tiempo de ejecución
            int menor = 0;
            for (int i = 1; i < queue.numProcesses; i++) {
                if (queue.processes[i]->lifetime < queue.processes[menor]->lifetime) {
                    menor = i; // Actualizar índice del proceso más corto
                }
            }

            // Obtener el proceso seleccionado
            int wait = queue.processes[menor]->lifetime;
            int pid = queue.processes[menor]->pid;
            printf("Proceso en ejecución (PID %d), tiempo de ejecución = %ds\n", pid, wait);

            // Eliminar el proceso seleccionado de la cola
            remove_process_from_queue(menor);
            pthread_mutex_unlock(&process_queue_mutex);

            // Ejecutar el proceso y medir el tiempo
            execute_process(wait, pid);

        } else {

            // Si no hay procesos en la cola
            printf("No hay procesos en la cola\n");
            pthread_mutex_unlock(&process_queue_mutex);
        }
    }
}


// Función para ejecutar la política Round Robin
void scheduler_rr(int quantum) {

    while (1) {

        // Esperar señal del Timer para activarse
        pthread_mutex_lock(&scheduler_mutex);
        pthread_cond_wait(&scheduler_cond, &scheduler_mutex);
        pthread_mutex_unlock(&scheduler_mutex);

        // Acceder a la cola de procesos
        pthread_mutex_lock(&process_queue_mutex);
        printf("Scheduler: (Activado por Timer) ");

        if (queue.numProcesses > 0) {

            // Tomar el primer proceso en la cola
            PCB* current_process = queue.processes[0];
            int pid = current_process->pid;
            int time_remaining = current_process->lifetime;

            // Eliminar el proceso de la cola temporalmente
            remove_process_from_queue(0);

            // Decidir cuánto tiempo ejecutará el proceso (mínimo entre quantum y tiempo restante)
            int execution_time = (time_remaining > quantum) ? quantum : time_remaining;
            printf("Proceso con PID %d, ejecutando por %d segundos de %d restantes\n", pid, execution_time, time_remaining);

            // Si el proceso no ha terminado, actualizar su tiempo restante y encolarlo de nuevo
            if (time_remaining > quantum) {
                current_process->lifetime = time_remaining - quantum;
                queue.processes[queue.numProcesses] = current_process;
                queue.numProcesses++;

            } else {
                // Si el proceso ha terminado
                free(current_process); // Liberar memoria del proceso
            }

            pthread_mutex_unlock(&process_queue_mutex);

            execute_process(execution_time, pid); // Ejecutar el proceso y medir el tiempo
            if (time_remaining <= quantum) printf("Scheduler: Proceso con PID %d finalizado. Ocupación de la queue = (%d/%d)\n", pid, queue.numProcesses, MAXPROCESSES);
            else printf("Scheduler: Proceso con PID %d interrumpido, tiempo restante = %d\n", pid, current_process->lifetime);

        } else {

            // Si no hay procesos en la cola
            printf("No hay procesos en la cola\n");
            pthread_mutex_unlock(&process_queue_mutex);
        }

    }
}

// Función principal del hilo del scheduler
void* scheduler_thread() {

    switch (politicaScheduler) {

        case 0: // FCFS
            scheduler_fcfs();
            break;

        case 1: // SJF
            scheduler_sjf();
            break;

        case 2: // Round Robin
            scheduler_rr(quantum); // Quantum de tiempo
            break;

        default:
            // Política no reconocida
            printf("Política no reconocida, error\n");
            exit(EXIT_FAILURE);
            break;
    }

    return NULL; // Nunca se llega aquí
}
