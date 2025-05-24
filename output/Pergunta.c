// pergunta.c
#include "pergunta.h" // Inclui o cabeçalho que declara as estruturas e funções

// Funcoes de Gerenciamento de Memoria e Strings

// Aloca memoria para uma string e copia o conteudo
char* copiarStringDinamica(const char* origem) {
    if (origem == NULL) {
        return NULL;
    }
    size_t tamanho = strlen(origem) + 1; // +1 para o terminador nulo '\0'
    char* destino = (char*)malloc(tamanho * sizeof(char));
    if (destino == NULL) {
        fprintf(stderr, "Erro de alocacao de memoria para string.\n");
        exit(EXIT_FAILURE); // Sai do programa em caso de erro grave
    }
    strcpy(destino, origem);
    return destino;
}

// Inicializa uma struct PerguntaHistoriaComputacao com memoria alocada para strings
void inicializarPergunta(PerguntaHistoriaComputacao* pergunta, 
                         const char* enunciado, const char* altA, const char* altB,
                         const char* altC, const char* altD, char correta, const char* nivel) {
    if (pergunta == NULL) return;

    pergunta->enunciado = copiarStringDinamica(enunciado);
    pergunta->alternativaA = copiarStringDinamica(altA);
    pergunta->alternativaB = copiarStringDinamica(altB);
    pergunta->alternativaC = copiarStringDinamica(altC);
    pergunta->alternativaD = copiarStringDinamica(altD);
    pergunta->correta = correta;
    pergunta->nivel = copiarStringDinamica(nivel);
}

// Libera a memoria alocada para as strings de uma PerguntaHistoriaComputacao
void liberarPergunta(PerguntaHistoriaComputacao* pergunta) {
    if (pergunta == NULL) return;

    free(pergunta->enunciado);
    free(pergunta->alternativaA);
    free(pergunta->alternativaB);
    free(pergunta->alternativaC);
    free(pergunta->alternativaD);
    free(pergunta->nivel);

    // O ponteiro 'pergunta' em si nao e liberado aqui se ele veio de um array.
    // Apenas os membros alocados dinamicamente DENTRO da struct.
}

// Exibe uma pergunta no console
void exibirPergunta(const PerguntaHistoriaComputacao* pergunta) {
    if (pergunta == NULL) return;
    printf("\nNivel: %s - %s\n", pergunta->nivel, pergunta->enunciado);
    printf("A) %s\n", pergunta->alternativaA);
    printf("B) %s\n", pergunta->alternativaB);
    printf("C) %s\n", pergunta->alternativaC);
    printf("D) %s\n", pergunta->alternativaD);
}

// Carrega as perguntas de um arquivo CSV
PerguntaHistoriaComputacao* carregarPerguntasDeCsv(const char* nomeArquivo, int* numPerguntas) {
    FILE* arquivo = fopen(nomeArquivo, "r");
    if (arquivo == NULL) {
        fprintf(stderr, "Erro: Nao foi possivel abrir o arquivo %s\n", nomeArquivo);
        *numPerguntas = 0;
        return NULL;
    }

    char linha[MAX_CSV_LINE_LEN];

    // Le a primeira linha (cabecalho) e ignora
    if (fgets(linha, sizeof(linha), arquivo) == NULL) {
        fprintf(stderr, "Erro: Arquivo CSV vazio ou erro de leitura do cabecalho.\n");
        fclose(arquivo);
        *numPerguntas = 0;
        return NULL;
    }

    // Primeira passagem para contar o numero de perguntas validas
    int contador_perguntas_validas = 0;
    long current_pos = ftell(arquivo); // Salva a posicao atual apos o cabecalho

    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        // Remove '\n' ou '\r\n' do final da linha para analise
        linha[strcspn(linha, "\n\r")] = 0; 
        
        // Ignora linhas que ficam vazias apos remover \n\r, ou sao muito curtas para serem validas
        if (strlen(linha) < 10) continue; // Um palpite de linha muito curta para ser pergunta valida

        // Tenta parsear para verificar se tem o numero correto de campos
        char s_enunciado[MAX_CSV_FIELD_LEN];
        char s_altA[MAX_CSV_FIELD_LEN];
        char s_altB[MAX_CSV_FIELD_LEN];
        char s_altC[MAX_CSV_FIELD_LEN];
        char s_altD[MAX_CSV_FIELD_LEN];
        char s_correta[2]; 
        char s_nivel[32];

        // %511[^;] limita a leitura para MAX_CSV_FIELD_LEN-1 caracteres (para o \0)
        int num_parsed = sscanf(linha, "%511[^;];%511[^;];%511[^;];%511[^;];%511[^;];%1c;%31[^\n\r]",
                                s_enunciado, s_altA, s_altB, s_altC, s_altD,
                                s_correta, s_nivel);

        if (num_parsed == 7) {
            contador_perguntas_validas++;
        } else {
            fprintf(stderr, "Aviso: Linha invalida ignorada durante contagem (campos incorretos): %s\n", linha);
        }
    }
    *numPerguntas = contador_perguntas_validas;

    if (*numPerguntas == 0) {
        fprintf(stderr, "Nenhuma pergunta valida encontrada no arquivo CSV.\n");
        fclose(arquivo);
        return NULL;
    }

    // Aloca memoria para o array de perguntas
    PerguntaHistoriaComputacao* perguntas_array = (PerguntaHistoriaComputacao*)malloc(*numPerguntas * sizeof(PerguntaHistoriaComputacao));
    if (perguntas_array == NULL) {
        fprintf(stderr, "Erro de alocacao de memoria para o array de perguntas.\n");
        fclose(arquivo);
        *numPerguntas = 0;
        return NULL;
    }

    // Volta o ponteiro do arquivo para a posicao apos o cabecalho
    fseek(arquivo, current_pos, SEEK_SET);

    // Segunda passagem para ler e popular as structs
    int i = 0;
    while (fgets(linha, sizeof(linha), arquivo) != NULL && i < *numPerguntas) {
        linha[strcspn(linha, "\n\r")] = 0; 
        if (strlen(linha) < 10) continue; // Ignora linhas curtas que nao serao validas

        char s_enunciado[MAX_CSV_FIELD_LEN];
        char s_altA[MAX_CSV_FIELD_LEN];
        char s_altB[MAX_CSV_FIELD_LEN];
        char s_altC[MAX_CSV_FIELD_LEN];
        char s_altD[MAX_CSV_FIELD_LEN];
        char s_correta[2];
        char s_nivel[32];

        int num_parsed = sscanf(linha, "%511[^;];%511[^;];%511[^;];%511[^;];%511[^;];%1c;%31[^\n\r]",
                                s_enunciado, s_altA, s_altB, s_altC, s_altD,
                                s_correta, s_nivel);

        if (num_parsed == 7) {
            inicializarPergunta(&perguntas_array[i], s_enunciado, s_altA, s_altB,
                                s_altC, s_altD, s_correta[0], s_nivel);
            i++;
        } else {
            // Este caso ja foi avisado na primeira passagem, mas pode ocorrer se o arquivo for modificado entre passagens
            // ou se a linha nao for detectada como vazia na primeira passagem.
        }
    }

    fclose(arquivo);
    return perguntas_array;
}