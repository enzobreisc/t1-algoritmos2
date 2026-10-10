#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

#define TAM 11
#define NUM_MAPAS 5
#define PRIMEIRO_MAPA_OFICIAL 3 // mapas 1 e 2 sao so de teste, nao aparecem no menu

#define VAZIO 0
#define PAREDE 1
#define JOGADOR 2
#define BLOCO 3
#define ALAVANCA 4
#define SAIDA 5
#define PORTA_A 6
#define PORTA_B 7

// resultados de processaTecla
#define TECLA_IGNORADA 0 // tecla do jogo, mas sem efeito (parede, giro fora da alavanca, jogo encerrado)
#define TECLA_OK 1
#define TECLA_VENCEU 2
#define TECLA_PERDEU 3
#define TECLA_INVALIDA 4 // nao e W, A, S, D, Q nem E

struct Posicao
{
    int lin, col;
};

struct Jogo
{
    int terreno[TAM][TAM];
    int ocupante[TAM][TAM];
    int n;
    int orientacao;
    Posicao jogador;
    int movimentos, rotacoes;
    int numeroMapa;
    string titulo;
    bool perdido, vencido;
};

struct Recorde
{
    string nome;
    int mapa, movimentos, rotacoes;
};

struct InfoMapa
{
    int numero;
    string titulo;
    bool valido;
    string motivo;
};

template <typename T>
void trocar(T &a, T &b)
{
    T aux = a;
    a = b;
    b = aux;
}

char leTecla();
void limpaTela();
void separaTerrenoEOcupante(const int mapa[][TAM], Jogo &j);
void carregaMapa(Jogo &j, int numeroDoMapa);
void exibeMenu(bool jogoEmAndamento);
void exibeSobre();
void exibePlacar(int numeroDoMapa, const string &nomeJogador);
void registraPlacar(int numeroDoMapa, const string &nomeJogador, int movimentos, int &posicao);
string leNomeJogador();
int escolheMapa(int primeiroMapa, int ultimoMapa);
void desenhaCenario(const Jogo &j);
void desenhaStatus(const Jogo &j);
void desenhaJogo(const Jogo &j);
Posicao localizaJogador(const Jogo &j);
bool portaEstaFechada(int celula, int orientacao);
bool celulaEhAtravessavel(const Jogo &j, int lin, int col);
bool moveJogador(Jogo &j, char tecla);
bool jogadorVenceu(const Jogo &j);
bool estaSobreAlavanca(const Jogo &j);
void copiaMatriz(const int origem[][TAM], int destino[][TAM], int n);
void giraDireita(Jogo &j);
void giraEsquerda(Jogo &j);
bool celulaSustentaBloco(const Jogo &j, int lin, int col);
void aplicaGravidade(Jogo &j);
void resolveEsmagamento(Jogo &j);
void giraCenario(Jogo &j, char tecla);
int processaTecla(Jogo &j, char tecla);
void reiniciaFase(Jogo &j);
void jogarPartida(Jogo &j);

char leTecla()
{
    char ch;

    system("stty -icanon -echo");

    ch = getchar();

    system("stty icanon echo");

    return ch;
}

void limpaTela()
{
    cout << "\033[2J\033[1;1H";
}

void separaTerrenoEOcupante(const int mapa[][TAM], Jogo &j)
{
    for (int lin = 0; lin < j.n; lin++)
    {
        for (int col = 0; col < j.n; col++)
        {
            int valor = mapa[lin][col];

            if (valor == JOGADOR || valor == BLOCO)
            {
                j.ocupante[lin][col] = valor;
                j.terreno[lin][col] = VAZIO;
            }
            else
            {
                j.ocupante[lin][col] = VAZIO;
                j.terreno[lin][col] = valor;
            }
        }
    }
}

