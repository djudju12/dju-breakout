#include <stdio.h>
#include <SDL2/SDL.h>
#include <assert.h>
#include <math.h>

#define overlaps(a, b) SDL_HasIntersection(a, b)

// consts
#define WIDTH 800
#define HEIGTH 800
#define FPS 60
#define DELTA_TIME_SEC 1.0 / FPS

#define PAD_HEIGTH 25
#define PAD_WIDTH 100
#define PAD_Y WIDTH - PAD_HEIGTH * 2
#define PAD_COLOR 0x275c32FF

#define BALL_SIZE 15
#define BALL_Y HEIGTH/2 + BALL_SIZE*2
#define BALL_X WIDTH/2 - BALL_SIZE/2

#define BALL_SPEED 350
#define PAD_VEL BALL_SPEED * 1.5

#define COL_LEN_TARGETS 5
#define ROW_LEN_TARGETS 8
#define TARGETS_IN_BETWEEN BALL_SIZE * 2
#define TARGET_COLOR 0x383a72FF

#define BG_COLOR 0x181818FF
#define WHITE_COLOR 0xFFFFFFFF

typedef struct 
{
   SDL_Rect rect;
   int alive;
} Target;

// state variables
SDL_Renderer *renderer;
SDL_Rect pad;
SDL_Rect ball;
int pad_x = WIDTH / 2 - PAD_WIDTH / 2;
int ball_x = BALL_X;
int ball_y = BALL_Y;
Uint8 ball_dx = 1;
Uint8 ball_dy = 1;
Uint8 running = 1;
Uint8 loose = 0;
Uint8 win = 0;
Uint8 count_targets = ROW_LEN_TARGETS * COL_LEN_TARGETS;
Uint8 pause = 0;
Target targets[ROW_LEN_TARGETS * COL_LEN_TARGETS];

