#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"
#include "estructuras.h"
#include <unistd.h> // Para usar sleep()

// Método que inicializa el hilo Timer
void* timer_thread() {

    int cycle_count = 0;        // Contador de ciclos de reloj inicializado en 0

    while (1) { // Bucle infinito para mantener el Timer activo
    
        // Espera a que el Clock envíe una señal
        pthread_mutex_lock(&timer_mutex);
        pthread_cond_wait(&timer_cond, &timer_mutex); // Espera señal del Clock
        pthread_mutex_unlock(&timer_mutex);

        // Incrementa el contador de ciclos
        cycle_count++;

        // Si el contador alcanza el intervalo de ticks, genera un tick
        if (cycle_count >= intervalTimer) {
            printf("Timer: Generando tick (interrupción)\n");

            // Activamos el Scheduler, señal a condicional del Scheduler
            pthread_mutex_lock(&scheduler_mutex);
            pthread_cond_signal(&scheduler_cond);
            pthread_mutex_unlock(&scheduler_mutex);

            cycle_count = 0; // Reinicia el contador
        }
    }

    return NULL;
}
