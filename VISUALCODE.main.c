#include <windows.h>
#include <stdio.h>
#include <string.h>

void configureSerialPort(HANDLE hSerial) {
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        printf("Error getting state\n");
        exit(1);
    }

    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        printf("Error setting state\n");
        exit(1);
    }

    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(hSerial, &timeouts)) {
        printf("Error setting timeouts\n");
        exit(1);
    }
}

void sendMessage(HANDLE hSerial, const char* message) {
    DWORD bytes_written;
    if (!WriteFile(hSerial, message, strlen(message), &bytes_written, NULL)) {
        printf("Error writing to serial port\n");
        exit(1);
    }
}

void readMessage(HANDLE hSerial, char* buffer, int buffer_size) {
    DWORD bytes_read;
    if (!ReadFile(hSerial, buffer, buffer_size, &bytes_read, NULL)) {
        printf("Error reading from serial port\n");
        exit(1);
    }
    buffer[bytes_read] = '\0'; // Null-terminate the string
}

void displayInitialMenu() {
    printf(" ___ MIZAEL, MYLENNA, SEBASTIÃO e HUGO ___\n");
    printf(" ___    PROGRAMACAOO APLICADA 2024/ 1  ___\n");
    printf("Escolha uma opcao:\n");
    printf("1 - Enviar\n");
    printf("2 - Receber\n");
}

void handlePCSelection(HANDLE hSerial) {
    printf("ESCREVA UM TEXTO PARA O ARDUINO:\n");
    char msg[256];
    fgets(msg, sizeof(msg), stdin);
    sendMessage(hSerial, msg);

    // Wait for confirmation from Arduino
    char read_buf[256];
    while (1) {
        readMessage(hSerial, read_buf, sizeof(read_buf) - 1);
        if (strstr(read_buf, "TEXT RECEIVED") != NULL) {
            break;
        }
    }
    printf("Mensagem enviada e recebida pelo Arduino.\n");
    Sleep(3000); // Espera 3 segundos
}

void handleInoSelection(HANDLE hSerial) {
    printf("Aguardando texto do Arduino...\n");
    char read_buf[256];
    while (1) {
        readMessage(hSerial, read_buf, sizeof(read_buf) - 1);
        if (strlen(read_buf) > 0) {
            printf("Arduino: %s\n", read_buf);
            if (strstr(read_buf, "MENU SELECTED") != NULL) {
                break;
            }
        }
    }
    Sleep(3000); // Espera 3 segundos
}

void handleMenuSelection(HANDLE hSerial, int choice) {
    if (choice == 1) {
        sendMessage(hSerial, "PC SELECIONADO\n");
        handlePCSelection(hSerial);
    } else if (choice == 2) {
        sendMessage(hSerial, "INO SELECIONADO\n");
        handleInoSelection(hSerial);
    } else {
        printf("Opção inválida, tente novamente.\n");
    }
}

void checkArduinoMessages(HANDLE hSerial) {
    char read_buf[256];
    readMessage(hSerial, read_buf, sizeof(read_buf) - 1);
    if (strlen(read_buf) > 0) {
        printf("Arduino: %s\n", read_buf);

        if (strstr(read_buf, "PC SELECIONADO")) {
            handleMenuSelection(hSerial, 1);
        } else if (strstr(read_buf, "INO SELECIONADO")) {
            handleMenuSelection(hSerial, 2);
        }
    }
}

int main() {
    HANDLE hSerial = CreateFile("COM1", GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (hSerial == INVALID_HANDLE_VALUE) {
        printf("Error in opening serial port\n");
        return 1;
    }

    configureSerialPort(hSerial);

    int choice;
    while (1) {
        displayInitialMenu();
        scanf("%d", &choice);
        getchar(); // consume the newline character
        handleMenuSelection(hSerial, choice);

        // Continuously check for messages from Arduino
        checkArduinoMessages(hSerial);
    }

    CloseHandle(hSerial);
    return 0;
}