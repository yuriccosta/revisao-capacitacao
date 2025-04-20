#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"

#include <stdio.h>
#include <hardware/pio.h>           
#include "hardware/clocks.h"        
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"

#include "animacao_matriz.pio.h" // Biblioteca PIO para controle de LEDs WS2818B

// Definição de constantes
#define LED_PIN_GREEN 11
#define LED_PIN_RED 13

#define JOY_X 27 // Joystick está de lado em relação ao que foi dito no pdf
#define JOY_Y 26
#define BUZZER_A 21
#define BUTTON_PIN_A 5          // Pino GPIO conectado ao botão A
#define LED_COUNT 25            // Número de LEDs na matriz
#define MATRIZ_PIN 7            // Pino GPIO conectado aos LEDs WS2818B
#define max_value_joy 4065.0 // (4081 - 16) que são os valores extremos máximos lidos pelo meu joystick

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C


// Declaração de variáveis globais
PIO pio;
uint sm;
ssd1306_t ssd; // Inicializa a estrutura do display
static volatile uint32_t last_time_button = 0; // Variável para armazenar o tempo do último evento
static volatile uint cor = 0; // Variável para armazenar a cor da borda do display
static volatile uint32_t last_time_alarm = 0; // Variável para armazenar o tempo do último evento de alarme
static volatile bool alarme_ativo = false; // Variável para armazenar o estado do alarme
static volatile uint32_t last_time_message = 0; // Variável para armazenar o tempo do último evento de mensagem


// Matriz para armazenar os desenhos da matriz de LEDs
uint padrao_led[10][LED_COUNT] = {
    {0, 0, 1, 0, 0,
     0, 0, 1, 0, 0,
     1, 1, 1, 1, 1,
     0, 0, 1, 0, 0,
     0, 0, 1, 0, 0,
    }, // Estado seguro
    {0, 0, 2, 0, 0,
     0, 0, 2, 0, 0,
     0, 2, 2, 2, 0,
     2, 2, 2, 2, 2,
     2, 2, 2, 2, 2,
    }, // Estado de alerta
    {
     3, 0, 0, 0, 3,
     0, 3, 0, 3, 0,
     0, 0, 3, 0, 0,
     0, 3, 0, 3, 0,
     3, 0, 0, 0, 3,
    }, // Estado de perigo
};

// Ordem da matriz de LEDS, útil para poder visualizar na matriz do código e escrever na ordem correta do hardware
int ordem[LED_COUNT] = {0, 1, 2, 3, 4, 9, 8, 7, 6, 5, 10, 11, 12, 13, 14, 19, 18, 17, 16, 15, 20, 21, 22, 23, 24};  


// Rotina para definição da intensidade de cores do led
uint32_t matrix_rgb(unsigned r, unsigned g, unsigned b){
    return (g << 24) | (r << 16) | (b << 8);
}

// Rotina para desenhar o padrão de LED
void display_desenho(int number){
    uint32_t valor_led;

    for (int i = 0; i < LED_COUNT; i++){
        // Define a cor do LED de acordo com o padrão
        if (padrao_led[number][ordem[24 - i]] == 1){
            valor_led = matrix_rgb(0, 10, 0); // Verde
        } else if (padrao_led[number][ordem[24 - i]] == 2){
            valor_led = matrix_rgb(30, 10, 0); // Amarelo
        } else if (padrao_led[number][ordem[24 - i]] == 3){
            valor_led = matrix_rgb(30, 0, 0); // Vermelho
        }else{
            valor_led = matrix_rgb(0, 0, 0); // Desliga o LED
        }
        // Atualiza o LED
        pio_sm_put_blocking(pio, sm, valor_led);
    }
}

// Função para iniciar o buzzer
void iniciar_buzzer(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin); // Obtém o slice correspondente

    pwm_set_clkdiv(slice_num, 125); // Define o divisor de clock
    pwm_set_wrap(slice_num, 1000);  // Define o valor máximo do PWM

    pwm_set_gpio_level(pin, 10); //Para um som mais baixo foi colocado em 10
    pwm_set_enabled(slice_num, true);
}

// Função para parar o buzzer
void parar_buzzer(uint pin) {
    uint slice_num = pwm_gpio_to_slice_num(pin); // Obtém o slice correspondente
    pwm_set_enabled(slice_num, false); // Desabilita o slice PWM
    gpio_put(pin, 0); // Coloca o pino em nível para garantir que o buzzer está desligado
}

void configuraGPIO(){
    // Configuração do LED RGB

    gpio_init(LED_PIN_RED);
    gpio_set_dir(LED_PIN_RED, GPIO_OUT);

    gpio_init(LED_PIN_GREEN);
    gpio_set_dir(LED_PIN_GREEN, GPIO_OUT);

    // Configura os botões
    gpio_init(BUTTON_PIN_A);
    gpio_set_dir(BUTTON_PIN_A, GPIO_IN);
    gpio_pull_up(BUTTON_PIN_A);
}


