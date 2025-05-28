#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>   // Para a função toupper (converter caracteres para maiúscula)
#include <time.h>    // Para a função time (usada para inicializar o gerador de números aleatórios)


#define MAX_PERGUNTAS_POSSIVEIS 100
#define TAM_MAX_ENUNCIADO 500
#define TAM_MAX_ALTERNATIVA 200
#define TAM_MAX_RESPOSTA_USUARIO 2
#define TAM_MAX_CAMPO_CSV 256
#define TAM_MAX_LINHA_CSV (TAM_MAX_ENUNCIADO + 4 * TAM_MAX_ALTERNATIVA + 20)
#define NUM_PERGUNTAS_NO_QUIZ 15
#define NUM_ETAPAS_PREMIOS (NUM_PERGUNTAS_NO_QUIZ + 1)

// Esta estrutura define o "molde" para cada pergunta do nosso quiz.
typedef struct {
    char enunciado[TAM_MAX_ENUNCIADO];
    char alternativaA[TAM_MAX_ALTERNATIVA];
    char alternativaB[TAM_MAX_ALTERNATIVA];
    char alternativaC[TAM_MAX_ALTERNATIVA];
    char alternativaD[TAM_MAX_ALTERNATIVA];
    char respostaCorreta; // Armazena a letra da alternativa correta (ex: 'A', 'B', 'C', 'D').
    int nivelDificuldade;    // Indica o nível de dificuldade da pergunta (1 a 5).
    int jaFoiUsada;    // Uma "bandeira": 0 significa que a pergunta ainda não foi usada, 1 significa que sim.
} Pergunta;

// --- Funções Auxiliares (Pequenos Ajudantes para Tarefas Específicas) ---

void removerQuebraLinha(char *str) {
    // Se a string for nula (não aponta para nada), não fazemos nada.
    if (str == NULL) return;
    
    size_t tamanho = strlen(str); // Pega o comprimento atual da string
    
    // Verifica se o último caractere é uma quebra de linha
    // e, se for, o substitui por '\0', encurtando a string.
    if (tamanho > 0 && (str[tamanho - 1] == '\n' || str[tamanho - 1] == '\r')) {
        str[tamanho - 1] = '\0';
    }
    if (tamanho > 1 && (str[tamanho - 2] == '\n' || str[tamanho - 2] == '\r')) {
        str[tamanho - 2] = '\0';
    }
}

//Funções Principais

