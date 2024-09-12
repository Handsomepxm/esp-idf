/* I2S Example

    This example code will output 100Hz sine wave and triangle wave to 2-channel of I2S driver
    Every 5 seconds, it will change bits_per_sample [16, 24, 32] for i2s data

    This example code is in the Public Domain (or CC0 licensed, at your option.)

    Unless required by applicable law or agreed to in writing, this
    software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
    CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include <math.h>

//(10000) 3.2ms   50us   110us
//(15000) 2ms     33us   74us 
//(20000) 1.6ms   25us   58us 
//(25000) 1.28ms  20us   47us
//(30000) 1ms     17us   39us 
//(35000) 910us   14.3us 35us 
//(40000) 800us   12.5us 32us 


#define SAMPLE_RATE     (30000)
//#define SAMPLE_RATE     (44100)
#define CLK_NUM  32

#define I2S_NUM         (0)
#define WAVE_FREQ_HZ    (100)
#define PI              (3.14159265)
#define I2S_BCK_IO      (GPIO_NUM_13)
#define I2S_WS_IO       (GPIO_NUM_15)
#define I2S_DO_IO       (GPIO_NUM_12)//GPIO_NUM_21
#define I2S_DI_IO       (-1)

#define SAMPLE_PER_CYCLE (SAMPLE_RATE/WAVE_FREQ_HZ)
#if 0
static void setup_triangle_sine_waves(int bits)
{
    int *samples_data = malloc(((bits+8)/16)*SAMPLE_PER_CYCLE*4);
    unsigned int i, sample_val;
    double sin_float, triangle_float, triangle_step = (double) pow(2, bits) / SAMPLE_PER_CYCLE;
    size_t i2s_bytes_write = 0;

    printf("\r\nTest bits=%d free mem=%d, written data=%d\n", bits, esp_get_free_heap_size(), ((bits+8)/16)*SAMPLE_PER_CYCLE*4);

    triangle_float = -(pow(2, bits)/2 - 1);

	#if 1
    for(i = 0; i < SAMPLE_PER_CYCLE; i++) {
        sin_float = sin(i * 2 * PI / SAMPLE_PER_CYCLE);
        if(sin_float >= 0)
            triangle_float += triangle_step;
        else
            triangle_float -= triangle_step;

        sin_float *= (pow(2, bits)/2 - 1);

        if (bits == 16) {
            sample_val = 0;
            sample_val += (short)triangle_float;
            sample_val = sample_val << 16;
            sample_val += (short) sin_float;
            samples_data[i] = sample_val;
        } else if (bits == 24) { //1-bytes unused
            samples_data[i*2] = ((int) triangle_float) << 8;
            samples_data[i*2 + 1] = ((int) sin_float) << 8;
        } else {
            //samples_data[i*2] = ((int) triangle_float);
            //samples_data[i*2 + 1] = ((int) sin_float);
            samples_data[i*2] = 0XAAAAAAAA;
            samples_data[i*2 + 1] = 0XAAAAAAAA;
        }

    }
	#endif
	for(i = SAMPLE_PER_CYCLE/2; i < SAMPLE_PER_CYCLE; i++) 	
	{
		samples_data[i*2] = 0XF0F0F0F0;
		//samples_data[i*2 + 1] = 0XF0F0F0F0;
		samples_data[i*2 + 1] = 0XAAAAAAAA;
	}
	
    i2s_set_clk(I2S_NUM, SAMPLE_RATE, bits, 2);
    //Using push
    // for(i = 0; i < SAMPLE_PER_CYCLE; i++) {
    //     if (bits == 16)
    //         i2s_push_sample(0, &samples_data[i], 100);
    //     else
    //         i2s_push_sample(0, &samples_data[i*2], 100);
    // }
    // or write

	i2s_write(I2S_NUM, samples_data, ((bits+8)/16)*SAMPLE_PER_CYCLE*4, &i2s_bytes_write, 100);	

    free(samples_data);
}
#endif


void set_gpio_init(uint32_t io_num)
{
	//管脚初始化，45
	gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = ((uint64_t)0X1<<io_num);//GPIO_OUTPUT_PIN_SEL
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}

void set_gpio_level(uint32_t io_num, uint32_t level)
{
	gpio_set_level(io_num, level);
}



static esp_timer_handle_t scan_timer;
int *samples_data;
int *samples_data2;
static uint64_t time1;
static uint64_t time2;


static void scan_timer_isr(void* arg)
{
	static int cnt;	
	size_t i2s_bytes_write = 0;
	#if 0
	cnt++;
	if(cnt&0x01)		
	{
		i2s_write(I2S_NUM, samples_data, ((32+8)/16)*CLK_NUM*4, &i2s_bytes_write, portMAX_DELAY);	
		//i2s_write(I2S_NUM, samples_data, ((32+8)/16)*CLK_NUM*4, &i2s_bytes_write, portMAX_DELAY);	
	}
	else
	{
		i2s_write(I2S_NUM, samples_data2, ((32+8)/16)*CLK_NUM*4, &i2s_bytes_write, portMAX_DELAY);	
		//i2s_write(I2S_NUM, samples_data2, ((32+8)/16)*CLK_NUM*4, &i2s_bytes_write, portMAX_DELAY);	
	}	
	#endif
	//i2s_stop(I2S_NUM);
	//i2s_write(I2S_NUM, samples_data2, ((32+8)/16)*CLK_NUM*4, &i2s_bytes_write, portMAX_DELAY);	
		
	esp_timer_start_once(scan_timer, 2000); 	
	#if 0
	time1 = esp_timer_get_time();
	i2s_write(I2S_NUM, samples_data, ((32+8)/16)*CLK_NUM*4, &i2s_bytes_write, portMAX_DELAY);	
	time2 = esp_timer_get_time();
	//printf("time:%lld \r\n",time2-time1);
	//i2s_write(I2S_NUM, samples_data2, ((32+8)/16)*CLK_NUM*4, &i2s_bytes_write, portMAX_DELAY);	
	//i2s_start(I2S_NUM);
	#endif
	/*
	cnt++;
	if(cnt==10000)
	{
		i2s_set_sample_rates(I2S_NUM,25000);
	
	}
	*/
	//set_gpio_level(14, 1);
	//ets_delay_us(300);	
	//set_gpio_level(14, 0);
	i2s_stop(I2S_NUM);	
	i2s_start(I2S_NUM);
	#if 0
	cnt++;
	if(cnt&0x01)
		set_gpio_level(14, 1);
	else
		set_gpio_level(14, 0);
	#endif
		
}


