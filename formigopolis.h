#ifndef FORMIGOPOLIS_H
#define FORMIGOPOLIS_H

#define GRAVIDA 0
#define IDOSO 1
#define DEFICIENTE 2
#define COMUM 3
#define QTD_PRIORIDADES 4
#define TEMPO 5

typedef struct
{
    int id;
    char nome[20];
    int prioridadeAtual;
    int prioridadeOriginal;
    int vezesFuradas;
} Pessoa;

typedef struct
{
    Pessoa *filaCaixa[QTD_PRIORIDADES * 2];
    int quantidadeDePessoasNaFila;
    pthread_mutex_t mutex;
    int caixaOcupado;
    pthread_cond_t condincaoPorPessoa[QTD_PRIORIDADES * 2];
    Pessoa *escolhidaGerente;
} monitor_caixa;

typedef struct
{
    Pessoa *pessoa;
    monitor_caixa *mc;
    int numVezesCaixa;
} ThreadData;

void estaNaFilaCaixa(Pessoa *pessoa, monitor_caixa *mc);
void atendidoPeloCaixa(Pessoa *pessoa, monitor_caixa *mc);
void vaiEmboraParaCasa(Pessoa *pessoa, monitor_caixa *mc);
void removeDaFila(Pessoa *pessoa, monitor_caixa *mc);
Pessoa *proximaPessoaPrioridade(monitor_caixa *mc);
void esperar(Pessoa *pessoa, monitor_caixa *mc);
void liberar(Pessoa *pessoa, monitor_caixa *mc);
void esperar(Pessoa *pessoa, monitor_caixa *mc);
void envelhecerPessoas(Pessoa *pessoaChamada, monitor_caixa *mc);
void *threadFuncao(void *argumento);
void imprimeFila(monitor_caixa *mc);
void monitorInit(monitor_caixa *mc);
void *threadGerente(void *argumento);
void detectouInanicao(Pessoa *pessoa);
void detectouDeadlock(Pessoa *pessoa);

#endif
