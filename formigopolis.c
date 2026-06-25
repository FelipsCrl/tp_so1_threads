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
    mc->quantidadeDePessoasNaFila = 0;

    // Inicializa filas e variáveis de condição dinamicamente
    for (int i = 0; i < QTD_PRIORIDADES * 2; i++)
    {
        pthread_cond_init(&mc->condincaoPorPessoa[i], NULL);
    }
}

void envelhecerPessoas(Pessoa *pessoaChamada, monitor_caixa *mc)
{
    int indice = -1;

    for (int i = 0; i < mc->quantidadeDePessoasNaFila; i++)
    {
        if (mc->filaCaixa[i] == pessoaChamada)
        {
            indice = i;
            break;
        }
    }

    for (int j = 0; j < indice; j++)
    {
        mc->filaCaixa[j]->vezesFuradas++;
        if (mc->filaCaixa[j]->vezesFuradas >= 2 && mc->filaCaixa[j]->prioridadeAtual > GRAVIDA)
        {
            mc->filaCaixa[j]->prioridadeAtual--; // aumenta a prioridade
            mc->filaCaixa[j]->vezesFuradas = 0;  // zera o contador de vezes furadas
            detectouInanicao(mc->filaCaixa[j]);
        }
    }
}

void esperar(Pessoa *pessoa, monitor_caixa *mc)
{
    pthread_mutex_lock(&mc->mutex);

    mc->filaCaixa[mc->quantidadeDePessoasNaFila] = pessoa;
    mc->quantidadeDePessoasNaFila++;

    printf("%s está na fila do caixa", pessoa->nome);
    imprimeFila(mc);
    fflush(stdout);

    while (mc->caixaOcupado || (pessoa != proximaPessoaPrioridade(mc) && pessoa != mc->escolhidaGerente))
    {
        pthread_cond_wait(&mc->condincaoPorPessoa[pessoa->id], &mc->mutex);
    }

    mc->caixaOcupado = 1;

    envelhecerPessoas(pessoa, mc);

    removeDaFila(pessoa, mc);

    pessoa->vezesFuradas = 0;
    pessoa->prioridadeAtual = pessoa->prioridadeOriginal;

    pthread_mutex_unlock(&mc->mutex);
}

