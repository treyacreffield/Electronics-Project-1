#include "mbed.h"

//Initialise LCD PINS
DigitalOut rs(PA_8); //command/data select
DigitalOut en(PB_14); //enable pulse
DigitalOut d4(PB_4);
DigitalOut d5(PB_5);
DigitalOut d6(PB_3);
DigitalOut d7(PA_10);

//Initialise BUTTONS
DigitalIn button1(PA_9, PullUp);
DigitalIn button2(PC_7, PullUp);
DigitalIn button3(PB_0, PullUp);
DigitalIn button4(PA_7, PullUp);

//Initialise LEDS
DigitalOut led1(PA_6);
DigitalOut led2(PA_5);
DigitalOut led3(PC_5);
DigitalOut led4(PC_4);

//GAME STATES
const int MENU = 0;
const int MEMORY_GAME = 1;
const int MATH_GAME = 2;
const int GAME_OVER = 3;

//LCD FUNCTIONS
//Sends enable pulse
void lcdPulse()
{
    en = 1;
    wait_us(1); //enable pulse must be > 450ns

    en = 0;
    wait_us(100);
}

//Sends 4 bits
void lcdWrite4(int value)
{
    d4 = value & 1; //get bit 0
    d5 = (value >> 1) & 1; //shift once, get bit 1
    d6 = (value >> 2) & 1; //shift twice, get bit 2
    d7 = (value >> 3) & 1; //shift three times, get bit 3

    lcdPulse();
}

//Sends command or data
void lcdSend(int value, int mode) //sends full 8 bit byte to LCD
{
    rs = mode; //0 is command, 1 is character data

    lcdWrite4(value >> 4);
    lcdWrite4(value);

    thread_sleep_for(2);
}

void lcdCommand(int command)
{
    lcdSend(command, 0); //for a command rs is set to 0
}

void lcdData(int data)
{
    lcdSend(data, 1); //for data rs is set to 1
}

void lcdClear()
{
    lcdCommand(0x01);
    thread_sleep_for(2);
}

void lcdLocate(int column, int row) //moves cursor to a specific location
{
    if(row == 0)
    {
        lcdCommand(0x80 + column);
    }
    else
    {
        lcdCommand(0xC0 + column);
    }
}

//Prints characters
void lcdPrint(const char* text) //pointer to string
{
    while(*text) //keeps looping until the null terminator (\0) - end of string
    {
        lcdData(*text); //sends current character to LCD
        text++; //moves pointer to the next character in memory
    }
}

//Prints single digit numbers
void lcdPrintNumber(int number)
{
    lcdData(number + '0');
}

//LCD setup - used Rohans guide to this
void lcdInit()
{
    thread_sleep_for(50);
    //sets control pins low
    rs = 0;
    en = 0;
    //send "Display Settings" 3 times (Only top nibble of 0x30 as we've got 4-bit bus)
    for (int i=0; i<3; i++) {
        lcdWrite4(0x3);
        thread_sleep_for(2); //this command takes 1.64ms, so thread_sleep_for for it
        }  
    //4-bit mode
    lcdWrite4(0x02); 

    lcdCommand(0x28); //Function set 001 BW N F - -
    lcdCommand(0x0C);
    lcdCommand(0x06); //Cursor Direction and Display Shift : 0000 01 CD S (CD 0-left, 1-right S(hift) 0-no, 1-yes

    lcdClear();
}

//HEART CHARACTER

int heart[8] =
{
    0b00000,
    0b01010,
    0b11111,
    0b11111,
    0b01110,
    0b00100,
    0b00000,
    0b00000
};

void lcdCreateCharacter()
{
    lcdCommand(0x40);

    for(int i = 0; i < 8; i++)
    {
        lcdData(heart[i]);
    }

    lcdClear();
}

//HELPER FUNCTIONS
//turns all LEDs off
void allLEDsOff()
{
    led1 = 0;
    led2 = 0;
    led3 = 0;
    led4 = 0;
}

