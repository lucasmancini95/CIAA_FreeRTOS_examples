
// Includes de FreeRTOS
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

// sAPI header
#include "sapi.h"

/*==================[definiciones y macros]==================================*/

   #define LED_1 LED1
   #define LED_2 LED2
   #define LED_3 LED3
   #define MASK_LED_1 (0x01)
   #define MASK_LED_2 (0x02)
   #define MASK_LED_3 (0x04)
   #define TIEMPO_PERMANENCIA 5000
   #define MAX_SIZE_PAQUETE 10

/*==================[definiciones de datos]=========================*/

xSemaphoreHandle sem_act;
QueueHandle_t xQueue_transm_datos;

/*==================[declaraciones de funciones/tareas]====================*/

// Prototipo de funcion de la tarea
void data_receiver( void* taskParmPtr );

//Funcion de inicializacion de interrupciones
void IRQ_Init(void);
// Funcion Handler de ISR UART_USB de Recepcion
void handler_dato_recibido(void *noUsado);
/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main(void){
// -------------CONFIGURACIONES ------------------------------
// Inicializar y configurar la plataforma
	   boardConfig();

//-------Inicializacion Interrupciones de la UART_USB----------
		IRQ_Init();

//Creacion de la colas
		xQueue_transm_datos = xQueueCreate(MAX_SIZE_PAQUETE, sizeof(uint8_t));
		if (xQueue_transm_datos == NULL) {
			printf("No se pudo crear la COLA\n");
			gpioWrite(LED1, ON);
			while (1)
				;
			/* se queda bloqueado el sistema hasta
			 que venga el técnico de mantenmiento   */
		}

//-----------------Inicializacion de los semaforos--------------

	   //sem_act=  vSemaphoreCreateBinary();
	   
//---------------Inicializacion de tareas------------------------

	   //Inicializacion de tareas
	   xTaskCreate(
		data_receiver,                     // Funcion de la tarea a ejecutar
	      (const char *)"data_receiver",     // Nombre de la tarea como String amigable para el usuario
	      configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
	      0,                          // Parametros de tarea
	      tskIDLE_PRIORITY+1,         // Prioridad de la tarea
	      0                           // Puntero a la tarea creada en el sistema
	   );


	   printf("Ingresar dato 1\r\n ");



// -------------Iniciar scheduler--------------------------
	   vTaskStartScheduler();

	   // ---------- REPETIR POR SIEMPRE --------------------------
	   while( TRUE ) {
	      // Si cae en este while 1 significa que no pudo iniciar el scheduler
	   }
	   return 0;
}


/*==================[definiciones de funciones/tareas]=====================*/


void data_receiver( void* taskParmPtr ){
	uint8_t received_value;
   while(TRUE) {
	   if (xQueueReceive( xQueue_transm_datos, &received_value, portMAX_DELAY) == pdTRUE ){
		   gpioWrite( LED_1,received_value & MASK_LED_1);
		   gpioWrite( LED_2,received_value & MASK_LED_2);
		   gpioWrite( LED_3,received_value & MASK_LED_3);
		   //received_value &= 0;
		   uartWriteString(UART_USB, "The value :" );
		   uartTxWrite(UART_USB,received_value);
		   uartWriteString(UART_USB, " was printed binary in the LEDS (3 LSB)\n" );
		   vTaskDelay( (TIEMPO_PERMANENCIA)/ portTICK_RATE_MS);
	   }
   }
}

//------------funciones de interrupciones-------------------

void IRQ_Init(void){

	/* Inicializar la UART_USB junto con las interrupciones de  Rx */
	uartConfig(UART_USB, 115200);

	// Seteo un callback al evento de recepcion y habilito su interrupcion
	uartCallbackSet(UART_USB, UART_RECEIVE, handler_dato_recibido, NULL);

	uartInterrupt(UART_USB, TRUE);
}

void handler_dato_recibido(void *noUsado) {

	uint8_t rx;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	rx = uartRxRead(UART_USB);
	xQueueSendFromISR(xQueue_transm_datos,&rx, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

}


/*==================[fin del archivo]========================================*/
