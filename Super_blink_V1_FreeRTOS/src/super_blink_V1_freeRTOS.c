
// Includes de FreeRTOS
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
# include "semphr.h"


// sAPI header
#include "sapi.h"

/*==================[definiciones y macros]==================================*/

   #define LED_ROJO     LED1
   #define LED_AMARILLO LED2
   #define LED_VERDE    LED3
   #define PULSADOR_ACT  TEC1
   #define PULSADOR_EMG  TEC4
   #define TIEMPO_V	500
   #define TIEMPO_A	2000
   #define TIEMPO_R	2500
   #define TIEMPO_P	5000

/*==================[definiciones de datos]=========================*/

xSemaphoreHandle sem_act; //este semaforo (contador) va a ser dado por la tarea "pulsador" para que se realice la accion de actuador (encender el led amarillo un tiempo)
xSemaphoreHandle sem_emg; //este semaforo va a ser dado por la tarea "pulsador" para que se realice la accion de emergencia
xSemaphoreHandle sem_stop_verde; //este semaforo va a ser dado por la tarea de emergencia para que se realice la accion de inactivar el led verde
xSemaphoreHandle sem_stop_amarillo; //este semaforo va a ser dado por la tarea de emergencia para que se realice la accion de inactivar el led amarillo

/*==================[declaraciones de funciones/tareas]====================*/

// Prototipo de funcion de la tarea
void Tarea_pulsador( void* taskParmPtr ); // se encarga de administrar los puladores y sus respectivos semaforos
void Tarea_blink_verde( void* taskParmPtr ); //tarea continua se ejecuta siempre que se pueda (pero se bloque al accionar el led) (ese detiene ante una emergencia)
void Tarea_blink_amarillo( void* taskParmPtr ); //tarea que se ejecuta por cierto tiempo en ele momento que pase el evento del pulsador
void Tarea_blink_rojo( void* taskParmPtr ); // tarea a de emergencia que va a detener las otras acciones (bloqueara las otras tareas por un tiempo)


/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main(void)
{
   // ---------- CONFIGURACIONES ------------------------------
   // Inicializar y configurar la plataforma
   boardConfig();

   /*Inicializacion de los semaforos*/

   sem_act= xSemaphoreCreateBinary(); //xSemaphoreCreateCounting(3, 0); //creo semaforo contador para poder registrar la cantidad de eventos que ocurren y que la tarea que lleva a cabo la accion no se pierda nada
   sem_emg=xSemaphoreCreateBinary(); //creo el semaforo
   sem_stop_verde=xSemaphoreCreateBinary();
   sem_stop_amarillo=xSemaphoreCreateBinary();


   // Crear tarea en freeRTOS

   xTaskCreate(
	Tarea_pulsador,                     // Funcion de la tarea a ejecutar
      (const char *)"Tarea_pulsador",     // Nombre de la tarea como String amigable para el usuario
      configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
      0,                          // Parametros de tarea
      tskIDLE_PRIORITY+1,         // Prioridad de la tarea
      0                           // Puntero a la tarea creada en el sistema
   );


   xTaskCreate(
	Tarea_blink_verde,                     // Funcion de la tarea a ejecutar
      (const char *)"Tarea_blink_verde",     // Nombre de la tarea como String amigable para el usuario
      configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
      0,                          // Parametros de tarea
      tskIDLE_PRIORITY+2,         // Prioridad de la tarea
      0                           // Puntero a la tarea creada en el sistema
   );

   xTaskCreate(
	Tarea_blink_amarillo,                     // Funcion de la tarea a ejecutar
      (const char *)"Tarea_blink_amarillo",     // Nombre de la tarea como String amigable para el usuario
      configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
      0,                          // Parametros de tarea
      tskIDLE_PRIORITY+3,         // Prioridad de la tarea
      0                           // Puntero a la tarea creada en el sistema
   );

  xTaskCreate(
	Tarea_blink_rojo,                     // Funcion de la tarea a ejecutar
      (const char *)"Tarea_blink_rojo",     // Nombre de la tarea como String amigable para el usuario
      configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
      0,                          // Parametros de tarea
      tskIDLE_PRIORITY+4,         // Prioridad de la tarea
      0                           // Puntero a la tarea creada en el sistema
   );

   // Iniciar scheduler
   vTaskStartScheduler();

   // ---------- REPETIR POR SIEMPRE --------------------------
   while( TRUE ) {
      // Si cae en este while 1 significa que no pudo iniciar el scheduler
   }

   // NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
   // directamenteno sobre un microcontroladore y no es llamado por ningun
   // Sistema Operativo, como en el caso de un programa para PC.
   return 0;
}


