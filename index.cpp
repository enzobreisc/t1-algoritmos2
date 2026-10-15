#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
using namespace std;

const int TAM = 21;
const int TAM_MIN = 11;
const int MAX_MAPAS = 100;
const int PAUSA_MS = 300;
const string ARQUIVO_JOGO_SALVO = "jogo_salvo.csv";
const string ARQUIVO_RECORDES = "recordes.csv";
const int MAX_RECORDES = 200;
const int MAX_RECORDES_EXIBIDOS = 5;

#define VAZIO 0
#define PAREDE 1
#define JOGADOR 2
#define BLOCO 3
#define ALAVANCA 4
#define SAIDA 5
#define PORTA_A 6
#define PORTA_B 7

#define TECLA_IGNORADA 0
#define TECLA_OK 1
#define TECLA_VENCEU 2
#define TECLA_PERDEU 3
#define TECLA_INVALIDA 4

#define SOL_VALIDA 0
#define SOL_INCOMPLETA 1
#define SOL_DERROTA 2
#define SOL_TECLA_INVALIDA 3

#define ESCOLHA_VOLTAR 0
#define ESCOLHA_TODOS -1

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
void pausa(int milissegundos);
void exibeMenu(bool temJogoSalvo);
void exibeSobre();
void exibeRecordes(const Recorde v[], int qtd, int mapa);
void exibeListaMapas(const InfoMapa mapas[], int qtd);
string leNomeJogador();
int escolheMapa(const InfoMapa mapas[], int qtd, bool paraSolucao);
void desenhaCenario(const Jogo &j);
void desenhaStatus(const Jogo &j);
void desenhaJogo(const Jogo &j);
void desenhaFimDePartida(const Jogo &j);

string nomeArquivoMapa(int numero, const string &extensao);
bool leLinha(ifstream &arquivo, string &linha);
bool pareceLinhaDoCenario(const string &linha);
int contaCaracteres(const string &texto);
bool converteInteiro(const string &texto, int &valor);
bool leLinhaDoMapa(const string &linha, int valores[], int n, string &motivo);
bool carregaMapa(int numero, Jogo &j, string &motivo);
int contaMapas();
int carregaInfoMapas(InfoMapa mapas[], int max);
bool carregaSolucao(int numero, string &teclas, string &motivo);
void gravaMatriz(ofstream &arquivo, const int matriz[][TAM], int n);
bool salvaJogo(const Jogo &j, string &motivo);
bool carregaJogoSalvo(Jogo &j, string &motivo);
bool existeJogoSalvo();
void apagaJogoSalvo();
bool registraRecorde(Recorde r, string &motivo);
int carregaRecordes(Recorde v[], int max);
void ordenaRecordes(Recorde v[], int qtd);

void separaTerrenoEOcupante(const int mapa[][TAM], Jogo &j);
bool validaMapa(const Jogo &j, string &motivo);
bool mapaEstavel(Jogo j);
int sorteiaMapaValido(const InfoMapa mapas[], int qtd);
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
int executaSolucao(Jogo &j, string teclas, int &posicao);
string descreveResultado(int resultado, int posicao, const Jogo &j);

bool reiniciaFase(Jogo &j, string &motivo);
int reproduzSolucao(Jogo &j, string teclas, int &posicao);
string verificaMapa(int numero);
void jogarPartida(Jogo &j);
void opcaoNovoJogo();
void opcaoContinuar();
void opcaoVerSolucao();
void opcaoRecordes();

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

void pausa(int milissegundos)
{
    cout.flush();

    usleep(milissegundos * 1000);
}

string nomeArquivoMapa(int numero, const string &extensao)
{
    return "mapa" + to_string(numero) + extensao;
}

bool leLinha(ifstream &arquivo, string &linha)
{
    if (!getline(arquivo, linha))
    {
        return false;
    }

    if (!linha.empty() && linha[linha.length() - 1] == '\r')
    {
        linha.erase(linha.length() - 1);
    }

    if (linha.compare(0, 3, "\xEF\xBB\xBF") == 0)
    {
        linha.erase(0, 3);
    }

    return true;
}

