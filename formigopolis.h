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
    int filaCaixa[QTD_PRIORIDADES];
    pthread_mutex_t mutex;
    int caixaOcupado;
    pthread_cond_t condincaoPorPrioridade[QTD_PRIORIDADES];
} monitor_caixa;

void esperar(Pessoa *pessoa, monitor_caixa *mc);
void liberar(Pessoa *pessoa, monitor_caixa *mc);
void verificar(Pessoa *pessoa, monitor_caixa *mc); // gerente verifica se ha deadlock
void atendidoPeloCaixa(Pessoa *pessoa);
void vaiEmboraParaCasa(Pessoa *pessoa);

#endif
