#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> // Para toupper
#include <time.h> // Para rand e srand
#include "Pergunta.h"

// Definicao do array de premios do "Show do Milhao"
// Os indices correspondem aos premios APOS responder corretamente a pergunta 'i'.
// Premio[0] = premio antes da P1, Premio[1] = premio apos P1, etc.
const int PREMIOS_MILHAO[NUM_NIVEIS_PREMIOS] = {
    0,      // Premio inicial (antes da P1)
    1000,   // Pergunta 1: 1.000
    2000,   // Pergunta 2: 2.000
    3000,   // Pergunta 3: 3.000
    4000,   // Pergunta 4: 4.000
    5000,   // Pergunta 5 (Marco de Seguranca 1): 5.000
    10000,  // Pergunta 6: 10.000
    20000,  // Pergunta 7: 20.000
    30000,  // Pergunta 8: 30.000
    40000,  // Pergunta 9: 40.000
    50000,  // Pergunta 10 (Marco de Seguranca 2): 50.000
    100000, // Pergunta 11: 100.000
    200000, // Pergunta 12: 200.000
    300000, // Pergunta 13: 300.000
    400000, // Pergunta 14: 400.000
    1000000 // Pergunta 15 (O Milhao): 1.000.000
};

// Funcao auxiliar para remover quebra de linha de uma string
void removerQuebraLinha(char *str) {
    if (str == NULL) return;
    size_t len = strlen(str);
    if (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[len - 1] = '\0';
    }
    if (len > 1 && (str[len - 2] == '\n' || str[len - 2] == '\r')) {
        str[len - 2] = '\0';
    }
}

