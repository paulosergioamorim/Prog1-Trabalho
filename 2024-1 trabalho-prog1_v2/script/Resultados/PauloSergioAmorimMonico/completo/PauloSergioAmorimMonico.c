#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARQUIVO_MAPA "mapa.txt"
#define ARQUIVO_INIMIGO "inimigo.txt"
#define ARQUIVO_INICIALIZACAO "./saida/inicializacao.txt"
#define ARQUIVO_RESUMO "./saida/resumo.txt"
#define ARQUIVO_ENTRADA "entrada.txt"
#define TAMANHO_MAX_LINHAS_CAMPO 42   // linhas + 2 bordas
#define TAMANHO_MAX_COLUNAS_CAMPO 102 // colunas + 2 bordas
#define TAMANHO_MAX_DIRETORIO 1001    // diretorio + '/'
#define QUANTIDADE_MAX_INIMIGOS 20
#define DIRECAO_DIREITA 1
#define DIRECAO_ESQUERDA -1
#define RESUMO_INIMIGO_COLIDIU_DIREITA 0
#define RESUMO_INIMIGO_COLIDIU_ESQUERDA 1
#define RESUMO_INIMIGO_ATINGIDO 2
#define RESUMO_JOGADOR_COLIDIU_ESQUERDA 3
#define RESUMO_JOGADOR_COLIDIU_DIREITA 4
#define RODAR_AUTOMATICAMENTE 1
#define DIRETORIO_MANUAL "./teste"

// typedef struct Point {
//     int x;
//     int y;
// } point;

// typedef struct Player {
//     point point;
// } player;

// typedef struct Enemy {
//     point point;
//     int alive;
// } enemy;

// typedef struct Shot {
//     point point;
//     int active;
// } shot;

// typedef struct Map {
//     int l;
//     int c;
//     char **values;
// } map;

// typedef struct EnemyDraw {
//     int animate;
//     char ***values;
// } enemy_draw;

// typedef struct GameDetails {
//     enemy_draw enemyDraw;
//     int enemiesDirection;
//     int points;
// } game_details;

// typedef struct Game {
//     map map;
//     player player;
//     enemy *enemies;
//     int enemiesCount;
//     game_details details;
// } game;

typedef struct
{
    int i;
    int j;
    int ativo;
} tTiro;

typedef struct
{
    int i;
    int j;
} tJogador;

typedef struct
{
    int id;
    int fileira;
    int i;
    int j;
    int vivo;
} tInimigo;

typedef struct
{
    int l;
    int c;
    char campo[TAMANHO_MAX_LINHAS_CAMPO][TAMANHO_MAX_COLUNAS_CAMPO];
} tMapa;

typedef struct
{
    tMapa mapa;
    tInimigo inimigos[QUANTIDADE_MAX_INIMIGOS];
    tJogador jogador;
    tTiro tiro;
    int jogada;
    int qtdInimigos;
    int direcaoInimigos;
    int animarInimigo;
    int animacaoAtual;
    char desenhoInimigo[3][3][3];
    int pontos;
} tPartida;

tPartida criarPartida(char const diretorio[]);
tPartida limparMapa(tPartida partida);
tPartida criarMapa(char const diretorio[], tPartida partida);
tPartida criarJogador(char const diretorio[], tPartida partida);
tPartida desenharInimigos(char const diretorio[], tPartida partida);
tPartida criarInimigos(char const diretorio[], tPartida partida);
tPartida desenharJogador(tPartida partida);
tPartida realizarPartida(char const diretorio[], tPartida partida);
tPartida realizarAcao(char const diretorio[], char acao, tPartida partida);
tPartida moverJogador(char const diretorio[], char acao, tPartida partida);
tPartida moverInimigos(char const diretorio[], tPartida partida);
tPartida moverTiro(tPartida partida);
tPartida dispararTiro(tPartida partida);
tPartida desenharTiro(tPartida partida);
tPartida destruirInimigo(tPartida partida, int indiceInimigo);
tPartida destruirTiro(tPartida partida);
tPartida atribuirPontos(tPartida partida, int indiceInimigo);
int eBorda(int i, int j, tMapa mapa);
int eBordaVertical(int j, tMapa mapa);
int eBordaHorizontal(int i, tMapa mapa);
int eCanto(int i, int j, tMapa mapa);
int estaVivo(tInimigo inimigo);
int algumInimigoVivo(tPartida partida);
int algumInimigoUltrapassouLinhaLimite(tPartida partida);
int eAcaoValida(char acao);
int atingiuInimigo(tPartida partida);
int tocandoBordaEsquerda(int j, tMapa mapa);
int tocandoBordaDireita(int j, tMapa mapa);
void salvarInicializacao(char const diretorio[], tPartida partida);
void salvarResumoInimigo(char const diretorio[], tPartida partida, int evento, tInimigo inimigo);
void salvarResumoJogador(char const diretorio[], tPartida partida, int evento);
void imprimirMapa(tMapa mapa);

