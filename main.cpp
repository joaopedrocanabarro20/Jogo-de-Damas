-#define _CRT_SECURE_NO_WARNINGS

#include <GL/glut.h>
#include <GL/freeglut_ext.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "gl_canvas2d.h"
#include "auxiliar.h"

/**
// Trabalho 3 - Jogo de Damas 2D
// Aluno: Joao Pedro
//
// Quesitos implementados:
// - Tabuleiro 8x8, 10x10 ou 12x12, com matriz alocada dinamicamente.
// - Desenho do tabuleiro e das pecas na Canvas2D.
// - Cursor controlado por teclado, sem sair do tabuleiro.
// - Selecao de peca com ESPACO e finalizacao da jogada com ESPACO.
// - Alternancia entre jogador 1 e jogador 2.
// - Movimentos diagonais validos, impedindo andar sobre peca propria/inimiga.
// - Captura por salto, removendo a peca capturada do tabuleiro.
// - Promocao para dama ao chegar ao lado oposto do tabuleiro.
// - Movimento tambem com mouse: clique na peca e depois clique no destino.
// - Lista de instrucoes desenhada na tela.
//
// Nao foram implementados: tempo de jogada e animacao de captura.
// **/

//valores que cada casa do tabuleiro pode ter
int VAZIO = 0;
int PECA_AZUL = 1;
int PECA_VERMELHA = 2;
int DAMA_AZUL = 3;
int DAMA_VERMELHA = 4;

//posicao e tamanho do tabuleiro na tela
int TAB_X = 200;
int TAB_Y = 160;
int N = 8;//casas por lado (8, 10 ou 12)
int TAM = 75;//tamanho de cada casa em pixels

int screenWidth = 1000;
int screenHeight = 800;
int mx, my;
int tela = 0;//0=menu 1=jogando 2=acabou

int** tab = NULL;

int curL = 0, curC = 0;//posicao do cursor
int selL = -1, selC = -1;//peca selecionada (-1 = nenhuma)
int turno = 0;//PECA_AZUL ou PECA_VERMELHA
int vencedor = 0;//0=nenhum  1=azul  2=vermelho

//quando uma peca captura e pode capturar de novo
bool capturando = false;
int capL = -1, capC = -1; //peca que DEVE continuar capturando

void DrawMouseScreenCoords() {

}
//retorna 1 se azul, 2 se vermelha, 0 se vazia
int corDaPeca(int p)
{
    if (p == PECA_AZUL || p == DAMA_AZUL)     return 1;
    if (p == PECA_VERMELHA || p == DAMA_VERMELHA) return 2;
    return 0;
}

//converte pixel X para coluna do tabuleiro (-1 se fora)
int pixelParaColuna(int x)
{
    if (x < TAB_X || x >= TAB_X + N * TAM) return -1;
    return (x - TAB_X) / TAM;
}

//converte pixel Y para linha do tabuleiro (-1 se fora)
int pixelParaLinha(int y)
{
    if (y < TAB_Y || y >= TAB_Y + N * TAM) return -1;
    return (y - TAB_Y) / TAM;
}

//cor do time que esta jogando agora
int corDoTurnoAtual()
{
    if (turno == PECA_AZUL) return 1;
    return 2;
}

