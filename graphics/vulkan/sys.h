#ifndef SYS_H
#define SYS_H

#include <GLFW/glfw3.h>
#include <time.h>
#include "../logger.h"
#include "vulkan_types.h"

#define TARGET_FPS 60
#define TARGET_FRAME_TIME_MS (1000 / TARGET_FPS)
#define WIDTH 800
#define HEIGHT 600



extern GLFWWindowContext window;
extern clock_t lastTime;
extern int frameCount;
extern float fps;

void sys_create_window(GLFWwindow** window, int width, int height);
bool sys_process_events();
void sys_start_time();
void sys_control_frameRate();
float sys_platform_get_elapsed_time();
void sys_destroy_window(void* window);

enum KeyState {
    KEY_RELEASED = 0,
    KEY_PRESSED = 1,
    STATE_UNKNOWN = -1
};

enum KeyNames {
    KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M,
    KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,
    KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,
    KEY_ESCAPE, KEY_ENTER, KEY_SPACE, KEY_BACKSPACE, KEY_TAB, KEY_SHIFT, KEY_CTRL, KEY_ALT,
    KEY_ARROW_UP, KEY_ARROW_DOWN, KEY_ARROW_LEFT, KEY_ARROW_RIGHT, KEY_UNKNOWN
};

enum KeyNames sys_get_key();
enum KeyState sys_get_key_state(enum KeyNames key);

void sys_get_mouse_position(double* x, double* y);
enum KeyState sys_get_mouse_button_state(int button);

#endif