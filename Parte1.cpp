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
#include <random>
#include <unistd.h>
using namespace std;

// Elementos para la generación de números random
vector<double> valores = {0.5, 1.0, 1.5, 2.0};
random_device rd;
mt19937 gen(rd());
uniform_int_distribution<> distrib(0, valores.size() - 1);
uniform_int_distribution<> distrib2(10, 15);

int count = 0;
int semanas = 2; // Declaración número de semanas
bool listo = false;

int cantidadSeriesBetflix = 0; // contador de series para Betflix
int cantidadSeriesDasney = 0; // contador de series para Dasney
map<string, int> estadoSeriesDasney; // Mapa para el estado de las series en Dasney
map<string, int> estadoSeriesBetflix; // Mapa para el estado de las series en Betflix
map<pthread_t, vector<double>> profesores;

pthread_cond_t cond_DasneyCreado = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_DasneyActualizado = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_actualizadoBetflix = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_actualizarDasney = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mapa = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexDasney = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexDasney2 = PTHREAD_MUTEX_INITIALIZER; // Mutex para Dasney
pthread_mutex_t mutexDasney3 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexBetflix = PTHREAD_MUTEX_INITIALIZER; // Mutex para Betflix
pthread_mutex_t mutexCount = PTHREAD_MUTEX_INITIALIZER;
sem_t semaforoDasney; // semáforo para controlar el acceso a Dasney
sem_t semaforoBetflix; // semáforo para controlar el acceso a Betflix

bool verificarCantidadElementos() {
    // Obtener el tamaño del primer vector para comparar con los demás
    auto it = profesores.begin();
    size_t tamaño_inicial = it->second.size();
    bool todos_iguales = true;

    // Iterar sobre el mapa y verificar que todos los vectores tengan el mismo tamaño
    for (const auto& [id, vector] : profesores) {
        if (vector.size() != tamaño_inicial) {
            todos_iguales = false;
            cout << "Thread " << id << " tiene un vector con tamaño diferente: " << vector.size() << endl;
        } else {
            cout << "Thread " << id << " tiene el tamaño correcto: " << vector.size() << endl;
        }
    }

    if (todos_iguales) {
        cout << "Todos los threads tienen la misma cantidad de elementos en sus vectores: " << tamaño_inicial << endl;
        return true;
    } else {
        return false;
    }
}



void* BetflixContenido(void* arg) {
    // Generación de una cantidad random entre 10 y 15 de series en la plataforma Betflix
    cantidadSeriesBetflix = distrib2(gen); // Establecer la cantidad de series
    cout << "Cantidad de series para Betflix: " << cantidadSeriesBetflix << endl;
    for (int i = 0; i < cantidadSeriesBetflix; i++) {
        estadoSeriesBetflix["Serie " + to_string(i)] = 0; // 0 significa que la serie no ha sido vista
    }
    // Señal para indicar que las series han sido creadas
    pthread_cond_broadcast(&cond_actualizadoBetflix);

    return nullptr;
}

//Función para inicializar Dasney
void CrearDasney(){
     cantidadSeriesDasney = distrib2(gen); // Establecer la cantidad de series
    cout << "Cantidad de series para Dasney: " << cantidadSeriesDasney << endl;
        // Señal para indicar que las series han sido creadas
         for (int i = 0; i < cantidadSeriesDasney; i++) {
        estadoSeriesDasney["Serie " + to_string(i)] = 0; // 0 significa que la serie no ha sido vista
        pthread_cond_broadcast(&cond_DasneyCreado);
    } 
}

void* DasneyContenido(void* arg) {

        while (semanas>0)
        {
        int seriesActuales = cantidadSeriesDasney;
        cantidadSeriesDasney = distrib2(gen); // Actualizar la cantidad de series
        

        // Espera hasta que todos los hilos hayan terminado su acceso en la semana
        pthread_mutex_lock(&mutexCount);
        while (count != 6) {
            pthread_cond_wait(&cond_actualizarDasney, &mutexCount);
        }
        pthread_mutex_unlock(&mutexCount);

        // Verifica que todos los hilos tengan vectores de mismo tamaño
        if (verificarCantidadElementos()) {
            cout << "Todos los hilos completaron correctamente la semana" << endl;
            cout << "Cantidad de series para Dasney: " << cantidadSeriesDasney << endl;

            // Actualiza las series para la próxima semana
            for (int i = 0; i < cantidadSeriesDasney; i++) {
                estadoSeriesDasney["Serie " + to_string(i + seriesActuales + 1)] = 0;
            }
            // Reinicia el contador y habilita la siguiente semana
            count = 0;
            semanas--;
            listo = true;

            pthread_cond_broadcast(&cond_DasneyActualizado);  // Libera a todos los threads

        }
        }
        
    
    return nullptr;
}