int main(int argc, char const *argv[])
{
    char diretorio[TAMANHO_MAX_DIRETORIO];
#if RODAR_AUTOMATICAMENTE
    if (argc < 2)
    {
        printf("ERRO: Informe o diretorio com os arquivos de configuracao.");
        return 1;
    }
    strcpy(diretorio, argv[1]);
#else
    strcpy(diretorio, DIRETORIO_MANUAL);
#endif
    tPartida partida = criarPartida(diretorio);
    salvarInicializacao(diretorio, partida);
    partida = realizarPartida(diretorio, partida);

    return 0;
}

tPartida criarMapa(char const diretorio[], tPartida partida)
{
    FILE *pFile;
    char filePath[TAMANHO_MAX_DIRETORIO + strlen(ARQUIVO_MAPA)];
    sprintf(filePath, "%s/%s", diretorio, ARQUIVO_MAPA);

    pFile = fopen(filePath, "r");

    fscanf(pFile, "%d %d", &partida.mapa.c, &partida.mapa.l);
    partida.mapa.l += 2;
    partida.mapa.c += 2;

    partida = limparMapa(partida);

    fclose(pFile);

    return partida;
}

tPartida criarJogador(char const diretorio[], tPartida partida)
{
    FILE *pFile;
    char filePath[TAMANHO_MAX_DIRETORIO + strlen(ARQUIVO_MAPA)];
    sprintf(filePath, "%s/%s", diretorio, ARQUIVO_MAPA);
    pFile = fopen(filePath, "r");
    fscanf(pFile, "%*[^\n]%*c(%d %d)", &partida.jogador.j, &partida.jogador.i);
    fclose(pFile);

    return partida;
}

tPartida desenharJogador(tPartida partida)
{
    for (int i = partida.jogador.i - 1; i <= partida.jogador.i + 1; i++)
        for (int j = partida.jogador.j - 1; j <= partida.jogador.j + 1; j++)
            if (!(j == partida.jogador.j && i == partida.jogador.i - 1))
                partida.mapa.campo[i][j] = 'M';
    partida.mapa.campo[partida.jogador.i - 2][0] = '-';
    partida.mapa.campo[partida.jogador.i - 2][partida.mapa.c - 1] = '-';

    return partida;
}

tPartida desenharInimigos(char const diretorio[], tPartida partida)
{
    FILE *pInimigoFile;
    char filePath[TAMANHO_MAX_DIRETORIO + strlen(ARQUIVO_INIMIGO)];
    sprintf(filePath, "%s/%s", diretorio, ARQUIVO_INIMIGO);

    pInimigoFile = fopen(filePath, "r");

    fscanf(pInimigoFile, "%d%*c", &partida.animarInimigo);
    for (int k = 0; k < 3; k++)
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
            {
                fscanf(pInimigoFile, "%c", &partida.desenhoInimigo[i][j][k]);
                if (j == 2)
                    fscanf(pInimigoFile, "%*c");
            }

    for (int k = 0; k < partida.qtdInimigos; k++)
    {
        tInimigo inimigo = partida.inimigos[k];
        if (!estaVivo(inimigo))
            continue;
        for (int i = inimigo.i - 1; i <= inimigo.i + 1; i++)
        {
            for (int j = inimigo.j - 1; j <= inimigo.j + 1; j++)
                partida.mapa.campo[i][j] = partida.desenhoInimigo[i - (inimigo.i - 1)][j - (inimigo.j - 1)][partida.animacaoAtual];
        }
    }

    if (partida.animarInimigo)
    {
        partida.animacaoAtual++;
        if (partida.animacaoAtual == 3)
            partida.animacaoAtual = 0;
    }

    fclose(pInimigoFile);

    return partida;
}

