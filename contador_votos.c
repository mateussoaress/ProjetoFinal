#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "inc/ssd1306.h"
#include "inc/font.h"

// Definição dos pinos
#define LED_VERDE 11
#define LED_VERMELHO 13
#define BOTAO_SIM 5
#define BOTAO_NAO 6
#define BOTAO_RESET 22
#define BUZZER 10  // Pino do buzzer

#define PORTA_I2C i2c1
#define SDA_I2C 14
#define SCL_I2C 15
#define ENDERECO_I2C 0x3C

// Variáveis globais
int votos_sim = 0;
int votos_nao = 0;
volatile bool led_green_on = false;
volatile bool led_red_on = false;
volatile uint32_t led_green_time = 0;
volatile uint32_t led_red_time = 0;
volatile uint32_t buzzer_time = 0;
volatile bool buzzer_on = false;
volatile uint32_t ultimo_tempo_botao_sim = 0;
volatile uint32_t ultimo_tempo_botao_nao = 0;
volatile uint32_t ultimo_tempo_botao_reset = 0;
#define DEBOUNCE_TIME 200

ssd1306_t ssd;

// Função para ativar o buzzer com PWM
void tocar_buzzer(int frequencia, int duracao) {
    uint slice_num = pwm_gpio_to_slice_num(BUZZER);
    
    // Define a frequência do PWM
    pwm_set_clkdiv(slice_num, 4.0f);  
    pwm_set_wrap(slice_num, 125000000 / (4 * frequencia));  
    pwm_set_gpio_level(BUZZER, pwm_hw->slice[slice_num].top / 2);  
    pwm_set_enabled(slice_num, true);  

    buzzer_on = true;
    buzzer_time = to_ms_since_boot(get_absolute_time()) + duracao;
}

// Função para atualizar o display com os votos
void atualizar_display() {
    char buffer[20];

    ssd1306_fill(&ssd, false);
    
    snprintf(buffer, sizeof(buffer), "Votos SIM: %d", votos_sim);
    ssd1306_draw_string(&ssd, buffer, 8, 10);

    snprintf(buffer, sizeof(buffer), "Votos NAO: %d", votos_nao);
    ssd1306_draw_string(&ssd, buffer, 8, 30);

    ssd1306_send_data(&ssd);
}

// Função de interrupção para os botões
void botao_callback(uint gpio, uint32_t events) {
    uint32_t tempo_atual = to_ms_since_boot(get_absolute_time());

    if (gpio == BOTAO_SIM && (tempo_atual - ultimo_tempo_botao_sim > DEBOUNCE_TIME)) {
        ultimo_tempo_botao_sim = tempo_atual;
        votos_sim++;
        printf("Voto registrado: SIM. Total: %d\n", votos_sim);
        led_green_on = true;
        led_green_time = to_ms_since_boot(get_absolute_time()) + 1000;
        gpio_put(LED_VERDE, 1);
        
        tocar_buzzer(2000, 100);  // Toca som curto de 100ms

        atualizar_display();
    } 
    else if (gpio == BOTAO_NAO && (tempo_atual - ultimo_tempo_botao_nao > DEBOUNCE_TIME)) {
        ultimo_tempo_botao_nao = tempo_atual;
        votos_nao++;
        printf("Voto registrado: NÃO. Total: %d\n", votos_nao);
        led_red_on = true;
        led_red_time = to_ms_since_boot(get_absolute_time()) + 1000;
        gpio_put(LED_VERMELHO, 1);

        tocar_buzzer(1500, 100);  // Toca som curto de 100ms

        atualizar_display();
    }
    else if (gpio == BOTAO_RESET && (tempo_atual - ultimo_tempo_botao_reset > DEBOUNCE_TIME)) {
        ultimo_tempo_botao_reset = tempo_atual;
        votos_sim = 0;
        votos_nao = 0;
        printf("Votação resetada!\n");

        tocar_buzzer(1000, 500);  // Toca som longo de 500ms

        atualizar_display();
    }
}

// Configuração do hardware
void configurar() {
    stdio_init_all();

    // Configuração dos LEDs
    gpio_init(LED_VERDE);
    gpio_set_dir(LED_VERDE, GPIO_OUT);
    gpio_put(LED_VERDE, 0);

    gpio_init(LED_VERMELHO);
    gpio_set_dir(LED_VERMELHO, GPIO_OUT);
    gpio_put(LED_VERMELHO, 0);

    // Configuração dos botões
    gpio_init(BOTAO_SIM);
    gpio_set_dir(BOTAO_SIM, GPIO_IN);
    gpio_pull_up(BOTAO_SIM);
    gpio_set_irq_enabled_with_callback(BOTAO_SIM, GPIO_IRQ_EDGE_FALL, true, &botao_callback);

    gpio_init(BOTAO_NAO);
    gpio_set_dir(BOTAO_NAO, GPIO_IN);
    gpio_pull_up(BOTAO_NAO);
    gpio_set_irq_enabled_with_callback(BOTAO_NAO, GPIO_IRQ_EDGE_FALL, true, &botao_callback);

    gpio_init(BOTAO_RESET);
    gpio_set_dir(BOTAO_RESET, GPIO_IN);
    gpio_pull_up(BOTAO_RESET);
    gpio_set_irq_enabled_with_callback(BOTAO_RESET, GPIO_IRQ_EDGE_FALL, true, &botao_callback);

    // Configuração do buzzer com PWM
    gpio_set_function(BUZZER, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(BUZZER);
    pwm_config config = pwm_get_default_config();
    pwm_init(slice_num, &config, false);

    // Inicialização do I2C e Display OLED
    i2c_init(PORTA_I2C, 400000);
    gpio_set_function(SDA_I2C, GPIO_FUNC_I2C);
    gpio_set_function(SCL_I2C, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_I2C);
    gpio_pull_up(SCL_I2C);

    ssd1306_init(&ssd, 128, 64, false, ENDERECO_I2C, PORTA_I2C);
    ssd1306_config(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    atualizar_display();
}

int main() {
    configurar();
    printf("Sistema de Votação Iniciado!\nPressione os botões para votar.\n");

    while (1) {
        uint32_t now = to_ms_since_boot(get_absolute_time());

        // Verifica se o LED verde deve ser apagado
        if (led_green_on && now >= led_green_time) {
            gpio_put(LED_VERDE, 0);
            led_green_on = false;
        }
        // Verifica se o LED vermelho deve ser apagado
        if (led_red_on && now >= led_red_time) {
            gpio_put(LED_VERMELHO, 0);
            led_red_on = false;
        }
        // Verifica se o buzzer deve ser desligado
        if (buzzer_on && now >= buzzer_time) {
            pwm_set_enabled(pwm_gpio_to_slice_num(BUZZER), false);
            buzzer_on = false;
        }

        sleep_ms(10); // Pequeno atraso para evitar processamento excessivo
    }
}
