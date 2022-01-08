#include <M5StickC.h>
#include <stdlib.h>
#include <stdint.h>
#include "EEPROM.h"


/*

 .----------------.  .----------------.  .----------------.  .----------------.  .----------------.  .----------------. 
| .--------------. || .--------------. || .--------------. || .--------------. || .--------------. || .--------------. |
| |  _________   | || |  _________   | || |  _________   | || |  _______     | || |     _____    | || |    _______   | |
| | |  _   _  |  | || | |_   ___  |  | || | |  _   _  |  | || | |_   __ \    | || |    |_   _|   | || |   /  ___  |  | |
| | |_/ | | \_|  | || |   | |_  \_|  | || | |_/ | | \_|  | || |   | |__) |   | || |      | |     | || |  |  (__ \_|  | |
| |     | |      | || |   |  _|  _   | || |     | |      | || |   |  __ /    | || |      | |     | || |   '.___`-.   | |
| |    _| |_     | || |  _| |___/ |  | || |    _| |_     | || |  _| |  \ \_  | || |     _| |_    | || |  |`\____) |  | |
| |   |_____|    | || | |_________|  | || |   |_____|    | || | |____| |___| | || |    |_____|   | || |  |_______.'  | |
| |              | || |              | || |              | || |              | || |              | || |              | |
| '--------------' || '--------------' || '--------------' || '--------------' || '--------------' || '--------------' |
 '----------------'  '----------------'  '----------------'  '----------------'  '----------------'  '----------------' 


                                                  Abdullah Sabaa Allil
                                               abdullah.sabaa.allil@vub.be
                                                        2021-2022

*/


// een struct om de positie van 1 vierkant bij te houden
typedef struct position
{
  uint8_t x;
  uint8_t y;
} Position;

// een struct om een blok voor te stellen
typedef struct block
{
  Position **squares;
  uint8_t type;
  uint8_t number_of_squares;
} Block;

void save_block(Block *block);
Position *load_square(uint8_t first_byte, uint8_t second_byte, uint8_t idx);
Block *load_block();
void save_game_values();
void load_game_values();
void save_current_falling_block();
void load_current_Falling_block();
void save_game();
void load_game();

uint8_t timed_block_or_not(uint8_t freq);
uint8_t pick_random_block(uint8_t with_mine);

Position *make_square(uint8_t x, uint8_t y);
Block *make_block(uint8_t type, uint8_t with_timed);
void free_block(block *block);
void free_current_falling_block();
void free_all();
uint8_t add_block_to_array(Block *block);
uint8_t add_block();
void remove_block(uint8_t key);
void initialize_game_field();

void draw_squares(Position **squares, uint8_t amount_of_squares, uint32_t color);
void draw_block(Block *block);
void clear_previous_block_position(Block *block);
void redraw_field();

uint8_t is_square_occupied(uint8_t content);
uint8_t squares_landed(Position **squares, uint8_t amount_of_squares);
uint8_t block_landed_on_block(Block *block);
uint8_t check_squares_left(Position **squares, uint8_t nosq);
uint8_t check_squares_right(Position **squares, uint8_t nosq);
uint8_t check_squares_down_border(Position **squares, uint8_t nosq);

void draw_squares_in_matrix(Position **squares, uint8_t nosq, uint8_t mat_content);
void move_squares(Position **squares, uint8_t nosq, uint8_t direction);
void move_block_right(Block *block);
void move_block_left(Block *block);
void move_block_down(Block *block);
void move_block_horitonztally(Block *block);
void move_falling_block();

uint8_t safe_to_swap(Position **squares, uint8_t nosq, Position *center);
void swap_x_y(Position *pos, Position *center);
void rotate_block(Block *block);

void find_and_update(uint8_t row, uint8_t col);
void move_after_row_clearing(uint8_t row);
void remove_square(uint8_t row, uint8_t col);

void score_up();

void empty_row(uint8_t which_row);
uint8_t is_row_full(uint8_t which_row);
void check_for_full_row(uint8_t row);
void check_for_full_landed_block_rows();
void check_for_full_rows();

uint8_t is_game_over();
void go_to_gameover();
void game_over();

void move_square_down_until_landing(uint8_t row, uint8_t col);
void move_down_after_kaboom(uint8_t row, uint8_t col);
void kaboom();

void update_timer(uint8_t with_timed);
uint8_t timed_block_still_there();

void game();

void start_menu();
uint8_t get_index();

#define DISPLAY_HEIGHT 160
#define DISPLAY_WIDTH 80
#define SQUARE_SIDE_LENGTH 8

#define GAME_FIELD_ROWS DISPLAY_HEIGHT / SQUARE_SIDE_LENGTH // 20
#define GAME_FIELD_COLS DISPLAY_WIDTH / SQUARE_SIDE_LENGTH  // 10
#define MOVING_UNIT 1
#define INIT_X GAME_FIELD_COLS / 2
#define INIT_Y 0

#define TIMED_BLOCK_FREQ 4

#define INIT_DELAY 150
#define DELAY_DECLINING_UNIT 15
#define CENTER_SQUARE 2
#define TIMER_ON 10       // het aantal blokken dat gaat vallen voor dat de speler verliest als het timed blok tegen dan actief is - 1 (het timed blok zelf)
#define TIMER_OFF 0

#define INDICATOR_X 70
#define INIT_MENU_Y 80
#define INC_Y 16
#define MENU_OPTIONS 4

#define NUMBER_OF_SHAPES 8
#define DEFAULT_INIT_NOSQ 4
#define MINES_NOSQ 1

#define SIZE 50 // de grootte van de array waarin blokken opgeslagen kunnen worden.
// er wordt gekozen voor 50 omdat het mogelijk is dat de array allemaal blokken kan bevatten
// van grootte 1x1, in dat geval zouden er 10*20 blokken aanwezig zijn
// dit is een worst case scenario. Er worden optimalisaties gemaakt die zorgen dat dit nooit voorkomt.

