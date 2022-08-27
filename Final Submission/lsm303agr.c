/**
 * @file lsm303agr.c
 * @author Leanna Nguyen
 * @version 1
 * @date 6/2020
 *
 * @brief Driver for the LSM303AGR accelerometer and magnetometer
 *
 * @note
 * Initializes sensor and communicates over I2C
 * Capable of reading temperature, acceleration, and magnetic field strength, and
 * angles for calculating tilt
 */

#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "lsm303agr.h"
#include "nrf_delay.h"

// Pointer to an initialized I2C instance to use for transactions, will be initialized in the sensor init functinos
static const nrf_twi_mngr_t *i2c_manager = NULL;

/**
 * @brief use I2C to read a regsiter value
 *
 * @param i2c_addr address of device
 * @param reg_addr address of register on the device
 * @return uint8_t value
 */
static uint8_t i2c_reg_read(uint8_t i2c_addr, uint8_t reg_addr)
{
  uint8_t rx_buf = 0; // initialize return buffer

  // i2c transfer struct
  nrf_twi_mngr_transfer_t const read_transfer[] = {
      NRF_TWI_MNGR_WRITE(i2c_addr, &reg_addr, 1, NRF_TWI_MNGR_NO_STOP), // write the register address (one byte)
      NRF_TWI_MNGR_READ(i2c_addr, &rx_buf, 1, 0)                        // read the value (one byte), essentially a repeted start condition
  };
  nrf_twi_mngr_perform(i2c_manager, NULL, read_transfer, 2, NULL); // perform the transfer with the I2C manager instance
  return rx_buf;
}

/**
 * @brief  Helper function to perform a 1-byte I2C write of a given register
 *
 * @param i2c_addr address of the device to write to
 * @param reg_addr address of the register within the device to write
 * @param data write data
 */
static void i2c_reg_write(uint8_t i2c_addr, uint8_t reg_addr, uint8_t data)
{
  // there should only be a single two-byte transfer to be performed
  // values are sent MSB first?
  uint8_t transfer_data2[2] = {reg_addr, data};

  // i2c transfer struct
  nrf_twi_mngr_transfer_t const write_transfer[] = {
      NRF_TWI_MNGR_WRITE(i2c_addr, transfer_data2, 2, 0) // perform a two byte read, sending reg address, then the data
  };
  nrf_twi_mngr_perform(i2c_manager, NULL, write_transfer, 1, NULL); // perform the transfer with the I2C manager instance
}

/**
 * @brief Initialize and configure the LSM303AGR accelerometer/magnetometer
 *
 * @param i2c i2c manager instance
 */
void lsm303agr_init(const nrf_twi_mngr_t *i2c)
{
  i2c_manager = i2c;

  // ---Initialize Accelerometer---

  // Reboot acclerometer
  i2c_reg_write(LSM303AGR_ACC_ADDRESS, LSM303AGR_ACC_CTRL_REG5, 0x80);
  nrf_delay_ms(100); // needs delay to wait for reboot

  // Enable Block Data Update
  // Only updates sensor data when both halves of the data has been read
  i2c_reg_write(LSM303AGR_ACC_ADDRESS, LSM303AGR_ACC_CTRL_REG4, 0x80);

  // Configure accelerometer at 100Hz, normal mode (10-bit)
  // Enable x, y and z axes
  i2c_reg_write(LSM303AGR_ACC_ADDRESS, LSM303AGR_ACC_CTRL_REG1, 0x77);

  // test to verify I2C communication with device ID
  /*
  // Read WHO AM I register
  // Always returns the same value if working
  uint8_t result = i2c_reg_read(LSM303AGR_ACC_ADDRESS, LSM303AGR_ACC_WHO_AM_I_REG);
  //TODO: check the result of the Accelerometer WHO AM I register
  if (result == 0x33)
    {
      printf("Correct Result for WHO_AM_I_A: %d\n",result);
    }
  else
    {
      printf("Incorrect Result for WHO_AM_I_A\n");
    }
  */

  // ---Initialize Magnetometer---

  // Reboot magnetometer
  i2c_reg_write(LSM303AGR_MAG_ADDRESS, LSM303AGR_MAG_CFG_REG_A, 0x40);
  nrf_delay_ms(200); // needs delay to wait for reboot

  // Enable Block Data Update
  // Only updates sensor data when both halves of the data has been read
  i2c_reg_write(LSM303AGR_MAG_ADDRESS, LSM303AGR_MAG_CFG_REG_C, 0x10);

  // Configure magnetometer at 100Hz, continuous mode
  i2c_reg_write(LSM303AGR_MAG_ADDRESS, LSM303AGR_MAG_CFG_REG_A, 0x0C);

  // test to verify I2C communication with device ID
  /*
  // Read WHO AM I register
  result = i2c_reg_read(LSM303AGR_MAG_ADDRESS, LSM303AGR_MAG_WHO_AM_I_REG);
  //TODO: check the result of the Magnetometer WHO AM I register
  if (result == 0x40)
    {
      printf("Correct result for WHO_AM_I_M: %d\n", result);
    }
  else
    {
      printf("Incorrect result for WHO_AM_I_M\n");
    }
  */

  // ---Initialize Temperature---

  // Enable temperature sensor
  i2c_reg_write(LSM303AGR_ACC_ADDRESS, LSM303AGR_ACC_TEMP_CFG_REG, 0xC0);
}

