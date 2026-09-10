# Labirinto Giratorio

Trabalho 1 da disciplina de Algoritmos e Programacao II (22817).

Jogo de puzzle em modo texto onde o jogador precisa alcancar a saida do
labirinto. O diferencial e que o cenario inteiro pode ser rotacionado em 90
graus, o que muda a direcao da gravidade para os blocos e alterna o estado das
portas.

## Equipe

- Bruno Naressi
- Enzo Breischatt

Professor: Tiago Felski
Mes/Ano: Setembro de 2026

## Como compilar e executar

O jogo usa `stty` para ler teclas sem precisar de Enter, entao roda em
macOS ou Linux.

```bash
clang++ -Wall -g index.cpp -o index
./index
```

Ou, com g++:

```bash
g++ -Wall -g index.cpp -o index
./index
```

No VS Code tambem existe uma task de build pronta (`Cmd+Shift+B`), definida em
`.vscode/tasks.json`.

## Controles

| Tecla     | Acao                                                        |
| --------- | ----------------------------------------------------------- |
| `W A S D` | Move o jogador uma celula por vez                           |
| `Q` / `E` | Gira o cenario (anti-horario / horario) - so sobre alavanca |
| `R`       | Reinicia a fase atual                                       |
| `ESC`     | Volta ao menu, preservando o jogo para a opcao "Continuar"  |

## Menu

- `[N]` Novo jogo - permite escolher um mapa especifico ou sortear um aleatorio
- `[C]` Continuar - aparece apenas quando existe uma partida em andamento
- `[S]` Sobre - dados da equipe e regras
- `[F]` Fim - encerra o programa

## Simbolos do cenario

| Simbolo | Significado              |
| ------- | ------------------------ |
| `@`     | Jogador                  |
| `O`     | Bloco (sofre gravidade)  |
| `#`     | Parede                   |
| `A`     | Alavanca (permite girar) |
| `S`     | Saida                    |
| `=`     | Porta A fechada          |
| `:`     | Porta A aberta           |
| `\|`    | Porta B fechada          |
| `;`     | Porta B aberta           |

## Regras principais

- O cenario so pode ser girado quando o jogador esta em cima de uma alavanca.
- Apos cada rotacao, os blocos caem ate encontrar apoio (parede, porta fechada,
  outro bloco ou o jogador).
- As portas alternam entre aberta e fechada conforme a orientacao do cenario:
  a porta A fecha em 0 e 180 graus, a porta B fecha em 90 e 270 graus.
- Se uma porta fechar em cima de um bloco, o bloco e destruido. Se fechar em
  cima do jogador, a fase e perdida.
- A fase e vencida quando o jogador alcanca a celula de saida.

## Mapas

Sao 5 mapas no total. Os mapas 1 e 2 sao apenas de teste (Etapas 1 e 6) e nao
aparecem no menu; os mapas jogaveis sao do 3 ao 5:

- **Mapa 3** - exige a queda do bloco para vencer
- **Mapa 4** - a porta A muda de estado com a rotacao
- **Mapa 5** - exige duas ou mais rotacoes e usa as portas A e B

Cada mapa tem, no proprio codigo, um comentario com a sequencia de teclas que
leva a vitoria.

## Organizacao do codigo

Todo o jogo esta em `index.cpp`. O mapa e mantido em duas matrizes separadas:

- `terreno` - o que e fixo na celula (parede, alavanca, saida, portas)
- `ocupante` - o que se move sobre a celula (jogador e blocos)

Essa separacao evita que o jogador ou um bloco "apague" a alavanca ou a saida
ao passar por cima.
