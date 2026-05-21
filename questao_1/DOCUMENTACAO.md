# Bin Packing — Meta-heurística de Busca Local

**Disciplina:** Pesquisa Operacional  
**Linguagem:** C++
**Algoritmo:** Best Fit Decreasing + Busca Local (Relocate / Swap — First Improvement)

---

## Sumário

1. [Representação da Solução](#1-representação-da-solução)
2. [Função de Avaliação](#2-função-de-avaliação)
3. [Estratégia de Busca Local](#3-estratégia-de-busca-local)
4. [Critério de Parada](#4-critério-de-parada)
5. [Compilação e Uso](#5-compilação-e-uso)
6. [Formato da Instância](#6-formato-da-instância)
7. [Saída do Programa](#7-saída-do-programa)
8. [Complexidade Aproximada](#8-complexidade-aproximada)
9. [Vantagens e Limitações](#9-vantagens-e-limitações)
10. [Melhorias Futuras](#10-melhorias-futuras)

---

## 1. Representação da Solução

### Estruturas de dados

| Estrutura  | Campos principais | Descrição |
|------------|-------------------|-----------|
| `Instance` | `n`, `capacity`, `sizes[]` | Dados estáticos da instância |
| `Bin`      | `items[]`, `load` | Um recipiente com seus itens e carga atual |
| `Solution` | `bins[]`          | Vetor de bins — a solução completa |

### Detalhes de projeto

A solução é representada **diretamente pelos bins**: cada `Bin` contém um `std::vector<int>` com os **índices** dos itens alocados e um `double load` que mantém a carga acumulada (evitando recalcular a cada acesso).

Essa representação é preferida à alternativa "item → id_do_bin" porque:

- Facilita checar se um bin ficou **vazio** após um relocate (condição de melhora).
- Torna os movimentos de vizinhança (relocate e swap) operações diretas de inserção/remoção nos vetores.
- Permite imprimir o conteúdo detalhado de cada bin ao final.

O método `compact()` remove bins vazios após movimentos, mantendo o índice do vetor sempre consistente com o número real de bins usados.

---

## 2. Função de Avaliação

```
f(sol) = número de bins não-vazios = sol.bins.size()  (após compact)
```

O objetivo é **minimizar** `f(sol)`.

- **Custo:** O(k) para percorrer os bins (ou O(1) se mantido sempre compacto).
- **Qualidade:** um bin completamente esvaziado representa uma redução direta de 1 unidade no valor objetivo.
- **Critério de melhora:** um movimento é considerado uma **melhora** se e somente se resulta em pelo menos um bin vazio (reduzindo `f` em 1 ou mais).

---

## 3. Estratégia de Busca Local

### Solução Inicial — Best Fit Decreasing (BFD)

Antes da busca local, constrói-se uma solução inicial gulosa:

1. Ordena os itens em **ordem decrescente** de tamanho.
2. Para cada item, insere-o no bin com **maior carga** que ainda o comporte (*best fit*).
3. Se nenhum bin disponível, abre um novo.

BFD é empiricamente superior ao First Fit Decreasing em instâncias diversas, pois tende a deixar menos espaço desperdiçado nos bins.

### Vizinhança Composta

A busca local utiliza **dois movimentos**:

#### Movimento 1 — Relocate

```
Dado: item i ∈ bin_src
Ação: mover i para bin_dst (diferente de bin_src)
Condição: load(bin_dst) + size(i) ≤ capacity
Aceita se: bin_src fica vazio após a remoção
```

O movimento é aceito apenas se esvaziar o bin de origem, o que reduz `f` diretamente. Caso contrário, o movimento é desfeito.

#### Movimento 2 — Swap

```
Dado: item a ∈ bin_i, item b ∈ bin_j  (i ≠ j)
Ação: trocar a e b de bins
Condição: load(bin_i) - size(a) + size(b) ≤ capacity
          load(bin_j) - size(b) + size(a) ≤ capacity
Aceita se: algum dos bins fica vazio após a troca
```

O swap por si só raramente esvazia um bin, mas **reorganiza as cargas** de forma que futuros relocates se tornem possíveis. Na implementação atual, o swap é aceito imediatamente se esvaziar algum bin.

### Política de Aceitação — First Improvement

O algoritmo **aceita e aplica o primeiro movimento que melhore** a solução (reduza `f`), reiniciando a varredura da vizinhança. Isso contrasta com *Best Improvement*, que percorreria toda a vizinhança para escolher o melhor movimento.

**Justificativa da escolha:**
- *First Improvement* converge mais rápido por iteração.
- Para o Bin Packing, qualquer redução de bin é já uma melhora máxima de passo (Δf = 1), então não há vantagem em continuar buscando após encontrar a primeira.

### Pseudo-código

```
solução ← BFD(instância)
enquanto tempo_restante > 0:
    melhora ← false
    para cada par (bin_i, bin_j):
        para cada item em bin_i:
            se Relocate(item, bin_i → bin_j) esvazia bin_i:
                aplicar; melhora ← true; reiniciar
        para cada (item_a ∈ bin_i, item_b ∈ bin_j):
            se Swap viável e esvazia algum bin:
                aplicar; melhora ← true; reiniciar
    se não melhora:
        parar  ← ótimo local atingido
retornar solução
```

---

## 4. Critério de Parada

O programa recebe via linha de comando:

- `<arquivo_instancia>` — caminho para o arquivo de entrada
- `<tempo_limite>` — limite em segundos (tipo `double`)

O algoritmo executa até que **uma das condições** seja satisfeita:

1. O tempo limite é atingido (`elapsed ≥ timeLimit`).
2. Um ótimo local é alcançado (nenhum movimento na vizinhança melhora a solução).

Caso (2) geralmente ocorre muito antes de (1) nesta implementação sem perturbação. Para aproveitar melhor o tempo disponível, seria necessário adicionar um mecanismo de **reinício com perturbação** (ver seção 10).

---

## 5. Compilação e Uso

### Compilar

```bash
g++ -O2 -std=c++17 -o binpacking binpacking.cpp
```

### Executar

```bash
./binpacking instancia.txt 60
```

onde `60` é o tempo limite em segundos.

---

## 6. Formato da Instância

```
<n>           ← número de itens
<capacidade>  ← capacidade de cada bin (ex: 1.0 ou 100)
<s_1>         ← tamanho do item 1
<s_2>         ← tamanho do item 2
...
<s_n>         ← tamanho do item n
```

**Exemplo (`instancia.txt`):**

```
5
1.0
0.42
0.70
0.15
0.55
0.80
```

---

## 7. Saída do Programa

```
Instância carregada: 20 itens, capacidade = 1
Solução inicial (BFD): 12 bins
  [iter 1 | t=0.00s] Melhora: 11 bins
  [iter 2 | t=0.00s] Melhora: 10 bins
  [iter 3] Ótimo local atingido.

╔══════════════════════════════════════════╗
║         SOLUÇÃO FINAL — BIN PACKING      ║
╠══════════════════════════════════════════╣
║  Bins utilizados  :    10                ║
║  Tempo de execução:    0.001 s           ║
╚══════════════════════════════════════════╝

Bin   1  [carga = 1.0000 / 1.0000]  itens: 7(0.10) 11(0.90)
Bin   2  [carga = 0.9900 / 1.0000]  itens: 2(0.70) 9(0.29)
...
Função objetivo (bins): 10
Iterações de busca local: 3
Tempo total: 0.001 s
```

---

## 8. Complexidade Aproximada

| Fase | Complexidade |
|------|-------------|
| BFD (solução inicial) | O(n log n + n·k) |
| Relocate (por iteração) | O(k² · n̄) onde n̄ = n/k médio |
| Swap (por iteração) | O(k² · n̄²) |
| Total (t iterações) | O(n log n + t · k² · n̄²) |

Na prática, `t` é pequeno (o ótimo local é atingido rapidamente), e BFD já produz soluções próximas do ótimo para instâncias bem comportadas.

---

## 9. Vantagens e Limitações

### ✅ Vantagens

- **Simplicidade:** código direto, fácil de entender e modificar.
- **BFD de alta qualidade:** a solução inicial já é competitiva.
- **Determinístico:** resultados reproduzíveis sem semente aleatória.
- **Baixo overhead:** sem estruturas de dados complexas.
- **Extensível:** fácil adicionar novos movimentos à vizinhança.

### ⚠️ Limitações

- **Ótimos locais:** sem mecanismo de escape, o algoritmo para ao primeiro ótimo local, que pode estar longe do ótimo global.
- **Tempo subutilizado:** para a maioria das instâncias, o ótimo local é atingido muito antes do limite de tempo.
- **Swap limitado:** a versão atual aceita swap apenas se esvaziar um bin imediatamente, o que é restritivo.
- **Escalabilidade:** para instâncias com milhares de itens, a complexidade quadrática do swap pode ser cara.

---

## 10. Melhorias Futuras

### 1. ILS — Iterated Local Search
Ao atingir ótimo local, **perturbar** a solução (ex: mover aleatoriamente 2–3 itens entre bins) e reiniciar a busca local. Aproveita o tempo limite disponível.

### 2. Simulated Annealing
Aceitar movimentos piores com probabilidade `e^{-Δf/T}`, diminuindo `T` (temperatura) ao longo do tempo. Escapa de ótimos locais de forma controlada.

### 3. Tabu Search
Manter uma **lista tabu** com movimentos recentemente aplicados, proibindo-os temporariamente. Força exploração de regiões novas do espaço de soluções.

### 4. VNS — Variable Neighborhood Search
Ao esgotar a vizinhança atual, mudar automaticamente para uma vizinhança maior (ex: mover 2 itens em vez de 1). Combina diversificação e intensificação.

### 5. Swap neutro como preparação
Aceitar swaps **neutros** (que não esvaziam bins imediatamente) para reorganizar cargas e viabilizar relocates subsequentes — aumenta significativamente o poder da busca local.

### 6. Paralelismo (OpenMP)
Explorar vizinhanças em múltiplas threads para acelerar a busca em instâncias grandes:

```cpp
#pragma omp parallel for schedule(dynamic)
for (int src = 0; src < nbins; ++src) { ... }
```

---

*Implementação desenvolvida para fins acadêmicos. Compilado e testado com g++ 12, C++17, Linux.*
