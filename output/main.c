#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "Pergunta.h"

int main() {
    Pergunta *perguntas = NULL;
    int numPerguntas = 0;

    srand(time(NULL));

    // Carrega as perguntas do projetobase.csv
    numPerguntas = carregarPerguntasDeCsv("projetobase.csv", &perguntas);

    if (numPerguntas > 0) {
        printf("\nIniciando o Show do Milhao!\n");
        printf("Temos %d perguntas carregadas no banco de dados.\n\n", numPerguntas);

        realizarQuiz(perguntas, numPerguntas);

    } else {
        printf("Nao foi possivel iniciar o quiz. Verifique o arquivo de perguntas.\n");
    }

    // Libera a memoria alocada para as perguntas
    liberarPerguntas(perguntas);

    return 0;
}
