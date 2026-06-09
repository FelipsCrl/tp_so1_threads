#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        printf("Número de parâmetros inválido!\n");
        return 1;
    }

    int num_vezes_caixa = atof(argv[1]);
    
    return 0;
}
