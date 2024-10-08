#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define NUM_CRUZAMENTOS 4
#define NUM_VEICULOS 4
#define DISTANCIA_CRUZAMENTO 500 // metros

typedef struct {
    char id;                     // Identificador único do semáforo
    bool estado;                 // Estado do semáforo (0 = vermelho, 1 = verde)
    int time_green_red;         // Tempo para vermelho e para o verde para mudar de estado (em segundos)
    SemaphoreHandle_t key_semaforo;    // Mutex para controle de acesso ao semáforo
} semaforo_t;

typedef struct {
    char id;                    // Identificador único do cruzamento
    semaforo_t semaforos[4];      // Semáforos de cada cruzamento
    SemaphoreHandle_t NS_Straight;           // Permissão para seguir em frente na via NS
    SemaphoreHandle_t EW_Straight;           // Permissão para seguir em frente na via EW
    SemaphoreHandle_t NS_Left;               // Permissão para conversão à esquerda na via NS
    SemaphoreHandle_t EW_Left;               // Permissão para conversão à esquerda na via EW
    SemaphoreHandle_t XX_Straight;           // Permissão para conversão à direita na via XX
} cruzamento_t;

typedef struct {
    int id;                 // Identificador do veículo
    cruzamento_t *cruzamento;  // Cruzamento que o veículo está tentando atravessar
    char movimento;         // 'L' para esquerda, 'R' para direita, 'F' para frente
    float velocidade;       // Velocidade do veículo em km/h
    int tempo_percurso;     // Tempo de percurso em segundos
} veiculo_t;

// Prototipação das funções
void vCruzamentoTask(void *pvParameters);
void vVeiculoTask(void *pvParameters);
void criarCruzamentos(void);
float calcularTempoPercurso(float velocidade);
char obterFaseSemaforica(cruzamento_t *cruzamento);
cruzamento_t* selecionarProximoCruzamento(cruzamento_t *atual);

extern void vAssertCalled(unsigned long ulLine, const char * const pcFileName); //funcao acerções??
void vApplicationIdleHook(void); //funcao ocioso

cruzamento_t cruzamentos[NUM_CRUZAMENTOS]; // cria um vetor de cruzamentos

void vAssertCalled(unsigned long ulLine, const char * const pcFileName) {
    // Loop infinito em caso de falha
    while (1) {
        
    }
}

void vApplicationIdleHook(void) {
    // função de ações durante o estado ocioso
}

// Função que cria as tarefas dos cruzamentos
void criarCruzamentos() {
    // Inicializando cruzamentos e semáforos
    for (int i = 0; i < NUM_CRUZAMENTOS; i++) {
        cruzamentos[i].id = 'A' + i; // A, B, C, D
        for (int j = 0; j < 4; j++) {
            cruzamentos[i].semaforos[j].id = j; // Semáforos 0, 1, 2, 3
            cruzamentos[i].semaforos[j].estado = 0; // Inicialmente vermelho
            cruzamentos[i].semaforos[j].time_green_red = 30; // 30 segundos
            cruzamentos[i].semaforos[j].key_semaforo = xSemaphoreCreateMutex(); // Cria um mutex para cada semáforo
        }
        cruzamentos[i].NS_Straight = xSemaphoreCreateBinary();
        cruzamentos[i].EW_Straight = xSemaphoreCreateBinary();
        cruzamentos[i].NS_Left = xSemaphoreCreateBinary();
        cruzamentos[i].EW_Left = xSemaphoreCreateBinary();
        cruzamentos[i].XX_Straight = xSemaphoreCreateBinary();

        // Cria a tarefa do cruzamento
        xTaskCreate(vCruzamentoTask, 
                    "Cruzamento Task", 
                    configMINIMAL_STACK_SIZE, 
                    &cruzamentos[i],  // Passa o cruzamento atual como parâmetro
                    1, 
                    NULL);
    }
}

