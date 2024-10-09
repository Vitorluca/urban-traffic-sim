LINK: [Youtube](https://www.youtube.com/watch?v=wHFvSEN3LUM)

# urban-traffic-sim
Simulador de controle de tráfego urbano

# install process

## update

```
sudo apt update
```

## install dependencias

```
sudo apt install git make gcc g++ cmake
```

## Buid do projeto

## Install library
```
$ sudo apt-get install libc6-dev-i386
```
## Build and Run 
```
make
```

```
./build/FreeRTOS-ubuntu
```

# Simulador de Controle de Tráfego Urbano

Este projeto implementa um simulador de controle de tráfego utilizando o FreeRTOS para gerenciar a sincronização entre cruzamentos, semáforos e veículos. O código simula o fluxo de veículos em uma rede urbana com quatro cruzamentos interligados, onde cada cruzamento contém quatro semáforos e as vias podem ser Norte-Sul (NS) ou Leste-Oeste (EW).

# Ilustração da rede de cruzamentos com distâncias

```python
[ Cruzamento A ] --500m-- [ Cruzamento B ]
       |                         |
     500m                      500m
       |                         |
[ Cruzamento C ] --500m-- [ Cruzamento D ]
```



## Estrutura do Código

### Definições e Tipos

- `NUM_CRUZAMENTOS`: Define o número de cruzamentos no sistema (4).
- `NUM_VEICULOS`: Define o número de veículos que serão simulados (10).
- `DISTANCIA_CRUZAMENTO`: Distância entre cruzamentos, utilizada para calcular o tempo de percurso de cada veículo (500 metros).

### Estruturas

- `semaforo_t`: Representa um semáforo, contendo um identificador único (`id`), o estado do semáforo (verde ou vermelho) e o tempo de mudança de estado. Um mutex (`key_semaforo`) é utilizado para garantir o controle do semáforo.
- `cruzamento_t`: Define um cruzamento, que possui quatro semáforos e permissões para diferentes movimentos dos veículos (em frente, conversão à esquerda e à direita).
- `veiculo_t`: Estrutura que modela um veículo, contendo seu identificador, o cruzamento que está tentando atravessar, o tipo de movimento (esquerda, direita, frente), a velocidade e o tempo estimado para atravessar.

### Funções

- **`criarCruzamentos`**: Inicializa os cruzamentos e semáforos, criando as permissões de movimento para as vias NS e EW. Também cria tarefas FreeRTOS para controlar cada cruzamento.
  
- **`vCruzamentoTask`**: Função responsável pelo controle de um cruzamento. Ela alterna as permissões de movimento para os veículos nas vias NS e EW, além de permitir a conversão à direita. Cada fase dura 30 segundos.

- **`calcularTempoPercurso`**: Calcula o tempo que um veículo leva para percorrer a distância entre cruzamentos com base na sua velocidade.

- **`vVeiculoTask`**: Simula o comportamento de um veículo. A tarefa gera uma velocidade aleatória e calcula o tempo de percurso até o próximo cruzamento. Dependendo do movimento que o veículo vai fazer (esquerda, direita ou frente), ele verifica se o semáforo correspondente está aberto e, em seguida, atravessa o cruzamento.

### Fluxo Principal (`main`)

1. **Inicialização**: O código começa criando os cruzamentos e as tarefas associadas a cada cruzamento.
2. **Simulação de Veículos**: Veículos são criados e atribuídos a cruzamentos de forma aleatória. Cada veículo executa uma tarefa que simula o seu movimento e interação com os semáforos nos cruzamentos.
3. **Agendador FreeRTOS**: O agendador do FreeRTOS é iniciado para executar as tarefas dos cruzamentos e dos veículos.

## Como Funciona

- Cada cruzamento tem quatro semáforos, controlados por tarefas que alternam entre as fases NS e EW. Durante cada fase, veículos podem seguir em frente ou virar à esquerda, dependendo da via.
- Os veículos são simulados como tarefas separadas, onde cada um decide de forma aleatória se vai seguir em frente, virar à esquerda ou à direita ao se aproximar de um cruzamento.
- Mutexes e binários são utilizados para controlar o acesso simultâneo aos semáforos, evitando que múltiplos veículos tentem atravessar o cruzamento ao mesmo tempo.

