#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h> // necessário para ceil

// --- configurações globais ---
static const int telaLargura = 1024;
static const int telaAltura = 768;
static const int perguntaLargura = 800;
static const int perguntaAltura = 150;
static const int altLargura = 800;
static const int altAltura = 60;
static const int altEspaco = 20;
static const int poderSize = 50;
static const int poderGap = 20;
static const float tempoInicial = 11.0f;
static const float duracaoTrans = 1.0f; // segundos de fade

// recompensas por questão (1..16)
static const int recompensaAcerto[] = {
    0, 1000, 2000, 3000, 4000, 5000,
    10000, 20000, 30000, 40000, 50000,
    100000, 200000, 300000, 400000, 500000, 1000000};
static const int recompensaGarantia[] = {
    0, 0, 1000, 2000, 3000, 4000,
    5000, 10000, 20000, 30000, 40000,
    50000, 100000, 200000, 300000, 400000, 500000};
static const int recompensaErro[] = {
    0, 0, 500, 1000, 1500, 2000,
    2500, 5000, 10000, 15000, 20000,
    25000, 50000, 100000, 150000, 200000, 0};

// --- definição de uma pergunta ---
typedef struct
{
    char enunciado[512];
    char alternativaA[256];
    char alternativaB[256];
    char alternativaC[256];
    char alternativaD[256];
    char correta; // 'A'..'D'
    int nivel;    // 1..5
    int jaUsada;  // flag
} Pergunta;

// remove CR/LF do fim
static void removeFimLinha(char *s)
{
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r'))
        s[--len] = '\0';
}

// conta linhas úteis no CSV (exceto cabeçalho)
static int contaLinhasCsv(const char *nomeArq)
{
    FILE *f = fopen(nomeArq, "r");
    if (!f)
        return 0;
    char buf[2048];
    int cnt = 0;
    // pula cabeçalho
    if (!fgets(buf, sizeof(buf), f))
    {
        fclose(f);
        return 0;
    }
    while (fgets(buf, sizeof(buf), f))
    {
        removeFimLinha(buf);
        if (buf[0] != '\0')
            cnt++;
    }
    fclose(f);
    return cnt;
}

// carrega CSV em vetor alocado. Retorna quantidade em *outTotal e ponteiro mallocado.
static Pergunta *carregarCsv(const char *nomeArq, int *outTotal)
{
    int total = contaLinhasCsv(nomeArq);
    *outTotal = total;
    if (total <= 0)
        return NULL;
    Pergunta *vet = calloc(total, sizeof(Pergunta));
    if (!vet)
        return NULL;
    FILE *f = fopen(nomeArq, "r");
    char linha[2048];
    // pula cabeçalho
    fgets(linha, sizeof(linha), f);
    int idx = 0;
    while (fgets(linha, sizeof(linha), f) && idx < total)
    {
        removeFimLinha(linha);
        if (linha[0] == '\0')
            continue;
        char *tok = strtok(linha, ",");
        if (!tok)
            continue;
        Pergunta *p = &vet[idx++];
        strncpy(p->enunciado, tok, 511);
        p->enunciado[511] = '\0';
        tok = strtok(NULL, ",");
        strncpy(p->alternativaA, tok ? tok : "", 255);
        p->alternativaA[255] = '\0';
        tok = strtok(NULL, ",");
        strncpy(p->alternativaB, tok ? tok : "", 255);
        p->alternativaB[255] = '\0';
        tok = strtok(NULL, ",");
        strncpy(p->alternativaC, tok ? tok : "", 255);
        p->alternativaC[255] = '\0';
        tok = strtok(NULL, ",");
        strncpy(p->alternativaD, tok ? tok : "", 255);
        p->alternativaD[255] = '\0';
        tok = strtok(NULL, ",");
        p->correta = tok ? toupper(tok[0]) : ' ';
        tok = strtok(NULL, ",");
        p->nivel = tok ? atoi(tok) : 1;
        p->jaUsada = 0;
    }
    fclose(f);
    return vet;
}

