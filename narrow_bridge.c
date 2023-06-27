#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_CARS 100

int carsA = 0;  // Liczba samochodów po stronie miasta A
int carsB = 0;  // Liczba samochodów po stronie miasta B
int queueA = 0; // Liczba samochodów oczekujących przed mostem (miasto A)
int queueB = 0; // Liczba samochodów oczekujących przed mostem (miasto B)

sem_t semA;     // Semafor dla samochodów z miasta A
sem_t semB;     // Semafor dla samochodów z miasta B
pthread_mutex_t mutex;  // Mutex dla synchronizacji dostępu do zmiennych

void enterBridge(char side, int carNumber) {
    pthread_mutex_lock(&mutex);  // Blokujemy mutex przed modyfikacją zmiennych

    if (side == 'A') {
        queueA++;  // Zwiększamy liczbę samochodów oczekujących w kolejce (miasto A)
        printf("A-%d %d>>> [>> %d >>] <<<%d %d-B\n", carsA, queueA, carNumber, queueB, carsB);
    } else {
        queueB++;  // Zwiększamy liczbę samochodów oczekujących w kolejce (miasto B)
        printf("A-%d %d>>> [<< %d <<] <<<%d %d-B\n", carsA, queueA, carNumber, queueB, carsB);
    }

    pthread_mutex_unlock(&mutex);  // Odblokowujemy mutex po modyfikacji zmiennych
}

void exitBridge(char side, int carNumber) {
    pthread_mutex_lock(&mutex);  // Blokujemy mutex przed modyfikacją zmiennych

    if (side == 'A') {
        queueA--;  // Zmniejszamy liczbę samochodów oczekujących w kolejce (miasto A)
        carsB++;   // Zwiększamy liczbę samochodów po stronie miasta A
       // printf("A-%d %d>>> [ %d ] <<<%d %d-B\n", carsA, queueA, carNumber, queueB, carsB);
    } else {
        queueB--;  // Zmniejszamy liczbę samochodów oczekujących w kolejce (miasto B)
        carsA++;   // Zwiększamy liczbę samochodów po stronie miasta B
        //printf("A-%d %d>>> [ %d ]<<<%d %d-B\n", carsA, queueA, carNumber, queueB, carsB);
    }

    pthread_mutex_unlock(&mutex);  // Odblokowujemy mutex po modyfikacji zmiennych
}

void* carThread(void* arg) {
    int carNumber = *(int*)arg;
    char side;

    if (rand() % 2 == 0) {
        side = 'A';  // Samochód zaczyna w mieście A
        sem_wait(&semA);  // Czekamy na zezwolenie wjazdu na most (semA)
    } else {
        side = 'B';  // Samochód zaczyna w mieście B
        sem_wait(&semB);  // Czekamy na zezwolenie wjazdu na most (semB)
    }

    enterBridge(side, carNumber);  // Samochód wjeżdża na most

    // Symulacja przejazdu przez most
    usleep(1000000);

    exitBridge(side, carNumber);  // Samochód zjeżdża z mostu

    if (side == 'A') {
        sem_post(&semB);  // Zgłaszamy gotowość do wjazdu na most (miasto B)
    } else {
        sem_post(&semA);  // Zgłaszamy gotowość do wjazdu na most (miasto A)
    }

    free(arg);
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Użycie: %s <liczba_samochodów>\n", argv[0]);
        return 1;
    }

    int numCars = atoi(argv[1]);
    if (numCars < 1 || numCars > MAX_CARS) {
        printf("Liczba samochodów musi być z zakresu 1-%d\n", MAX_CARS);
        return 1;
    }

    pthread_t threads[numCars];
    srand(time(NULL));

    // Inicjalizacja semaforów
    sem_init(&semA, 0, 1);
    sem_init(&semB, 0, 0);

    // Inicjalizacja mutexu
    pthread_mutex_init(&mutex, NULL);

    // Tworzenie wątków samochodów
    for (int i = 0; i < numCars; i++) {
        int* carNumber = malloc(sizeof(int));
        *carNumber = i + 1;
        pthread_create(&threads[i], NULL, carThread, carNumber);
    }

    // Oczekiwanie na zakończenie wątków
    for (int i = 0; i < numCars; i++) {
        pthread_join(threads[i], NULL);
    }

    // Zwalnianie zasobów
    sem_destroy(&semA);
    sem_destroy(&semB);
    pthread_mutex_destroy(&mutex);

    return 0;
}
