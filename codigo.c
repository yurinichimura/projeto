#define _GNU_SOURCE
#include <stdlib.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sched.h>
#include <stdio.h>
#include <pthread.h> // Adicionamos a biblioteca pthread para usar mutex
#include <time.h>    // Adicionamos a biblioteca time para inicializar a semente aleatória

// 64kB stack
#define FIBER_STACK (1024 * 64)

struct c {
    int saldo;
};

typedef struct c conta;
conta contas[2]; // Alteramos para um array de contas
int valor;

// Mutex para controlar o acesso às contas
pthread_mutex_t mutex;

// Função de transferência
void* transferencia(void* arg)
{
    int i;
    for (i = 0; i < 10; i++) {
        // Escolhe contas aleatoriamente
        int conta_debitar = rand() % 2;
        int conta_creditar = 1 - conta_debitar;

        pthread_mutex_lock(&mutex); // Bloqueia o acesso às contas
        if (contas[conta_debitar].saldo >= valor) {
            contas[conta_debitar].saldo -= valor;
            contas[conta_creditar].saldo += valor;
        }
        pthread_mutex_unlock(&mutex); // Libera o acesso às contas

        // Constrói as mensagens
        char msg_saldo_conta0[100];
        char msg_saldo_conta1[100];
        char msg_transferencia[100];

        snprintf(msg_saldo_conta0, sizeof(msg_saldo_conta0), "Saldo da conta 0: %d\n", contas[0].saldo);
        snprintf(msg_saldo_conta1, sizeof(msg_saldo_conta1), "Saldo da conta 1: %d\n", contas[1].saldo);
        snprintf(msg_transferencia, sizeof(msg_transferencia), "Transferência concluída com sucesso!\n");

        // Imprime as mensagens
        printf("%s%s%s", msg_saldo_conta0, msg_saldo_conta1, msg_transferencia);
    }
    return NULL;
}

int main()
{
    void* stack;
    pid_t pid;
    int i;

    // Inicializa o mutex
    pthread_mutex_init(&mutex, NULL);

    // Inicializa a semente aleatória
    srand(time(NULL));

    // Aloca o stack
    stack = malloc(FIBER_STACK);
    if (stack == 0) {
        perror("malloc: could not allocate stack");
        exit(1);
    }

    // Inicializa as contas com saldo 100
    for (i = 0; i < 2; i++) {
        contas[i].saldo = 100;
    }

    printf("Transferindo 10 entre contas aleatórias\n");
    valor = 10;

    // Cria 100 threads para realizar as transferências
    pthread_t threads[100];
    for (i = 0; i < 100; i++) {
        if (pthread_create(&threads[i], NULL, transferencia, NULL) != 0) {
            perror("pthread_create");
            exit(2);
        }
    }

    // Espera que todas as threads terminem
    for (i = 0; i < 100; i++) {
        pthread_join(threads[i], NULL);
    }

    // Libera o stack
    free(stack);

    // Destrói o mutex
    pthread_mutex_destroy(&mutex);

    printf("Transferências concluídas e memória liberada.\n");
    return 0;
}
# projeto
