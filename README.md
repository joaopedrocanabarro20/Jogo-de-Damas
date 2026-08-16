# ♟️ Jogo de Damas 2D

Um jogo de damas desenvolvido em **C++** como projeto da disciplina de **Lógica e Algoritmos**.

O jogo possui uma interface gráfica 2D baseada em **OpenGL/GLUT e Canvas2D**, permitindo partidas para dois jogadores, com diferentes tamanhos de tabuleiro e regras de movimentação, captura e promoção de peças.

---

## 🎮 Funcionalidades

- Tabuleiro nos tamanhos:
  - 8×8
  - 10×10
  - 12×12
- Dois jogadores: **Azul** e **Vermelho**
- Movimentação das peças na diagonal
- Captura de peças adversárias
- Capturas consecutivas
- Captura obrigatória quando disponível
- Promoção de peça para **dama**
- Movimentação utilizando:
  - ⌨️ Teclado
  - 🖱️ Mouse
- Cursor para seleção das casas do tabuleiro
- Identificação automática do vencedor
- Opção para reiniciar a partida
- Retorno ao menu principal

---

## 🖼️ Sobre o jogo

Ao iniciar o programa, o jogador pode escolher entre três tamanhos de tabuleiro:

```text
┌───────────────────────┐
│        DAMAS          │
│                       │
│  1 → Tabuleiro 8×8    │
│  2 → Tabuleiro 10×10  │
│  3 → Tabuleiro 12×12  │
└───────────────────────┘