tPartida criarInimigos(char const diretorio[], tPartida partida)
{
    FILE *pFile;
    char filePath[TAMANHO_MAX_DIRETORIO + strlen(ARQUIVO_MAPA)];
    sprintf(filePath, "%s/%s", diretorio, ARQUIVO_MAPA);
    pFile = fopen(filePath, "r");
    fscanf(pFile, "%*[^\n]\n%*[^\n]\n");

    char lixo = 0;
    tInimigo inimigo;
    inimigo.fileira = 1;
    inimigo.vivo = 1;
    inimigo.id = 1;
    for (int i = 0; !feof(pFile); inimigo.id++, partida.qtdInimigos++)
    {
        if (i > 0)
        {
            fscanf(pFile, "%c", &lixo);
            if (lixo == '\n')
            {
                inimigo.fileira++;
                inimigo.id = 1;
            }
        }
        if (fscanf(pFile, "(%d %d)", &inimigo.j, &inimigo.i) == 2)
        {
            partida.inimigos[i] = inimigo;
            i++;
        }
    }

    fclose(pFile);

    return partida;
}

int eBorda(int i, int j, tMapa mapa)
{
    return eBordaHorizontal(i, mapa) || eBordaVertical(j, mapa);
}

int eBordaVertical(int j, tMapa mapa)
{
    return j == 0 || j == mapa.c - 1;
}

int eBordaHorizontal(int i, tMapa mapa)
{
    return i == 0 || i == mapa.l - 1;
}

int eCanto(int i, int j, tMapa mapa)
{
    return eBordaHorizontal(i, mapa) && eBordaVertical(j, mapa);
}

void imprimirMapa(tMapa mapa)
{
    for (int i = 0; i < mapa.l; i++)
        for (int j = 0; j < mapa.c; j++)
        {
            printf("%c", mapa.campo[i][j]);
            if (j == mapa.c - 1)
                printf("\n");
        }
}

void salvarInicializacao(char const diretorio[], tPartida partida)
{
    FILE *pFile;
    char filePath[TAMANHO_MAX_DIRETORIO + strlen(ARQUIVO_INICIALIZACAO)];
    sprintf(filePath, "%s/%s", diretorio, ARQUIVO_INICIALIZACAO);
    pFile = fopen(filePath, "w+");

    for (int i = 0; i < partida.mapa.l; i++)
        for (int j = 0; j < partida.mapa.c; j++)
        {
            fprintf(pFile, "%c", partida.mapa.campo[i][j]);
            if (j == partida.mapa.c - 1)
                fprintf(pFile, "\n");
        }
    fprintf(pFile, "A posicao central do jogador iniciara em (%d %d).", partida.jogador.j, partida.jogador.i);
    fclose(pFile);
}

tPartida criarPartida(char const diretorio[])
{
    tPartida partida;

    partida.direcaoInimigos = DIRECAO_DIREITA;
    partida.animacaoAtual = 0;
    partida.pontos = 0;

    partida = criarMapa(diretorio, partida);
    partida = criarJogador(diretorio, partida);
    partida = desenharJogador(partida);
    partida = criarInimigos(diretorio, partida);
    partida = desenharInimigos(diretorio, partida);
    partida = destruirTiro(partida);

    return partida;
}

int estaVivo(tInimigo inimigo)
{
    return inimigo.vivo;
}

tPartida realizarPartida(char const diretorio[], tPartida partida)
{
    FILE *pEntradaFile;
    char filePath[TAMANHO_MAX_DIRETORIO + strlen(ARQUIVO_ENTRADA)];
    sprintf(filePath, "%s/%s", diretorio, ARQUIVO_ENTRADA);
    pEntradaFile = fopen(filePath, "r");
    char acao = 0;
    int idInimigoAtingido = -1;

    for (partida.jogada = 0; 1; partida.jogada++)
    {
        printf("Pontos: %d | Iteracoes: %d\n", partida.pontos, partida.jogada);
        imprimirMapa(partida.mapa);
        if (!algumInimigoVivo(partida))
        {
            printf("Parabéns, você ganhou!\n");
            return partida;
        }
        if (algumInimigoUltrapassouLinhaLimite(partida))
        {
            printf("Você perdeu, tente novamente!\n");
            return partida;
        }
        if ((idInimigoAtingido = atingiuInimigo(partida)) != -1)
        {
            partida = atribuirPontos(partida, idInimigoAtingido);
            salvarResumoInimigo(diretorio, partida, RESUMO_INIMIGO_ATINGIDO, partida.inimigos[idInimigoAtingido]);
            partida = destruirInimigo(partida, idInimigoAtingido);
            partida = destruirTiro(partida);
        }
        while ((acao = fgetc(pEntradaFile)) == '\n')
            ;
        partida = moverInimigos(diretorio, partida);
        partida = moverTiro(partida);
        partida = realizarAcao(diretorio, acao, partida);
        partida = limparMapa(partida);
        partida = desenharJogador(partida);
        partida = desenharInimigos(diretorio, partida);
        partida = desenharTiro(partida);
    }

    return partida;
}