//flashes one LED
void flashLED(int led)
{
    allLEDsOff();

    if(led == 0)
    {
        led1 = 1;
    }

    else if(led == 1)
    {
        led2 = 1;
    }

    else if(led == 2)
    {
        led3 = 1;
    }

    else if(led == 3)
    {
        led4 = 1;
    }

    thread_sleep_for(600);

    allLEDsOff();

    thread_sleep_for(300);
}

//Read when/which button is pressed
int readButton()
{
    while(true)
    {
        if(button1 == 0)
        {
            thread_sleep_for(200);
            return 0;
        }

        if(button2 == 0)
        {
            thread_sleep_for(200);
            return 1;
        }

        if(button3 == 0)
        {
            thread_sleep_for(200);
            return 2;
        }

        if(button4 == 0)
        {
            thread_sleep_for(200);
            return 3;
        }
    }
}

//Displays lives as hearts
void showLives(int lives)
{
    lcdLocate(0,1);

    lcdPrint("Lives:");

    for(int i = 0; i < lives; i++)
    {
        lcdLocate(7 + i,1);

        lcdData(0);
    }
}

//MEMORY GAME
int playMemoryGame(){
    
    int sequence[10];
    int lives = 3;
    int round = 1;

    while(lives > 0)
    {
        //Generate random sequence
        for(int i = 0; i < round; i++)
        {
            sequence[i] = rand() % 4;
        }

        //Show round number
        lcdClear();

        lcdLocate(0,0);
        lcdPrint("Watch LEDs");

        showLives(lives);

        thread_sleep_for(1000);

        //Flash sequence
        for(int i = 0; i < round; i++)
        {
            flashLED(sequence[i]);
        }

        //User input
        lcdClear();

        lcdLocate(0,0);
        lcdPrint("Repeat!");

        showLives(lives);

        bool correct = true;

        for(int i = 0; i < round; i++)
        {
            int userInput = readButton();

            flashLED(userInput);

            if(userInput != sequence[i])
            {
                correct = false;
            }
        }

        //Correct answer
        if(correct == true)
        {
            lcdClear();

            lcdLocate(0,0);
            lcdPrint("Correct!");

            thread_sleep_for(1000);

            round++; //if player answers correctly, next round becomes harder
        }

        //Wrong answer
        else
        {
            lives--; //if player scores incorrectly they will lose a 'life'

            lcdClear();

            lcdLocate(0,0);
            lcdPrint("Wrong!");

            showLives(lives);

            thread_sleep_for(1000);
        }
    }
    return round - 1;
}

//MAIN PROGRAM

int main(){
    //Initialise LCD Screen and Custom Characters
    lcdInit();
    lcdCreateCharacter();
    int state = MENU;
    int finalScore = 0; //stores score of last played game to be displayed on game over screen, both games return a 'final score'

    //Works as a simple state machine
    while(true)
    {
        if(state == MENU) //in MENU state user has options to select either memory or math game with button inputs
        {
            lcdClear();
            lcdLocate(0,0);
            lcdPrint("1:Memory 2:Math");

            lcdLocate(0,1);
            lcdPrint("Choose game");

            int choice = readButton(); 

            if(choice == 0) //if button 0 is pressed state will change to memory game
            {
                state = MEMORY_GAME; 
            }
            else if(choice == 1) //if button 1 is pressed state will change to math game
            {
                state = MATH_GAME;
            }
        }

        else if(state == MEMORY_GAME)
        {
            finalScore = playMemoryGame();
            state = GAME_OVER;
        }

        else if(state == MATH_GAME)
        {
            finalScore = playMathGame();
            state = GAME_OVER;
        }

        else if(state == GAME_OVER) //in state GAMEOVER: game over will be printed on screen followed by the final score
        {
            lcdClear();

            lcdLocate(0,0);
            lcdPrint("Game Over");

            lcdLocate(0,1);
            lcdPrint("Score:");
            lcdPrintNumber(finalScore);

            thread_sleep_for(3000);

            state = MENU; //game will then return to the menu screen
        }
    }
}
