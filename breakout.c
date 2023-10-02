#include <stdio.h>
#include <SDL2/SDL.h>
#include <assert.h>
#include <math.h>

#define overlaps(a, b) SDL_HasIntersection(a, b)

// consts
#define WIDTH 640
#define HEIGTH 640
#define FPS 60
#define DELTA_TIME_SEC 1.0 / FPS
#define PAD_HEIGTH 30
#define PAD_WIDTH 100
#define BALL_SIZE 15
#define BALL_SPEED 350
#define PAD_VEL BALL_SPEED * 1.5
#define PAD_Y WIDTH - PAD_HEIGTH * 2
#define PAD_COLOR 0x275c32FF
#define BG_COLOR 0x181818FF
#define WHITE_COLOR 0xFFFFFFFF
#define TARGETS_IN_BETWEEN BALL_SIZE*2
#define TARGETS_OFFSETY PAD_HEIGTH
#define TARGETS_OFFSETX PAD_WIDTH


// state variables
SDL_Renderer *renderer;
SDL_Rect pad;
SDL_Rect ball;
SDL_Rect *targets;
int pad_x = WIDTH / 2 - PAD_WIDTH / 2;
int ball_x = WIDTH / 2 - BALL_SIZE / 2;
int ball_y = HEIGTH / 2 - BALL_SIZE / 2;
int ball_dx = 1;
int ball_dy = 1;
int running = 1;

// debug funcs
void debug_padx()
{
   printf("[DEBUG] padx = %d\n", pad_x);
}

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

void init_targets()
{
   //    -   -   -   - 
   //    -   -   -   - 
   //    -   -   -   - 
   //    -   -   -   - 
   // 
   // 
   //          -
   
   for (size_t i = TARGETS_OFFSETX; i < TARGET_COUNT; i++) {
      for (size_t j = TARGETS_OFFSETY; j < count; j++)
      {
         /* code */
      }
      
      targets[i] = (SDL_Rect) {0, 0, PAD_WIDTH, PAD_HEIGTH};
   }
   
}

void ball_hcollision()
{
   int new_x = ball_x + ball_dx * BALL_SPEED * DELTA_TIME_SEC;
   if ((new_x < 0) ||
       (new_x + BALL_SIZE > WIDTH))
      ball_dx *= -1;

   ball_x = new_x;
}

void ball_vcollision()
{
   int new_y = ball_y + ball_dy * BALL_SPEED * DELTA_TIME_SEC;
   if ((new_y < 0) ||
       (new_y + BALL_SIZE > HEIGTH))
      ball_dy *= -1;

   ball_y = new_y;
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
      ball_hcollision();
      ball_vcollision();
   }

   ball = ball_rect(ball_x, ball_y);
}

void render()
{
   set_color(renderer, PAD_COLOR);
   SDL_RenderFillRect(renderer, &pad);

   set_color(renderer, WHITE_COLOR);
   SDL_RenderFillRect(renderer, &ball);
}

void move_left()
{
   if (pad_x < 0)
      return;
   pad_x -= PAD_VEL * DELTA_TIME_SEC;
}

void move_right()
{
   if (pad_x + PAD_WIDTH > WIDTH)
      return;
   pad_x += PAD_VEL * DELTA_TIME_SEC;
}

int main(void)
{
   if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
      goto err_SDL_INIT;

   SDL_Window *win = SDL_CreateWindow("BREAKOUT", 0, 0, WIDTH, HEIGTH, SDL_WINDOW_BORDERLESS);
   if (!win)
      goto err_SDL_WINDOW;

   renderer = SDL_CreateRenderer(win, -1, 0);
   if (renderer < 0)
      goto err_SDL_RENDERER;

   const Uint8 *keyboard = SDL_GetKeyboardState(NULL);

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

         default:
            break;
         }
      }

      if (keyboard[SDL_SCANCODE_A])
         move_left();
      if (keyboard[SDL_SCANCODE_D])
         move_right();

      set_color(renderer, BG_COLOR);
      SDL_RenderClear(renderer);

      update(DELTA_TIME_SEC);
      render(renderer);

      SDL_RenderPresent(renderer);

      debug_padx();

      SDL_Delay(1000 / FPS);
   }

   goto CLEAN_UP;

//  error handling
err_SDL_INIT:
   printf("[ERROR] error on initializing SDL %s\n", SDL_GetError());

err_SDL_WINDOW:
   printf("[ERROR] error creating sdl window %s\n", SDL_GetError());
   SDL_DestroyWindow(win);

err_SDL_RENDERER:
   printf("[ERROR] error creating sdl renderer %s\n", SDL_GetError());
   SDL_DestroyWindow(win);
   SDL_DestroyRenderer(renderer);

// cleaning up
CLEAN_UP:
   printf("[INFO] cleaning up...\n");
   SDL_DestroyWindow(win);
   SDL_DestroyRenderer(renderer);
   return 0;
}
