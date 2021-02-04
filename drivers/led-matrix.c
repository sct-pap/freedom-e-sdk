#include <stdio.h>
#include <stdint.h>
#include "platform.h"

#define BIT(x) (1 << x)

#define DIN PIN_2_OFFSET
#define CS PIN_3_OFFSET
#define CLK PIN_4_OFFSET

#define max7219_reg_noop        0x00
#define max7219_reg_digit0      0x01
#define max7219_reg_digit1      0x02
#define max7219_reg_digit2      0x03
#define max7219_reg_digit3      0x04
#define max7219_reg_digit4      0x05
#define max7219_reg_digit5      0x06
#define max7219_reg_digit6      0x07
#define max7219_reg_digit7      0x08
#define max7219_reg_decodeMode  0x09
#define max7219_reg_intensity   0x0a
#define max7219_reg_scanLimit   0x0b
#define max7219_reg_shutdown    0x0c
#define max7219_reg_displayTest 0x0f

#define HIGH 1
#define LOW 0

const int num_matrices = 4;

void set_pin(uint32_t pin, uint8_t val)
{
    if (val == 0) {
        GPIO_REG(GPIO_OUTPUT_VAL) &= ~BIT(pin);
    } else {
        GPIO_REG(GPIO_OUTPUT_VAL) |= BIT(pin);
    }
}

void send_byte(uint8_t b)
{
    for (int i = 7; i >= 0; i--) {
        set_pin(DIN, (b >> i) & 1);
        set_pin(CLK, HIGH);
        set_pin(CLK, LOW);
    }
}

void send_command(uint8_t cmd, uint8_t val)
{
    set_pin(CS, LOW);
    for(int i=0; i < num_matrices; i++) {
        send_byte(cmd);
        send_byte(val);
    }
    set_pin(CS, HIGH);
}

#define assign_bit(res, n, v) do {\
    res ^= (-v ^ res) & (1UL << n); \
} while (0)

void set_column(uint8_t col, uint8_t val)
{
    uint8_t n = col / 8;
    uint8_t c = col % 8;

    set_pin(CS, LOW);
    for (int i=0; i < num_matrices; i++) {
        if (i == n) {
            send_byte(c + 1);
            send_byte(val);
        } else {
            send_byte(0);
            send_byte(0);
        }
    }
    set_pin(CS, HIGH);
}

void set_column_all(uint8_t col, uint8_t val)
{
    set_pin(CS, LOW);
    for (int i=0; i<num_matrices; i++) {
        send_byte(col + 1);
        send_byte(val);
    }
    set_pin(CS, HIGH);
}

void clear(void)
{
    for (int i=0; i < 8; ++i) {
        set_column_all(i, 0);
    }
}

void set_intensity(uint8_t intensity)
{
    send_command(max7219_reg_intensity, intensity);
}

void setup_matrix(void)
{
    GPIO_REG(GPIO_OUTPUT_EN) |= BIT(PIN_2_OFFSET);
    GPIO_REG(GPIO_INPUT_EN) &= ~BIT(PIN_2_OFFSET);

    GPIO_REG(GPIO_OUTPUT_EN) |= BIT(PIN_3_OFFSET);
    GPIO_REG(GPIO_INPUT_EN) &= ~BIT(PIN_3_OFFSET);

    GPIO_REG(GPIO_OUTPUT_EN) |= BIT(PIN_4_OFFSET);
    GPIO_REG(GPIO_INPUT_EN) &= ~BIT(PIN_4_OFFSET);

    set_pin(CS, HIGH);

    send_command(max7219_reg_scanLimit, 0x07);
    send_command(max7219_reg_decodeMode, 0x00);
    send_command(max7219_reg_shutdown, 0x01);
    send_command(max7219_reg_displayTest, 0x00);

    set_intensity(0x0f);
}
