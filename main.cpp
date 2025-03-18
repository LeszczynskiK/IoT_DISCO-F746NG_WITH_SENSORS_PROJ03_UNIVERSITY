#include "EthernetInterface.h"
#include "mbed.h"
#include <cstring>
#include <chrono>

DigitalOut led1(LED1);
DigitalOut LEDB(A3);       // blue LED
DigitalOut LEDR(D12);      // red LED
DigitalOut LEDG(D11);      // green LED
I2C i2c(I2C_SDA, I2C_SCL); // main i2c object
AnalogIn adc_temp(ADC_TEMP);

EthernetInterface net;
TCPSocket server;
TCPSocket *clientSocket;
SocketAddress clientAddress;
char rxBuf[512] = {0};
char txBuf[512] = {0};

Thread serverThread;

const int addr8bit = 0x90;    // Adres I2C dla STLM75 (8-bitowy)
const int LPS331_ADR = 0xBA;  // Adres I2C dla LPS331 (8-bitowy)

// Funkcje STLM75
void STLM75_Config(uint8_t conf) {
    char buf[2] = {0};
    buf[0] = 0x01; // config register
    buf[1] = conf;
    i2c.write(addr8bit, buf, 2);
}

int STLM75_ReadTemp() {
    uint8_t rej_temp = 0;
    int16_t temperatura = 0;
    char buf[2] = {0};

    buf[0] = rej_temp;
    i2c.write(addr8bit, buf, 1);
    i2c.read(addr8bit, buf, 2);
    temperatura = ((int16_t)buf[0] << 8) | buf[1];
    temperatura = temperatura >> 8;
    return temperatura;
}

// Funkcje LPS331
void LPS331_WriteReg(char adr, char wart) {
    char buf[2] = {0};
    buf[0] = adr;
    buf[1] = wart;
    i2c.write(LPS331_ADR, buf, 2);
}

void LPS331_ContInit() {
    LPS331_WriteReg(0x20, 0x00); // CTRL1_REG power down
    LPS331_WriteReg(0x10, 0x79); // RES_CONF
    LPS331_WriteReg(0x20, 0x94); // CTRL_REG1 active mode
}

int LPS331_PressRead() {
    int cisnienie = 0;
    char buf[3] = {0};
    char adr_rej = 0;

    buf[0] = 0x20;
    buf[1] = 0x94;
    i2c.write(LPS331_ADR, buf, 2);

    adr_rej = 0x28; // PRESS_OUT_XL
    i2c.write(LPS331_ADR, &adr_rej, 1);
    i2c.read(LPS331_ADR, &buf[0], 1);
    
    adr_rej = 0x29; // PRESS_OUT_L
    i2c.write(LPS331_ADR, &adr_rej, 1);
    i2c.read(LPS331_ADR, &buf[1], 1);
    
    adr_rej = 0x2a; // PRESS_OUT_H
    i2c.write(LPS331_ADR, &adr_rej, 1);
    i2c.read(LPS331_ADR, &buf[2], 1);
    
    cisnienie = (buf[2] << 16) | (buf[1] << 8) | buf[0];
    cisnienie = cisnienie / 4096;
    return cisnienie;
}

void webServer() {
    net.connect();
    
    SocketAddress ip;
    SocketAddress netmask;
    SocketAddress gateway;

    net.get_ip_address(&ip);
    net.get_netmask(&netmask);
    net.get_gateway(&gateway);

    ip.set_port(80);

    printf("IP address: %s\r\n", ip.get_ip_address());
    printf("Netmask: %s\r\n", netmask.get_ip_address());
    printf("Gateway: %s\r\n\r\n", gateway.get_ip_address());

    server.open(&net);
    server.bind(ip);
    server.listen(5);

    while (true) {
        printf("***********************\r\n");

        nsapi_error_t error = 0;
        clientSocket = server.accept(&error);
        
        if (error != 0) {
            printf("Connection failed!\r\n");
        } else {
            clientSocket->set_timeout(200);
            clientSocket->getpeername(&clientAddress);
            printf("Klient o adresie IP %s polaczony.\r\n\r\n",
                   clientAddress.get_ip_address());
            error = clientSocket->recv(rxBuf, sizeof(rxBuf));

            printf("Odebrano: %d\n\r%.*s\n\r", strlen(rxBuf), strlen(rxBuf), rxBuf);
            if (rxBuf[0] == 'G' && rxBuf[1] == 'E' && rxBuf[2] == 'T') {
                // Pobierz aktualne wartości z sensorów
                int temp = STLM75_ReadTemp();
                int press = LPS331_PressRead();
                
                // Dane rzeczywiste
                sprintf(rxBuf,
                        "Temperatura powietrza: %d\r\n"
                        "Cisnienie otoczenia: %d\r\n",
                        temp, press);
                
                sprintf(txBuf,
                        "HTTP/1.1 200 OK\nContent-Length: %d\r\nContent-Type: "
                        "text\r\nConnection: Close\r\n\r\n",
                        strlen(rxBuf));
                
                clientSocket->send(txBuf, strlen(txBuf));
                printf("\r\n%s\r\n", txBuf);
                printf("Wyslano:\r\n%d\r\n", strlen(txBuf));
                clientSocket->send(rxBuf, strlen(rxBuf));
                printf("Wyslano:\r\n%d\r\n", strlen(rxBuf));
                printf("\r\n%s\r\n", rxBuf);
                printf("***********************\r\n");
            }
        }
        clientSocket->close();
        printf("Client socket closed\r\n");
    }
}

int main() {
    LEDB = 1;
    LEDR = 1;
    LEDG = 1;

    STLM75_Config(0x00); // Inicjalizacja STLM75
    LPS331_ContInit();   // Inicjalizacja LPS331

    printf("Connecting...\r\n");
    serverThread.start(callback(webServer));

    while (true) {
        led1 = !led1;
        ThisThread::sleep_for(3000);
    }
}