void* Dasney(void* arg) {
    pthread_mutex_lock(&mapa);
    pthread_t thread_id = pthread_self();
    profesores[thread_id] = {};  // Inicializa el vector de series del thread
    pthread_mutex_unlock(&mapa);

    while (semanas>0)
    {
        sem_wait(&semaforoDasney);
        pthread_mutex_lock(&mutexDasney);

        // Espera hasta que las series estén actualizadas para la semana actual
        while (estadoSeriesDasney.size() == 0) {
            pthread_cond_wait(&cond_DasneyCreado, &mutexDasney);
        }
         pthread_mutex_unlock(&mutexDasney);

         pthread_mutex_lock(&mutexDasney3);

        // Ver cuántas series puede ver el hilo
        double seriesPorVer = valores[distrib(gen)];
        profesores[thread_id].push_back(seriesPorVer);
        cout << "EL thread " << arg << " vio " << seriesPorVer << " series en la semana" << endl;

        pthread_mutex_unlock(&mutexDasney3);

        sem_post(&semaforoDasney);

        // Contabiliza el acceso de cada hilo
        pthread_mutex_lock(&mutexCount);
        count++;
        if(count == 6) {
            pthread_cond_signal(&cond_actualizarDasney);  // Señaliza que todos terminaron
        }
        pthread_mutex_unlock(&mutexCount);

        // Espera hasta que todos los hilos completen la semana y se actualicen las series
        pthread_mutex_lock(&mutexDasney2);
        while (!listo) {
            pthread_cond_wait(&cond_DasneyActualizado, &mutexDasney2);
        }
        pthread_mutex_unlock(&mutexDasney2);

        cout << "termino la semana" << endl;
        sleep(2);
    }
    
    
    return nullptr;
}


void* Betflix(void* arg) {
    sem_wait(&semaforoBetflix);
    pthread_mutex_lock(&mutexBetflix);
    while (estadoSeriesBetflix.size() == 0) { // Espera hasta que todas las series hayan sido creadas
        pthread_cond_wait(&cond_actualizadoBetflix, &mutexBetflix);
    }
    cout << "Accedió a la plataforma Betflix el thread " << arg << endl;
    pthread_mutex_unlock(&mutexBetflix);
    sem_post(&semaforoBetflix);
    return nullptr;
}

void imprimirContenidos() {
    // Imprimir el contenido de los vectores
    for (const auto& [id, vector] : profesores) {
        cout << "Contenido del thread " << id << ": ";
        for (double val : vector) {
            cout << val << " ";
        }
        cout << endl;
    }
}

int main(int argc, char* argv[]) {
    // Declaración de los threads                 
    pthread_t threads[14];
    int status;

    CrearDasney();

    // Inicialización de los semáforos
    sem_init(&semaforoDasney, 0, 2);
    sem_init(&semaforoBetflix, 0, 1);

    // Crear hilos para Dasney y Betflix
    for (long i = 0; i < 14; i++) {
        if (i <= 5) {
            printf("[main] Creando thread %ld para Dasney\n", i);
            status = pthread_create(&threads[i], NULL, Dasney, (void*)i);
        } else if(i>5 && i<=11){
            printf("[main] Creando thread %ld para Betflix\n", i);
            status = pthread_create(&threads[i], NULL, Betflix, (void*)i);
        }else if(i==12){
            status = pthread_create(&threads[i], NULL, BetflixContenido, (void*)i);
        } else if(i==13){
            status = pthread_create(&threads[i], NULL, DasneyContenido, (void*)i);
        }
        
        if (status != 0) {
            cerr << "Error al crear el thread " << i << endl;
            exit(EXIT_FAILURE);
        }
    }

    // Esperar a que todos los hilos terminen
    for (int i = 0; i < 12; i++) {
        pthread_join(threads[i], NULL);
    }

    // Imprimir los contenidos de los vectores
    imprimirContenidos();
    // Limpiar los semáforos
    sem_destroy(&semaforoDasney);
    sem_destroy(&semaforoBetflix);

    return 0;
}






