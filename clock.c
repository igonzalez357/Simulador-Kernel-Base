#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "clock.h"
#include "estructuras.h"

#define CLOCK_INTERVAL 1 // Intervalo del Clock en segundos

// Función que simula el ciclo de ejecución del Clock
void* clock_thread() {

    while (1) { // El Clock funciona indefinidamente
    
        // Envía señal al scheduler en cada ciclo para que actualice la cola de procesos
        pthread_mutex_lock(&clock_mutex); // Bloquea el mutex para sincronización
        pthread_cond_signal(&clock_cond); // Envía señal al Scheduler
        pthread_mutex_unlock(&clock_mutex); // Desbloquea el mutex

        // Enviar señal al Timer en cada ciclo
        pthread_mutex_lock(&timer_mutex);   // Bloquea el mutex para sincronización
        pthread_cond_signal(&timer_cond);   // Envía señal al Timer
        pthread_mutex_unlock(&timer_mutex); // Desbloquea el mutex

        // Esperar el intervalo configurado antes del próximo ciclo
        sleep(CLOCK_INTERVAL);
    }

    return NULL; // Nunca se alcanza
}
