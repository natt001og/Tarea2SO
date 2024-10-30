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
using namespace std;

// Elementos para la generación de números random
vector<double> valores = {0.5, 1.0, 1.5, 2.0};
random_device rd;
mt19937 gen(rd());
uniform_int_distribution<> distrib(0, valores.size() - 1);
uniform_int_distribution<> distrib2(10, 15);

int cont = 0;
int count = 0;
int semanas = 2; // Declaración número de semanas
bool listo = false;

int cantidadSeriesBetflix = 0; // contador de series para Betflix
int cantidadSeriesDasney = 0; // contador de series para Dasney
map<string, int> estadoSeriesDasney; // Mapa para el estado de las series en Dasney
map<string, int> estadoSeriesBetflix; // Mapa para el estado de las series en Betflix
map<pthread_t, vector<double>> profesores;

pthread_cond_t cond_actualizadoDasney = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_actualizadoDasney2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_actualizadoBetflix = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mapa = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexDasney = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexDasney2 = PTHREAD_MUTEX_INITIALIZER; // Mutex para Dasney
pthread_mutex_t mutexBetflix = PTHREAD_MUTEX_INITIALIZER; // Mutex para Betflix
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

void DasneyContenido() {
    // Generación de una cantidad random entre 10 y 15 de series en la plataforma Dasney
    cantidadSeriesDasney = distrib2(gen); // Establecer la cantidad de series
    cout << "Cantidad de series para Dasney: " << cantidadSeriesDasney << endl;
    for (int i = 0; i < cantidadSeriesDasney; i++) {
        estadoSeriesDasney["Serie " + to_string(i)] = 0; // 0 significa que la serie no ha sido vista
    }
    if (cont == 0) {
        // Señal para indicar que las series han sido creadas
        cont = 1;
        pthread_cond_broadcast(&cond_actualizadoDasney);
    } else if (verificarCantidadElementos()) {  //aqui verifica que la plataforma se actualizó correctamente despues de que los threads hayan 
        cout << "Todos pasaron" << endl;        //pasado la semana
        listo = true;
        pthread_cond_broadcast(&cond_actualizadoDasney2);
        pthread_mutex_unlock(&mutexDasney2);
    }
}

void BetflixContenido() {
    // Generación de una cantidad random entre 10 y 15 de series en la plataforma Betflix
    cantidadSeriesBetflix = distrib2(gen); // Establecer la cantidad de series
    cout << "Cantidad de series para Betflix: " << cantidadSeriesBetflix << endl;
    for (int i = 0; i < cantidadSeriesBetflix; i++) {
        estadoSeriesBetflix["Serie " + to_string(i)] = 0; // 0 significa que la serie no ha sido vista
    }
    // Señal para indicar que las series han sido creadas
    pthread_cond_broadcast(&cond_actualizadoBetflix);
}

void* Dasney(void* arg) {

    //ponemos a los threads en el mapa
    pthread_mutex_lock(&mapa);
    pthread_t thread_id = pthread_self();
    pthread_mutex_unlock(&mapa);
    
    //inicia la semana, se controla el acceso con los semaforos
    sem_wait(&semaforoDasney); // decrementar en 1 el contador
    pthread_mutex_lock(&mutexDasney); // Bloquear mutex para evitar condiciones de carrera
    while (estadoSeriesDasney.size() == 0) { // Espera hasta que todas las series hayan sido creadas
        pthread_cond_wait(&cond_actualizadoDasney, &mutexDasney);
    }
    cout << "Accedió a la plataforma Dasney el thread " << arg << endl;
    
    double seriesPorVer = valores[distrib(gen)]; // determina de forma random la cantidad de series para ver
    profesores[thread_id].push_back(seriesPorVer); // Añadir seriesPorVer al vector del hilo actual
    cout << "EL thread " << arg << " vio " << seriesPorVer << " series en la semana" << endl;

    pthread_mutex_unlock(&mutexDasney); // Desbloquear el mutex
    sem_post(&semaforoDasney); // Liberar el semáforo
    cout << "salió del thread " << arg << endl;

    pthread_mutex_lock(&mutexDasney2);
    cout << "entró al mutex un registro" << endl;
    count++;
    if (count == 6) {
        cout << "se ejecutará la función de contenido de Dasney..." << endl;
        DasneyContenido();
    }
    while (!listo) {
        pthread_cond_wait(&cond_actualizadoDasney2, &mutexDasney2);
    }
    pthread_mutex_unlock(&mutexDasney2);
    
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
    cout<<estadoSeriesBetflix.size();

    // Limpiar los semáforos
    sem_destroy(&semaforoDasney);
    sem_destroy(&semaforoBetflix);

    return 0;
}






