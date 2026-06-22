#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "formigopolis.h"

void monitorInit(monitor_caixa *mc)
{
    pthread_mutex_init(&mc->mutex, NULL);
    mc->caixaOcupado = 0;
    mc->quantidadeDePessoasNaFila=0;

    // Inicializa filas e variáveis de condição dinamicamente
    for (int i = 0; i < QTD_PRIORIDADES; i++)
    {
        pthread_cond_init(&mc->condincaoPorPrioridade[i], NULL);
    }
}

void esperar(Pessoa *pessoa, monitor_caixa *mc)
{
    pthread_mutex_lock(&mc->mutex);

    printf("%s está na fila do caixa", pessoa->nome);
    imprimeFila(mc);

    mc->filaCaixa[mc->quantidadeDePessoasNaFila] = *pessoa;
    mc->quantidadeDePessoasNaFila++;

    while (mc->caixaOcupado == 1 || (proximaPrioridade(mc) < pessoa->prioridadeAtual))
    {
        pthread_cond_wait(&mc->condincaoPorPrioridade[pessoa->prioridadeAtual], &mc->mutex);
    }

    mc->caixaOcupado = 1;

    removeDaFila(pessoa, mc);

    pthread_mutex_unlock(&mc->mutex);
}

void removeDaFila(Pessoa *pessoa, monitor_caixa *mc)
{
    int indice = -1;

    for (int i = 0; i < mc->quantidadeDePessoasNaFila; i++)
    {

        if (strcmp(mc->filaCaixa[i].nome, pessoa->nome) == 0)
        {
            indice = i;
            break;
        }
    }

    if (indice != -1)
    {
        for (int i = indice; i < mc->quantidadeDePessoasNaFila - 1; i++)
        {
            mc->filaCaixa[i] = mc->filaCaixa[i + 1];
        }

        mc->quantidadeDePessoasNaFila--;
    }
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

// void verificar(Pessoa *pessoa, monitor_caixa *mc)
// {
//     // gerente verifica se ha deadlock
// }

void imprimeFila(monitor_caixa *mc)
{
    printf("{fila:");
    for (int i = 0; i < mc->quantidadeDePessoasNaFila; i++)
    {
        printf("%c", mc->filaCaixa[i].nome[0]);
    }
    printf("}\n");
}

int proximaPrioridade(monitor_caixa *mc)
{
    int proxPrio = 5;

    for (int i = 0; i < QTD_PRIORIDADES; i++)
    {
        if (mc->filaCaixa[i].prioridadeAtual > 0)
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
    monitorInit(&monitorCaixa);
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