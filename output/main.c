#include <stdio.h>
#include <stdlib.h> // Para usar srand, rand
#include <time.h>   // Para usar time para a semente de srand
#include "Pergunta.h" // Inclui o cabeçalho das funções de pergunta

int main() {
    Pergunta *perguntas = NULL; // Ponteiro para o array de perguntas
    int numPerguntas = 0;

    // Inicializa o gerador de numeros aleatorios com base no tempo atual
    srand(time(NULL));

    // Carrega as perguntas do arquivo CSV
    numPerguntas = carregarPerguntasDeCsv("projetobase.csv", &perguntas);

    if (numPerguntas > 0) {
        printf("\nIniciando o Show do Milhao!\n");
        printf("Temos %d perguntas carregadas no banco de dados.\n\n", numPerguntas);

        // Realiza o quiz no estilo Show do Milhao
        realizarQuiz(perguntas, numPerguntas);

    } else {
        printf("Nao foi possivel iniciar o quiz. Verifique o arquivo de perguntas e se ha perguntas validas.\n");
    }

    // Libera a memoria alocada para as perguntas
    liberarPerguntas(perguntas);

    return 0;
}
