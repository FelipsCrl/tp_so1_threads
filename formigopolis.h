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
    int id; //ID para thread das pessoas
    char nome[20]; //Nome para identicação da thread
    int prioridadeAtual; //Prioridade da pesssoa, ao longo do software
    int prioridadeOriginal; //Prioridade em que a thread começa
    int vezesFuradas; //Contagem de vezes em que a pessoa foi pulada
} Pessoa;

typedef struct
{
    Pessoa *filaCaixa[QTD_PRIORIDADES * 2]; //Fila do caixa
    int quantidadeDePessoasNaFila; //Quantidade de pessoas na fila do caixa
    pthread_mutex_t mutex; //Mutex do monitor
    int caixaOcupado; //1 para caixa ocupado e 0 para caixa liberado
    pthread_cond_t condincaoPorPessoa[QTD_PRIORIDADES * 2]; //Condição para acordar ou travar cada thread individualmente
    Pessoa *escolhidaGerente; //Pessoa escolhida pelo gerente em caso de deadlock
} MonitorCaixa;

typedef struct
{
    Pessoa *pessoa; //Qual pessoa a thread representa
    MonitorCaixa *mc; //Informações do monitor
    int numVezesCaixa; //Número de vezes que cada pessoa deve ir na lotérica
} ThreadData;

void estaNaFilaCaixa(Pessoa *pessoa, MonitorCaixa *mc);
void atendidoPeloCaixa(Pessoa *pessoa, MonitorCaixa *mc);
void vaiEmboraParaCasa(Pessoa *pessoa, MonitorCaixa *mc);
void removeDaFila(Pessoa *pessoa, MonitorCaixa *mc);
Pessoa *proximaPessoaPrioridade(MonitorCaixa *mc);
void esperar(Pessoa *pessoa, MonitorCaixa *mc);
void liberar(Pessoa *pessoa, MonitorCaixa *mc);
void esperar(Pessoa *pessoa, MonitorCaixa *mc);
void envelhecerPessoas(Pessoa *pessoaChamada, MonitorCaixa *mc);
void *threadFuncao(void *argumento);
void imprimeFila(MonitorCaixa *mc);
void monitorInit(MonitorCaixa *mc);
void *threadGerente(void *argumento);
void detectouInanicao(Pessoa *pessoa);
void detectouDeadlock(Pessoa *pessoa);

#endif
