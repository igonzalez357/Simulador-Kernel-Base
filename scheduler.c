#include "estructuras.h"
#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"
#include <pthread.h>
#include <time.h>  // Para usar clock_gettime()
#include <unistd.h> // Para usar sleep()
#include <string.h>

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

void actualizarColaProcesos (){

    for(int i = 0; i < queue.numProcesses && numHilosDisponibles < numHilosTotales; i++){
        
        // Decrementar el tiempo de vida de cada proceso si está en estado RUNNING
        if(strcmp(queue.processes[i]->state, "RUNNING") == 0 || strcmp(queue.processes[i]->state, "STOPPED") == 0){

            if (politicaScheduler == 2 && queue.processes[i]->quantum == 1){

                queue.processes[i]->quantumTime--; // Decrementar el tiempo de ejecución con quantum

                if(queue.processes[i]->quantumTime == 0){

                    queue.processes[i]->quantum = 0; // Indica que el proceso ya no se ejecutará con quantum
                    strcpy(queue.processes[i]->state, "STOPPED"); // Cambiar el estado del proceso a STOPPED
                    printf("Proceso con PID %d interrumpido, tiempo restante = %d. Encolado al final de la queue\n", queue.processes[i]->pid, queue.processes[i]->lifetime);
                
                }

                continue;
            }
            

            queue.processes[i]->lifetime--;
            if(queue.processes[i]->lifetime == 0){
                printf("Proceso con PID %d finalizado. Ocupación de la queue = (%d/%d)\n", queue.processes[i]->pid, queue.numProcesses - 1, MAXPROCESSES);
                remove_process_from_queue(i);
                i--;
                numHilosDisponibles++;
            }

        }
    }

}

// Función para ejecutar la política FCFS (First-Come, First-Served)
void scheduler_fcfs() {

    while (1) {

        for (int i = 0; i < intervalTimer - 1; i++) {
            
            // Esperar señal del clock para actualizar la cola de procesos
            pthread_mutex_lock(&clock_mutex);
            pthread_cond_wait(&clock_cond, &clock_mutex);

            // Acceder a la cola de procesos y actualizarla
            pthread_mutex_lock(&process_queue_mutex);
            actualizarColaProcesos();
            pthread_mutex_unlock(&process_queue_mutex);

            pthread_mutex_unlock(&clock_mutex);
        }

        // Esperar señal del Timer para activarse
        pthread_mutex_lock(&scheduler_mutex);
        pthread_cond_wait(&scheduler_cond, &scheduler_mutex);
        pthread_mutex_unlock(&scheduler_mutex);

        // Acceder a la cola de procesos
        pthread_mutex_lock(&process_queue_mutex);
        printf("Scheduler: (Activado por Timer) \n");

        if (queue.numProcesses > 0 && numHilosDisponibles > 0) { // Si hay procesos en la cola y no todos los hilos están ocupados

            for (int i = 0; numHilosDisponibles > 0 && i < queue.numProcesses; i++) {

                // Obtener la información del primer proceso en la cola en estado READY
                if(strcmp(queue.processes[i]->state, "READY") == 0){
                    strcpy(queue.processes[i]->state, "RUNNING"); // Cambiar el estado del proceso a RUNNING
                    numHilosDisponibles--;
                    printf("Proceso con PID %d en ejecución. Tiempo estimado = %d s\n", queue.processes[i]->pid, queue.processes[i]->lifetime);
                }
                pthread_mutex_unlock(&process_queue_mutex);

            }

        } else {

            // Si no hay procesos en la cola
            if (queue.numProcesses == 0) {
                printf("No hay procesos en la cola\n");
            } else if (numHilosDisponibles == 0) {
                printf("Todos los hilos están ocupados\n");
            }
            pthread_mutex_unlock(&process_queue_mutex);
        }
    }
}

