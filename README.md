Este proyecto es un simulador de un **kernel** con diferentes políticas de planificación de procesos. Está diseñado para simular cómo un kernel maneja y programa los procesos en un sistema operativo utilizando varias técnicas de planificación de CPU.

## Partes del Proyecto

El proyecto se divide en dos versiones:

1. **Simulador-Kernel**: Contiene la implementación completa, con tres partes principales de funcionamiento:
    - Gestión de procesos.
    - Planificación de procesos con políticas como FCFS (First-Come, First-Served), SJF (Shortest Job First), y Round Robin.
    - Gestión de la memoria virtual y física, así como ejecutar programas con instrucciones de tipo add, ld, st, sub.
  
2. **Simulador-Kernel-Base**: La actual, contiene solo las dos primeras partes, es decir:
    - Gestión básica de procesos.
    - Implementación de las políticas de planificación básicas.

## Tecnologías Utilizadas

- **C** para la implementación del simulador.
- **Pthreads** para la gestión de hilos y sincronización.
- **Mutexes** y **condiciones** para garantizar la sincronización entre las diferentes partes del sistema.

Para compilar el proyecto:
gcc -O2 -o kernelSimulatorBase main.c clock.c processGenerator.c scheduler.c timer.c

Para ejecutar:
./kernelSimulatorBase