// Het spelveld is een veld van integers
// Deze integers zijn "keys", dit zijn de indices naar de parent blokken.
// Dit zijn de blokken diens blokjes in die positie in het spelveld aanwezig zijn.
#define MOVING SIZE + 1
#define EMPTY SIZE + 2
#define INVALID SIZE + 3
#define MEM_SIZE 512



enum block_types
{
  yellow,
  blue,
  red,
  green,
  orange,
  rose,
  purple,
  mine,
  timed
};

enum direction
{
  down,
  left,
  right
};

enum program_state
{
  MENU,
  GAME,
  GAME_ENDED
};

enum game_options
{
  mines_timed,
  no_mines_no_timed,
  no_mines_timed,
  mines_no_timed

};

enum errnos
{
  allo_block,
  allo_pos,
  allo_pos_arr,
  allo_block_arr,
  se_sq
};


// De nodige kleuren voor het spel
uint32_t black_color = M5.Lcd.color565(0, 0, 0);
uint32_t yellow_color = M5.Lcd.color565(255, 255, 0);
uint32_t blue_color = M5.Lcd.color565(0, 0, 255);
uint32_t red_color = M5.Lcd.color565(255, 0, 0);
uint32_t green_color = M5.Lcd.color565(0, 255, 0);
uint32_t orange_color = M5.Lcd.color565(255, 180, 0);
uint32_t rose_color = M5.Lcd.color565(100, 0, 50);
uint32_t purple_color = M5.Lcd.color565(50, 25, 50);
uint32_t mine_color = M5.Lcd.color565(165, 42, 42);    // bruin
uint32_t timed_color = M5.Lcd.color565(255, 255, 255); // wit


// de verschillende blokvormen
Position yellow_p[DEFAULT_INIT_NOSQ] = {
    {INIT_X, INIT_Y},
    {INIT_X + MOVING_UNIT, INIT_Y},
    {INIT_X, INIT_Y + MOVING_UNIT},
    {INIT_X + MOVING_UNIT, INIT_Y + MOVING_UNIT}};

Position red_p[DEFAULT_INIT_NOSQ] = {
    {INIT_X, INIT_Y + MOVING_UNIT},
    {INIT_X + MOVING_UNIT, INIT_Y},
    {INIT_X - MOVING_UNIT, INIT_Y + MOVING_UNIT},
    {INIT_X, INIT_Y}};

Position blue_p[DEFAULT_INIT_NOSQ] = {
    {INIT_X, INIT_Y},
    {INIT_X, INIT_Y + MOVING_UNIT},
    {INIT_X, INIT_Y + 2 * MOVING_UNIT},
    {INIT_X, INIT_Y + 3 * MOVING_UNIT}};

Position green_p[DEFAULT_INIT_NOSQ] = {
    {INIT_X, INIT_Y},
    {INIT_X - MOVING_UNIT, INIT_Y},
    {INIT_X + MOVING_UNIT, INIT_Y + MOVING_UNIT},
    {INIT_X, INIT_Y + MOVING_UNIT}};

Position orange_p[DEFAULT_INIT_NOSQ] = {
    {INIT_X, INIT_Y},
    {INIT_X, INIT_Y + MOVING_UNIT},
    {INIT_X, INIT_Y + 2 * MOVING_UNIT},
    {INIT_X + MOVING_UNIT, INIT_Y + 2 * MOVING_UNIT}};

Position rose_p[DEFAULT_INIT_NOSQ] = {
    {INIT_X, INIT_Y},
    {INIT_X, INIT_Y + MOVING_UNIT},
    {INIT_X, INIT_Y + 2 * MOVING_UNIT},
    {INIT_X - MOVING_UNIT, INIT_Y + 2 * MOVING_UNIT}};

Position purple_p[DEFAULT_INIT_NOSQ] = {
    {INIT_X, INIT_Y},
    {INIT_X + MOVING_UNIT, INIT_Y},
    {INIT_X, INIT_Y + 2 * MOVING_UNIT},
    {INIT_X, INIT_Y + MOVING_UNIT}};

Position mine_p[MINES_NOSQ] = {{INIT_X, INIT_Y}};

// een simpele dictionary om de spelblokken en de overeenkomstige kleuren bij te houden
Position *blocks[NUMBER_OF_SHAPES] = {yellow_p, blue_p, red_p, green_p, orange_p, rose_p, purple_p, mine_p};
uint32_t colors[NUMBER_OF_SHAPES + 1] = {yellow_color, blue_color, red_color, green_color, orange_color, rose_color, purple_color, mine_color, timed_color};

// het spelveld
uint8_t game_field[GAME_FIELD_ROWS][GAME_FIELD_COLS];
// het blok dat momenteel naar beneden aan het vallen is ("de speler")
Block *current_falling_block = NULL;

// de array waarin de blokken opgeslagen worden
Block *storage[SIZE];
// een variabele die aangeeft waar er in de opslagarray momenteel een vrije plaats is.
uint8_t firstfree = 0;
uint8_t blocks_ctr = 0;

// de spelvariabelen
uint8_t score = 0;
uint8_t loop_delay = 0;
uint8_t default_delay = INIT_DELAY;

uint8_t timer = TIMER_OFF;
uint8_t timed_block_key = INVALID;

// wat is momenteel actief is in het programma (startmenu, spel, gameover)
uint8_t currently_active = MENU;

// de positie van de indicator/cursor in het startmenu
uint8_t y = INIT_MENU_Y;

uint8_t with_mines = 1;
uint8_t with_timed = 1;



