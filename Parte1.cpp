//ARCHIVO CORRESPONDIENTE AL PUNTO 1 DE LA TAREA
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
#include <fstream>
using namespace std;

// Elementos para la generación de números random
vector<double> valores = {0.5, 1.0, 1.5, 2.0};
random_device rd;
mt19937 gen(rd());
uniform_int_distribution<> distrib(0, valores.size() - 1);
uniform_int_distribution<> distrib2(10, 15);

bool signal1=0;
bool signal2=0;
int count = 0;
int count1 = 0;
int semanas;
bool listo = false;
int numeroSemana=0;
int cantidadSeriesBetflix=0; // contador de series para Betflix
int cantidadSeriesDasney=0; // contador de series para Dasney
map<string, int> estadoSeriesDasney; // Mapa para el estado de las series en Dasney
map<string, int> estadoSeriesBetflix; // Mapa para el estado de las series en Betflix
map<int, vector<double>> profesores;
map<int, vector<double>> profesoresTemp;
map<int, vector<string>> seriesProfes;
map<int, vector<string>> seriesProfesTemp;



pthread_cond_t cond_DasneyCreado = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_actualizarDasney = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mapa = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexDasney = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexDasney2 = PTHREAD_MUTEX_INITIALIZER; // Mutex para Dasney
pthread_mutex_t mutexDasney3 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexBetflix = PTHREAD_MUTEX_INITIALIZER; // Mutex para Betflix
pthread_mutex_t mutexCount = PTHREAD_MUTEX_INITIALIZER;
sem_t semaforoDasney; // semáforo para controlar el acceso a Dasney
sem_t semaforoBetflix; // semáforo para controlar el acceso a Betflix


pthread_cond_t cond_BetflixCreado = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutexCount1 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_actualizarBetflix = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutexBetflix2 = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t mutexBetflix3 = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutexActualizarSemana= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_PlataformasActualizadas = PTHREAD_COND_INITIALIZER;


void ImprimirSeriesProfesTempo() {
    cout << "-----------------Series vistas por los profesores en en el periodo ----------" << endl;
    for (auto& par : seriesProfesTemp) { // Itera sobre cada par clave-valor en el mapa
        int thread_index = par.first; // Obtiene la clave (índice del hilo)
        vector<string>& seriesVistas = par.second; // Obtiene el vector de series vistas (sin const para poder modificarlo)
        
        cout << "El profesor " << thread_index << " vio las series: ";
        
        // Imprime las series vistas
        if (seriesVistas.empty()) {
            cout << "No vio ninguna serie." << endl;
        } else {
            for (const string& serie : seriesVistas) {
                cout << serie << " - "; // Imprime cada serie en la misma línea
            }
            cout << endl; // Salto de línea al final de la lista de series
        }

        cout<<endl;
        
        // Vaciar el vector de series vistas
        seriesVistas.clear();
    }
}


void ImprimirSeriesProfes() {
    cout<<"-----------------Series vistas por los profesores en total ----------------------"<<endl;
    for (const auto& par : seriesProfes) { // Itera sobre cada par clave-valor en el mapa
        int thread_index = par.first; // Obtiene la clave (índice del hilo)
        const vector<string>& seriesVistas = par.second; // Obtiene el vector de series vistas
        
        cout << "El profesor " << thread_index << " vio las series: ";
        
        // Imprime las series vistas
        if (seriesVistas.empty()) {
            cout << "No vio ninguna serie." << endl;
        } else {
            for (const string& serie : seriesVistas) {
                cout << serie << " - "; // Imprime cada serie en la misma línea
            }
            cout << endl; // Salto de línea al final de la lista de series
        }

        cout<<endl;
    }
}

