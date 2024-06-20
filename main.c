#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAP_MAX_L 40
#define MAP_MAX_C 100
#define FILE_MAP "mapa.txt"
#define FILE_ENEMY "inimigo.txt"
#define FILE_INPUT "entrada.txt"
#define FILE_INIT "./saida/inicializacao.txt"
#define FILE_RESUME "./saida/resumo.txt"
#define FILE_RANKING "./saida/ranking.txt"
#define FILE_STATISTICS "./saida/estatiscas.txt"
#define FILE_HEATMAP "./saida/heatmap.txt"
#define DIRECTION_RIGHT 1
#define DIRECTION_LEFT -1
#define NOT_HITTED -1
#define DIR_MAX_LENGTH 1000
#define ENEMIES_MAX_COUNT 100

typedef struct Player
{
    int i;
    int j;
} Player;

typedef struct Enemy
{
    int id;
    int row;
    int alive;
    int i;
    int j;
} Enemy;

typedef struct Shot
{
    int i;
    int j;
    int active;
} Shot;

typedef struct EnemyDraw
{
    int animate;
    int currentAnimation;
    char values[3][3][3];
} EnemyDraw;

typedef struct Map
{
    int l;
    int c;
    char values[MAP_MAX_L + 2][MAP_MAX_C + 2];
} Map;

typedef struct Ranking
{
    int id;
    int row;
    int line;
    int iteration;
} Ranking;

typedef struct Statistics
{
    int playerMoves;
    int effectiveShots;
    int notEffectiveShots;
    int enemiesMovesDown;
} Statistics;

typedef struct Heatmap
{
    int l;
    int c;
    int values[MAP_MAX_L][MAP_MAX_C];
} Heatmap;

typedef struct Game
{
    Map map;
    Player player;
    Shot shot;
    Enemy enemies[ENEMIES_MAX_COUNT];
    Ranking rankings[ENEMIES_MAX_COUNT];
    EnemyDraw enemyDraw;
    Heatmap heatmap;
    Statistics statistics;
    int enemiesCount;
    int enemiesDirection;
    int rankingsCount;
    int iteration;
    int points;
} Game;

FILE *createFile(char dir[], char fileName[], char modes[]);
Player createPlayer(char dir[]);
int createEnemies(char dir[], Enemy enemies[]);
EnemyDraw createEnemyDraw(char dir[]);
Game createGame(char dir[]);
Map createMap(char dir[]);
Map cleanMap(Map map);
int calculatePlayerLimit(Player player);
Map drawMatrix(Map map, int i, int j, char values[3][3]);
Map drawPlayer(Map map, int i, int j);
Map drawPlayerLimit(Map map, Player player);
EnemyDraw advanceAnimation(EnemyDraw enemyDraw);
Map drawEnemies(Map map, int enemiesCount, Enemy enemies[], EnemyDraw enemyDraw);
Map drawShot(Map map, Shot shot);
Game playGame(char dir[], Game game);
int moveEnemies(Map map, int enemiesCount, Enemy enemies[], int enemiesDirection);
void moveDownEnemies(int enemiesCount, Enemy enemies[]);
Shot moveShot(Shot shot);
Shot shootShot(Shot shot, Player player);
Player movePlayer(Map map, Player player, char action);
void printGameState(Game game);
Game updateGameState(Game game);
void printMap(Map map);
int isAlive(Enemy enemy);
int someEnemyAlive(int enemiesCount, Enemy enemies[]);
int someEnemyCrossPlayerLimit(int enemiesCount, Enemy enemies[], Player player);
int someEnemyHitted(int enemiesCount, Enemy enemies[], Shot shot);
int calculatePoints(Map map, Enemy enemy);
int isLeftBorder(int j);
int isRightBorder(Map map, int j);
int isHorizontalBorder(Map map, int i);
int isVerticalBorder(Map map, int j);
int isBorder(Map map, int i, int j);
int isCorner(Map map, int i, int j);
int isTouchingLeftBorder(int j);
int isTouchingRightBorder(Map map, int j);
int isValidAction(char action);
void killEnemy(Enemy enemies[], int id);
Shot disableShot(Shot shot);
void generateInitialization(char dir[], Map map, Player player);