// een procedure om de mogelijke fouten te melden en het programma te beeindigen wanneer een fout gebeurt.
void error_handler(int errno)
{
  M5.Lcd.fillScreen(blue_color);
  M5.Lcd.printf("ERROR\nOCCURED");
  switch (errno)
  {
  case allo_block:
    fprintf(stderr, "NULL pointer returned while allocating memory for a Block pointer\n;");
    break;
  case allo_pos:
    fprintf(stderr, "NULL pointer returned while allocating memory for a Position pointer\n;");
    break;
  case allo_pos_arr:
    fprintf(stderr, "NULL pointer returned while allocating memory for a Position pointers array\n;");
    break;
  case allo_block_arr:
    fprintf(stderr, "No memory is available for storing new blocks\n;");
    break;
  case se_sq:
    fprintf(stderr, "An eeror occured while trying to remove a square\n");
    break;
  }
  delay(5000);
  exit(1);
}



/*

het basisidee:
- je zoekt het kleinste datatype waarin je gegevens inpassen (keuze tss int64, 32, 16, 8)
- je steekt de waarden allemaal erin (met een or)
- je leest 8 bits pet 8 bits met een and (en een shift)
- je  slaat elke 8 bits op naar de memory

Blokken opslaan:

hoe de flash memory verder minimaliseren:
    #blokken is maximaal 4                    (nosq)
         1 0 0                  (3 bits)
    type is maximaal 8
       1 0 0 0                  (4 bits)
      
    Per blokje:
    x (col) is max 9
        1 0 0 1                 (4 bits)
    y (rij) is max 19
      1 0 0 1 1                 (5 bits)
    
- #blokken en type worden in eerste 7 bits opgeslagen
- ik decode beide waarden
- afhankelijk van wat #blokken zijn ga ik verder decoden


voor een blok met 4 blokjes: 3 + 4 + (5 + 4)*4 = 43 bits (48 bits, 6 bytes)
    1 0 0 0 1 0 0 1      0 0 1 1 0 0 1 1    1 0 0 1 1 0 0 1      1 1 0 0 1 1 0 0        1 1 1 0 0 1 1 0     0 1 1 0 0 0 0 0

voor een blok met 3 blokjes: 3 + 4 + (5 + 4)*3 = 34 bits (40 bits, 5 bytes)
    0 0 0 0 0 0 1 0     0 0 1 0 0 1 0 0    1 1 0 0 1 1 1 0    0 1 1 0 0 1 1 1    0 0 1 1 0 0 1 1

voor een blok met 2 blokjes: 3 + 4 + (5 + 4)*2 = 25 bits (32 bits, 4 bytes)
    0 0 0 0 0 0 0 1   0 0 0 1 0 0 1 0   0 1 1 0 0 1 1 1   0 0 1 1 0 0 1 1

voor een blok met 1 blokje : 3 + 4 + (5 + 4)*1 = 16 bits (16 bits, 2 bytes)
    1 0 0 0 1 0 0 1     0 0 1 1 0 0 1 1

ze worden eerst in een 64 bit getal opgeslagen, en dan wordt dat getal gesliced naar 8 bit getallen die weggeschreven worden naar de memory

Per spelblok: 6 bytes nodig
flash memory nodig om een spel van 50 blokken die uit 4 blokjes bestaan op te slaan(~ worst case):
50*(6 bytes) + 4 bytes = 304 bytes

*/

// het adres in de flash memory
int16_t address = 0;

void save_block(Block *block)
{

  uint8_t nosq = block->number_of_squares;
  uint64_t tmp = ((uint64_t)block->type) << 44; // 44 want we beginnen altijd waarden op te slaan vanaf de 48e bit in het 64bit getal
  uint64_t tmp_2 = ((uint64_t)nosq) << 41;      // 41 omdat het bloktype 4 bits nodig heeft, en het aantal blokjes 3 bits
  uint64_t res = tmp | tmp_2;                   // alles opslaan in de res variabele
  uint8_t to_mem = 0;                           // de waarde die weggeschreven gaat worden naar de memory
  // de eerste x bytes van het 64 bit zullen opgeslagen worden naar het geheugen
  // x is: 6 bytes voor een blok met 4 blokjes, 5 bytes voor een blok met 3 blokjes, 4 bytes voor een blok met 2 blokjes, 2 bytes voor een blok met 1 blokje
  for (uint8_t i = 0; i < block->number_of_squares; i++)
  {
    // x is tussen 0 en 9, dus 4 bits nodig, y is tussen 0 en 19, dus 5 bits voor nodig.
    // hier wordt er berekend hoeveel gebitshifted moet worden
    tmp = ((uint64_t)block->squares[i]->x) << (41 - (4 * (i + 1)) - (5 * i));
    tmp_2 = ((uint64_t)block->squares[i]->y) << (41 - (4 * (i + 1)) - (5 * (i + 1)));
    res = tmp | tmp_2 | res;
  }

  // "where to stop geeft aan vanaf welke byte we moeten stoppen met wegschrijven naar memory
  // voor 4 blokjes: 6 bytes , dus where_to_stop is 0 (vanaf 5e byte tot en met 0e byte wegschrijven)
  // voor 3 blokjes: 5 bytes; dus where_to_stop is 1 (5e byte tem 1e byte wegschrijven)
  // enzovoort
  uint8_t where_to_stop = 4 >> (nosq - 1);
  for (int8_t i = 5; i >= where_to_stop; i--)
  {
    to_mem = (res >> (8 * i)) & 0xff;
    EEPROM.writeByte(address, to_mem);
    address++;
  }
}



// de masks die nodig zijn om de coordinaten van een positie te decoderen.
uint8_t square_masks[4][2] = {{0b11100000, 0b00011111},
                              {0b11110000, 0b00001111},
                              {0b01111000, 0b00000111},
                              {0b00111100, 0b00000011}};

