// Using SDL2, standard IO, math, and strings
#include <stdio.h>
#include <deque>
#include <string>
#include <cmath>
#include <chrono>
#include <map>

#include <SDL2/SDL.h>

#include "Chip8.h"

//Screen dimension constants
const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 726;

//Starts up SDL and creates window
bool init();

//Frees media and shuts down SDL
void close();

//The window we'll be rendering to
static SDL_Window* gWindow = nullptr;

//The window renderer
static SDL_Renderer* gRenderer = nullptr;

// texture to draw on
static SDL_Texture* texture;

// audio specifications
static SDL_AudioSpec obtained;

// sound queue data
static std::deque<std::pair<unsigned/*samples*/,bool/*volume*/> > AudioQueue;

bool init()
{
    //Initialization flag
    bool success = true;

    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0 )
    {
        printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
        success = false;
    }
    else
    {
        //Set texture filtering to linear
        if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
        {
            printf( "Warning: Linear texture filtering not enabled!" );
        }

        //Create window
        gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
        if( gWindow == nullptr )
        {
            printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
            success = false;
        }
        else
        {
            //Create renderer for window
            gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED );
            if( gRenderer == nullptr )
            {
                printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
                success = false;
            }
            else
            {
                //Initialize renderer color
                SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

                 texture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, CHIP8_W,CHIP8_H);
                 if (!texture) {
                     printf("texture could not initialize!\n");
                     success = false;
                 } else {
                    //Initialize SDL_audio
                    SDL_AudioSpec spec;
                    spec.freq     = 44100;
                    spec.format   = AUDIO_S16SYS;
                    spec.channels = 2;
                    spec.samples  = spec.freq / 20; // 0.05 seconds of latency
                    spec.callback = [] (void*, Uint8* stream, int len)
                    {
                        // Generate square wave
                        short* target = (short*)stream;
                        while(len > 0 && !AudioQueue.empty())
                        {
                            auto& data = AudioQueue.front();
                            for(; len && data.first; target += 2, len -= 4, --data.first)
                                target[0] = target[1] = data.second*300*((len&128)-64);
                            if(!data.first) AudioQueue.pop_front();
                        }
                    };
                    if(SDL_OpenAudio(&spec, &obtained) != 0 ) {
                        printf( "SDL_audio could not initialize! SDL_mixer Error: %s\n", SDL_GetError());
                        success = false;
                    }
                    SDL_PauseAudio(0);
                 }
            }
        }
    }

    return success;
}

void close()
{
    //Destroy window
    SDL_DestroyRenderer( gRenderer );
    SDL_DestroyWindow( gWindow );
    gWindow = nullptr;
    gRenderer = nullptr;

    SDL_AudioQuit();

    //Quit SDL subsystems
    SDL_Quit();
}

int main( int argc, char* args[] )
{
    if (argc != 2) {
        printf("Usage: Chip8Emul filename\n");
        return 1;
    }

    Chip8Struct cpu;
    cpu.clear();
    Chip8Logic logic;
    logic.loadFontset(cpu.mem);
    logic.loadProgram(cpu.mem, cpu.waitKey, cpu.PC, args[1]);
    // map keyboard to keys in chip8;
    std::map<int,int> keymap = {
        {SDLK_1, 0x1}, {SDLK_2, 0x2}, {SDLK_3, 0x3}, {SDLK_4, 0xC},
        {SDLK_q, 0x4}, {SDLK_w, 0x5}, {SDLK_e, 0x6}, {SDLK_r, 0xD},
        {SDLK_a, 0x7}, {SDLK_s, 0x8}, {SDLK_d, 0x9}, {SDLK_f, 0xE},
        {SDLK_z, 0xA}, {SDLK_x, 0x0}, {SDLK_c, 0xB}, {SDLK_v, 0xF},
        //{SDLK_5, 0x5}, {SDLK_6, 0x6}, {SDLK_7, 0x7},
        {SDLK_8, 0x8}, {SDLK_9, 0x9}, {SDLK_0, 0x0}, {SDLK_ESCAPE,-1}
    };

    //Start up SDL and create window
    if( !init() )
    {
        printf( "Failed to initialize!\n" );
    }
    else
    {
        //Main loop flag
        bool quit = false;

        //Event handler
        SDL_Event e;

        unsigned insns_per_frame       = 7;
        unsigned max_consecutive_insns = 0;
        int frames_done = 0;

        auto start = std::chrono::system_clock::now();

        //While application is running
        while( !quit )
        {
            for(unsigned a=0; a<max_consecutive_insns && !logic.isWaitingForKey(cpu.waitKey); ++a) {
                logic.executeCommand(cpu, logic.getNextOpcode(cpu.mem, cpu.PC, cpu.waitKey));
            }

            // Handle events on queue
            while( SDL_PollEvent( &e ) != 0 )
            {
                std::map<int,int>::iterator it;
                switch(e.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                    it = keymap.find(e.key.keysym.sym);
                    if (it == keymap.end()) break;
                    if (it->second == -1) {quit = true; break;}
                    logic.setKeypad(cpu.V, cpu.keys, cpu.waitKey, it->second, SDL_KEYDOWN == e.type);
                    break;
                }
            }
            auto cur = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_seconds = cur-start;
            int frames = int(elapsed_seconds.count() * 60) - frames_done;

            if(frames > 0)
            {
                frames_done += frames;
                int st = std::min(frames, cpu.SoundTimer+0); cpu.SoundTimer -= st;
                int dt = std::min(frames, cpu.DelayTimer+0); cpu.DelayTimer -= dt;

                // Render audio
                SDL_LockAudio();
                 AudioQueue.emplace_back( obtained.freq*(         st)/60,  true );
                 AudioQueue.emplace_back( obtained.freq*(frames - st)/60, false );
                SDL_UnlockAudio();

                Uint32 pixels[CHIP8_W*CHIP8_H]; logic.renderTo(cpu.disp, pixels);

                SDL_SetRenderDrawColor(gRenderer, 0,0,0,0);
                SDL_RenderClear(gRenderer);
                int PIXEL_W = SCREEN_WIDTH/CHIP8_W;
                int PIXEL_H = SCREEN_HEIGHT/CHIP8_H;
                SDL_SetRenderDrawColor(gRenderer, 0xFF,0xFF,0xFF,0xFF);
                for (int i=0; i<CHIP8_H; ++i) {
                    for (int j=0; j<CHIP8_W; ++j) {
                        if (pixels[i*CHIP8_W + j]) {
                            SDL_Rect rect = { j*PIXEL_W, i*PIXEL_H, PIXEL_W, PIXEL_H };
                            SDL_RenderFillRect(gRenderer, &rect);
                        }
                    }
                }

//                    SDL_UpdateTexture(texture, nullptr, pixels, 4*CHIP8_W);
//                    SDL_RenderCopy(gRenderer, texture, nullptr, nullptr);
                SDL_RenderPresent(gRenderer);
            }

            max_consecutive_insns = std::max(frames, 1) * insns_per_frame;
            if(logic.isWaitingForKey(cpu.waitKey) || !frames) SDL_Delay(1000/60);
        }
    }

    //Free resources and close SDL
    close();

    return 0;
}