// Función para ejecutar la política SJF (Shortest Job First)
void scheduler_sjf() {

    while (1) {
    
        for (int i = 0; i < intervalTimer - 1; i++) {
            
            // Esperar señal del clock para actualizar la cola de procesos
            pthread_mutex_lock(&clock_mutex);
            pthread_cond_wait(&clock_cond, &clock_mutex);

            // Acceder a la cola de procesos y actualizarla
            pthread_mutex_lock(&process_queue_mutex);
            actualizarColaProcesos();
            pthread_mutex_unlock(&process_queue_mutex);

            pthread_mutex_unlock(&clock_mutex);
        }

        // Esperar señal del Timer para activarse
        pthread_mutex_lock(&scheduler_mutex);
        pthread_cond_wait(&scheduler_cond, &scheduler_mutex);
        pthread_mutex_unlock(&scheduler_mutex);

        // Acceder a la cola de procesos
        pthread_mutex_lock(&process_queue_mutex);
        printf("Scheduler: (Activado por Timer) \n");

        if (queue.numProcesses > 0 && numHilosDisponibles > 0) { // Si hay procesos en la cola y no todos los hilos están ocupados

            for (int i = 0; numHilosDisponibles > 0 && i < queue.numProcesses; i++) {

                // Encontrar el proceso con el menor tiempo de ejecución en estado READY
                if(strcmp(queue.processes[i]->state, "READY") == 0){
                    int menor = i;
                    for (int j = 0; j < queue.numProcesses; j++) {
                        if (queue.processes[j]->lifetime < queue.processes[menor]->lifetime && strcmp(queue.processes[j]->state, "READY") == 0) {
                            menor = j; // Actualizar índice del proceso más corto
                        }
                    }

                    // Poner el proceso en estado RUNNING
                    strcpy(queue.processes[menor]->state, "RUNNING");
                    numHilosDisponibles--;
                    printf("Proceso con PID %d en ejecución. Tiempo estimado = %d s\n", queue.processes[menor]->pid, queue.processes[menor]->lifetime);
                }
            }

            pthread_mutex_unlock(&process_queue_mutex);

        } else {

            // Si no hay procesos en la cola o todos los hilos están ocupados
            if (queue.numProcesses == 0) {
                printf("No hay procesos en la cola\n");
            } else if (numHilosDisponibles == 0) {
                printf("Todos los hilos están ocupados\n");
            }

            pthread_mutex_unlock(&process_queue_mutex);
        }
    }
}


// Función para ejecutar la política Round Robin
void scheduler_rr(int quantum) {

    while (1) {

        for (int i = 0; i < intervalTimer - 1; i++) {
            
            // Esperar señal del clock para actualizar la cola de procesos
            pthread_mutex_lock(&clock_mutex);
            pthread_cond_wait(&clock_cond, &clock_mutex);

            // Acceder a la cola de procesos y actualizarla
            pthread_mutex_lock(&process_queue_mutex);
            actualizarColaProcesos();
            pthread_mutex_unlock(&process_queue_mutex);

            pthread_mutex_unlock(&clock_mutex);
        }

        // Esperar señal del Timer para activarse
        pthread_mutex_lock(&scheduler_mutex);
        pthread_cond_wait(&scheduler_cond, &scheduler_mutex);
        pthread_mutex_unlock(&scheduler_mutex);

        // Acceder a la cola de procesos
        pthread_mutex_lock(&process_queue_mutex);
        printf("Scheduler: (Activado por Timer) \n");

        if (queue.numProcesses > 0 && numHilosDisponibles > 0) { // Si hay procesos en la cola y no todos los hilos están ocupados

            for (int i = 0; numHilosDisponibles > 0 && i < queue.numProcesses; i++) {

                if (strcmp(queue.processes[i]->state, "READY") == 0 || strcmp(queue.processes[i]->state, "STOPPED") == 0) {

                    // Tomar el primer proceso en la cola
                    PCB* current_process = queue.processes[i];
                    int pid = current_process->pid;
                    int time_remaining = current_process->lifetime;
                    strcpy(current_process->state, "RUNNING");
                    numHilosDisponibles--;

                    // Eliminar el proceso de la cola temporalmente
                    remove_process_from_queue(i);
                    i--;

                    // Decidir cuánto tiempo ejecutará el proceso (mínimo entre quantum y tiempo restante)
                    int execution_time = (time_remaining > quantum) ? quantum : time_remaining;
                    printf("Proceso con PID %d, ejecutando por %d segundos de %d restantes. ", pid, execution_time, time_remaining);

                    // Si el proceso no ha terminado, actualizar su tiempo restante y encolarlo de nuevo
                    if (time_remaining > quantum) {
                        current_process->quantum = 1; // Indica que el proceso se ejecutará el quantum de tiempo 
                        current_process->quantumTime = quantum; // Establece el tiempo de ejecución con quantum
                        current_process->lifetime = time_remaining - quantum;
                        queue.processes[queue.numProcesses] = current_process;
                        queue.numProcesses++;

                    } 
                    else {
                        printf("Extrayendo de la cola. Ocupación de la queue = (%d/%d)", queue.numProcesses, MAXPROCESSES);
                    }

                    printf("\n");
                }
            }

            pthread_mutex_unlock(&process_queue_mutex);
        }
        else {

            // Si no hay procesos en la cola o todos los hilos están ocupados
            if (queue.numProcesses == 0) {
                printf("No hay procesos en la cola\n");
            } else if (numHilosDisponibles == 0) {
                printf("Todos los hilos están ocupados\n");
            }
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
