#include <iostream>
#include <termios.h>
#include <unistd.h>
using namespace std;

#define TAM 11

#define VAZIO 0
#define PAREDE 1
#define JOGADOR 2
#define BLOCO 3
#define ALAVANCA 4
#define SAIDA 5
#define PORTA_A 6
#define PORTA_B 7

char leTecla();
void limpaTela();
void carregaMapa(int m[][TAM], int n, int numeroDoMapa);
void desenhaCenario(const int m[][TAM], int n, int orientacao);
void desenhaStatus(int numeroDoMapa, int orientacao, int movimentos, int rotacoes);
void localizaJogador(const int m[][TAM], int n, int &px, int &py);
bool celulaEhAtravessavel(const int m[][TAM], int n, int lin, int col);
void moveJogador(int m[][TAM], int n, int &px, int &py, char tecla);
bool jogadorVenceu(const int m[][TAM], int n, int px, int py);

char leTecla()
{
    struct termios antigo, novo;
    char ch;

    tcgetattr(STDIN_FILENO, &antigo);
    novo = antigo;
    novo.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &novo);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &antigo);
    return ch;
}

void limpaTela()
{
    cout << "\033[2J\033[1;1H";
}

void carregaMapa(int m[][TAM], int n, int numeroDoMapa)
{
    int mapaTeste[TAM][TAM] = {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1},
        {1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1},
        {1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1},
        {1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1},
        {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1},
        {1, 4, 0, 0, 1, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1},
        {1, 2, 0, 0, 0, 0, 0, 0, 0, 5, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            m[i][j] = mapaTeste[i][j];
        }
    }
}

void desenhaCenario(const int m[][TAM], int n, int orientacao)
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            char simbolo;
            switch (m[i][j])
            {
            case VAZIO:
                simbolo = ' ';
                break;
            case PAREDE:
                simbolo = '#';
                break;
            case JOGADOR:
                simbolo = '@';
                break;
            case BLOCO:
                simbolo = 'O';
                break;
            case ALAVANCA:
                simbolo = 'A';
                break;
            case SAIDA:
                simbolo = 'S';
                break;
            case PORTA_A:
                simbolo = '=';
                break;
            case PORTA_B:
                simbolo = '|';
                break;
            default:
                simbolo = '?';
                break;
            }
            cout << simbolo;
        }
        cout << endl;
    }
}

void desenhaStatus(int numeroDoMapa, int orientacao, int movimentos, int rotacoes)
{
    cout << "Mapa: " << numeroDoMapa;
    cout << "  Orientacao: " << orientacao;
    cout << "  Movimentos: " << movimentos;
    cout << "  Rotacoes: " << rotacoes << endl;
}

void localizaJogador(const int m[][TAM], int n, int &px, int &py)
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            if (m[i][j] == JOGADOR)
            {
                px = i;
                py = j;
                return;
            }
        }
    }
}

bool celulaEhAtravessavel(const int m[][TAM], int n, int lin, int col)
{
    if (lin < 0 || lin >= n || col < 0 || col >= n)
    {
        return false;
    }
    int valor = m[lin][col];
    return valor == VAZIO || valor == ALAVANCA || valor == SAIDA;
}

void moveJogador(int m[][TAM], int n, int &px, int &py, int &celulaSobJogador, char tecla)
{
    int novoLin = px;
    int novoCol = py;

    if (tecla == 'w')
        novoLin = px - 1;
    else if (tecla == 's')
        novoLin = px + 1;
    else if (tecla == 'a')
        novoCol = py - 1;
    else if (tecla == 'd')
        novoCol = py + 1;

    if (celulaEhAtravessavel(m, n, novoLin, novoCol))
    {
        m[px][py] = celulaSobJogador;
        celulaSobJogador = m[novoLin][novoCol];
        m[novoLin][novoCol] = JOGADOR;
        px = novoLin;
        py = novoCol;
    }
}

bool jogadorVenceu(int celulaSobJogador)
{
    return celulaSobJogador == SAIDA;
}

void giraDireita(const int origem[][TAM], int destino[][TAM], int n)
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            destino[j][n - 1 - i] = origem[i][j];
        }
    }
}

void giraEsquerda(const int origem[][TAM], int destino[][TAM], int n)
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            destino[n - 1 - j][i] = origem[i][j];
        }
    }
}

bool estaSobreAlavanca(int celulaSobJogador)
{
    return celulaSobJogador == ALAVANCA;
}

int main()
{
    int cenario[TAM][TAM];
    int orientacao = 0;
    int px, py;
    int movimentos = 0;
    int numeroDoMapa = 1;
    int celulaSobJogador = VAZIO;
    int rotacoes = 0;

    carregaMapa(cenario, TAM, numeroDoMapa);
    localizaJogador(cenario, TAM, px, py);
    desenhaStatus(numeroDoMapa, orientacao, movimentos, rotacoes);

    bool jogando = true;
    while (jogando)
    {
        limpaTela();
        desenhaStatus(numeroDoMapa, orientacao, movimentos, rotacoes);
        desenhaCenario(cenario, TAM, orientacao);

        char tecla = leTecla();

        if (tecla == 'w' || tecla == 'a' || tecla == 's' || tecla == 'd')
        {
            moveJogador(cenario, TAM, px, py, celulaSobJogador, tecla);
            movimentos++;
        }

        else if (tecla == 'q' || tecla == 'e')
        {
            if (estaSobreAlavanca(celulaSobJogador))
            {
                int auxiliar[TAM][TAM];

                if (tecla == 'e')
                {
                    giraDireita(cenario, auxiliar, TAM);
                    orientacao = (orientacao + 90) % 360;
                }
                else
                {
                    giraEsquerda(cenario, auxiliar, TAM);
                    orientacao = (orientacao + 270) % 360;
                }

                for (int i = 0; i < TAM; i++)
                {
                    for (int j = 0; j < TAM; j++)
                    {
                        cenario[i][j] = auxiliar[i][j];
                    }
                }

                localizaJogador(cenario, TAM, px, py);
                rotacoes++;
            }
        }
        else if (tecla == 'x')
        {
            jogando = false;
        }

        if (jogadorVenceu(celulaSobJogador))
        {
            limpaTela();
            desenhaStatus(numeroDoMapa, orientacao, movimentos, rotacoes);
            desenhaCenario(cenario, TAM, orientacao);
            cout << "Parabéns! Você venceu!" << endl;
            jogando = false;
        }
    }

    return 0;
}