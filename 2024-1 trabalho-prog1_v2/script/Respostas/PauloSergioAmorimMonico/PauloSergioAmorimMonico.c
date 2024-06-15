#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define ARQUIVO_MAPA "mapa.txt"
#define ARQUIVO_INIMIGO "inimigo.txt"
#define ARQUIVO_INICIALIZACAO "./saida/inicializacao.txt"
#define ARQUIVO_ENTRADA "entrada.txt"
#define TAMANHO_MAX_LINHAS_CAMPO 42
#define TAMANHO_MAX_COLUNAS_CAMPO 102
#define TAMANHO_MAX_DIRETORIO 1000
#define QUANTIDADE_MAX_INIMIGOS 20
#define DIRECAO_DIREITA 1
#define DIRECAO_ESQUERDA -1

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
tPartida realizarJogo(char const diretorio[], tPartida partida);
tPartida realizarJogada(char acao, tPartida partida);
tPartida moverJogador(char acao, tPartida partida);
tPartida moverInimigos(tPartida partida);
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
int algumInimigoUltrapassouOJogador(tPartida partida);
int eAcaoValida(char acao);
int atingiuInimigo(tPartida partida);
void salvarInicializacao(char const diretorio[], tPartida partida);
void imprimirMapa(tMapa mapa);

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        printf("ERRO: Informe o diretorio com os arquivos de configuracao.");
        return 1;
    }
    char diretorio[TAMANHO_MAX_DIRETORIO];
    strcpy(diretorio, argv[1]);
    tPartida partida = criarPartida(diretorio);
    salvarInicializacao(diretorio, partida);
    partida = realizarJogo(diretorio, partida);

    return 0;
}

tPartida criarMapa(char const diretorio[], tPartida partida)
{
    FILE *pMapaFile;
    char filePath[TAMANHO_MAX_DIRETORIO + strlen(ARQUIVO_MAPA)];
    sprintf(filePath, "%s/%s", diretorio, ARQUIVO_MAPA);

    pMapaFile = fopen(filePath, "r");

    fscanf(pMapaFile, "%d %d", &partida.mapa.c, &partida.mapa.l);
    partida.mapa.l += 2;
    partida.mapa.c += 2;

    partida = limparMapa(partida);

    fclose(pMapaFile);

    return partida;
}

tPartida criarJogador(char const diretorio[], tPartida partida)
{
    FILE *pMapaFile;
    char filePath[TAMANHO_MAX_DIRETORIO + strlen(ARQUIVO_MAPA)];
    sprintf(filePath, "%s/%s", diretorio, ARQUIVO_MAPA);
    pMapaFile = fopen(filePath, "r");
    fscanf(pMapaFile, "%*[^\n]%*c(%d %d)", &partida.jogador.j, &partida.jogador.i);
    fclose(pMapaFile);

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
    FILE *pMapaFile;
    char filePath[TAMANHO_MAX_DIRETORIO + strlen(ARQUIVO_MAPA)];
    sprintf(filePath, "%s/%s", diretorio, ARQUIVO_MAPA);
    pMapaFile = fopen(filePath, "r");
    fscanf(pMapaFile, "%*[^\n]\n%*[^\n]\n");

    tInimigo inimigo;
    for (int i = 0; fscanf(pMapaFile, "(%d %d) ", &inimigo.j, &inimigo.i) == 2; i++)
    {
        inimigo.id = i + 1;
        inimigo.vivo = 1;
        partida.inimigos[i] = inimigo;
        partida.qtdInimigos++;
    }

    fclose(pMapaFile);

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

tPartida realizarJogo(char const diretorio[], tPartida partida)
{
    FILE *pEntradaFile;
    char filePath[TAMANHO_MAX_DIRETORIO + strlen(ARQUIVO_ENTRADA)];
    sprintf(filePath, "%s/%s", diretorio, ARQUIVO_ENTRADA);
    pEntradaFile = fopen(filePath, "r");
    char acao = 0;
    int indiceInimigoAtingido = -1;

    for (int i = 0; 1; i++)
    {
        printf("Pontos: %d | Iteracoes: %d\n", partida.pontos, i);
        imprimirMapa(partida.mapa);
        if (!algumInimigoVivo(partida))
        {
            printf("Parabéns, você ganhou!\n");
            return partida;
        }
        if (algumInimigoUltrapassouOJogador(partida))
        {
            printf("Você perdeu, tente novamente!\n");
            return partida;
        }
        if ((indiceInimigoAtingido = atingiuInimigo(partida)) != -1)
        {
            partida = atribuirPontos(partida, indiceInimigoAtingido);
            partida = destruirInimigo(partida, indiceInimigoAtingido);
            partida = destruirTiro(partida);
        }
        acao = fgetc(pEntradaFile);
        partida = moverTiro(partida);
        partida = realizarJogada(acao, partida);
        partida = moverInimigos(partida);
        partida = limparMapa(partida);
        partida = desenharJogador(partida);
        partida = desenharInimigos(diretorio, partida);
        partida = desenharTiro(partida);
        while (acao != '\n')
            acao = fgetc(pEntradaFile);
    }

    return partida;
}

tPartida realizarJogada(char acao, tPartida partida)
{
    if (!eAcaoValida(acao))
        return partida;

    if (acao == 's')
        return partida;

    if (acao == ' ')
        return dispararTiro(partida);

    return moverJogador(acao, partida);
}

tPartida moverJogador(char acao, tPartida partida)
{
    if (acao == 'a')
    {
        if (eBordaVertical(partida.jogador.j - 2, partida.mapa))
            return partida;
        partida.jogador.j--;
        return partida;
    }
    else if (acao == 'd')
    {
        if (eBordaVertical(partida.jogador.j + 2, partida.mapa))
            return partida;
        partida.jogador.j++;
        return partida;
    }

    return partida;
}

tPartida moverInimigos(tPartida partida)
{
    for (int i = 0; i < partida.qtdInimigos; i++)
    {
        if (!estaVivo(partida.inimigos[i]))
            continue;
        if (partida.direcaoInimigos == DIRECAO_DIREITA)
        {
            if (eBordaVertical(partida.inimigos[i].j + 2, partida.mapa))
            {
                for (int j = 0; j < partida.qtdInimigos; j++)
                {
                    if (!estaVivo(partida.inimigos[i]))
                        continue;
                    partida.inimigos[j].i++;
                }
                partida.direcaoInimigos = DIRECAO_ESQUERDA;
                return partida;
            }
        }
        if (partida.direcaoInimigos == DIRECAO_ESQUERDA)
        {
            if (eBordaVertical(partida.inimigos[i].j - 2, partida.mapa))
            {
                for (int j = 0; j < partida.qtdInimigos; j++)
                {
                    if (!estaVivo(partida.inimigos[i]))
                        continue;
                    partida.inimigos[j].i++;
                }
                partida.direcaoInimigos = DIRECAO_DIREITA;
                return partida;
            }
        }
    }

    for (int i = 0; i < partida.qtdInimigos; i++)
        partida.inimigos[i].j += partida.direcaoInimigos;

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

int algumInimigoUltrapassouOJogador(tPartida partida)
{
    for (int i = 0; i < partida.qtdInimigos; i++)
        if (estaVivo(partida.inimigos[i]) && partida.inimigos[i].i + 1 == partida.jogador.i - 1)
            return 1;
    return 0;
}
