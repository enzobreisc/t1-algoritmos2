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
void moveJogador(int m[][TAM], int n, int &px, int &py, int &celulaSobJogador, char tecla,  int orientacao);
bool jogadorVenceu(int celulaSobJogador);
bool portaEstaFechada(int celula, int orientacao);
bool celulaEhAtravessavel(const int m[][TAM], int n, int lin, int col, int orientacao);
void giraDireita(const int origem[][TAM], int destino[][TAM], int n);
void giraEsquerda(const int origem[][TAM], int destino[][TAM], int n);
bool estaSobreAlavanca(int celulaSobJogador);
void giraCenario(int m[][TAM], int n, int &orientacao, char tecla);
bool celulaSustentaBloco(const int m[][TAM], int n, int lin, int col, int orientacao);
void aplicaGravidade(int m[][TAM], int n, int orientacao);

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
        {1, 0, 1, 0, 1, 0, 1, 6, 1, 0, 1},
        {1, 0, 1, 3, 0, 0, 0, 0, 1, 0, 1},
        {1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1},
        {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 0, 1, 7, 1, 0, 1},
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

bool portaEstaFechada(int celula, int orientacao) {
    if (celula == PORTA_A) {
        return orientacao == 0 || orientacao == 180;
    }
    if (celula == PORTA_B) {
        return orientacao == 90 || orientacao == 270;
    }
    return false;
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
                simbolo = portaEstaFechada(m[i][j], orientacao) ? '=' : ':';
                break;
            case PORTA_B:
                simbolo = portaEstaFechada(m[i][j], orientacao) ? '|' : ';';
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

void giraCenario(int m[][TAM], int n, int &orientacao, char tecla)
{
    int auxiliar[TAM][TAM];

    if (tecla == 'e')
    {
        giraDireita(m, auxiliar, n);
        orientacao = (orientacao + 90) % 360;
    }
    else
    {
        giraEsquerda(m, auxiliar, n);
        orientacao = (orientacao + 270) % 360;
    }

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            m[i][j] = auxiliar[i][j];
        }
    }

    aplicaGravidade(m, n, orientacao);
}

bool celulaSustentaBloco(const int m[][TAM], int n, int lin, int col, int orientacao)
{
    if (lin < 0 || lin >= n || col < 0 || col >= n)
    {
        return true;
    }

    int valor = m[lin][col];

    if (valor == PORTA_A || valor == PORTA_B)
    {
        return portaEstaFechada(valor, orientacao);
    }

    return valor == PAREDE || valor == BLOCO || valor == JOGADOR || valor == ALAVANCA || valor == SAIDA;
}

void aplicaGravidade(int m[][TAM], int n, int orientacao)
{
    for (int i = n - 2; i >= 0; i--)
    {
        for (int j = 0; j < n; j++)
        {
            if (m[i][j] == BLOCO)
            {
                int lin = i;

                while (!celulaSustentaBloco(m, n, lin + 1, j, orientacao))
                {
                    m[lin][j] = VAZIO;
                    m[lin + 1][j] = BLOCO;
                    lin++;
                }
            }
        }
    }
}

bool celulaEhAtravessavel(const int m[][TAM], int n, int lin, int col, int orientacao) {
    if (lin < 0 || lin >= n || col < 0 || col >= n) {
        return false;
    }
    int valor = m[lin][col];
    if (valor == PORTA_A || valor == PORTA_B) {
        return !portaEstaFechada(valor, orientacao);
    }
    return valor == VAZIO || valor == ALAVANCA || valor == SAIDA;
}

void moveJogador(int m[][TAM], int n, int &px, int &py, int &celulaSobJogador, char tecla, int orientacao)
{
    int novoLin = px;
    int novoCol = py;

    if (tecla == 'w') novoLin = px - 1;
    else if (tecla == 's') novoLin = px + 1;
    else if (tecla == 'a') novoCol = py - 1;
    else if (tecla == 'd') novoCol = py + 1;

    if (celulaEhAtravessavel(m, n, novoLin, novoCol, orientacao))
    {
        m[px][py] = celulaSobJogador;
        celulaSobJogador = m[novoLin][novoCol];
        m[novoLin][novoCol] = JOGADOR;
        px = novoLin;
        py = novoCol;
    }
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
            moveJogador(cenario, TAM, px, py, celulaSobJogador, tecla, orientacao);
            movimentos++;
        }

        else if (tecla == 'q' || tecla == 'e')
        {
            if (estaSobreAlavanca(celulaSobJogador))
            {
                giraCenario(cenario, TAM, orientacao, tecla);
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