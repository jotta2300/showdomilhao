// pergunta.h
#ifndef PERGUNTA_H
#define PERGUNTA_H

#include <stdio.h>   // Para FILE, NULL, fprintf
#include <stdlib.h>  // Para malloc, free, exit, size_t
#include <string.h>  // Para strcpy, strlen, strcmp, strcspn
#include <stdbool.h> // Para o tipo bool (true/false)

// Define um tamanho máximo para uma linha do CSV
#define MAX_CSV_LINE_LEN 1024
// Define um tamanho máximo para um campo individual do CSV
#define MAX_CSV_FIELD_LEN 512

// 1. Definição da Estrutura PerguntaHistoriaComputacao
// Usando char* para strings, que serao alocadas dinamicamente.
typedef struct {
    char* enunciado;
    char* alternativaA;
    char* alternativaB;
    char* alternativaC;
    char* alternativaD;
    char correta;
    char* nivel;
} PerguntaHistoriaComputacao;

// 2. Declarações de Funções de Gerenciamento de Perguntas

// Aloca memoria para uma string e copia o conteudo
char* copiarStringDinamica(const char* origem);

// Inicializa uma struct PerguntaHistoriaComputacao com memoria alocada para strings
void inicializarPergunta(PerguntaHistoriaComputacao* pergunta, 
                         const char* enunciado, const char* altA, const char* altB,
                         const char* altC, const char* altD, char correta, const char* nivel);

// Libera a memoria alocada para as strings de uma PerguntaHistoriaComputacao
void liberarPergunta(PerguntaHistoriaComputacao* pergunta);

// Exibe uma pergunta no console
void exibirPergunta(const PerguntaHistoriaComputacao* pergunta);

// Carrega as perguntas de um arquivo CSV
// Retorna um ponteiro para um array de PerguntaHistoriaComputacao
// e o numero total de perguntas carregadas via ponteiro 'numPerguntas'
PerguntaHistoriaComputacao* carregarPerguntasDeCsv(const char* nomeArquivo, int* numPerguntas);

#endif // PERGUNTA_H