// Função de tarefa que controla cada cruzamento
void vCruzamentoTask(void *pvParameters) {
    cruzamento_t *cruzamento = (cruzamento_t *)pvParameters;

    while (1) {
        // Fase NS-Straight e EW-Left
        printf("Cruzamento %c: Fase NS-Straight e EW-Left\n", cruzamento->id);
        xSemaphoreGive(cruzamento->NS_Straight);
        xSemaphoreGive(cruzamento->EW_Left);
        vTaskDelay(pdMS_TO_TICKS(10000)); // 10 segundos

        //bloqueia os semáforos
        xSemaphoreTake(cruzamento->NS_Straight,0);
        xSemaphoreTake(cruzamento->EW_Left,0);

        // Fase EW-Straight e NS-Left
        printf("Cruzamento %c: Fase EW-Straight e NS-Left\n", cruzamento->id);
        xSemaphoreGive(cruzamento->EW_Straight);
        xSemaphoreGive(cruzamento->NS_Left);
        vTaskDelay(pdMS_TO_TICKS(10000)); // 10 segundos

        //bloqueia os semáforos
        xSemaphoreTake(cruzamento->EW_Straight,0);
        xSemaphoreTake(cruzamento->NS_Left,0);

        // Permite a conversão à direita (XX_Straight)
        printf("Cruzamento %c: Permissão para conversão à direita\n", cruzamento->id);
        xSemaphoreGive(cruzamento->XX_Straight);
        vTaskDelay(pdMS_TO_TICKS(10000)); // 10 segundos

        //bloqueia os semáforos
        xSemaphoreTake(cruzamento->XX_Straight,0);
    }
}

// Função que calcula o tempo de percurso com base na velocidade
float calcularTempoPercurso(float velocidade) {
    // Converte velocidade de km/h para m/s (1 km/h = 1000 m / 3600 s)
    float velocidade_ms = (velocidade * 1000) / 3600;
    // Tempo = Distância / Velocidade
    return DISTANCIA_CRUZAMENTO / velocidade_ms;
}

// Função que obtém a fase semafórica atual de um cruzamento
char obterFaseSemaforica(cruzamento_t *cruzamento) {
    if (xSemaphoreTake(cruzamento->NS_Straight, 0)) {
        xSemaphoreGive(cruzamento->NS_Straight);
        return 'N'; // NS-Straight e EW-Left
    } else if (xSemaphoreTake(cruzamento->EW_Straight, 0)) {
        xSemaphoreGive(cruzamento->EW_Straight);
        return 'E'; // EW-Straight e NS-Left
    } else if (xSemaphoreTake(cruzamento->XX_Straight, 0)) {
        xSemaphoreGive(cruzamento->XX_Straight);
        return 'X'; // XX-Straight (direita)
    }
    return 'U'; // Indeterminado
}

// Função que seleciona o próximo cruzamento com base na localização atual
cruzamento_t* selecionarProximoCruzamento(cruzamento_t *atual) {
    cruzamento_t *proximo = NULL;
    switch (atual->id) {
        case 'A':
            // Pode se mover para B (direita) ou C (baixo)
            proximo = (rand() % 2 == 0) ? &cruzamentos[1] : &cruzamentos[2];
            break;
        case 'B':
            // Pode se mover para A (esquerda) ou D (baixo)
            proximo = (rand() % 2 == 0) ? &cruzamentos[0] : &cruzamentos[3];
            break;
        case 'C':
            // Pode se mover para A (cima) ou D (direita)
            proximo = (rand() % 2 == 0) ? &cruzamentos[0] : &cruzamentos[3];
            break;
        case 'D':
            // Pode se mover para B (cima) ou C (esquerda)
            proximo = (rand() % 2 == 0) ? &cruzamentos[1] : &cruzamentos[2];
            break;
    }
    return proximo;
}