int main(int argc, char const *argv[])
{
    char dir[DIR_MAX_LENGTH];
    if (argc < 2)
    {
        printf("ERRO: Informe o diretorio com os arquivos de configuracao.");
        return 1;
    }
    strcpy(dir, argv[1]);
    Game game = createGame(dir);
    return 0;
}

FILE *createFile(char dir[], char fileName[], char modes[])
{
    FILE *pFile;
    char filePath[DIR_MAX_LENGTH + strlen(fileName) + 1];
    sprintf(filePath, "%s/%s", dir, fileName);
    pFile = fopen(filePath, modes);
    if (!pFile)
    {
        printf("ERRO! Não foi possivel ler o arquivo %s", filePath);
        exit(1);
    }
    return pFile;
}

Map createMap(char dir[])
{
    Map map;
    FILE *pFile = createFile(dir, FILE_MAP, "r");
    fscanf(pFile, "%d %d", &map.c, &map.l);
    map.l += 2;
    map.c += 2;
    fclose(pFile);
    return map;
}

Map cleanMap(Map map)
{
    for (int i = 0; i < map.l; i++)
    {
        for (int j = 0; j < map.c; j++)
        {
            if (isCorner(map, i, j))
            {
                map.values[i][j] = '+';
            }
            else if (isHorizontalBorder(map, j))
            {
                map.values[i][j] = '-';
            }
            else if (isVerticalBorder(map, i))
            {
                map.values[i][j] = '|';
            }
            else
            {
                map.values[i][j] = ' ';
            }
        }
    }
    return map;
}

Player createPlayer(char dir[])
{
    Player player;
    FILE *pFile = createFile(dir, FILE_MAP, "r");
    fscanf(pFile, "%*[^\n]%*c");
    fscanf(pFile, &player.j, &player.i);
    fclose(pFile);
    return player;
}

int createEnemies(char dir[], Enemy enemies[])
{
    FILE *pFile = createFile(dir, FILE_MAP, "r");
    fscanf(pFile, "%*[^\n]%*c%*[^\n]%*c");
    int enemiesCount = 0;
    char c = 0;
    Enemy enemy;
    enemy.id = 1;
    enemy.row = 1;
    enemy.alive = 1;
    while (!feof(pFile))
    {
        fscanf(pFile, "%c", &c);
        if (c == '\n')
        {
            enemy.row++;
            enemy.id++;
        }
        if (fscanf(pFile, (enemiesCount == 0 && c == '(') ? "%d %d)" : "(%d %d)"), &enemy.j, &enemy.i)
        {
            enemies[enemiesCount] = enemy;
            enemiesCount++;
        }
    }
    fclose(pFile);
    return enemiesCount;
}

EnemyDraw createEnemyDraw(char dir[])
{
    EnemyDraw enemyDraw;
    enemyDraw.currentAnimation = 0;
    FILE *pFile = createFile(dir, FILE_ENEMY, "r");
    fscanf(pFile, "%d%*c", &enemyDraw.animate);
    for (int k = 0; k < 3; k++)
    {
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                fscanf(pFile, "%c", &enemyDraw.values[i][j][k]);
                if (j == 2)
                {
                    fscanf(pFile, "%*c");
                }
            }
        }
    }
    fclose(pFile);
    return enemyDraw;
}

Game createGame(char dir[])
{
    Game game;
    Player player;
    game.map = createMap(dir);
    game.enemyDraw = createEnemyDraw(dir);
    game.enemiesCount = createEnemies(dir, game.enemies);
    game.rankingsCount = 0;
    game.points = 0;
    player = createPlayer(dir);
    game = updateGameState(game);
    return game;
}

int isAlive(Enemy enemy)
{
    return enemy.alive;
}

int someEnemyAlive(int enemiesCount, Enemy enemies[])
{
    for (int i = 0; i < enemiesCount; i++)
    {
        if (isAlive(enemies[i]))
        {
            return 1;
        }
    }
    return 0;
}

int someEnemyCrossPlayerLimit(int enemiesCount, Enemy enemies[], Player player)
{
    for (int i = 0; i < enemiesCount; i++)
    {
        Enemy enemy = enemies[i];
        if (!isAlive(enemy))
        {
            continue;
        }
        if (enemy.i == calculatePlayerLimit(player))
        {
            return 1;
        }
    }
    return 0;
}

