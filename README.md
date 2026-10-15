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
Mes/Ano: Outubro de 2026

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
| `ESC`     | Salva a partida em `jogo_salvo.csv` e volta ao menu         |

## Menu

- `[N]` Novo jogo - lista os mapas pelo titulo; escolha por numero ou aleatoria.
  Se ja existir um jogo salvo, pede confirmacao antes de substitui-lo
- `[C]` Continuar - aparece apenas quando existe um `jogo_salvo.csv` valido e
  retoma a partida, mesmo depois de fechar o programa
- `[V]` Ver solucao - reproduz o `mapaN.sol` de um mapa ou verifica todos
- `[R]` Recordes - pergunta o mapa e mostra os 5 melhores resultados
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
- Apos cada movimento ou rotacao, os blocos caem ate encontrar apoio (parede,
  porta fechada, outro bloco ou o jogador). Assim, um bloco apoiado no jogador
  cai quando ele sai de baixo.
- As portas alternam entre aberta e fechada conforme a orientacao do cenario:
  a porta A fecha em 0 e 180 graus, a porta B fecha em 90 e 270 graus.
- Se uma porta fechar em cima de um bloco, o bloco e destruido. Se fechar em
  cima do jogador, a fase e perdida.
- A fase e vencida quando o jogador alcanca a celula de saida.

## Mapas

Os mapas ficam em arquivos na pasta de execucao: `mapa1.csv`, `mapa2.csv`, ...
O jogo descobre quantos existem tentando abrir um por um ate faltar um numero.
Cada mapa tem a sua solucao em `mapaN.sol`.

- **Mapa 1 - Bloco que abre passagem** - exige a queda do bloco para vencer
- **Mapa 2 - Porta que some** - a porta A muda de estado com a rotacao
- **Mapa 3 - Duas rotacoes** - exige duas rotacoes e usa as portas A e B
- **Mapa 4 - Bloco no caminho** (15x15, autoral) - um bloco fecha o corredor
  depois da primeira porta A. Girando com E, a gravidade leva o bloco para o
  fundo do corredor, em cima de outra porta A, que o esmaga na rotacao
  seguinte. Girando com Q, o bloco cai pela porta ate ficar apoiado no
  jogador, e so desce quando o jogador sai de baixo dele. Nos dois casos a
  saida exige 2 rotacoes.
- **Mapa 5 - Tres alavancas** (13x13, autoral) - quatro salas ligadas por
  portas A, B e A, cada uma com uma alavanca. Cada rotacao inverte as portas,
  entao chegar a saida exige no minimo 3 rotacoes.

As solucoes e o minimo de rotacoes dos mapas 4 e 5 foram conferidos com uma
busca em largura que usa a propria `processaTecla` do jogo.

Formato do `mapaN.csv`: a linha 1 traz a dimensao `n` (de 11 a 21), a linha 2 o
titulo e as `n` linhas seguintes trazem `n` codigos de 0 a 7 separados por
virgula. Se a linha 2 for uma linha de codigos, o titulo e considerado ausente.
Um mapa invalido aparece na lista como indisponivel, com o motivo.

Os arquivos podem ter fim de linha do Windows (`\r\n`) e a marca BOM que o
Excel e alguns editores colocam no inicio de arquivos UTF-8.

Formato do `mapaN.sol`: uma unica linha so com as teclas `W A S D Q E`
(maiusculas ou minusculas), sem espacos.

## Jogo salvo (`jogo_salvo.csv`)

Ao sair de uma partida com `ESC`, o jogo grava o estado atual neste formato:

```
numeroMapa,n,orientacao,movimentos,rotacoes
titulo
n linhas do terreno (codigos de 0 a 7 separados por virgula)
n linhas do ocupante (0 = vazio, 2 = jogador, 3 = bloco)
```

Exemplo do cabecalho: `1,11,1,5,1` (mapa 1, 11x11, girado 90 graus, 5
movimentos, 1 rotacao). Como o terreno e o ocupante sao gravados separados, o
jogador sobre a alavanca volta exatamente como estava. A posicao do jogador nao
e gravada: e recalculada a partir do ocupante.

Na leitura o arquivo e validado (cabecalho numerico, `n` de 11 a 21, orientacao
de 0 a 3, quantidade de linhas e colunas, valores no intervalo e exatamente um
jogador). Arquivo ausente, vazio ou corrompido conta como "sem jogo salvo", e a
opcao Continuar nao aparece.

Quando o arquivo deixa de existir:

- **Vitoria** - o jogo salvo e apagado, entao uma partida vencida nao pode ser
  continuada.
- **ESC depois de uma derrota** - nada e gravado e o arquivo e apagado, para o
  menu nao oferecer Continuar de um jogo perdido.
- **Novo jogo confirmado** - a partida nova substitui a salva.

