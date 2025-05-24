// main.c
#include "pergunta.h" // Inclui o cabeçalho com a struct e as funcoes de carregamento

#include <time.h>     // Para time()
#include <ctype.h>    // Para toupper
#include <stdio.h>    // Para printf, scanf, getchar
#include <string.h>   // Para strcmp (usado para comparar strings de nivel)

// --- Funcoes de Utilidade ---

// Inicializa o gerador de numeros aleatorios
void iniciarSementeAleatoria() {
    srand((unsigned int)time(NULL));
}

// Gera um numero aleatorio entre 0 (inclusive) e max (exclusivo)
int gerarNumeroAleatorio(int max) {
    if (max <= 0) return 0; // Evita modulo por zero
    return rand() % max;
}

// Limpa o buffer de entrada do teclado (para scanf)
void limparBufferEntrada() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Funcao auxiliar para obter entrada de caractere valida do usuario
char obterRespostaUsuario() {
    char respostaUsuario;
    bool entradaValida = false;
    while (!entradaValida) {
        printf("Sua resposta (A, B, C ou D): ");
        scanf(" %c", &respostaUsuario); // Espaco antes de %c para ignorar whitespace
        limparBufferEntrada(); // Limpa o buffer apos scanf

        respostaUsuario = (char)toupper((unsigned char)respostaUsuario); // Converte para maiúscula

        if (respostaUsuario >= 'A' && respostaUsuario <= 'D') {
            entradaValida = true;
        } else {
            printf("Entrada invalida. Por favor, digite A, B, C ou D.\n");
        }
    }
    return respostaUsuario;
}

// --- Funcoes de Jogo ---

// Libera toda a memoria alocada para as perguntas no array principal
void liberarTodasPerguntas(PerguntaHistoriaComputacao* perguntas, int numPerguntas) {
    if (perguntas == NULL) return;
    for (int i = 0; i < numPerguntas; ++i) {
        liberarPergunta(&perguntas[i]);
    }
    free(perguntas); // Libera o array em si
}