int someEnemyHitted(int enemiesCount, Enemy enemies[], Shot shot)
{
    if (!shot.active)
        return NOT_HITTED;
    for (int id = 0; id < enemiesCount; id++)
    {
        Enemy enemy = enemies[id];
        if (!isAlive(enemy))
        {
            continue;
        }
        for (int i = enemy.i - 1; i <= enemy.i + 1; i++)
        {
            for (int j = enemy.j - 1; j <= enemy.j + 1; j++)
            {
                if (i == shot.i && j == shot.j)
                    return id;
            }
        }
    }
    return NOT_HITTED;
}

int calculatePoints(Map map, Enemy enemy)
{
    return (map.l - enemy.i - 2) * enemy.j;
}

int isLeftBorder(int j)
{
    return j == 0;
}

int isRightBorder(Map map, int j)
{
    return j == map.c - 1;
}

int isHorizontalBorder(Map map, int i)
{
    return i == 0 || i == map.l - 1;
}

int isVerticalBorder(Map map, int j)
{
    return isLeftBorder(j) || isRightBorder(map, j);
}

int isBorder(Map map, int i, int j)
{
    return isHorizontalBorder(map, i) || isVerticalBorder(map, j);
}

int isCorner(Map map, int i, int j)
{
    return isHorizontalBorder(map, j) && isVerticalBorder(map, i);
}

int isTouchingLeftBorder(int j)
{
    return isLeftBorder(j - 2);
}

int isTouchingRightBorder(Map map, int j)
{
    return isRightBorder(map, j + 2);
}

int isValidAction(char action)
{
    switch (action)
    {
    case 'a':
    case 'd':
    case 's':
    case ' ':
        return 1;
    default:
        return 0;
    }
}

void killEnemy(Enemy enemies[], int id)
{
    enemies[id].alive = 0;
}

Shot disableShot(Shot shot)
{
    shot.active = 0;
    return shot;
}

void generateInitialization(char dir[], Map map, Player player)
{
    FILE *pFile = createFile(dir, FILE_INIT, "w+");
     for (int i = 0; i < map.l; i++)
        for (int j = 0; j < map.c; j++)
        {
            fprintf(pFile, "%c", map.values[i][j]);
            if (j == map.c - 1)
                fprintf(pFile, "\n");
        }
    fprintf(pFile, "A posicao central do jogador iniciara em (%d %d).", player.j, player.i);
    fclose(pFile);
}

int calculatePlayerLimit(Player player)
{
    return player.i - 2;
}

Map drawMatrix(Map map, int i, int j, char values[3][3])
{
    for (int a = i - 1; a <= i + 1; a++)
    {
        for (int b = j - 1; b <= j + 1; b++)
        {
            map.values[i][j] = values[a - (i - 1)][b - (j - 1)];
        }
    }
    return map;
}

Map drawPlayer(Map map, int i, int j)
{
    char playerDraw[3][3] = {{'M', ' ', 'M'}, {'M', 'M', 'M'}, {'M', 'M', 'M'}};
    map = drawMatrix(map, i, j, playerDraw);
    return map;
}

Map drawPlayerLimit(Map map, Player player)
{
    int i = calculatePlayerLimit(player);
    map.values[i][0] = '-';
    map.values[i][map.c - 1] = '-';
    return map;
}

EnemyDraw advanceAnimation(EnemyDraw enemyDraw)
{
    if (!enemyDraw.animate)
        return enemyDraw;
    enemyDraw.currentAnimation++;
    if (enemyDraw.currentAnimation == 3)
        enemyDraw.currentAnimation = 0;
    return enemyDraw;
}

Map drawEnemies(Map map, int enemiesCount, Enemy enemies[], EnemyDraw enemyDraw)
{
    for (int i = 0; i < enemiesCount; i++)
    {
        Enemy enemy = enemies[i];
        if (isAlive(enemy))
        {
            map = drawMatrix(map, enemy.i, enemy.j, enemyDraw.values[enemyDraw.currentAnimation]);
        }
    }
    return map;
}

Map drawShot(Map map, Shot shot)
{
    map.values[shot.i][shot.j] = 'o';
    return map;
}

