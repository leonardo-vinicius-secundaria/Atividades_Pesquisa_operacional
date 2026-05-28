# ============================================================
#  Makefile — Problemas de Pesquisa Operacional com CPLEX
# ============================================================
#
#  DETECÇÃO AUTOMÁTICA DO CPLEX
#  ──────────────────────────────────────────────────────────
#  O Makefile tenta encontrar o CPLEX nos caminhos mais comuns.
#  Se ele não encontrar, defina manualmente:
#
#      make CPLEX_DIR=/caminho/para/CPLEX_Studio
#
#  Ou edite a linha CPLEX_DIR_OVERRIDE abaixo.
# ============================================================

# >>> Sobrescreva aqui se quiser forçar um caminho específico:
CPLEX_DIR_OVERRIDE =

# ── Lista de caminhos comuns onde o CPLEX pode estar ────────
CPLEX_CANDIDATES = \
  $(HOME)/CPLEX_Studio2211 \
  $(HOME)/CPLEX_Studio221 \
  $(HOME)/CPLEX_Studio201 \
  $(HOME)/CPLEX_Studio129 \
  $(HOME)/CPLEX_Studio128 \
  /opt/ibm/ILOG/CPLEX_Studio2211 \
  /opt/ibm/ILOG/CPLEX_Studio221 \
  /opt/ibm/ILOG/CPLEX_Studio201 \
  /opt/ibm/ILOG/CPLEX_Studio129 \
  /opt/ibm/ILOG/CPLEX_Studio128 \
  /opt/CPLEX_Studio2211 \
  /opt/CPLEX_Studio129 \
  /usr/local/CPLEX_Studio2211 \
  /usr/local/CPLEX_Studio129

# ── Detecção automática (a primeira pasta válida vence) ─────
ifneq ($(CPLEX_DIR_OVERRIDE),)
  CPLEX_DIR := $(CPLEX_DIR_OVERRIDE)
else ifneq ($(CPLEX_DIR),)
  # passado via linha de comando: make CPLEX_DIR=...
else
  CPLEX_DIR := $(firstword $(foreach d,$(CPLEX_CANDIDATES),$(if $(wildcard $(d)/cplex/include/ilcplex/ilocplex.h),$(d))))
endif

# ── Validação: aborta se nada foi encontrado ────────────────
ifeq ($(CPLEX_DIR),)
  $(error CPLEX nao encontrado. Defina manualmente: make CPLEX_DIR=/caminho/CPLEX_Studio)
endif

CPLEX_INC   = $(CPLEX_DIR)/cplex/include
CONCERT_INC = $(CPLEX_DIR)/concert/include

# Detecta a arquitetura da pasta de libs (Linux x86-64 ou ARM)
ifneq ($(wildcard $(CPLEX_DIR)/cplex/lib/x86-64_linux/static_pic),)
  ARCH = x86-64_linux
else ifneq ($(wildcard $(CPLEX_DIR)/cplex/lib/x86_64_linux/static_pic),)
  ARCH = x86_64_linux
else
  ARCH = x86-64_linux
endif

CPLEX_LIB   = $(CPLEX_DIR)/cplex/lib/$(ARCH)/static_pic
CONCERT_LIB = $(CPLEX_DIR)/concert/lib/$(ARCH)/static_pic

# ── Compilador e flags ──────────────────────────────────────
CXX      = g++
CXXFLAGS = -O2 -std=c++17 -Wall -m64 -fPIC \
           -DIL_STD \
           -I$(CPLEX_INC) \
           -I$(CONCERT_INC)

LDFLAGS  = -L$(CPLEX_LIB) -L$(CONCERT_LIB) \
           -lilocplex -lcplex -lconcert \
           -lm -lpthread -ldl

# ── Alvos ───────────────────────────────────────────────────
TARGETS = racao dieta plantio tintas transporte fluxo_maximo \
          escalonamento cobertura mochila clique_maxima \
          padroes facilidades frequencia

# ── Regras ──────────────────────────────────────────────────
.PHONY: all clean help info $(addprefix run_,$(TARGETS))

all: $(TARGETS)

info:
	@echo "CPLEX_DIR    = $(CPLEX_DIR)"
	@echo "ARCH         = $(ARCH)"
	@echo "Includes     = $(CPLEX_INC), $(CONCERT_INC)"
	@echo "Libraries    = $(CPLEX_LIB), $(CONCERT_LIB)"

# Regra genérica: cada problema.cpp -> problema (executável)
%: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

# Atalhos run_<problema> = compila e executa
$(addprefix run_,$(TARGETS)): run_%: %
	./$<

clean:
	rm -f $(TARGETS)

# ── Ajuda rápida ────────────────────────────────────────────
help:
	@echo "Alvos disponíveis:"
	@echo "  all            - compila todos os 13 programas (padrão)"
	@echo "  <problema>     - compila um problema (ex: make racao)"
	@echo "  run_<problema> - compila e executa (ex: make run_racao)"
	@echo "  info           - mostra os caminhos detectados do CPLEX"
	@echo "  clean          - remove os executáveis"
	@echo "  help           - mostra esta ajuda"
	@echo ""
	@echo "Problemas: $(TARGETS)"
	@echo ""
	@echo "Para sobrescrever o caminho do CPLEX:"
	@echo "  make CPLEX_DIR=/caminho/CPLEX_Studio2211"
# ============================================================
