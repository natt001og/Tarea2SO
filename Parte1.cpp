#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int main(int argc, char *argv[])
{   //declaracion de los threads                 
    pthread_t threads[12];
    int status;
    void* Dasney (){
            return;
        }

   //creación de los threads
   for(long i=0; i < 12; i++)
    {
        if(i<=5){
        printf("[main] Creando thread %ld\n", i);
        status = pthread_create(&threads[i], NULL, Dasney, (void *)i);    
        }else{
            printf("[main] Creando thread %ld\n", i);
        status = pthread_create(&threads[i], NULL, Betflix, (void *)i);
        }
        if (status != 0)
        {
            printf("[main] Oops. pthread_create retorno el codigo de error %d\n", status);
            exit(-1);
        }
    }
}