//direcao de linha que uma peca normal anda (azul sobe, vermelha desce)
int direcaoDePeca(int cor)
{
    if (cor == 1) return -1;
    return 1;
}
//checa se a peca em (l,c) consegue capturar alguma inimiga
bool podeCapturar(int l, int c, int peca)
{
    int  minhaCor = corDaPeca(peca);
    bool dama = (peca == DAMA_AZUL || peca == DAMA_VERMELHA);

    if (!dama) {
        int dir = direcaoDePeca(minhaCor);

        if (l + 2 * dir >= 0 && l + 2 * dir < N && c - 2 >= 0) {
            int corAlvo = corDaPeca(tab[l + dir][c - 1]);
            if (corAlvo != 0 && corAlvo != minhaCor && tab[l + 2 * dir][c - 2] == VAZIO)
                return true;
        }
        if (l + 2 * dir >= 0 && l + 2 * dir < N && c + 2 < N) {
            int corAlvo = corDaPeca(tab[l + dir][c + 1]);
            if (corAlvo != 0 && corAlvo != minhaCor && tab[l + 2 * dir][c + 2] == VAZIO)
                return true;
        }
        return false;
    }

    //dama verifica nas 4 diagonais
    int dl[] = { -1, -1,  1,  1 };
    int dc[] = { -1,  1, -1,  1 };

    for (int d = 0; d < 4; d++) {
        int i = 1;
        bool achouInimigo = false;
        while (true) {
            int nl = l + i * dl[d];
            int nc = c + i * dc[d];
            if (nl < 0 || nl >= N || nc < 0 || nc >= N) break;

            int p = tab[nl][nc];
            if (p != VAZIO) {
                int cor = corDaPeca(p);
                if (cor == minhaCor || achouInimigo) break;
                achouInimigo = true;
            }
            else {
                if (achouInimigo) return true;
            }
            i++;
        }
    }
    return false;
}

//retorna true se alguma peca do turno atual consegue capturar
bool alguemDeveCapturar()
{
    int minhaCor = corDoTurnoAtual();
    for (int l = 0; l < N; l++)
        for (int c = 0; c < N; c++) {
            int p = tab[l][c];
            if (p != VAZIO && corDaPeca(p) == minhaCor)
                if (podeCapturar(l, c, p)) return true;
        }
    return false;
}
void checarFimDeJogo()
{
    int azuis = 0;
    int vermelhos = 0;

    for (int l = 0; l < N; l++)
        for (int c = 0; c < N; c++) {
            if (corDaPeca(tab[l][c]) == 1) azuis++;
            if (corDaPeca(tab[l][c]) == 2) vermelhos++;
        }

    if (azuis == 0) { vencedor = 2; tela = 2; return; }
    if (vermelhos == 0) { vencedor = 1; tela = 2; return; }

    //veja se o proximo jogador tem algum lance disponivel
    int proximoTurno = PECA_VERMELHA;
    if (turno == PECA_VERMELHA) proximoTurno = PECA_AZUL;

    int corProximo = 2;
    if (proximoTurno == PECA_AZUL) corProximo = 1;

    bool temLance = false;

    for (int l = 0; l < N && !temLance; l++) {
        for (int c = 0; c < N && !temLance; c++) {
            if (corDaPeca(tab[l][c]) != corProximo) continue;

            int  p = tab[l][c];
            bool dam = (p == DAMA_AZUL || p == DAMA_VERMELHA);
            int  dir = direcaoDePeca(corProximo);

            if (!dam) {
                if (l + dir >= 0 && l + dir < N) {
                    if (c - 1 >= 0 && tab[l + dir][c - 1] == VAZIO) temLance = true;
                    if (c + 1 < N && tab[l + dir][c + 1] == VAZIO) temLance = true;
                }
                if (podeCapturar(l, c, p)) temLance = true;
            }
            else {
                int dl[] = { -1, -1, 1, 1 };
                int dc[] = { -1,  1,-1, 1 };
                for (int d = 0; d < 4 && !temLance; d++) {
                    int nl = l + dl[d], nc = c + dc[d];
                    if (nl >= 0 && nl < N && nc >= 0 && nc < N && tab[nl][nc] == VAZIO)
                        temLance = true;
                }
                if (podeCapturar(l, c, p)) temLance = true;
            }
        }
    }

    if (!temLance) {
        //quem nao tem movimento perde — vence quem jogou por ultimo
        if (turno == PECA_AZUL) vencedor = 1;
        else                    vencedor = 2;
        tela = 2;
    }
}
void liberarTab()
{
    if (tab == NULL) return;
    for (int i = 0; i < N; i++) free(tab[i]);
    free(tab);
    tab = NULL;
}