void set_color(SDL_Renderer *renderer, Uint32 color)
{
   Uint8 r = color >> 8 * 3;
   Uint8 g = color >> 8 * 2;
   Uint8 b = color >> 8 * 1;
   Uint8 a = color >> 8 * 0;
   SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

struct SDL_Rect pad_rect(int x)
{
   return (SDL_Rect){x, PAD_Y, PAD_WIDTH, PAD_HEIGTH};
}

struct SDL_Rect ball_rect(int x, int y)
{
   return (SDL_Rect){x, y, BALL_SIZE, BALL_SIZE};
}

void ball_hcollision(float dt)
{
   int new_x = ball_x + ball_dx * BALL_SPEED * dt;
   if ((new_x < 0) ||
       (new_x + BALL_SIZE > WIDTH)) ball_dx *= -1;

   ball_x = new_x;
}

void ball_vcollision(float dt)
{
   int new_y = ball_y + ball_dy * BALL_SPEED * dt;
   if (new_y + BALL_SIZE > HEIGTH) loose = 1;
   if (new_y < 0) ball_dy *= -1; 

   ball_y = new_y;
}

void ball_tcollision()
{
   for (size_t i = 0; i < ROW_LEN_TARGETS; i++){
      for (size_t j = 0; j < COL_LEN_TARGETS; j++){
         Target *target = &targets[i * ROW_LEN_TARGETS + j];
         if (overlaps(&ball, &target->rect) && target->alive){
            ball_dy *= -1;
            target->alive = 0;
            count_targets--;
         }
      }
   }
}

void update(float dt)
{
   pad = pad_rect(pad_x);

   if (overlaps(&ball, &pad))
   {
      ball_y = PAD_Y - PAD_HEIGTH / 2 - BALL_SIZE - 1.0;
      ball_dy *= -1;
   }
   else
   {
      ball_hcollision(dt);
      ball_vcollision(dt);
      ball_tcollision();
   }

   ball = ball_rect(ball_x, ball_y);
}

void render()
{

   set_color(renderer, PAD_COLOR);
   SDL_RenderFillRect(renderer, &pad);

   set_color(renderer, TARGET_COLOR);
   for (size_t i = 0; i < ROW_LEN_TARGETS; i++){
      for (size_t j = 0; j < COL_LEN_TARGETS; j++){
         Target target = targets[i * ROW_LEN_TARGETS + j];
         if (target.alive) SDL_RenderFillRect(renderer, &target.rect);
      }
   }

   set_color(renderer, WHITE_COLOR);
   SDL_RenderFillRect(renderer, &ball);
}

void init_targets()
{
   float x_space = 1.5;
   float y_space = 2.0;
   int initial_x = WIDTH/2 - (COL_LEN_TARGETS*PAD_WIDTH*x_space)/2.0;

   for (size_t i = 0; i < ROW_LEN_TARGETS; i++)
   {
      for (size_t j = 0; j < COL_LEN_TARGETS; j++)
      {
          SDL_Rect rect = {
             .x = initial_x + j*PAD_WIDTH*x_space,
             .y = PAD_HEIGTH*y_space + i*PAD_HEIGTH*y_space,
             .w = PAD_WIDTH,
             .h = PAD_HEIGTH
             };

         targets[i * ROW_LEN_TARGETS + j] = (Target){rect, 1};
      }
   }
}

void move_left()
{
   if (pad_x < 0) return;
   pad_x -= PAD_VEL * DELTA_TIME_SEC;
}

void move_right()
{
   if (pad_x + PAD_WIDTH > WIDTH) return;
   pad_x += PAD_VEL * DELTA_TIME_SEC;
}

void reset()
{
   loose = 0;
   win = 0;
   count_targets = ROW_LEN_TARGETS * COL_LEN_TARGETS;
   pause = 1;
   ball_x = BALL_X;
   ball_y = BALL_Y;
   init_targets();
}

int main(void)
{
   if (SDL_Init(SDL_INIT_EVERYTHING) != 0) goto err_SDL_INIT;

   SDL_Window *window = SDL_CreateWindow("BREAKOUT", 0, 0, WIDTH, HEIGTH, SDL_WINDOW_BORDERLESS);
   if (!window) goto err_SDL_WINDOW;

   renderer = SDL_CreateRenderer(window, -1, 0);
   if (renderer < 0) goto err_SDL_RENDERER;

   const Uint8 *keyboard = SDL_GetKeyboardState(NULL);

   init_targets();

   while (running)
   {
      SDL_Event event;
      while (SDL_PollEvent(&event))
      {
         if (event.type != SDL_KEYDOWN)
            continue;

         if (event.type == SDL_QUIT)
            running = 0;

         switch (event.key.keysym.sym)
         {
         case SDLK_ESCAPE:
            running = 0;
            break;

         case SDLK_SPACE:
            pause = !pause;
            break;

         default:
            break;
         }
      }

      if (pause) continue;
      if (keyboard[SDL_SCANCODE_A]) move_left();
      if (keyboard[SDL_SCANCODE_D]) move_right();
      win = count_targets == 0;
      if (loose || win) reset();

      set_color(renderer, BG_COLOR);
      SDL_RenderClear(renderer);

      update(DELTA_TIME_SEC);
      render(renderer);

      SDL_RenderPresent(renderer);

      SDL_Delay(1000 / FPS);
   }

   printf("[INFO] cleaning up...\n");
   SDL_DestroyWindow(window);
   SDL_DestroyRenderer(renderer);
   return 0;

//  error handling
err_SDL_INIT:
   printf("[ERROR] error on initializing SDL %s\n", SDL_GetError());

err_SDL_WINDOW:
   printf("[ERROR] error creating sdl window %s\n", SDL_GetError());
   SDL_DestroyWindow(window);

err_SDL_RENDERER:
   printf("[ERROR] error creating sdl renderer %s\n", SDL_GetError());
   SDL_DestroyWindow(window);
   SDL_DestroyRenderer(renderer);
}
