#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> // Para a função toupper (converter caracteres para maiúscula)
#include <time.h>  // Para a função time (usada para inicializar o gerador de números aleatórios)

typedef struct
{
    char enunciado[500];
    char alternativaA[200];
    char alternativaB[200];
    char alternativaC[200];
    char alternativaD[200];
    char respostaCorreta; // Armazena a letra da alternativa correta (ex: 'A', 'B', 'C', 'D').
    int nivel;            // Indica o nível de dificuldade da pergunta (1 a 5).
    int jaUsada;          // Uma "bandeira": 0 significa que a pergunta ainda não foi usada, 1 significa que sim.
} Pergunta;

// Funções Auxiliares

void removerQuebraLinha(char *str)
{
    // Se a string for nula, não fazemos nada.
    if (str == NULL)
        return;

    size_t tamanho = strlen(str); // Pega o comprimento atual da string

    // Verifica se o último caractere é uma quebra de linha
    // e, se for, o substitui por '\0', encurtando a string.
    if (tamanho > 0 && (str[tamanho - 1] == '\n' || str[tamanho - 1] == '\r'))
    {
        str[tamanho - 1] = '\0';
    }
    if (tamanho > 1 && (str[tamanho - 2] == '\n' || str[tamanho - 2] == '\r'))
    {
        str[tamanho - 2] = '\0';
    }
}

// Funções Principais

