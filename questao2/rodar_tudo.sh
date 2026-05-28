#!/bin/bash
# ============================================================
#  Compila e roda todos os problemas de pesquisa operacional
#
#  Uso:
#    ./rodar_tudo.sh              # compila e roda tudo
#    ./rodar_tudo.sh racao        # roda só um problema
#    ./rodar_tudo.sh questao1     # roda só o bin packing
# ============================================================

# ── Detecta onde o CPLEX está instalado ──────────────────────
if [ -d "/home/micaelsv/CPLEX_Studio129" ]; then
    CPLEX_DIR="/home/micaelsv/CPLEX_Studio129"
elif [ -d "/opt/ibm/ILOG/CPLEX_Studio2211" ]; then
    CPLEX_DIR="/opt/ibm/ILOG/CPLEX_Studio2211"
elif [ -d "/opt/ibm/ILOG/CPLEX_Studio129" ]; then
    CPLEX_DIR="/opt/ibm/ILOG/CPLEX_Studio129"
else
    echo "ERRO: CPLEX nao encontrado."
    echo "Edite a variavel CPLEX_DIR neste script."
    exit 1
fi

echo "Usando CPLEX em: $CPLEX_DIR"
echo ""

# Caminho da pasta (funciona no WSL com /mnt/c/...)
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
Q1="$SCRIPT_DIR/questao_1"
Q2="$SCRIPT_DIR/questao2"

# ── Questão 1: Bin Packing (sem CPLEX) ───────────────────────
run_questao1() {
    echo "============================================"
    echo " QUESTÃO 1 — Bin Packing"
    echo "============================================"
    cd "$Q1"
    make clean -s
    make -s
    if [ $? -ne 0 ]; then echo "ERRO na compilação"; return 1; fi
    echo ""
    echo "--- Rodando com binpack1.txt ---"
    ./binpacking_or_instancy binpack1.txt 100
    echo ""
}

# ── Questão 2: Problemas CPLEX ───────────────────────────────
run_questao2() {
    echo "============================================"
    echo " QUESTÃO 2 — Problemas CPLEX"
    echo "============================================"
    cd "$Q2"

    # Ajusta o CPLEX_DIR no Makefile temporariamente
    sed -i "s|CPLEX_DIR = .*|CPLEX_DIR = $CPLEX_DIR|" Makefile

    make clean -s
    echo "Compilando todos os problemas..."
    make all 2>&1 | grep -v "^make\[" | grep -v "^g++"
    if [ $? -ne 0 ]; then echo "ERRO na compilação"; return 1; fi
    echo "Compilação OK!"
    echo ""

    PROBLEMAS=(racao dieta plantio tintas transporte fluxo_maximo escalonamento cobertura mochila clique_maxima padroes facilidades frequencia)

    for P in "${PROBLEMAS[@]}"; do
        if [ -f "./$P" ]; then
            echo "--------------------------------------------"
            echo " Rodando: $P"
            echo "--------------------------------------------"
            ./$P
            echo ""
        fi
    done
}

# ── Roda um problema específico ──────────────────────────────
run_one() {
    PROB=$1
    cd "$Q2"
    sed -i "s|CPLEX_DIR = .*|CPLEX_DIR = $CPLEX_DIR|" Makefile
    make $PROB -s 2>&1
    if [ -f "./$PROB" ]; then
        echo "--- Rodando: $PROB ---"
        ./$PROB
    else
        echo "ERRO: $PROB nao compilou"
    fi
}

# ── Decide o que rodar ───────────────────────────────────────
case "${1:-tudo}" in
    questao1) run_questao1 ;;
    questao2) run_questao2 ;;
    tudo)
        run_questao1
        run_questao2
        ;;
    *)
        # Tenta rodar como nome de problema da questao2
        run_one "$1"
        ;;
esac
