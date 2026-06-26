#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "formigopolis.h"

// Inicializa o monitor (caixa da lotérica)
void monitorInit(MonitorCaixa *mc)
{
    pthread_mutex_init(&mc->mutex, NULL);
    mc->caixaOcupado = 0;
    mc->quantidadeDePessoasNaFila = 0;
    mc->escolhidaGerente = NULL;

    // Inicializa filas e variáveis de condição dinamicamente
    for (int i = 0; i < QTD_PRIORIDADES * 2; i++)
    {
        pthread_cond_init(&mc->condincaoPorPessoa[i], NULL);
    }
}

// Função para 'envelhecer' as pessoas que foram puladas na fila
void envelhecerPessoas(Pessoa *pessoaChamada, MonitorCaixa *mc)
{
    int indice = -1;

    // Encontra o índice da pessoa (a que teve maior prioridade e furou a fila) na fila
    for (int i = 0; i < mc->quantidadeDePessoasNaFila; i++)
    {
        if (mc->filaCaixa[i] == pessoaChamada)
        {
            indice = i;
            break;
        }
    }

    // Após encontrar a pessoa e verificar se furou a fila, 'envelhece' todas as outras que foram puladas por ela na fila
    for (int j = 0; j < indice; j++)
    {
        mc->filaCaixa[j]->vezesFuradas++;

        // Se a pessoa foi pulada 2 vezes e ainda não está na prioridade máxima, então ela ganha prioridade
        if (mc->filaCaixa[j]->vezesFuradas >= 2 && mc->filaCaixa[j]->prioridadeAtual > GRAVIDA)
        {
            mc->filaCaixa[j]->prioridadeAtual--; // Aumenta a prioridade
            mc->filaCaixa[j]->vezesFuradas = 0;  // Zera o contador de vezes furadas
            detectouInanicao(mc->filaCaixa[j]);  // Exibre mensagem de detecção de inanição
        }
    }
}

// Função para colocar as threads (pessoas) na fila do caixa
void esperar(Pessoa *pessoa, MonitorCaixa *mc)
{
    pthread_mutex_lock(&mc->mutex); // Tranca o monitor para somente uma thread usar

    // Insere a pessoa na fila e incrementa o contador
    mc->filaCaixa[mc->quantidadeDePessoasNaFila] = pessoa;
    mc->quantidadeDePessoasNaFila++;

    estaNaFilaCaixa(pessoa, mc); // Exibre mensagem que a pessoa está na fila do caixa

    // Se o caixa está ocupado e não é a vez da pessoa, então ela se tranca
    while (mc->caixaOcupado || (mc->escolhidaGerente != NULL && pessoa != mc->escolhidaGerente) || (mc->escolhidaGerente == NULL && pessoa != proximaPessoaPrioridade(mc)))
    {
        pthread_cond_wait(&mc->condincaoPorPessoa[pessoa->id], &mc->mutex); // Thread se tranca até ser chamada
    }

    mc->caixaOcupado = 1; // Caixa ocupado pela pessoa que foi chamada

    envelhecerPessoas(pessoa, mc); // Verifica se furou a fila
    removeDaFila(pessoa, mc);      // Remove a pessoa da fila de espera

    atendidoPeloCaixa(pessoa, mc); // Exibe mensagem que entrou para o caixa

    if (mc->escolhidaGerente == pessoa)
        mc->escolhidaGerente = NULL; // Reseta a pessoa escolhida pelo gerente

    pthread_mutex_unlock(&mc->mutex); // Destranca o monitor
}

// Função que remove a pessoa da fila, se foi para ser atendida no caixa
void removeDaFila(Pessoa *pessoa, MonitorCaixa *mc)
{
    int indice = -1;

    for (int i = 0; i < mc->quantidadeDePessoasNaFila; i++)
    {
        // Busca a pessoa a ser removida da fila
        if (strcmp(mc->filaCaixa[i]->nome, pessoa->nome) == 0)
        {
            indice = i;
            break;
        }
    }

    if (indice != -1)
    {
        // Reorganiza a fila, para não conter 'buracos', após a remoção
        for (int i = indice; i < mc->quantidadeDePessoasNaFila - 1; i++)
        {
            mc->filaCaixa[i] = mc->filaCaixa[i + 1];
        }

        mc->quantidadeDePessoasNaFila--; // Decrementa o contador de pessoas na fila
    }
}