bool pareceLinhaDoCenario(const string &linha)
{
    if (linha.find(',') == string::npos)
    {
        return false;
    }

    for (size_t i = 0; i < linha.length(); i++)
    {
        if (!isdigit((unsigned char)linha[i]) && linha[i] != ',')
        {
            return false;
        }
    }

    return true;
}

int contaCaracteres(const string &texto)
{
    int total = 0;

    for (size_t i = 0; i < texto.length(); i++)
    {
        if (((unsigned char)texto[i] & 0xC0) != 0x80)
        {
            total++;
        }
    }

    return total;
}

bool converteInteiro(const string &texto, int &valor)
{
    if (texto.empty())
    {
        return false;
    }

    for (size_t i = 0; i < texto.length(); i++)
    {
        if (!isdigit((unsigned char)texto[i]))
        {
            return false;
        }
    }

    try
    {
        valor = stoi(texto);
    }
    catch (...)
    {
        return false;
    }

    return true;
}

bool leLinhaDoMapa(const string &linha, int valores[], int n, string &motivo)
{
    int colunas = 1;

    for (size_t i = 0; i < linha.length(); i++)
    {
        if (linha[i] == ',')
        {
            colunas++;
        }
    }

    if (colunas != n)
    {
        motivo = "quantidade de colunas diferente de " + to_string(n);
        return false;
    }

    stringstream ss(linha);
    string campo;

    for (int col = 0; col < n; col++)
    {
        getline(ss, campo, ',');

        if (!converteInteiro(campo, valores[col]))
        {
            motivo = "valor nao numerico (" + campo + ")";
            return false;
        }

        if (valores[col] < VAZIO || valores[col] > PORTA_B)
        {
            motivo = "valor fora do intervalo de 0 a 7 (" + campo + ")";
            return false;
        }
    }

    return true;
}

bool carregaMapa(int numero, Jogo &j, string &motivo)
{
    ifstream arquivo(nomeArquivoMapa(numero, ".csv"));

    if (!arquivo.is_open())
    {
        motivo = "arquivo inexistente";
        return false;
    }

    Jogo lido;
    string linha;

    if (!leLinha(arquivo, linha) || linha.empty())
    {
        motivo = "dimensao ausente";
        return false;
    }

    if (!converteInteiro(linha, lido.n))
    {
        motivo = "dimensao nao numerica";
        return false;
    }

    if (lido.n < TAM_MIN || lido.n > TAM)
    {
        motivo = "dimensao fora do intervalo de " + to_string(TAM_MIN) + " a " + to_string(TAM);
        return false;
    }

    if (!leLinha(arquivo, linha) || linha.empty())
    {
        motivo = "titulo ausente";
        return false;
    }

    if (pareceLinhaDoCenario(linha))
    {
        motivo = "titulo ausente";
        return false;
    }

    lido.titulo = linha;

    int mapa[TAM][TAM];

    for (int lin = 0; lin < lido.n; lin++)
    {
        if (!leLinha(arquivo, linha))
        {
            motivo = "menos linhas do que a dimensao informada";
            return false;
        }

        string motivoDaLinha;

        if (!leLinhaDoMapa(linha, mapa[lin], lido.n, motivoDaLinha))
        {
            motivo = "linha " + to_string(lin + 1) + " do cenario: " + motivoDaLinha;
            return false;
        }
    }

    separaTerrenoEOcupante(mapa, lido);

    lido.numeroMapa = numero;
    lido.orientacao = 0;
    lido.movimentos = 0;
    lido.rotacoes = 0;
    lido.perdido = false;
    lido.vencido = false;
    lido.jogador = localizaJogador(lido);

    if (!validaMapa(lido, motivo))
    {
        return false;
    }

    j = lido;

    return true;
}

int contaMapas()
{
    int total = 0;
    bool existe = true;

    while (existe)
    {
        ifstream arquivo(nomeArquivoMapa(total + 1, ".csv"));

        if (arquivo.is_open())
        {
            total++;
        }
        else
        {
            existe = false;
        }
    }

    return total;
}