int carregarPerguntasDeCsv(const char *nomeArquivo, Pergunta **enderecoDoArrayDePerguntas) {
    FILE *arquivo = fopen(nomeArquivo, "r"); // Tenta abrir o arquivo CSV no modo de leitura ("r").
    char linhaDoArquivo[TAM_MAX_LINHA_CSV]; // Buffer para armazenar temporariamente cada linha lida do arquivo.
    int numeroDePerguntasValidas = 0; // Contador para o número de perguntas que conseguimos ler e validar.
    int indiceParaPreencher = 0; // Índice para saber em qual posição do array de perguntas estamos.

    // Verifica se o arquivo foi aberto com sucesso. Se `arquivo` for NULL, significa que não abriu.
    if (arquivo == NULL) {
        printf("Nao foi possivel abrir o arquivo '%s'.\n", nomeArquivo);
        printf("Por favor, verifique se ele existe na mesma pasta do programa e se o nome esta correto.\n");
        fflush(stdout); // Garante que a mensagem seja exibida na tela imediatamente.
        *enderecoDoArrayDePerguntas = NULL; // Assegura que o ponteiro externo seja nulo em caso de erro.
        return 0; // Retorna 0 perguntas carregadas, indicando falha.
    }

    printf("DEBUG: Arquivo '%s' aberto com sucesso. Vamos la!\n", nomeArquivo);
    fflush(stdout);

    // Pula a primeira linha do CSV.
    if (fgets(linhaDoArquivo, sizeof(linhaDoArquivo), arquivo) == NULL) {
        printf("O arquivo CSV esta vazio ou houve um problema ao ler o cabecalho.\n");
        printf("Nao ha perguntas para carregar. Certifique-se de que o arquivo tem pelo menos uma linha de cabecalho.\n");
        fflush(stdout);
        fclose(arquivo); // Fecha o arquivo.
        *enderecoDoArrayDePerguntas = NULL;
        return 0;
    }
    removerQuebraLinha(linhaDoArquivo); // Limpa quaisquer quebras de linha do cabeçalho.
    printf("DEBUG: Cabecalho do CSV ignorado: '%s'\n", linhaDoArquivo);
    fflush(stdout);

    // Fazemos uma primeira leitura do arquivo para contar quantas linhas têm o formato correto (7 campos).
    int contadorDeLinhasComFormatoCorreto = 0;
    // Guarda a posição atual do ponteiro do arquivo. Vamos voltar para cá depois da contagem.
    long posicaoParaVoltarNoArquivo = ftell(arquivo); 

    while (fgets(linhaDoArquivo, sizeof(linhaDoArquivo), arquivo) != NULL) {
        removerQuebraLinha(linhaDoArquivo); // Limpa a linha lida.
        if (strlen(linhaDoArquivo) == 0) {
            continue; // Se a linha estiver vazia, pulamos para a próxima.
        }

        // `strtok` modifica a string original, então criamos uma cópia para não perder a linha original.
        char copiaDaLinhaParaContagem[TAM_MAX_LINHA_CSV];
        strncpy(copiaDaLinhaParaContagem, linhaDoArquivo, TAM_MAX_LINHA_CSV - 1);
        copiaDaLinhaParaContagem[TAM_MAX_LINHA_CSV - 1] = '\0'; // Garante terminação nula.

        char *token;
        int camposContadosNestaLinha = 0;
        
        token = strtok(copiaDaLinhaParaContagem, ";"); // Pega o primeiro campo usando ';' como separador.
        while (token != NULL) {
            camposContadosNestaLinha++;
            token = strtok(NULL, ";"); // Pega o próximo campo.
        }

        // Se a linha tem 7 campos (enunciado;A;B;C;D;correta;nivel), consideramos ela válida.
        if (camposContadosNestaLinha == 7) {
            contadorDeLinhasComFormatoCorreto++;
        } else {
            printf("Linha com formato invalido ignorada durante a contagem (esperado 7 campos, encontrado %d): '%s'\n", camposContadosNestaLinha, linhaDoArquivo);
            fflush(stdout);
        }
    }
    printf("DEBUG: Contagem inicial concluida. Encontradas %d perguntas com formato correto.\n", contadorDeLinhasComFormatoCorreto);
    fflush(stdout);

    // Se não encontramos nenhuma linha válida, avisamos e encerramos.
    if (contadorDeLinhasComFormatoCorreto == 0) {
        printf("Nao foi possivel encontrar nenhuma pergunta valida no arquivo CSV.\n");
        printf("Por favor, verifique se o formato de cada linha esta correto.\n");
        fflush(stdout);
        fclose(arquivo);
        *enderecoDoArrayDePerguntas = NULL;
        return 0;
    }

    // Aloca memória para o array de perguntas. Usamos `malloc` porque não sabemos o tamanho exato
    // do array em tempo de compilação, ele depende do conteúdo do CSV.
    *enderecoDoArrayDePerguntas = (Pergunta *)malloc(contadorDeLinhasComFormatoCorreto * sizeof(Pergunta));
    if (*enderecoDoArrayDePerguntas == NULL) {
        printf("Nao foi possivel alocar memoria suficiente para as perguntas.\n");
        printf("Seu computador esta sem memoria ou a quantidade de perguntas e muito grande?\n");
        fflush(stdout);
        fclose(arquivo);
        return 0;
    }
    numeroDePerguntasValidas = contadorDeLinhasComFormatoCorreto; // Define o número total de perguntas que serão carregadas.

    printf("Memoria alocada para %d perguntas. Iniciando a leitura detalhada das perguntas...\n", numeroDePerguntasValidas);
    fflush(stdout);

    // Volta o ponteiro do arquivo para a posição inicial após o cabeçalho.
    fseek(arquivo, posicaoParaVoltarNoArquivo, SEEK_SET);

    // Agora que a memória está alocada, lemos o arquivo novamente, linha por linha,
    // e preenchemos os campos de cada estrutura `Pergunta`.
    indiceParaPreencher = 0;
    while (fgets(linhaDoArquivo, sizeof(linhaDoArquivo), arquivo) != NULL && indiceParaPreencher < numeroDePerguntasValidas) {
        removerQuebraLinha(linhaDoArquivo); // Limpa a linha lida.
        if (strlen(linhaDoArquivo) == 0) {
            continue; // Pula linhas em branco.
        }
        
        // Novamente, uma cópia da linha para `strtok` não estragar a original.
        char copiaDaLinhaParaParse[TAM_MAX_LINHA_CSV];
        strncpy(copiaDaLinhaParaParse, linhaDoArquivo, TAM_MAX_LINHA_CSV - 1);
        copiaDaLinhaParaParse[TAM_MAX_LINHA_CSV - 1] = '\0';

        char *token;
        
        // O `strtok` divide a string em "tokens" usando o delimitador ';'.
        // Cada chamada a `strtok(NULL, ";")` pega o próximo pedaço.
        token = strtok(copiaDaLinhaParaParse, ";"); // Pega o enunciado.
        if (token != NULL) {
            strncpy((*enderecoDoArrayDePerguntas)[indiceParaPreencher].enunciado, token, TAM_MAX_ENUNCIADO - 1);
            (*enderecoDoArrayDePerguntas)[indiceParaPreencher].enunciado[TAM_MAX_ENUNCIADO - 1] = '\0';
        } else { printf("AVISO: Falha ao ler enunciado na linha: '%s'\n", linhaDoArquivo); fflush(stdout); continue; }

        token = strtok(NULL, ";"); // Pega a alternativa A.
        if (token != NULL) {
            strncpy((*enderecoDoArrayDePerguntas)[indiceParaPreencher].alternativaA, token, TAM_MAX_ALTERNATIVA - 1);
            (*enderecoDoArrayDePerguntas)[indiceParaPreencher].alternativaA[TAM_MAX_ALTERNATIVA - 1] = '\0';
        } else { printf("AVISO: Falha ao ler alternativa A na linha: '%s'\n", linhaDoArquivo); fflush(stdout); continue; }

        token = strtok(NULL, ";"); // Pega a alternativa B.
        if (token != NULL) {
            strncpy((*enderecoDoArrayDePerguntas)[indiceParaPreencher].alternativaB, token, TAM_MAX_ALTERNATIVA - 1);
            (*enderecoDoArrayDePerguntas)[indiceParaPreencher].alternativaB[TAM_MAX_ALTERNATIVA - 1] = '\0';
        } else { printf("AVISO: Falha ao ler alternativa B na linha: '%s'\n", linhaDoArquivo); fflush(stdout); continue; }

        token = strtok(NULL, ";"); // Pega a alternativa C.
        if (token != NULL) {
            strncpy((*enderecoDoArrayDePerguntas)[indiceParaPreencher].alternativaC, token, TAM_MAX_ALTERNATIVA - 1);
            (*enderecoDoArrayDePerguntas)[indiceParaPreencher].alternativaC[TAM_MAX_ALTERNATIVA - 1] = '\0';
        } else { printf("AVISO: Falha ao ler alternativa C na linha: '%s'\n", linhaDoArquivo); fflush(stdout); continue; }

        token = strtok(NULL, ";"); // Pega a alternativa D.
        if (token != NULL) {
            strncpy((*enderecoDoArrayDePerguntas)[indiceParaPreencher].alternativaD, token, TAM_MAX_ALTERNATIVA - 1);
            (*enderecoDoArrayDePerguntas)[indiceParaPreencher].alternativaD[TAM_MAX_ALTERNATIVA - 1] = '\0';
        } else { printf("AVISO: Falha ao ler alternativa D na linha: '%s'\n", linhaDoArquivo); fflush(stdout); continue; }

        token = strtok(NULL, ";"); // Pega a resposta correta.
        if (token != NULL) {
            (*enderecoDoArrayDePerguntas)[indiceParaPreencher].respostaCorreta = toupper(token[0]); // Converte para maiúscula para padronizar.
        } else { printf("AVISO: Falha ao ler resposta correta na linha: '%s'\n", linhaDoArquivo); fflush(stdout); continue; }
        
        token = strtok(NULL, ";"); // Pega o nível de dificuldade.
        if (token != NULL) {
            (*enderecoDoArrayDePerguntas)[indiceParaPreencher].nivelDificuldade = atoi(token); // Converte a string do nível para um número inteiro.
        } else { printf("AVISO: Falha ao ler nivel de dificuldade na linha: '%s'\n", linhaDoArquivo); fflush(stdout); continue; }

        // Inicializa a flag `jaFoiUsada` como 0 (falso), indicando que a pergunta ainda não foi feita.
        (*enderecoDoArrayDePerguntas)[indiceParaPreencher].jaFoiUsada = 0; 
        indiceParaPreencher++; // Avança para a próxima posição no array.
    }
    
    printf("DEBUG: Leitura e preenchimento de perguntas concluido. Total de %d perguntas prontas!\n", indiceParaPreencher);
    fflush(stdout);

    fclose(arquivo); // Fecha o arquivo CSV.
    return indiceParaPreencher; // Retorna o número de perguntas que foram realmente carregadas.
}

