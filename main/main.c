
#include <stdio.h>
#include "unity.h"
#include "driver/i2c.h"
#include "mpu6050.h"
#include "esp_system.h"
#include "esp_log.h"

#define I2C_MASTER_SCL_IO 1      /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 0     /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_0  /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ 100000 /*!< I2C master clock frequency */

#define SAMPLE_PERIOD_MS 1
#define DURATION_SEC 1
#define NUM_SAMPLES 10000

typedef struct {
    float accx;
    float accy;
    float accz;
    float gyrox;
    float gyroy;
    float gyroz;
    float temp;
} sensor_data;

sensor_data data[NUM_SAMPLES]; //initialize struct with specified num of samples


static const char *TAG = "mpu6050 test";
static mpu6050_handle_t mpu6050 = NULL;

/**
 * @brief i2c master initialization
 */
static void i2c_bus_init(void)
{
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = (gpio_num_t)I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = (gpio_num_t)I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    conf.clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL;

    esp_err_t ret = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C config returned error");
        return;
    }

    ret = i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C install returned error");
        return;
    }
}

/**
 * @brief i2c master initialization
 */
static void i2c_sensor_mpu6050_init(void)
{
    esp_err_t ret;

    i2c_bus_init();
    mpu6050 = mpu6050_create(I2C_MASTER_NUM, MPU6050_I2C_ADDRESS);
    if (mpu6050 == NULL)
    {
        ESP_LOGE(TAG, "MPU6050 create returned NULL");
        return;
    }

    ret = mpu6050_config(mpu6050, ACCE_FS_4G, GYRO_FS_500DPS);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "MPU6050 config error");
        return;
    }

    ret = mpu6050_wake_up(mpu6050);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "MPU6050 wake up error");
        return;
    }
}

void write_to_csv(sensor_data* data, int num_samples){
    FILE* f = fopen("C:\\Users\\ryanz\\Documents\\CS528\\gyroscope\\IMU\\sensor_data.csv", "w");
    
    if (f == NULL){
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }

    fprintf(f, "accx, accy, accz, gyrox, gyroy, gyroz, temp\n");

    for (int i = 0; i < num_samples; i++) {
        fprintf(f, "%.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f",
        data[i].accx, data[i].accy, data[i].accz, 
        data[i].gyrox, data[i].gyroy, data[i].gyroz, data[i].temp);
    }

    fclose(f);
    ESP_LOGI(TAG, "info logged to csv");

}

void app_main()
{
    esp_err_t ret;
    uint8_t mpu6050_deviceid;
    mpu6050_acce_value_t acce;
    mpu6050_gyro_value_t gyro;
    mpu6050_temp_value_t temp;

    i2c_sensor_mpu6050_init();

    ret = mpu6050_get_deviceid(mpu6050, &mpu6050_deviceid);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get MPU6050 device ID");
    }

    int sample_index = 0;
    ESP_LOGI(TAG, "Sample,AccX,AccY,AccZ,GyroX,GyroY,GyroZ,Temp\n");
    while (1){
        ret = mpu6050_get_acce(mpu6050, &acce);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to get accelerometer data");
        }

        //ESP_LOGI(TAG, "acce_x:%.2f, acce_y:%.2f, acce_z:%.2f\n", acce.acce_x, acce.acce_y, acce.acce_z);

        ret = mpu6050_get_gyro(mpu6050, &gyro);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to get gyroscope data");
        }

        //ESP_LOGI(TAG, "gyro_x:%.2f, gyro_y:%.2f, gyro_z:%.2f\n", gyro.gyro_x, gyro.gyro_y, gyro.gyro_z);

        ret = mpu6050_get_temp(mpu6050, &temp);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to get temperature data");
        }

        //ESP_LOGI(TAG, "t:%.2f \n", temp.temp);

        sample_index++;
        //ESP_LOGI(TAG, "sample:%d \n", sample_index);
        ESP_LOGI(TAG,"%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
               sample_index + 1, 
               acce.acce_x, 
               acce.acce_y, 
               acce.acce_z, 
               gyro.gyro_x, 
               gyro.gyro_y, 
               gyro.gyro_z, 
               temp.temp);

        vTaskDelay(SAMPLE_PERIOD_MS);
    }

    
    
    mpu6050_delete(mpu6050);
    ret = i2c_driver_delete(I2C_MASTER_NUM);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to delete I2C driver");
    }
}