// Funcao para carregar perguntas de um arquivo CSV
int carregarPerguntasDeCsv(const char *nomeArquivo, Pergunta **perguntas) {
    FILE *arquivo = fopen(nomeArquivo, "r");
    char linha[MAX_LINHA_CSV];
    int numPerguntas = 0;
    int i = 0;

    if (arquivo == NULL) {
        printf("Erro: Nao foi possivel abrir o arquivo '%s'. Verifique o caminho e as permissoes.\n", nomeArquivo);
        fflush(stdout);
        *perguntas = NULL;
        return 0;
    }

    printf("DEBUG: Arquivo '%s' aberto com sucesso.\n", nomeArquivo);
    fflush(stdout);

    // Pular o cabecalho
    if (fgets(linha, sizeof(linha), arquivo) == NULL) {
        printf("Erro: Arquivo CSV vazio ou erro de leitura do cabecalho.\n");
        fflush(stdout);
        fclose(arquivo);
        *perguntas = NULL;
        return 0;
    }
    removerQuebraLinha(linha);
    printf("DEBUG: Cabecalho do CSV lido e ignorado: '%s'\n", linha);
    fflush(stdout);

    // Primeira passagem: Contar o numero de perguntas validas
    int contador_linhas_validas = 0;
    long pos_atual = ftell(arquivo);

    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        removerQuebraLinha(linha);
        if (strlen(linha) == 0) {
            continue;
        }

        char temp_linha[MAX_LINHA_CSV];
        strncpy(temp_linha, linha, MAX_LINHA_CSV - 1);
        temp_linha[MAX_LINHA_CSV - 1] = '\0';

        char *token;
        int campos_contados = 0;
        // Usa uma copia da linha para strtok para nao destruir a original
        char *linha_copia = strdup(temp_linha);
        if (linha_copia == NULL) {
            printf("Erro: Falha ao alocar memoria para copia da linha.\n");
            fflush(stdout);
            continue;
        }
        token = strtok(linha_copia, ";");
        while (token != NULL) {
            campos_contados++;
            token = strtok(NULL, ";");
        }
        free(linha_copia); // Libera a memoria alocada por strdup

        if (campos_contados == 7) {
            contador_linhas_validas++;
        } else {
            printf("AVISO: Linha invalida ignorada durante contagem (campos incorretos: %d esperados 7): '%s'\n", campos_contados, linha);
            fflush(stdout);
        }
    }
    printf("DEBUG: Primeira passagem concluida. Total de linhas validas contadas: %d\n", contador_linhas_validas);
    fflush(stdout);

    if (contador_linhas_validas == 0) {
        printf("Erro: Nao foi possivel carregar nenhuma pergunta valida do arquivo CSV.\n");
        fflush(stdout);
        fclose(arquivo);
        *perguntas = NULL;
        return 0;
    }

    *perguntas = (Pergunta *)malloc(contador_linhas_validas * sizeof(Pergunta));
    if (*perguntas == NULL) {
        printf("Erro: Falha ao alocar memoria para as perguntas.\n");
        fflush(stdout);
        fclose(arquivo);
        return 0;
    }
    numPerguntas = contador_linhas_validas;

    printf("DEBUG: Memoria alocada para %d perguntas.\n", numPerguntas);
    fflush(stdout);

    fseek(arquivo, pos_atual, SEEK_SET);

    i = 0;
    while (fgets(linha, sizeof(linha), arquivo) != NULL && i < numPerguntas) {
        removerQuebraLinha(linha);
        if (strlen(linha) == 0) {
            continue;
        }
        
        char *token;
        char temp_linha_parse[MAX_LINHA_CSV];
        strncpy(temp_linha_parse, linha, MAX_LINHA_CSV - 1);
        temp_linha_parse[MAX_LINHA_CSV - 1] = '\0';

        // Usa uma copia da linha para strtok para nao destruir a original
        char *linha_para_parsear = strdup(temp_linha_parse);
        if (linha_para_parsear == NULL) {
            printf("Erro: Falha ao alocar memoria para parse da linha.\n");
            fflush(stdout);
            continue;
        }

        token = strtok(linha_para_parsear, ";");
        if (token != NULL) {
            strncpy((*perguntas)[i].enunciado, token, MAX_ENUNCIADO - 1);
            (*perguntas)[i].enunciado[MAX_ENUNCIADO - 1] = '\0';
        } else { printf("AVISO: Erro ao parsear enunciado na linha: '%s'\n", linha); free(linha_para_parsear); fflush(stdout); continue; }

        token = strtok(NULL, ";");
        if (token != NULL) {
            strncpy((*perguntas)[i].alternativaA, token, MAX_ALTERNATIVA - 1);
            (*perguntas)[i].alternativaA[MAX_ALTERNATIVA - 1] = '\0';
        } else { printf("AVISO: Erro ao parsear alternativaA na linha: '%s'\n", linha); free(linha_para_parsear); fflush(stdout); continue; }

        token = strtok(NULL, ";");
        if (token != NULL) {
            strncpy((*perguntas)[i].alternativaB, token, MAX_ALTERNATIVA - 1);
            (*perguntas)[i].alternativaB[MAX_ALTERNATIVA - 1] = '\0';
        } else { printf("AVISO: Erro ao parsear alternativaB na linha: '%s'\n", linha); free(linha_para_parsear); fflush(stdout); continue; }

        token = strtok(NULL, ";");
        if (token != NULL) {
            strncpy((*perguntas)[i].alternativaC, token, MAX_ALTERNATIVA - 1);
            (*perguntas)[i].alternativaC[MAX_ALTERNATIVA - 1] = '\0';
        } else { printf("AVISO: Erro ao parsear alternativaC na linha: '%s'\n", linha); free(linha_para_parsear); fflush(stdout); continue; }

        token = strtok(NULL, ";");
        if (token != NULL) {
            strncpy((*perguntas)[i].alternativaD, token, MAX_ALTERNATIVA - 1);
            (*perguntas)[i].alternativaD[MAX_ALTERNATIVA - 1] = '\0';
        } else { printf("AVISO: Erro ao parsear alternativaD na linha: '%s'\n", linha); free(linha_para_parsear); fflush(stdout); continue; }

        token = strtok(NULL, ";");
        if (token != NULL) {
            (*perguntas)[i].correta = toupper(token[0]);
        } else { printf("AVISO: Erro ao parsear resposta correta na linha: '%s'\n", linha); free(linha_para_parsear); fflush(stdout); continue; }
        
        token = strtok(NULL, ";");
        if (token != NULL) {
            (*perguntas)[i].nivel = atoi(token);
        } else { printf("AVISO: Erro ao parsear nivel na linha: '%s'\n", linha); free(linha_para_parsear); fflush(stdout); continue; }

        free(linha_para_parsear); // Libera a memoria da linha para parsear
        (*perguntas)[i].usada = 0; // Inicializa a flag 'usada' como falso (0)
        i++;
    }
    
    printf("DEBUG: Total de perguntas validas carregadas na segunda passagem: %d\n", i);
    fflush(stdout);

    fclose(arquivo);
    return i;
}