int carregaInfoMapas(InfoMapa mapas[], int max)
{
    int qtd = contaMapas();

    if (qtd > max)
    {
        qtd = max;
    }

    for (int i = 0; i < qtd; i++)
    {
        Jogo j;

        mapas[i].numero = i + 1;
        mapas[i].valido = carregaMapa(i + 1, j, mapas[i].motivo);
        mapas[i].titulo = mapas[i].valido ? j.titulo : nomeArquivoMapa(i + 1, ".csv");
    }

    return qtd;
}

bool carregaSolucao(int numero, string &teclas, string &motivo)
{
    ifstream arquivo(nomeArquivoMapa(numero, ".sol"));

    if (!arquivo.is_open())
    {
        motivo = "arquivo " + nomeArquivoMapa(numero, ".sol") + " inexistente";
        return false;
    }

    if (!leLinha(arquivo, teclas))
    {
        teclas = "";
    }

    return true;
}

void gravaMatriz(ofstream &arquivo, const int matriz[][TAM], int n)
{
    for (int lin = 0; lin < n; lin++)
    {
        for (int col = 0; col < n; col++)
        {
            if (col > 0)
            {
                arquivo << ",";
            }

            arquivo << matriz[lin][col];
        }

        arquivo << endl;
    }
}

bool salvaJogo(const Jogo &j, string &motivo)
{
    ofstream arquivo(ARQUIVO_JOGO_SALVO, ios::trunc);

    if (!arquivo.is_open())
    {
        motivo = "nao foi possivel criar " + ARQUIVO_JOGO_SALVO;
        return false;
    }

    arquivo << j.numeroMapa << "," << j.n << "," << j.orientacao << ","
            << j.movimentos << "," << j.rotacoes << endl;
    arquivo << j.titulo << endl;

    gravaMatriz(arquivo, j.terreno, j.n);
    gravaMatriz(arquivo, j.ocupante, j.n);

    if (!arquivo)
    {
        motivo = "erro ao gravar " + ARQUIVO_JOGO_SALVO;
        return false;
    }

    return true;
}

bool carregaJogoSalvo(Jogo &j, string &motivo)
{
    ifstream arquivo(ARQUIVO_JOGO_SALVO);

    if (!arquivo.is_open())
    {
        motivo = "nenhum jogo salvo";
        return false;
    }

    Jogo lido;
    string linha;

    if (!leLinha(arquivo, linha) || linha.empty())
    {
        motivo = "arquivo vazio";
        return false;
    }

    int campos = 1;

    for (size_t i = 0; i < linha.length(); i++)
    {
        if (linha[i] == ',')
        {
            campos++;
        }
    }

    if (campos != 5)
    {
        motivo = "cabecalho deve ter 5 campos";
        return false;
    }

    int cabecalho[5];
    stringstream ss(linha);
    string campo;

    for (int i = 0; i < 5; i++)
    {
        getline(ss, campo, ',');

        if (!converteInteiro(campo, cabecalho[i]))
        {
            motivo = "cabecalho nao numerico";
            return false;
        }
    }

    lido.numeroMapa = cabecalho[0];
    lido.n = cabecalho[1];
    lido.orientacao = cabecalho[2];
    lido.movimentos = cabecalho[3];
    lido.rotacoes = cabecalho[4];

    if (lido.numeroMapa < 1)
    {
        motivo = "numero do mapa invalido";
        return false;
    }

    if (lido.n < TAM_MIN || lido.n > TAM)
    {
        motivo = "dimensao fora do intervalo de " + to_string(TAM_MIN) + " a " + to_string(TAM);
        return false;
    }

    if (lido.orientacao > 3)
    {
        motivo = "orientacao fora do intervalo de 0 a 3";
        return false;
    }

    if (!leLinha(arquivo, linha) || linha.empty())
    {
        motivo = "titulo ausente";
        return false;
    }

    lido.titulo = linha;

    for (int lin = 0; lin < lido.n; lin++)
    {
        string motivoDaLinha;

        if (!leLinha(arquivo, linha))
        {
            motivo = "terreno com menos linhas do que a dimensao";
            return false;
        }

        if (!leLinhaDoMapa(linha, lido.terreno[lin], lido.n, motivoDaLinha))
        {
            motivo = "linha " + to_string(lin + 1) + " do terreno: " + motivoDaLinha;
            return false;
        }

        for (int col = 0; col < lido.n; col++)
        {
            if (lido.terreno[lin][col] == JOGADOR || lido.terreno[lin][col] == BLOCO)
            {
                motivo = "terreno com jogador ou bloco";
                return false;
            }
        }
    }

    int jogadores = 0;

    for (int lin = 0; lin < lido.n; lin++)
    {
        string motivoDaLinha;

        if (!leLinha(arquivo, linha))
        {
            motivo = "ocupante com menos linhas do que a dimensao";
            return false;
        }

        if (!leLinhaDoMapa(linha, lido.ocupante[lin], lido.n, motivoDaLinha))
        {
            motivo = "linha " + to_string(lin + 1) + " do ocupante: " + motivoDaLinha;
            return false;
        }

        for (int col = 0; col < lido.n; col++)
        {
            int valor = lido.ocupante[lin][col];

            if (valor != VAZIO && valor != JOGADOR && valor != BLOCO)
            {
                motivo = "ocupante com valor diferente de 0, 2 ou 3";
                return false;
            }

            if (valor != VAZIO && lido.terreno[lin][col] == PAREDE)
            {
                motivo = "jogador ou bloco dentro de uma parede";
                return false;
            }

            if (valor == JOGADOR)
            {
                jogadores++;
            }
        }
    }

    if (jogadores != 1)
    {
        motivo = "quantidade de jogadores diferente de 1";
        return false;
    }

    lido.perdido = false;
    lido.vencido = false;
    lido.jogador = localizaJogador(lido);

    j = lido;

    return true;
}