// een positie van een blokje inladen, gegeven de twee bytes waarin de coordinaten opgeslagen zijn, en gegeven de idx van het blokje.
Position *load_square(uint8_t first_byte, uint8_t second_byte, uint8_t idx)
{
  uint8_t x_pos, y_pos;
  if (idx == 0)
  {

    x_pos = (((second_byte & square_masks[idx][0]) >> 1) | first_byte) >> 4;
    y_pos = second_byte & square_masks[idx][1];
  }
  else
  {
    x_pos = (first_byte & square_masks[idx][0]) >> (8 - (3 + idx));
    y_pos = ((first_byte & square_masks[idx][1]) << idx) | (second_byte >> (8 - idx));
  }
  return make_square(x_pos, y_pos);
}

Block *load_block()
{
  uint8_t n = EEPROM.readByte(address);
  address++;
  uint8_t mask_nosq = 0b00001110;
  uint8_t mask_type = 0b11110000;
  uint8_t shifted_nosq = mask_nosq & n;
  uint8_t shifted_type = mask_type & n;

  uint8_t nosq = shifted_nosq >> 1;
  uint8_t type = shifted_type >> 4;
  Block *block = (Block *)malloc(sizeof(Block));
  if (block == NULL)
    error_handler(allo_block);
  Position **squares = (Position **)calloc(nosq, sizeof(Position *));
  if (squares == NULL)
    error_handler(allo_pos_arr);
  block->number_of_squares = nosq;
  block->type = type;
  block->squares = squares;

  n <<= 7; // de laatste bit van de eerst ingelezen byte is de eerste bit van de x coord van het eerste blokje
  uint8_t o = EEPROM.readByte(address);
  address++;

  block->squares[0] = load_square(n, o, 0);

  if (nosq > 1)
  {
    n = EEPROM.readByte(address);
    address++;

    o = EEPROM.readByte(address);
    address++;
    for (uint8_t i = 1; i < nosq; i++)
    {
      block->squares[i] = load_square(n, o, i);
      n = o;
      if (i != (nosq - 1))
      {
        o = EEPROM.readByte(address);
        address++;
      }
    }
  }

  return block;
}

void save_game_values()
{
  printf("mines: %d timer: %d ttimer: %d\n", with_mines, with_timed, timer);
  printf("score: %d, delay: %d, ctr: %d\n", score, default_delay, blocks_ctr);


  uint8_t shifted_with_mines = with_mines << 7;
  uint8_t shifted_with_timed = with_timed << 6;
  // de variabelen die aangeven welk spel nu actief is en de timer van de timed block
  // worden allemaal in 1 byte bewaard
  uint8_t res = shifted_with_mines | shifted_with_timed | timer;
  EEPROM.writeByte(address, res);
  address++;
  EEPROM.writeByte(address, score);
  address++;
  EEPROM.writeByte(address, default_delay);
  address++;
  EEPROM.writeByte(address, blocks_ctr);
  address++;
}

void load_game_values()
{
  uint8_t n = EEPROM.readByte(address);
  address++;
  uint8_t mask_with_mines = 0b10000000;
  uint8_t mask_with_timed = 0b01000000;
  uint8_t mask_timer = 0b00111111;

  uint8_t shifted_with_mines = mask_with_mines & n;
  uint8_t shifted_with_timed = mask_with_timed & n;
  uint8_t shifted_timer = mask_timer & n;

  with_mines = shifted_with_mines >> 7;
  with_timed = shifted_with_timed >> 6;
  timer = shifted_timer;

  score = EEPROM.readByte(address);
  address++;
  default_delay = EEPROM.readByte(address);
  address++;
  blocks_ctr = EEPROM.readByte(address);
  address++;


  printf("mines: %d timer: %d ttimer: %d\n", with_mines, with_timed, timer);
  printf("score: %d, delay: %d, ctr: %d\n", score, default_delay, blocks_ctr);
}

void save_current_falling_block()
{
  if (current_falling_block != NULL)
  {
    save_block(current_falling_block);
  }
  else
  {
    // indien er geen vallend blok aanwezig was op het moment van het opslaan
    // (wat heel weinig voorkomt maar you never say never)
    // dan wordt 0 opgeslagen om aan te geven dat er geen vallend blok actief is.
    EEPROM.writeByte(address, 0);
    address++;
  }
}

void load_current_Falling_block()
{
  uint8_t n = EEPROM.readByte(address);
  if (n != 0)
  {
    current_falling_block = load_block();
  }
  else
  {
    address++;
  }
}

// de staat van het spel bewaren in de flash memory
void save_game()
{
  address = 0;
  save_game_values();
  save_current_falling_block();
  for (uint8_t i = 0; i < SIZE; i++)
  {
    if (storage[i] != NULL)
    {
      save_block(storage[i]);
    }
  }
  EEPROM.commit();
}

// de staat van het spel inlezen uit de memory
void load_game()
{
  address = 0;
  free_all();
  free_current_falling_block();
  load_game_values();
  load_current_Falling_block();
  initialize_game_field();
  m5.lcd.fillScreen(black_color);

  for (uint8_t i = 0; i < blocks_ctr; i++)
  {
    Block *block = load_block();
    uint8_t idx = add_block_to_array(block);
    if(block->type == timed) {
      timed_block_key = idx;
    }
    draw_squares_in_matrix(block->squares, block->number_of_squares, idx);
    draw_block(block);
  }

  if (current_falling_block)
  {
    for (uint8_t i = 0; i < current_falling_block->number_of_squares; i++)
    {
      uint8_t row = current_falling_block->squares[i]->y;
      uint8_t col = current_falling_block->squares[i]->x;

      game_field[row][col] = MOVING;
    }
    draw_block(current_falling_block);
  }
}



// een predikaat die aangeeft of het net gemaakte blok een timed blok moet worden of niet
uint8_t timed_block_or_not(uint8_t freq)
{
  return random(0, freq) == 0;
}

// een predikaat die aangeeft of er mijnen ook gemaakt mogen worden
uint8_t pick_random_block(uint8_t with_mine)
{
  uint8_t top = purple;
  if (with_mine)
  {
    top = mine;
  }
  return random(yellow, top + 1);
}