// Função que libera as pessoas do caixa
void liberar(Pessoa *pessoa, MonitorCaixa *mc)
{
    pthread_mutex_lock(&mc->mutex); // Tranca o monitor para somente uma thread usar

    mc->caixaOcupado = 0; // Coloca o caixa como 'liberado'

    // Se a pessoa foi atendida, reseta as vezes furadas e sua prioridade
    pessoa->vezesFuradas = 0;
    pessoa->prioridadeAtual = pessoa->prioridadeOriginal;

    // Verifica se ainda existe pessoas esperando para serem atendidas
    if (mc->quantidadeDePessoasNaFila != 0)
    {
        // Se o gerente escolheu alguém (houve deadlock), chama essa pessoa
        if (mc->escolhidaGerente != NULL)
        {
            // Acorda a thread escolhida para usar o caixa
            pthread_cond_signal(&mc->condincaoPorPessoa[mc->escolhidaGerente->id]);
        }
        else
        {
            Pessoa *proximaPessoa = proximaPessoaPrioridade(mc); // Vê qual deve ser a próxima pessoa a ser atendida

            if (proximaPessoa != NULL)
            {
                // Acorda a thread escolhida para usar o caixa
                pthread_cond_signal(&mc->condincaoPorPessoa[proximaPessoa->id]);
            }
        }
    }

    vaiEmboraParaCasa(pessoa, mc); // Exibe mensagem que vai para casa

    pthread_mutex_unlock(&mc->mutex); // Destranca o monitor
}

// Função que exibe mensagem que a pessoa está sendo atendida
void atendidoPeloCaixa(Pessoa *pessoa, MonitorCaixa *mc)
{
    printf("%s está sendo atendido(a)", pessoa->nome);
    imprimeFila(mc);
    fflush(stdout);
}

// Função que exibe mensagem que a pessoa está na fila do caixa
void estaNaFilaCaixa(Pessoa *pessoa, MonitorCaixa *mc)
{
    printf("%s está na fila do caixa", pessoa->nome);
    imprimeFila(mc);
    fflush(stdout);
}

// Função que exibe mensagem que gerente detectou deadlock
void detectouDeadlock(Pessoa *pessoa)
{
    printf("Gerente detectou deadlock, liberando %s para atendimento\n", pessoa->nome);
    fflush(stdout);
}

// Função que exibe mensagem que gerente detectou inanição
void detectouInanicao(Pessoa *pessoa)
{
    printf("Gerente detectou inanição, aumentando prioridade de %s\n", pessoa->nome);
    fflush(stdout);
}

// Função que exibe mensagem que a pessoa foi atendida e está indo para casa
void vaiEmboraParaCasa(Pessoa *pessoa, MonitorCaixa *mc)
{
    printf("%s vai para casa", pessoa->nome);
    imprimeFila(mc);
    fflush(stdout);
}

// Função usada pelo gerente para verificar a ocorrência de deadlock
void verificar(MonitorCaixa *mc)
{
    pthread_mutex_lock(&mc->mutex); // Tranca o monitor para somente uma thread usar

    // Varíaveis para verificar a existência de grávida, idoso e deficiente
    int temGravida = 0, temIdoso = 0, temDeficiente = 0;

    // Se no caixa existe pelo menos 3 pessoas e ele está liberado, verifica se dentre as pessoas existe uma de cada tipo
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

    // Se existe uma pessoa de cada tipo, há um deadlock
    if (temGravida && temIdoso && temDeficiente)
    {
        Pessoa *candidatos[QTD_PRIORIDADES * 2];
        int numeroDePessoas = 0;

        // Separa da fila e pega dela somente as pessoas que tem prioridade para entrar no sorteio
        for (int i = 0; i < mc->quantidadeDePessoasNaFila; i++)
        {
            if (mc->filaCaixa[i]->prioridadeAtual != COMUM)
            {
                candidatos[numeroDePessoas++] = mc->filaCaixa[i];
            }
        }

        Pessoa *escolhida = candidatos[rand() % numeroDePessoas]; // Sorteia a pessoa a ser chamada

        mc->escolhidaGerente = escolhida; // Atribui a escolha do gerente

        detectouDeadlock(escolhida); // Exibe mensagem de deadlock

        pthread_cond_signal(&mc->condincaoPorPessoa[escolhida->id]); // Acorda a thread escolhida para ser a próxima
    }

    pthread_mutex_unlock(&mc->mutex); // Destranca o monitor
}

// Função que imprime a ordem da fila no exato momento
void imprimeFila(MonitorCaixa *mc)
{
    printf(" {fila:");
    for (int i = 0; i < mc->quantidadeDePessoasNaFila; i++)
    {
        printf("%c", mc->filaCaixa[i]->nome[0]);
    }
    printf("}\n");
    fflush(stdout);
}

