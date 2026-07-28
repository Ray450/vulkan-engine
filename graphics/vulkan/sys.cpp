#include "sys.h"

GLFWWindowContext window;
clock_t lastTime = 0;
int frameCount = 0;
float fps = 0.0f;

static void sys_frame_buffer_resize_callback(GLFWwindow* window, int width, int height) {
    //framebufferResized = true;
}
void sys_create_window(GLFWwindow** window, int width, int height) {
    if (!glfwInit()) {
        LOG_FATAL("Failed to initialize GLFW");
        return;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    *window = glfwCreateWindow(width, height, "Vulkan", NULL, NULL);
    if (!*window) {
        const char* msg = "Failed to create GLFW window";
        glfwTerminate();
        LOG_FATAL("GLFW_CHECK failed: %s", msg);
        return;
    }

    glfwSetFramebufferSizeCallback(*window, sys_frame_buffer_resize_callback);

    LOG_INFO("GLFW window created successfully");
    return;
}

bool sys_process_events() {
        if (glfwWindowShouldClose(context.window)) {
            return false;
        }
        
        glfwPollEvents();

    return true;
}

#include <unistd.h>

clock_t startTime;

void sys_start_time() {
    startTime = clock();
}

void sys_controlFrameRate() {
    clock_t endTime = clock();
    int frameTime = (int)((endTime - startTime) * 1000 / CLOCKS_PER_SEC);  // Convert to milliseconds
    int sleepTime = TARGET_FRAME_TIME_MS - frameTime;
    if (sleepTime > 0) {
        sleep(sleepTime);
    }
}

float sys_platform_get_elapsed_time() {
    clock_t currentTime = clock();
    return (float)(currentTime - startTime) / CLOCKS_PER_SEC;
}

enum KeyState sys_get_key_state(enum KeyNames key) {
    int glfwKey = -1;
    
    switch (key) {
        case KEY_A: glfwKey = GLFW_KEY_A; break;
        case KEY_B: glfwKey = GLFW_KEY_B; break;
        case KEY_C: glfwKey = GLFW_KEY_C; break;
        case KEY_D: glfwKey = GLFW_KEY_D; break;
        case KEY_E: glfwKey = GLFW_KEY_E; break;
        case KEY_F: glfwKey = GLFW_KEY_F; break;
        case KEY_G: glfwKey = GLFW_KEY_G; break;
        case KEY_H: glfwKey = GLFW_KEY_H; break;
        case KEY_I: glfwKey = GLFW_KEY_I; break;
        case KEY_J: glfwKey = GLFW_KEY_J; break;
        case KEY_K: glfwKey = GLFW_KEY_K; break;
        case KEY_L: glfwKey = GLFW_KEY_L; break;
        case KEY_M: glfwKey = GLFW_KEY_M; break;
        case KEY_N: glfwKey = GLFW_KEY_N; break;
        case KEY_O: glfwKey = GLFW_KEY_O; break;
        case KEY_P: glfwKey = GLFW_KEY_P; break;
        case KEY_Q: glfwKey = GLFW_KEY_Q; break;
        case KEY_R: glfwKey = GLFW_KEY_R; break;
        case KEY_S: glfwKey = GLFW_KEY_S; break;
        case KEY_T: glfwKey = GLFW_KEY_T; break;
        case KEY_U: glfwKey = GLFW_KEY_U; break;
        case KEY_V: glfwKey = GLFW_KEY_V; break;
        case KEY_W: glfwKey = GLFW_KEY_W; break;
        case KEY_X: glfwKey = GLFW_KEY_X; break;
        case KEY_Y: glfwKey = GLFW_KEY_Y; break;
        case KEY_Z: glfwKey = GLFW_KEY_Z; break;

        case KEY_0: glfwKey = GLFW_KEY_0; break;
        case KEY_1: glfwKey = GLFW_KEY_1; break;
        case KEY_2: glfwKey = GLFW_KEY_2; break;
        case KEY_3: glfwKey = GLFW_KEY_3; break;
        case KEY_4: glfwKey = GLFW_KEY_4; break;
        case KEY_5: glfwKey = GLFW_KEY_5; break;
        case KEY_6: glfwKey = GLFW_KEY_6; break;
        case KEY_7: glfwKey = GLFW_KEY_7; break;
        case KEY_8: glfwKey = GLFW_KEY_8; break;
        case KEY_9: glfwKey = GLFW_KEY_9; break;

        case KEY_ESCAPE: glfwKey = GLFW_KEY_ESCAPE; break;
        case KEY_ENTER: glfwKey = GLFW_KEY_ENTER; break;
        case KEY_SPACE: glfwKey = GLFW_KEY_SPACE; break;
        case KEY_BACKSPACE: glfwKey = GLFW_KEY_BACKSPACE; break;
        case KEY_TAB: glfwKey = GLFW_KEY_TAB; break;
        case KEY_SHIFT: glfwKey = GLFW_KEY_LEFT_SHIFT; break;
        case KEY_CTRL: glfwKey = GLFW_KEY_LEFT_CONTROL; break;
        case KEY_ALT: glfwKey = GLFW_KEY_LEFT_ALT; break;

        case KEY_ARROW_UP: glfwKey = GLFW_KEY_UP; break;
        case KEY_ARROW_DOWN: glfwKey = GLFW_KEY_DOWN; break;
        case KEY_ARROW_LEFT: glfwKey = GLFW_KEY_LEFT; break;
        case KEY_ARROW_RIGHT: glfwKey = GLFW_KEY_RIGHT; break;

        default: return STATE_UNKNOWN;
    }

    return glfwGetKey(context.window, glfwKey) == GLFW_PRESS ? KEY_PRESSED : KEY_RELEASED;
}

enum KeyNames sys_get_key() {
    if (glfwGetKey(context.window, GLFW_KEY_A) == GLFW_PRESS) return KEY_A;
    if (glfwGetKey(context.window, GLFW_KEY_B) == GLFW_PRESS) return KEY_B;
    if (glfwGetKey(context.window, GLFW_KEY_C) == GLFW_PRESS) return KEY_C;
    if (glfwGetKey(context.window, GLFW_KEY_D) == GLFW_PRESS) return KEY_D;
    if (glfwGetKey(context.window, GLFW_KEY_E) == GLFW_PRESS) return KEY_E;
    if (glfwGetKey(context.window, GLFW_KEY_F) == GLFW_PRESS) return KEY_F;
    if (glfwGetKey(context.window, GLFW_KEY_G) == GLFW_PRESS) return KEY_G;
    if (glfwGetKey(context.window, GLFW_KEY_H) == GLFW_PRESS) return KEY_H;
    if (glfwGetKey(context.window, GLFW_KEY_I) == GLFW_PRESS) return KEY_I;
    if (glfwGetKey(context.window, GLFW_KEY_J) == GLFW_PRESS) return KEY_J;
    if (glfwGetKey(context.window, GLFW_KEY_K) == GLFW_PRESS) return KEY_K;
    if (glfwGetKey(context.window, GLFW_KEY_L) == GLFW_PRESS) return KEY_L;
    if (glfwGetKey(context.window, GLFW_KEY_M) == GLFW_PRESS) return KEY_M;
    if (glfwGetKey(context.window, GLFW_KEY_N) == GLFW_PRESS) return KEY_N;
    if (glfwGetKey(context.window, GLFW_KEY_O) == GLFW_PRESS) return KEY_O;
    if (glfwGetKey(context.window, GLFW_KEY_P) == GLFW_PRESS) return KEY_P;
    if (glfwGetKey(context.window, GLFW_KEY_Q) == GLFW_PRESS) return KEY_Q;
    if (glfwGetKey(context.window, GLFW_KEY_R) == GLFW_PRESS) return KEY_R;
    if (glfwGetKey(context.window, GLFW_KEY_S) == GLFW_PRESS) return KEY_S;
    if (glfwGetKey(context.window, GLFW_KEY_T) == GLFW_PRESS) return KEY_T;
    if (glfwGetKey(context.window, GLFW_KEY_U) == GLFW_PRESS) return KEY_U;
    if (glfwGetKey(context.window, GLFW_KEY_V) == GLFW_PRESS) return KEY_V;
    if (glfwGetKey(context.window, GLFW_KEY_W) == GLFW_PRESS) return KEY_W;
    if (glfwGetKey(context.window, GLFW_KEY_X) == GLFW_PRESS) return KEY_X;
    if (glfwGetKey(context.window, GLFW_KEY_Y) == GLFW_PRESS) return KEY_Y;
    if (glfwGetKey(context.window, GLFW_KEY_Z) == GLFW_PRESS) return KEY_Z;

    if (glfwGetKey(context.window, GLFW_KEY_0) == GLFW_PRESS) return KEY_0;
    if (glfwGetKey(context.window, GLFW_KEY_1) == GLFW_PRESS) return KEY_1;
    if (glfwGetKey(context.window, GLFW_KEY_2) == GLFW_PRESS) return KEY_2;
    if (glfwGetKey(context.window, GLFW_KEY_3) == GLFW_PRESS) return KEY_3;
    if (glfwGetKey(context.window, GLFW_KEY_4) == GLFW_PRESS) return KEY_4;
    if (glfwGetKey(context.window, GLFW_KEY_5) == GLFW_PRESS) return KEY_5;
    if (glfwGetKey(context.window, GLFW_KEY_6) == GLFW_PRESS) return KEY_6;
    if (glfwGetKey(context.window, GLFW_KEY_7) == GLFW_PRESS) return KEY_7;
    if (glfwGetKey(context.window, GLFW_KEY_8) == GLFW_PRESS) return KEY_8;
    if (glfwGetKey(context.window, GLFW_KEY_9) == GLFW_PRESS) return KEY_9;

    if (glfwGetKey(context.window, GLFW_KEY_ESCAPE) == GLFW_PRESS) return KEY_ESCAPE;
    if (glfwGetKey(context.window, GLFW_KEY_ENTER) == GLFW_PRESS) return KEY_ENTER;
    if (glfwGetKey(context.window, GLFW_KEY_SPACE) == GLFW_PRESS) return KEY_SPACE;
    if (glfwGetKey(context.window, GLFW_KEY_BACKSPACE) == GLFW_PRESS) return KEY_BACKSPACE;
    if (glfwGetKey(context.window, GLFW_KEY_TAB) == GLFW_PRESS) return KEY_TAB;
    if (glfwGetKey(context.window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) return KEY_SHIFT;
    if (glfwGetKey(context.window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) return KEY_CTRL;
    if (glfwGetKey(context.window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) return KEY_ALT;

    if (glfwGetKey(context.window, GLFW_KEY_UP) == GLFW_PRESS) return KEY_ARROW_UP;
    if (glfwGetKey(context.window, GLFW_KEY_DOWN) == GLFW_PRESS) return KEY_ARROW_DOWN;
    if (glfwGetKey(context.window, GLFW_KEY_LEFT) == GLFW_PRESS) return KEY_ARROW_LEFT;
    if (glfwGetKey(context.window, GLFW_KEY_RIGHT) == GLFW_PRESS) return KEY_ARROW_RIGHT;

    return KEY_UNKNOWN;
}

void sys_destroy_window(void* window) {
    GLFWwindow* glfwWindow = (GLFWwindow*)window;
    glfwDestroyWindow(glfwWindow);
    glfwTerminate();
}

void sys_get_mouse_position(double* x, double* y) {
    glfwGetCursorPos(context.window, x, y);
}

enum KeyState sys_get_mouse_button_state(int button) {
    return glfwGetMouseButton(context.window, button) == GLFW_PRESS ? KEY_PRESSED : KEY_RELEASED;
}