void iniciarJogo()
{
    liberarTab();

    tab = (int**)malloc(N * sizeof(int*));
    for (int i = 0; i < N; i++)
        tab[i] = (int*)malloc(N * sizeof(int));

    int fileiras = N / 2 - 1; //linhas de pecas por lado

    for (int l = 0; l < N; l++)
        for (int c = 0; c < N; c++) {
            if ((l + c) % 2 == 1) {
                if (l < fileiras)      tab[l][c] = PECA_VERMELHA;
                else if (l >= N - fileiras) tab[l][c] = PECA_AZUL;
                else                        tab[l][c] = VAZIO;
            }
            else {
                tab[l][c] = VAZIO;
            }
        }

    turno = PECA_VERMELHA; //vermelho comeca
    curL = curC = 0;
    selL = selC = -1;
    vencedor = 0;
    capturando = false;
    capL = capC = -1;
}



void desenhaMenu()//desenho
{
    CV::color(1);
    CV::text(370, 350, "DAMAS");
    CV::color(3);
    CV::text(350, 400, "Pressione '1' para tabuleiro 8x8");
    CV::text(350, 430, "Pressione '2' para tabuleiro 10x10");
    CV::text(350, 460, "Pressione '3' para tabuleiro 12x12");
    CV::color(0.5f, 0.5f, 0.5f);
   

    //botoes clicaveis
    CV::color(0.2f, 0.2f, 0.2f);
    CV::rectFill(350, 530, 530, 570);
    CV::rectFill(350, 580, 530, 620);
    CV::rectFill(350, 630, 530, 670);

    CV::color(1);
    CV::rect(350, 530, 530, 570);
    CV::rect(350, 580, 530, 620);
    CV::rect(350, 630, 530, 670);

    CV::text(390, 555, "8x8");
    CV::text(390, 605, "10x10");
    CV::text(390, 655, "12x12");
}

void desenhaFim()
{
    //escurece o tabuleiro
    CV::color(0.08f, 0.08f, 0.08f);
    CV::rectFill(TAB_X, TAB_Y, TAB_X + N * TAM, TAB_Y + N * TAM);

    int meioY = TAB_Y + N * TAM / 2;

    CV::color(1.0f, 0.85f, 0.0f);
    CV::text(TAB_X + 160, meioY + 60, "ACABOU");

    if (vencedor == 1) {
        CV::color(0.0f, 0.5f, 1.0f);
        CV::text(TAB_X + 130, meioY + 20, "Jogador AZUL venceu!");
    }
    else {
        CV::color(1.0f, 0.2f, 0.2f);
        CV::text(TAB_X + 100, meioY + 20, "Jogador VERMELHO venceu!");
    }

    CV::color(0.6f, 0.6f, 0.6f);
    CV::text(TAB_X + 100, meioY - 20, "R = jogar de novo");
    CV::text(TAB_X + 110, meioY - 50, "ESC = menu");
}

void render()
{
    CV::clear(0, 0, 0);

    if (tela == 0){ 
        desenhaMenu(); return;
    }

    //cabecalho
    CV::color(8);
    CV::text(430, 40, "| JOGO DE DAMAS |");
    CV::color(1);
    CV::text(260, 70, "setas / mouse para mover  |  espaco / clique para selecionar");

    if (tela == 1) {
        if (turno == PECA_AZUL) {
            CV::color(0, 0, 1);
            CV::text(420, 100, "vez do Jogador: azul");
        }
        else {
            CV::color(2);
            CV::text(400, 100, "vez do Jogador: vermelho");
        }

        if (capturando) {
            CV::color(1, 0, 0);
            CV::text(360, 130, "CAPTURA MULTIPLA - voce tem que capturar!");
        }
        else if (alguemDeveCapturar()) {
            CV::color(1, 1, 0);
            CV::text(410, 130, "CAPTURA OBRIGATORIA!");
        }
    }

    //casas do tabuleiro
    CV::color(1);
    CV::rect(TAB_X, TAB_Y, TAB_X + N * TAM, TAB_Y + N * TAM);

    int raio = (int)(TAM * 0.4);

    for (int l = 0; l < N; l++) {
        for (int c = 0; c < N; c++) {
            int px = TAB_X + c * TAM;
            int py = TAB_Y + l * TAM;
            int cx = px + TAM / 2;
            int cy = py + TAM / 2;

            if ((l + c) % 2 == 1) {
                CV::color(1);
                CV::rectFill(px, py, px + TAM, py + TAM);
            }

            //verde na selecionada
            if (l == selL && c == selC) {
                CV::color(0, 1, 0);
                CV::rectFill(px, py, px + TAM, py + TAM);
            }

            int p = tab[l][c];
            if (p == VAZIO) continue;

            if (corDaPeca(p) == 1)
            {
                CV::color(0, 0, 1);
            }
            else {
                CV::color(2);
            }
            CV::circleFill(cx, cy, raio, 40);

            //criei um circulo menor amarelo para representar a dama
            if (p == DAMA_AZUL || p == DAMA_VERMELHA) {
                CV::color(1, 1, 0);
                CV::circleFill(cx, cy, raio / 2, 40);
            }
        }
    }

    //cursor amarelo — segue o mouse quando ele esta dentro do tabuleiro
    if (tela == 1) {
        int px = TAB_X + curC * TAM;
        int py = TAB_Y + curL * TAM;
        CV::color(1, 1, 0);
        CV::rect(px, py, px + TAM, py + TAM);
        CV::rect(px + 1, py + 1, px + TAM - 1, py + TAM - 1);
    }

    if (tela == 2) desenhaFim();

    funcaoDeOutroArquivo();
    DrawMouseScreenCoords();
}


