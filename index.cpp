#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
using namespace std;

#define TAM 11
#define NUM_MAPAS 5
#define PRIMEIRO_MAPA_OFICIAL 3 // mapas 1 e 2 sao so de teste (Etapas 1 e 6), nao aparecem no menu

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
void separaTerrenoEOcupante(const int mapa[][TAM], int terreno[][TAM], int ocupante[][TAM], int n);
void carregaMapa(int terreno[][TAM], int ocupante[][TAM], int n, int numeroDoMapa);
void exibeMenu(bool jogoEmAndamento);
void exibeSobre();
int escolheMapa(int primeiroMapa, int ultimoMapa);
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
void jogarPartida(int terreno[][TAM], int ocupante[][TAM], int n, int numeroDoMapa, int &orientacao, int &px, int &py, int &movimentos, int &rotacoes, bool &jogoPerdido, bool &jogoVencido);

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

void separaTerrenoEOcupante(const int mapa[][TAM], int terreno[][TAM], int ocupante[][TAM], int n)
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            int valor = mapa[i][j];
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

void carregaMapa(int terreno[][TAM], int ocupante[][TAM], int n, int numeroDoMapa)
{
    int mapa1[TAM][TAM] = {
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

    // Mapa de teste do esmagamento (Etapa 6 / Exemplo 2 do enunciado):
    // bloco parado sobre porta A fechada; 2x E abre a porta (bloco cai e fica sobre
    // ela, agora aberta) e fecha de novo (bloco esmagado). Nao e um dos 3 mapas da
    // Etapa 9, so serve pra validar resolveEsmagamento isoladamente.
    int mapa2[TAM][TAM] = {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 5, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 2, 4, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 3, 6, 1, 0, 0, 0, 0, 1},
        {1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

    // Etapa 9 - mapa oficial 1: a solucao depende de um bloco caindo pra abrir
    // passagem. Sem girar, o bloco em pe bloqueia o unico caminho ate a saida.
    // Sequencia que resolve: w e a s s s s a
    int mapaBlocoCai[TAM][TAM] = {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 2, 0, 3, 0, 0, 0, 1, 1, 1, 1},
        {1, 1, 1, 1, 0, 5, 0, 1, 1, 1, 1},
        {1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

    // Etapa 9 - mapa oficial 2: a solucao depende de uma porta que some com a
    // rotacao. Porta A fechada em 0 graus bloqueia o corredor; girar uma vez
    // (90 graus) abre a porta A e libera o caminho ate a saida.
    // Sequencia que resolve: w e a s s s s s s s s s
    int mapaPortaSome[TAM][TAM] = {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 2, 0, 0, 0, 6, 0, 0, 0, 5, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

    // Etapa 9 - mapa oficial 3: a solucao exige 2 ou mais rotacoes. Porta A
    // (fechada em 0/180) e porta B (fechada em 90/270) nunca ficam abertas ao
    // mesmo tempo: gira 1x pra abrir a porta A e alcancar a segunda alavanca,
    // gira de novo (90 -> 180) pra fechar a porta A (ja atravessada) e abrir a
    // porta B, so entao a saida fica alcancavel.
    // Sequencia que resolve: w e a s s s s d e w a a a a
    int mapaDuasRotacoes[TAM][TAM] = {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 4, 1, 1, 1, 4, 1, 1, 1, 1, 1},
        {1, 2, 0, 6, 0, 0, 0, 7, 0, 5, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

    switch (numeroDoMapa)
    {
    case 2:
        separaTerrenoEOcupante(mapa2, terreno, ocupante, n);
        break;
    case 3:
        separaTerrenoEOcupante(mapaBlocoCai, terreno, ocupante, n);
        break;
    case 4:
        separaTerrenoEOcupante(mapaPortaSome, terreno, ocupante, n);
        break;
    case 5:
        separaTerrenoEOcupante(mapaDuasRotacoes, terreno, ocupante, n);
        break;
    case 1:
    default:
        separaTerrenoEOcupante(mapa1, terreno, ocupante, n);
        break;
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

void exibeMenu(bool jogoEmAndamento)
{
    limpaTela();
    cout << "===== LABIRINTO GIRATORIO =====" << endl;
    cout << "[N] Novo jogo" << endl;
    if (jogoEmAndamento)
    {
        cout << "[C] Continuar" << endl;
    }
    cout << "[S] Sobre" << endl;
    cout << "[F] Fim" << endl;
    cout << "> ";
}

void exibeSobre()
{
    limpaTela();
    cout << "===== SOBRE =====" << endl;
    cout << "Equipe: Bruno Naressi, Enzo Breichatt" << endl;
    cout << "Mes/Ano: Setembro de 2026" << endl;
    cout << "Disciplina: Algoritmos e Programacao II (22817)" << endl;
    cout << "Professor: Tiago Felski" << endl;
    cout << "Codigo-base: versao Linux (termios.h/unistd.h)" << endl;
    cout << endl;
    cout << "Regras:" << endl;
    cout << " W/A/S/D  - move o jogador uma celula por vez" << endl;
    cout << " Q/E      - gira o cenario (anti-horario/horario), so sobre uma alavanca" << endl;
    cout << " R        - reinicia a fase atual" << endl;
    cout << " ESC      - volta ao menu, preservando o jogo para Continuar" << endl;
    cout << endl;
    cout << "Pressione qualquer tecla para voltar ao menu..." << endl;
    leTecla();
}

int escolheMapa(int primeiroMapa, int ultimoMapa)
{
    cout << "Novo jogo - mapa especifico ou aleatorio? (E/A): ";
    char tecla = leTecla();
    cout << endl;

    if (tecla == 'a')
    {
        return primeiroMapa + (rand() % (ultimoMapa - primeiroMapa + 1));
    }

    int escolhido;
    do
    {
        cout << "Escolha o mapa (" << primeiroMapa << " a " << ultimoMapa << "): ";
        cin >> escolhido;
        if (cin.fail())
        {
            cin.clear();
            escolhido = 0;
        }
        cin.ignore(10000, '\n');
    } while (escolhido < primeiroMapa || escolhido > ultimoMapa);

    return escolhido;
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

    if (tecla == 'w')
    {
        novoLin = px - 1;
    }
    else if (tecla == 's')
    {
        novoLin = px + 1;
    }
    else if (tecla == 'a')
    {
        novoCol = py - 1;
    }
    else if (tecla == 'd')
    {
        novoCol = py + 1;
    }

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

void jogarPartida(int terreno[][TAM], int ocupante[][TAM], int n, int numeroDoMapa, int &orientacao, int &px, int &py, int &movimentos, int &rotacoes, bool &jogoPerdido, bool &jogoVencido)
{
    bool jogando = true;
    while (jogando)
    {
        limpaTela();
        desenhaStatus(numeroDoMapa, orientacao, movimentos, rotacoes);
        desenhaCenario(terreno, ocupante, n, orientacao);

        if (jogoPerdido)
        {
            cout << "Voce foi esmagado pela porta! Fase perdida." << endl;
            cout << "Pressione R para reiniciar ou ESC para voltar ao menu." << endl;
        }
        else if (jogoVencido)
        {
            cout << "Parabens! Voce venceu!" << endl;
            cout << "Pressione R para jogar novamente ou ESC para voltar ao menu." << endl;
        }

        char tecla = leTecla();

        if (tecla == 27) // ESC
        {
            jogando = false;
            continue;
        }

        if (tecla == 'r')
        {
            reiniciaFase(terreno, ocupante, n, numeroDoMapa, orientacao, px, py, movimentos, rotacoes, jogoPerdido);
            jogoVencido = false;
            continue;
        }

        if (jogoPerdido || jogoVencido)
        {
            continue;
        }

        if (tecla == 'w' || tecla == 'a' || tecla == 's' || tecla == 'd')
        {
            moveJogador(terreno, ocupante, n, px, py, tecla, orientacao);
            movimentos++;
        }
        else if (tecla == 'q' || tecla == 'e')
        {
            if (estaSobreAlavanca(terreno, px, py))
            {
                giraCenario(terreno, ocupante, n, orientacao, tecla, jogoPerdido);
                localizaJogador(ocupante, n, px, py);
                rotacoes++;
            }
        }

        if (!jogoPerdido && jogadorVenceu(terreno, px, py))
        {
            jogoVencido = true;
        }
    }
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
    bool jogoPerdido = false;
    bool jogoVencido = false;
    bool jogoEmAndamento = false;

    srand((unsigned int)time(nullptr));

    bool executando = true;
    while (executando)
    {
        exibeMenu(jogoEmAndamento);
        char opcao = leTecla();

        bool jogar = false;

        if (opcao == 'f')
        {
            executando = false;
        }
        else if (opcao == 's')
        {
            exibeSobre();
        }
        else if (opcao == 'n')
        {
            numeroDoMapa = escolheMapa(PRIMEIRO_MAPA_OFICIAL, NUM_MAPAS);
            reiniciaFase(terreno, ocupante, TAM, numeroDoMapa, orientacao, px, py, movimentos, rotacoes, jogoPerdido);
            jogoVencido = false;
            jogoEmAndamento = true;
            jogar = true;
        }
        else if (opcao == 'c' && jogoEmAndamento)
        {
            jogar = true;
        }

        if (jogar)
        {
            jogarPartida(terreno, ocupante, TAM, numeroDoMapa, orientacao, px, py, movimentos, rotacoes, jogoPerdido, jogoVencido);
        }
    }

    return 0;
}