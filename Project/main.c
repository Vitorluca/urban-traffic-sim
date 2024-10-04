#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define NUM_CRUZAMENTOS 4
#define NUM_VEICULOS 10
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

extern void vAssertCalled(unsigned long ulLine, const char * const pcFileName); //funcao acerções??
void vApplicationIdleHook(void); //funcao ocioso

cruzamento_t cruzamentos[NUM_CRUZAMENTOS]; // cria um vetor de cruzamentos

void vAssertCalled(unsigned long ulLine, const char * const pcFileName) {
    // Loop infinito em caso de falha
    while (1) {
        // Você pode adicionar um breakpoint aqui se estiver depurando
    }
}

void vApplicationIdleHook(void) {
    // Esta função pode estar vazia se você não precisar de ações durante o estado ocioso
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

        // Inicializa os semáforos de movimento como livres
        xSemaphoreGive(cruzamentos[i].NS_Straight);
        xSemaphoreGive(cruzamentos[i].EW_Straight);
        xSemaphoreGive(cruzamentos[i].NS_Left);
        xSemaphoreGive(cruzamentos[i].EW_Left);
        xSemaphoreGive(cruzamentos[i].XX_Straight);

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
        vTaskDelay(pdMS_TO_TICKS(30000)); // 30 segundos

        // Fase EW-Straight e NS-Left
        printf("Cruzamento %c: Fase EW-Straight e NS-Left\n", cruzamento->id);
        xSemaphoreGive(cruzamento->EW_Straight);
        xSemaphoreGive(cruzamento->NS_Left);
        vTaskDelay(pdMS_TO_TICKS(30000)); // 30 segundos

        // Permite a conversão à direita (XX_Straight)
        printf("Cruzamento %c: Permissão para conversão à direita\n", cruzamento->id);
        xSemaphoreGive(cruzamento->XX_Straight);
        vTaskDelay(pdMS_TO_TICKS(30000)); // 30 segundos
    }
}

// Função que calcula o tempo de percurso com base na velocidade
float calcularTempoPercurso(float velocidade) {
    // Converte velocidade de km/h para m/s (1 km/h = 1000 m / 3600 s)
    float velocidade_ms = (velocidade * 1000) / 3600;
    // Tempo = Distância / Velocidade
    return DISTANCIA_CRUZAMENTO / velocidade_ms;
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
        if (veiculo->movimento == 'F') {
            // Verifica semáforo NS_Straight
            if (xSemaphoreTake(veiculo->cruzamento->NS_Straight, portMAX_DELAY)) {
                printf("Veículo %d atravessou o cruzamento %c em frente\n", 
                       veiculo->id, veiculo->cruzamento->id);
                vTaskDelay(pdMS_TO_TICKS(veiculo->tempo_percurso * 1000)); // Atravessa o cruzamento
                xSemaphoreGive(veiculo->cruzamento->NS_Straight);
            }
        } else if (veiculo->movimento == 'L') {
            // Verifica semáforo NS_Left
            if (xSemaphoreTake(veiculo->cruzamento->NS_Left, portMAX_DELAY)) {
                printf("Veículo %d virou à esquerda no cruzamento %c\n", 
                       veiculo->id, veiculo->cruzamento->id);
                vTaskDelay(pdMS_TO_TICKS(veiculo->tempo_percurso * 1000)); // Atravessa o cruzamento
                xSemaphoreGive(veiculo->cruzamento->NS_Left);
            }
        } else if (veiculo->movimento == 'R') {
            // Verifica semáforo XX_Straight
            if (xSemaphoreTake(veiculo->cruzamento->XX_Straight, portMAX_DELAY)) {
                printf("Veículo %d virou à direita no cruzamento %c\n", 
                       veiculo->id, veiculo->cruzamento->id);
                vTaskDelay(pdMS_TO_TICKS(veiculo->tempo_percurso * 1000)); // Atravessa o cruzamento
                xSemaphoreGive(veiculo->cruzamento->XX_Straight);
            }
        }

        // Espera um tempo aleatório antes de gerar o próximo veículo
        vTaskDelay(pdMS_TO_TICKS(rand() % 3000 + 2000)); // Espera entre 2 e 5 segundos
    }
}

// Função principal
int main(void) {
    srand(time(NULL)); // Inicializa o gerador de números aleatórios

    criarCruzamentos(); // Cria os cruzamentos e as tarefas

    // Cria veículos
    for (int i = 0; i < NUM_VEICULOS; i++) {
        veiculo_t *veiculo = (veiculo_t *)malloc(sizeof(veiculo_t));
        veiculo->id = i + 1; // ID do veículo começa em 1
        veiculo->cruzamento = &cruzamentos[rand() % NUM_CRUZAMENTOS]; // Atribui um cruzamento aleatório
        veiculo->movimento = (rand() % 3) == 0 ? 'L' : (rand() % 3) == 1 ? 'R' : 'F'; // Movimento aleatório
        xTaskCreate(vVeiculoTask, 
                    "Veiculo Task", 
                    configMINIMAL_STACK_SIZE, 
                    veiculo, // Passa o veículo como parâmetro
                    2, 
                    NULL);
    }

    vTaskStartScheduler(); // Inicia o agendador FreeRTOS

    return 0; // Este ponto nunca deve ser alcançado
}
