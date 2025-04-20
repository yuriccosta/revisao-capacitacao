# Simulador de Alarme Industrial

## Objetivo Geral

O objetivo do projeto é simular um sistema que monitora as condições de trabalho de uma indústria e facilita a visualização dos níveis de temperatura e gás no ambiente. O sistema indica quando os níveis estão em uma faixa segura, de atenção ou de perigo imediato. Caso uma condição de perigo imediato seja detectada, um alarme é ativado, visando proteger os trabalhadores.

## Descrição Funcional

O sistema captura continuamente os valores de temperatura e gás do ambiente. Os valores considerados seguros, de alerta e de perigo são definidos apenas para fins de simulação.

- **Faixa Segura**:  
  - Temperatura < 40°C  
  - Gás < 1000  
  - LED RGB na cor **verde**  
  - Matriz de LEDs exibe o símbolo **"+" verde**

- **Faixa de Alerta**:  
  - Quando **não se enquadra** nas faixas de segurança ou de perigo  
  - LED RGB na cor **amarela**  
  - Matriz de LEDs exibe um **triângulo amarelo**

- **Faixa de Perigo Imediato**:  
  - Temperatura > 55°C **ou** Gás > 1500  
  - LED RGB na cor **vermelha**  
  - Matriz de LEDs exibe o símbolo **"X vermelho"**  
  - **Buzzer é ativado**

Uma vez que o alarme sonoro é ativado, ele **permanece ativo** mesmo que os valores voltem a uma faixa segura. Para desativá-lo, é necessário pressionar o **botão A**. Enquanto o alarme não for desligado, a matriz de LEDs continuará exibindo o estado de perigo.

A cada 1,5 segundos, as informações de temperatura e gás, bem como o estado do alarme (ativado ou desativado), são enviadas via **comunicação UART USB**.

O **display OLED** exibe um quadrado 8x8 que se movimenta conforme os valores lidos do **joystick**.

## Uso dos Periféricos da BitDogLab

- **Potenciômetro do Joystick**:  
  Utilizado para simular os dados de temperatura e gás:  
  - **Eixo Y** representa a **temperatura**  
  - **Eixo X** representa o **gás**  
  Os valores são normalizados para faixas esperadas de sensores reais.

- **Botões**:  
  - **Botão A**: usado para desativar o alarme sonoro, quando pressionado.

- **Display OLED**:  
  - Exibe um quadrado 8x8 que varia sua posição com base nos dados do joystick.

- **Matriz de LEDs**:  
  - Exibe diferentes símbolos conforme o nível de risco:  
    - "+" verde para seguro  
    - Triângulo amarelo para alerta  
    - "X" vermelho para perigo  
  - Permanece indicando perigo até o alarme ser desligado manualmente.

- **LED RGB**:  
  - Indica visualmente a situação atual em tempo real:  
    - Verde (seguro), amarelo (alerta), vermelho (perigo)  
  - Continua atualizando mesmo com o alarme ativado.

- **Buzzer**:  
  - Emite alarme sonoro quando detectada uma condição de perigo.

- **Interrupções**:  
  - Utilizadas para detectar o pressionamento do botão e desligar o alarme sem a necessidade de verificação constante no código principal.

- **Tratamento de debounce dos botões**:  
  - Implementado para evitar leituras incorretas.  
  - Verifica se o tempo desde o último acionamento é superior a 200ms.

---

**Observação**: Os valores de temperatura e gás utilizados são fictícios e foram definidos apenas para fins de simulação.
