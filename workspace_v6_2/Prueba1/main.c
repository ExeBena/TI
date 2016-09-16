/*Prueba1
 * es la primer prueba para hacer andar el
 * TP4 de digitales3
 *
 * En esta prueba intentaremos mandar datos
 * del sensor de temperatura on-chip a traves
 * del puerto UART
 *
 */
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"

#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/adc.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"

#include "utils/uartstdio.h"


unsigned long ulADC0Value[4];
volatile unsigned long ulTempAvg;
volatile unsigned long ulTempValueC;
volatile unsigned long ulTempValueF;
unsigned long ulPeriod;

unsigned long contador=0;

//void Timer0IntHandler(void);

//void UARTIntHandler(void)
//{
////	unsigned long ulStatus;
////	unsigned char received_character;
////	ulStatus = UARTIntStatus(UART0_BASE, true); //get interrupt status
////	UARTIntClear(UART0_BASE, ulStatus); //clear the asserted interrupts
//
//
//	UARTprintf("La temperatura actual es: %d /n",ulTempAvg); //
//
//
//	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2); //blink LED
//	SysCtlDelay(SysCtlClockGet() / (1000 * 3)); //delay ~1 msec
//	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0); //turn off LED
////	while(UARTCharsAvail(UART0_BASE)) //loop while there are chars
////	{
////		received_character = UARTCharGet(UART0_BASE);
////		UARTCharPutNonBlocking(UART0_BASE, received_character); //echo character
////		if (received_character == 13) UARTCharPutNonBlocking(UART0_BASE, 10);
////		//echo character
////		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2); //blink LED
////		SysCtlDelay(SysCtlClockGet() / (1000 * 3)); //delay ~1 msec
////		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0); //turn off LED
////	}
//}





int main(void)
{
//	Frecuencia del sistema
	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);

//	Habilitamos los perifericos que vamos a utilizar
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

//    Configuramos el timer
    TimerConfigure(TIMER0_BASE, TIMER_CFG_32_BIT_PER);

//    Configuramos el GPIO F
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);
	SysCtlADCSpeedSet(SYSCTL_ADCSPEED_250KSPS);

//	Desabilitamos el secuenciador del ADC
	ADCSequenceDisable(ADC0_BASE, 1);

//	Configuramos el "secuenciador" del ADC
	ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH1);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_CH1);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_CH1);
	ADCSequenceStepConfigure(ADC0_BASE, 1, 3, ADC_CTL_CH1 | ADC_CTL_IE | ADC_CTL_END);
	ADCSequenceEnable(ADC0_BASE, 1);

//	Configuramos que pines del GPIO A van a ser usados con la UART
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

//
//    Configuramos la UART con 115200 baudios y 8-N-1
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
//    Configuramos los niveles de FIFO de transmisor y receptor de UART
    UARTFIFOLevelSet(UART0_BASE, UART_FIFO_TX4_8, UART_FIFO_RX4_8); // FIFO 8 chars
    UARTFIFOEnable(UART0_BASE); //enable FIFOs

    IntMasterEnable();
    IntEnable(INT_UART0);
    UARTIntEnable(UART0_BASE, UART_INT_RX);

//    Establecemos qeu vamos a usar las UARTstdio para la UART0
    UARTStdioInit(0); //tells uartstdio functions to use UART0

    //	Ahora configuraré la frecuencia de salto de la interrupcion para ello utilizaré una frecuencia de 20HZ (debido a
    //	que quiero que el LED parpadee a 10HZ y debo usar una interrupcion para encenderlo y otra para apagarlo. También
    //	comenzaré dividiendo la frecuencia del sistema para obtener esa frecuencia. Por último le resto 1 ya que la interrupción
    //	salta cuando la cuenta llega a cero.
    	ulPeriod = (SysCtlClockGet()/100);
    	TimerLoadSet(TIMER0_BASE, TIMER_A, ulPeriod-1);

    //	Aquí habilitaremos la interrupcion del evento TIMEOUT (en este caso del Timer0A), habiendo habilitado las interrupciones
    //	asociadas al Timer0A y por último habilitaremos todas las interrupciones
    	IntEnable(INT_TIMER0A);
    	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    	IntMasterEnable();
	
//    	Habilitamos el TIMER (importante!)
    	TimerEnable(TIMER0_BASE, TIMER_A);

	while(1)
	{
//		Limpiamos banderas de interrupciones (no usado)
//		ADCIntClear(ADC0_BASE, 1);
		ADCProcessorTrigger(ADC0_BASE, 1); //??


//		while(!ADCIntStatus(ADC0_BASE, 1, false))//?????????????????
//		{
//		}

//		Medimos la temperatura y hacemos los calculos necesarios para expresarlo (habiendo hecho
//		un promedio de por medio para mejor exactitud) en C o en F
		ADCSequenceDataGet(ADC0_BASE, 1, ulADC0Value);
		ulTempAvg = (ulADC0Value[0] + ulADC0Value[1] + ulADC0Value[2] + ulADC0Value[3] + 2)/4;
		ulTempValueC = (1475 - ((2475 * ulTempAvg)) / 4096)/10;
		ulTempValueF = ((ulTempValueC * 9) + 160) / 5;

//		UARTprintf("La temperatura actual es: %d \n",ulTempAvg);
	}
}


//Interrupcion del timer0
void Timer0IntHandler(void)
{
	if(contador>0)
	{
		contador--;

		//Clear the timer interrupt
		TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

		//Mandamos el dato
		UARTprintf("%d \n",ulTempAvg); //
		//	UARTprintf("La temperatura actual es: %d C \n",ulTempValueC);

		//	Prendemos y apagamos el LED de la placa para saber que estamos mandando
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2); //blink LED
		SysCtlDelay(SysCtlClockGet() / (1000 * 3)); //delay ~1 msec
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0); //turn off LED
	}
	else


}


void UART0IntHandler(void)
{

	char*leo;

	UARTgets(leo,2);



}
