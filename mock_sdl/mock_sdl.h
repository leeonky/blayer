#ifndef MOCK_SDL_
#define MOCK_SDL_

#include <cunitexd.h>
#include <SDL2/SDL.h>

extern_mock_function_1(int, SDL_InitSubSystem, Uint32);
extern_mock_void_function_1(SDL_QuitSubSystem, Uint32);

extern_mock_function_3(SDL_Renderer *, SDL_CreateRenderer, SDL_Window *, int, Uint32);
extern_mock_void_function_1(SDL_DestroyRenderer, SDL_Renderer *);

extern_mock_function_6(SDL_Window *, SDL_CreateWindow, const char *, int, int, int, int, Uint32);
extern_mock_void_function_1(SDL_DestroyWindow, SDL_Window *);

extern_mock_function_1(int, SDL_ShowCursor, int);

extern_mock_function_5(SDL_Texture *, SDL_CreateTexture, SDL_Renderer *, Uint32, int, int, int);
extern_mock_void_function_1(SDL_DestroyTexture, SDL_Texture *);

#endif
