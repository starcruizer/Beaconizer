#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "boards.h"
#include "app_util_platform.h"
#include "app_error.h"
#include "nrf_drv_twi.h"
#include "nrf_delay.h"
#include "mpu6050.h"
#include "app_uart.h"


#include "nrf.h"
#include "bsp.h"
#if defined (UART_PRESENT)
#include "nrf_uart.h"
#endif
#if defined (UARTE_PRESENT)
#include "nrf_uarte.h"
#endif

///* TWI instance ID. */
#define TWI_INSTANCE_ID     0

float MPU6050_Acc[3] = {0};
//float MPU6050_Gyro[3] = {0};
char tx_message[256] ;
/* Indicates if operation on TWI has ended. */
static volatile bool m_xfer_done = false;

/* TWI instance. */
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);

bool i2c_write( uint8_t device_address, uint8_t register_address, uint8_t *value, uint8_t number_of_bytes );
bool i2c_read( uint8_t device_address, uint8_t register_address, uint8_t *destination, uint8_t number_of_bytes );



void twi_init (void)
{
    ret_code_t err_code;

    const nrf_drv_twi_config_t twi_lm75b_config = {
       .scl                = ARDUINO_SCL_PIN,
       .sda                = ARDUINO_SDA_PIN,
       .frequency          = NRF_DRV_TWI_FREQ_100K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = false
    };

    err_code = nrf_drv_twi_init(&m_twi, &twi_lm75b_config, NULL, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&m_twi);
}

bool i2c_write(uint8_t device_address, uint8_t register_address, uint8_t *value, uint8_t number_of_bytes )
{  
     uint8_t w_data[number_of_bytes+1], i;
   
     w_data[0] = register_address;
     for ( i = 0 ; i < number_of_bytes ; i++ )
     {
        w_data[i+1] = value[i];  
     }

     nrf_drv_twi_tx(&m_twi, (device_address)>>1, w_data, number_of_bytes+1, false);
     
     return true;
}

bool i2c_read(uint8_t device_address, uint8_t register_address, uint8_t * destination, uint8_t number_of_bytes)
{    
     nrf_drv_twi_tx(&m_twi, (device_address)>>1, &register_address, 1, true);
     nrf_drv_twi_rx(&m_twi, (device_address)>>1, destination, number_of_bytes);
       
     return true;  
}





#define MAX_TEST_DATA_BYTES     (15U)                /**< max number of test bytes to be used for tx and rx. */
#define UART_TX_BUF_SIZE 256                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 256                         /**< UART RX buffer size. */

void uart_error_handle(app_uart_evt_t * p_event)
{
    if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_communication);
    }
    else if (p_event->evt_type == APP_UART_FIFO_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_code);
    }
}

/* When UART is used for communication with the host do not use flow control.*/
#define UART_HWFC APP_UART_FLOW_CONTROL_DISABLED

void uart_transfer(char ch)
{
  app_uart_put(ch);
}


void UART_config()
{
  uint32_t err_code;

    

    const app_uart_comm_params_t comm_params =
      {
          SER_APP_RX_PIN,
          SER_APP_TX_PIN,
          RTS_PIN_NUMBER,
          CTS_PIN_NUMBER,
          UART_HWFC,
          false,
#if defined (UART_PRESENT)
          NRF_UART_BAUDRATE_115200
#else
          NRF_UARTE_BAUDRATE_115200
#endif
      };

    APP_UART_FIFO_INIT(&comm_params,
                         UART_RX_BUF_SIZE,
                         UART_TX_BUF_SIZE,
                         uart_error_handle,
                         APP_IRQ_PRIORITY_LOWEST,
                         err_code);

    APP_ERROR_CHECK(err_code);
}

int main(void)
{
    UART_config();
    twi_init();	//I2C Init
    nrf_delay_ms(50);
   
    MPU6050_Init();	
    MPU6050_Acc[0]=MPU6050_Acc[1]=MPU6050_Acc[2]=0.0;
    while (true)
    {
        memset(tx_message, '\0', sizeof(tx_message));
        nrf_delay_ms(50);
        MPU6050_GetAccData(MPU6050_Acc);
        //MPU6050_Acc[0]=2.3;MPU6050_Acc[1]=3.3;MPU6050_Acc[2]=4.3;
        //transmit accelerometer data
        int val=sqrt(MPU6050_Acc[0]*MPU6050_Acc[0]+MPU6050_Acc[1]*MPU6050_Acc[1]+MPU6050_Acc[2]*MPU6050_Acc[2]);
        if(val>10)// motion encountered
        {
          //sprintf(tx_message,"A: %d; %d; %d; val=%d \r\n",(int)MPU6050_Acc[0],(int)MPU6050_Acc[1],(int)MPU6050_Acc[2], val);
            sprintf(tx_message, "Interrupt fired at g value=%d \r\n", val);
          for(int i=0;i<=sizeof(tx_message);i++)
          {
              uart_transfer(tx_message[i]);
          }
        }
        nrf_delay_ms(50);
    }
}