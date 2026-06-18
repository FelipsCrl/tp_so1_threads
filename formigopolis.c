#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
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
}

void esperar(Pessoa *pessoa, monitor_caixa *mc)
{
    pthread_mutex_lock(&mc->mutex);
    printf("%s está na fila do caixa\n", pessoa->nome);
    pthread_mutex_unlock(&mc->mutex);
}

void liberar(Pessoa *pessoa, monitor_caixa *mc)
{
    printf("%s vai para casa\n", pessoa->nome);
}

void atendidoPeloCaixa(Pessoa *pessoa)
{
    printf("%s está sendo atendido(a)\n",pessoa->nome);
}

void vaiEmboraParaCasa(Pessoa *pessoa)
{
    printf("%s vai para casa\n",pessoa->nome);
}

void verificar(Pessoa *pessoa, monitor_caixa *mc)
{
    // gerente verifica se ha deadlock
}

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        printf("Número de parâmetros inválido!\n");
        return 1;
    }

    int num_vezes_caixa = atoi(argv[1]);
    return 0;
}