/*==================[definiciones de funciones/tareas]=====================*/

void Tarea_pulsador( void* taskParmPtr ){ // Tareas de procesamiento continuo --> tiene prioridad baja y simpre que se pueda va a estar corriendose									//ej: Sensor, muestrear un buffer, esperar datos bluetooth para procesar.
   // ---------- REPETIR POR SIEMPRE --------------------------
//xSemaphoreTake (sem_act,portMAX_DELAY);
   while(TRUE) {
	    if(gpioRead(PULSADOR_ACT)== FALSE){
			  xSemaphoreGive(sem_act);
			  vTaskDelay( 1000/ portTICK_RATE_MS );
		  }
	    if(gpioRead(PULSADOR_EMG)== FALSE){
			  xSemaphoreGive(sem_emg);
			  vTaskDelay( TIEMPO_P/ portTICK_RATE_MS );
		  }
   }
}


void Tarea_blink_verde( void* taskParmPtr ){
   // ---------- REPETIR POR SIEMPRE --------------------------
   while(TRUE) {
		  gpioWrite( LED_VERDE, ON );
		  vTaskDelay( TIEMPO_V/ portTICK_RATE_MS ); // durante este delay la tarea se blockea --> se divide el tiempo por el macro de tickrate para traducirlo al tiempo del
		  gpioWrite( LED_VERDE, OFF);
		  vTaskDelay( TIEMPO_V/ portTICK_RATE_MS );

		   if(xSemaphoreTake(sem_stop_verde , 10/ portTICK_RATE_MS) == pdTRUE ){   /* Si se tiene el semáforo : se activa el led amarillo*/
			   gpioWrite( LED_VERDE, OFF);
			   vTaskDelay( (TIEMPO_R*2)/ portTICK_RATE_MS);
		   }
	   }
}


void Tarea_blink_amarillo( void* taskParmPtr ){
   // ---------- REPETIR POR SIEMPRE --------------------------

   while(TRUE) {
	   if(xSemaphoreTake (sem_act, 20/ portTICK_RATE_MS) == pdTRUE ){   /* Si se tiene el semáforo : se activa el led amarillo*/ //
		   gpioWrite( LED_AMARILLO, !gpioRead(LED_AMARILLO));
	   }
	 if(xSemaphoreTake (sem_stop_amarillo, 20/ portTICK_RATE_MS) == pdTRUE ){   /* Si se tiene el semáforo : se activa el led amarillo*/
		gpioWrite( LED_AMARILLO, OFF);
	   }
   }
}

void Tarea_blink_rojo( void* taskParmPtr ){

   // ---------- REPETIR POR SIEMPRE --------------------------
   while(TRUE) {
	   if(xSemaphoreTake (sem_emg,portMAX_DELAY) == pdTRUE ){   /* Si se tiene el semáforo del pulsador : se activa el led rojo*/
		gpioWrite( LED_ROJO, ON);
		xSemaphoreGive(sem_stop_amarillo); //activo el semaforo que hara que el led se apague por un tiempo
		xSemaphoreGive(sem_stop_verde); //activo el semaforo que hara que el led se apague por un tiempo
		vTaskDelay( TIEMPO_R/ portTICK_RATE_MS );
		gpioWrite( LED_ROJO, OFF);
		//xSemaphoreGive(sem_act);
		}
   }
}

/*==================[fin del archivo]========================================*/
