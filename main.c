#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

/* Estructura para el LCD */
typedef struct {
  uint8_t rs;		/* GPIO de RS */
  uint8_t en;		/* GPIO de EN */
  uint8_t d4;		/* GPIO de D4 */
  uint8_t d5;		/* GPIO de D5 */
  uint8_t d6;		/* GPIO de D6 */
  uint8_t d7;		/* GPIO de D7 */
} lcd_t;

/* Inline functions */

/* Genera una estructura por defecto para el LCD */
static inline lcd_t lcd_get_default_config(void) {
  lcd_t lcd = { 0, 1, 2, 3, 4, 5 };
  return lcd;
}

/* Obtiene la máscara de pines de datos */
static inline uint32_t lcd_get_data_mask(lcd_t lcd) {
  return 1 << lcd.d4 | 1 << lcd.d5 | 1 << lcd.d6 | 1 << lcd.d7;
}

/* Obtiene la mascara de pines conectadas al LCD */
static inline uint32_t lcd_get_mask(lcd_t lcd) {
  return 1 << lcd.rs | 1 << lcd.en | lcd_get_data_mask(lcd);
}

/* Function prototypes */
void lcd_init(lcd_t lcd);
void lcd_put_nibble(lcd_t lcd, uint8_t nibble);
void lcd_put_command(lcd_t lcd, uint8_t cmd);
void lcd_putc(lcd_t lcd, char c);
void lcd_puts(lcd_t lcd, const char* str);
void lcd_clear(lcd_t lcd);
void lcd_go_to_xy(lcd_t lcd, uint8_t x,  uint8_t y);


/* Programa principal */

int main() {
	/* Inicializo el USB */
  stdio_init_all();
	/* Obtengo una variable para elegir los pines del LCD */
  lcd_t lcd = lcd_get_default_config();
	/* Por defecto, los pines son:
	 	- GP0 (RS)
		- GP1 (EN) 
		- GP2 (D4)
		- GP3 (D5)
		- GP4 (D6)
		- GP5 (D7)

  /*Inicio de entrada de datos del infrarojo*/
  adc_init ();
  adc_gpio_init (28);
  gpio_select_input (0);

  /*Alimentación del buzzer y del servo */
  gpio_init (7);
  gpio_set_dir(7, true);
  /*salida al control del servo*/
  gpio_init (8);
  gpio_set_dir(8, true);
  
  float high_peak = 0;
  float low_peak = 0;
  float absolute_time_to = 0;
  float absolute_time_from = 0;
	float time = 0;
  float heart_rate = 0;
  float no_pulse = 0;

	/* Inicializo el LCD */
	lcd_init(lcd);

  while (true) {
		uint16_t voltage_value = adc_read();

    /*        PWM y Alimentación          */
    if (no_pulse >= 10){
      gpio_put (7, true);
      while (no_pulse >= 10 ){
        gpio_put (8,true);
        sleep_ms ();
        gpio_put (8,false);
      }
      gpio_put (7, false);
    }


		/* Limpio el LCD. Por defecto, va al 0;0 */
    lcd_clear(lcd);
    
    /*       Algoritmo para los datos de entrada      */

    if (voltage_value > 950) {
      if (high_peak = 0){
        absolute_time_from = get_absolute_time();
        high_peak = 1;

      }
      if (high peak = 2){
        absolute_time_to = get_absolute_time();
        time = absolute_time_to - absolute_time_from;
        time = time / 1000000;
        high_peak = 0;
        no_pulse = 0 ;
      }
		}
    else {
      if (low_peak = 0){
        high_peak = 2;
        no_pulse = no_pulse + 1;
      }
    }
    heart_rate = 60 / time;

		lcd_puts(lcd, "Heart Rate: %f",heart_rate);

		sleep_ms(100);
  }
}

/*
 * 	@brief	Initialize LCD with default configuration
 * 	@param	lcd: struct to an LCD
 */