int carregarcsv(const char *nomeArquivo, Pergunta **arrayperguntas)
{
    FILE *arquivo = fopen(nomeArquivo, "r"); // Tenta abrir o arquivo CSV no modo de leitura ("r").
    char linha[1320];                        // Buffer para armazenar temporariamente cada linha lida do arquivo.
    int numeroperguntas = 0;                 // Contador para o número de perguntas.
    int indice = 0;                          // Índice para saber em qual posição do array de perguntas estamos.

    // Verifica se o arquivo foi aberto com sucesso. Se `arquivo` for NULL, significa que não abriu.
    if (arquivo == NULL)
    {
        printf("Nao foi possivel abrir o arquivo '%s'.\n", nomeArquivo);
        printf("Por favor, verifique se ele existe na mesma pasta do programa e se o nome esta correto.\n");
        fflush(stdout); // Garante que a mensagem seja exibida na tela imediatamente.
        *arrayperguntas = NULL;
        return 0;
    }

    fflush(stdout);

    // Pula a primeira linha do CSV.
    if (fgets(linha, sizeof(linha), arquivo) == NULL)
    {
        printf("O arquivo CSV esta vazio ou houve um problema ao ler o cabecalho.\n");
        printf("Nao ha perguntas para carregar. Certifique-se de que o arquivo tem pelo menos uma linha de cabecalho.\n");
        fflush(stdout);
        fclose(arquivo);
        *arrayperguntas = NULL;
        return 0;
    }
    removerQuebraLinha(linha); // Limpa quaisquer quebras de linha do cabeçalho.
    fflush(stdout);

    // Fazemos uma primeira leitura do arquivo para contar quantas linhas têm o formato correto (7 campos).
    int contadorlinhas = 0;
    long posicao = ftell(arquivo);

    while (fgets(linha, sizeof(linha), arquivo) != NULL)
    {
        removerQuebraLinha(linha); // Limpa a linha lida.
        if (strlen(linha) == 0)
        {
            continue; // Se a linha estiver vazia, pulamos para a próxima.
        }
        char copialinha[1320];
        strncpy(copialinha, linha, 1319);
        copialinha[1319] = '\0'; // Garante terminação nula.

        char *token;
        int camposplinha = 0;

        token = strtok(copialinha, ";"); // Pega o primeiro campo usando ';' como separador.
        while (token != NULL)
        {
            camposplinha++;
            token = strtok(NULL, ";"); // Pega o próximo campo.
        }

        // Se a linha tem 7 campos (enunciado;A;B;C;D;correta;nivel), consideramos ela válida.
        if (camposplinha == 7)
        {
            contadorlinhas++;
        }
        else
        {
            printf("Linha com formato invalido ignorada durante a contagem (esperado 7 campos, encontrado %d): '%s'\n", camposplinha, linha);
            fflush(stdout);
        }
    }
    printf("DEBUG: Contagem inicial concluida. Encontradas %d perguntas com formato correto.\n", contadorlinhas);
    fflush(stdout);

    // Se não encontramos nenhuma linha válida, avisamos e encerramos.
    if (contadorlinhas == 0)
    {
        printf("Nao foi possivel encontrar nenhuma pergunta valida no arquivo CSV.\n");
        printf("Por favor, verifique se o formato de cada linha esta correto.\n");
        fflush(stdout);
        fclose(arquivo);
        *arrayperguntas = NULL;
        return 0;
    }

    // Aloca memória para o array de perguntas. Usamos `malloc` porque não sabemos o tamanho exato
    // do array em tempo de compilação, ele depende do conteúdo do CSV.
    *arrayperguntas = (Pergunta *)malloc(contadorlinhas * sizeof(Pergunta));
    if (*arrayperguntas == NULL)
    {
        printf("Nao foi possivel alocar memoria suficiente para as perguntas.\n");
        printf("Seu computador esta sem memoria ou a quantidade de perguntas e muito grande?\n");
        fflush(stdout);
        fclose(arquivo);
        return 0;
    }
    numeroperguntas = contadorlinhas; // Define o número total de perguntas que serão carregadas.
    fflush(stdout);

    // Volta o ponteiro do arquivo para a posição inicial após o cabeçalho.
    fseek(arquivo, posicao, SEEK_SET);

    // Agora que a memória está alocada, lemos o arquivo novamente
    indice = 0;
    while (fgets(linha, sizeof(linha), arquivo) != NULL && indice < numeroperguntas)
    {
        removerQuebraLinha(linha); // Limpa a linha lida.
        if (strlen(linha) == 0)
        {
            continue; // Pula linhas em branco.
        }

        // cópia da linha para `strtok` não estragar a original.
        char copiaparse[1320];
        strncpy(copiaparse, linha, 1319);
        copiaparse[1319] = '\0';

        char *token;

        // O `strtok` divide a string em "tokens" usando o delimitador ';'.
        token = strtok(copiaparse, ";"); // Pega o enunciado.
        if (token != NULL)
        {
            strncpy((*arrayperguntas)[indice].enunciado, token, 499);
            (*arrayperguntas)[indice].enunciado[499] = '\0';
        }
        else
        {
            printf("Falha ao ler enunciado na linha: '%s'\n", linha);
            fflush(stdout);
            continue;
        }

        token = strtok(NULL, ";"); // Pega a alternativa A.
        if (token != NULL)
        {
            strncpy((*arrayperguntas)[indice].alternativaA, token, 199);
            (*arrayperguntas)[indice].alternativaA[199] = '\0';
        }
        else
        {
            printf("AVISO: Falha ao ler alternativa A na linha: '%s'\n", linha);
            fflush(stdout);
            continue;
        }

        token = strtok(NULL, ";"); // Pega a alternativa B.
        if (token != NULL)
        {
            strncpy((*arrayperguntas)[indice].alternativaB, token, 199);
            (*arrayperguntas)[indice].alternativaB[199] = '\0';
        }
        else
        {
            printf("AVISO: Falha ao ler alternativa B na linha: '%s'\n", linha);
            fflush(stdout);
            continue;
        }

        token = strtok(NULL, ";"); // Pega a alternativa C.
        if (token != NULL)
        {
            strncpy((*arrayperguntas)[indice].alternativaC, token, 200 - 1);
            (*arrayperguntas)[indice].alternativaC[200 - 1] = '\0';
        }
        else
        {
            printf("AVISO: Falha ao ler alternativa C na linha: '%s'\n", linha);
            fflush(stdout);
            continue;
        }

        token = strtok(NULL, ";"); // Pega a alternativa D.
        if (token != NULL)
        {
            strncpy((*arrayperguntas)[indice].alternativaD, token, 199);
            (*arrayperguntas)[indice].alternativaD[199] = '\0';
        }
        else
        {
            printf("AVISO: Falha ao ler alternativa D na linha: '%s'\n", linha);
            fflush(stdout);
            continue;
        }

        token = strtok(NULL, ";"); // Pega a resposta correta.
        if (token != NULL)
        {
            (*arrayperguntas)[indice].respostaCorreta = toupper(token[0]); // Converte para maiúscula para padronizar.
        }
        else
        {
            printf("AVISO: Falha ao ler resposta correta na linha: '%s'\n", linha);
            fflush(stdout);
            continue;
        }

        token = strtok(NULL, ";"); // Pega o nível de dificuldade.
        if (token != NULL)
        {
            (*arrayperguntas)[indice].nivel = atoi(token); // Converte a string do nível para um número inteiro.
        }
        else
        {
            printf("AVISO: Falha ao ler nivel de dificuldade na linha: '%s'\n", linha);
            fflush(stdout);
            continue;
        }

        // Inicializa a flag `jaUsada` como 0 (falso), indicando que a pergunta ainda não foi feita.
        (*arrayperguntas)[indice].jaUsada = 0;
        indice++; // Avança para a próxima posição no array.
    }

    fflush(stdout);

    fclose(arquivo); // Fecha o arquivo CSV.
    return indice;   // Retorna o número de perguntas que foram realmente carregadas.
}