tPartida realizarAcao(char const diretorio[], char acao, tPartida partida)
{
    if (!eAcaoValida(acao))
        return partida;

    if (acao == 's')
        return partida;

    if (acao == ' ')
        return dispararTiro(partida);

    return moverJogador(diretorio, acao, partida);
}

tPartida moverJogador(char const diretorio[], char acao, tPartida partida)
{
    if (acao == 'a')
    {
        if (tocandoBordaEsquerda(partida.jogador.j, partida.mapa))
        {
            salvarResumoJogador(diretorio, partida, RESUMO_JOGADOR_COLIDIU_ESQUERDA);
            return partida;
        }
        partida.jogador.j--;
        return partida;
    }
    else if (acao == 'd')
    {
        if (tocandoBordaDireita(partida.jogador.j, partida.mapa))
        {
            salvarResumoJogador(diretorio, partida, RESUMO_JOGADOR_COLIDIU_DIREITA);
            return partida;
        }
        partida.jogador.j++;
        return partida;
    }

    return partida;
}

tPartida moverInimigos(char const diretorio[], tPartida partida)
{
    for (int i = 0; i < partida.qtdInimigos; i++)
    {
        if (!estaVivo(partida.inimigos[i]))
            continue;
        if (partida.direcaoInimigos == DIRECAO_DIREITA && tocandoBordaDireita(partida.inimigos[i].j, partida.mapa))
        {
            for (int j = 0; j < partida.qtdInimigos; j++)
            {
                if (!estaVivo(partida.inimigos[j]))
                    continue;
                partida.inimigos[j].i++;
            }
            partida.direcaoInimigos = DIRECAO_ESQUERDA;
            return partida;
        }
        if (partida.direcaoInimigos == DIRECAO_ESQUERDA && tocandoBordaEsquerda(partida.inimigos[i].j, partida.mapa))
        {
            for (int j = 0; j < partida.qtdInimigos; j++)
            {
                if (!estaVivo(partida.inimigos[j]))
                    continue;
                partida.inimigos[j].i++;
            }
            partida.direcaoInimigos = DIRECAO_DIREITA;
            return partida;
        }
    }

    for (int i = 0; i < partida.qtdInimigos; i++)
    {
        if (estaVivo(partida.inimigos[i]))
        {
            partida.inimigos[i].j += partida.direcaoInimigos;
            if (tocandoBordaDireita(partida.inimigos[i].j, partida.mapa))
                salvarResumoInimigo(diretorio, partida, RESUMO_INIMIGO_COLIDIU_DIREITA, partida.inimigos[i]);
            if (tocandoBordaEsquerda(partida.inimigos[i].j, partida.mapa))
                salvarResumoInimigo(diretorio, partida, RESUMO_INIMIGO_COLIDIU_ESQUERDA, partida.inimigos[i]);
        }
    }

    return partida;
}

tPartida moverTiro(tPartida partida)
{
    if (!partida.tiro.ativo)
        return partida;
    partida.tiro.i--;
    if (eBordaHorizontal(partida.tiro.i, partida.mapa))
        partida = destruirTiro(partida);
    return partida;
}

tPartida dispararTiro(tPartida partida)
{
    if (partida.tiro.ativo)
        return partida;

    partida.tiro.i = partida.jogador.i - 2;
    partida.tiro.j = partida.jogador.j;
    partida.tiro.ativo = 1;
    return partida;
}

tPartida desenharTiro(tPartida partida)
{
    if (!partida.tiro.ativo)
        return partida;
    partida.mapa.campo[partida.tiro.i][partida.tiro.j] = 'o';
    return partida;
}

tPartida destruirInimigo(tPartida partida, int indiceInimigo)
{
    partida.inimigos[indiceInimigo].i = 0;
    partida.inimigos[indiceInimigo].j = 0;
    partida.inimigos[indiceInimigo].vivo = 0;
    return partida;
}

