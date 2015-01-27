//
//  main.m
//  ogam-january
//
//  Created by Steve Richey on 1/25/15.
//  Copyright (c) 2015 stvr. All rights reserved.
//

#import <iostream>
#import <SDL2/SDL.h>
#import <SDL2_image/SDL_image.h>
#import <SDL2_ttf/SDL_ttf.h>

#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480

// log anything to the console
void log(const std::string &message) {
    std::cout << message << std::endl;
}

// log anything to the console, appends the SDL error message
void log_error(const std::string &message) {
    log(message + SDL_GetError());
}

// load a texture from a provided file path
SDL_Texture* load_texture(const std::string filename, SDL_Renderer *renderer) {
    SDL_Texture *texture = IMG_LoadTexture(renderer, filename.c_str());
    
    if (texture == nullptr) {
        log_error("IMG_LoadTexture error");
    }
    
    return texture;
}

// render a texture at (x,y) with size w x h
void render_texture(SDL_Texture *texture, SDL_Renderer *renderer, int x, int y, int w, int h) {
    // create a target rect
    SDL_Rect dest;
    dest.x = x;
    dest.y = y;
    dest.w = w;
    dest.h = h;
    
    // really render!
    SDL_RenderCopy(renderer, texture, nullptr, &dest);
}

// render a texture at (x,y) with the texture's default width and height
void render_texture(SDL_Texture *texture, SDL_Renderer *renderer, int x, int y) {
    // create a destination rectangle
    int w;
    int h;
    
    // get width and height from texture
    SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);
    
    // render!
    render_texture(texture, renderer, x, y, w, h);
}

// entry point
int main(int argc, const char * argv[]) {
    log("Beginning game...");
    
    // initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        log_error("SDL Init Error");
        return 1;
    }
    
    // create a window
    SDL_Window *window = SDL_CreateWindow("ogam-january", 100, 100, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    
    // verify window
    if (window == nullptr){
        log_error("SDL Create Window Error");
        SDL_Quit();
        return 1;
    }
    
    // create a rendering context
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    
    // verify rendering context
    if (renderer == nullptr){
        log_error("SDL Create Renderer Error");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    // load textures
    std::string resource_path = "Resources/player.bmp";
    SDL_Texture *player = load_texture(resource_path, renderer);
    
    if (player == nullptr) {
        log_error("Player load texture error!");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    int player_w, player_h;
    SDL_QueryTexture(player, nullptr, nullptr, &player_w, &player_h);
    int player_x = (SCREEN_WIDTH - player_w) / 2;
    int player_y = (SCREEN_HEIGHT - player_h) / 2;
    
    render_texture(player, renderer, player_x, player_y);
    SDL_RenderPresent(renderer);
    
    SDL_Delay(2000);
    
    SDL_DestroyTexture(player);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    log("Game over!");
    
    return 0;
}