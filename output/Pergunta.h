#ifndef PERGUNTA_H
#define PERGUNTA_H


#define MAX_PERGUNTAS 100
#define MAX_ENUNCIADO 500
#define MAX_ALTERNATIVA 200
#define MAX_RESPOSTA_USUARIO 2
#define MAX_CAMPO_CSV 256
#define MAX_LINHA_CSV (MAX_ENUNCIADO + 4 * MAX_ALTERNATIVA + 20)

#define NUM_PERGUNTAS_SHOW_MILHAO 15
#define NUM_NIVEIS_PREMIOS (NUM_PERGUNTAS_SHOW_MILHAO + 1)


extern const int PREMIOS_MILHAO[NUM_NIVEIS_PREMIOS];

// Estrutura que representa uma pergunta do quiz
typedef struct {
    char enunciado[MAX_ENUNCIADO];
    char alternativaA[MAX_ALTERNATIVA];
    char alternativaB[MAX_ALTERNATIVA];
    char alternativaC[MAX_ALTERNATIVA];
    char alternativaD[MAX_ALTERNATIVA];
    char correta;
    int nivel;
    int usada;    // Flag para indicar se a pergunta ja foi usada no quiz
} Pergunta;


int carregarPerguntasDeCsv(const char *nomeArquivo, Pergunta **perguntas);
void liberarPerguntas(Pergunta *perguntas);
void embaralharPerguntas(Pergunta *perguntas, int numPerguntas); // Nao sera usada para selecao de perguntas, mas pode ser util para outras coisas
void realizarQuiz(Pergunta *perguntas, int numPerguntas);

#endif 