void liberarmemoria(Pergunta *array2deperguntas)
{
    if (array2deperguntas != NULL)
    {
        free(array2deperguntas);
        fflush(stdout);
    }
}

void iniciarjogo(Pergunta *todasperguntas, int totalperguntas, const int tabelapremios[])
{
    // Se não há perguntas ou o array é nulo, não podemos iniciar o jogo.
    if (todasperguntas == NULL || totalperguntas == 0)
    {
        printf("Nao ha perguntas validas disponiveis no nosso banco de dados.\n");
        return; // Encerra a função.
    }

    // No início de cada jogo, todas as perguntas são marcadas como "não usadas".
    // Isso é importante para que, se o jogo for reiniciado, todas as perguntas estejam disponíveis novamente.
    for (int i = 0; i < totalperguntas; i++)
    {
        todasperguntas[i].jaUsada = 0;
    }

    int premioacumulado = 0;    // O dinheiro que o jogador tem no momento.
    int premiogarantido = 0; // O dinheiro que o jogador leva para casa se errar (marcos de segurança).

    char resposta[2]; // Buffer para armazenar a resposta do jogador.

    // Mensagens de boas-vindas e introdução ao jogo.
    printf("\nBem-vindo ao Show do Milhao!\n");
    printf("Prepare-se para testar seus conhecimentos e ir em busca de R$ 1.000.000!\n");
    printf("Voce comeca zerado. Boa sorte!\n");
    fflush(stdout);

    // O jogo avança por 15 perguntas, de 1 a 15.
    for (int rodadaatual = 1; rodadaatual <= 15; rodadaatual++)
    {
        int dificuldaderequerida; 

        // Define o nível de dificuldade com base na rodada atual, seguindo a lógica do Show do Milhão.
        if (rodadaatual >= 1 && rodadaatual <= 2)
        {
            dificuldaderequerida = 1; // Muito Fácil
        }
        else if (rodadaatual >= 3 && rodadaatual <= 4)
        {
            dificuldaderequerida = 2; // Fácil
        }
        else if (rodadaatual >= 5 && rodadaatual <= 8)
        {
            dificuldaderequerida = 3; // Médio
        }
        else if (rodadaatual >= 9 && rodadaatual <= 12)
        {
            dificuldaderequerida = 4; // Difícil
        }
        else
        {                                    // Perguntas 13 a 15 (as mais desafiadoras!)
            dificuldaderequerida = 5; // Muito Difícil
        }

        // Na 5ª pergunta, o valor acumulado até a 4ª pergunta (premio após P4) vira prêmio garantido.
        if (rodadaatual == 5)
        {
            premiogarantido = tabelapremios[4]; // O prêmio da tabela na posição 4 (após P4).
            printf("\nVoce atingiu o primeiro porto seguro! Seu premio de R$ %d esta garantido!\n", premiogarantido);
            fflush(stdout);
        }
        // Na 10ª pergunta, o valor acumulado até a 9ª pergunta (premio após P9) vira prêmio garantido.
        else if (rodadaatual == 10)
        {
            premiogarantido = tabelapremios[9]; // O prêmio da tabela na posição 9 (após P9).
            printf("\nVoce atingiu o segundo porto seguro! R$ %d ja sao seus!\n", premiogarantido);
            fflush(stdout);
        }

        // Exibe o status atual do jogo para o jogador.
        printf("\n--- RODADA %d --- Nivel: %d --- Dinheiro Acumulado: R$ %d --- Premio Garantido: R$ %d ---\n",
               rodadaatual, dificuldaderequerida, premioacumulado, premiogarantido);
        fflush(stdout);

        // Seleção da Pergunta
        int indperguntaselecionada = -1;                 // Variável para guardar o índice da pergunta que vamos usar.
        int *indperguntasdoatualnivel = NULL; // Um array temporário para guardar os índices das perguntas que podemos escolher.
        int qtdperguntasdoatualnivel = 0;   // Contador de perguntas disponíveis para o nível atual.

        // Primeiro, contamos quantas perguntas *não usadas* existem para o nível de dificuldade atual.
        for (int i = 0; i < totalperguntas; i++)
        {
            if (todasperguntas[i].nivel == dificuldaderequerida && todasperguntas[i].jaUsada == 0)
            {
                qtdperguntasdoatualnivel++;
            }
        }

        if (qtdperguntasdoatualnivel == 0)
        {
            printf("Nao ha perguntas ineditas do nivel %d disponiveis no nosso banco de dados.\n", dificuldaderequerida);
            printf("Fim de jogo inesperado por falta de perguntas. Voce leva para casa: R$ %d\n", premiogarantido);
            fflush(stdout);
            return; // Termina a função do jogo.
        }

        // Alocamos memória para o array temporário que guardará os índices das perguntas disponíveis.
        indperguntasdoatualnivel = (int *)malloc(qtdperguntasdoatualnivel * sizeof(int));
        if (indperguntasdoatualnivel == NULL)
        {
            printf("Nao foi possivel alocar memoria para selecionar a proxima pergunta.\n");
            printf("Fim de jogo por erro de memoria. Voce leva para casa: R$ %d\n", premiogarantido);
            fflush(stdout);
            return;
        }

        // Preenche o array temporário com os índices das perguntas que atendem aos critérios (nível e não usada).
        int indicearraysdisponiveis = 0;
        for (int i = 0; i < totalperguntas; i++)
        {
            if (todasperguntas[i].nivel == dificuldaderequerida && todasperguntas[i].jaUsada == 0)
            {
                indperguntasdoatualnivel[indicearraysdisponiveis++] = i;
            }
        }

        // Escolhe um índice aleatório dentro do array de perguntas disponíveis para este nível.
        indperguntaselecionada = indperguntasdoatualnivel[rand() % qtdperguntasdoatualnivel];

        // Libera a memória do array temporário.
        free(indperguntasdoatualnivel);

        // Marca a pergunta selecionada como "usada" para que ela não seja repetida.
        todasperguntas[indperguntaselecionada].jaUsada = 1;

        // Exibição da Pergunta e Coleta da Resposta do Jogador
        printf("%s\n", todasperguntas[indperguntaselecionada].enunciado);
        printf("A) %s\n", todasperguntas[indperguntaselecionada].alternativaA);
        printf("B) %s\n", todasperguntas[indperguntaselecionada].alternativaB);
        printf("C) %s\n", todasperguntas[indperguntaselecionada].alternativaC);
        printf("D) %s\n", todasperguntas[indperguntaselecionada].alternativaD);

        printf("Qual a sua resposta? ");
        scanf("%s", resposta); // Lê a resposta do jogador.
        // Limpa o buffer do teclado.
        while (getchar() != '\n')
            ;

        // Converte a primeira letra da resposta do jogador para maiúscula para facilitar a comparação.
        char respostaDoJogadorEmMaiuscula = toupper(resposta[0]);

        // Verificação da Resposta
        if (respostaDoJogadorEmMaiuscula == todasperguntas[indperguntaselecionada].respostaCorreta)
        {
            printf("Resposta correta!\n");
            // Atualiza o prêmio acumulado com o valor correspondente à rodada atual na tabela de prêmios.
            premioacumulado = tabelapremios[rodadaatual];
            printf("Seu premio atual e de R$ %d.\n", premioacumulado);
            fflush(stdout);

            // Se o jogador acertou a última pergunta (a 15ª), ele ganhou o milhão!
            if (rodadaatual == 15)
            {
                printf("\nPARABENS, VOCE GANHOU O SHOW DO MILHAO!\n");
                printf("Voce levou o grande premio de R$ %d!\n", premioacumulado);
                fflush(stdout);
                return;
            }
        }
        else
        {
            printf("Resposta incorreta. A resposta certa era: %c\n", todasperguntas[indperguntaselecionada].respostaCorreta);
            printf("\nFIM DE JOGO\n");
            printf("Voce leva para casa o premio garantido de: R$ %d\n", premiogarantido);
            fflush(stdout);
            return; // Encerra o jogo, o jogador errou.
        }
    }
}