bool existeJogoSalvo()
{
    Jogo j;
    string motivo;

    return carregaJogoSalvo(j, motivo);
}

void apagaJogoSalvo()
{
    remove(ARQUIVO_JOGO_SALVO.c_str());
}

bool validaMapa(const Jogo &j, string &motivo)
{
    int jogadores = 0;
    int saidas = 0;

    for (int lin = 0; lin < j.n; lin++)
    {
        for (int col = 0; col < j.n; col++)
        {
            if (j.ocupante[lin][col] == JOGADOR)
            {
                jogadores++;
            }

            if (j.terreno[lin][col] == SAIDA)
            {
                saidas++;
            }
        }
    }

    if (jogadores == 0)
    {
        motivo = "nenhum jogador no mapa";
        return false;
    }

    if (jogadores > 1)
    {
        motivo = "mais de um jogador no mapa";
        return false;
    }

    if (saidas == 0)
    {
        motivo = "nenhuma saida no mapa";
        return false;
    }

    if (!mapaEstavel(j))
    {
        motivo = "mapa instavel: algum bloco comeca no ar";
        return false;
    }

    return true;
}

bool mapaEstavel(Jogo j)
{
    Jogo original = j;

    aplicaGravidade(j);

    for (int lin = 0; lin < j.n; lin++)
    {
        for (int col = 0; col < j.n; col++)
        {
            if (j.ocupante[lin][col] != original.ocupante[lin][col])
            {
                return false;
            }
        }
    }

    return true;
}

int sorteiaMapaValido(const InfoMapa mapas[], int qtd)
{
    int validos = 0;

    for (int i = 0; i < qtd; i++)
    {
        if (mapas[i].valido)
        {
            validos++;
        }
    }

    if (validos == 0)
    {
        return ESCOLHA_VOLTAR;
    }

    int sorteado = rand() % validos;

    for (int i = 0; i < qtd; i++)
    {
        if (mapas[i].valido)
        {
            if (sorteado == 0)
            {
                return mapas[i].numero;
            }

            sorteado--;
        }
    }

    return ESCOLHA_VOLTAR;
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
}