// een procedure om een positie van een blokje aan te maken.
Position *make_square(uint8_t x, uint8_t y)
{
  Position *square = (Position *)malloc(sizeof(Position));
  if (square == NULL)
    error_handler(allo_pos);
  square->x = x;
  square->y = y;
  return square;
}


// een nieuw blok aanmaken en initialiseren volgens het meegegeven type
Block *make_block(uint8_t type, uint8_t with_timed)
{
  Block *block = (Block *)malloc(sizeof(Block));
  if (block == NULL)
    error_handler(allo_block);
  block->type = type;
  // als het een mijn is dan moet het maar 1 blokje bevatten
  block->number_of_squares = (type == mine) ? MINES_NOSQ : DEFAULT_INIT_NOSQ;
  // voor de algemeenheid wordt er ook een array gemaakt voor mijnen (1x1 blokken)
  block->squares = (Position **)calloc(block->number_of_squares, sizeof(Position *));
  if (block->squares == NULL)
    error_handler(allo_pos_arr);

  for (uint8_t i = 0; i < block->number_of_squares; i++)
  {
    block->squares[i] = make_square((blocks[type])[i].x, (blocks[type])[i].y);
    game_field[(block->squares[i])->y][(block->squares[i])->x] = MOVING;
    loop_delay = default_delay;
  }

  if (with_timed)
  {
    if (timed_block_or_not(TIMED_BLOCK_FREQ) &&
        with_timed &&
        timer == TIMER_OFF && // er mag geen aanwezige timed blok zijn
        block->type != mine)  // mijnen kunnen geen timed blokken zijn (omdat het toch niet nuttig is)
    {
      block->type = timed;
      timer = TIMER_ON;
    }
  }
  return block;
}


// een block (incl zijn inhoud) freeen uit het geheugen
void free_block(block *block)
{
  for (uint8_t j = 0; j < block->number_of_squares; j++)
  {
    free(block->squares[j]);
  }
  free(block->squares);
  free(block);
}

void free_current_falling_block()
{
  if (current_falling_block)
  {
    free_block(current_falling_block);
    current_falling_block = NULL;
  }
}

// alle blokken (incl hun inhoud) uit de memory freeen
void free_all()
{
  for (uint8_t i = 0; i < SIZE; i++)
  {
    if (storage[i] != NULL)
    {
      remove_block(i);
    }
  }

  firstfree = 0;
}

uint8_t search_for_free_memory()
{
  for (uint8_t i = 0; i < SIZE; i++)
  {
    if (storage[i] == NULL)
      return i;
  }
  return INVALID;
}
// een blok toevoegen naar de opslagruimte
uint8_t add_block_to_array(Block *block)
{

  uint8_t block_idx = firstfree++;
  if (firstfree >= SIZE || storage[firstfree] != NULL)
    firstfree = INVALID;

  if (block_idx != INVALID)
    storage[block_idx] = block;
  else
  {
    block_idx = search_for_free_memory();
    if (block_idx == INVALID)
    {
      error_handler(allo_block_arr);
    }
  }

  return block_idx;
}

// het blok dat naar beneden valt en dat nu beland heeft aan de array toevoegen
uint8_t add_block()
{
  blocks_ctr++; // een counter voor het aantal aanwezige blokken in het spel
  return add_block_to_array(current_falling_block);
}

// een blok verwijderen uit de opslagruimte
void remove_block(uint8_t key)
{
  free(storage[key]->squares);
  free(storage[key]);
  storage[key] = NULL;
  if (firstfree == INVALID) // voor het geval dat de array vol was
    firstfree = key;        // zetten we firstfree op de net vrijgekomen kotje
  blocks_ctr--;
}





void draw_squares(Position **squares, uint8_t amount_of_squares, uint32_t color)
{
  for (uint8_t i = 0; i < amount_of_squares; i++)
  {
    M5.Lcd.fillRect(squares[i]->x * SQUARE_SIDE_LENGTH, squares[i]->y * SQUARE_SIDE_LENGTH, SQUARE_SIDE_LENGTH, SQUARE_SIDE_LENGTH, color);
  }
}

// een blok tekenen op het scherm
void draw_block(Block *block)
{

  // de kleur van het blok halen uit de dictionary
  uint32_t color = colors[block->type];
  draw_squares(block->squares, block->number_of_squares, color);
}

void clear_previous_block_position(Block *block)
{
  draw_squares(block->squares, block->number_of_squares, black_color);
}

// een perdikaat die aangeeft of een vakje in het spelveld een blokje bevat
// dat geen deel is van het bewegende blok (de speler)
uint8_t is_square_occupied(uint8_t content)
{
  return content != EMPTY && content != MOVING;
}

// een predikaat die aangeeft of een arrays van blokjes beland is boven een blokje
// of de bodem
uint8_t squares_landed(Position **squares, uint8_t amount_of_squares)
{
  for (uint8_t i = 0; i < amount_of_squares; i++)
  {
    // de square onder de huidige square
    uint8_t bottom = game_field[squares[i]->y + 1][squares[i]->x];
    if (is_square_occupied(bottom))
      return 1;
  }
  return 0;
}

// een predikaat die aangeeft of een blok beland heeft
uint8_t block_landed_on_block(Block *block)
{
  return squares_landed(block->squares, block->number_of_squares);
}

// alle vakjes van het spelveld op EMPTY zetten
void initialize_game_field()
{
  for (uint8_t i = 0; i < GAME_FIELD_ROWS; i++)
  {
    for (uint8_t j = 0; j < GAME_FIELD_COLS; j++)
    {
      game_field[i][j] = EMPTY;
    }
  }
}

// de vakjes die overeenkomen met de blokjes invullen met de gevraagde inhoud
void draw_squares_in_matrix(Position **squares, uint8_t nosq, uint8_t mat_content)
{
  for (uint8_t i = 0; i < nosq; i++)
  {
    game_field[squares[i]->y][squares[i]->x] = mat_content;
  }
}