void liberarPerguntas(Pergunta *perguntas) {
    if (perguntas != NULL) {
        free(perguntas);
        printf("DEBUG: Memoria das perguntas liberada.\n");
        fflush(stdout);
    }
}

// Funcao de embaralhamento permanece a mesma, mas nao sera usada diretamente para a selecao de perguntas aqui
void embaralharPerguntas(Pergunta *perguntas, int numPerguntas) {
    if (numPerguntas <= 1) return;

    for (int i = numPerguntas - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Pergunta temp = perguntas[i];
        perguntas[i] = perguntas[j];
        perguntas[j] = temp;
    }
    printf("DEBUG: Perguntas embaralhadas.\n");
    fflush(stdout);
}


// Funcao principal para realizar o quiz no estilo "Show do Milhao"
void realizarQuiz(Pergunta *perguntas, int numPerguntas) {
    if (perguntas == NULL || numPerguntas == 0) {
        printf("Nao ha perguntas para o quiz.\n");
        return;
    }

    // Marca todas as perguntas como nao usadas antes de comecar um novo quiz
    for (int i = 0; i < numPerguntas; i++) {
        perguntas[i].usada = 0;
    }

    long int premioAtual = 0; // Premio que o jogador tem no momento
    long int premioSeguro = 0; // Premio garantido em caso de erro

    char resposta[MAX_RESPOSTA_USUARIO];

    printf("\n--- Bem-vindo ao Show do Milhao! ---\n");
    printf("Teste seus conhecimentos e tente chegar ao premio maximo de R$ 1.000.000!\n");
    printf("Voce comeca com R$ 0. Boa sorte!\n");
    fflush(stdout);

    // Loop para as 15 perguntas
    for (int numPerguntaAtual = 1; numPerguntaAtual <= NUM_PERGUNTAS_SHOW_MILHAO; numPerguntaAtual++) {
        int nivelDificuldade;

        // Define o nivel de dificuldade com base na numeracao da pergunta
        if (numPerguntaAtual >= 1 && numPerguntaAtual <= 2) {
            nivelDificuldade = 1; // Muito Facil
        } else if (numPerguntaAtual >= 3 && numPerguntaAtual <= 4) {
            nivelDificuldade = 2; // Facil
        } else if (numPerguntaAtual >= 5 && numPerguntaAtual <= 8) {
            nivelDificuldade = 3; // Medio
        } else if (numPerguntaAtual >= 9 && numPerguntaAtual <= 12) {
            nivelDificuldade = 4; // Dificil
        } else { // Perguntas 13 a 15
            nivelDificuldade = 5; // Muito Dificil
        }

        // Atualiza os marcos de seguranca
        if (numPerguntaAtual == 5) {
            premioSeguro = PREMIOS_MILHAO[4]; // Premio apos acertar a 4a pergunta (antes da 5a ser jogada)
            printf("\n--- MARCO DE SEGURANCA ATINGIDO! Seu premio de R$ %ld esta GARANTIDO!---\n", premioSeguro);
            fflush(stdout);
        } else if (numPerguntaAtual == 10) {
            premioSeguro = PREMIOS_MILHAO[9]; // Premio apos acertar a 9a pergunta (antes da 10a ser jogada)
            printf("\n--- MARCO DE SEGURANCA ATINGIDO! Seu premio de R$ %ld esta GARANTIDO!---\n", premioSeguro);
            fflush(stdout);
        }

        printf("\n--- PERGUNTA %d --- Nivel: %d --- Premio Acumulado: R$ %ld --- Premio Seguro: R$ %ld ---\n",
               numPerguntaAtual, nivelDificuldade, premioAtual, premioSeguro);
        fflush(stdout);

        // Encontrar uma pergunta do nivel desejado que nao foi usada
        int idxPerguntaSelecionada = -1;
        int *indicesDisponiveis = NULL;
        int countDisponiveis = 0;

        // Contar quantas perguntas disponiveis (nao usadas) existem para o nivel atual
        for (int k = 0; k < numPerguntas; k++) {
            if (perguntas[k].nivel == nivelDificuldade && perguntas[k].usada == 0) {
                countDisponiveis++;
            }
        }

        if (countDisponiveis == 0) {
            printf("Erro: Nao ha perguntas nao utilizadas do nivel %d disponiveis no CSV. O jogo nao pode continuar.\n", nivelDificuldade);
            printf("DEBUG: Fim de jogo devido a falta de perguntas. Premio final: R$ %ld\n", premioSeguro); // Leva o seguro
            fflush(stdout);
            return; // Termina o jogo
        }

        // Aloca memoria para os indices das perguntas disponiveis para este nivel
        indicesDisponiveis = (int *)malloc(countDisponiveis * sizeof(int));
        if (indicesDisponiveis == NULL) {
            printf("Erro: Falha ao alocar memoria para indices disponiveis.\n");
            printf("DEBUG: Fim de jogo devido a erro de memoria. Premio final: R$ %ld\n", premioSeguro); // Leva o seguro
            fflush(stdout);
            return;
        }

        // Preenche o array com os indices das perguntas disponiveis
        int current_idx_preencher = 0;
        for (int k = 0; k < numPerguntas; k++) {
            if (perguntas[k].nivel == nivelDificuldade && perguntas[k].usada == 0) {
                indicesDisponiveis[current_idx_preencher++] = k;
            }
        }
        
        // Escolhe uma pergunta aleatoria entre as disponiveis
        idxPerguntaSelecionada = indicesDisponiveis[rand() % countDisponiveis];
        
        free(indicesDisponiveis); // Libera a memoria alocada para indicesDisponiveis

        perguntas[idxPerguntaSelecionada].usada = 1; // Marca a pergunta como usada

        printf("%s\n", perguntas[idxPerguntaSelecionada].enunciado);
        printf("A) %s\n", perguntas[idxPerguntaSelecionada].alternativaA);
        printf("B) %s\n", perguntas[idxPerguntaSelecionada].alternativaB);
        printf("C) %s\n", perguntas[idxPerguntaSelecionada].alternativaC);
        printf("D) %s\n", perguntas[idxPerguntaSelecionada].alternativaD);
        
        printf("Sua resposta: ");
        scanf("%s", resposta);
        while (getchar() != '\n'); // Limpa o buffer do teclado

        char respostaMaiuscula = toupper(resposta[0]);

        if (respostaMaiuscula == perguntas[idxPerguntaSelecionada].correta) {
            printf("Correto!\n");
            // Atualiza o premio para o valor apos acertar a pergunta atual
            premioAtual = PREMIOS_MILHAO[numPerguntaAtual];
            printf("Voce acumulou R$ %ld.\n", premioAtual);
            fflush(stdout);

            if (numPerguntaAtual == NUM_PERGUNTAS_SHOW_MILHAO) {
                printf("\nPARABENS! VOCE GANHOU O SHOW DO MILHAO!\n");
                printf("Seu premio final e de R$ %ld!\n", premioAtual);
                fflush(stdout);
                return; // Fim de jogo, o jogador ganhou!
            }

        } else {
            printf("Errado. A resposta correta era: %c\n", perguntas[idxPerguntaSelecionada].correta);
            printf("\n--- FIM DE JOGO ---\n");
            printf("Voce errou a pergunta e leva para casa: R$ %ld\n", premioSeguro); // Leva o ultimo premio seguro
            fflush(stdout);
            return; // Encerra o jogo
        }
    }
}