Game playGame(char dir[], Game game)
{
    FILE *pFile = createFile(dir, FILE_INPUT, "r");
    char action = 0;
    int hittedEnemyId = NOT_HITTED;
    for (game.iteration = 0;; game.iteration++)
    {
        printGameState(game);
        if (!someEnemyAlive(game.enemiesCount, game.enemies))
        {
            printf("Parabéns, você ganhou!\n");
            return game;
        }
        if (someEnemyCrossPlayerLimit(game.enemiesCount, game.enemies, game.player))
        {
            printf("Você perdeu, tente novamente!\n");
            return game;
        }
        if ((hittedEnemyId = someEnemyHitted(game.enemiesCount, game.enemies, game.shot)) != NOT_HITTED)
        {
            game.points += calculatePoints(game.map, game.enemies[hittedEnemyId]);
            killEnemy(game.enemies, hittedEnemyId);
            game.shot = disableShot(game.shot);
        }
        do
        {
            fscanf("%c", &action);
        } while (action == '\n');
        game.enemiesDirection = moveEnemies(game.map, game.enemiesCount, game.enemies, game.enemiesDirection);
        game.shot = moveShot(game.shot);
        if (isValidAction(action))
        {
            if (action == ' ')
            {
                game.shot = shootShot(game.shot, game.player);
            }
            if (action != 's')
            {
                game.player = movePlayer(game.map, game.player, action);
            }
        }
        game = updateGameState(game);
    }
    fclose(pFile);
}

int moveEnemies(Map map, int enemiesCount, Enemy enemies[], int enemiesDirection)
{
    for (int i = 0; i < enemiesCount; i++)
    {
        Enemy enemy = enemies[i];
        if (!isAlive(enemy))
        {
            continue;
        }
        if (enemiesDirection == DIRECTION_RIGHT && isTouchingRightBorder(map, enemy.j))
        {
            moveDownEnemies(enemiesCount, enemies);
            return DIRECTION_LEFT;
        }
        if (enemiesDirection == DIRECTION_LEFT && isTouchingLeftBorder(enemy.j))
        {
            moveDownEnemies(enemiesCount, enemies);
            return DIRECTION_RIGHT;
        }
    }
    for (int i = 0; i < enemiesCount; i++)
    {
        Enemy enemy = enemies[i];
        if (!isAlive(enemy))
        {
            continue;
        }
        enemy.j += enemiesDirection;
    }
    return enemiesDirection;
}

void moveDownEnemies(int enemiesCount, Enemy enemies[])
{
    for (int i = 0; i < enemiesCount; i++)
    {
        Enemy enemy = enemies[i];
        if (!isAlive(enemy))
        {
            continue;
        }
        enemy.i++;
        enemies[i] = enemy;
    }
}

Shot moveShot(Shot shot)
{
    if (!shot.active)
    {
        return shot;
    }
    shot.i--;
    if (shot.i == 0)
    {
        shot = disableShot(shot);
    }
    return shot;
}

Shot shootShot(Shot shot, Player player)
{
    shot.active = 1;
    shot.i = calculatePlayerLimit(player);
    shot.j = player.j;
    return shot;
}

Player movePlayer(Map map, Player player, char action)
{
    if (action == 'a' && !isTouchingLeftBorder(player.j))
    {
        player.j--;
    }
    else if (action == 'd' && !isTouchingRightBorder(map, player.j))
    {
        player.j++;
    }
    return player;
}

void printGameState(Game game)
{
    printf("Pontos: %d | Iteracoes: %d", game.points, game.iteration);
    printMap(game.map);
}

Game updateGameState(Game game)
{
    Player player = game.player;
    game.map = cleanMap(game.map);
    game.map = drawPlayer(game.map, player.i, player.j);
    game.map = drawPlayerLimit(game.map, player);
    game.map = drawEnemies(game.map, game.enemiesCount, game.enemies, game.enemyDraw);
    game.map = drawShot(game.map, game.shot);
    return game;
}

void printMap(Map map)
{
    for (int i = 0; i < map.l; i++)
    {
        for (int j = 0; j < map.c; j++)
        {
            printf("%c", getMapValue(map, i, j));
            if (j == map.c - 1)
                printf("\n");
        }
    }
}