/**
 * @brief Read the internal temperature sensor
 *
 * @return float - measurement in degrees C
 */
float lsm303agr_read_temperature(void)
{
  uint8_t temp_low = i2c_reg_read(LSM303AGR_ACC_ADDRESS, LSM303AGR_ACC_TEMP_L);  // get least significant byte
  uint8_t temp_high = i2c_reg_read(LSM303AGR_ACC_ADDRESS, LSM303AGR_ACC_TEMP_H); // get most significant byte

  uint16_t utemp_16 = (temp_high << 8) | (temp_low); // combine bytes
  int16_t temp_16 = (int16_t)utemp_16;               // cast to signed
  float result = (float)temp_16;                     // cast to float
  result = (result / 256.0) + 25.0;                  // conversion factor according to device datasheet

  return result;
}

/**
 * @brief Get the phi angle
 *
 * @param accel - pass in a measurement struct
 * @return float - calculated phi angle based on 3 axis of acceleration
 */
float get_phi(lsm303agr_measurement_t accel)
{

  float x = accel.x_axis;
  float y = accel.y_axis;
  float z = accel.z_axis;

  float phi = atan(sqrt(x * x + y * y) / z);
  phi = phi * 180.0 / M_PI;

  return phi;
}

/**
 * @brief Get the theta angle
 *
 * @param accel - pass in a measurement struct
 * @return float - calculated theta angle based on 3 axis of acceleration
 */
float get_theta(lsm303agr_measurement_t accel)
{
  float x = accel.x_axis;
  float y = accel.y_axis;
  float z = accel.z_axis;

  float theta = atan(x / sqrt(y * y + z * z));
  theta = theta * 180.0 / M_PI;

  return theta;
}

/**
 * @brief Read the accelerometer using I2C
 *
 * @return lsm303agr_measurement_t - measurement struct
 */