void imprimirContenidosTempo() {
    // Imprimir el contenido de los vectores
    int semana;
    for (auto& [id, vector] : profesoresTemp) { // Cambiado a auto& para permitir la modificación
        cout << "El profesor " << id << " ha visto : ";
         semana = numeroSemana;  // Inicializa el contador de semana
        for (double val : vector) {
            cout << " Semana " << semana+1 << ": " << val << " series; ";
            semana++;  // Incrementa el contador de semana
        }
        cout << endl;

        cout<<endl;

        // Vaciar el vector de series vistas
        vector.clear(); // Esto vacía el vector de este profesor
    }
    numeroSemana=semana;
}

void imprimirContenidos() {
    // Imprimir el contenido de los vectores
    for (const auto& [id, vector] : profesores) {
        cout << "El profesor " << id << " ha visto : ";
        int semana = 1;  // Inicializa el contador de semana
        for (double val : vector) {
            cout << " Semana " << semana << ": " << val << " series; ";
            semana++;  // Incrementa el contador de semana
        }
        cout << endl;
    }
}
void imprimirNumeroTotal() {
    // Imprimir el contenido de los vectores
    for (const auto& [id, vector] : profesores) {
        int semana = 1;  // Inicializa el contador de semana
        double totalSeries = 0; // Inicializa el total de series vistas por el profesor

        for (double val : vector) {
            totalSeries += val; // Suma el valor al total
            semana++;  // Incrementa el contador de semana
        }

        // Imprime el total de series vistas por el profesor
        cout << "Total de series vistas por el profesor "<<id<<" : " << totalSeries << endl; 
    }
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

void CrearBetflix(){
     cantidadSeriesBetflix = distrib2(gen); // Establecer la cantidad de series
    cout << "Cantidad de series para Betflix: " << cantidadSeriesBetflix << endl;
        // Señal para indicar que las series han sido creadas
         for (int i = 0; i < cantidadSeriesBetflix; i++) {
        estadoSeriesBetflix["Serie " + to_string(i)] = 0; // 0 significa que la serie no ha sido vista
        pthread_cond_broadcast(&cond_BetflixCreado);
    } 
}


//FUNCIÓN QUE EJECUTA UN THEAD ENCARGADO DE ACTUALIZAR PARAMETROS PARA LA SIGUIENTE SEMANA
void* NuevaSemana(void* arg){
    int i=0; //realiza conteo de las semanas (solo para un tema de tener un output mas entendible)
      while (semanas > 0) { //este while se ejecuta mientras queden mas de 0 semanas
      cout<< "---------SE ACABÓ UNA SEMANA---------"<<endl;
        i++; //aumenta el i en 1 para indicar que estamos en la ejecución de la primera semana

        //A continuación un ciclo que espera que tanto Signal1 como Signal 2 
        //cambien a valor 1. Esto ocurre cuando ambas plataformas actualizaron sus series
        pthread_mutex_lock(&mutexActualizarSemana);
        while (signal1 != 1 || signal2 != 1) {
            pthread_mutex_unlock(&mutexActualizarSemana);
            sleep(1); // Ajustar el tiempo de espera para evitar busy waiting
            pthread_mutex_lock(&mutexActualizarSemana);
        }
        
        //Reestablece las señales signal, disminuye en 1 las semanas y hace listo "true"
        signal1 = 0;
        signal2 = 0;
        semanas--;
        listo = true; // Indica que la semana ha terminado y deja avanzar a todos los threads 

        //envía una señal para que los threads que estaban esperando el listo =true avancen
        pthread_cond_broadcast(&cond_PlataformasActualizadas); 

        //espera unos segundos para esperar a que los threads avancen de la variable de condicion
        //esto para esperar que la señal "listo" vuelva a bloquear la variable 
        //antes de que todos los threads pasen
        sleep(8);

        
        
        if(i==4){
            cout<<"-----------YA PASÓ UN MES--------------"<<endl;
            imprimirContenidosTempo();
            ImprimirSeriesProfesTempo();

        }
        if(i==24){
            cout<<"-----------YA PASARON 6 MESES--------------"<<endl;
            imprimirContenidosTempo();
            ImprimirSeriesProfesTempo();

        }
        if(i==48){
            cout<<"-----------YA PASÓ UN AÑO--------------"<<endl;
            imprimirContenidosTempo();
            ImprimirSeriesProfesTempo();

        }
        listo = false; // Resetea la bandera de la variable de condicion 
        
        pthread_mutex_unlock(&mutexActualizarSemana);

        

}
return nullptr;
}

void* BetflixContenido(void* arg) {
    while (semanas > 0) {
        int seriesNuevas = distrib2(gen); // Número de series nuevas para agregar esta semana
        int seriesActuales = cantidadSeriesBetflix; // Cantidad actual de series en Betflix
        
        // Espera hasta que el contador sea 6
        pthread_mutex_lock(&mutexCount1);
        while (count1 != 6) {
            pthread_cond_wait(&cond_actualizarBetflix, &mutexCount1);
        }
        pthread_mutex_unlock(&mutexCount1);
        
        // Agrega nuevas series a Betflix
        cantidadSeriesBetflix += seriesNuevas; // Acumula la cantidad de series
        cout << "Esta semana Betflix añadirá : " << seriesNuevas <<" nuevas series"<< endl;
        cout << "Total de series en Betflix: " << cantidadSeriesBetflix << endl;

        // Actualiza el estado de las nuevas series
        for (int i = 0; i < seriesNuevas; i++) {
            estadoSeriesBetflix["Serie " + to_string(i + seriesActuales)] = 0; // Comienza desde la serie actual
        }

        count1 = 0;
        signal2 = 1;

        // Espera el paso a la otra semana
        pthread_mutex_lock(&mutexDasney2);
        while (!listo) {
            pthread_cond_wait(&cond_PlataformasActualizadas, &mutexDasney2);
        }
        pthread_mutex_unlock(&mutexDasney2);
    }

    return nullptr;
}


//Funcion exactamente igual que BetflixContenido, pero para Betflix
void* DasneyContenido(void* arg) {
    while (semanas > 0) {
        int seriesNuevas = distrib2(gen); // Número de series nuevas para agregar esta semana
        int seriesActuales = cantidadSeriesDasney; // Cantidad actual de series en Dasney
        
        pthread_mutex_lock(&mutexCount);
        while (count != 6) {
            pthread_cond_wait(&cond_actualizarDasney, &mutexCount);
        }
        pthread_mutex_unlock(&mutexCount);

        cantidadSeriesDasney += seriesNuevas; // Acumula la cantidad de series
        cout << "Esta semana Dasney añadirá : " << seriesNuevas <<" nuevas series"<< endl;
        cout << "Total de series en Dasney: " << cantidadSeriesDasney << endl;

        for (int i = 0; i < seriesNuevas; i++) {
            estadoSeriesDasney["Serie " + to_string(i + seriesActuales + 1)] = 0;
        }

        count = 0;
        signal1 = 1;

        // Espera el paso a la otra semana
        pthread_mutex_lock(&mutexDasney2);
        while (!listo) {
            pthread_cond_wait(&cond_PlataformasActualizadas, &mutexDasney2);
        }
        pthread_mutex_unlock(&mutexDasney2);
    }

    return nullptr;
}


void* Dasney(void* arg) {
    int thread_index = (long)arg;
    sleep(2);
    pthread_mutex_lock(&mapa);
    profesores[thread_index] = {};  // Inicializa el vector de series del thread
    seriesProfes[thread_index] = {}; 
    profesoresTemp[thread_index] = {}; 
    pthread_mutex_unlock(&mapa);
    

    while (semanas > 0) {
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
        profesores[thread_index].push_back(seriesPorVer);
        profesoresTemp[thread_index].push_back(seriesPorVer);

        // Marcar aleatoriamente las series vistas
        vector<string> series_disponibles;

        // Llenar series_disponibles solo con las series no vistas
        for (auto& [nombre, estado] : estadoSeriesDasney) {
            if (estado == 0) {
                series_disponibles.push_back(nombre);
            }
        }

        // Marcar como vistas una cantidad igual a seriesPorVer
        for (int i = 0; i < seriesPorVer && i < series_disponibles.size(); i++) {
            estadoSeriesDasney[series_disponibles[i]] = 1;
            seriesProfes[thread_index].push_back(series_disponibles[i]);
            seriesProfesTemp[thread_index].push_back(series_disponibles[i]);
        }

        pthread_mutex_unlock(&mutexDasney3);

        sem_post(&semaforoDasney);

        // Contabiliza el acceso de cada hilo
        pthread_mutex_lock(&mutexCount);
        count++;
        if (count == 6) {
            pthread_cond_signal(&cond_actualizarDasney);  // Señaliza que todos terminaron
        }
        pthread_mutex_unlock(&mutexCount);

        // Espera hasta que todos los hilos completen la semana y se actualicen las series
        pthread_mutex_lock(&mutexDasney2);
        while (!listo) {
            pthread_cond_wait(&cond_PlataformasActualizadas, &mutexDasney2);
        }
        pthread_mutex_unlock(&mutexDasney2);
        sleep(15);
    }

    return nullptr;
}


void* Betflix(void* arg) {
    sleep(3);
    pthread_mutex_lock(&mapa);
    int thread_index = (long)arg;
    profesores[thread_index] = {};  // Inicializa el vector de series del thread
    seriesProfes[thread_index] = {}; 
     profesoresTemp[thread_index] = {};
    pthread_mutex_unlock(&mapa);

    while (semanas > 0) {
        sem_wait(&semaforoBetflix);

        pthread_mutex_lock(&mutexBetflix);
        // Espera hasta que las series estén actualizadas para la semana actual
        while (estadoSeriesBetflix.size() == 0) {
            pthread_cond_wait(&cond_BetflixCreado, &mutexBetflix);
        }
        pthread_mutex_unlock(&mutexBetflix);

        pthread_mutex_lock(&mutexBetflix3);

        // Ver cuántas series puede ver el hilo
        double seriesPorVer = valores[distrib(gen)];
        profesores[thread_index].push_back(seriesPorVer);
        profesoresTemp[thread_index].push_back(seriesPorVer);

        // Marcar aleatoriamente las series vistas
        vector<string> series_disponibles;

        // Llenar series_disponibles solo con las series no vistas
        for (auto& [nombre, estado] : estadoSeriesBetflix) {
            if (estado == 0) {
                series_disponibles.push_back(nombre);
            }
        }

        // Marcar como vistas una cantidad igual a seriesPorVer
        for (int i = 0; i < seriesPorVer && i < series_disponibles.size(); i++) {
            estadoSeriesBetflix[series_disponibles[i]] = 1;
            seriesProfes[thread_index].push_back(series_disponibles[i]);
            seriesProfesTemp[thread_index].push_back(series_disponibles[i]);
            
        }

        pthread_mutex_unlock(&mutexBetflix3);

        sem_post(&semaforoBetflix);

        // Contabiliza el acceso de cada hilo
        pthread_mutex_lock(&mutexCount1);
        count1++;
        if (count1 == 6) {
            pthread_cond_signal(&cond_actualizarBetflix);  // Señaliza que todos terminaron
        }
        pthread_mutex_unlock(&mutexCount1);

        // Espera hasta que todos los hilos completen la semana y se actualicen las series
        pthread_mutex_lock(&mutexBetflix2);
        while (!listo) {
            pthread_cond_wait(&cond_PlataformasActualizadas, &mutexBetflix2);
        }
        pthread_mutex_unlock(&mutexBetflix2);
        sleep(15);
    }

    return nullptr;
}

void imprimirEstadoSeriesDasney() {
    cout << "Series vistas y no vistas:" << endl;
    for (const auto& [nombre, estado] : estadoSeriesDasney) {
        cout << "Serie: " << nombre << ", Estado: " << (estado == 0 ? "No vista" : "Vista") << endl;
    }
}

void imprimirEstadoSeriesBetflix() {
    cout << "Series vistas y no vistas:" << endl;
    for (const auto& [nombre, estado] : estadoSeriesBetflix) {
        cout << "Serie: " << nombre << ", Estado: " << (estado == 0 ? "No vista" : "Vista") << endl;
    }
}

int main(int argc, char* argv[]) {
    pthread_t threads[14];
    int status, option, personalizado;
    ofstream archivoSalida;
    streambuf* coutbuf = cout.rdbuf();  // Guarda el buffer original de cout

    while (true) {
        cout << "Indique el número de la opción que quiere ejecutar:\n";
        cout << "1. 1 semana\n";
        cout << "2. 1 mes\n";
        cout << "3. 6 meses\n";
        cout << "4. 1 año\n";
        cout << "5. Personalizado\n";
        cout << "Opción: ";
        cin >> option;

        if (option == 1) {
            semanas = 1;
            break;
        } else if (option == 2) {
            semanas = 4;
            break;
        } else if (option == 3) {
            semanas = 24;
            break;
        } else if (option == 4) {
            semanas = 48;
            archivoSalida.open("salida_punto1.txt");
            cout.rdbuf(archivoSalida.rdbuf());
            break;
        } else if (option == 5) {
            cout << "Ingrese el número de semanas a evaluar: ";
            cin >> personalizado;
            semanas = personalizado;
            if (personalizado >= 48) {
                cout << "Para las ejecuciones de más de 48 semanas, los resultados se guardarán en un archivo .txt" << endl;
                archivoSalida.open("salida_punto1.txt");
                cout.rdbuf(archivoSalida.rdbuf());
            }
            break;
        } else {
            cout << "Error. Opción inválida" << endl;
        }
    }

    CrearDasney();
    CrearBetflix();

    sem_init(&semaforoDasney, 0, 2);
    sem_init(&semaforoBetflix, 0, 1);

    for (long i = 0; i < 15; i++) {
        if (i <= 5) {
            printf("[main] Creando thread %ld para Dasney\n", i);
            status = pthread_create(&threads[i], NULL, Dasney, (void*)i);
        } else if (i > 5 && i <= 11) {
            printf("[main] Creando thread %ld para Betflix\n", i);
            status = pthread_create(&threads[i], NULL, Betflix, (void*)i);
        } else if (i == 12) {
            status = pthread_create(&threads[i], NULL, BetflixContenido, (void*)i);
        } else if (i == 13) {
            status = pthread_create(&threads[i], NULL, DasneyContenido, (void*)i);
        } else if (i == 14) {
            status = pthread_create(&threads[i], NULL, NuevaSemana, (void*)i);
        }

        if (status != 0) {
            cerr << "Error al crear el thread " << i << endl;
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < 15; i++) {
        pthread_join(threads[i], NULL);
    }

    cout << endl << "----------------------------------RESUMEN FINAL----------------------------------" << endl;
    imprimirNumeroTotal();
    cout << endl << "--------DETALLE DE LAS SERIES VISTAS POR PROFESOR------------------" << endl;
    ImprimirSeriesProfes();
    cout << endl << "--------------------INFORMACIÓN PLATAFORMA DASNEY--------------------------------" << endl;
    imprimirEstadoSeriesDasney();
    cout << endl << "--------------------INFORMACIÓN PLATAFORMA BETFLIX--------------------------------" << endl;
    imprimirEstadoSeriesBetflix();

    if (semanas >= 48) {
        cout.rdbuf(coutbuf);
        archivoSalida.close();
    }

    sem_destroy(&semaforoDasney);
    sem_destroy(&semaforoBetflix);

    return 0;
}