// checken of er obstakels zijn links van het blok
uint8_t check_squares_left(Position **squares, uint8_t nosq)
{
  for (uint8_t i = 0; i < nosq; i++)
  {
    if (squares[i]->x == 0 ||
        is_square_occupied(game_field[squares[i]->y][squares[i]->x - MOVING_UNIT]))
      return 1;
  }
  return 0;
}

// checken of er obstakels zijn links van het blok
uint8_t check_squares_right(Position **squares, uint8_t nosq)
{
  for (uint8_t i = 0; i < nosq; i++)
  {
    if (squares[i]->x >= GAME_FIELD_COLS - 1 ||
        is_square_occupied(game_field[squares[i]->y][squares[i]->x + MOVING_UNIT]))
      return 1;
  }
  return 0;
}

uint8_t check_squares_down_border(Position **squares, uint8_t nosq)
{
  for (uint8_t i = 0; i < nosq; i++)
  {
    if (squares[i]->y >= GAME_FIELD_ROWS)
      return 1;
  }
  return 0;
}

uint8_t landed_on_bottom(Block *block)
{
  return check_squares_down_border(block->squares, block->number_of_squares);
}

// een array van blokjes laten bewegen
void move_squares(Position **squares, uint8_t nosq, uint8_t direction)
{
  // de vorige posities clearen in het spelveld
  draw_squares_in_matrix(squares, nosq, EMPTY);
  switch (direction)
  {
  case down:
    if (!block_landed_on_block(current_falling_block))
      for (uint8_t i = 0; i < nosq; i++)
      {
        squares[i]->y += MOVING_UNIT;
      }
    break;

  case left:
    if (!check_squares_left(squares, nosq))
      for (uint8_t i = 0; i < nosq; i++)
      {
        squares[i]->x -= MOVING_UNIT;
      }
    break;

  case right:
    if (!check_squares_right(squares, nosq))
      for (uint8_t i = 0; i < nosq; i++)
      {
        squares[i]->x += MOVING_UNIT;
      }
    break;
  }

  draw_squares_in_matrix(squares, nosq, MOVING);
}

void move_block_right(Block *block)
{
  move_squares(block->squares, block->number_of_squares, right);
}

void move_block_left(Block *block)
{
  move_squares(block->squares, block->number_of_squares, left);
}

void move_block_down(Block *block)
{
  move_squares(block->squares, block->number_of_squares, down);
}

#define MIN_TILT_X 0.15
#define MIN_TILT_Y_FORWARDS 0.7
#define MIN_TILT_Y_BACKWARDS 0.2
void move_block_horitonztally(Block *block)
{
  float acc_x = 0, acc_y = 0, acc_z = 0;
  M5.IMU.getAccelData(&acc_x, &acc_y, &acc_z);
  if (acc_x > MIN_TILT_X)
  {
    move_block_left(block);
  }
  else if (acc_x < -MIN_TILT_X)
  {
    move_block_right(block);
  }

  if (acc_y > MIN_TILT_Y_FORWARDS)
  {
    loop_delay = default_delay / 3; // sneller
  }
  else if (acc_y < -MIN_TILT_Y_BACKWARDS)
  {
    loop_delay = default_delay; // trager
  }
}

// checken of het mogelijk is om een positie rond een andere positie (center) te roteren
uint8_t safe_to_swap(Position **squares, uint8_t nosq, Position *center)
{
  for (uint8_t i = 0; i < nosq; i++)
  {
    char possible_x = -(squares[i]->y - center->y) + center->x;
    char possible_y = (squares[i]->x - center->x) + center->y;

    if (!(possible_x >= 0 && possible_x <= GAME_FIELD_COLS - 1 &&
          possible_y >= 0 && possible_y < GAME_FIELD_ROWS))
      return 0;

    if (is_square_occupied(game_field[(uint8_t)possible_y][(uint8_t)possible_x]))
      return 0;
  }
  return 1;
}

// de positie van een blokje roteren rond center
// dit is een standaard lineaire algebra rotatie
// 90 graden tegenwijzezin
void swap_x_y(Position *pos, Position *center)
{
  int8_t new_x = -(pos->y - center->y) + center->x;
  int8_t new_y = (pos->x - center->x) + center->y;
  game_field[pos->y][pos->x] = EMPTY;
  pos->x = new_x;
  pos->y = new_y;
  game_field[pos->y][pos->x] = MOVING;
}

// het blok dat naar beneden valt roteren
void rotate_block(Block *block)
{

  if (!(block->type == mine ||
        block->type == yellow)) // geel is een vierkant
  {

    uint8_t nosq = block->number_of_squares;
    Position *center = block->squares[CENTER_SQUARE];
    if (safe_to_swap(block->squares, nosq, center))
      for (uint8_t i = 0; i < nosq; i++)
      {
        swap_x_y(block->squares[i], center);
      }
  }
}

// heel het spelveld hertekenen (wanneer er bvb een bom ontploft is, of wanneer een rij verwijderd is)
void redraw_field()
{

  M5.Lcd.fillScreen(black_color);
  for (uint8_t i = 0; i < GAME_FIELD_ROWS; i++)
  {
    for (uint8_t j = 0; j < GAME_FIELD_COLS; j++)
    {
      uint8_t key = game_field[i][j];

      if (key == EMPTY)
        continue;
      draw_block(storage[key]);
    }
  }
}

// vind een blokje in een blok en verhoog de y positie
void find_and_update(uint8_t row, uint8_t col)
{
  uint8_t key = game_field[row][col];
  Block *block = storage[key];
  for (uint8_t i = 0; i < block->number_of_squares; i++)
  {
    if (block->squares[i]->y == row && block->squares[i]->x == col)
    {
      (block->squares[i]->y)++;
      break;
    }
  }
}