void processarClique(int l, int c)//tanto pelo mouse como pelo teclado aqui
{
    if (tela != 1) return;

    int minhaCor = corDoTurnoAtual();
    int pCur = tab[l][c];

    //sem peca selecionada:tenta selecionar
    if (selL == -1) {
        if (corDaPeca(pCur) != minhaCor) return;

        if (capturando) {
            //so pode selecionar a peca que esta em cadeia de captura
            if (l == capL && c == capC)
                selL = l, selC = c;
        }
        else if (alguemDeveCapturar()) {
            //captura obrigatoria: so seleciona quem pode capturar
            if (podeCapturar(l, c, pCur))
                selL = l, selC = c;
        }
        else {
            selL = l; selC = c;
        }
        return;
    }

    //clicou na peca selecionada: deseleciona
    if (selL == l && selC == c) { selL = selC = -1; return; }

    //destino tem que ser casa escura e vazia
    if (tab[l][c] != VAZIO || (l + c) % 2 != 1) {
        //clicou em outra peca propria: troca a selecao
        if (corDaPeca(tab[l][c]) == minhaCor && !capturando) {
            selL = l; selC = c;
        }
        return;
    }

    int pSel = tab[selL][selC];
    bool dam = (pSel == DAMA_AZUL || pSel == DAMA_VERMELHA);
    int dL = l - selL;
    int dC = c - selC;
    int aL = abs(dL);
    int aC = abs(dC);

    if (aL != aC || aL == 0) return; //tem que ser diagonal

    bool moveu = false;
    bool capturou = false;
    int  mL = -1, mC = -1; //posicao da peca capturada

    if (!dam) {
        //movimento simples: 1 casa na direcao correta
        if (aL == 1 && !alguemDeveCapturar() && !capturando) {
            if (pSel == PECA_AZUL && dL == -1) moveu = true;
            if (pSel == PECA_VERMELHA && dL == 1) moveu = true;
        }
        //captura: 2 casas, inimigo no meio
        if (aL == 2) {
            int dl2 = (selL + l) / 2;
            int dc2 = (selC + c) / 2;
            if (pSel == PECA_AZUL && dL == -2) {
                int meio = tab[dl2][dc2];
                if (meio != VAZIO && corDaPeca(meio) != minhaCor) {
                    moveu = capturou = true;
                    mL = dl2; mC = dc2;
                }
            }
            if (pSel == PECA_VERMELHA && dL == 2) {
                int meio = tab[dl2][dc2];
                if (meio != VAZIO && corDaPeca(meio) != minhaCor) {
                    moveu = capturou = true;
                    mL = dl2; mC = dc2;
                }
            }
        }
    }
    else {
        //dama: anda quantas casas quiser na diagonal
        int dl = dL / aL;
        int dc = dC / aC;
        int inimigos = 0;

        for (int i = 1; i < aL; i++) {
            int pl = selL + i * dl;
            int pc = selC + i * dc;
            int p = tab[pl][pc];
            if (p == VAZIO) continue;
            if (corDaPeca(p) == minhaCor) { inimigos = 99; break; }
            inimigos++;
            mL = pl; mC = pc;
        }

        if (inimigos == 0 && !alguemDeveCapturar() && !capturando)
            moveu = true;
        else if (inimigos == 1)
            moveu = capturou = true;
    }

    if (!moveu) return;

    //faz o movimento no tabuleiro
    tab[l][c] = pSel;
    tab[selL][selC] = VAZIO;
    if (capturou) tab[mL][mC] = VAZIO;

    //promocao a dama ao chegar na ultima linha
    if (tab[l][c] == PECA_AZUL && l == 0)     tab[l][c] =DAMA_AZUL;
    if (tab[l][c] == PECA_VERMELHA && l == N - 1) tab[l][c] =DAMA_VERMELHA;

    //se capturou e ainda pode capturar, mantem o turno
    if (capturou && podeCapturar(l, c, tab[l][c])) {
        capturando = true;
        capL = l; capC = c;
        selL = selC = -1;
        return;
    }

    //fim da jogada — troca turno e verifica se acabou
    capturando = false;
    capL = capC = -1;
    selL = selC = -1;

    if (turno == PECA_AZUL) turno = PECA_VERMELHA;
    else                    turno = PECA_AZUL;

    checarFimDeJogo();
}