void configura_i2c(){
    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA); // Pull up the data line
    gpio_pull_up(I2C_SCL); // Pull up the clock line
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd); // Configura o display
    ssd1306_send_data(&ssd); // Envia os dados para o display

    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
}

void display_quadrado(uint16_t vrx_value, uint16_t vry_value){
    // x e y são o centro do quadrado
    uint16_t x = (vrx_value * WIDTH) / max_value_joy; // Calcula a posição do eixo x 
    uint16_t y = HEIGHT - ((vry_value * HEIGHT) / max_value_joy); // Calcula a posição do eixo y

    // Limita a posição do quadrado para não ultrapassar as bordas do retangulo
    if (x > 120){ 
        x = 120;
    } else if (x < 8){ 
        x = 8;
    }

    if (y > 56){ 
        y = 56; 
    } else if (y < 8){
        y = 8;
    }

    x = x - 4; // Ajusta a posição do quadrado para passar para a função de desenho com o x sendo o centro do quadrado
    y = y - 4; // Ajusta a posição do quadrado para passar para a função de desenho com o y sendo o centro do quadrado
    
    ssd1306_fill(&ssd, cor); // Limpa o display
    ssd1306_rect(&ssd, 3, 3, 122, 58, !cor, cor); // Desenha um retângulo
    ssd1306_rect(&ssd, y, x, 8, 8, true, true); // Desenha um quadrado
    ssd1306_send_data(&ssd); // Atualiza o display
}

static void gpio_irq_handler(uint gpio, uint32_t events) {
     // Obtém o tempo atual em milissegundos
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    // Verificação de tempo para debounce
    if (current_time - last_time_button > 200){
        if(gpio == BUTTON_PIN_A){
            parar_buzzer(BUZZER_A); // Desliga o buzzer
            last_time_alarm = current_time; // Atualiza o tempo do último evento
            alarme_ativo = false; // Desativa o alarme
        }

        last_time_button = current_time; // Atualiza o tempo do último evento
    }
}

int main(){

    // Configuração do I2C
    configura_i2c(); 

    // Configuração do PIO
    pio = pio0; 
    uint offset = pio_add_program(pio, &animacao_matriz_program);
    sm = pio_claim_unused_sm(pio, true);
    animacao_matriz_program_init(pio, sm, offset, MATRIZ_PIN);

    // Configura os LEDs e botões
    configuraGPIO();

    // Configuração do ADC
    adc_init();
    adc_gpio_init(JOY_X);
    adc_gpio_init(JOY_Y);

    // Configuração da interrupção
    gpio_set_irq_enabled_with_callback(BUTTON_PIN_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);


    stdio_init_all();

    while (true){
        // Leitura dos valores do joystick
        adc_select_input(1);  
        uint16_t vrx_value = adc_read(); 

        adc_select_input(0);  
        uint16_t vry_value = adc_read(); 
        
        // Simula leitura de temperatura e gás
        uint16_t gas = ((vrx_value - 16) / max_value_joy) * 2000; // Converte o valor do eixo x para a faixa de 0 a 2000
        int temp = ((vry_value - 16) / max_value_joy) * 95 - 10;  // Converte o valor do eixo y para a faixa de -10 a 85
        
        
        // Verifica a faixa de temperatura e gás
        if (temp < 40 && gas < 1000) {
            gpio_put(LED_PIN_GREEN, 1); // Liga o LED verde
            gpio_put(LED_PIN_RED, 0); // Desliga o LED vermelho
            if(!alarme_ativo) display_desenho(0); // Mostra o estado seguro na matriz
        } else if (temp > 55 || gas > 1500){
            gpio_put(LED_PIN_GREEN, 0); // Desliga o LED verde
            gpio_put(LED_PIN_RED, 1); // Liga o LED vermelho
            
            iniciar_buzzer(BUZZER_A); // Liga o buzzer
            alarme_ativo = true; // Ativa o alarme
            display_desenho(2); // Mostra o estado de perigo na matriz
        } else {
            gpio_put(LED_PIN_GREEN, 1); // Liga o LED verde
            gpio_put(LED_PIN_RED, 1); // Liga o LED vermelho
            if(!alarme_ativo) display_desenho(1); // Mostra o estado de alerta na matriz
        }
        

        // Comunicação serial feita a cada 1.5 segundos
        uint32_t current_time = to_ms_since_boot(get_absolute_time());
        if (current_time - last_time_message > 1500) {
            last_time_message = current_time; // Atualiza o tempo do último evento
            printf("\nTemperatura: %d\n", temp);
            printf("Gás: %d\n", gas);
            printf("Alarme: %s\n", alarme_ativo ? "Ativo!" : "Desativado");
        }


        display_quadrado(vrx_value, vry_value); // Atualiza o display com a posição do quadrado

        sleep_ms(100); 
    }

    return 0;
}