// alle blokjes die nog aanwezig zijn 1 rij naar beneden laten zakken
void move_after_row_clearing(uint8_t row)
{
  for (uint8_t i = row - 1; i > 0; i--)
  {
    for (uint8_t j = 0; j < GAME_FIELD_COLS; j++)
    {
      uint8_t key = game_field[i][j];
      if (key != EMPTY)
        find_and_update(i, j);
    }
  }

  for (uint8_t i = row - 1; i > 0; i--)
  {
    for (uint8_t j = 0; j < GAME_FIELD_COLS; j++)
    {
      uint8_t key = game_field[i][j];
      game_field[i][j] = game_field[i - 1][j];
      if (key != EMPTY)
        game_field[i + 1][j] = key;
    }
  }
}

// een blokje verwijderen uit het parent blok
void remove_square(uint8_t row, uint8_t col)
{
  uint8_t key = INVALID;
  uint8_t b_key = game_field[row][col];
  Block *block = storage[b_key];
  uint8_t nosq = block->number_of_squares;
  // het blokje (de idx daarvan) vinden in de array van blokjes in het parent blok
  for (uint8_t i = 0; i < nosq; i++)
  {
    if (block->squares[i]->x == col && block->squares[i]->y == row)
    {

      key = i;
      break;
    }
  }
  // als de idx niet gevonden wordt dan is er iets fout
  if (key == INVALID)
  {
    error_handler(se_sq);
  }
  // het blokje freeen uit de memory
  free(block->squares[key]);
  // een storage move uitvoeren op de array van blokjes
  // om te zorgen dat er gaan gaatjes in zijn
  //(het is een kleine array, dus er is geen grote performantieverlies)
  if (!(key == nosq - 1))
  {
    for (uint8_t i = key; i < nosq - 1; i++)
    {

      block->squares[i] = block->squares[i + 1];
    }
  }
  --(block->number_of_squares);
  // als het aantal aanwezige blokjes 0 is, dan moet je het parent blok verwijderen
  if (storage[b_key]->number_of_squares == 0)
  {
    remove_block(b_key);
    // de score van de speler verhogen
    score_up();
  }
}

// de score van de speler verhogen en de delay verlagen (zodat het spel sneller wordt)
void score_up()
{
  score++;
  if (default_delay > 0)
  {
    default_delay -= DELAY_DECLINING_UNIT;
  }
}

// een rij verwijderen uit het spel
void empty_row(uint8_t which_row)
{
  for (uint8_t i = 0; i < GAME_FIELD_COLS; i++)
  {
    remove_square(which_row, i);
    game_field[which_row][i] = EMPTY;
  }
}

// checken of een rij vol is (geen vrije vakjes meer)
uint8_t is_row_full(uint8_t which_row)
{
  for (uint8_t i = 0; i < GAME_FIELD_COLS; i++)
  {
    if (game_field[which_row][i] == EMPTY)
      return 0;
  }

  return 1;
}

// checken of een rij vol is en indien die vol is, die dan ook verwijderen
// en de rest van het spel updaten
void check_for_full_row(uint8_t row)
{
  if (is_row_full(row))
  {
    score_up();
    empty_row(row);
    move_after_row_clearing(row);
    redraw_field();
  }
}

// checken of de rijen van de blokjes van het vallende blok (nadat het blok beland heeft) vol zijn
void check_for_full_landed_block_rows()
{
  for (uint8_t i = 0; i < current_falling_block->number_of_squares; i++)
  {
    uint8_t row = current_falling_block->squares[i]->y;
    check_for_full_row(row);
  }
}

// alle rijen van het spel checken voor volle rijen
void check_for_full_rows()
{
  for (uint8_t row = 0; row < GAME_FIELD_ROWS; row++)
  {
    check_for_full_row(row);
  }
}

// een blokje naar beneden laten bewegen tot het beland
void move_square_down_until_landing(uint8_t row, uint8_t col)
{
  for (uint8_t i = row; i < GAME_FIELD_ROWS; i++)
  {
    if (i + 1 >= GAME_FIELD_ROWS || game_field[i + 1][col] != EMPTY)
    {
      break;
    }

    game_field[i + 1][col] = game_field[i][col];
    find_and_update(i, col);
    game_field[i][col] = EMPTY;
  }
}

// laat alle blokjes naar beneden zakken na een ontploffing
void move_down_after_kaboom(uint8_t row, uint8_t col)
{
  for (int8_t i = row; i >= 0; i--)
  {
    move_square_down_until_landing(i, col);
  }
}

// de ontploffing nadat een mijn tegen iets aanbotst (bodem of een blokje vanonder)
void kaboom()
{

  uint8_t row = current_falling_block->squares[0]->y;
  uint8_t col = current_falling_block->squares[0]->x;

  game_field[row][col] = EMPTY;
  free_current_falling_block();

  // voor alle rijen vanaf de rij boven het element tot de rij onder het element.
  for (int8_t row_offset = -1; row_offset < 2; row_offset++)
  {
    // voor alle kolommen vanaf de kolom links van het element tot de kolom rechts van het element.
    for (int8_t col_offset = -1; col_offset < 2; col_offset++)
    {

      // de randgevallen: wanneer het vakje aan de rand van het spel zit
      if ((row_offset == col_offset) && (row_offset == 0))
        continue;
      else if ((row_offset + row) < 0 || (row_offset + row >= GAME_FIELD_ROWS))
        continue;
      else if ((col_offset + col) < 0 || (col_offset + col >= GAME_FIELD_COLS))
        continue;
      // vind het corresponderende parent blok en verwijder het blokje daaruit
      uint8_t key = game_field[row_offset + row][col_offset + col];
      if (key != EMPTY)
      {
        remove_square(row_offset + row, col_offset + col);
        game_field[row_offset + row][col_offset + col] = EMPTY;
        // laat alle blokjes daarboven naar beneden vallen
        move_down_after_kaboom(row + row_offset - 1, col + col_offset);
      }
    }
  }
  redraw_field();
}