void liberarMemoriaDoArrayDePerguntas(Pergunta *arrayDePerguntas) {
    if (arrayDePerguntas != NULL) {
        free(arrayDePerguntas); // Libera a memória alocada por `malloc`.
        printf("DEBUG: Memoria das perguntas liberada. Tchau, tchau, memoria!\n");
        fflush(stdout);
    }
}

void iniciarShowDoMilhao(Pergunta *todasAsPerguntas, int numeroTotalDePerguntas, const int tabelaDePremios[]) {
    // Se não há perguntas ou o array é nulo, não podemos iniciar o jogo.
    if (todasAsPerguntas == NULL || numeroTotalDePerguntas == 0) {
        printf("Nao da pra jogar! Nao ha perguntas validas disponiveis no nosso banco de dados.\n");
        return; // Encerra a função.
    }

    // No início de cada jogo, todas as perguntas são marcadas como "não usadas".
    // Isso é importante para que, se o jogo for reiniciado, todas as perguntas estejam disponíveis novamente.
    for (int i = 0; i < numeroTotalDePerguntas; i++) {
        todasAsPerguntas[i].jaFoiUsada = 0;
    }

    long int premioAcumuladoDoJogador = 0;    // O dinheiro que o jogador tem no momento.
    long int premioGarantidoEmCasoDeErro = 0; // O dinheiro que o jogador leva para casa se errar (marcos de segurança).

    char respostaDigitadaPeloJogador[TAM_MAX_RESPOSTA_USUARIO]; // Buffer para armazenar a resposta do jogador.

    // Mensagens de boas-vindas e introdução ao jogo.
    printf("\nBem-vindo ao Show do Milhao! A sua chance de ficar rico!\n");
    printf("Prepare-se para testar seus conhecimentos e ir em busca de R$ 1.000.000!\n");
    printf("Voce comeca zerado. Boa sorte!\n");
    fflush(stdout);

    // O jogo avança por 15 perguntas, de 1 a 15.
    for (int numeroDaRodadaAtual = 1; numeroDaRodadaAtual <= NUM_PERGUNTAS_NO_QUIZ; numeroDaRodadaAtual++) {
        int nivelDeDificuldadeRequerido; // O nível de dificuldade da pergunta para a rodada atual.

        // Define o nível de dificuldade com base na rodada atual, seguindo a lógica do Show do Milhão.
        if (numeroDaRodadaAtual >= 1 && numeroDaRodadaAtual <= 2) {
            nivelDeDificuldadeRequerido = 1; // Muito Fácil
        } else if (numeroDaRodadaAtual >= 3 && numeroDaRodadaAtual <= 4) {
            nivelDeDificuldadeRequerido = 2; // Fácil
        } else if (numeroDaRodadaAtual >= 5 && numeroDaRodadaAtual <= 8) {
            nivelDeDificuldadeRequerido = 3; // Médio
        } else if (numeroDaRodadaAtual >= 9 && numeroDaRodadaAtual <= 12) {
            nivelDeDificuldadeRequerido = 4; // Difícil
        } else { // Perguntas 13 a 15 (as mais desafiadoras!)
            nivelDeDificuldadeRequerido = 5; // Muito Difícil
        }

        // Na 5ª pergunta, o valor acumulado até a 4ª pergunta (premio após P4) vira prêmio garantido.
        if (numeroDaRodadaAtual == 5) {
            premioGarantidoEmCasoDeErro = tabelaDePremios[4]; // O prêmio da tabela na posição 4 (após P4).
            printf("\nVoce atingiu o PRIMEIRO MARCO DE SEGURANCA! Seu premio de R$ %ld esta GARANTIDO!\n", premioGarantidoEmCasoDeErro);
            fflush(stdout);
        }
        // Na 10ª pergunta, o valor acumulado até a 9ª pergunta (premio após P9) vira prêmio garantido.
        else if (numeroDaRodadaAtual == 10) {
            premioGarantidoEmCasoDeErro = tabelaDePremios[9]; // O prêmio da tabela na posição 9 (após P9).
            printf("\nVoce atingiu o SEGUNDO MARCO DE SEGURANCA! R$ %ld ja sao seus!\n", premioGarantidoEmCasoDeErro);
            printf("Falta pouco para o milhao, nao desista!\n");
            fflush(stdout);
        }

        // Exibe o status atual do jogo para o jogador.
        printf("\n--- RODADA %d --- Nivel: %d --- Dinheiro Acumulado: R$ %ld --- Premio Garantido: R$ %ld ---\n",
               numeroDaRodadaAtual, nivelDeDificuldadeRequerido, premioAcumuladoDoJogador, premioGarantidoEmCasoDeErro);
        fflush(stdout);

        //Seleção da Pergunta
        int indiceDaPerguntaSelecionada = -1; // Variável para guardar o índice da pergunta que vamos usar.
        int *indicesDasPerguntasDisponiveisDesteNivel = NULL; // Um array temporário para guardar os índices das perguntas que podemos escolher.
        int quantidadeDePerguntasDisponiveisDesteNivel = 0; // Contador de perguntas disponíveis para o nível atual.

        // Primeiro, contamos quantas perguntas *não usadas* existem para o nível de dificuldade atual.
        for (int k = 0; k < numeroTotalDePerguntas; k++) {
            if (todasAsPerguntas[k].nivelDificuldade == nivelDeDificuldadeRequerido && todasAsPerguntas[k].jaFoiUsada == 0) {
                quantidadeDePerguntasDisponiveisDesteNivel++;
            }
        }

        // Se não há perguntas disponíveis para o nível atual, avisamos e encerramos o jogo.
        if (quantidadeDePerguntasDisponiveisDesteNivel == 0) {
            printf("Nao ha perguntas ineditas do nivel %d disponiveis no nosso banco de dados. O jogo nao pode continuar.\n", nivelDeDificuldadeRequerido);
            printf("Fim de jogo inesperado por falta de perguntas. Voce leva para casa: R$ %ld\n", premioGarantidoEmCasoDeErro);
            fflush(stdout);
            return; // Termina a função do jogo.
        }

        // Alocamos memória para o array temporário que guardará os índices das perguntas disponíveis.
        indicesDasPerguntasDisponiveisDesteNivel = (int *)malloc(quantidadeDePerguntasDisponiveisDesteNivel * sizeof(int));
        if (indicesDasPerguntasDisponiveisDesteNivel == NULL) {
            printf("Nao foi possivel alocar memoria para selecionar a proxima pergunta.\n");
            printf("Fim de jogo por erro de memoria. Voce leva para casa: R$ %ld\n", premioGarantidoEmCasoDeErro);
            fflush(stdout);
            return;
        }

        // Preenche o array temporário com os índices das perguntas que atendem aos critérios (nível e não usada).
        int indiceParaPreencherArrayDisponiveis = 0;
        for (int k = 0; k < numeroTotalDePerguntas; k++) {
            if (todasAsPerguntas[k].nivelDificuldade == nivelDeDificuldadeRequerido && todasAsPerguntas[k].jaFoiUsada == 0) {
                indicesDasPerguntasDisponiveisDesteNivel[indiceParaPreencherArrayDisponiveis++] = k;
            }
        }
        
        // Escolhe um índice aleatório dentro do array de perguntas disponíveis para este nível.
        indiceDaPerguntaSelecionada = indicesDasPerguntasDisponiveisDesteNivel[rand() % quantidadeDePerguntasDisponiveisDesteNivel];
        
        // Libera a memória do array temporário.
        free(indicesDasPerguntasDisponiveisDesteNivel); 

        // Marca a pergunta selecionada como "usada" para que ela não seja repetida.
        todasAsPerguntas[indiceDaPerguntaSelecionada].jaFoiUsada = 1; 

        //Exibição da Pergunta e Coleta da Resposta do Jogador
        printf("%s\n", todasAsPerguntas[indiceDaPerguntaSelecionada].enunciado);
        printf("A) %s\n", todasAsPerguntas[indiceDaPerguntaSelecionada].alternativaA);
        printf("B) %s\n", todasAsPerguntas[indiceDaPerguntaSelecionada].alternativaB);
        printf("C) %s\n", todasAsPerguntas[indiceDaPerguntaSelecionada].alternativaC);
        printf("D) %s\n", todasAsPerguntas[indiceDaPerguntaSelecionada].alternativaD);
        
        printf("Qual a sua resposta? (A, B, C ou D): ");
        scanf("%s", respostaDigitadaPeloJogador); // Lê a resposta do jogador.
        // Limpa o buffer do teclado.
        while (getchar() != '\n'); 

        // Converte a primeira letra da resposta do jogador para maiúscula para facilitar a comparação.
        char respostaDoJogadorEmMaiuscula = toupper(respostaDigitadaPeloJogador[0]);

        //Verificação da Resposta
        if (respostaDoJogadorEmMaiuscula == todasAsPerguntas[indiceDaPerguntaSelecionada].respostaCorreta) {
            printf("Uau! Resposta correta! Voce mandou muito bem!\n");
            // Atualiza o prêmio acumulado com o valor correspondente à rodada atual na tabela de prêmios.
            premioAcumuladoDoJogador = tabelaDePremios[numeroDaRodadaAtual];
            printf("Seu premio atual e de R$ %ld.\n", premioAcumuladoDoJogador);
            fflush(stdout);

            // Se o jogador acertou a última pergunta (a 15ª), ele ganhou o milhão!
            if (numeroDaRodadaAtual == NUM_PERGUNTAS_NO_QUIZ) {
                printf("\nPARABENS! VOCE GANHOU O SHOW DO MILHAO!\n");
                printf("Com muito conhecimento, voce levou o grande premio de R$ %ld! Que incrivel!\n", premioAcumuladoDoJogador);
                fflush(stdout);
                return; // Encerra o jogo, o jogador venceu.
            }

        } else {
            printf("Que pena! Resposta incorreta. A resposta certa era: %c\n", todasAsPerguntas[indiceDaPerguntaSelecionada].respostaCorreta);
            printf("\nFIM DE JOGO\n");
            printf("Voce leva para casa o premio garantido de: R$ %ld\n", premioGarantidoEmCasoDeErro);
            fflush(stdout);
            return; // Encerra o jogo, o jogador errou.
        }
    }
}

