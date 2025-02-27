# Sistema de Votação com Raspberry Pi Pico

## Descrição
Este projeto implementa um sistema de votação utilizando um Raspberry Pi Pico, botões para votação, LEDs indicativos, um buzzer para feedback sonoro e um display OLED para exibir os votos registrados. A votação conta com dois botões principais para registrar votos "SIM" e "NÃO", além de um botão de reset para zerar os votos. Os resultados são exibidos em um display SSD1306 via comunicação I2C.

## Componentes Utilizados
- **Raspberry Pi Pico**
- **LEDs**: Verde e vermelho para indicar votos
- **Botões**: Três botões (SIM, NÃO e RESET)
- **Buzzer**: Para emitir sons de feedback
- **Display OLED SSD1306**: Para exibição do número de votos
- **Resistores de pull-up** (internos do RP2040 ativados para os botões)

## Funcionamento do Código
O código principal segue um fluxo baseado em interrupções de hardware para os botões, garantindo resposta rápida ao pressionamento. Além disso, conta com um mecanismo de debounce para evitar múltiplos acionamentos indesejados.

### Principais Funcionalidades:
1. **Registro de votos**:
   - Quando o botão "SIM" é pressionado, um voto é adicionado à contagem e o LED verde acende por 1 segundo.
   - Quando o botão "NÃO" é pressionado, um voto é adicionado à contagem e o LED vermelho acende por 1 segundo.
   - O display OLED é atualizado automaticamente com os votos registrados.
   - Um buzzer emite um som curto indicando o registro do voto.

2. **Reset da votação**:
   - O botão de reset zera a contagem de votos.
   - Um som mais longo é emitido para indicar a reinicialização da votação.
   - O display OLED é atualizado para exibir os votos zerados.

3. **Gerenciamento do tempo**:
   - O LED correspondente ao voto se apaga automaticamente após 1 segundo.
   - O buzzer desliga automaticamente após o tempo programado.

---

## Explicação do Código

### Definição de Pinos e Variáveis Globais
Os pinos dos LEDs, botões, buzzer e I2C são definidos, assim como variáveis globais para armazenar os votos e controlar estados de hardware.

```c
#define LED_VERDE 11
#define LED_VERMELHO 13
#define BOTAO_SIM 5
#define BOTAO_NAO 6
#define BOTAO_RESET 22
#define BUZZER 10

#define PORTA_I2C i2c1
#define SDA_I2C 14
#define SCL_I2C 15
#define ENDERECO_I2C 0x3C
```

As variáveis globais são usadas para armazenar a contagem de votos, estados dos LEDs e buzzer, além de timestamps para debounce:
```c
int votos_sim = 0;
int votos_nao = 0;
volatile bool led_green_on = false;
volatile bool led_red_on = false;
volatile uint32_t buzzer_time = 0;
```

### Função `configurar()`
Inicializa os periféricos:
- Configura LEDs como saída.
- Configura botões com resistores de pull-up e interrupções.
- Configura o buzzer usando PWM.
- Inicializa a comunicação I2C e o display OLED.

```c
gpio_init(LED_VERDE);
gpio_set_dir(LED_VERDE, GPIO_OUT);
gpio_put(LED_VERDE, 0);
```

### Função `botao_callback()`
Lida com interrupções dos botões e registra votos:
- Implementa debounce.
- Atualiza a contagem de votos.
- Aciona LEDs e o buzzer.
- Atualiza o display OLED.

```c
if (gpio == BOTAO_SIM && (tempo_atual - ultimo_tempo_botao_sim > DEBOUNCE_TIME)) {
    votos_sim++;
    led_green_on = true;
    led_green_time = to_ms_since_boot(get_absolute_time()) + 1000;
    gpio_put(LED_VERDE, 1);
    tocar_buzzer(2000, 100);
    atualizar_display();
}
```

### Função `atualizar_display()`
Atualiza a exibição dos votos no display OLED.
```c
snprintf(buffer, sizeof(buffer), "Votos SIM: %d", votos_sim);
ssd1306_draw_string(&ssd, buffer, 8, 10);
```

### Função `tocar_buzzer()`
Ativa o buzzer com PWM para emitir um som curto ao registrar um voto e um som longo ao resetar a votação.
```c
pwm_set_gpio_level(BUZZER, pwm_hw->slice[slice_num].top / 2);
pwm_set_enabled(slice_num, true);
```

### Loop Principal
Verifica periodicamente o estado dos LEDs e do buzzer, apagando-os conforme o tempo programado.
```c
if (led_green_on && now >= led_green_time) {
    gpio_put(LED_VERDE, 0);
    led_green_on = false;
}
```

---

## Conclusão
Este código implementa um sistema de votação interativo e responsivo, utilizando interrupções para os botões e controle eficiente dos LEDs, buzzer e display OLED. O uso de debounce previne leituras erradas dos botões, e a lógica do loop principal garante que os dispositivos sejam desligados automaticamente quando necessário.

