#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "oled.h"
#include "driver/gpio.h"

int ball_x = 64;  // Initial horizontal position of the ball
int ball_y = 32; 
int ball_dx = 1; // Initial horizontal velocity of the ball
int ball_dy = 1; // Initial vertical velocity of the ball
int ball_size = 2; 

int paddle_x = 54; // Initial horizontal position of the paddle
int paddle_y = 4;  // Initial vertical position of the paddle
int paddle_w = 20; // Width of the paddle
int paddle_h = 3;  // Height of the paddle

int score = 0;

#define Button_right GPIO_NUM_12
#define Button_left GPIO_NUM_13

void app_main(void)
{
    oled_init(SPI2_HOST);
    // Configure buttons as input with pull-up resistors
    gpio_set_direction(Button_right, GPIO_MODE_DEF_INPUT);
    gpio_set_direction(Button_left, GPIO_MODE_DEF_INPUT);
    gpio_set_pull_mode(Button_right, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(Button_left, GPIO_PULLUP_ONLY);

    while (1) {
        oled_clear(); // Clear the display

        ball_x += ball_dx; 
        ball_y += ball_dy; // Update ball position

        if (ball_x <= 0 || ball_x >= 128 - ball_size)
        {
            ball_dx = -ball_dx;
        }

        if (ball_y >= 64 - ball_size)
        {
            ball_dy = -ball_dy;
        }

        // Move the paddle based on button input
        if (gpio_get_level(Button_right) == 0 && paddle_x < 128 - paddle_w)
        {
            paddle_x += 2;
        }

        // Move the paddle based on button input
        if (gpio_get_level(Button_left) == 0 && paddle_x > 0)
        {
            paddle_x -= 2;
        }

        // Check for collision with the paddle
        if ((ball_y + ball_size) <= (paddle_y + paddle_h) &&
            ball_x + ball_size >= paddle_x &&
            ball_x <= paddle_x + paddle_w)
        {
            ball_dy = -ball_dy; // Reverse vertical direction
            score++; // Increment score
        }

        // Reset the game if the ball goes below the paddle
        if (ball_y <= 1)
        {
            ball_x = 64;
            ball_y = 20;
            ball_dx = 1;
            ball_dy = 1;
        }

        oled_draw_rect(ball_x, ball_y, ball_size, ball_size, true);
        oled_draw_rect(paddle_x, paddle_y, paddle_w, paddle_h, true);

        oled_update();
        

        vTaskDelay(pdMS_TO_TICKS(30));
    }
}