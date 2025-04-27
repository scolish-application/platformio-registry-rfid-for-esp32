| Pino MFRC522    | Pino ESP32          | Descrição                             |
|-----------------|---------------------|---------------------------------------|
| **SDA** (SS)    | **GPIO5** (D5)      | Pino de seleção do escravo (Slave Select) |
| **SCK**         | **GPIO18**          | Pino de relógio SPI (Clock)          |
| **MISO**        | **GPIO19**          | Pino de entrada SPI (Master In Slave Out) |
| **MOSI**        | **GPIO23**          | Pino de saída SPI (Master Out Slave In) |
| **IRQ**         | **Não conectado**   | Não é necessário para este projeto   |
| **GND**         | **GND**             | Pino de aterramento (Ground)         |
| **RST**         | **GPIO22** (D22)    | Pino de Reset do MFRC522             |
| **3.3V**        | **3V3**              | Pino de alimentação (3.3V)           |


| Pino Buzzer     | Pino ESP32          | Descrição                             |
|-----------------|---------------------|---------------------------------------|
| **Buzzer+**     | **GPIO12** (D12)    | Pino de controlo do buzzer           |
| **Buzzer-**     | **GND**             | Pino de aterramento (Ground)         |
