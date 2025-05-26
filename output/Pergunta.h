#ifndef PERGUNTA_H
#define PERGUNTA_H

#define MAX_PERGUNTAS 100 // Aumentamos para ter mais espaço, se necessário
#define MAX_ENUNCIADO 500 // Aumentado
#define MAX_ALTERNATIVA 200 // Aumentado
#define MAX_RESPOSTA_USUARIO 2
#define MAX_CAMPO_CSV 256  // Aumentado, cada campo pode ser bem longo
#define MAX_LINHA_CSV (MAX_ENUNCIADO + 4 * MAX_ALTERNATIVA + 20) // Ajustado para ser maior que a soma dos campos
                                                                // + um pouco de folga para ponto e vírgulas e nível

typedef struct {
    char enunciado[MAX_ENUNCIADO];
    char alternativaA[MAX_ALTERNATIVA];
    char alternativaB[MAX_ALTERNATIVA];
    char alternativaC[MAX_ALTERNATIVA];
    char alternativaD[MAX_ALTERNATIVA];
    char correta;
    int nivel;
} Pergunta;

// Funções
int carregarPerguntasDeCsv(const char *nomeArquivo, Pergunta **perguntas);
void liberarPerguntas(Pergunta *perguntas);
void embaralharPerguntas(Pergunta *perguntas, int numPerguntas);
void realizarQuiz(Pergunta *perguntas, int numPerguntas);
int quiz(Pergunta *perguntas, int numPerguntas, int nivelDesejado, int numPerguntasDesejadas);

#endif // PERGUNTA_H
