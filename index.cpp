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

/*
 * DECISAO DE PROJETO (Desafio: "como representar duas coisas na mesma celula"):
 *
 * O cenario passou a ser representado por DUAS matrizes paralelas, em vez de uma so:
 *
 *   - terreno[][]  -> o que e FIXO do cenario: parede, vazio, alavanca, saida, porta A/B.
 *                     So muda quando o cenario gira (nunca e alterado pela gravidade).
 *   - ocupante[][] -> o que se MOVE por cima do terreno: jogador ou bloco (ou VAZIO,
 *                     se a celula nao tem ocupante). E o que a gravidade movimenta.
 *
 * Isso resolve de forma direta os casos de duas coisas na mesma celula:
 *   - jogador sobre a alavanca  -> ocupante=JOGADOR, terreno=ALAVANCA
 *   - bloco sobre uma porta aberta -> ocupante=BLOCO, terreno=PORTA_A/B
 *   - jogador na saida           -> ocupante=JOGADOR, terreno=SAIDA
 *
 * E, principalmente, resolve o problema do ESMAGAMENTO: como o valor da porta em
 * terreno[][] nunca e sobrescrito pelo bloco, ao girar o cenario da para checar,
 * depois da rotacao, se uma porta ficou fechada exatamente onde ha um ocupante.
 */

char leTecla();
void limpaTela();
void carregaMapa(int terreno[][TAM], int ocupante[][TAM], int n, int numeroDoMapa);
void desenhaCenario(const int terreno[][TAM], const int ocupante[][TAM], int n, int orientacao);
void desenhaStatus(int numeroDoMapa, int orientacao, int movimentos, int rotacoes);
void localizaJogador(const int ocupante[][TAM], int n, int &px, int &py);
bool portaEstaFechada(int celula, int orientacao);
bool celulaEhAtravessavel(const int terreno[][TAM], const int ocupante[][TAM], int n, int lin, int col, int orientacao);
void moveJogador(int terreno[][TAM], int ocupante[][TAM], int n, int &px, int &py, char tecla, int orientacao);
bool jogadorVenceu(const int terreno[][TAM], int px, int py);
bool estaSobreAlavanca(const int terreno[][TAM], int px, int py);
void giraDireita(const int origem[][TAM], int destino[][TAM], int n);
void giraEsquerda(const int origem[][TAM], int destino[][TAM], int n);
bool celulaSustentaBloco(const int terreno[][TAM], const int ocupante[][TAM], int n, int lin, int col, int orientacao);
void aplicaGravidade(const int terreno[][TAM], int ocupante[][TAM], int n, int orientacao);
void resolveEsmagamento(const int terreno[][TAM], int ocupante[][TAM], int n, int orientacao, bool &jogoPerdido);
void giraCenario(int terreno[][TAM], int ocupante[][TAM], int n, int &orientacao, char tecla, bool &jogoPerdido);
void reiniciaFase(int terreno[][TAM], int ocupante[][TAM], int n, int numeroDoMapa, int &orientacao, int &px, int &py, int &movimentos, int &rotacoes, bool &jogoPerdido);

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

void carregaMapa(int terreno[][TAM], int ocupante[][TAM], int n, int numeroDoMapa)
{
    
    int mapaTeste[TAM][TAM] = {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 4, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 2, 0, 6, 0, 3, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 5, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            int valor = mapaTeste[i][j];
            if (valor == JOGADOR || valor == BLOCO)
            {
                ocupante[i][j] = valor;
                terreno[i][j] = VAZIO;
            }
            else
            {
                ocupante[i][j] = VAZIO;
                terreno[i][j] = valor;
            }
        }
    }
}

bool portaEstaFechada(int celula, int orientacao)
{
    if (celula == PORTA_A)
    {
        return orientacao == 0 || orientacao == 180;
    }
    if (celula == PORTA_B)
    {
        return orientacao == 90 || orientacao == 270;
    }
    return false;
}