// Função que analisa qual é a próxima pessoa a ser atendida
Pessoa *proximaPessoaPrioridade(MonitorCaixa *mc)
{
    // Varíaveis para verificar a existência de grávida, idoso e deficiente
    int temGravida = 0, temIdoso = 0, temDeficiente = 0;

    // Verifica se existe pelo menos uma pessoa de cada uma das três prioridades
    for (int i = 0; i < mc->quantidadeDePessoasNaFila; i++)
    {
        if (mc->filaCaixa[i]->prioridadeAtual == GRAVIDA)
            temGravida = 1;
        if (mc->filaCaixa[i]->prioridadeAtual == IDOSO)
            temIdoso = 1;
        if (mc->filaCaixa[i]->prioridadeAtual == DEFICIENTE)
            temDeficiente = 1;
    }

    // Se houver as três prioridades ao mesmo tempo, há um deadlock e a única a ser chamada deve ser a escolhida pelo gerente
    if (temGravida && temIdoso && temDeficiente)
    {
        if (mc->escolhidaGerente != NULL)
            return mc->escolhidaGerente;

        // Retorna vazio, pois deve esperar a escolha do gerente
        return NULL;
    }

    // Inicialmente considera a primeira pessoa da fila como a próxima
    Pessoa *proximaPessoa = mc->filaCaixa[0];

    // Percorre toda a fila procurando a pessoa de maior prioridade
    for (int i = 1; i < mc->quantidadeDePessoasNaFila; i++)
    {
        Pessoa *candidato = mc->filaCaixa[i];

        // Se existir uma grávida e um deficiente, deve chamar o deficiente
        if (proximaPessoa->prioridadeAtual == GRAVIDA &&
            candidato->prioridadeAtual == DEFICIENTE)
        {
            proximaPessoa = candidato;
        }

        // Caso contrário, escolhe normalmente a de maior prioridade
        else if (candidato->prioridadeAtual < proximaPessoa->prioridadeAtual)
        {
            // Evita desfazer a regra acima
            if (!(candidato->prioridadeAtual == GRAVIDA &&
                  proximaPessoa->prioridadeAtual == DEFICIENTE))
            {
                proximaPessoa = candidato;
            }
        }
    }

    // Retorna a próxima pessoa a ser chamada
    return proximaPessoa;
}

// Função usada pelas threads das pessoas
void *threadFuncao(void *argumento)
{
    ThreadData *argumentos = (ThreadData *)argumento;
    Pessoa *pessoa = argumentos->pessoa;
    MonitorCaixa *mc = argumentos->mc;

    // Funcionamento de cada pessoa
    for (int i = 0; i < argumentos->numVezesCaixa; i++)
    {
        sleep((rand() % 3) + 3);

        esperar(pessoa, mc); // Pessoa entra pra fila e espera até ser chamada

        sleep(1);

        liberar(pessoa, mc); // Pessoa sai do caixa e libera a próxima
    }

    return NULL;
}

// Função usada pela thread do gerente
void *threadGerente(void *argumento)
{
    MonitorCaixa *mc = (MonitorCaixa *)argumento;

    // Monitor deve existir enquanto o programa está ativo
    while (1)
    {
        sleep(TEMPO);  // Tempo que deve ficar verificando
        verificar(mc); // Verifica se há deadlock
    }

    return NULL;
}

int main(int argc, char const *argv[])
{
    // Verifica se os parâmetros estão corretos
    if (argc != 2)
    {
        printf("Número de parâmetros inválido!\n");
        return 1;
    }

    int numVezesCaixa = atoi(argv[1]); // Pega o parâmetro de quantas vezes cada pessoa deve ir no ciaxa

    srand(time(NULL));

    // Inicializa o monitor do caixa
    MonitorCaixa monitorCaixa;
    monitorInit(&monitorCaixa);

    // Inicializa todas pessoas que devem existir no programa e suas respectivas prioridades
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

    pthread_t threadsPessoas[QTD_PRIORIDADES * 2]; // Vetor para armazenar os ids das threads das pessoas
    ThreadData argumentos[QTD_PRIORIDADES * 2];    // Vetor que armazena os dados que cada thread deve possuir

    for (int i = 0; i < QTD_PRIORIDADES * 2; i++)
    {
        argumentos[i].pessoa = &civil[i];            // Atribui a pessoa que a thread deve representar
        argumentos[i].pessoa->id = i;                // Id de criação para separar as threads
        argumentos[i].mc = &monitorCaixa;            // Atribuir o monitor caixa para a thread ter acesso
        argumentos[i].numVezesCaixa = numVezesCaixa; // Atribui o número de vezes que a thread deve existir (pessoa ir até a lotérica)

        pthread_create(&threadsPessoas[i], NULL, threadFuncao, &argumentos[i]); // Cria a thread
    }

    pthread_t idGerente;                                            // Id do gerente
    pthread_create(&idGerente, NULL, threadGerente, &monitorCaixa); // Cria thread do gerente

    // Espera todas as threas estarem prontas para finalizar o programa
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