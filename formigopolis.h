#ifndef FORMIGOPOLIS_H
#define FORMIGOPOLIS_H

#define GRAVIDA 0
#define IDOSO 1
#define DEFICIENTE 2
#define COMUM 3
#define QTD_PRIORIDADES 4

typedef struct
{
    char nome[20];
    int prioridadeAtual;
    int prioridadeOriginal;
    int vezesFuradas;
} Pessoa;

typedef struct
{
    Pessoa *pessoa;
    monitor_caixa *mc;
    int numVezesCaixa;
} ThreadData;

typedef struct
{
    int filaCaixa[QTD_PRIORIDADES];
    char filaLetrasNome[(QTD_PRIORIDADES * 2) + 1];
    pthread_mutex_t mutex;
    int caixaOcupado;
    pthread_cond_t condincaoPorPrioridade[QTD_PRIORIDADES];
} monitor_caixa;

void atendidoPeloCaixa(Pessoa *pessoa, monitor_caixa *mc);
void vaiEmboraParaCasa(Pessoa *pessoa, monitor_caixa *mc);
void adicionar_letra_fila(char inicial, monitor_caixa *mc);
void remover_letra_fila(char inicial, monitor_caixa *mc);
void verificar(Pessoa *pessoa, monitor_caixa *mc); // gerente verifica se ha deadlock
void esperar(Pessoa *pessoa, monitor_caixa *mc);
void liberar(Pessoa *pessoa, monitor_caixa *mc);
void esperar(Pessoa *pessoa, monitor_caixa *mc);
int proximaPrioridade(monitor_caixa *mc);
void *threadFuncao(void *argumento);
void imprimeFila(monitor_caixa *mc);
void monitor_init(monitor_caixa *mc);

#endif