void desenhaCenario(const int terreno[][TAM], const int ocupante[][TAM], int n, int orientacao)
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            char simbolo;

            // O ocupante (jogador/bloco), quando existe, e desenhado por cima do terreno.
            if (ocupante[i][j] == JOGADOR)
            {
                simbolo = '@';
            }
            else if (ocupante[i][j] == BLOCO)
            {
                simbolo = 'O';
            }
            else
            {
                switch (terreno[i][j])
                {
                case VAZIO:
                    simbolo = ' ';
                    break;
                case PAREDE:
                    simbolo = '#';
                    break;
                case ALAVANCA:
                    simbolo = 'A';
                    break;
                case SAIDA:
                    simbolo = 'S';
                    break;
                case PORTA_A:
                    simbolo = portaEstaFechada(terreno[i][j], orientacao) ? '=' : ':';
                    break;
                case PORTA_B:
                    simbolo = portaEstaFechada(terreno[i][j], orientacao) ? '|' : ';';
                    break;
                default:
                    simbolo = '?';
                    break;
                }
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

void localizaJogador(const int ocupante[][TAM], int n, int &px, int &py)
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            if (ocupante[i][j] == JOGADOR)
            {
                px = i;
                py = j;
                return;
            }
        }
    }
}

bool jogadorVenceu(const int terreno[][TAM], int px, int py)
{
    return terreno[px][py] == SAIDA;
}

bool estaSobreAlavanca(const int terreno[][TAM], int px, int py)
{
    return terreno[px][py] == ALAVANCA;
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

bool celulaEhAtravessavel(const int terreno[][TAM], const int ocupante[][TAM], int n, int lin, int col, int orientacao)
{
    if (lin < 0 || lin >= n || col < 0 || col >= n)
    {
        return false;
    }
    if (ocupante[lin][col] != VAZIO)
    {
        return false; 
    }
    int valorTerreno = terreno[lin][col];
    if (valorTerreno == PORTA_A || valorTerreno == PORTA_B)
    {
        return !portaEstaFechada(valorTerreno, orientacao);
    }
    return valorTerreno == VAZIO || valorTerreno == ALAVANCA || valorTerreno == SAIDA;
}

void moveJogador(int terreno[][TAM], int ocupante[][TAM], int n, int &px, int &py, char tecla, int orientacao)
{
    int novoLin = px;
    int novoCol = py;

    if (tecla == 'w') novoLin = px - 1;
    else if (tecla == 's') novoLin = px + 1;
    else if (tecla == 'a') novoCol = py - 1;
    else if (tecla == 'd') novoCol = py + 1;

    if (celulaEhAtravessavel(terreno, ocupante, n, novoLin, novoCol, orientacao))
    {
        ocupante[px][py] = VAZIO;
        ocupante[novoLin][novoCol] = JOGADOR;
        px = novoLin;
        py = novoCol;
    }
}


bool celulaSustentaBloco(const int terreno[][TAM], const int ocupante[][TAM], int n, int lin, int col, int orientacao)
{
    if (lin < 0 || lin >= n || col < 0 || col >= n)
    {
        return true;
    }

    if (ocupante[lin][col] == BLOCO || ocupante[lin][col] == JOGADOR)
    {
        return true;
    }

    int valorTerreno = terreno[lin][col];

    if (valorTerreno == PORTA_A || valorTerreno == PORTA_B)
    {
        return portaEstaFechada(valorTerreno, orientacao);
    }

    return valorTerreno == PAREDE || valorTerreno == ALAVANCA || valorTerreno == SAIDA;
}


void aplicaGravidade(const int terreno[][TAM], int ocupante[][TAM], int n, int orientacao)
{
    for (int i = n - 2; i >= 0; i--)
    {
        for (int j = 0; j < n; j++)
        {
            if (ocupante[i][j] == BLOCO)
            {
                int lin = i;

                while (!celulaSustentaBloco(terreno, ocupante, n, lin + 1, j, orientacao))
                {
                    ocupante[lin][j] = VAZIO;
                    ocupante[lin + 1][j] = BLOCO;
                    lin++;
                }
            }
        }
    }
}


void resolveEsmagamento(const int terreno[][TAM], int ocupante[][TAM], int n, int orientacao, bool &jogoPerdido)
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            int valorTerreno = terreno[i][j];
            bool ehPorta = (valorTerreno == PORTA_A || valorTerreno == PORTA_B);

            if (ehPorta && portaEstaFechada(valorTerreno, orientacao))
            {
                if (ocupante[i][j] == BLOCO)
                {
                    ocupante[i][j] = VAZIO; 
                }
                else if (ocupante[i][j] == JOGADOR)
                {
                    jogoPerdido = true; 
                }
            }
        }
    }
}