void keyboard(int key)//teclado
{
    if (tela == 0) {
        if (key == '1') {
            N = 8;  TAM = 600 / N; tela = 1; iniciarJogo();
        }
        if (key == '2') {
            N = 10; TAM = 600 / N; tela = 1; iniciarJogo(); 
        }
        if (key == '3') {
            N = 12; TAM = 600 / N; tela = 1; iniciarJogo(); 
        }
        return;
    }

    if (tela == 2) {
        if (key == 'r' || key == 'R') { tela = 1; iniciarJogo(); }
        if (key == 27) { tela = 0; liberarTab(); }
        return;
    }

    //jogo ativo
    if (key == 27) {
        tela = 0; liberarTab(); return; 
    }
    if (key == 'r' || key == 'R') { 
        iniciarJogo();          return; 
    }

    if (key == 203 && curL > 0)curL--;
    if (key == 201 && curL < N - 1)curL++;
    if (key == 202 && curC < N - 1)curC++;
    if (key == 200 && curC > 0)curC--;

    if (key == 32) processarClique(curL, curC);
}

void keyboardUp(int key) { 
    (void)key;
}


//utilizacao do mouse
void mouse(int button, int state, int wheel, int direction, int x, int y)
{
    mx = x; my = y;

    //atualiza o cursor quando o mouse esta dentro do tabuleiro
    if (tela == 1 && TAM > 0) {
        int c = pixelParaColuna(x);
        int l = pixelParaLinha(y);
        if (l != -1 && c != -1) {
            curL = l;
            curC = c;
        }
    }

    //so processa clique esquerdo
    if (button != 0 || state != 0) return;

    //menu:clique nos botoes
    if (tela == 0) {
        if (x >= 350 && x <= 530) {
            if (y >= 530 && y <= 570) { 
                N = 8;  TAM = 600 / N; tela = 1; iniciarJogo(); 
            }
            if (y >= 580 && y <= 620) {
                N = 10; TAM = 600 / N; tela = 1; iniciarJogo(); 
            }
            if (y >= 630 && y <= 670) {
                N = 12; TAM = 600 / N; tela = 1; iniciarJogo(); 
            }
        }
        return;
    }

    //tela de fim
    if (tela == 2) {
        return;
    }

    int c = pixelParaColuna(x);
    int l = pixelParaLinha(y);
    if (l == -1 || c == -1) {
        return;
    }
    processarClique(l, c);
}

int main(void)
{
    CV::init(&screenWidth, &screenHeight, "trabalho 3 joao pedro");
    CV::run();
    liberarTab();
    return 0;
}