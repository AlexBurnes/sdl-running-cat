# running-cat
SDL example of animated shaped window

This application was created for learning SDL and how to create shaped window.
The image of cat was taken from book SDL Game Development of Shaun Ross Mitchell

## Requirements

* SDL2 2.0.8
* SDL2 2.0.2 image extentions

## Build

```
cmake ./
make
```

## SDL 2.0.8 Patch note

SDL has no function to reset shape assigned to window, when a shape is changed its value
combined with previous shape. The only way I found is call SDL_ResizeWindow and bitmaks for
shaper is cleaned inside SDL library. If your shape changed every 100ms then a window of
application is blinked randomly and it looks awful.
I add SDL_ResetWindowShape function which  reset shaper bitmaks without resizing a window
and there is no blinking window.