void scan_timer_init(void)//扫描定时器
{
	const esp_timer_create_args_t scan_timer_args = {
            .callback = &scan_timer_isr,
            /* name is optional, but may help identify the timer when debugging */
            .name = "scan_timer"
    };

	esp_timer_create(&scan_timer_args, &scan_timer);
	esp_timer_start_once(scan_timer, 500);//500us
	//esp_timer_start_periodic(scan_timer, 250);
	//scan_pwm_timer_init();
}


static void output_data_test(int bits)
{
	size_t i2s_bytes_write = 0;
	unsigned int i;
	
	samples_data = malloc(((bits+8)/16)*CLK_NUM*4);	
	for(i=0;i<CLK_NUM/2;i++)
	{		
		samples_data[i*2] = 0XFFFFFFFF;//0XFFFFFFFF
		samples_data[i*2 + 1] = 0XFFFFFFFF;//0XAAAAAAAA		
	}
	for(i=CLK_NUM/2;i<CLK_NUM;i++)
	{		
		samples_data[i*2] = 0XFFFFFFFF;//0XFFFFFFFF
		samples_data[i*2 + 1] = 0XFFFFFFFF;//0XAAAAAAAA		
	}
	
	samples_data2 = malloc(((bits+8)/16)*CLK_NUM*4);
	for(i=0;i<CLK_NUM/2;i++)
	{		
		samples_data2[i*2] = 0XFFFFFFFF;//0XFFFFFFFF
		samples_data2[i*2 + 1] = 0XFFFFFFFF;//0XAAAAAAAA 	
	}	
	for(i=CLK_NUM/2;i<CLK_NUM;i++)
	{
		samples_data2[i*2] = 0;
		samples_data2[i*2 + 1] = 0;//0XAAAAAAAA
	}
	

	i2s_set_clk(I2S_NUM, SAMPLE_RATE, bits, 2);			
	i2s_write(I2S_NUM, samples_data, ((bits+8)/16)*CLK_NUM*4, &i2s_bytes_write, portMAX_DELAY);	
	i2s_write(I2S_NUM, samples_data, ((bits+8)/16)*CLK_NUM*4, &i2s_bytes_write, portMAX_DELAY);	
	//i2s_stop(I2S_NUM);
	scan_timer_init();
	set_gpio_init(14);
	set_gpio_level(14, 0);
	static int cnt;	
	while(1)
	{	
		#if 0
		if(cnt&0X01)
		{
			i2s_write(I2S_NUM, samples_data, ((32+8)/16)*CLK_NUM*4, &i2s_bytes_write, portMAX_DELAY);	
			i2s_write(I2S_NUM, samples_data, ((32+8)/16)*CLK_NUM*4, &i2s_bytes_write, portMAX_DELAY);	
		}	
		else
		{
			i2s_write(I2S_NUM, samples_data2, ((32+8)/16)*CLK_NUM*4, &i2s_bytes_write, portMAX_DELAY);	
			i2s_write(I2S_NUM, samples_data2, ((32+8)/16)*CLK_NUM*4, &i2s_bytes_write, portMAX_DELAY);	
		}
			
		cnt++;
		#endif
		vTaskDelay(1/portTICK_RATE_MS);
	}	
	free(samples_data);
	free(samples_data2);
}

void app_main(void)
{
    //for 36Khz sample rates, we create 100Hz sine wave, every cycle need 36000/100 = 360 samples (4-bytes or 8-bytes each sample)
    //depend on bits_per_sample
    //using 6 buffers, we need 60-samples per buffer
    //if 2-channels, 16-bit each channel, total buffer is 360*4 = 1440 bytes
    //if 2-channels, 24/32-bit each channel, total buffer is 360*8 = 2880 bytes
    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX,                                  // Only TX
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = 16,//I2S_BITS_PER_SAMPLE_16BIT
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,                           //2-channels
        .communication_format = I2S_COMM_FORMAT_STAND_MSB,
        .dma_buf_count = 2,//6
        .dma_buf_len = CLK_NUM,//60
        .use_apll = true,//false
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,                                //Interrupt level 1
        //.tx_desc_auto_clear = 1
    };
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCK_IO,
        .ws_io_num = I2S_WS_IO,
        .data_out_num = I2S_DO_IO,
        .data_in_num = I2S_DI_IO                                               //Not used
    };
    i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM, &pin_config);

    int test_bits = 16;
    while (1) {
		test_bits = 32;	
        //setup_triangle_sine_waves(test_bits);
        output_data_test(32); 		
        //printf("in %lld microsecond ... \t\n", esp_timer_get_time());
        vTaskDelay(1000/portTICK_RATE_MS);
        test_bits += 8;
        if(test_bits > 32)
            test_bits = 16;

    }

}