void giraCenario(int terreno[][TAM], int ocupante[][TAM], int n, int &orientacao, char tecla, bool &jogoPerdido)
{
    int terrenoAux[TAM][TAM];
    int ocupanteAux[TAM][TAM];

    if (tecla == 'e')
    {
        giraDireita(terreno, terrenoAux, n);
        giraDireita(ocupante, ocupanteAux, n);
        orientacao = (orientacao + 90) % 360;
    }
    else
    {
        giraEsquerda(terreno, terrenoAux, n);
        giraEsquerda(ocupante, ocupanteAux, n);
        orientacao = (orientacao + 270) % 360;
    }

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            terreno[i][j] = terrenoAux[i][j];
            ocupante[i][j] = ocupanteAux[i][j];
        }
    }

    resolveEsmagamento(terreno, ocupante, n, orientacao, jogoPerdido);
    aplicaGravidade(terreno, ocupante, n, orientacao);
}

void reiniciaFase(int terreno[][TAM], int ocupante[][TAM], int n, int numeroDoMapa, int &orientacao, int &px, int &py, int &movimentos, int &rotacoes, bool &jogoPerdido)
{
    carregaMapa(terreno, ocupante, n, numeroDoMapa);
    localizaJogador(ocupante, n, px, py);
    orientacao = 0;
    movimentos = 0;
    rotacoes = 0;
    jogoPerdido = false;
}

int main()
{
    int terreno[TAM][TAM];
    int ocupante[TAM][TAM];
    int orientacao;
    int px, py;
    int movimentos;
    int numeroDoMapa = 1;
    int rotacoes;
    bool jogoPerdido;
    bool jogoVencido = false;

    reiniciaFase(terreno, ocupante, TAM, numeroDoMapa, orientacao, px, py, movimentos, rotacoes, jogoPerdido);

    bool jogando = true;
    while (jogando)
    {
        limpaTela();
        desenhaStatus(numeroDoMapa, orientacao, movimentos, rotacoes);
        desenhaCenario(terreno, ocupante, TAM, orientacao);

        if (jogoPerdido)
        {
            cout << "Voce foi esmagado pela porta! Fase perdida." << endl;
            cout << "Pressione R para reiniciar ou X para sair." << endl;
        }
        else if (jogoVencido)
        {
            cout << "Parabens! Voce venceu!" << endl;
            cout << "Pressione R para jogar novamente ou X para sair." << endl;
        }

        char tecla = leTecla();

        if (tecla == 'r')
        {
            reiniciaFase(terreno, ocupante, TAM, numeroDoMapa, orientacao, px, py, movimentos, rotacoes, jogoPerdido);
            jogoVencido = false;
            continue;
        }

        if (tecla == 'x')
        {
            jogando = false;
            continue;
        }

        if (jogoPerdido || jogoVencido)
        {
            continue; 
        }

        if (tecla == 'w' || tecla == 'a' || tecla == 's' || tecla == 'd')
        {
            moveJogador(terreno, ocupante, TAM, px, py, tecla, orientacao);
            movimentos++;
        }
        else if (tecla == 'q' || tecla == 'e')
        {
            if (estaSobreAlavanca(terreno, px, py))
            {
                giraCenario(terreno, ocupante, TAM, orientacao, tecla, jogoPerdido);
                localizaJogador(ocupante, TAM, px, py);
                rotacoes++;
            }
        }

        if (!jogoPerdido && jogadorVenceu(terreno, px, py))
        {
            jogoVencido = true;
        }
    }

    return 0;
}