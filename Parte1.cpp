#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <vector>
#include <string>
#include <map>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <semaphore.h>

using namespace std;

int cantidadSeriesBetflix = 0; // contador de series para Betflix
int cantidadSeriesDasney = 0; // contador de series para Dasney
map<string, int> estadoSeriesDasney; // Mapa para el estado de las series en Dasney
map<string, int> estadoSeriesBetflix; // Mapa para el estado de las series en Betflix

pthread_cond_t cond_actualizadoDasney = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_actualizadoBetflix = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutexDasney = PTHREAD_MUTEX_INITIALIZER; // Mutex para Dasney
pthread_mutex_t mutexBetflix = PTHREAD_MUTEX_INITIALIZER; // Mutex para Betflix
sem_t semaforoDasney; // semáforo para controlar el acceso a Dasney
sem_t semaforoBetflix; // semáforo para controlar el acceso a Betflix

void* Dasney(void* arg) {
    sem_wait(&semaforoDasney); // decrementar en 1 el contador
    pthread_mutex_lock(&mutexDasney); // Bloquear mutex para evitar condiciones de carrera
    while (estadoSeriesDasney.size() ==0 ){ // Espera hasta que todas las series hayan sido creadas
        pthread_cond_wait(&cond_actualizadoDasney, &mutexDasney);
    }
    cout << "Accedió a la plataforma Dasney el thread " << arg << endl;
    pthread_mutex_unlock(&mutexDasney); // Desbloquear el mutex
    sem_post(&semaforoDasney); // Liberar el semáforo
    return nullptr; 
}

void* Betflix(void* arg) {
    sem_wait(&semaforoBetflix);
    pthread_mutex_lock(&mutexBetflix);
    while (estadoSeriesBetflix.size() ==0) { // Espera hasta que todas las series hayan sido creadas
        pthread_cond_wait(&cond_actualizadoBetflix, &mutexBetflix);
    }
    cout << "Accedió a la plataforma Betflix el thread " << arg << endl;
    pthread_mutex_unlock(&mutexBetflix);
    sem_post(&semaforoBetflix);
    return nullptr; 
}

void* DasneyContenido() {
    // Generación de una cantidad random entre 10 y 15 de series en la plataforma Dasney
    cantidadSeriesDasney = 10 + rand() % 6; // Establecer la cantidad de series
    cout << "Cantidad de series para Dasney: " << cantidadSeriesDasney << endl;
    for (int i = 0; i < cantidadSeriesDasney; i++) {
        estadoSeriesDasney["Serie " + to_string(i)] = 0; // 0 significa que la serie no ha sido vista
    }
    // Signal para indicar que las series han sido creadas
    pthread_cond_broadcast(&cond_actualizadoDasney);
    return nullptr; 
}

void* BetflixContenido() {
    // Generación de una cantidad random entre 10 y 15 de series en la plataforma Betflix
    cantidadSeriesBetflix = 10 + rand() % 6; // Establecer la cantidad de series
    cout << "Cantidad de series para Betflix: " << cantidadSeriesBetflix << endl;
    for (int i = 0; i < cantidadSeriesBetflix; i++) {
        estadoSeriesBetflix["Serie " + to_string(i)] = 0; // 0 significa que la serie no ha sido vista
    }
    // Señal para indicar que las series han sido creadas
    pthread_cond_broadcast(&cond_actualizadoBetflix);
    return nullptr; 
}

int main(int argc, char* argv[]) {
    // Inicialización de la semilla aleatoria
    srand(time(nullptr));

    // Crear las series en Dasney y Betflix
    DasneyContenido();
    BetflixContenido();

    // Declaración de los threads                 
    pthread_t threads[12];
    int status;

    // Inicialización de los semáforos
    sem_init(&semaforoDasney, 0, 2);
    sem_init(&semaforoBetflix, 0, 1);

    // Crear hilos para Dasney y Betflix
    for (long i = 0; i < 12; i++) {
        if (i <= 5) {
            printf("[main] Creando thread %ld para Dasney\n", i);
            status = pthread_create(&threads[i], NULL, Dasney, (void*)i);    
        } else {
            printf("[main] Creando thread %ld para Betflix\n", i);
            status = pthread_create(&threads[i], NULL, Betflix, (void*)i);
        }
        
        if (status != 0) {
            printf("[main] Error. pthread_create retornó el código de error %d\n", status);
            exit(-1);
        }
    }

    // Esperar a que todos los threads terminen
    for (int i = 0; i < 12; ++i) {
        pthread_join(threads[i], NULL);
    }

    // Destruir los semáforos
    sem_destroy(&semaforoDasney);
    sem_destroy(&semaforoBetflix);

    return 0;
}