void desenhaFimDePartida(const Jogo &j)
{
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

bool registraRecorde(Recorde r, string &motivo)
{
    ofstream arquivo(ARQUIVO_RECORDES, ios_base::app);

    if (!arquivo.is_open())
    {
        motivo = "nao foi possivel abrir " + ARQUIVO_RECORDES;
        return false;
    }

    arquivo << r.nome << "," << r.mapa << "," << r.movimentos << "," << r.rotacoes << endl;

    if (!arquivo)
    {
        motivo = "erro ao gravar " + ARQUIVO_RECORDES;
        return false;
    }

    return true;
}

int carregaRecordes(Recorde v[], int max)
{
    ifstream arquivo(ARQUIVO_RECORDES);
    string linha;
    int qtd = 0;

    while (qtd < max && leLinha(arquivo, linha))
    {
        int virgulas = 0;

        for (size_t i = 0; i < linha.length(); i++)
        {
            if (linha[i] == ',')
            {
                virgulas++;
            }
        }

        if (virgulas != 3)
        {
            continue;
        }

        stringstream ss(linha);
        string mapaTexto, movimentosTexto, rotacoesTexto;
        Recorde r;

        getline(ss, r.nome, ',');
        getline(ss, mapaTexto, ',');
        getline(ss, movimentosTexto, ',');
        getline(ss, rotacoesTexto, ',');

        if (!r.nome.empty() &&
            converteInteiro(mapaTexto, r.mapa) &&
            converteInteiro(movimentosTexto, r.movimentos) &&
            converteInteiro(rotacoesTexto, r.rotacoes))
        {
            v[qtd] = r;
            qtd++;
        }
    }

    return qtd;
}

void ordenaRecordes(Recorde v[], int qtd)
{
    for (int i = 0; i < qtd - 1; i++)
    {
        for (int k = 0; k < qtd - 1 - i; k++)
        {
            bool proximoEhMelhor =
                v[k + 1].movimentos < v[k].movimentos ||
                (v[k + 1].movimentos == v[k].movimentos && v[k + 1].rotacoes < v[k].rotacoes);

            if (proximoEhMelhor)
            {
                trocar<Recorde>(v[k], v[k + 1]);
            }
        }
    }
}

void exibeRecordes(const Recorde v[], int qtd, int mapa)
{
    cout << "===== RECORDES - MAPA " << mapa << " =====" << endl;

    int exibidos = 0;

    for (int i = 0; i < qtd && exibidos < MAX_RECORDES_EXIBIDOS; i++)
    {
        if (v[i].mapa == mapa)
        {
            exibidos++;

            cout << exibidos << ". " << v[i].nome << " - "
                 << v[i].movimentos << " movimentos, "
                 << v[i].rotacoes << " rotacoes" << endl;
        }
    }

    if (exibidos == 0)
    {
        cout << "Ainda nao ha recordes neste mapa. Venca-o para aparecer aqui!" << endl;
    }

    cout << endl;
}

void exibeMenu(bool temJogoSalvo)
{
    limpaTela();

    cout << "===== LABIRINTO GIRATORIO =====" << endl;
    cout << "[N] Novo jogo" << endl;

    if (temJogoSalvo)
    {
        cout << "[C] Continuar" << endl;
    }

    cout << "[V] Ver solucao" << endl;
    cout << "[R] Recordes" << endl;
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

    cout << "Objetivo: levar o jogador (@) ate a saida (S)." << endl;
    cout << endl;

    cout << "Teclas da partida:" << endl;
    cout << " W/A/S/D  - move o jogador uma celula por vez" << endl;
    cout << " Q/E      - gira o cenario (anti-horario/horario), so sobre uma alavanca" << endl;
    cout << " R        - reinicia a fase atual" << endl;
    cout << " ESC      - salva a partida e volta ao menu" << endl;
    cout << endl;

    cout << "Regras:" << endl;
    cout << " - Depois de cada rotacao, os blocos (O) caem ate encontrar apoio." << endl;
    cout << " - A porta A fecha em 0 e 180 graus; a porta B, em 90 e 270 graus." << endl;
    cout << " - Porta que fecha sobre um bloco destroi o bloco; sobre o jogador," << endl;
    cout << "   a fase e perdida." << endl;
    cout << endl;

    cout << "Menu:" << endl;
    cout << " [N] Novo jogo   - escolhe o mapa pelo numero ou sorteia um (A)" << endl;
    cout << " [C] Continuar   - retoma a partida salva com ESC, mesmo depois de" << endl;
    cout << "                   fechar o programa" << endl;
    cout << " [V] Ver solucao - reproduz o mapaN.sol de um mapa; T verifica todos" << endl;
    cout << " [R] Recordes    - os 5 melhores de um mapa (menos movimentos e, no" << endl;
    cout << "                   empate, menos rotacoes)" << endl;
    cout << " [S] Sobre       - esta tela" << endl;
    cout << " [F] Fim         - encerra o programa" << endl;
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

        if (!getline(cin, nome))
        {
            cin.clear();
            return "Anonimo";
        }

        if (nome.empty())
            cout << "O nome nao pode ficar vazio." << endl;
        else if (contaCaracteres(nome) > 15)
            cout << "O nome pode ter no maximo 15 caracteres." << endl;
        else if (nome.find(',') != string::npos)
            cout << "O nome nao pode ter virgula." << endl;
        else
            nomeValido = true;
    }

    return nome;
}

