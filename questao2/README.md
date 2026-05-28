# Questão 2 — Problemas de Programação Linear com CPLEX

---

## Problemas implementados

| Arquivo | Problema | Tipo | Resultado esperado |
|---------|----------|------|-------------------|
| `racao.cpp` | Problema da Ração | LP | Lucro máximo: R$ 74.444,44 |
| `dieta.cpp` | Problema da Dieta | LP | Custo mínimo: 179,00 |
| `plantio.cpp` | Problema do Plantio | LP | Lucro máximo: R$ 4.361.904,76 |
| `tintas.cpp` | Empresa de Tintas | LP | Custo mínimo: R$ 1.458,33 |
| `transporte.cpp` | Problema do Transporte | LP | Custo mínimo: 1920,00 |
| `fluxo_maximo.cpp` | Fluxo Máximo em Rede | LP | Fluxo máximo: 13 |
| `escalonamento.cpp` | Escalonamento de Enfermeiras | ILP | 8 enfermeiras |
| `cobertura.cpp` | Cobertura de Escolas | ILP Binário | 2 escolas (B3 e B6) |
| `mochila.cpp` | Problema da Mochila 0/1 | ILP Binário | Valor máximo: 25 |
| `clique_maxima.cpp` | Clique Máxima em Grafo | ILP Binário | Clique de 4 vértices |
| `padroes.cpp` | Problema dos Padrões | ILP | Lucro: 10.522 u. |
| `facilidades.cpp` | Localização de Facilidades | ILP Binário | Custo mínimo: 37,00 |
| `frequencia.cpp` | Atribuição de Frequências | ILP | 3 frequências |

---

## Pré-requisitos

- **Linux** ou **WSL** (Windows Subsystem for Linux)
- **g++** versão 11 ou superior
- **IBM ILOG CPLEX Studio** 12.9, 20.x ou 22.x

---

## Como compilar e rodar

### Detecção automática do CPLEX

O `Makefile` busca automaticamente o CPLEX nos seguintes caminhos:

```
$HOME/CPLEX_Studio2211, $HOME/CPLEX_Studio221, $HOME/CPLEX_Studio201,
$HOME/CPLEX_Studio129, $HOME/CPLEX_Studio128
/opt/ibm/ILOG/CPLEX_Studio2211, /opt/ibm/ILOG/CPLEX_Studio129, ...
/opt/CPLEX_Studio2211, /usr/local/CPLEX_Studio2211, ...
```

Se o seu CPLEX está em algum desses lugares, **não precisa configurar nada**.

Para conferir o que foi detectado:
```bash
make info
```

Caso o caminho seja diferente, passe explicitamente:
```bash
make CPLEX_DIR=/seu/caminho/CPLEX_Studio2211
```

---

### Comandos principais

```bash
# Abre o WSL (no Windows: digite "wsl" no PowerShell)
wsl

# Navega até a pasta
cd /mnt/c/Users/SEU_USUARIO/OneDrive/.../questao2

# Compila todos os 13 problemas
make all

# Roda um problema individual
./racao
./dieta
./mochila
# ... etc

# Compila e executa de uma vez
make run_racao
make run_mochila

# Mostra os caminhos detectados do CPLEX
make info

# Remove os executáveis
make clean

# Mostra a ajuda
make help
```

---

### Atalho: rodar tudo de uma vez

Use o script `rodar_tudo.sh` (que está na pasta pai):

```bash
cd ..
chmod +x rodar_tudo.sh
./rodar_tudo.sh questao2    # compila e executa todos os 13 problemas
./rodar_tudo.sh racao       # roda só o problema da ração
```

---

## Resultados validados

Todos os 13 problemas retornam `Status: Optimal` com os valores esperados:

```
[OK]   racao           -> R$ 74444.44      (5555,56 kg AMGS + 1111,11 kg RE)
[OK]   dieta           -> 179.0000          (5 ing.5 + 2 ing.6)
[OK]   plantio         -> R$ 4361904.76    (proporção 67,86% por fazenda)
[OK]   tintas          -> R$ 1458.3333     (só SolA e SolB)
[OK]   transporte      -> 1920.00           (oferta = demanda exata)
[OK]   fluxo_maximo    -> 13.00             (corte mínimo)
[OK]   escalonamento   -> 8 enfermeiras
[OK]   cobertura       -> 2 escolas         (B3 e B6 cobrem todos)
[OK]   mochila         -> Valor 25          (itens 0, 2, 3, peso 10/10)
[OK]   clique_maxima   -> Clique 4 vértices {0,1,2,3}
[OK]   padroes         -> Lucro 10522 u.    (211 latinhas)
[OK]   facilidades     -> 37.00             (Dep.2 atende todos)
[OK]   frequencia      -> 3 frequências
```

---

## Compilar manualmente (sem Makefile)

Se preferir não usar o Makefile, o comando de compilação é:

```bash
# Substitua /caminho/CPLEX pelo caminho da sua instalação
CPLEX=/home/SEU_USUARIO/CPLEX_Studio129

g++ -O2 -std=c++17 -DIL_STD \
    -I$CPLEX/cplex/include \
    -I$CPLEX/concert/include \
    racao.cpp \
    -L$CPLEX/cplex/lib/x86-64_linux/static_pic \
    -L$CPLEX/concert/lib/x86-64_linux/static_pic \
    -lilocplex -lcplex -lconcert -lm -lpthread -ldl \
    -o racao

./racao
```

Troque `racao.cpp` / `racao` pelo nome do problema que quiser compilar.
