Simple "Doodle Jump" like game for the Microbit V2 Development Board. 

 * Once the microbit is powered on, the device enters it's initial state, where
 * a string is displayed on the led matrix. 
 * 
 * Once either button on the microbit is pressed, the game transistions into the play state
 * 
 * In the play state, the character is a 1x1 pixel that is falling a constant speed. 
 * 
 * Platform of random sizes and x-positions are falling slower than the speed that the 
 * character falls. Tilt the microbit in order for the character to move left and right
 * as it is falling. The goal is to have the character land on a platform before the 
 * platform falls below the last row. Once the character has made contact with a platform, 
 * it jumps up a few rows. 
 * 
 * Keep tilting and making contact with the platforms to stay alive. Note that the character will 
 * not detect a collision on the way up with platforms (i.e. the character will jump only when
 * making contact with a platform on its way down, just like doodle jump the game). Every platform
 * that falls below the last row earns the player points. 
 * 
 * The game is over when the character falls below the last row, and the game transitions 
 * to a game over state, where a string is displayed for the score. 

main.c: main function of the game - initializes periperals, clocks, and game engine 

char.c/h: code that controls the movement of the "character," which takes up a single pixel of the 5x5 LED matrix. Movement includes the default downwards motion of the character, and the upwards motion after hitting a platform. Motion also includes the left and right motion of the character when tilting the board. 

font.c/h: Not very relevant 

led_matrix.c/h: code that controls what is actually being displayed on the LED matrix, depending on the state of the character and each platform. PWM concepts are used to make the character brighter than the platform. 

lsm303agr.c/h: use the accelerometer and gyro peripherals to calculate degrees of tilt in order to move the character

platform.c/h: control the falling array of platforms 

rng.c/h: driver for the rng peripheral in order to initialize the array of platforms with random lengths, and starting from random positions on the LED matrix