tPartida destruirTiro(tPartida partida)
{
    partida.tiro.i = 0;
    partida.tiro.j = 0;
    partida.tiro.ativo = 0;
    return partida;
}

int algumInimigoVivo(tPartida partida)
{
    for (int i = 0; i < partida.qtdInimigos; i++)
        if (estaVivo(partida.inimigos[i]))
            return 1;
    return 0;
}

int eAcaoValida(char acao)
{
    return acao == 'a' || acao == 'd' || acao == 's' || acao == ' ';
}

int atingiuInimigo(tPartida partida)
{
    for (int k = 0; k < partida.qtdInimigos; k++)
    {
        tInimigo inimigo = partida.inimigos[k];
        if (!estaVivo(inimigo))
            continue;
        for (int i = inimigo.i - 1; i <= inimigo.i + 1; i++)
            for (int j = inimigo.j - 1; j <= inimigo.j + 1; j++)
                if (i == partida.tiro.i && j == partida.tiro.j)
                    return k;
    }

    return -1;
}

tPartida atribuirPontos(tPartida partida, int indiceInimigo)
{
    partida.pontos += (partida.mapa.l - partida.inimigos[indiceInimigo].i - 2) * partida.inimigos[indiceInimigo].j;
    return partida;
}

tPartida limparMapa(tPartida partida)
{
    for (int i = 0; i < partida.mapa.l; i++)
        for (int j = 0; j < partida.mapa.c; j++)
        {
            if (eCanto(i, j, partida.mapa))
                partida.mapa.campo[i][j] = '+';
            else if (eBordaVertical(j, partida.mapa))
                partida.mapa.campo[i][j] = '|';
            else if (eBordaHorizontal(i, partida.mapa))
                partida.mapa.campo[i][j] = '-';
            else
                partida.mapa.campo[i][j] = ' ';
        }

    return partida;
}

int algumInimigoUltrapassouLinhaLimite(tPartida partida)
{
    for (int i = 0; i < partida.qtdInimigos; i++)
        if (estaVivo(partida.inimigos[i]) && partida.inimigos[i].i + 1 == partida.jogador.i - 1)
            return 1;
    return 0;
}

int tocandoBordaEsquerda(int j, tMapa mapa)
{
    return eBordaVertical(j - 2, mapa);
}

int tocandoBordaDireita(int j, tMapa mapa)
{
    return eBordaVertical(j + 2, mapa);
}

void salvarResumoInimigo(char const diretorio[], tPartida partida, int evento, tInimigo inimigo)
{
    FILE *pFile;
    char filePath[TAMANHO_MAX_DIRETORIO + strlen(ARQUIVO_RESUMO)];
    sprintf(filePath, "%s/%s", diretorio, ARQUIVO_RESUMO);
    pFile = fopen(filePath, "a+");

    if (evento == RESUMO_INIMIGO_COLIDIU_DIREITA)
        fprintf(pFile, "[%d] Inimigo de indice %d da fileira %d colidiu na lateral direita.\n", partida.jogada, inimigo.id, inimigo.fileira);
    else if (evento == RESUMO_INIMIGO_COLIDIU_ESQUERDA)
        fprintf(pFile, "[%d] Inimigo de indice %d da fileira %d colidiu na lateral esquerda.\n", partida.jogada, inimigo.id, inimigo.fileira);
    else if (evento == RESUMO_INIMIGO_ATINGIDO)
        fprintf(pFile, "[%d] Inimigo de indice %d da fileira %d foi atingido na posicao (%d %d).\n", partida.jogada, inimigo.id, inimigo.fileira, partida.tiro.j, partida.tiro.i);

    fclose(pFile);
}

void salvarResumoJogador(char const diretorio[], tPartida partida, int evento)
{
    FILE *pFile;
    char filePath[TAMANHO_MAX_DIRETORIO + strlen(ARQUIVO_RESUMO)];
    sprintf(filePath, "%s/%s", diretorio, ARQUIVO_RESUMO);
    pFile = fopen(filePath, "a+");

    if (evento == RESUMO_JOGADOR_COLIDIU_DIREITA)
        fprintf(pFile, "[%d] Jogador colidiu na lateral direita.\n", partida.jogada + 1);
    else if (evento == RESUMO_JOGADOR_COLIDIU_ESQUERDA)
        fprintf(pFile, "[%d] Jogador colidiu na lateral esquerda.\n", partida.jogada + 1);

    fclose(pFile);
}
