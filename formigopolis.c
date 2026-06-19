#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "formigopolis.h"

void monitor_init(monitor_caixa *mc)
{
    pthread_mutex_init(&mc->mutex, NULL);

    mc->caixaOcupado = 0;

    // Inicializa filas e variáveis de condição dinamicamente
    for (int i = 0; i < QTD_PRIORIDADES; i++)
    {
        mc->filaCaixa[i] = 0;
        pthread_cond_init(&mc->condincaoPorPrioridade[i], NULL);
    }

    mc->filaLetrasNome[0] = '\0'; // Inicializa a fila de letras dos nomes
}

void esperar(Pessoa *pessoa, monitor_caixa *mc)
{
    pthread_mutex_lock(&mc->mutex);

    adicionar_letra_fila(pessoa->nome[0], mc);
    printf("%s está na fila do caixa", pessoa->nome);
    imprimeFila(mc);

    mc->filaCaixa[pessoa->prioridadeAtual]++;

    while (mc->caixaOcupado == 1 || (proximaPrioridade(mc) < pessoa->prioridadeAtual))
    {
        pthread_cond_wait(&mc->condincaoPorPrioridade[pessoa->prioridadeAtual], &mc->mutex);
    }

    mc->filaCaixa[pessoa->prioridadeAtual]--;
    mc->caixaOcupado = 1;

    remover_letra_fila(pessoa->nome[0], mc);

    pthread_mutex_unlock(&mc->mutex);
}

void liberar(Pessoa *pessoa, monitor_caixa *mc)
{
    pthread_mutex_lock(&mc->mutex);

    mc->caixaOcupado = 0;
    
    // Reseta a prioridade de quem acabou de usar o caixa
    pessoa->prioridadeAtual = pessoa->prioridadeOriginal;
    pessoa->vezesFuradas = 0;

    int proximaPessoa = proximaPrioridade(mc);

    for (int i = 0; i < QTD_PRIORIDADES; i++)
    {
        /* code */
    }

    if (proximaPessoa != 5)
    {
        pthread_cond_signal(&mc->condincaoPorPrioridade[proximaPessoa]);
    }

    pthread_mutex_unlock(&mc->mutex);
}

void atendidoPeloCaixa(Pessoa *pessoa, monitor_caixa *mc)
{
    printf("%s está sendo atendido(a)", pessoa->nome);
    imprimeFila(mc);
}

void vaiEmboraParaCasa(Pessoa *pessoa, monitor_caixa *mc)
{
    printf("%s vai para casa", pessoa->nome);
    imprimeFila(mc);
}

void verificar(Pessoa *pessoa, monitor_caixa *mc)
{
    // gerente verifica se ha deadlock
}

void imprimeFila(monitor_caixa *mc)
{
    printf("{fila:%s}\n", mc->filaLetrasNome);
}

// Adiciona a letra no final da string
void adicionar_letra_fila(char inicial, monitor_caixa *mc)
{
    int tamanho = strlen(mc->filaLetrasNome);
    mc->filaLetrasNome[tamanho] = inicial;
    mc->filaLetrasNome[tamanho + 1] = '\0';
}

// Remove a letra quando a pessoa sai da fila para ser atendida
void remover_letra_fila(char inicial, monitor_caixa *mc)
{
    int tamanho = strlen(mc->filaLetrasNome);
    for (int i = 0; i < tamanho; i++)
    {
        if (mc->filaLetrasNome[i] == inicial)
        {
            // Puxa todas as letras da direita para a esquerda
            for (int j = i; j < tamanho; j++)
            {
                mc->filaLetrasNome[j] = mc->filaLetrasNome[j + 1];
            }
            break; // Remove só a primeira que encontrar
        }
    }
}

int proximaPrioridade(monitor_caixa *mc)
{
    int proxPrio = 5;

    for (int i = 0; i < QTD_PRIORIDADES; i++)
    {
        if (mc->filaCaixa[i] > 0)
        {
            proxPrio = i;
            return proxPrio;
        }
    }
    return proxPrio;
}

void *threadFuncao(void *argumento)
{
    ThreadData *argumentos = (ThreadData *)argumento;
    Pessoa *pessoa = argumentos->pessoa;
    monitor_caixa *mc = argumentos->mc;

    for (int i = 0; i < argumentos->numVezesCaixa; i++)
    {
        sleep((rand() % 3) + 3);

        esperar(pessoa, mc);

        atendidoPeloCaixa(pessoa, mc);

        sleep(1);

        liberar(pessoa, mc);
        vaiEmboraParaCasa(pessoa, mc);
    }
    return NULL;
}

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        printf("Número de parâmetros inválido!\n");
        return 1;
    }

    int numVezesCaixa = atoi(argv[1]);
    srand(time(NULL));
    monitor_caixa monitorCaixa;
    monitor_init(&monitorCaixa);
    Pessoa civil[QTD_PRIORIDADES * 2] =
        {

            {"Maria", GRAVIDA, GRAVIDA, 0},
            {"Marcos", GRAVIDA, GRAVIDA, 0},
            {"Vanda", IDOSO, IDOSO, 0},
            {"Valter", IDOSO, IDOSO, 0},
            {"Paula", DEFICIENTE, DEFICIENTE, 0},
            {"Pedro", DEFICIENTE, DEFICIENTE, 0},
            {"Sueli", COMUM, COMUM, 0},
            {"Silas", COMUM, COMUM, 0}

        };

    pthread_t threadsPessoas[QTD_PRIORIDADES * 2];
    ThreadData argumentos[QTD_PRIORIDADES * 2];

    for (int i = 0; i < QTD_PRIORIDADES * 2; i++)
    {
        argumentos[i].pessoa = &civil[i];
        argumentos[i].mc = &monitorCaixa;
        argumentos[i].numVezesCaixa = numVezesCaixa;
        pthread_create(&threadsPessoas[i], NULL, threadFuncao, &argumentos[i]);
    }

    for (int i = 0; i < QTD_PRIORIDADES * 2; i++)
    {
        pthread_join(threadsPessoas[i], NULL);
    }

    // Destroi o mutex e as variáveis de condição
    pthread_mutex_destroy(&monitorCaixa.mutex);
    for (int i = 0; i < QTD_PRIORIDADES; i++)
    {
        pthread_cond_destroy(&monitorCaixa.condincaoPorPrioridade[i]);
    }

    return 0;
}