lsm303agr_measurement_t lsm303agr_read_accelerometer(void)
{
  // Read high and low bits of each axis of the acceleration data, and combining the two halfs for each axis
  uint8_t accel_X_L = i2c_reg_read(LSM303AGR_ACC_ADDRESS, LSM303AGR_ACC_OUT_X_L);
  uint8_t accel_X_H = i2c_reg_read(LSM303AGR_ACC_ADDRESS, LSM303AGR_ACC_OUT_X_H);
  uint16_t uaccel_X = (accel_X_H << 8) | (accel_X_L);
  int16_t accel_X = uaccel_X;
  accel_X = accel_X >> 6; // in normal mode, accelerometer data is 10 bits, thus we shift right by 6
  float accel_X_data = (float)accel_X;

  uint8_t accel_Y_L = i2c_reg_read(LSM303AGR_ACC_ADDRESS, LSM303AGR_ACC_OUT_Y_L);
  uint8_t accel_Y_H = i2c_reg_read(LSM303AGR_ACC_ADDRESS, LSM303AGR_ACC_OUT_Y_H);
  uint16_t uaccel_Y = (accel_Y_H << 8) | (accel_Y_L);
  int16_t accel_Y = (int16_t)uaccel_Y;
  accel_Y = accel_Y >> 6; // in normal mode, accelerometer data is 10 bits, thus we shift right by 6
  float accel_Y_data = (float)accel_Y;

  uint8_t accel_Z_L = i2c_reg_read(LSM303AGR_ACC_ADDRESS, LSM303AGR_ACC_OUT_Z_L);
  uint8_t accel_Z_H = i2c_reg_read(LSM303AGR_ACC_ADDRESS, LSM303AGR_ACC_OUT_Z_H);
  uint16_t uaccel_Z = (accel_Z_H << 8) | (accel_Z_L);
  int16_t accel_Z = (int16_t)uaccel_Z;
  accel_Z = accel_Z >> 6; // in normal mode, accelerometer data is 10 bits, thus we shift right by 6
  float accel_Z_data = (float)accel_Z;

  // multiply each value by sensitivity factor, to get mg, and then by .001, to get g
  accel_X_data = accel_X_data * 3.9 * 0.001; // 1LSb = 3.9 mg in normal mode according to datasheet footnote 8 on pg 14
  accel_Y_data = accel_Y_data * 3.9 * 0.001;
  accel_Z_data = accel_Z_data * 3.9 * 0.001;

  lsm303agr_measurement_t measurement = {0};
  measurement.x_axis = accel_X_data;
  measurement.y_axis = accel_Y_data;
  measurement.z_axis = accel_Z_data;
  return measurement;
}

/**
 * @brief read the magnetometer using I2C
 *
 * @return lsm303agr_measurement_t
 */
lsm303agr_measurement_t lsm303agr_read_magnetometer(void)
{
  // Read the magnetometer values, combine them into a 16 bit data value
  uint8_t mag_X_L = i2c_reg_read(LSM303AGR_MAG_ADDRESS, LSM303AGR_MAG_OUT_X_L_REG);
  uint8_t mag_X_H = i2c_reg_read(LSM303AGR_MAG_ADDRESS, LSM303AGR_MAG_OUT_X_H_REG);
  uint16_t umag_X = (mag_X_H << 8) | mag_X_L;
  int16_t mag_X = (int16_t)umag_X;
  float mag_X_data = (float)mag_X;

  uint8_t mag_Y_L = i2c_reg_read(LSM303AGR_MAG_ADDRESS, LSM303AGR_MAG_OUT_Y_L_REG);
  uint8_t mag_Y_H = i2c_reg_read(LSM303AGR_MAG_ADDRESS, LSM303AGR_MAG_OUT_Y_H_REG);
  uint16_t umag_Y = (mag_Y_H << 8) | mag_Y_L;
  int16_t mag_Y = (int16_t)umag_Y;
  float mag_Y_data = (float)mag_Y;

  uint8_t mag_Z_L = i2c_reg_read(LSM303AGR_MAG_ADDRESS, LSM303AGR_MAG_OUT_Z_L_REG);
  uint8_t mag_Z_H = i2c_reg_read(LSM303AGR_MAG_ADDRESS, LSM303AGR_MAG_OUT_Z_H_REG);
  uint16_t umag_Z = (mag_Z_H << 8) | mag_Z_L;
  int16_t mag_Z = (int16_t)umag_Z;
  float mag_Z_data = (float)mag_Z;

  // multiply by sensitivity factor and convert to readible units
  mag_X_data = mag_X_data * 1.5 * 0.1; // 1LSB = 1.5 mgauss
  mag_Y_data = mag_Y_data * 1.5 * 0.1;
  mag_Z_data = mag_Z_data * 1.5 * 0.1;

  lsm303agr_measurement_t measurement = {0};
  measurement.x_axis = mag_X_data;
  measurement.y_axis = mag_Y_data;
  measurement.z_axis = mag_Z_data;

  return measurement;
}
