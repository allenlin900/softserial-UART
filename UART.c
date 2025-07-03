#include <msp430.h>

#define UART_TXD BIT1
#define UART_RXD BIT2
#define UART_TBIT_DIV_2 (416)
#define UART_TBIT       (833)
#define RX_BUFFER_SIZE  64

unsigned int txData;
volatile char rxBuffer[RX_BUFFER_SIZE];
volatile unsigned int rxIndex = 0;  //是否超過接收的字數 
volatile unsigned char messageReady = 0;  //是否整串訊息傳完 

void TimerA_UART_init(void);
void TimerA_UART_tx(unsigned char byte);
void TimerA_UART_print(char *string);

void main(void)
{
    WDTCTL = WDTPW + WDTHOLD;
    DCOCTL = 0x00;
    BCSCTL1 = CALBC1_8MHZ;
    DCOCTL = CALDCO_8MHZ;
    P1OUT = 0x00;
    P1SEL |= UART_RXD;
    P1SEL &= ~UART_TXD;
    P1DIR |= UART_TXD;
    P1DIR &= ~UART_RXD;
    P1DIR |= BIT0;  // LED 輸出

    __enable_interrupt();
    
    TimerA_UART_init();
    TimerA_UART_print("READY.\r\n");

    for (;;) {
        if (messageReady) {
            TimerA_UART_print((char *)rxBuffer);
            TimerA_UART_tx('\r');
            TimerA_UART_tx('\n');
            rxIndex = 0;
            messageReady = 0;
        }
    }
}

void TimerA_UART_print(char *string) {
    while (*string) {
        TimerA_UART_tx(*string++);
        __delay_cycles(1000); // 避免傳太快出錯
    }
}

void TimerA_UART_init(void) {
    TACCTL1 = SCS + CM1 + CAP + CCIE;
    TACTL = TASSEL_2 + MC_2;
    P1OUT |= UART_TXD;  // TXD idle high
}

void TimerA_UART_tx(unsigned char byte) {
    while (TACCTL0 & CCIE);  // 等待前一字元傳完
    txData = byte;
    txData |= 0x100;
    txData <<= 1;
    TACCR0 = TAR + UART_TBIT;
    TACCTL0 |= CCIE;
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0_ISR(void) {
    static unsigned char txBitCnt = 10;

    if (txBitCnt == 0) {
        TACCTL0 &= ~CCIE;
        txBitCnt = 10;
        P1OUT |= UART_TXD; // idle high
    } else {
        if (txData & 0x01)
            P1OUT |= UART_TXD;
        else
            P1OUT &= ~UART_TXD;

        txData >>= 1;
        TACCR0 += UART_TBIT;
        txBitCnt--;
    }
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void Timer_A1_ISR(void) {
    static unsigned char rxBitCnt = 8;
    static unsigned char rxData = 0;

    switch (__even_in_range(TA0IV, TA0IV_TAIFG)) {
        case TA0IV_TACCR1:
            TACCR1 += UART_TBIT;
            if (TACCTL1 & CAP) {
                TACCTL1 &= ~CAP;
                TACCR1 += UART_TBIT_DIV_2;
            } else {
                rxData >>= 1;
                if (P1IN & UART_RXD)
                    rxData |= 0x80;
                rxBitCnt--;
                if (rxBitCnt == 0) {
                    if (!messageReady && rxIndex < RX_BUFFER_SIZE - 1) {
                        rxBuffer[rxIndex] = rxData;
                        if (rxData == '\r' || rxData == '\n') {
                            rxBuffer[rxIndex] = '\0';
                            messageReady = 1;

                            P1OUT |= BIT0;              // 亮紅燈
                            __delay_cycles(50000);      // 約 50ms
                            P1OUT &= ~BIT0;             // 熄燈
                        } else {
                            rxIndex++;
                        }
                    }
                    rxBitCnt = 8;
                    TACCTL1 |= CAP;
                }
            }
            break;
    }
}

