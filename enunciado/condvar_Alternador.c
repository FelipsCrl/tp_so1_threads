// gcc -o alternador.bin -lpthread ./condvar_Alternador.c ; ./alternador.bin

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// Define a quantidade de threads
#define NUM_THREADS 5

// Variáveis globais
pthread_mutex_t mutex;
pthread_cond_t cond[NUM_THREADS];
int vez_de_quem = 0; // Variável para controlar a vez de cada thread

// Estrutura para passar dados para a thread
typedef struct {
    int id;
} ThreadData;

/**
 * @brief Função que será executada por cada thread.
 *
 * A thread espera pela sua vez (indicada pela variável 'vez_de_quem'). Quando for a sua vez,
 * ela imprime seu identificador, atualiza a variável 'vez_de_quem' para a próxima thread
 * e sinaliza para que a próxima thread possa executar.
 *
 * @param arg Um ponteiro para a estrutura ThreadData contendo o ID da thread.
 */
void *thread_function(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    int meu_id = data->id;

    while (1) {
        // Bloqueia o mutex para acessar a variável compartilhada 'vez_de_quem'
        pthread_mutex_lock(&mutex);

        // --- Início da Seção Crítica ---

		        // A thread espera enquanto não for a sua vez (vez_de_quem != meu_id)
		        while (vez_de_quem != meu_id) {
		            // A thread libera o mutex e espera pelo sinal na sua variável de condição
		            pthread_cond_wait(&cond[meu_id], &mutex);
		        }

		        // É a vez desta thread, então ela imprime seu ID
		        printf("%d ", meu_id + 1);
		        fflush(stdout); // Garante que a saída seja impressa imediatamente

                if(meu_id == NUM_THREADS-1) printf("\n"); // pro DEBUG ficar bonitinho

                // Adiciona um pequeno atraso para tornar a alternância mais visível (opcional)
                usleep(250000); // 250 ms
                // sleep(1); // 1 segundo

		        // Passa a vez para a próxima thread
		        vez_de_quem = (vez_de_quem + 1) % NUM_THREADS;

		        // Sinaliza para a próxima thread que agora é a vez dela
		        pthread_cond_signal(&cond[vez_de_quem]);

        // --- Fim da Seção Crítica ---

        // Libera o mutex
        pthread_mutex_unlock(&mutex);

        
    }

    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];

// INICIALIZANDO VARIÁVEIS
    // Inicializa o mutex comum a todos
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        perror("Erro ao inicializar o mutex");
        return 1;
    }
    // Inicializa as variáveis de condição
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_cond_init(&cond[i], NULL) != 0) {
            perror("Erro ao inicializar a variável de condição");
            return 1;
        }
    }

// EXECUTANDO AS THREADS E, DEPOIS, AGUARDANDO-AS
    vez_de_quem = 0;
    // Cria as threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].id = i; // Para fins de DEBUG, utiliza o próprio índice como id da Thread
        if (pthread_create(&threads[i], NULL, thread_function, &thread_data[i]) != 0) {
            perror("Erro ao criar a thread");
            return 1;
        }
    }

    // OBS.: como o loop contido na função das threads é infinito, o join nunca será alcançado.
    // Em uma aplicação real, você precisaria de uma condição de parada.
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }


// LIBERANDO ESPAÇO
    // Destroi o mutex e as variáveis de condição
    pthread_mutex_destroy(&mutex);
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_cond_destroy(&cond[i]);
    }

    return 0;
}