## Recordes (`recordes.csv`)

Ao vencer, o jogo pede o nome do jogador (ate 15 caracteres, sem virgula) e
acrescenta uma linha ao final de `recordes.csv`, no formato
`nome,mapa,movimentos,rotacoes` (exemplo: `Ana,1,14,1`). Toda vitoria e
registrada, e o arquivo e criado na primeira.

A opcao `[R]` Recordes carrega ate 200 registros num vetor simples, ordena e
mostra os 5 melhores do mapa escolhido. A ordenacao e um bubble sort proprio,
que troca os registros com o template `trocar`:

1. menos movimentos;
2. no empate, menos rotacoes;
3. no empate total, quem venceu primeiro fica na frente (o bubble sort so troca
   quando o proximo e estritamente melhor, entao a ordem de chegada se mantem).

Linhas mal formadas no arquivo sao ignoradas, e sem `recordes.csv` a tela
mostra uma mensagem de que ainda nao ha recordes.

## Organizacao do codigo

Todo o jogo esta em `index.cpp`. O mapa e mantido em duas matrizes separadas:

- `terreno` - o que e fixo na celula (parede, alavanca, saida, portas)
- `ocupante` - o que se move sobre a celula (jogador e blocos)

Essa separacao evita que o jogador ou um bloco "apague" a alavanca ou a saida
ao passar por cima.

Cada funcao tem um unico papel:

- **Regra** - altera o `Jogo`, nao imprime e nao acessa arquivo (`processaTecla`,
  `moveJogador`, `giraCenario`, `aplicaGravidade`, `validaMapa`, ...).
  `processaTecla` e a unica porta de entrada das regras: o jogo pelo teclado e a
  reproducao do `.sol` chamam a mesma funcao.
- **Tela** - so imprime e le o teclado, sem alterar o jogo (`desenhaJogo`,
  `exibeMenu`, `exibeRecordes`, `exibeSobre`, `escolheMapa`, ...).
- **Arquivo** - le ou grava, nao imprime e devolve `bool` + `string &motivo`
  quando pode falhar (`carregaMapa`, `carregaSolucao`, `salvaJogo`,
  `carregaJogoSalvo`, `registraRecorde`, ...).
- **Orquestracao** - cada opcao do menu (`opcaoNovoJogo`, `opcaoContinuar`,
  `opcaoVerSolucao`, `opcaoRecordes`) e `jogarPartida` ligam os tres papeis. O
  `main` so sorteia a semente e chama essas funcoes num `switch`.

## Arquivos

| Arquivo                     | Conteudo                                 |
| --------------------------- | ---------------------------------------- |
| `index.cpp`                 | codigo do jogo                           |
| `mapa1.csv` ... `mapaN.csv` | mapas, na mesma pasta do executavel      |
| `mapa1.sol` ... `mapaN.sol` | solucao de cada mapa                     |
| `jogo_salvo.csv`            | partida salva com ESC (criado pelo jogo) |
| `recordes.csv`              | uma linha por vitoria (criado pelo jogo) |

`jogo_salvo.csv` e `recordes.csv` ficam fora do repositorio (`.gitignore`).

## Defeitos da M1 corrigidos

- **Solucao do mapa 4 ("Porta que some")** - a sequencia do comentario
  (`DDWWDWWAAWAAASSS`) travava na 9a tecla. A correta e `ddwwdwwaqwddsssss`
  (17 teclas, 1 rotacao).
- **Solucao do mapa 5 ("Duas rotacoes")** - a sequencia do comentario
  (`DSDDWDDDDD`) travava na 4a tecla. A correta e `wqwwdwwaqwaaaa`
  (14 teclas, 2 rotacoes).
- **Teclas maiusculas** - com Caps Lock ligado nenhuma tecla funcionava. Agora
  a tecla lida passa por `tolower` no jogo, no menu e na escolha do mapa
  (a escolha aleatoria aceita `A` e `a`).
- **Continuar depois do fim** - `jogoEmAndamento` nunca voltava a `false`, entao
  dava para "continuar" uma partida ja vencida ou perdida.
- **Nome do jogador** - nao havia limite de tamanho e um `|` no nome quebrava o
  `placar.txt`. O nome agora e pedido so ao vencer, com ate 15 caracteres e sem
  virgula.
- **Versao do codigo-base** - a tela "Sobre" dizia "Windows", mas o codigo usa
  `stty`. Agora a tela e o README dizem Linux/macOS.
- **Bloco flutuando** - a gravidade so era aplicada depois de girar. Se o
  jogador sustentava um bloco e saia de baixo, o bloco ficava parado no ar (no
  mapa 1, por exemplo, com `wddssqes`). Agora `processaTecla` aplica a
  gravidade tambem depois de cada movimento. As solucoes de todos os mapas
  continuam valendo.
