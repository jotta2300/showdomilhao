#ifndef PERGUNTA_H
#define PERGUNTA_H

// Definicoes de tamanho para buffers e arrays
#define MAX_PERGUNTAS 100 // Numero maximo de perguntas que podem ser carregadas
#define MAX_ENUNCIADO 500 // Tamanho maximo para o enunciado de uma pergunta
#define MAX_ALTERNATIVA 200 // Tamanho maximo para cada alternativa de resposta
#define MAX_RESPOSTA_USUARIO 2 // Tamanho maximo para a resposta do usuario (A, B, C, D ou E + \0)
#define MAX_CAMPO_CSV 256  // Tamanho maximo esperado para um campo individual no CSV
#define MAX_LINHA_CSV (MAX_ENUNCIADO + 4 * MAX_ALTERNATIVA + 20) // Tamanho maximo para uma linha completa do CSV

#define NUM_PERGUNTAS_SHOW_MILHAO 15 // O jogo tera 15 perguntas
#define NUM_NIVEIS_PREMIOS (NUM_PERGUNTAS_SHOW_MILHAO + 1) // Do 0 ao 15 (16 etapas de premios)

// Declara o array de premios (sera definido em Pergunta.c)
extern const int PREMIOS_MILHAO[NUM_NIVEIS_PREMIOS];

// Estrutura que representa uma pergunta do quiz
typedef struct {
    char enunciado[MAX_ENUNCIADO];
    char alternativaA[MAX_ALTERNATIVA];
    char alternativaB[MAX_ALTERNATIVA];
    char alternativaC[MAX_ALTERNATIVA];
    char alternativaD[MAX_ALTERNATIVA];
    char correta; // 'A', 'B', 'C', 'D'
    int nivel;    // Nivel de dificuldade da pergunta (1, 2, 3, 4, 5)
    int usada;    // Flag para indicar se a pergunta ja foi usada no quiz
} Pergunta;

// Prototipos das funcoes
int carregarPerguntasDeCsv(const char *nomeArquivo, Pergunta **perguntas);
void liberarPerguntas(Pergunta *perguntas);
void embaralharPerguntas(Pergunta *perguntas, int numPerguntas); // Nao sera usada para selecao de perguntas, mas pode ser util para outras coisas
void realizarQuiz(Pergunta *perguntas, int numPerguntas);

#endif // PERGUNTA_H
