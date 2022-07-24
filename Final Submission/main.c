// LED Matrix app
//
// Display messages on the LED matrix

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>


#include "nrf_delay.h"

#include "led_matrix.h"
#include "microbit_v2.h"
#include "lsm303agr.h"
#include "nrf_twi_mngr.h"
#include "rng.h"

NRF_TWI_MNGR_DEF(twi_mngr_instance, 1, 0);

int main(void) {
  printf("Board started!\n");
  map_char('!');
  //initialize led_matrix and rng peripheral
  led_matrix_init();
  rng_init();

  // Initialize I2C peripheral and driver
  nrf_drv_twi_config_t i2c_config = NRF_DRV_TWI_DEFAULT_CONFIG;
  i2c_config.scl = I2C_SCL;
  i2c_config.sda = I2C_SDA;
  i2c_config.frequency = NRF_TWIM_FREQ_100K;
  i2c_config.interrupt_priority = 0;
  nrf_twi_mngr_init(&twi_mngr_instance, &i2c_config);

  // Initialize the LSM303AGR accelerometer/magnetometer sensor
  lsm303agr_init(&twi_mngr_instance);


  //testing purposes
  // lsm303agr_measurement_t accel;
  //float theta;
  


  // loop forever
  while (1) {
    //testing purposes
    //Acceleration
    // accel = lsm303agr_read_accelerometer();
    //printf("Accel_X: %f     Accel_Y: %f     Accel_Z: %f\n", accel.x_axis, accel.y_axis, accel.z_axis);
    //Tilt
    //theta = get_theta(accel);
    //printf("Tilt angel theta: %f\n", theta);
    
    nrf_delay_ms(1000);
  }
}

