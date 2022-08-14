Simple "Doodle Jump" like game for the Microbit V2 Development Board. 

Gameplay is on the 5x5 LED matrix on the Microbit V2 board. A single pixel is falling at a constant rate. Platforms are also falling at a constant rate, slightly slower than the character. Tilt the board to move the character in order to land on a falling platform, at which point the character will jump upwards. The game is over once the character falls past the bottom row of the 5x5 matrix. 

main.c: main function of the game - initializes periperals, clocks, and game engine 

char.c/h: code that controls the movement of the "character," which takes up a single pixel of the 5x5 LED matrix. Movement includes the default downwards motion of the character, and the upwards motion after hitting a platform. Motion also includes the left and right motion of the character when tilting the board. 

font.c/h: Not very relevant 

led_matrix.c/h: code that controls what is actually being displayed on the LED matrix, depending on the state of the character and each platform. PWM concepts are used to make the character brighter than the platform. 

lsm303agr.c/h: use the accelerometer and gyro peripherals to calculate degrees of tilt in order to move the character

platform.c/h: control the falling array of platforms 

rng.c/h: driver for the rng peripheral in order to initialize the array of platforms with random lengths, and starting from random positions on the LED matrix
