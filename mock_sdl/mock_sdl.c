#include "mock_sdl.h"

mock_function_1(int, SDL_InitSubSystem, Uint32);
mock_void_function_1(SDL_QuitSubSystem, Uint32);

mock_function_3(SDL_Renderer *, SDL_CreateRenderer, SDL_Window *, int, Uint32);
mock_void_function_1(SDL_DestroyRenderer, SDL_Renderer *);

mock_function_6(SDL_Window *, SDL_CreateWindow, const char *, int, int, int, int, Uint32);
mock_void_function_1(SDL_DestroyWindow, SDL_Window *);