void removeDaFila(Pessoa *pessoa, monitor_caixa *mc)
{
    int indice = -1;

    for (int i = 0; i < mc->quantidadeDePessoasNaFila; i++)
    {

        if (strcmp(mc->filaCaixa[i]->nome, pessoa->nome) == 0)
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

    if (mc->quantidadeDePessoasNaFila != 0)
    {
        Pessoa *proximaPessoa = proximaPessoaPrioridade(mc);
        pthread_cond_signal(&mc->condincaoPorPessoa[proximaPessoa->id]);
    }

    pthread_mutex_unlock(&mc->mutex);
}

void atendidoPeloCaixa(Pessoa *pessoa, monitor_caixa *mc)
{
    printf("%s está sendo atendido(a)", pessoa->nome);
    imprimeFila(mc);
    fflush(stdout);
}

void detectouInanicao(Pessoa *pessoa)
{
    printf("Gerente detectou inanição, aumentando prioridade de %s\n", pessoa->nome);
    fflush(stdout);
}

void vaiEmboraParaCasa(Pessoa *pessoa, monitor_caixa *mc)
{
    printf("%s vai para casa", pessoa->nome);
    imprimeFila(mc);
    fflush(stdout);
}

void verificar(monitor_caixa *mc)
{
    pthread_mutex_lock(&mc->mutex);
    int temGravida = 0;
    int temIdoso = 0;
    int temDeficiente = 0;

    if (mc->caixaOcupado == 0 && mc->quantidadeDePessoasNaFila >= 3)
    {
        for (int i = 0; i < mc->quantidadeDePessoasNaFila; i++)
        {
            if (mc->filaCaixa[i]->prioridadeAtual == GRAVIDA)
                temGravida = 1;
            else if (mc->filaCaixa[i]->prioridadeAtual == IDOSO)
                temIdoso = 1;
            else if (mc->filaCaixa[i]->prioridadeAtual == DEFICIENTE)
                temDeficiente = 1;
        }
    }

    if (temGravida && temIdoso && temDeficiente)
    {
        int numeroEscolhido = rand() % mc->quantidadeDePessoasNaFila;
        Pessoa *escolhida = mc->filaCaixa[numeroEscolhido];
        mc->escolhidaGerente = escolhida;
        printf("Gerente detectou deadlock, liberando %s para atendimento\n", escolhida->nome);
        pthread_cond_signal(&mc->condincaoPorPessoa[escolhida->id]);
    }
    pthread_mutex_unlock(&mc->mutex);
}

void imprimeFila(monitor_caixa *mc)
{
    printf(" {fila:");
    for (int i = 0; i < mc->quantidadeDePessoasNaFila; i++)
    {
        printf("%c", mc->filaCaixa[i]->nome[0]);
    }
    printf("}\n");
    fflush(stdout);
}

Pessoa *proximaPessoaPrioridade(monitor_caixa *mc)
{
    int temGravida = 0, temIdoso = 0, temDeficiente = 0;

    for (int i = 0; i < mc->quantidadeDePessoasNaFila; i++) {
        if (mc->filaCaixa[i]->prioridadeAtual == GRAVIDA) temGravida = 1;
        if (mc->filaCaixa[i]->prioridadeAtual == IDOSO) temIdoso = 1;
        if (mc->filaCaixa[i]->prioridadeAtual == DEFICIENTE) temDeficiente = 1;
    }
    if (temGravida && temIdoso && temDeficiente) {
        return NULL; 
    }

    Pessoa *escolhida = mc->filaCaixa[0];

    for (int i = 1; i < mc->quantidadeDePessoasNaFila; i++)
    {
        Pessoa *candidato = mc->filaCaixa[i];
        

        if (escolhida->prioridadeAtual == GRAVIDA && candidato->prioridadeAtual == DEFICIENTE)
        {
            escolhida = candidato;
        }

        else if (candidato->prioridadeAtual < escolhida->prioridadeAtual)
        {
            if (!(candidato->prioridadeAtual == GRAVIDA && escolhida->prioridadeAtual == DEFICIENTE))
            {
                escolhida = candidato;
            }
        }
    }
    return escolhida;
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
void *threadGerente(void *argumento)
{
    monitor_caixa *mc = (monitor_caixa *)argumento;

    while (1)
    {
        sleep(TEMPO);
        verificar(mc);
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

            {0, "Maria", GRAVIDA, GRAVIDA, 0},
            {0, "Marcos", GRAVIDA, GRAVIDA, 0},
            {0, "Vanda", IDOSO, IDOSO, 0},
            {0, "Valter", IDOSO, IDOSO, 0},
            {0, "Paula", DEFICIENTE, DEFICIENTE, 0},
            {0, "Pedro", DEFICIENTE, DEFICIENTE, 0},
            {0, "Sueli", COMUM, COMUM, 0},
            {0, "Silas", COMUM, COMUM, 0}

        };

    pthread_t threadsPessoas[QTD_PRIORIDADES * 2];
    ThreadData argumentos[QTD_PRIORIDADES * 2];

    for (int i = 0; i < QTD_PRIORIDADES * 2; i++)
    {
        argumentos[i].pessoa = &civil[i];
        argumentos[i].pessoa->id = i; // Id de criação para separar as threads
        argumentos[i].mc = &monitorCaixa;
        argumentos[i].numVezesCaixa = numVezesCaixa;
        pthread_create(&threadsPessoas[i], NULL, threadFuncao, &argumentos[i]);
    }

    pthread_t idGerente;
    pthread_create(&idGerente, NULL, threadGerente, &monitorCaixa);

    for (int i = 0; i < QTD_PRIORIDADES * 2; i++)
    {
        pthread_join(threadsPessoas[i], NULL);
    }

    // Destroi o mutex e as variáveis de condição
    pthread_mutex_destroy(&monitorCaixa.mutex);
    for (int i = 0; i < QTD_PRIORIDADES * 2; i++)
    {
        pthread_cond_destroy(&monitorCaixa.condincaoPorPessoa[i]);
    }

    return 0;
}