// escolhe pergunta de dado nível não usada
static int escolhePergunta(Pergunta *banco, int total, int nivel)
{
    int *pos = malloc(total * sizeof(int)), n = 0;
    for (int i = 0; i < total; i++)
    {
        if (!banco[i].jaUsada && banco[i].nivel == nivel)
            pos[n++] = i;
    }
    if (n == 0)
    {
        free(pos);
        return -1;
    }
    int sel = pos[rand() % n];
    banco[sel].jaUsada = 1;
    free(pos);
    return sel;
}

typedef enum
{
    EST_MENU,
    EST_JOGANDO,
    EST_CREDITOS,
    EST_TEMPO_ESGOTADO,
    EST_ERRO,
    EST_VITORIA
} Estado;

int main(void)
{
    srand(time(NULL));
    InitWindow(telaLargura, telaAltura, "Show do Milhao");
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);

    // texturas
    Texture2D texPular = LoadTexture("jump_dica.png");
    Texture2D tex5050 = LoadTexture("5050_DICA.png");
    Texture2D texJoker = LoadTexture("jocker_dica.png");
    Texture2D texJequiti = LoadTexture("jequiti.png");

    // carrega bancos dinamicamente
    int totalPerg, totalCalc;
    Pergunta *bancoPerg = carregarCsv("projetobase.csv", &totalPerg);
    Pergunta *bancoCalc = carregarCsv("math.csv", &totalCalc);
    if (!bancoPerg || !bancoCalc)
    {
        fprintf(stderr, "Erro ao abrir CSVs\n");
        return 1;
    }

    // UI rectangles
    Rectangle btnIniciar = {(telaLargura - 200) / 2, (telaAltura - 120) / 2, 200, 50};
    Rectangle btnCreds = {btnIniciar.x, btnIniciar.y + 70, 200, 50};
    Rectangle areaPerg = {(telaLargura - perguntaLargura) / 2, 120, perguntaLargura, perguntaAltura};
    Rectangle altRects[4];
    for (int i = 0; i < 4; i++)
    {
        altRects[i] = (Rectangle){
            areaPerg.x,
            areaPerg.y + perguntaAltura + 30 + i * (altAltura + altEspaco),
            altLargura, altAltura};
    }
    Rectangle poderRects[3];
    int baseX = (telaLargura - (3 * poderSize + 2 * poderGap)) / 2;
    for (int i = 0; i < 3; i++)
    {
        poderRects[i] = (Rectangle){baseX + i * (poderSize + poderGap), 10, poderSize, poderSize};
    }
    Rectangle btnCalculo = {10, 10, 100, 30};
    Rectangle btnDesistir = {telaLargura - 120, 10, 110, 30};

    Font fonte = GetFontDefault();
    Estado estado = EST_MENU;

    int numQuestao = 0;
    int idxAtual = -1;
    int dinheiro = 0;
    int garantido = 0;
    float timerRest = tempoInicial;
    bool emTransicao = false;
    float timerTrans = 0.0f;
    bool modoCalculo = false;
    bool poderUsado[3] = {false, false, false};
    bool altRemovida[4] = {false, false, false, false};

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        Vector2 mp = GetMousePosition();

        // — UPDATE —
        switch (estado)
        {
        case EST_MENU:
            if (CheckCollisionPointRec(mp, btnIniciar) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                estado = EST_JOGANDO;
                numQuestao = 1;
                dinheiro = 0;
                garantido = 0;
                timerRest = tempoInicial;
                emTransicao = false;
                modoCalculo = false;
                memset(poderUsado, 0, sizeof(poderUsado));
                memset(altRemovida, 0, sizeof(altRemovida));
                for (int i = 0; i < totalPerg; i++)
                    bancoPerg[i].jaUsada = 0;
                for (int i = 0; i < totalCalc; i++)
                    bancoCalc[i].jaUsada = 0;
                int niv = (numQuestao <= 2 ? 1 : numQuestao <= 4 ? 2
                                             : numQuestao <= 8   ? 3
                                             : numQuestao <= 12  ? 4
                                                                 : 5);
                idxAtual = escolhePergunta(bancoPerg, totalPerg, niv);
            }
            if (CheckCollisionPointRec(mp, btnCreds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                estado = EST_CREDITOS;
            }
            break;

        case EST_CREDITOS:
            if (IsKeyPressed(KEY_ESCAPE))
                estado = EST_MENU;
            break;

        case EST_JOGANDO:
        {
            Pergunta *b = modoCalculo ? bancoCalc : bancoPerg;

            // poderes em modo normal
            if (!modoCalculo)
            {
                // pular
                if (!poderUsado[0] && CheckCollisionPointRec(mp, poderRects[0]) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    poderUsado[0] = true;
                    int niv = (numQuestao <= 2 ? 1 : numQuestao <= 4 ? 2
                                                 : numQuestao <= 8   ? 3
                                                 : numQuestao <= 12  ? 4
                                                                     : 5);
                    idxAtual = escolhePergunta(bancoPerg, totalPerg, niv);
                    timerRest = tempoInicial;
                    memset(altRemovida, 0, sizeof(altRemovida));
                }
                // 50/50
                if (!poderUsado[1] && CheckCollisionPointRec(mp, poderRects[1]) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    poderUsado[1] = true;
                    int errs[3], n = 0;
                    for (int i = 0; i < 4; i++)
                        if ('A' + i != bancoPerg[idxAtual].correta)
                            errs[n++] = i;
                    for (int k = n - 1; k > 1; k--)
                    {
                        int r = rand() % (k + 1), t = errs[r];
                        errs[r] = errs[k];
                        errs[k] = t;
                    }
                    altRemovida[errs[0]] = altRemovida[errs[1]] = true;
                }
                // joker
                if (!poderUsado[2] && CheckCollisionPointRec(mp, poderRects[2]) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    poderUsado[2] = true;
                    int errs[3], n = 0;
                    for (int i = 0; i < 4; i++)
                        if ('A' + i != bancoPerg[idxAtual].correta)
                            errs[n++] = i;
                    int rc = rand() % (n + 1);
                    for (int k = 0; k < rc; k++)
                        altRemovida[errs[k]] = true;
                }
            }

            // cálculo em q5,10,15
            if ((numQuestao == 5 || numQuestao == 10 || numQuestao == 15) && CheckCollisionPointRec(mp, btnCalculo) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                modoCalculo = true;
                int niv = (numQuestao == 5 ? 1 : numQuestao == 10 ? 2
                                                                  : 3);
                idxAtual = escolhePergunta(bancoCalc, totalCalc, niv);
                timerRest = tempoInicial;
                emTransicao = false;
                memset(altRemovida, 0, sizeof(altRemovida));
            }

            // desistir
            if (CheckCollisionPointRec(mp, btnDesistir) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                estado = EST_VITORIA;
            }

            // escolha/tempo
            if (!emTransicao)
            {
                timerRest -= dt;
                if (timerRest <= 1.0f)
                {
                    estado = EST_TEMPO_ESGOTADO;
                }
                else
                {
                    for (int i = 0; i < 4; i++)
                    {
                        if (!altRemovida[i] && CheckCollisionPointRec(mp, altRects[i]) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                        {
                            char chute = 'A' + i;
                            if (chute == b[idxAtual].correta)
                            {
                                emTransicao = true;
                                timerTrans = 0.0f;
                                if (!modoCalculo)
                                {
                                    dinheiro = recompensaAcerto[numQuestao];
                                    garantido = recompensaGarantia[numQuestao];
                                }
                            }
                            else
                            {
                                estado = EST_ERRO;
                            }
                        }
                    }
                }
            }
            else
            {
                timerTrans += dt;
                if (timerTrans >= duracaoTrans)
                {
                    emTransicao = false;
                    timerTrans = 0.0f;
                    if (modoCalculo)
                    {
                        modoCalculo = false;
                        numQuestao++;
                    }
                    else
                    {
                        numQuestao++;
                    }
                    if (numQuestao > 16)
                    {
                        estado = EST_VITORIA;
                    }
                    else
                    {
                        int niv = (numQuestao <= 2 ? 1 : numQuestao <= 4 ? 2
                                                     : numQuestao <= 8   ? 3
                                                     : numQuestao <= 12  ? 4
                                                                         : 5);
                        idxAtual = escolhePergunta(bancoPerg, totalPerg, niv);
                        timerRest = tempoInicial;
                        memset(altRemovida, 0, sizeof(altRemovida));
                    }
                }
            }
        }
        break;

        case EST_TEMPO_ESGOTADO:
        case EST_ERRO:
        case EST_VITORIA:
            if (IsKeyPressed(KEY_ENTER))
                estado = EST_MENU;
            break;
        }

        // — DRAW —
        BeginDrawing();
        ClearBackground(DARKBLUE);

        if (estado == EST_MENU)
        {
            DrawTextEx(fonte, "SHOW DO MILHAO",
                       (Vector2){telaLargura / 2 - 240, telaAltura / 2 - 120},
                       50, 2, WHITE);
            DrawRectangleRec(btnIniciar, WHITE);
            DrawText("INICIAR", btnIniciar.x + 60, btnIniciar.y + 15, 30, BLACK);
            DrawRectangleRec(btnCreds, WHITE);
            DrawText("CREDITOS", btnCreds.x + 50, btnCreds.y + 15, 30, BLACK);
        }
        else if (estado == EST_CREDITOS)
        {
            DrawTextEx(fonte, "CREDITOS",
                       (Vector2){telaLargura / 2 - 80, 100}, 40, 2, WHITE);
            DrawText("feito por Pedro Zarranz, Raul Stucchi e Jota",
                     telaLargura / 2 - 220, telaAltura / 2 - 20, 20, WHITE);
            DrawText("ESC = voltar", telaLargura / 2 - 70, telaAltura / 2 + 40, 20, LIGHTGRAY);
        }
        else if (estado == EST_JOGANDO)
        {
            Pergunta *b = modoCalculo ? bancoCalc : bancoPerg;

            // poderes
            if (!modoCalculo)
            {
                if (!poderUsado[0])
                    DrawTexturePro(texPular, (Rectangle){0, 0, texPular.width, texPular.height}, poderRects[0], (Vector2){0, 0}, 0, WHITE);
                else
                    DrawRectangleRec(poderRects[0], DARKGRAY);
                if (!poderUsado[1])
                    DrawTexturePro(tex5050, (Rectangle){0, 0, tex5050.width, tex5050.height}, poderRects[1], (Vector2){0, 0}, 0, WHITE);
                else
                    DrawRectangleRec(poderRects[1], DARKGRAY);
                if (!poderUsado[2])
                    DrawTexturePro(texJoker, (Rectangle){0, 0, texJoker.width, texJoker.height}, poderRects[2], (Vector2){0, 0}, 0, WHITE);
                else
                    DrawRectangleRec(poderRects[2], DARKGRAY);
            }

            // cálculo
            if (numQuestao == 5 || numQuestao == 10 || numQuestao == 15)
            {
                DrawRectangleRec(btnCalculo, WHITE);
                DrawText("CALCULO", btnCalculo.x + 10, btnCalculo.y + 5, 20, BLACK);
            }
            // desistir
            DrawRectangleRec(btnDesistir, WHITE);
            DrawText("DESISTIR", btnDesistir.x + 10, btnDesistir.y + 5, 20, RED);

            // pergunta e HUD
            DrawRectangleRounded(areaPerg, 0.05f, 10, WHITE);
            DrawTextEx(fonte, TextFormat("Questao %d", numQuestao),
                       (Vector2){areaPerg.x + 10, areaPerg.y + 5}, 20, 1, BLACK);
            Vector2 ts = MeasureTextEx(fonte, b[idxAtual].enunciado, 20, 1);
            DrawTextEx(fonte, b[idxAtual].enunciado,
                       (Vector2){areaPerg.x + (perguntaLargura - ts.x) / 2,
                                 areaPerg.y + (perguntaAltura - ts.y) / 2},
                       20, 1, BLACK);
            DrawTextEx(fonte, TextFormat("Nivel: %d", b[idxAtual].nivel),
                       (Vector2){areaPerg.x + (perguntaLargura - MeasureTextEx(fonte, TextFormat("Nivel: %d", b[idxAtual].nivel), 18, 1).x) / 2,
                                 areaPerg.y + perguntaAltura - 22},
                       18, 1, GRAY);
            DrawText(TextFormat("%02d", (int)ceil(timerRest)),
                     areaPerg.x + perguntaLargura - 40, areaPerg.y + 5, 20, RED);
            if (!modoCalculo)
            {
                DrawTextEx(fonte, TextFormat("R$ %d", dinheiro),
                           (Vector2){areaPerg.x + perguntaLargura - 100, areaPerg.y + perguntaAltura - 22},
                           18, 1, GREEN);
            }

            // alternativas
            for (int i = 0; i < 4; i++)
            {
                if (altRemovida[i])
                    continue;
                bool hov = CheckCollisionPointRec(mp, altRects[i]);
                DrawRectangleRec(altRects[i], hov ? LIGHTGRAY : YELLOW);
                char buf[512];
                sprintf(buf, "%c) %s", 'A' + i,
                        (i == 0 ? b[idxAtual].alternativaA : i == 1 ? b[idxAtual].alternativaB
                                                         : i == 2   ? b[idxAtual].alternativaC
                                                                    : b[idxAtual].alternativaD));
                Vector2 sz2 = MeasureTextEx(fonte, buf, 20, 1);
                DrawTextEx(fonte, buf,
                           (Vector2){altRects[i].x + (altLargura - sz2.x) / 2,
                                     altRects[i].y + (altAltura - sz2.y) / 2},
                           20, 1, BLACK);
            }

            // transição
            if (emTransicao)
            {
                float alpha = timerTrans / duracaoTrans;
                DrawRectangle(0, 0, telaLargura, telaAltura, WHITE);
                DrawTextureEx(texJequiti, (Vector2){telaLargura / 2 - 175, telaAltura / 2 - 175}, 0, 0.15f, WHITE);
            }
        }
        else if (estado == EST_TEMPO_ESGOTADO)
        {
            DrawText("TEMPO ESGOTADO!", telaLargura / 2 - 150, telaAltura / 2 - 20, 40, RED);
            DrawText(TextFormat("Voce leva R$ %d", garantido),
                     telaLargura / 2 - 80, telaAltura / 2 + 40, 30, WHITE);
            DrawText("ENTER = menu", telaLargura / 2 - 70, telaAltura / 2 + 100, 20, GRAY);
        }
        else if (estado == EST_ERRO)
        {
            DrawText("INCORRETA!", telaLargura / 2 - 100, telaAltura / 2 - 20, 40, RED);
            DrawText(TextFormat("Voce leva R$ %d", recompensaErro[numQuestao]),
                     telaLargura / 2 - 80, telaAltura / 2 + 40, 30, WHITE);
            DrawText("ENTER = menu", telaLargura / 2 - 70, telaAltura / 2 + 100, 20, GRAY);
        }
        else
        { // VITORIA
            DrawText("PARABENS, VOCE GANHOU!", telaLargura / 2 - 200, telaAltura / 2 - 60, 40, GREEN);
            DrawText(TextFormat("Total: R$ %d", dinheiro),
                     telaLargura / 2 - 80, telaAltura / 2, 30, WHITE);
            DrawText("VOCE E FODA!", telaLargura / 2 - 90, telaAltura / 2 + 60, 30, YELLOW);
            DrawText("ENTER = menu", telaLargura / 2 - 70, telaAltura / 2 + 120, 20, GRAY);
        }

        EndDrawing();
    }

    // libera
    UnloadTexture(texPular);
    UnloadTexture(tex5050);
    UnloadTexture(texJoker);
    UnloadTexture(texJequiti);
    free(bancoPerg);
    free(bancoCalc);

    CloseWindow();
    return 0;
}