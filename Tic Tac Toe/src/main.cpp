#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDR 0x3C

Adafruit_SSD1306 screen(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define TABLE_SIZE 3

char board[TABLE_SIZE][TABLE_SIZE] = {{' ', ' ', ' '},
                                      {' ', ' ', ' '},
                                      {' ', ' ', ' '}};

const int button_pin[9] = {2, 3, 4, 5, 6, 7, 8, 9, 10};

char player = 'X';

void drawBoard()
{

  screen.clearDisplay();

  screen.drawLine(40, 0, 40, 60, WHITE);
  screen.drawLine(80, 0, 80, 60, WHITE);
  screen.drawLine(0, 20, 120, 20, WHITE);
  screen.drawLine(0, 40, 120, 40, WHITE);

  screen.setTextSize(1);
  screen.setTextColor(WHITE);

  for (int i = 0; i < TABLE_SIZE; i++)
  {
    for (int j = 0; j < TABLE_SIZE; j++)
    {
      screen.setCursor(40 * j + 10, 20 * i + 10);
      screen.print(board[i][j]);
    }
  }

  screen.display();
}

char checkWinner()
{

  // Check for winning patterns in rows and columns

  for (int i = 0; i < TABLE_SIZE; i++)
  {
    if ((board[i][0] == board[i][1]) && (board[i][1] == board[i][2]))
      return board[i][0];
    if ((board[0][i] == board[1][i]) && (board[1][i] == board[2][i]))
      return board[0][i];
  }

  // Check for winner in the diagonalas

  if ((board[0][0] == board[1][1]) && (board[1][1] == board[2][2]))
    return board[0][0];
  if ((board[2][0] == board[1][1]) && (board[1][1] == board[0][2]))
    return board[2][0];

  // Check if there is a tie

  for (int row = 0; row < TABLE_SIZE; row++)
  {
    for (int col = 0; col < TABLE_SIZE; col++)
    {
      if (board[row][col] != 'X' && board[row][col] != 'O')
        return ' ';
    }
  }

  return 'T';
}

void isWinner(char winner)
{

  screen.clearDisplay();
  screen.setTextSize(2);
  screen.setCursor(10, 10);

  if (winner == 'T')
  {
    screen.print("Its a tie");
  }
  else
  {
    screen.print("Player ");
    screen.print(winner);
    screen.print(" wins!");
  }

  screen.display();

  delay(3000);
}

void waitForRelease()
{

  // Function to avoid ghosting

  bool allReleased;

  do
  {
    allReleased = true;
    for (int i = 0; i < 9; i++)
    {
      if (digitalRead(button_pin[i]) == LOW)
      {
        allReleased = false;
        break;
      }
    }
    delay(50);
  } while (!allReleased);
}

void resetGame()
{
  char initialBoard[TABLE_SIZE][TABLE_SIZE] = {{' ', ' ', ' '},
                                               {' ', ' ', ' '},
                                               {' ', ' ', ' '}};
  memcpy(board, initialBoard, sizeof(board));
  player = 'X';
  drawBoard();

  waitForRelease();
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);

  screen.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);

  for (int i = 0; i < 9; i++)
  {
    pinMode(button_pin[i], INPUT_PULLUP);
  }

  screen.clearDisplay();
  screen.display();

  // resetGame();
  drawBoard();
  waitForRelease();
}

void loop()
{
  // put your main code here, to run repeatedly:
  for (int i = 0; i < 9; i++)
  {

    if (digitalRead(button_pin[i]) == LOW)
    {

      int row = i / 3;
      int col = i % 3;

      if (board[row][col] != 'X' && board[row][col] != 'O')
      {
        board[row][col] = player;
        player = (player == 'X') ? 'O' : 'X';
        drawBoard();
      }

      char winner = checkWinner();

      if (winner != ' ')
      {
        isWinner(winner);
        resetGame();
      }
      delay(15);
    }
  }
}