void exibeListaMapas(const InfoMapa mapas[], int qtd)
{
    if (qtd == 0)
    {
        cout << "Nenhum arquivo de mapa encontrado (mapa1.csv, mapa2.csv, ...)." << endl;
    }

    for (int i = 0; i < qtd; i++)
    {
        cout << mapas[i].numero << ". " << mapas[i].titulo;

        if (!mapas[i].valido)
        {
            cout << " - indisponivel: " << mapas[i].motivo;
        }

        cout << endl;
    }

    cout << endl;
}

int escolheMapa(const InfoMapa mapas[], int qtd, bool paraSolucao)
{
    exibeListaMapas(mapas, qtd);

    if (sorteiaMapaValido(mapas, qtd) == ESCOLHA_VOLTAR)
    {
        cout << "Nenhum mapa disponivel. Pressione qualquer tecla para voltar ao menu..." << endl;
        leTecla();

        return ESCOLHA_VOLTAR;
    }

    while (true)
    {
        if (paraSolucao)
            cout << "Numero do mapa, T para verificar todos ou 0 para voltar: ";
        else
            cout << "Numero do mapa, A para aleatorio ou 0 para voltar: ";

        string resposta;

        if (!getline(cin, resposta))
        {
            cin.clear();
            return ESCOLHA_VOLTAR;
        }

        int numero;

        if (resposta.length() == 1 && tolower(resposta[0]) == 't' && paraSolucao)
        {
            return ESCOLHA_TODOS;
        }

        if (resposta.length() == 1 && tolower(resposta[0]) == 'a' && !paraSolucao)
        {
            return sorteiaMapaValido(mapas, qtd);
        }

        if (!converteInteiro(resposta, numero) || numero > qtd)
        {
            cout << "Opcao invalida." << endl;
        }
        else if (numero == 0)
        {
            return ESCOLHA_VOLTAR;
        }
        else if (!mapas[numero - 1].valido)
        {
            cout << "Esse mapa esta indisponivel: " << mapas[numero - 1].motivo << endl;
        }
        else
        {
            return numero;
        }
    }
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

int executaSolucao(Jogo &j, string teclas, int &posicao)
{
    posicao = 0;

    for (size_t i = 0; i < teclas.length(); i++)
    {
        posicao = i + 1;

        int resultado = processaTecla(j, teclas[i]);

        if (resultado == TECLA_INVALIDA)
            return SOL_TECLA_INVALIDA;

        if (resultado == TECLA_PERDEU)
            return SOL_DERROTA;

        if (resultado == TECLA_VENCEU)
            return SOL_VALIDA;
    }

    return SOL_INCOMPLETA;
}

string descreveResultado(int resultado, int posicao, const Jogo &j)
{
    switch (resultado)
    {
    case SOL_VALIDA:
        return "solucao valida: " + to_string(posicao) + " teclas, " +
               to_string(j.movimentos) + " movimentos, " +
               to_string(j.rotacoes) + " rotacoes";

    case SOL_DERROTA:
        return "derrota na tecla " + to_string(posicao);

    case SOL_TECLA_INVALIDA:
        return "tecla invalida na posicao " + to_string(posicao);

    default:
        return "as teclas acabaram e o jogador nao chegou a saida";
    }
}

bool reiniciaFase(Jogo &j, string &motivo)
{
    return carregaMapa(j.numeroMapa, j, motivo);
}

int reproduzSolucao(Jogo &j, string teclas, int &posicao)
{
    int resultado = SOL_INCOMPLETA;

    posicao = 0;

    desenhaJogo(j);
    pausa(PAUSA_MS);

    for (size_t i = 0; i < teclas.length() && resultado == SOL_INCOMPLETA; i++)
    {
        int posicaoNaTecla;

        resultado = executaSolucao(j, string(1, teclas[i]), posicaoNaTecla);
        posicao = i + 1;

        desenhaJogo(j);
        cout << "Tecla " << posicao << " de " << teclas.length() << ": " << teclas[i] << endl;
        pausa(PAUSA_MS);
    }

    return resultado;
}

string verificaMapa(int numero)
{
    Jogo j;
    string teclas, motivo;

    if (!carregaMapa(numero, j, motivo))
    {
        return "indisponivel: " + motivo;
    }

    if (!carregaSolucao(numero, teclas, motivo))
    {
        return motivo;
    }

    int posicao;
    int resultado = executaSolucao(j, teclas, posicao);

    return descreveResultado(resultado, posicao, j);
}

void jogarPartida(Jogo &j)
{
    bool jogando = true;
    string aviso;

    while (jogando)
    {
        desenhaJogo(j);
        desenhaFimDePartida(j);

        if (!aviso.empty())
        {
            cout << aviso << endl;
            aviso = "";
        }

        char tecla = tolower(leTecla());

        if (tecla == 27)
        {
            jogando = false;

            if (j.perdido)
            {
                apagaJogoSalvo();
            }
            else if (!j.vencido)
            {
                string motivo;

                if (!salvaJogo(j, motivo))
                {
                    cout << "Nao foi possivel salvar o jogo: " << motivo << endl;
                    cout << "Pressione qualquer tecla para voltar ao menu..." << endl;
                    leTecla();
                }
            }
        }
        else if (tecla == 'r')
        {
            string motivo;

            if (!reiniciaFase(j, motivo))
            {
                aviso = "Nao foi possivel reiniciar a fase: " + motivo;
            }
        }
        else if (processaTecla(j, tecla) == TECLA_VENCEU)
        {
            apagaJogoSalvo();

            limpaTela();
            cout << "===== MAPA " << j.numeroMapa << " CONCLUIDO =====" << endl;
            cout << "Movimentos nesta tentativa: " << j.movimentos << endl;
            cout << "Rotacoes: " << j.rotacoes << endl;
            cout << endl;

            Recorde r;
            r.nome = leNomeJogador();
            r.mapa = j.numeroMapa;
            r.movimentos = j.movimentos;
            r.rotacoes = j.rotacoes;

            string motivo;

            if (!registraRecorde(r, motivo))
            {
                cout << "Nao foi possivel registrar o recorde: " << motivo << endl;
            }

            Recorde recordes[MAX_RECORDES];
            int qtd = carregaRecordes(recordes, MAX_RECORDES);
            ordenaRecordes(recordes, qtd);

            cout << endl;
            exibeRecordes(recordes, qtd, j.numeroMapa);

            cout << "Pressione qualquer tecla para continuar..." << endl;
            leTecla();
        }
    }
}

void opcaoNovoJogo()
{
    InfoMapa mapas[MAX_MAPAS];
    int qtd = carregaInfoMapas(mapas, MAX_MAPAS);

    limpaTela();
    cout << "===== NOVO JOGO =====" << endl;

    if (existeJogoSalvo())
    {
        cout << "Existe um jogo salvo. Comecar um novo jogo vai substitui-lo." << endl;
        cout << "Deseja continuar? (S/N): ";

        char resposta = tolower(leTecla());

        cout << endl;

        if (resposta != 's')
        {
            return;
        }
    }

    int escolha = escolheMapa(mapas, qtd, false);

    if (escolha == ESCOLHA_VOLTAR)
    {
        return;
    }

    Jogo jogo;
    string motivo;

    if (!carregaMapa(escolha, jogo, motivo))
    {
        cout << "Nao foi possivel carregar o mapa: " << motivo << endl;
        cout << "Pressione qualquer tecla para voltar ao menu..." << endl;
        leTecla();

        return;
    }

    apagaJogoSalvo();

    jogarPartida(jogo);
}

void opcaoContinuar()
{
    Jogo jogo;
    string motivo;

    if (!carregaJogoSalvo(jogo, motivo))
    {
        limpaTela();
        cout << "Nao foi possivel continuar: " << motivo << endl;
        cout << "Pressione qualquer tecla para voltar ao menu..." << endl;
        leTecla();

        return;
    }

    jogarPartida(jogo);
}

void opcaoVerSolucao()
{
    InfoMapa mapas[MAX_MAPAS];
    int qtd = carregaInfoMapas(mapas, MAX_MAPAS);

    limpaTela();
    cout << "===== VER SOLUCAO =====" << endl;

    int escolha = escolheMapa(mapas, qtd, true);

    if (escolha == ESCOLHA_VOLTAR)
    {
        return;
    }

    if (escolha == ESCOLHA_TODOS)
    {
        limpaTela();
        cout << "===== VERIFICAR TODOS OS MAPAS =====" << endl;

        for (int i = 0; i < qtd; i++)
        {
            cout << mapas[i].numero << ". " << mapas[i].titulo << " - "
                 << verificaMapa(mapas[i].numero) << endl;
        }
    }
    else
    {
        Jogo j;
        string teclas, motivo;

        if (!carregaMapa(escolha, j, motivo))
        {
            cout << "Mapa indisponivel: " << motivo << endl;
        }
        else if (!carregaSolucao(escolha, teclas, motivo))
        {
            cout << "Nao ha solucao para reproduzir: " << motivo << endl;
        }
        else
        {
            int posicao;
            int resultado = reproduzSolucao(j, teclas, posicao);

            cout << "Resultado: " << descreveResultado(resultado, posicao, j) << endl;
        }
    }

    cout << endl;
    cout << "Pressione qualquer tecla para voltar ao menu..." << endl;
    leTecla();
}

void opcaoRecordes()
{
    InfoMapa mapas[MAX_MAPAS];
    int qtdMapas = carregaInfoMapas(mapas, MAX_MAPAS);

    limpaTela();
    cout << "===== RECORDES =====" << endl;

    exibeListaMapas(mapas, qtdMapas);

    int numero = ESCOLHA_VOLTAR;
    bool respondeu = (qtdMapas == 0);

    while (!respondeu)
    {
        cout << "Numero do mapa ou 0 para voltar: ";

        string resposta;

        if (!getline(cin, resposta))
        {
            cin.clear();
            respondeu = true;
        }
        else if (converteInteiro(resposta, numero) && numero <= qtdMapas)
        {
            respondeu = true;
        }
        else
        {
            cout << "Opcao invalida." << endl;
            numero = ESCOLHA_VOLTAR;
        }
    }

    if (numero != ESCOLHA_VOLTAR)
    {
        Recorde recordes[MAX_RECORDES];
        int qtd = carregaRecordes(recordes, MAX_RECORDES);
        ordenaRecordes(recordes, qtd);

        cout << endl;
        exibeRecordes(recordes, qtd, numero);
    }

    cout << "Pressione qualquer tecla para voltar ao menu..." << endl;
    leTecla();
}

int main()
{
    srand((unsigned int)time(nullptr));

    bool executando = true;

    while (executando)
    {
        bool temJogoSalvo = existeJogoSalvo();

        exibeMenu(temJogoSalvo);

        switch (tolower(leTecla()))
        {
        case 'n':
            opcaoNovoJogo();
            break;

        case 'c':
            if (temJogoSalvo)
            {
                opcaoContinuar();
            }
            break;

        case 'v':
            opcaoVerSolucao();
            break;

        case 'r':
            opcaoRecordes();
            break;

        case 's':
            exibeSobre();
            break;

        case 'f':
            executando = false;
            break;
        }
    }

    return 0;
}