void lcd_init(lcd_t lcd) {
  /* Get pin mask */
  uint32_t mask = lcd_get_mask(lcd);
  /* Initialize GPIO pins */
  gpio_init_mask(mask);
  /* Set pins direction as output */
  gpio_set_dir_out_masked(mask);
	/* Function set: 8 bits interface length */
	lcd_put_command(lcd, 0x03);
	/* Wait for 4.1 ms */
	sleep_ms(5);
	/* Function set: 8 bits interface length */
	lcd_put_command(lcd, 0x03);
	/* Wait for 100 us */
	sleep_us(100);
	/* Function set: 8 bits interface length */
	lcd_put_command(lcd, 0x03);
	/* Now the other instructions can be send */

	/* Function set: set interface to 4 bits length */
	lcd_put_command(lcd, 0x02);
	/* Function set: interface is 4 bits length */
	lcd_put_command(lcd, 0x02);
  /* Function set: two row display and 5x7 font */
  lcd_put_command(lcd, 0x08 | (false << 2));
  /* Display on, cursor off, blink off */
  lcd_put_command(lcd, 0x00);
  lcd_put_command(lcd, (3 << 2) | (false) | false);
  /* Input set command: increment cursor */
  lcd_put_command(lcd, 0x00);
  lcd_put_command(lcd, 0x06);
}

/*
 * 	@brief	Write a new nibble in data pins
 * 	@param	lcd: LCD pin struct
 * 	@param	nibble: four bit data
 */
void lcd_put(lcd_t lcd, uint8_t nibble) {
  /* Get data pins mask */
  uint32_t mask = lcd_get_data_mask(lcd);
	/* Get value from nibble */
	uint32_t value = 	((nibble & 0x8)? true : false) << lcd.d7 | 
										((nibble & 0x4)? true : false) << lcd.d6 |
										((nibble & 0x2)? true : false) << lcd.d5 | 
										((nibble & 0x1)? true : false) << lcd.d4;
  /* Send nibble */
  gpio_put_masked(mask, value);
	/* Set enable pin */
	gpio_put(lcd.en, true);
	/* Wait for 40 us */
	sleep_us(40);
	/* Clear enable pin */
	gpio_put(lcd.en, false);
}

/*
 * 	@brief	Write a new command in data pins
 * 	@param	lcd: LCD pin struct
 * 	@param	cmd: four bit command
 */
void lcd_put_command(lcd_t lcd, uint8_t cmd) {
	/* Clear rs pin */
	gpio_put(lcd.rs, false);
	/* Put command in data pins */
	lcd_put(lcd, cmd);
}

/*
 * 	@brief	Write a new character in data pins
 * 	@param	lcd: LCD pin struct
 * 	@param	c: char to be sent
 */
void lcd_putc(lcd_t lcd, char c) {
	/* Set rs pin */
	gpio_put(lcd.rs, true);
	/* Repeat twice */
	for(uint8_t nibble = 0; nibble < 2; nibble++) {
		/* Send lower nibble if it's the first time. Send higher nibble if second */
		uint8_t n = (nibble)? c & 0x0f : c >> 4;
		/* Write nibble */
		lcd_put(lcd, n);
	}
}

/*
 * 	@brief	Write a new string in LCD
 * 	@param	lcd: LCD pin struct
 * 	@param	str: string to be sent
 */
void lcd_puts(lcd_t lcd, const char* str) {
	/* Loop intul null character */
	while(*str) {
		/* Write the char */
		lcd_putc(lcd, *str);
		/* Increment pointer */
		str++;
	}
}

/*
 * 	@brief	Clear screen screen
 * 	@param	lcd: LCD pin struct
 */
void lcd_clear(lcd_t lcd) {
	/* Send clear command:  first nibble */
	lcd_put_command(lcd, 0x00);
	/* Send clear command: second nibble */
	lcd_put_command(lcd, 0x01);
	/* Wait for 4ms */
	sleep_ms(4);
}

/*
 * 	@brief	Go to starting coordinate
 * 	@param	lcd: LCD pin struct
 * 	@param	x: column number (starting in 0)
 * 	@param	y: row number (starting in 0)
 */
void lcd_go_to_xy(lcd_t lcd, uint8_t x, uint8_t y) {
	/* Auxiliary variable */
	uint8_t aux;
	/* Check if the first row (0) is required */
	if(y == 0) {
		/* Set direction */
		aux = 0x40 + x;
		/* Send first nibble */
		lcd_put_command(lcd, aux >> 4);
		/* Send seconds byte */
		lcd_put_command(lcd, aux & 0x0F);
	}
	/* Check if the second row (2) is required  */
	else if (y == 1) {
		/* Set direction */
		aux = 0xC0 + x;;
		/* Send first nibble */
		lcd_put_command(lcd, aux >> 4);
		/* End second nibble */
		lcd_put_command(lcd, aux & 0x0F);
	}
}