// Função Principal do Programa
//  Esta é a primeira função que o programa executa.
int main()
{
    Pergunta *bancoperguntas = NULL;       // Este ponteiro vai guardar o array de todas as perguntas carregadas.
    int qtdperguntascarregadas = 0; 

    // Inicializa o gerador de números aleatórios
    // para que a sequência de números aleatórios seja diferente a cada vez que o programa é executado.
    srand(time(NULL));

    // Define a tabela de prêmios do "Show do Milhão".
    // Esta é uma array constante, ou seja, seus valores não podem ser alterados.
    // Ela será passada para a função `iniciarjogo`.
    const int premios[16] = {
        0,      // Premio inicial (antes da Pergunta 1)
        1000,   // Premio apos Pergunta 1
        2000,   // Premio apos Pergunta 2
        3000,   // Premio apos Pergunta 3
        4000,   // Premio apos Pergunta 4
        5000,   // Premio apos Pergunta 5 (1o. Marco de Seguranca)
        10000,  // Premio apos Pergunta 6
        20000,  // Premio apos Pergunta 7
        30000,  // Premio apos Pergunta 8
        40000,  // Premio apos Pergunta 9
        50000,  // Premio apos Pergunta 10 (2o. Marco de Seguranca)
        100000, // Premio apos Pergunta 11
        200000, // Premio apos Pergunta 12
        300000, // Premio apos Pergunta 13
        400000, // Premio apos Pergunta 14
        1000000 // Premio apos Pergunta 15 (O Grande Milhao)
    };

    // A função `carregarcsv` alocará a memória necessária para `bancoperguntas`
    // e preencherá esse array com os dados do arquivo.
    qtdperguntascarregadas = carregarcsv("../projetobase.csv", &bancoperguntas);

    // Verifica se conseguimos carregar alguma pergunta do arquivo.
    if (qtdperguntascarregadas > 0)
    {
        printf("\nAs perguntas foram carregadas com sucesso.\n");
        printf("Temos %d perguntas prontas no nosso banco.\n\n", qtdperguntascarregadas);

        // Chama a função que inicia e gerencia todo o jogo "Show do Milhão".
        // Passamos a ela o array de perguntas, o número total delas e a tabela de prêmios.
        iniciarjogo(bancoperguntas, qtdperguntascarregadas, premios);
    }
    else
    {
        printf("Nao foi possivel iniciar o quiz. Por favor, verifique se o arquivo 'projetobase.csv' esta correto e com perguntas validas.\n");
    }

    liberarmemoria(bancoperguntas);

    printf("\nObrigado por jogar! Volte sempre e treine seu conhecimento!\n");

    return 0; // Indica que o programa terminou com sucesso.
}
    printf("\nObrigado por jogar! Volte sempre e treine seu conhecimento!\n");

    return 0; // Indica que o programa terminou com sucesso.
}