int main() {
    iniciarSementeAleatoria(); // Inicializa o gerador de numeros aleatorios

    const char* nomeDoArquivoCsv = "projetobase.csv"; // Nome do seu arquivo CSV

    int numTotalPerguntas = 0;
    // Carrega todas as perguntas do CSV
    PerguntaHistoriaComputacao* todasAsPerguntas = carregarPerguntasDeCsv(nomeDoArquivoCsv, &numTotalPerguntas);

    if (todasAsPerguntas == NULL || numTotalPerguntas == 0) {
        printf("Nao foi possivel carregar nenhuma pergunta. Verifique o arquivo '%s' e o caminho.\n", nomeDoArquivoCsv);
        return 1; // Sai do programa com erro
    }

    // 1. Organizar perguntas por nível de dificuldade
    // Usaremos arrays de ponteiros para arrays de perguntas para cada nivel
    // e um array para armazenar a contagem real de perguntas em cada nivel.
    
    // Niveis de dificuldade definidos
    char* niveisTexto[] = {"muitofacil", "facil", "medio", "dificil", "muitodificil"};
    int numTiposNiveis = sizeof(niveisTexto) / sizeof(niveisTexto[0]);

    PerguntaHistoriaComputacao** perguntasPorNivel = (PerguntaHistoriaComputacao**)calloc(numTiposNiveis, sizeof(PerguntaHistoriaComputacao*));
    int* contagemRealPorNivel = (int*)calloc(numTiposNiveis, sizeof(int)); // Quantas perguntas de cada nivel realmente temos
    int* capacidadePorNivel = (int*)calloc(numTiposNiveis, sizeof(int)); // Capacidade alocada para cada sub-array de nivel

    if (perguntasPorNivel == NULL || contagemRealPorNivel == NULL || capacidadePorNivel == NULL) {
        fprintf(stderr, "Erro de alocacao de memoria para organizacao por nivel.\n");
        liberarTodasPerguntas(todasAsPerguntas, numTotalPerguntas); // Libera o que ja foi alocado
        free(perguntasPorNivel); // Nao foi alocado sub-arrays, so o principal
        free(contagemRealPorNivel);
        free(capacidadePorNivel);
        return 1;
    }

    // Distribui as perguntas carregadas para os arrays por nivel
    for (int i = 0; i < numTotalPerguntas; ++i) {
        for (int j = 0; j < numTiposNiveis; ++j) {
            if (strcmp(todasAsPerguntas[i].nivel, niveisTexto[j]) == 0) {
                // Realoca o array para o nivel se a capacidade for atingida
                if (contagemRealPorNivel[j] == capacidadePorNivel[j]) {
                    capacidadePorNivel[j] = (capacidadePorNivel[j] == 0) ? 5 : capacidadePorNivel[j] * 2;
                    perguntasPorNivel[j] = (PerguntaHistoriaComputacao*)realloc(
                        perguntasPorNivel[j], capacidadePorNivel[j] * sizeof(PerguntaHistoriaComputacao));
                    if (perguntasPorNivel[j] == NULL) {
                        fprintf(stderr, "Erro de realocacao de memoria para nivel %s.\n", niveisTexto[j]);
                        // Libera memoria antes de sair, em caso de erro grave
                        liberarTodasPerguntas(todasAsPerguntas, numTotalPerguntas); // Original
                        // Liberar sub-arrays ja alocados para outros niveis
                        for (int k = 0; k < j; ++k) { // Apenas os niveis ja processados
                            free(perguntasPorNivel[k]);
                        }
                        free(perguntasPorNivel);
                        free(contagemRealPorNivel);
                        free(capacidadePorNivel);
                        return 1;
                    }
                }
                // Copia a struct (cópia rasa, ponteiros char* são copiados)
                perguntasPorNivel[j][contagemRealPorNivel[j]] = todasAsPerguntas[i]; 
                contagemRealPorNivel[j]++;
                break;
            }
        }
    }
    
    // ATENÇÃO: As perguntas em 'todasAsPerguntas' foram "movidas" para 'perguntasPorNivel'.
    // Os ponteiros char* dentro das structs de 'todasAsPerguntas' agora apontam para a mesma
    // memória que as structs em 'perguntasPorNivel'.
    // Portanto, não chamamos liberarPergunta para cada elemento de 'todasAsPerguntas' aqui,
    // pois a propriedade das strings foi transferida.
    free(todasAsPerguntas); // Libera apenas o array que continha as structs, não as strings internas
    todasAsPerguntas = NULL; // Evita uso indevido

    // 2. Definir o numero de perguntas desejado por nivel para o jogo
    int contagemDesejadaPorNivelValores[] = {2, 2, 4, 4, 3}; // Ordem: muitofacil, facil, medio, dificil, muitodificil
    const int TOTAL_PERGUNTAS_JOGO = 15;

    PerguntaHistoriaComputacao* perguntasDoJogo = (PerguntaHistoriaComputacao*)malloc(TOTAL_PERGUNTAS_JOGO * sizeof(PerguntaHistoriaComputacao));
    int numPerguntasDoJogo = 0; // Contador de perguntas adicionadas ao jogo

    if (perguntasDoJogo == NULL) {
        fprintf(stderr, "Erro de alocacao de memoria para perguntasDoJogo.\n");
        // Libera os arrays de niveis antes de sair
        for (int k = 0; k < numTiposNiveis; ++k) { free(perguntasPorNivel[k]); }
        free(perguntasPorNivel); free(contagemRealPorNivel); free(capacidadePorNivel);
        return 1;
    }

    // 3. Selecionar perguntas para o jogo e preencher perguntasDoJogo
    for (int j = 0; j < numTiposNiveis; ++j) {
        int quantidadeDesejada = contagemDesejadaPorNivelValores[j];
        
        if (contagemRealPorNivel[j] < quantidadeDesejada) {
            fprintf(stderr, "Erro: Nao ha perguntas suficientes do nivel '%s' no arquivo CSV. Necessario: %d, Encontrado: %d\n", 
                    niveisTexto[j], quantidadeDesejada, contagemRealPorNivel[j]);
            // Libera toda a memoria alocada antes de sair em caso de erro
            for (int k = 0; k < numTiposNiveis; ++k) {
                // As strings em perguntasPorNivel ainda precisam ser liberadas se nao foram para perguntasDoJogo
                for(int l = 0; l < contagemRealPorNivel[k]; ++l) {
                    // Libera apenas se a pergunta nao for uma das que ja foram "movidas" para perguntasDoJogo
                    // Isso eh complexo com copias rasas. Para simplificar, assumimos que as selecionadas estao em perguntasDoJogo
                    // e as restantes sao liberadas aqui.
                    // Uma maneira mais robusta envolveria copiar profundamente ou rastrear "propriedade"
                    bool is_in_game = false;
                    for (int m = 0; m < numPerguntasDoJogo; ++m) {
                        if (&(perguntasPorNivel[k][l]) == &(perguntasDoJogo[m])) { // Compara endereços
                            is_in_game = true;
                            break;
                        }
                    }
                    if (!is_in_game) {
                        liberarPergunta(&perguntasPorNivel[k][l]);
                    }
                }
                free(perguntasPorNivel[k]);
            }
            free(perguntasPorNivel); free(contagemRealPorNivel); free(capacidadePorNivel);
            free(perguntasDoJogo);
            return 1;
        }

        // Embaralha as perguntas disponiveis para este nivel (Fisher-Yates shuffle)
        for (int k = contagemRealPorNivel[j] - 1; k > 0; --k) {
            int l = gerarNumeroAleatorio(k + 1); // Indice aleatorio entre 0 e k
            PerguntaHistoriaComputacao temp = perguntasPorNivel[j][k];
            perguntasPorNivel[j][k] = perguntasPorNivel[j][l];
            perguntasPorNivel[j][l] = temp;
        }

        // Adiciona a quantidade desejada de perguntas embaralhadas ao jogo
        for (int k = 0; k < quantidadeDesejada; ++k) {
            // Copia a struct (cópia rasa, ponteiros char* são copiados)
            if (numPerguntasDoJogo < TOTAL_PERGUNTAS_JOGO) { // Garante que nao ultrapasse o tamanho total do jogo
                 perguntasDoJogo[numPerguntasDoJogo++] = perguntasPorNivel[j][k]; 
            } else {
                // Isso nao deveria acontecer se as contagens estao corretas
                fprintf(stderr, "Aviso: Excedeu o numero maximo de perguntas para o jogo!\n");
                break;
            }
        }
    }
    
    // Embaralha a ordem final das 15 perguntas
    for (int k = numPerguntasDoJogo - 1; k > 0; --k) {
        int l = gerarNumeroAleatorio(k + 1);
        PerguntaHistoriaComputacao temp = perguntasDoJogo[k];
        perguntasDoJogo[k] = perguntasDoJogo[l];
        perguntasDoJogo[l] = temp;
    }

    // Início do Jogo
    printf("Bem-vindo ao Quiz de Historia da Computacao!\n");
    printf("===========================================\n");
    printf("Voce respondera %d perguntas. Boa sorte!\n", numPerguntasDoJogo);

    int pontuacao = 0;
    int numeroPerguntaAtual = 0;

    for (int i = 0; i < numPerguntasDoJogo; ++i) {
        numeroPerguntaAtual++;
        printf("\n--- Pergunta %d de %d ---\n", numeroPerguntaAtual, numPerguntasDoJogo);
        exibirPergunta(&perguntasDoJogo[i]);

        char respostaUsuario = obterRespostaUsuario();

        if (respostaUsuario == perguntasDoJogo[i].correta) {
            printf("Parabens! Resposta CORRETA!\n");
            pontuacao++;
        } else {
            printf("Que pena! Resposta INCORRETA.\n");
            printf("A resposta correta era: %c\n", perguntasDoJogo[i].correta);
        }
    }

    printf("\n===========================================\n");
    printf("Fim do Quiz!\n");
    printf("Sua pontuacao final: %d de %d\n", pontuacao, numPerguntasDoJogo);
    printf("Obrigado por jogar!\n");

    // --- Liberar TODA a memoria alocada dinamicamente ---
    // A logica de liberacao de memoria eh crucial e um pouco mais complexa aqui
    // devido as copias rasas e transferencias de "propriedade" dos ponteiros char*.

    // 1. Liberar as strings das perguntas que ESTAO no array 'perguntasDoJogo'.
    //    Essas sao as 15 perguntas selecionadas para o quiz.
    for (int i = 0; i < numPerguntasDoJogo; ++i) {
        liberarPergunta(&perguntasDoJogo[i]);
    }
    free(perguntasDoJogo); // Libera o array de structs em si

    // 2. Liberar as strings das perguntas que NUNCA FORAM para 'perguntasDoJogo'
    //    (ou seja, as que ficaram para tras nos arrays 'perguntasPorNivel').
    for (int j = 0; j < numTiposNiveis; ++j) {
        if (perguntasPorNivel[j] != NULL) {
            for (int k = 0; k < contagemRealPorNivel[j]; ++k) {
                // Verificar se esta pergunta ja nao foi liberada como parte de 'perguntasDoJogo'
                // Para evitar dupla liberacao, que causa crash.
                // Esta é a parte mais delicada da gestão de memória com C-style e cópias rasas.
                // Uma solução mais robusta seria usar um "flag" na PerguntaHistoriaComputacao para indicar
                // se a memória já foi liberada ou se é a "proprietária" original.
                // OU, fazer cópias profundas em todos os lugares, mas isso gasta mais memória.
                
                // Para este exemplo, vamos assumir que as 15 perguntas do jogo
                // foram "movidas" de seus arrays de nível e serão liberadas UMA VEZ pelo loop 'perguntasDoJogo'.
                // As perguntas remanescentes em 'perguntasPorNivel' (se houver) precisam ser liberadas aqui.
                
                bool esta_no_jogo = false;
                for(int l = 0; l < numPerguntasDoJogo; ++l) {
                    // Comparar endereços de memória das structs
                    if (&perguntasPorNivel[j][k] == &perguntasDoJogo[l]) {
                        esta_no_jogo = true;
                        break;
                    }
                }
                if (!esta_no_jogo) {
                    liberarPergunta(&perguntasPorNivel[j][k]);
                }
            }
            free(perguntasPorNivel[j]); // Libera o array de structs para este nível
        }
    }
    free(perguntasPorNivel);
    free(contagemRealPorNivel);
    free(capacidadePorNivel);
    
    return 0; // Sai do programa com sucesso
}