void update_timer(uint8_t with_timed)
{
  if (with_timed && timer != TIMER_OFF)
  {
    if (--timer == 0)
    {
      if (timed_block_still_there())
      {
        go_to_gameover();
      }
      else
      {
        timer = TIMER_OFF;
        timed_block_key = INVALID;
      }
    }
  }
}

uint8_t timed_block_still_there()
{
  return (timed_block_key != INVALID &&
          storage[timed_block_key] != NULL &&
          (storage[timed_block_key]->type == timed &&
           storage[timed_block_key]->number_of_squares != 0));
}

void move_falling_block()
{
  clear_previous_block_position(current_falling_block);
  move_block_horitonztally(current_falling_block);
  // laat het blok verder naar beneden zakken
  move_block_down(current_falling_block);
  // roteer het blok als A ingedrukt was
  if (M5.BtnA.wasPressed())
    rotate_block(current_falling_block);
  draw_block(current_falling_block);
}

void game()
{

  if (current_falling_block)
  {
    if (landed_on_bottom(current_falling_block) || block_landed_on_block(current_falling_block))
    {
      // als het vallende blok een mijn is, laat het ontploffen.
      if (current_falling_block->type == mine)
      {
        kaboom();
        // check na de ontploffing of er volledige rijen zijn
        check_for_full_rows();
      }
      else // het vallende blok was geen mijn
      {
        uint8_t idx_in_storage = add_block();
        if (with_timed && current_falling_block->type == timed) // als het blok een timed block was, en het spel in geconfigureerd met timed blocks, zet de timed_block_key op de key van het blok
        {
          timed_block_key = idx_in_storage;
        }
        draw_squares_in_matrix(current_falling_block->squares, current_falling_block->number_of_squares, idx_in_storage);
        // ga na of de rijen van de blokjes van het blok dat net beland heeft volledig zijn
        check_for_full_landed_block_rows();
        current_falling_block = NULL;
      }

      if (is_game_over())
      {
        go_to_gameover();
      }
      update_timer(with_timed);
    }

    else
    {
      move_falling_block();
    }
  }
  else
  {
    // als er geen vallende blok is, maak er 1 aan
    current_falling_block = make_block(pick_random_block(with_mines), with_timed);
  }

  // het spel inladen/opslaan wanneer de juiste knoppen ingedrukt zijn
  if (m5.BtnB.wasPressed() && m5.BtnA.wasPressed())
  {
    load_game();
  }
  else if (m5.BtnB.wasPressed())
  {
    save_game();
  }
}

/*        START MENU        */

// geeft de index van de keuze die de gebruiker heeft aangeduid terug
uint8_t get_index()
{
  return (y - INIT_MENU_Y) / INC_Y;
}
void start_menu()
{
  m5.Lcd.setCursor(0, 5);
  m5.Lcd.printf("Select a game\n\n");
  m5.Lcd.printf("M = \nmines\n\n");
  m5.Lcd.printf("T = \ntimed blocks\n\n");
  m5.Lcd.setCursor(0, INIT_MENU_Y);
  m5.Lcd.printf("M & T\n\n");
  m5.Lcd.printf("No M & No T\n\n");
  m5.Lcd.printf("No M & T\n\n");
  m5.Lcd.printf("M & No T\n\n");
  m5.Lcd.setCursor(INDICATOR_X, y);
  m5.Lcd.printf("<");

  // navigeren tussen de selectoes
  if (M5.BtnA.wasPressed())
  {
    m5.Lcd.fillRect(INDICATOR_X, y, 10, 10, black_color);
    y += INC_Y;
    if (y >= INIT_MENU_Y + (INC_Y * MENU_OPTIONS))
    {
      y = INIT_MENU_Y;
    }
  }

  // een selectie bevestigen
  if (M5.BtnB.wasPressed())
  {
    currently_active = GAME;
    switch (get_index())
    {
    case mines_timed:
      with_mines = 1;
      with_timed = 1;
      break;
    case no_mines_no_timed:
      with_mines = 0;
      with_timed = 0;
      break;
    case no_mines_timed:
      with_mines = 0;
      with_timed = 1;
      break;
    case mines_no_timed:
      with_mines = 1;
      with_timed = 0;
      break;
    }
    loop_delay = default_delay;
    M5.Lcd.fillScreen(black_color);
  }
}

/*        GAME OVER       */

// nagaan of het spel gedaan is
// het spel is gedaan als de blokken de top van het scherm hebben bereikt
uint8_t is_game_over()
{
  // voor het geval (dat wss nooit zal voorkomen), dat de score 255 is, dan is het spel gedaan.
  if (score >= 255)
    return 1;

  for (uint8_t i = 0; i < GAME_FIELD_COLS; i++)
  {
    if (game_field[0][i] != EMPTY)
      return 1;
  }
  return 0;
}

void go_to_gameover()
{
  free_all();
  initialize_game_field();
  m5.Lcd.fillScreen(black_color);
  currently_active = GAME_ENDED;
  loop_delay = 0;
}

// de score printen op het einde van het spel wanneer de speler verliest
void game_over()
{
  M5.Lcd.setCursor(0, 50);
  m5.Lcd.printf("Game over!\n Score: %d", score);
  if (M5.BtnA.wasPressed())
  {
    default_delay = INIT_DELAY;
    score = 0;
    currently_active = MENU;
    m5.Lcd.fillScreen(black_color);
  }
}

void setup()
{
  M5.begin();
  M5.IMU.Init();
  Serial.begin(115200);
  Serial.flush();
  M5.Lcd.fillScreen(black_color);
  EEPROM.begin(MEM_SIZE);
  randomSeed(analogRead(36));
  initialize_game_field();
}

void loop()
{

  switch (currently_active)
  {
  case MENU:
    start_menu();
    break;
  case GAME:
    game();
    break;
  case GAME_ENDED:
    game_over();
    break;
  }

  M5.update();
  delay(loop_delay);
}