// Função de tarefa que representa um veículo
void vVeiculoTask(void *pvParameters) {
    veiculo_t *veiculo = (veiculo_t *)pvParameters;

    while (1) {
        // Simula o movimento do veículo
        // Determina uma velocidade aleatória
        if (veiculo->cruzamento->id == 'A' || veiculo->cruzamento->id == 'B') {
            // Vias Norte-Sul
            veiculo->velocidade = 60 + (rand() % 11 - 5); // Variação de ±5 km/h
        } else {
            // Vias Leste-Oeste
            veiculo->velocidade = 50 + (rand() % 11 - 5); // Variação de ±5 km/h
        }

        // Calcula o tempo de percurso
        veiculo->tempo_percurso = calcularTempoPercurso(veiculo->velocidade);
        printf("Veículo %d se aproximando do cruzamento %c para mover %c com velocidade %.2f km/h. Tempo de percurso: %d segundos\n", 
               veiculo->id, veiculo->cruzamento->id, veiculo->movimento, veiculo->velocidade, veiculo->tempo_percurso);

        // Verifica o semáforo e se o movimento é permitido
        while (1) {
            char fase = obterFaseSemaforica(veiculo->cruzamento);
            if ((veiculo->movimento == 'F' && fase == 'N') ||
                (veiculo->movimento == 'L' && fase == 'E') ||
                (veiculo->movimento == 'R' && fase == 'X')) {
                // Movimento permitido na fase atual
                if (veiculo->movimento == 'F' && xSemaphoreTake(veiculo->cruzamento->NS_Straight, portMAX_DELAY)) {
                    printf("Veículo %d atravessou o cruzamento %c em frente\n", 
                           veiculo->id, veiculo->cruzamento->id);
                    vTaskDelay(pdMS_TO_TICKS(veiculo->tempo_percurso * 1000)); // Atravessa o cruzamento
                    xSemaphoreGive(veiculo->cruzamento->NS_Straight);
                    break;
                } else if (veiculo->movimento == 'L' && xSemaphoreTake(veiculo->cruzamento->NS_Left, portMAX_DELAY)) {
                    printf("Veículo %d virou à esquerda no cruzamento %c\n", 
                           veiculo->id, veiculo->cruzamento->id);
                    vTaskDelay(pdMS_TO_TICKS(veiculo->tempo_percurso * 1000)); // Atravessa o cruzamento
                    xSemaphoreGive(veiculo->cruzamento->NS_Left);
                    break;
                } else if (veiculo->movimento == 'R' && xSemaphoreTake(veiculo->cruzamento->XX_Straight, portMAX_DELAY)) {
                    printf("Veículo %d virou à direita no cruzamento %c\n", 
                           veiculo->id, veiculo->cruzamento->id);
                    vTaskDelay(pdMS_TO_TICKS(veiculo->tempo_percurso * 1000)); // Atravessa o cruzamento
                    xSemaphoreGive(veiculo->cruzamento->XX_Straight);
                    break;
                }
            } else {
                // Movimento não permitido, aguarda a mudança de fase
                vTaskDelay(pdMS_TO_TICKS(1000)); // Aguarda 1 segundo antes de verificar novamente
            }
        }

        // Seleciona o próximo cruzamento ou finaliza a jornada
        if (rand() % 2 == 0 && veiculo->movimento != 'F') {
            veiculo->cruzamento = selecionarProximoCruzamento(veiculo->cruzamento);
            printf("Veículo %d se dirigindo ao próximo cruzamento %c\n", veiculo->id, veiculo->cruzamento->id);
        } else {
            printf("Veículo %d finalizou sua jornada\n", veiculo->id);
            vTaskDelete(NULL); // Finaliza a tarefa do veículo
        }

        // Espera um tempo aleatório antes de gerar o próximo movimento
        vTaskDelay(pdMS_TO_TICKS(rand() % 3000 + 2000)); // Espera entre 2 e 5 segundos
    }
}

// Função principal
int main(void) {

    veiculo_t veiculos[NUM_VEICULOS]; // Cria um vetor de veículos

    srand(time(NULL)); // Inicializa o gerador de números aleatórios

    criarCruzamentos(); // Cria os cruzamentos e as tarefas

    // Cria veículos
    for (int i = 0; i < NUM_VEICULOS; i++) {
        veiculos[i].id = i + 1; // ID do veículo começa em 1
        veiculos[i].cruzamento = &cruzamentos[rand() % NUM_CRUZAMENTOS]; // Atribui um cruzamento aleatório
        veiculos[i].movimento = (rand() % 3) == 0 ? 'L' : (rand() % 3) == 1 ? 'R' : 'F'; // Movimento aleatório
    
        // Cria a tarefa passando o veículo do array como parâmetro
        xTaskCreate(vVeiculoTask, 
            "Veiculo Task", 
            configMINIMAL_STACK_SIZE, 
            &veiculos[i], // Passa o veículo como parâmetro
            2, 
            NULL);
    }

    vTaskStartScheduler(); // Inicia o agendador FreeRTOS

    return 0; // Este ponto nunca deve ser alcançado
}