//Função Principal do Programa
// Esta é a primeira função que o programa executa.
int main() {
    Pergunta *bancoDePerguntas = NULL; // Este ponteiro vai guardar o array de todas as perguntas carregadas.
    int quantidadeDePerguntasCarregadas = 0; // Variável para armazenar quantas perguntas conseguimos ler do CSV.

    // Inicializa o gerador de números aleatórios. Usamos `time(NULL)` como "semente"
    // para que a sequência de números aleatórios seja diferente a cada vez que o programa é executado.
    srand(time(NULL));

    // Define a tabela de prêmios do "Show do Milhão".
    // Esta é uma array constante, ou seja, seus valores não podem ser alterados.
    // Ela será passada para a função `iniciarShowDoMilhao`.
    const int premiosDoMilhao[NUM_ETAPAS_PREMIOS] = {
        0,         // Premio inicial (antes da Pergunta 1)
        1000,      // Premio apos Pergunta 1
        2000,      // Premio apos Pergunta 2
        3000,      // Premio apos Pergunta 3
        4000,      // Premio apos Pergunta 4
        5000,      // Premio apos Pergunta 5 (1o. Marco de Seguranca)
        10000,     // Premio apos Pergunta 6
        20000,     // Premio apos Pergunta 7
        30000,     // Premio apos Pergunta 8
        40000,     // Premio apos Pergunta 9
        50000,     // Premio apos Pergunta 10 (2o. Marco de Seguranca)
        100000,    // Premio apos Pergunta 11
        200000,    // Premio apos Pergunta 12
        300000,    // Premio apos Pergunta 13
        400000,    // Premio apos Pergunta 14
        1000000    // Premio apos Pergunta 15 (O Grande Milhao)
    };

    // Tenta carregar as perguntas do arquivo "projetobase.csv".
    // A função `carregarPerguntasDeCsv` alocará a memória necessária para `bancoDePerguntas`
    // e preencherá esse array com os dados do arquivo.
    quantidadeDePerguntasCarregadas = carregarPerguntasDeCsv("../projetobase.csv", &bancoDePerguntas);

    // Verifica se conseguimos carregar alguma pergunta do arquivo.
    if (quantidadeDePerguntasCarregadas > 0) {
        printf("\nExcelente! As perguntas foram carregadas com sucesso.\n");
        printf("Prepare-se para o desafio! Temos %d perguntas prontas no nosso banco.\n\n", quantidadeDePerguntasCarregadas);

        // Chama a função que inicia e gerencia todo o jogo "Show do Milhão".
        // Passamos a ela o array de perguntas, o número total delas e a tabela de prêmios.
        iniciarShowDoMilhao(bancoDePerguntas, quantidadeDePerguntasCarregadas, premiosDoMilhao);

    } else {
        printf("Nao foi possivel iniciar o quiz. Por favor, verifique se o arquivo 'projetobase.csv' esta correto e com perguntas validas.\n");
    }

    liberarMemoriaDoArrayDePerguntas(bancoDePerguntas);

    printf("\nObrigado por jogar! Volte sempre e treine seu conhecimento!\n");

    return 0; // Indica que o programa terminou com sucesso.
}