void carregaMapa(Jogo &j, int numeroDoMapa)
{
    // clang-format off
    // Sequencia para vencer: W, Q, D, W, W, W, W, W, W, D, D
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

    // Sequencia para vencer: W
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

    // MAPA 3 - exige a queda do bloco para vencer.
    // Sequencia para vencer: W, D, D, S, S, E, D, S, S, S, D, D, D
    int mapaBlocoCai[TAM][TAM] = {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 5, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1},
        {1, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1},
        {1, 2, 1, 0, 0, 0, 3, 0, 0, 1, 1},
        {1, 0, 1, 4, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

    // MAPA 4 - porta A muda de estado com a rotacao.
    // Sequencia para vencer: D, D, W, W, D, W, W, A, Q, W, D, D, S, S, S, S, S
    int mapaPortaSome[TAM][TAM] = {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1},
        {1, 0, 1, 0, 0, 1, 0, 1, 0, 4, 1},
        {1, 0, 1, 0, 0, 0, 1, 1, 0, 1, 1},
        {1, 0, 0, 0, 0, 1, 1, 4, 0, 0, 1},
        {1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1},
        {1, 0, 0, 5, 0, 6, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1},
        {1, 0, 1, 1, 1, 2, 0, 0, 0, 0, 1},
        {1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

    // MAPA 5 - exige duas ou mais rotacoes e usa portas A e B.
    // Sequencia para vencer: W, Q, W, W, D, W, W, A, Q, W, A, A, A, A
    int mapaDuasRotacoes[TAM][TAM] = {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1},
        {1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1},
        {1, 4, 0, 0, 1, 4, 1, 1, 1, 1, 1},
        {1, 2, 0, 6, 0, 0, 0, 7, 0, 5, 1},
        {1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};
    // clang-format on

    j.n = TAM;
    j.numeroMapa = numeroDoMapa;

    switch (numeroDoMapa)
    {
    case 2:
        j.titulo = "Teste de esmagamento";
        separaTerrenoEOcupante(mapa2, j);
        break;

    case 3:
        j.titulo = "Bloco que cai";
        separaTerrenoEOcupante(mapaBlocoCai, j);
        break;

    case 4:
        j.titulo = "Porta que some";
        separaTerrenoEOcupante(mapaPortaSome, j);
        break;

    case 5:
        j.titulo = "Duas rotacoes";
        separaTerrenoEOcupante(mapaDuasRotacoes, j);
        break;

    case 1:
    default:
        j.titulo = "Teste de rotacao";
        separaTerrenoEOcupante(mapa1, j);
        break;
    }
}

bool portaEstaFechada(int celula, int orientacao)
{
    if (celula == PORTA_A)
    {
        return orientacao == 0 || orientacao == 2;
    }

    if (celula == PORTA_B)
    {
        return orientacao == 1 || orientacao == 3;
    }

    return false;
}

void desenhaCenario(const Jogo &j)
{
    for (int lin = 0; lin < j.n; lin++)
    {
        for (int col = 0; col < j.n; col++)
        {
            char simbolo;

            if (j.ocupante[lin][col] == JOGADOR)
            {
                simbolo = '@';
            }
            else if (j.ocupante[lin][col] == BLOCO)
            {
                simbolo = 'O';
            }
            else
            {
                switch (j.terreno[lin][col])
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
                    simbolo = portaEstaFechada(j.terreno[lin][col], j.orientacao) ? '=' : ':';
                    break;

                case PORTA_B:
                    simbolo = portaEstaFechada(j.terreno[lin][col], j.orientacao) ? '|' : ';';
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

void desenhaStatus(const Jogo &j)
{
    cout << "Mapa: " << j.numeroMapa << " - " << j.titulo << endl;
    cout << "Orientacao: " << j.orientacao * 90;
    cout << "  Movimentos: " << j.movimentos;
    cout << "  Rotacoes: " << j.rotacoes << endl;
}

void desenhaJogo(const Jogo &j)
{
    limpaTela();

    desenhaStatus(j);

    desenhaCenario(j);

    if (j.perdido)
    {
        cout << "Voce foi esmagado pela porta! Fase perdida." << endl;
        cout << "Pressione R para reiniciar ou ESC para voltar ao menu." << endl;
    }
    else if (j.vencido)
    {
        cout << "Parabens! Voce venceu!" << endl;
        cout << "Pressione R para jogar novamente ou ESC para voltar ao menu." << endl;
    }
}

void carregaTodosOsPlacar(vector<Recorde> &registros)
{
    ifstream arquivo("placar.txt");

    if (!arquivo.is_open())
        return;

    string linha;

    while (getline(arquivo, linha))
    {
        if (linha.empty())
            continue;

        stringstream ss(linha);
        string mapaTexto, nome, movimentosTexto;

        getline(ss, mapaTexto, '|');
        getline(ss, nome, '|');
        getline(ss, movimentosTexto, '|');

        if (mapaTexto.empty() || nome.empty() || movimentosTexto.empty())
            continue;

        try
        {
            Recorde registro;
            registro.mapa = stoi(mapaTexto);
            registro.nome = nome;
            registro.movimentos = stoi(movimentosTexto);
            registro.rotacoes = 0;
            registros.push_back(registro);
        }
        catch (...)
        {
        }
    }
}

void salvaTodosOsPlacar(const vector<Recorde> &registros)
{
    ofstream arquivo("placar.txt", ios::trunc);

    for (const Recorde &registro : registros)
    {
        arquivo << registro.mapa << "|"
                << registro.nome << "|"
                << registro.movimentos << endl;
    }
}

void ordenaPlacar(vector<Recorde> &registros)
{
    sort(registros.begin(), registros.end(),
         [](const Recorde &a, const Recorde &b)
         {
             if (a.mapa != b.mapa)
                 return a.mapa < b.mapa;

             if (a.movimentos != b.movimentos)
                 return a.movimentos < b.movimentos;

             return a.nome < b.nome;
         });
}

void exibePlacar(int numeroDoMapa, const string &nomeJogador)
{
    vector<Recorde> registros;
    carregaTodosOsPlacar(registros);
    ordenaPlacar(registros);

    cout << endl;
    cout << "===== TOP 10 - MAPA " << numeroDoMapa << " =====" << endl;

    int posicao = 1;
    bool encontrouJogador = false;

    for (const Recorde &registro : registros)
    {
        if (registro.mapa != numeroDoMapa)
            continue;

        cout << posicao << ". " << registro.nome
             << " - " << registro.movimentos << " movimentos";

        if (registro.nome == nomeJogador)
        {
            cout << "  <-- SEU MELHOR RESULTADO";
            encontrouJogador = true;
        }

        cout << endl;
        posicao++;

        if (posicao > 10)
            break;
    }

    if (!encontrouJogador)
    {
        cout << "Seu melhor resultado ainda esta fora do TOP 10." << endl;
    }

    cout << endl;
}

void registraPlacar(int numeroDoMapa, const string &nomeJogador, int movimentos, int &posicao)
{
    vector<Recorde> registros;
    carregaTodosOsPlacar(registros);

    int melhorAnterior = -1;

    for (const Recorde &registro : registros)
    {
        if (registro.mapa == numeroDoMapa &&
            registro.nome == nomeJogador)
        {
            melhorAnterior = registro.movimentos;
            break;
        }
    }

    int melhorResultado = movimentos;

    if (melhorAnterior != -1 && melhorAnterior < melhorResultado)
        melhorResultado = melhorAnterior;

    vector<Recorde> novosRegistros;

    for (const Recorde &registro : registros)
    {
        if (registro.mapa == numeroDoMapa &&
            registro.nome == nomeJogador)
        {
            continue;
        }

        novosRegistros.push_back(registro);
    }

    novosRegistros.push_back({nomeJogador, numeroDoMapa, melhorResultado, 0});

    ordenaPlacar(novosRegistros);

    vector<Recorde> resultadoFinal;
    int quantidadeDoMapa = 0;

    for (const Recorde &registro : novosRegistros)
    {
        if (registro.mapa == numeroDoMapa)
        {
            if (quantidadeDoMapa < 10)
            {
                resultadoFinal.push_back(registro);
                quantidadeDoMapa++;
            }
        }
        else
        {
            resultadoFinal.push_back(registro);
        }
    }

    ordenaPlacar(resultadoFinal);
    salvaTodosOsPlacar(resultadoFinal);

    posicao = 11;
    int contador = 0;

    for (const Recorde &registro : resultadoFinal)
    {
        if (registro.mapa != numeroDoMapa)
            continue;

        contador++;

        if (registro.nome == nomeJogador)
        {
            posicao = contador;
            break;
        }
    }
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
    cout << "Equipe: Bruno Naressi, Enzo Breischatt" << endl;
    cout << "Mes/Ano: Outubro de 2026" << endl;
    cout << "Disciplina: Algoritmos e Programacao II (22817)" << endl;
    cout << "Professor: Tiago Felski" << endl;
    cout << "Codigo-base: Linux/macOS" << endl;
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

string leNomeJogador()
{
    string nome;
    bool nomeValido = false;

    while (!nomeValido)
    {
        cout << "Digite o seu nome (ate 15 caracteres, sem virgula): ";
        getline(cin, nome);

        if (nome.empty())
            cout << "O nome nao pode ficar vazio." << endl;
        else if (nome.length() > 15)
            cout << "O nome pode ter no maximo 15 caracteres." << endl;
        else if (nome.find(',') != string::npos || nome.find('|') != string::npos)
            cout << "O nome nao pode ter virgula nem barra vertical." << endl;
        else
            nomeValido = true;
    }

    return nome;
}

int escolheMapa(int primeiroMapa, int ultimoMapa)
{
    cout << "Novo jogo - mapa especifico ou aleatorio? (E/A): ";

    char tecla = tolower(leTecla());

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

Posicao localizaJogador(const Jogo &j)
{
    Posicao p = {0, 0};

    for (int lin = 0; lin < j.n; lin++)
    {
        for (int col = 0; col < j.n; col++)
        {
            if (j.ocupante[lin][col] == JOGADOR)
            {
                p.lin = lin;
                p.col = col;

                return p;
            }
        }
    }

    return p;
}

bool jogadorVenceu(const Jogo &j)
{
    return j.terreno[j.jogador.lin][j.jogador.col] == SAIDA;
}

bool estaSobreAlavanca(const Jogo &j)
{
    return j.terreno[j.jogador.lin][j.jogador.col] == ALAVANCA;
}

void copiaMatriz(const int origem[][TAM], int destino[][TAM], int n)
{
    for (int lin = 0; lin < n; lin++)
    {
        for (int col = 0; col < n; col++)
        {
            destino[lin][col] = origem[lin][col];
        }
    }
}

void giraDireita(Jogo &j)
{
    int terrenoAux[TAM][TAM];
    int ocupanteAux[TAM][TAM];

    for (int lin = 0; lin < j.n; lin++)
    {
        for (int col = 0; col < j.n; col++)
        {
            terrenoAux[col][j.n - 1 - lin] = j.terreno[lin][col];
            ocupanteAux[col][j.n - 1 - lin] = j.ocupante[lin][col];
        }
    }

    copiaMatriz(terrenoAux, j.terreno, j.n);
    copiaMatriz(ocupanteAux, j.ocupante, j.n);
}

void giraEsquerda(Jogo &j)
{
    int terrenoAux[TAM][TAM];
    int ocupanteAux[TAM][TAM];

    for (int lin = 0; lin < j.n; lin++)
    {
        for (int col = 0; col < j.n; col++)
        {
            terrenoAux[j.n - 1 - col][lin] = j.terreno[lin][col];
            ocupanteAux[j.n - 1 - col][lin] = j.ocupante[lin][col];
        }
    }

    copiaMatriz(terrenoAux, j.terreno, j.n);
    copiaMatriz(ocupanteAux, j.ocupante, j.n);
}

bool celulaEhAtravessavel(const Jogo &j, int lin, int col)
{
    if (lin < 0 || lin >= j.n || col < 0 || col >= j.n)
    {
        return false;
    }

    if (j.ocupante[lin][col] != VAZIO)
    {
        return false;
    }

    int valorTerreno = j.terreno[lin][col];

    if (valorTerreno == PORTA_A || valorTerreno == PORTA_B)
    {
        return !portaEstaFechada(valorTerreno, j.orientacao);
    }

    return valorTerreno == VAZIO || valorTerreno == ALAVANCA || valorTerreno == SAIDA;
}

bool moveJogador(Jogo &j, char tecla)
{
    Posicao nova = j.jogador;

    if (tecla == 'w')
        nova.lin--;
    else if (tecla == 's')
        nova.lin++;
    else if (tecla == 'a')
        nova.col--;
    else if (tecla == 'd')
        nova.col++;
    else
        return false;

    if (!celulaEhAtravessavel(j, nova.lin, nova.col))
        return false;

    j.ocupante[j.jogador.lin][j.jogador.col] = VAZIO;
    j.ocupante[nova.lin][nova.col] = JOGADOR;

    j.jogador = nova;

    return true;
}

bool celulaSustentaBloco(const Jogo &j, int lin, int col)
{
    if (lin < 0 || lin >= j.n || col < 0 || col >= j.n)
    {
        return true;
    }

    if (j.ocupante[lin][col] == BLOCO || j.ocupante[lin][col] == JOGADOR)
    {
        return true;
    }

    int valorTerreno = j.terreno[lin][col];

    if (valorTerreno == PORTA_A || valorTerreno == PORTA_B)
    {
        return portaEstaFechada(valorTerreno, j.orientacao);
    }

    return valorTerreno == PAREDE;
}

void aplicaGravidade(Jogo &j)
{
    for (int i = j.n - 2; i >= 0; i--)
    {
        for (int col = 0; col < j.n; col++)
        {
            if (j.ocupante[i][col] == BLOCO)
            {
                int lin = i;

                while (!celulaSustentaBloco(j, lin + 1, col))
                {
                    j.ocupante[lin][col] = VAZIO;
                    j.ocupante[lin + 1][col] = BLOCO;

                    lin++;
                }
            }
        }
    }
}

void resolveEsmagamento(Jogo &j)
{
    for (int lin = 0; lin < j.n; lin++)
    {
        for (int col = 0; col < j.n; col++)
        {
            int valorTerreno = j.terreno[lin][col];

            bool ehPorta = (valorTerreno == PORTA_A || valorTerreno == PORTA_B);

            if (ehPorta && portaEstaFechada(valorTerreno, j.orientacao))
            {
                if (j.ocupante[lin][col] == BLOCO)
                {
                    j.ocupante[lin][col] = VAZIO;
                }
                else if (j.ocupante[lin][col] == JOGADOR)
                {
                    j.perdido = true;
                }
            }
        }
    }
}

void giraCenario(Jogo &j, char tecla)
{
    if (tecla == 'e')
    {
        giraDireita(j);

        j.orientacao = (j.orientacao + 1) % 4;
    }
    else
    {
        giraEsquerda(j);

        j.orientacao = (j.orientacao + 3) % 4;
    }

    resolveEsmagamento(j);

    aplicaGravidade(j);

    // unico ponto, alem de moveJogador, em que a posicao guardada muda
    j.jogador = localizaJogador(j);
}

int processaTecla(Jogo &j, char tecla)
{
    tecla = tolower(tecla);

    bool ehMovimento = (tecla == 'w' || tecla == 'a' || tecla == 's' || tecla == 'd');
    bool ehRotacao = (tecla == 'q' || tecla == 'e');

    if (!ehMovimento && !ehRotacao)
    {
        return TECLA_INVALIDA;
    }

    if (j.perdido || j.vencido)
    {
        return TECLA_IGNORADA;
    }

    if (ehMovimento)
    {
        if (!moveJogador(j, tecla))
        {
            return TECLA_IGNORADA;
        }

        j.movimentos++;
    }
    else
    {
        if (!estaSobreAlavanca(j))
        {
            return TECLA_IGNORADA;
        }

        giraCenario(j, tecla);

        j.rotacoes++;
    }

    if (j.perdido)
    {
        return TECLA_PERDEU;
    }

    if (jogadorVenceu(j))
    {
        j.vencido = true;

        return TECLA_VENCEU;
    }

    return TECLA_OK;
}

void reiniciaFase(Jogo &j)
{
    carregaMapa(j, j.numeroMapa);

    j.jogador = localizaJogador(j);

    j.orientacao = 0;
    j.movimentos = 0;
    j.rotacoes = 0;
    j.perdido = false;
    j.vencido = false;
}

void jogarPartida(Jogo &j)
{
    bool jogando = true;

    while (jogando)
    {
        desenhaJogo(j);

        char tecla = tolower(leTecla());

        // ESC e R sao comandos da partida, nao teclas do jogo: ficam fora de processaTecla
        if (tecla == 27)
        {
            jogando = false;
        }
        else if (tecla == 'r')
        {
            reiniciaFase(j);
        }
        else if (processaTecla(j, tecla) == TECLA_VENCEU)
        {
            limpaTela();
            cout << "===== MAPA " << j.numeroMapa << " CONCLUIDO =====" << endl;
            cout << "Movimentos nesta tentativa: " << j.movimentos << endl;
            cout << "Rotacoes: " << j.rotacoes << endl;
            cout << endl;

            string nomeJogador = leNomeJogador();

            int posicao;
            registraPlacar(j.numeroMapa, nomeJogador, j.movimentos, posicao);

            if (posicao <= 10)
            {
                cout << endl;
                cout << "Seu melhor resultado esta no " << posicao
                     << "o lugar do TOP 10!" << endl;
            }
            else
            {
                cout << endl;
                cout << "Seu melhor resultado esta fora do TOP 10." << endl;
            }

            exibePlacar(j.numeroMapa, nomeJogador);

            cout << "Pressione qualquer tecla para continuar..." << endl;
            leTecla();
        }
    }
}

int main()
{
    Jogo jogo;

    bool jogoEmAndamento = false;

    srand((unsigned int)time(nullptr));

    bool executando = true;

    while (executando)
    {
        exibeMenu(jogoEmAndamento);

        char opcao = tolower(leTecla());

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
            jogo.numeroMapa = escolheMapa(PRIMEIRO_MAPA_OFICIAL, NUM_MAPAS);

            reiniciaFase(jogo);

            jogoEmAndamento = true;
            jogar = true;
        }
        else if (opcao == 'c' && jogoEmAndamento)
        {
            jogar = true;
        }

        if (jogar)
        {
            jogarPartida(jogo);

            jogoEmAndamento = !jogo.perdido && !jogo.vencido;
        }
    }

    return 0;
}
