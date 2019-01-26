#include "SDL2/SDL.h"
#include "SDL2/SDL_shape.h"
#include "SDL2/SDL_image.h"

#define not !
#define and &&
#define or ||
#define unless(x) if (not(x))
#define null NULL

#define __cleanup(id, func, ...) \
    inline void _clean_up_fn_##id(void * arg) { func(__VA_ARGS__); } \
    struct {} _clean_up_st_##id __attribute__ ((cleanup(_clean_up_fn_##id))) \

#define _cleanup(id, func, ...) __cleanup(id, func, __VA_ARGS__)
#define cleanup(func, ...) _cleanup(__COUNTER__, func, __VA_ARGS__)

typedef enum {
    error = 0,
    ok
} exit_e;

#define le(format, ...) SDL_LogError(SDL_LOG_CATEGORY_ERROR, format, ##__VA_ARGS__)
#define lg(format, ...) printf(format "\n", ##__VA_ARGS__);

extern unsigned char gl_cat_image[], gl_tac_image[];
SDL_Rect cat_rect = {0, 0, 128, 82};
Uint32 gl_delay   = 100;
Uint32 gl_frames  = 6;
int gl_velocity = 14;

int main(int argc, char* argv[]) {

    // init SDL
    if (SDL_VideoInit(null)) {
        le("failed init SDL: %s", SDL_GetError());
        return error;
    }
    cleanup(SDL_VideoQuit);

    if (IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG != IMG_INIT_PNG) {
        le("failed init sdl image extentions: %s", SDL_GetError());
        return error;
    }

    int displays = SDL_GetNumVideoDisplays();
    SDL_Rect display_rect;
    SDL_Rect desktop_rect = {0, 0, 0, 0};
    for(int i = 0; i < displays; i++) {
        SDL_GetDisplayBounds(i, &display_rect);
        //lg("display [%d]: pos %d:%d size %d:%d", i, display_rect.x, display_rect.y, display_rect.w, display_rect.h);
        if (desktop_rect.w < display_rect.x + display_rect.w) {
            desktop_rect.w = display_rect.x + display_rect.w;
        }
        if (desktop_rect.h < display_rect.y + display_rect.h) {
            desktop_rect.h = display_rect.y + display_rect.h;
        }
    }

    SDL_Window *window = SDL_CreateShapedWindow(
        "running-cat",
        0, desktop_rect.h - cat_rect.h,
        cat_rect.w, cat_rect.h,
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_SKIP_TASKBAR | SDL_WINDOW_TOOLTIP
    );
    
    unless (window) {
        le("failed create window: %s", SDL_GetError());
        return error;
    }
    cleanup(SDL_DestroyWindow, window);
    
    //BUG with SDL shaped windows, on CreateShapedWindow its position is not set
    SDL_SetWindowPosition(window, 0, desktop_rect.h - cat_rect.h);

    //  make window transparent
    if(SDL_SetWindowOpacity(window, 0.0f) < 0) {
        le("failed set opacity of window: %s", SDL_GetError());
        return error;
    }
        
    SDL_Renderer *render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC); //
    if (render == null) {
        le("failed create render: %s", SDL_GetError());
        return error;
    }
    cleanup(SDL_DestroyRenderer, render);

    if (SDL_SetRenderDrawBlendMode(render, SDL_BLENDMODE_ADD) < 0) {
        le("failed set blend mode: %s", SDL_GetError());
        return error;
    }

    SDL_RWops *cat_image_raw = SDL_RWFromConstMem(gl_cat_image, sizeof(char) * cat_rect.w * cat_rect.h);
    SDL_Surface *cat_surface = IMG_Load_RW(cat_image_raw, 1);
    SDL_FreeRW(cat_image_raw);
    unless (cat_surface) {
        le("failed init cat surface from image raw data: %s", IMG_GetError());
        return error;
    }
    cleanup(SDL_FreeSurface, cat_surface);

    SDL_RWops *tac_image_raw = SDL_RWFromConstMem(gl_tac_image, sizeof(char) * cat_rect.w * cat_rect.h);
    SDL_Surface *tac_surface = IMG_Load_RW(tac_image_raw, 1);
    SDL_FreeRW(tac_image_raw);
    unless (tac_surface) {
        le("failed init tac surface from image raw data: %s", IMG_GetError());
        return error;
    }
    cleanup(SDL_FreeSurface, tac_surface);

    SDL_Surface *shape_surface = cat_surface;
    SDL_Texture *shape_texture = SDL_CreateTextureFromSurface(render, shape_surface);
    unless (shape_texture) {
        le("failed create cat texture from cat surface: %s", SDL_GetError());
        return error;
    }
    cleanup(SDL_DestroyTexture, shape_texture);

    SDL_Color black = {0,0,0,0xff};
    SDL_WindowShapeMode shape_mode;

    shape_mode.mode = ShapeModeColorKey;    //ShapeModeBinarizeAlpha;
    shape_mode.parameters.colorKey = black; //binarizationCutoff = SDL_ALPHA_TRANSPARENT;

    //Create RGB surface for drawing

    Uint32 rmask, gmask, bmask, amask;
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        rmask = (Uint32) 0xff000000;
        gmask = (Uint32) 0x00ff0000;
        bmask = (Uint32) 0x0000ff00;
        amask = (Uint32) 0x000000ff;
    #else
        rmask = (Uint32) 0x000000ff;
        gmask = (Uint32) 0x0000ff00;
        bmask = (Uint32) 0x00ff0000;
        amask = (Uint32) 0xff000000;
    #endif

    SDL_Surface *window_surface = null;
    SDL_Texture *window_texture = null;

    window_surface = SDL_CreateRGBSurface(0, cat_rect.w, cat_rect.h, 32, rmask, gmask, bmask, amask);
    cleanup(SDL_FreeSurface, window_surface);
    SDL_BlitSurface(cat_surface, &cat_rect, window_surface, &cat_rect);    
    SDL_SetWindowSize(window, cat_rect.w, cat_rect.h);
    SDL_SetWindowShape(window, window_surface, &shape_mode);

    SDL_SetWindowPosition(window, desktop_rect.x, desktop_rect.h - cat_rect.h);

    int gl_frame = 0;
    int velocity = 24;

    void draw(void) {

        SDL_Surface *window_surface_prev = window_surface;
        SDL_Texture *window_texture_prev = window_texture;

        int frame = ((SDL_GetTicks() / (gl_delay)) % gl_frames);
        SDL_Rect frame_rect;
        frame_rect.y = 0;
        frame_rect.x = cat_rect.w * frame;
        frame_rect.w = cat_rect.w;
        frame_rect.h = cat_rect.h;

        gl_frame = frame;

        if (desktop_rect.x < desktop_rect.w + cat_rect.w and desktop_rect.x > -cat_rect.w) {
            desktop_rect.x += velocity;
        }
        else {
            SDL_DestroyTexture(shape_texture);
            shape_surface = velocity > 0 ? tac_surface : cat_surface;
            shape_texture = SDL_CreateTextureFromSurface(render, shape_surface);
            velocity *= -1;
            desktop_rect.x += velocity;
        }
        
        SDL_BlitSurface(shape_surface, &frame_rect, window_surface, &cat_rect);
        window_texture = SDL_CreateTextureFromSurface(render, window_surface);

        SDL_ResetWindowShape(window); // for this need patch SDL to add this function
        SDL_SetWindowShape(window, window_surface, &shape_mode);
                
        SDL_SetWindowPosition(window, desktop_rect.x, desktop_rect.h - cat_rect.h);
        
        SDL_SetRenderDrawColor(render, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(render);       

        SDL_RenderCopy(render, window_texture, &cat_rect, &cat_rect);
    
        SDL_RenderPresent(render);
    
        if (window_texture_prev) SDL_DestroyTexture(window_texture_prev);
        
        return;
    }

    draw();
    SDL_Event event;
    int loop = 1;

    Uint32 timeout = SDL_GetTicks() + gl_delay;
    while (loop) {
        if (SDL_PollEvent(&event) or SDL_WaitEventTimeout(&event, gl_delay/2)) {
            switch (event.type) {
                case SDL_QUIT:
                    loop = 0;
                        break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        loop = 0;
                    }
                    break;
            }
        }
        if (SDL_TICKS_PASSED(SDL_GetTicks(), timeout)) {
            draw();
            timeout = SDL_GetTicks() + gl_delay;
        }
    }
    if (window_texture) {
        SDL_DestroyTexture(window_texture);
    }
    
    return ok;

}

