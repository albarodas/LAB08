/* 
 * File:   LAB_08.c
 * Author: ALBA RODAS
 *
 * Created on 23 de abril de 2022, 07:27 PM
 */

// PIC16F887 Configuration Bit Settings
// 'C' source line config statements
// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

// CONFIGURATION WORDS 1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIGURATION WORDS 2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.
/*------------------------------------------------------------------------------
 * LIBRERIAS 
 ------------------------------------------------------------------------------*/
// INCLUIMOS LIBRERIAS A UTILIZAR, SEGUN FUNCIONES
#include <xc.h>
#include <stdint.h>

#define _XTAL_FREQ 4000000

/*------------------------------------------------------------------------------
 * VARIABLES 
 ------------------------------------------------------------------------------*/
// NO HAY PARA EL PRELAB
uint8_t value_numero;                    // COUNTER
uint8_t estado_actual;                      // ESTADO DE TRANSISTORES
uint16_t ayuda;                         
// PARA POSTLAB SI HAY Y SON ESTAS:

uint8_t cen;                    // UNIDAD, DECENA, CENTENA
uint8_t dec;
uint8_t uni;
// ASIGNAR VALOR A DISPLAYS:
uint8_t centena_display;                  // VERIABES QUE TIENE EN VALOR TRADUCIDO POR VALUES
uint8_t decena_display;                  
uint8_t unidad_display;

/*------------------------------------------------------------------------------
 * PROTOTIPO DE FUNCIONES 
 ------------------------------------------------------------------------------*/
void config_io(void);
void config_clk(void);
void config_tmr0(void);
void config_ADC(void);
void config_interrupciones(void);
//DIVISION:
void division(uint16_t numero, uint8_t *centena, uint8_t *decena, uint8_t *unidad);
uint8_t values(uint8_t valor_traduccion);

/*------------------------------------------------------------------------------
 * INTERRUPCIONES 
 ------------------------------------------------------------------------------*/
void __interrupt() isr (void)
{
    if(PIR1bits.ADIF)               // BANDERA ON, ENTRO
    {
        if(ADCON0bits.CHS == 12)    
            value_numero = ADRESH;
        else                        
            PORTA = ADRESH;         
        // *CASO VISTO EN CLASE: Estamos utilizando más de un canal analógico
        /*else if (ADCON0bits.CHS == 1)
         * {
            PORTD = ADRESH;         // DEFINIMIOS NUEVO VALOR AL ADRESH
        }*/
        PIR1bits.ADIF = 0;          // CLR BANDERA
    }
    else if(INTCONbits.T0IF)        // BANDERA ON, ENTRO
    {
        INTCONbits.T0IF = 0;            // CLR BANDERA
        PORTD = 0;                          // CLR A ESTAODS 
        switch(estado_actual)
        {
        case 0:
            estado_actual = 1;
            PORTD = 0b001;                  // TRANSISTOR 1 = ON
            PORTC = centena_display;        // DISPLAY = CENTENAS
            TMR0 = 131;                     // TIMER0 = 2ms
            break;
        case 1:
            estado_actual = 2;
            PORTD = 0b010;                  // TRANSISTOR 2 = ON
            PORTC = decena_display;         // DISPLAY = DECENAS
            TMR0 = 131;                     // TIMER0 = 2ms
            break;
        case 2:
            estado_actual = 0;
            PORTD = 0b100;                  // TRANSISTOR 3 = ON
            PORTC = unidad_display;         // DISPLAY = UNIDADES
            TMR0 = 131;                     // TIMER0 = 2ms
            break;
        default:
            estado_actual = 0;
            TMR0 = 131;                      // TIMER0 = 2ms = CARGA ES 131
            break;
        }
    }
}

/*------------------------------------------------------------------------------
 * CICLO PRINCIPAL
 ------------------------------------------------------------------------------*/
void main(void) 
{
    config_io();
    config_clk();
    config_tmr0();
    config_ADC();
    config_interrupciones();
    
    ADCON0bits.GO = 1;          // ACTIVO CONVERSTION
    
    while(1)
    {
        if(ADCON0bits.GO == 0)        
        {
            // *CASO: Estamos utilizando más de un canal analógico
            /* CASO VISTO EN CLASE:
            if(ADCON0bits.CHS == 0b0000)    
                ADCON0bits.CHS = 0b0001;    // Change de canal
            else if(ADCON0bits.CHS == 0b0001)
                ADCON0bits.CHS = 0b0000;    // Change de canal
            __delay_us(40);                 // Debo tener cuidado con el tiempo de estabilizacion de cambio de canal, que sea
             * suficiente para que la circuiteria interna se estabilice.
             * TENEMOS 14 CANALES ANALOGICOS POR UTILIZAR.
            */
            if(ADCON0bits.CHS == 13)    // CANAL = 13 --> CAMBIO A 12
                ADCON0bits.CHS = 12;
            else                        // ULTIMO CANAL NO IGUAL A 12 --> CAMBIO A 12
                ADCON0bits.CHS = 13;
            
            __delay_us(50);             // DELAY OBLIGATORIO > 40
            ADCON0bits.GO = 1;          // INICIO LA CONVERSION (PROCESO).
        }
        
        ayuda = value_numero*100/51;            
        
        division(ayuda, &cen, &dec, &uni); // DIVISION EN PORTC
        
        unidad_display = values(uni);    // VARIABLE = VALOR TRADUCIDO POR TABLA
        decena_display = values(dec);    
        centena_display = values(cen);    
    }
}

