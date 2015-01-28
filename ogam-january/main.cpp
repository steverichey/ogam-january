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
#import <SDL2_mixer/SDL_mixer.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define TILE_SIZE 32
#define MOVE_SPEED 2
#define DOUBLE_MOVE_SPEED MOVE_SPEED * 2
#define ANGLE_SPEED sqrt(DOUBLE_MOVE_SPEED)
#define FONT_SIZE 32
#define FONT_OFFSET 16
#define SCORE_PREFIX "Score "
#define ENTITY_SIZE 32
#define SCORE_INCREMENT_LIMIT 10

// log anything to the console
void log(const std::string &message) {
    std::cout << message << std::endl;
}

// log errors to the console, appends the SDL error message
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

TTF_Font* load_font(const std::string file_name, int font_size) {
    TTF_Font *font = TTF_OpenFont(file_name.c_str(), font_size);
    
    if (font == nullptr) {
        log_error("TTF open font error");
    }
    
    return font;
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

void render_text(const std::string &message, TTF_Font *font, SDL_Color color, SDL_Renderer *renderer, int x, int y) {
    // render text to a surface
    SDL_Surface *surface = TTF_RenderText_Solid(font, message.c_str(), color);
    
    if (surface == nullptr){
        log_error("TTF_RenderText error");
    }
    
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    
    if (texture == nullptr){
        log_error("CreateTexture error");
    }
    
    // clean up
    SDL_FreeSurface(surface);
    
    render_texture(texture, renderer, x, y);
}

// entry point
int main(int argc, const char * argv[]) {
    // initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        log_error("SDL Init Error");
        return 1;
    }
    
    // initialize SDL_Image
    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG){
        log_error("IMG_Init error");
        SDL_Quit();
        return 1;
    }
    
    // initialize SDL_TTF
    if (TTF_Init() != 0) {
        log_error("TTF_Init error");
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    
    // initialize SDL_Mixer
    if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 4096) != 0) {
        log_error("Mix_OpenAudio error!");
        IMG_Quit();
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    
    // create a window
    SDL_Window *window = SDL_CreateWindow("Don't Crash the Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    
    // verify window
    if (window == nullptr){
        log_error("SDL Create Window Error");
        IMG_Quit();
        TTF_Quit();
        Mix_Quit();
        SDL_Quit();
        return 1;
    }
    
    // create a rendering context
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    
    // verify rendering context
    if (renderer == nullptr){
        log_error("SDL Create Renderer Error");
        SDL_DestroyWindow(window);
        IMG_Quit();
        TTF_Quit();
        Mix_Quit();
        SDL_Quit();
        return 1;
    }
    
    // load textures
    std::string player_img_path = "Resources/player.png";
    SDL_Texture *player = load_texture(player_img_path, renderer);
    
    std::string bg_img_path = "Resources/bg.png";
    SDL_Texture *background = load_texture(bg_img_path, renderer);
    
    std::string enemy_img_path = "Resources/enemy.png";
    SDL_Texture *enemy = load_texture(enemy_img_path, renderer);
    
    if (player == nullptr || background == nullptr || enemy == nullptr) {
        log_error("Load texture error!");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        TTF_Quit();
        Mix_Quit();
        SDL_Quit();
        return 1;
    }
    
    // score tracking
    int score = 0;
    int score_counter = 0;
    std::string score_string = "";
    
    // load font
    std::string font_path = "Resources/ivory.ttf";
    TTF_Font *game_font = load_font(font_path, FONT_SIZE);
    SDL_Color font_color = {255, 0, 255, 255};
    
    if (game_font == nullptr) {
        log_error("Font load error");
        SDL_DestroyTexture(player);
        SDL_DestroyTexture(background);
        SDL_DestroyTexture(enemy);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        TTF_Quit();
        Mix_CloseAudio();
        SDL_Quit();
        return 1;
    }
    
    int player_w, player_h;
    SDL_QueryTexture(player, nullptr, nullptr, &player_w, &player_h);
    int player_x = (SCREEN_WIDTH - player_w) / 2;
    int player_y = (SCREEN_HEIGHT - player_h) / 2;
    
    // enemy position
    int enemy_x = 0;
    int enemy_y = 0;
    int enemy_move_x = -1;
    int enemy_move_y = -1;
    int enemy_move_x_speed = 1;
    int enemy_move_y_speed = 1;
    bool bounced = false;
    
    // sound FX
    Mix_Music *music = Mix_LoadMUS("Resources/haran.wav");
    Mix_Chunk *bounce = Mix_LoadWAV("Resources/bounce.wav");
    
    if (music == nullptr || bounce == nullptr) {
        log_error("Sound load error");
        TTF_CloseFont(game_font);
        SDL_DestroyTexture(player);
        SDL_DestroyTexture(background);
        SDL_DestroyTexture(enemy);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        Mix_CloseAudio();
        IMG_Quit();
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    
    // calculate the number of tiles to fill the screen
    int xTiles = SCREEN_WIDTH / TILE_SIZE;
    int yTiles = SCREEN_HEIGHT / TILE_SIZE;
    
    // bg position values
    int bg_x = 0;
    int bg_y = 0;
    
    // create event for input
    SDL_Event event;
    bool quit = false;
    
    // input tracking
    bool down_pressed  = false;
    bool up_pressed    = false;
    bool right_pressed = false;
    bool left_pressed  = false;
    
    // start some tunes!
    
    Mix_PlayMusic(music, 100);
    
    // MAIN GAME LOOP YO
    while (!quit) {
        // use input to set _pressed flags
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
            
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        quit = true;
                        break;
                    case SDLK_DOWN:
                        down_pressed = true;
                        break;
                    case SDLK_UP:
                        up_pressed = true;
                        break;
                    case SDLK_RIGHT:
                        right_pressed = true;
                        break;
                    case SDLK_LEFT:
                        left_pressed = true;
                        break;
                }
            }
            
            if (event.type == SDL_KEYUP) {
                switch (event.key.keysym.sym) {
                    case SDLK_DOWN:
                        down_pressed = false;
                        break;
                    case SDLK_UP:
                        up_pressed = false;
                        break;
                    case SDLK_RIGHT:
                        right_pressed = false;
                        break;
                    case SDLK_LEFT:
                        left_pressed = false;
                        break;
                }
            }
        }
        
        // pressed flag processing
        if (down_pressed && right_pressed) {
            player_x += ANGLE_SPEED;
            player_y += ANGLE_SPEED;
        } else if (down_pressed && left_pressed) {
            player_x -= ANGLE_SPEED;
            player_y += ANGLE_SPEED;
        } else if (up_pressed && right_pressed) {
            player_x += ANGLE_SPEED;
            player_y -= ANGLE_SPEED;
        } else if (up_pressed && left_pressed) {
            player_x -= ANGLE_SPEED;
            player_y -= ANGLE_SPEED;
        } else if (down_pressed) {
            player_y += MOVE_SPEED;
        } else if (up_pressed) {
            player_y -= MOVE_SPEED;
        } else if (right_pressed) {
            player_x += MOVE_SPEED;
        } else if (left_pressed) {
            player_x -= MOVE_SPEED;
        }
        
        // player bounds checking
        if (player_x < 0) {
            player_x = 0;
        }
        
        if (player_x > SCREEN_WIDTH - player_w) {
            player_x = SCREEN_WIDTH - player_w;
        }
        
        if (player_y < 0) {
            player_y = 0;
        }
        
        if (player_y > SCREEN_HEIGHT - player_h) {
            player_y = SCREEN_HEIGHT - player_h;
        }
        
        // enemy positioning
        enemy_x += enemy_move_x * enemy_move_x_speed;
        enemy_y += enemy_move_y * enemy_move_y_speed;
        
        if (enemy_x < 0) {
            enemy_move_x = 1;
            enemy_move_x_speed++;
            bounced = true;
        }
        
        if (enemy_x > SCREEN_WIDTH - ENTITY_SIZE) {
            enemy_move_x = -1;
            enemy_move_x_speed++;
            bounced = true;
        }
        
        if (enemy_y < 0) {
            enemy_move_y = 1;
            enemy_move_y_speed++;
            bounced = true;
        }
        
        if (enemy_y > SCREEN_HEIGHT - ENTITY_SIZE) {
            enemy_move_y = -1;
            enemy_move_y_speed++;
            bounced = true;
        }
        
        if (bounced) {
            if (Mix_PlayChannel(-1, bounce, 0) != 0) {
                log_error("Play sound error");
            }
            
            bounced = false;
        }
        
        // update score
        score_counter++;
        
        if (score_counter > SCORE_INCREMENT_LIMIT) {
            score_counter = 0;
            score++;
        }
        
        // clear renderer
        SDL_RenderClear(renderer);
        
        // render tiles to cover screen
        for (int i = 0; i < xTiles * yTiles; i++) {
            bg_x = i % xTiles;
            bg_y = i / xTiles;
            render_texture(background, renderer, bg_x * TILE_SIZE, bg_y * TILE_SIZE, TILE_SIZE, TILE_SIZE);
        }
        
        // draw the enemy
        render_texture(enemy, renderer, enemy_x, enemy_y);
        
        // draw the player
        render_texture(player, renderer, player_x, player_y);
        
        // draw the text
        score_string = SCORE_PREFIX + std::to_string(score);
        render_text(score_string, game_font, font_color, renderer, FONT_OFFSET, FONT_OFFSET);
        
        // lose state checking
        if (player_x + ENTITY_SIZE > enemy_x) {
            if (player_y + ENTITY_SIZE > enemy_y) {
                if (player_x < enemy_x + ENTITY_SIZE) {
                    if (player_y < enemy_y + ENTITY_SIZE) {
                        abort();
                    }
                }
            }
        }
        
        // render things!
        SDL_RenderPresent(renderer);
    }
    
    // close down game
    Mix_FreeChunk(bounce);
    Mix_FreeMusic(music);
    TTF_CloseFont(game_font);
    SDL_DestroyTexture(enemy);
    SDL_DestroyTexture(player);
    SDL_DestroyTexture(background);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    
    return 0;
}