/*------------------------------------------------------------------------------
 * CONFIGURACION 
 ------------------------------------------------------------------------------*/
void config_io(void)            // INS AND OUTS
{
    ANSEL   =   0;              // INICIALIZAMOS PUERTOS DIGITALES
    ANSELH  =   0b00110000;     // AN13 y AN12 --> PUERTOS ANALOGICOS, TENEMOS 14 POR USAR DE OPCION
     //DEFINO ENTRADAS Y SALIDAS
    TRISA   =   0;              // PORTA --> SALIDA
    TRISB   =   0b00100001;     // PORTB; 0, 5 --> ENTRADA
    TRISC   =   0;              // PORTC --> SALIDA
    TRISD   =   0;              // PORTD --> SALIDA
    
    // CLRF A PUERTOS PARA EVITAR ERRORES
    PORTA   =   0;              // CLR PORTA
    PORTB   =   0;              // CLR PORTB
    PORTC   =   0;              // CLR PORTC
    PORTD   =   0;              // CLR PORTD
    return;
}

void config_clk(void)        
{
    // CONFIGURACION DEL RELOJ INTERNO:
    OSCCONbits.IRCF2 = 1;       // OSCILADOR A '4MHz'
    OSCCONbits.IRCF1 = 1;        
    OSCCONbits.IRCF0 = 0;        
    OSCCONbits.SCS = 1;         // HABILITAMOS EL RELOJ INTERNO
    return;
}

//CONFIGURACION TMR0
void config_tmr0(void)
{
    OPTION_REGbits.T0CS = 0;    // RELOJ 4MHz
    OPTION_REGbits.T0SE = 0;    // SELECCIONO FLACOS POSITIVOS
    OPTION_REGbits.PSA = 0;     // PRESCALER -> TMR0
    OPTION_REGbits.PS2 = 0;     // PRESCALER DE: (011) --> 1:8
    OPTION_REGbits.PS1 = 1;     
    OPTION_REGbits.PS0 = 1;     
    TMR0 = 131;                 // CARGA TMR0 --> 2ms
    return;
}

void config_ADC(void)           // Configuración del ADC
{
    // CONFIGURAMOS REFERENCIAS INTERNAS:
    ADCON1bits.ADFM = 0;        // Justificación a la izquierda
    ADCON1bits.VCFG0 = 0;       // REFERENCIA --> VDD
    ADCON1bits.VCFG1 = 0;       // REFERENCIA --> VSS
    
    // CONFIGURAMOS QUE CANAL ANALOGICO SERA NUESTRA ENTRADA:
    ADCON0bits.ADCS = 0b01;     // UTILIZAMOS FOSC/8
    ADCON0bits.CHS = 12;        // SELECCIONO AL PIN QUE NECESITAMOS 'B0' y 'B5'
    ADCON0bits.ADON = 1;        // ADC ON
    __delay_us(50);             // DELAY 50uS
    return;
}

void config_interrupciones(void)           // CONFIGURO LAS INTERRUPCIONES
{
    INTCONbits.GIE  = 1;        // ACTIVO LAS INTERRUPCIONES GLOBALES, PARA PODER UTILIZAR INTERRUPCIONES.
    INTCONbits.PEIE = 1;        // Activar interrupciones periféricas
    PIE1bits.ADIE = 1;          // Activar interrupción de ADC
    PIR1bits.ADIF = 0;          // Limpiar bandera de ADC
    INTCONbits.T0IE = 1;        // ACTIVO INTERRUPCIONES PARA EL TMR0.
    INTCONbits.T0IF = 0;        // APAGO LA BANDERA DEL TMR0.
    return;
}

void division(uint16_t numero, uint8_t *centena, uint8_t *decena, uint8_t *unidad)
{
    uint8_t inicia_division;            // DEFINO VARIABLE PARA DIVIDIR.
    *centena = numero / 100;    // '100' --> CENTENAS, LO DIVIDO DENTRO DE ESA MAGNITUD.
    inicia_division = numero % 100;     // AISLO DECENAS DE LAS UNIDADES, UTILIZANDO UN 'MODULO'.
    *decena = inicia_division / 10;     // DIVIDO MI VARIABLE DEFINIDA DENTRO DE 10, PARA OBTENER DECENAS. Dividir el aux por 10 para obtener decenas
    *unidad = inicia_division % 10;     // VALOR DE VARIABLE DE DIVISION --> UNIDADES.
    return;
}
/*------------------------------------------------------------------------------
 * TABLA
 ------------------------------------------------------------------------------*/
// TRADUZCO VALORES DEL 0 AL 9, BINARIO --> DECIMAL
uint8_t values(uint8_t valor_traduccion)    // EN MI DIVISION, DEFINÍ PARA MI TABLA, UN 'value_traduccionñ'. DEPENDIENDO ESE VALOR LO BUSCO EN MI 'TABLA'.
{
    switch(valor_traduccion)
    {
        case 0:
            return 0b00111111;
            break;
        case 1:
            return 0b00000110;
            break;
        case 2:
            return 0b01011011;
            break;
        case 3:
            return 0b01001111;
            break;
        case 4:
            return 0b01100110;
            break;
        case 5:
            return 0b01101101;
            break;
        case 6:
            return 0b01111101;
            break;
        case 7:
            return 0b00000111;
            break;
        case 8:
            return 0b01111111;
            break;
        case 9:
            return 0b01101111;
            break;
        default:
            return 0b00111111;  // COMO DEFAULT, VOLVEMOS A CERO.
            break;
    }
}