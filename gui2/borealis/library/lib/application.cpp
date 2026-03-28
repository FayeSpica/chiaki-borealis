/*
    Borealis, a Nintendo Switch UI Library
    Copyright (C) 2019-2020  natinusala
    Copyright (C) 2019  p-sam
    Copyright (C) 2020  WerWolv

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <algorithm>
#include <borealis.hpp>
#include <string>

#ifdef __SDL2__
#include <SDL.h>
#ifdef ANDROID
#include <GLES3/gl3.h>
#define NANOVG_GLES3_IMPLEMENTATION
#else
#include <glad/glad.h>
#define NANOVG_GL3_IMPLEMENTATION
#endif
#else
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#define NANOVG_GL3_IMPLEMENTATION
#endif

#define GLM_FORCE_PURE
#define GLM_ENABLE_EXPERIMENTAL
#include <nanovg/nanovg.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <nanovg/nanovg_gl.h>

#ifdef __SWITCH__
#include <switch.h>
#endif

#include <chrono>
#include <set>
#include <thread>

// Constants used for scaling as well as
// creating a window of the right size on PC
constexpr uint32_t WINDOW_WIDTH  = 1280;
constexpr uint32_t WINDOW_HEIGHT = 720;

#define DEFAULT_FPS 60
#define BUTTON_REPEAT_DELAY 15
#define BUTTON_REPEAT_CADENCY 5

using namespace brls::i18n::literals;

namespace brls
{

#ifdef __SDL2__

// SDL2 backend

static void sdlWindowSizeChanged(int width, int height)
{
    if (!width || !height)
        return;

    glViewport(0, 0, width, height);
    Application::windowScale = (float)width / (float)WINDOW_WIDTH;

    float contentHeight = ((float)height / (Application::windowScale * (float)WINDOW_HEIGHT)) * (float)WINDOW_HEIGHT;

    Application::contentWidth  = WINDOW_WIDTH;
    Application::contentHeight = (unsigned)roundf(contentHeight);

    Application::resizeNotificationManager();

    Logger::info("Window size changed to {}x{}", width, height);
    Logger::info("New scale factor is {}", Application::windowScale);
}

// Returns the borealis button index for a given SDL keycode, or -1
static int sdlKeyToButton(SDL_Keycode key)
{
    switch (key)
    {
        case SDLK_LEFT:      return BRLS_GAMEPAD_BUTTON_DPAD_LEFT;
        case SDLK_RIGHT:     return BRLS_GAMEPAD_BUTTON_DPAD_RIGHT;
        case SDLK_UP:        return BRLS_GAMEPAD_BUTTON_DPAD_UP;
        case SDLK_DOWN:      return BRLS_GAMEPAD_BUTTON_DPAD_DOWN;
        case SDLK_RETURN:    return BRLS_GAMEPAD_BUTTON_A;
        case SDLK_BACKSPACE: return BRLS_GAMEPAD_BUTTON_B;
        case SDLK_ESCAPE:    return BRLS_GAMEPAD_BUTTON_START;
        case SDLK_AC_BACK:   return BRLS_GAMEPAD_BUTTON_B;
        case SDLK_F1:        return BRLS_GAMEPAD_BUTTON_BACK;
        case SDLK_l:         return BRLS_GAMEPAD_BUTTON_LEFT_BUMPER;
        case SDLK_r:         return BRLS_GAMEPAD_BUTTON_RIGHT_BUMPER;
        default:             return -1;
    }
}

static void sdlReadController(BrlsGamepadState& gamepad, SDL_GameController* controller)
{
    if (!controller) return;

    // Buttons
    gamepad.buttons[BRLS_GAMEPAD_BUTTON_A]            = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A) ? BRLS_PRESS : BRLS_RELEASE;
    gamepad.buttons[BRLS_GAMEPAD_BUTTON_B]            = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B) ? BRLS_PRESS : BRLS_RELEASE;
    gamepad.buttons[BRLS_GAMEPAD_BUTTON_X]            = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_X) ? BRLS_PRESS : BRLS_RELEASE;
    gamepad.buttons[BRLS_GAMEPAD_BUTTON_Y]            = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_Y) ? BRLS_PRESS : BRLS_RELEASE;
    gamepad.buttons[BRLS_GAMEPAD_BUTTON_LEFT_BUMPER]  = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER) ? BRLS_PRESS : BRLS_RELEASE;
    gamepad.buttons[BRLS_GAMEPAD_BUTTON_RIGHT_BUMPER] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) ? BRLS_PRESS : BRLS_RELEASE;
    gamepad.buttons[BRLS_GAMEPAD_BUTTON_BACK]         = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_BACK) ? BRLS_PRESS : BRLS_RELEASE;
    gamepad.buttons[BRLS_GAMEPAD_BUTTON_START]        = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_START) ? BRLS_PRESS : BRLS_RELEASE;
    gamepad.buttons[BRLS_GAMEPAD_BUTTON_GUIDE]        = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_GUIDE) ? BRLS_PRESS : BRLS_RELEASE;
    gamepad.buttons[BRLS_GAMEPAD_BUTTON_LEFT_THUMB]   = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSTICK) ? BRLS_PRESS : BRLS_RELEASE;
    gamepad.buttons[BRLS_GAMEPAD_BUTTON_RIGHT_THUMB]  = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSTICK) ? BRLS_PRESS : BRLS_RELEASE;
    gamepad.buttons[BRLS_GAMEPAD_BUTTON_DPAD_UP]      = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP) ? BRLS_PRESS : BRLS_RELEASE;
    gamepad.buttons[BRLS_GAMEPAD_BUTTON_DPAD_DOWN]    = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN) ? BRLS_PRESS : BRLS_RELEASE;
    gamepad.buttons[BRLS_GAMEPAD_BUTTON_DPAD_LEFT]    = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT) ? BRLS_PRESS : BRLS_RELEASE;
    gamepad.buttons[BRLS_GAMEPAD_BUTTON_DPAD_RIGHT]   = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) ? BRLS_PRESS : BRLS_RELEASE;
}

bool Application::init(std::string title, Style* style, LibraryViewsThemeVariantsWrapper* themeVariantsWrapper)
{
    // Init rng
    std::srand(std::time(nullptr));

    // Init managers
    Application::taskManager         = new TaskManager();
    Application::notificationManager = new NotificationManager();

    // Init static variables
    Application::currentFocus = nullptr;
    Application::oldGamepad   = {};
    Application::gamepad      = {};
    Application::title        = title;
    Application::shouldClose  = false;

    // Init theme and style
    if (!themeVariantsWrapper)
        themeVariantsWrapper = new LibraryViewsThemeVariantsWrapper(new HorizonLightTheme(), new HorizonDarkTheme());

    if (!style)
        style = new HorizonStyle();

    Application::currentThemeVariantsWrapper = themeVariantsWrapper;
    Application::currentStyle                = style;

    // Init SDL2
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER) < 0)
    {
        Logger::error("Failed to initialize SDL: {}", SDL_GetError());
        return false;
    }

#ifdef ANDROID
    // Android: use OpenGL ES 3.0
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_SetHint(SDL_HINT_ANDROID_TRAP_BACK_BUTTON, "1");
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
#else
    // Desktop: use OpenGL Core 3.2+
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#ifdef __APPLE__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#endif
#endif

    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // Create window
    Uint32 windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;
#ifdef ANDROID
    windowFlags |= SDL_WINDOW_FULLSCREEN;
#else
    windowFlags |= SDL_WINDOW_RESIZABLE;
#endif

    Application::window = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        windowFlags);

    if (!window)
    {
        Logger::error("SDL: failed to create window: {}", SDL_GetError());
        SDL_Quit();
        return false;
    }

    // Create GL context
    Application::glContext = SDL_GL_CreateContext(window);
    if (!glContext)
    {
        Logger::error("SDL: failed to create GL context: {}", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    SDL_GL_MakeCurrent(window, glContext);
    SDL_GL_SetSwapInterval(1);

#ifndef ANDROID
    // Load OpenGL routines using glad (not needed on Android with GLES3)
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        Logger::error("Failed to initialize glad");
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }
#endif

    Logger::info("GL Vendor: {}", (const char*)glGetString(GL_VENDOR));
    Logger::info("GL Renderer: {}", (const char*)glGetString(GL_RENDERER));
    Logger::info("GL Version: {}", (const char*)glGetString(GL_VERSION));

    // Open first game controller
    for (int i = 0; i < SDL_NumJoysticks(); i++)
    {
        if (SDL_IsGameController(i))
        {
            Application::sdlController = SDL_GameControllerOpen(i);
            if (sdlController)
            {
                Logger::info("Gamepad detected: {}", SDL_GameControllerName(sdlController));
                break;
            }
        }
    }

    // Initialize nanovg
#ifdef ANDROID
    Application::vg = nvgCreateGLES3(NVG_STENCIL_STROKES | NVG_ANTIALIAS);
#else
    Application::vg = nvgCreateGL3(NVG_STENCIL_STROKES | NVG_ANTIALIAS);
#endif
    if (!vg)
    {
        Logger::error("Unable to init nanovg");
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    int windowW, windowH;
    SDL_GL_GetDrawableSize(window, &windowW, &windowH);
    sdlWindowSizeChanged(windowW, windowH);

    // Load fonts
#ifdef ANDROID
    // On Android use assets path
    if (access(BOREALIS_ASSET("inter/Inter-Switch.ttf"), F_OK) != -1)
        Application::fontStash.regular = Application::loadFont("regular", BOREALIS_ASSET("inter/Inter-Switch.ttf"));

    if (Application::fontStash.regular == -1)
        brls::Logger::warning("Couldn't load regular font, no text will be displayed!");
#else
    // Use illegal font if available
    if (access(BOREALIS_ASSET("Illegal-Font.ttf"), F_OK) != -1)
        Application::fontStash.regular = Application::loadFont("regular", BOREALIS_ASSET("Illegal-Font.ttf"));
    else
        Application::fontStash.regular = Application::loadFont("regular", BOREALIS_ASSET("inter/Inter-Switch.ttf"));

    if (Application::fontStash.regular == -1)
        brls::Logger::warning("Couldn't load regular font, no text will be displayed!");

    if (access(BOREALIS_ASSET("Wingdings.ttf"), F_OK) != -1)
        Application::fontStash.sharedSymbols = Application::loadFont("sharedSymbols", BOREALIS_ASSET("Wingdings.ttf"));
#endif

    // Material font
    if (access(BOREALIS_ASSET("material/MaterialIcons-Regular.ttf"), F_OK) != -1)
        Application::fontStash.material = Application::loadFont("material", BOREALIS_ASSET("material/MaterialIcons-Regular.ttf"));

    // Set symbols font as fallback
    if (Application::fontStash.sharedSymbols)
    {
        Logger::info("Using shared symbols font");
        nvgAddFallbackFontId(Application::vg, Application::fontStash.regular, Application::fontStash.sharedSymbols);
    }

    // Set Material as fallback
    if (Application::fontStash.material)
    {
        Logger::info("Using Material font");
        nvgAddFallbackFontId(Application::vg, Application::fontStash.regular, Application::fontStash.material);
    }
    else
    {
        Logger::warning("Material font not found");
    }

    // Load theme - default to dark on Android TV
#ifdef ANDROID
    Application::currentThemeVariant = ThemeVariant::DARK;
#else
    char* themeEnv = getenv("BOREALIS_THEME");
    if (themeEnv != nullptr && !strcasecmp(themeEnv, "DARK"))
        Application::currentThemeVariant = ThemeVariant::DARK;
    else
        Application::currentThemeVariant = ThemeVariant::LIGHT;
#endif

    // Init window size
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    Application::windowWidth  = viewport[2];
    Application::windowHeight = viewport[3];

    // Init animations engine
    menu_animation_init();

    // Default FPS cap
    Application::setMaximumFPS(DEFAULT_FPS);

    return true;
}

bool Application::mainLoop()
{
    // Frame start
    retro_time_t frameStart = 0;
    if (Application::frameTime > 0.0f)
        frameStart = cpu_features_get_time_usec();

    // SDL events
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                Application::shouldClose = true;
                break;

            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED ||
                    event.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    int w, h;
                    SDL_GL_GetDrawableSize(Application::window, &w, &h);
                    sdlWindowSizeChanged(w, h);
                }
                break;

            case SDL_KEYDOWN:
                if (Application::textInputActive)
                {
                    // RETURN/ENTER: ignore from soft keyboard, only dialog OK button confirms
                    // BACK/ESCAPE: cancel input
                    if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_AC_BACK)
                        Application::stopTextInput(false);
                    else if (event.key.keysym.sym == SDLK_BACKSPACE && !Application::textInputBuffer.empty())
                    {
                        Application::textInputBuffer.pop_back();
                        Application::updateTextInputDialog();
                    }
                }
                else
                {
                    // Directly trigger borealis button press from key event
                    // (polling via SDL_GetKeyboardState misses short key presses on Android)
                    int btn = sdlKeyToButton(event.key.keysym.sym);
                    if (btn >= 0)
                        Application::onGamepadButtonPressed(btn, false);
                }
                break;

            case SDL_TEXTINPUT:
                if (Application::textInputActive)
                {
                    if ((int)Application::textInputBuffer.length() < Application::textInputMaxLength)
                    {
                        Application::textInputBuffer += event.text.text;
                        Application::updateTextInputDialog();
                    }
                }
                break;

            case SDL_CONTROLLERBUTTONDOWN:
                if (Application::textInputActive)
                {
                    // B button cancels text input
                    if (event.cbutton.button == SDL_CONTROLLER_BUTTON_B ||
                        event.cbutton.button == SDL_CONTROLLER_BUTTON_BACK)
                        Application::stopTextInput(false);
                    // A button confirms text input
                    else if (event.cbutton.button == SDL_CONTROLLER_BUTTON_A ||
                             event.cbutton.button == SDL_CONTROLLER_BUTTON_START)
                        Application::stopTextInput(true);
                }
                break;

            case SDL_CONTROLLERDEVICEADDED:
                if (!Application::sdlController)
                {
                    Application::sdlController = SDL_GameControllerOpen(event.cdevice.which);
                    if (sdlController)
                        Logger::info("Controller connected: {}", SDL_GameControllerName(sdlController));
                }
                break;

            case SDL_CONTROLLERDEVICEREMOVED:
                if (Application::sdlController &&
                    event.cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(sdlController)))
                {
                    SDL_GameControllerClose(Application::sdlController);
                    Application::sdlController = nullptr;
                    Logger::info("Controller disconnected");
                }
                break;

            default:
                break;
        }
    }

    if (Application::shouldClose)
    {
        Application::exit();
        return false;
    }

    // Skip controller/gamepad processing while text input is active
    if (Application::textInputActive)
    {
        // Still render frames
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        unsigned newWidth  = viewport[2];
        unsigned newHeight = viewport[3];
        if (Application::windowWidth != newWidth || Application::windowHeight != newHeight)
        {
            Application::windowWidth  = newWidth;
            Application::windowHeight = newHeight;
            Application::onWindowSizeChanged();
        }
        menu_animation_update();
        Application::taskManager->frame();
        Application::frame();
        SDL_GL_SwapWindow(window);
        if (Application::frameTime > 0.0f)
        {
            retro_time_t currentFrameTime = cpu_features_get_time_usec() - frameStart;
            retro_time_t ft = (retro_time_t)(Application::frameTime * 1000);
            if (ft > currentFrameTime)
                std::this_thread::sleep_for(std::chrono::microseconds(ft - currentFrameTime));
        }
        return true;
    }

    // Read controller state
    memset(&Application::gamepad, 0, sizeof(Application::gamepad));
    if (Application::sdlController)
    {
        sdlReadController(Application::gamepad, Application::sdlController);
    }
    else
    {
        // Keyboard fallback - read current key state
        const Uint8* keys = SDL_GetKeyboardState(nullptr);
        if (keys)
        {
            Application::gamepad.buttons[BRLS_GAMEPAD_BUTTON_DPAD_LEFT]    = keys[SDL_SCANCODE_LEFT]      ? BRLS_PRESS : BRLS_RELEASE;
            Application::gamepad.buttons[BRLS_GAMEPAD_BUTTON_DPAD_RIGHT]   = keys[SDL_SCANCODE_RIGHT]     ? BRLS_PRESS : BRLS_RELEASE;
            Application::gamepad.buttons[BRLS_GAMEPAD_BUTTON_DPAD_UP]      = keys[SDL_SCANCODE_UP]        ? BRLS_PRESS : BRLS_RELEASE;
            Application::gamepad.buttons[BRLS_GAMEPAD_BUTTON_DPAD_DOWN]    = keys[SDL_SCANCODE_DOWN]      ? BRLS_PRESS : BRLS_RELEASE;
            Application::gamepad.buttons[BRLS_GAMEPAD_BUTTON_A]            = keys[SDL_SCANCODE_RETURN]    ? BRLS_PRESS : BRLS_RELEASE;
            Application::gamepad.buttons[BRLS_GAMEPAD_BUTTON_B]            = keys[SDL_SCANCODE_BACKSPACE] ? BRLS_PRESS : BRLS_RELEASE;
            Application::gamepad.buttons[BRLS_GAMEPAD_BUTTON_START]        = keys[SDL_SCANCODE_ESCAPE]    ? BRLS_PRESS : BRLS_RELEASE;
            Application::gamepad.buttons[BRLS_GAMEPAD_BUTTON_BACK]         = keys[SDL_SCANCODE_F1]        ? BRLS_PRESS : BRLS_RELEASE;
            Application::gamepad.buttons[BRLS_GAMEPAD_BUTTON_LEFT_BUMPER]  = keys[SDL_SCANCODE_L]         ? BRLS_PRESS : BRLS_RELEASE;
            Application::gamepad.buttons[BRLS_GAMEPAD_BUTTON_RIGHT_BUMPER] = keys[SDL_SCANCODE_R]         ? BRLS_PRESS : BRLS_RELEASE;
        }
    }

    // Trigger gamepad events
    bool anyButtonPressed               = false;
    bool repeating                      = false;
    static retro_time_t buttonPressTime = 0;
    static int repeatingButtonTimer     = 0;

    for (int i = BRLS_GAMEPAD_BUTTON_A; i <= BRLS_GAMEPAD_BUTTON_LAST; i++)
    {
        if (Application::gamepad.buttons[i] == BRLS_PRESS)
        {
            anyButtonPressed = true;
            repeating        = (repeatingButtonTimer > BUTTON_REPEAT_DELAY && repeatingButtonTimer % BUTTON_REPEAT_CADENCY == 0);

            if (Application::oldGamepad.buttons[i] != BRLS_PRESS || repeating)
                Application::onGamepadButtonPressed(i, repeating);
        }

        if (Application::gamepad.buttons[i] != Application::oldGamepad.buttons[i])
            buttonPressTime = repeatingButtonTimer = 0;
    }

    if (anyButtonPressed && cpu_features_get_time_usec() - buttonPressTime > 1000)
    {
        buttonPressTime = cpu_features_get_time_usec();
        repeatingButtonTimer++;
    }

    Application::oldGamepad = Application::gamepad;

    // Handle window size changes
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    unsigned newWidth  = viewport[2];
    unsigned newHeight = viewport[3];

    if (Application::windowWidth != newWidth || Application::windowHeight != newHeight)
    {
        Application::windowWidth  = newWidth;
        Application::windowHeight = newHeight;
        Application::onWindowSizeChanged();
    }

    // Animations
    menu_animation_update();

    // Tasks
    Application::taskManager->frame();

    // Render
    Application::frame();
    SDL_GL_SwapWindow(window);

    // Sleep if necessary
    if (Application::frameTime > 0.0f)
    {
        retro_time_t currentFrameTime = cpu_features_get_time_usec() - frameStart;
        retro_time_t frameTime        = (retro_time_t)(Application::frameTime * 1000);

        if (frameTime > currentFrameTime)
        {
            retro_time_t toSleep = frameTime - currentFrameTime;
            std::this_thread::sleep_for(std::chrono::microseconds(toSleep));
        }
    }

    return true;
}

void Application::quit()
{
    Application::shouldClose = true;
}

void Application::startTextInput(const std::string& header, const std::string& initial, int maxLen, std::function<void(std::string)> cb)
{
    Application::textInputActive = true;
    Application::textInputBuffer = initial;
    Application::textInputHeader = header;
    Application::textInputMaxLength = maxLen;
    Application::textInputCallback = cb;

    // Build display text
    std::string displayText = header + "\n\n> " + initial + "_";

    auto* dialog = new Dialog(displayText);
    dialog->setCancelable(false);
    dialog->addButton("Cancel", [](View* view) {
        Application::stopTextInput(false);
    });
    dialog->addButton("OK", [](View* view) {
        Application::stopTextInput(true);
    });
    dialog->open();
    Application::textInputDialog = dialog;

    SDL_Rect rect = {0, 360, 1280, 360};
    SDL_SetTextInputRect(&rect);
    SDL_StartTextInput();

    Logger::info("Text input started: {}", header);
}

void Application::stopTextInput(bool submit)
{
    SDL_StopTextInput();
    Application::textInputActive = false;

    // Close the dialog
    if (Application::textInputDialog)
    {
        Application::textInputDialog->close();
        Application::textInputDialog = nullptr;
    }

    if (submit && Application::textInputCallback && !Application::textInputBuffer.empty())
    {
        auto cb = Application::textInputCallback;
        auto text = Application::textInputBuffer;
        Application::textInputCallback = nullptr;
        cb(text);
    }
    else
    {
        Application::textInputCallback = nullptr;
    }

    Logger::info("Text input stopped, submit={}", submit);
}

void Application::updateTextInputDialog()
{
    // Just notify with current text - don't recreate dialog
    // The dialog stays open, we show progress via notification
    Application::notify("> " + Application::textInputBuffer + "_");
}

#else // GLFW backend

// TODO: Use this instead of a glViewport each frame
static void windowFramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    if (!width || !height)
        return;

    glViewport(0, 0, width, height);
    Application::windowScale = (float)width / (float)WINDOW_WIDTH;

    float contentHeight = ((float)height / (Application::windowScale * (float)WINDOW_HEIGHT)) * (float)WINDOW_HEIGHT;

    Application::contentWidth  = WINDOW_WIDTH;
    Application::contentHeight = (unsigned)roundf(contentHeight);

    Application::resizeNotificationManager();

    Logger::info("Window size changed to {}x{}", width, height);
    Logger::info("New scale factor is {}", Application::windowScale);
}

static void joystickCallback(int jid, int event)
{
    if (event == GLFW_CONNECTED)
    {
        Logger::info("Joystick {} connected", jid);
        if (glfwJoystickIsGamepad(jid))
            Logger::info("Joystick {} is gamepad: \"{}\"", jid, glfwGetGamepadName(jid));
    }
    else if (event == GLFW_DISCONNECTED)
        Logger::info("Joystick {} disconnected", jid);
}

static void errorCallback(int errorCode, const char* description)
{
    Logger::error("[GLFW:{}] {}", errorCode, description);
}

static void windowKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        // Check for toggle-fullscreen combo
        if (key == GLFW_KEY_ENTER && mods == GLFW_MOD_ALT)
        {
            static int saved_x, saved_y, saved_width, saved_height;

            if (!glfwGetWindowMonitor(window))
            {
                glfwGetWindowPos(window, &saved_x, &saved_y);
                glfwGetWindowSize(window, &saved_width, &saved_height);

                GLFWmonitor* monitor    = glfwGetPrimaryMonitor();
                const GLFWvidmode* mode = glfwGetVideoMode(monitor);
                glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            }
            else
            {
                glfwSetWindowMonitor(window, nullptr, saved_x, saved_y, saved_width, saved_height, GLFW_DONT_CARE);
            }
        }
    }
}

bool Application::init(std::string title, Style* style, LibraryViewsThemeVariantsWrapper* themeVariantsWrapper)
{
    // Init rng
    std::srand(std::time(nullptr));

    // Init managers
    Application::taskManager         = new TaskManager();
    Application::notificationManager = new NotificationManager();

    // Init static variables
    Application::currentFocus = nullptr;
    Application::oldGamepad   = {};
    Application::gamepad      = {};
    Application::title        = title;

    // Init theme and style
    if (!themeVariantsWrapper)
        themeVariantsWrapper = new LibraryViewsThemeVariantsWrapper(new HorizonLightTheme(), new HorizonDarkTheme());

    if (!style)
        style = new HorizonStyle();

    Application::currentThemeVariantsWrapper = themeVariantsWrapper;
    Application::currentStyle                = style;

    // Init glfw
    glfwSetErrorCallback(errorCallback);
    glfwInitHint(GLFW_JOYSTICK_HAT_BUTTONS, GLFW_FALSE);
    if (!glfwInit())
    {
        Logger::error("Failed to initialize glfw");
        return false;
    }

    // Create window
#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

    GLFWmonitor* monitor    = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    Application::window = glfwCreateWindow(mode->width, mode->height, title.c_str(), nullptr, nullptr);
    if (!window)
    {
        Logger::error("glfw: failed to create window");
        glfwTerminate();
        return false;
    }

    glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, windowFramebufferSizeCallback);
    glfwSetKeyCallback(window, windowKeyCallback);
    glfwSetJoystickCallback(joystickCallback);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);

    Logger::info("GL Vendor: {}", glGetString(GL_VENDOR));
    Logger::info("GL Renderer: {}", glGetString(GL_RENDERER));
    Logger::info("GL Version: {}", glGetString(GL_VERSION));

    if (glfwJoystickIsGamepad(GLFW_JOYSTICK_1))
    {
        GLFWgamepadstate state;
        Logger::info("Gamepad detected: {}", glfwGetGamepadName(GLFW_JOYSTICK_1));
        glfwGetGamepadState(GLFW_JOYSTICK_1, &state);
    }

    Application::vg = nvgCreateGL3(NVG_STENCIL_STROKES | NVG_ANTIALIAS);
    if (!vg)
    {
        Logger::error("Unable to init nanovg");
        glfwTerminate();
        return false;
    }

    windowFramebufferSizeCallback(window, mode->width, mode->height);
    glfwSetTime(0.0);

    // Load fonts
#ifdef __SWITCH__
    {
        PlFontData font;

        Result rc = plGetSharedFontByType(&font, PlSharedFontType_Standard);
        if (R_SUCCEEDED(rc))
        {
            Logger::info("Using Switch shared font");
            Application::fontStash.regular = Application::loadFontFromMemory("regular", font.address, font.size, false);
        }

        rc = plGetSharedFontByType(&font, PlSharedFontType_KO);
        if (R_SUCCEEDED(rc))
        {
            Logger::info("Adding Switch shared Korean font");
            Application::fontStash.korean = Application::loadFontFromMemory("korean", font.address, font.size, false);
            nvgAddFallbackFontId(Application::vg, Application::fontStash.regular, Application::fontStash.korean);
        }

        rc = plGetSharedFontByType(&font, PlSharedFontType_NintendoExt);
        if (R_SUCCEEDED(rc))
        {
            Logger::info("Using Switch shared symbols font");
            Application::fontStash.sharedSymbols = Application::loadFontFromMemory("symbols", font.address, font.size, false);
        }
    }
#else
    if (access(BOREALIS_ASSET("Illegal-Font.ttf"), F_OK) != -1)
        Application::fontStash.regular = Application::loadFont("regular", BOREALIS_ASSET("Illegal-Font.ttf"));
    else
        Application::fontStash.regular = Application::loadFont("regular", BOREALIS_ASSET("inter/Inter-Switch.ttf"));

    if (Application::fontStash.regular == -1)
        brls::Logger::warning("Couldn't load regular font, no text will be displayed!");

    if (access(BOREALIS_ASSET("Wingdings.ttf"), F_OK) != -1)
        Application::fontStash.sharedSymbols = Application::loadFont("sharedSymbols", BOREALIS_ASSET("Wingdings.ttf"));
#endif

    if (access(BOREALIS_ASSET("material/MaterialIcons-Regular.ttf"), F_OK) != -1)
        Application::fontStash.material = Application::loadFont("material", BOREALIS_ASSET("material/MaterialIcons-Regular.ttf"));

    if (Application::fontStash.sharedSymbols)
    {
        Logger::info("Using shared symbols font");
        nvgAddFallbackFontId(Application::vg, Application::fontStash.regular, Application::fontStash.sharedSymbols);
    }
    else
    {
        Logger::warning("Shared symbols font not found");
    }

    if (Application::fontStash.material)
    {
        Logger::info("Using Material font");
        nvgAddFallbackFontId(Application::vg, Application::fontStash.regular, Application::fontStash.material);
    }
    else
    {
        Logger::warning("Material font not found");
    }

    // Load theme
#ifdef __SWITCH__
    ColorSetId nxTheme;
    setsysGetColorSetId(&nxTheme);

    if (nxTheme == ColorSetId_Dark)
        Application::currentThemeVariant = ThemeVariant::DARK;
    else
        Application::currentThemeVariant = ThemeVariant::LIGHT;
#else
    char* themeEnv = getenv("BOREALIS_THEME");
    if (themeEnv != nullptr && !strcasecmp(themeEnv, "DARK"))
        Application::currentThemeVariant = ThemeVariant::DARK;
    else
        Application::currentThemeVariant = ThemeVariant::LIGHT;
#endif

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    Application::windowWidth  = viewport[2];
    Application::windowHeight = viewport[3];

    menu_animation_init();
    Application::setMaximumFPS(DEFAULT_FPS);

    return true;
}

bool Application::mainLoop()
{
    retro_time_t frameStart = 0;
    if (Application::frameTime > 0.0f)
        frameStart = cpu_features_get_time_usec();

    bool is_active;
    do
    {
        is_active = !glfwGetWindowAttrib(Application::window, GLFW_ICONIFIED);
        if (is_active)
            glfwPollEvents();
        else
            glfwWaitEvents();

        if (glfwWindowShouldClose(Application::window))
        {
            Application::exit();
            return false;
        }
    } while (!is_active);

#ifdef __SWITCH__
    if (!appletMainLoop())
    {
        Application::exit();
        return false;
    }
#endif

    if (!glfwGetGamepadState(GLFW_JOYSTICK_1, &Application::gamepad))
    {
        Application::gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT]    = glfwGetKey(window, GLFW_KEY_LEFT);
        Application::gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT]   = glfwGetKey(window, GLFW_KEY_RIGHT);
        Application::gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP]      = glfwGetKey(window, GLFW_KEY_UP);
        Application::gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN]    = glfwGetKey(window, GLFW_KEY_DOWN);
        Application::gamepad.buttons[GLFW_GAMEPAD_BUTTON_START]        = glfwGetKey(window, GLFW_KEY_ESCAPE);
        Application::gamepad.buttons[GLFW_GAMEPAD_BUTTON_BACK]         = glfwGetKey(window, GLFW_KEY_F1);
        Application::gamepad.buttons[GLFW_GAMEPAD_BUTTON_A]            = glfwGetKey(window, GLFW_KEY_ENTER);
        Application::gamepad.buttons[GLFW_GAMEPAD_BUTTON_B]            = glfwGetKey(window, GLFW_KEY_BACKSPACE);
        Application::gamepad.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER]  = glfwGetKey(window, GLFW_KEY_L);
        Application::gamepad.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] = glfwGetKey(window, GLFW_KEY_R);
    }

    bool anyButtonPressed               = false;
    bool repeating                      = false;
    static retro_time_t buttonPressTime = 0;
    static int repeatingButtonTimer     = 0;

    for (int i = GLFW_GAMEPAD_BUTTON_A; i <= GLFW_GAMEPAD_BUTTON_LAST; i++)
    {
        if (Application::gamepad.buttons[i] == GLFW_PRESS)
        {
            anyButtonPressed = true;
            repeating        = (repeatingButtonTimer > BUTTON_REPEAT_DELAY && repeatingButtonTimer % BUTTON_REPEAT_CADENCY == 0);

            if (Application::oldGamepad.buttons[i] != GLFW_PRESS || repeating)
                Application::onGamepadButtonPressed(i, repeating);
        }

        if (Application::gamepad.buttons[i] != Application::oldGamepad.buttons[i])
            buttonPressTime = repeatingButtonTimer = 0;
    }

    if (anyButtonPressed && cpu_features_get_time_usec() - buttonPressTime > 1000)
    {
        buttonPressTime = cpu_features_get_time_usec();
        repeatingButtonTimer++;
    }

    Application::oldGamepad = Application::gamepad;

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    unsigned newWidth  = viewport[2];
    unsigned newHeight = viewport[3];

    if (Application::windowWidth != newWidth || Application::windowHeight != newHeight)
    {
        Application::windowWidth  = newWidth;
        Application::windowHeight = newHeight;
        Application::onWindowSizeChanged();
    }

    menu_animation_update();
    Application::taskManager->frame();
    Application::frame();
    glfwSwapBuffers(window);

    if (Application::frameTime > 0.0f)
    {
        retro_time_t currentFrameTime = cpu_features_get_time_usec() - frameStart;
        retro_time_t frameTime        = (retro_time_t)(Application::frameTime * 1000);

        if (frameTime > currentFrameTime)
        {
            retro_time_t toSleep = frameTime - currentFrameTime;
            std::this_thread::sleep_for(std::chrono::microseconds(toSleep));
        }
    }

    return true;
}

void Application::quit()
{
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

#endif // __SDL2__ / GLFW

// ===== Common code (shared by both backends) =====

void Application::navigate(FocusDirection direction)
{
    View* currentFocus = Application::currentFocus;

    if (!currentFocus || !currentFocus->hasParent())
        return;

    View* nextFocus = currentFocus->getParent()->getNextFocus(direction, currentFocus);

    while (!nextFocus)
    {
        if (!currentFocus->hasParent() || !currentFocus->getParent()->hasParent())
            break;

        currentFocus = currentFocus->getParent();
        nextFocus    = currentFocus->getParent()->getNextFocus(direction, currentFocus);
    }

    if (!nextFocus)
    {
        Application::currentFocus->shakeHighlight(direction);
        return;
    }

    Application::giveFocus(nextFocus);
}

void Application::onGamepadButtonPressed(char button, bool repeating)
{
    if (Application::blockInputsTokens != 0)
        return;

    if (repeating && Application::repetitionOldFocus == Application::currentFocus)
        return;

    Application::repetitionOldFocus = Application::currentFocus;

    if (Application::handleAction(button))
        return;

    // Navigation - button constants are the same layout between GLFW and BRLS
    switch (button)
    {
#ifdef __SDL2__
        case BRLS_GAMEPAD_BUTTON_DPAD_DOWN:
#else
        case GLFW_GAMEPAD_BUTTON_DPAD_DOWN:
#endif
            Application::navigate(FocusDirection::DOWN);
            break;
#ifdef __SDL2__
        case BRLS_GAMEPAD_BUTTON_DPAD_UP:
#else
        case GLFW_GAMEPAD_BUTTON_DPAD_UP:
#endif
            Application::navigate(FocusDirection::UP);
            break;
#ifdef __SDL2__
        case BRLS_GAMEPAD_BUTTON_DPAD_LEFT:
#else
        case GLFW_GAMEPAD_BUTTON_DPAD_LEFT:
#endif
            Application::navigate(FocusDirection::LEFT);
            break;
#ifdef __SDL2__
        case BRLS_GAMEPAD_BUTTON_DPAD_RIGHT:
#else
        case GLFW_GAMEPAD_BUTTON_DPAD_RIGHT:
#endif
            Application::navigate(FocusDirection::RIGHT);
            break;
        default:
            break;
    }
}

View* Application::getCurrentFocus()
{
    return Application::currentFocus;
}

bool Application::handleAction(char button)
{
    if (Application::viewStack.empty())
        return false;

    View* hintParent = Application::currentFocus;
    std::set<Key> consumedKeys;

    if (!hintParent)
        hintParent = Application::viewStack[Application::viewStack.size() - 1];

    while (hintParent)
    {
        for (auto& action : hintParent->getActions())
        {
            if (action.key != static_cast<Key>(button))
                continue;

            if (consumedKeys.find(action.key) != consumedKeys.end())
                continue;

            if (action.available)
                if (action.actionListener())
                    consumedKeys.insert(action.key);
        }

        hintParent = hintParent->getParent();
    }

    return !consumedKeys.empty();
}

void Application::frame()
{
    FrameContext frameContext = FrameContext();

    frameContext.pixelRatio = (float)Application::windowWidth / (float)Application::windowHeight;
    frameContext.vg         = Application::vg;
    frameContext.fontStash  = &Application::fontStash;
    frameContext.theme      = Application::getTheme();

    glClearColor(
        frameContext.theme->backgroundColor[0],
        frameContext.theme->backgroundColor[1],
        frameContext.theme->backgroundColor[2],
        1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    if (Application::background)
        Application::background->preFrame();

    nvgBeginFrame(Application::vg, Application::windowWidth, Application::windowHeight, frameContext.pixelRatio);
    nvgScale(Application::vg, Application::windowScale, Application::windowScale);

    std::vector<View*> viewsToDraw;

    for (size_t i = 0; i < Application::viewStack.size(); i++)
    {
        View* view = Application::viewStack[Application::viewStack.size() - 1 - i];
        viewsToDraw.push_back(view);

        if (!view->isTranslucent())
            break;
    }

    if (Application::background)
        Application::background->frame(&frameContext);

    for (size_t i = 0; i < viewsToDraw.size(); i++)
    {
        View* view = viewsToDraw[viewsToDraw.size() - 1 - i];
        view->frame(&frameContext);
    }

    if (Application::framerateCounter)
        Application::framerateCounter->frame(&frameContext);

    Application::notificationManager->frame(&frameContext);

    nvgResetTransform(Application::vg);
    nvgEndFrame(Application::vg);

    if (Application::background)
        Application::background->postFrame();
}

void Application::exit()
{
    Application::clear();

    if (Application::vg)
    {
#ifdef __SDL2__
#ifdef ANDROID
        nvgDeleteGLES3(Application::vg);
#else
        nvgDeleteGL3(Application::vg);
#endif
    }
    if (Application::sdlController)
        SDL_GameControllerClose(Application::sdlController);
    if (Application::glContext)
        SDL_GL_DeleteContext(Application::glContext);
    if (Application::window)
        SDL_DestroyWindow(Application::window);
    SDL_Quit();
#else
        nvgDeleteGL3(Application::vg);
    }
    glfwTerminate();
#endif

    menu_animation_free();

    if (Application::framerateCounter)
        delete Application::framerateCounter;

    delete Application::taskManager;
    delete Application::notificationManager;

    delete Application::currentThemeVariantsWrapper;
    delete Application::currentStyle;
}

void Application::setDisplayFramerate(bool enabled)
{
    if (!Application::framerateCounter && enabled)
    {
        Logger::debug("Enabling framerate counter");
        Application::framerateCounter = new FramerateCounter();
        Application::resizeFramerateCounter();
    }
    else if (Application::framerateCounter && !enabled)
    {
        Logger::debug("Disabling framerate counter");
        delete Application::framerateCounter;
        Application::framerateCounter = nullptr;
    }
}

void Application::toggleFramerateDisplay()
{
    Application::setDisplayFramerate(!Application::framerateCounter);
}

void Application::resizeFramerateCounter()
{
    if (!Application::framerateCounter)
        return;

    Style* style                   = Application::getStyle();
    unsigned framerateCounterWidth = style->FramerateCounter.width;
    unsigned width                 = WINDOW_WIDTH;

    Application::framerateCounter->setBoundaries(
        width - framerateCounterWidth,
        0,
        framerateCounterWidth,
        style->FramerateCounter.height);
    Application::framerateCounter->invalidate();
}

void Application::resizeNotificationManager()
{
    Application::notificationManager->setBoundaries(0, 0, Application::contentWidth, Application::contentHeight);
    Application::notificationManager->invalidate();
}

void Application::notify(std::string text)
{
    Application::notificationManager->notify(text);
}

NotificationManager* Application::getNotificationManager()
{
    return Application::notificationManager;
}

void Application::giveFocus(View* view)
{
    View* oldFocus = Application::currentFocus;
    View* newFocus = view ? view->getDefaultFocus() : nullptr;

    if (oldFocus != newFocus)
    {
        if (oldFocus)
            oldFocus->onFocusLost();

        Application::currentFocus = newFocus;
        Application::globalFocusChangeEvent.fire(newFocus);

        if (newFocus)
        {
            newFocus->onFocusGained();
            Logger::debug("Giving focus to {}", newFocus->describe());
        }
    }
}

void Application::popView(ViewAnimation animation, std::function<void(void)> cb)
{
    if (Application::viewStack.size() <= 1)
        return;

    Application::blockInputs();

    View* last = Application::viewStack[Application::viewStack.size() - 1];
    last->willDisappear(true);

    last->setForceTranslucent(true);

    bool wait = animation == ViewAnimation::FADE;

    last->hide([last, animation, wait, cb]() {
        last->setForceTranslucent(false);
        Application::viewStack.pop_back();
        delete last;

        if (Application::viewStack.size() > 0 && wait)
        {
            View* newLast = Application::viewStack[Application::viewStack.size() - 1];

            if (newLast->isHidden())
            {
                newLast->willAppear(false);
                newLast->show(cb, true, animation);
            }
            else
            {
                cb();
            }
        }

        Application::unblockInputs();
    },
        true, animation);

    if (!wait && Application::viewStack.size() > 1)
    {
        View* toShow = Application::viewStack[Application::viewStack.size() - 2];
        toShow->willAppear(false);
        toShow->show(cb, true, animation);
    }

    if (Application::focusStack.size() > 0)
    {
        View* newFocus = Application::focusStack[Application::focusStack.size() - 1];

        Logger::debug("Giving focus to {}, and removing it from the focus stack", newFocus->describe());

        Application::giveFocus(newFocus);
        Application::focusStack.pop_back();
    }
}

void Application::pushView(View* view, ViewAnimation animation)
{
    Application::blockInputs();

    View* last = nullptr;
    if (Application::viewStack.size() > 0)
        last = Application::viewStack[Application::viewStack.size() - 1];

    bool fadeOut = last && !last->isTranslucent() && !view->isTranslucent();
    bool wait    = animation == ViewAnimation::FADE;

    view->registerAction("brls/hints/exit"_i18n, Key::PLUS, [] { Application::quit(); return true; });
    view->registerAction(
        "FPS", Key::MINUS, [] { Application::toggleFramerateDisplay(); return true; }, true);

    if (fadeOut)
    {
        view->setForceTranslucent(true);

        if (!wait)
        {
            view->show([]() {
                Application::unblockInputs();
            },
                true, animation);
        }

        last->hide([animation, wait]() {
            View* newLast = Application::viewStack[Application::viewStack.size() - 1];
            newLast->setForceTranslucent(false);

            if (wait)
                newLast->show([]() { Application::unblockInputs(); }, true, animation);
        },
            true, animation);
    }

    view->setBoundaries(0, 0, Application::contentWidth, Application::contentHeight);

    if (!fadeOut)
        view->show([]() { Application::unblockInputs(); }, true, animation);
    else
        view->alpha = 0.0f;

    if (Application::viewStack.size() > 0 && Application::currentFocus != nullptr)
    {
        Logger::debug("Pushing {} to the focus stack", Application::currentFocus->describe());
        Application::focusStack.push_back(Application::currentFocus);
    }

    view->invalidate(true);
    view->willAppear(true);
    Application::giveFocus(view->getDefaultFocus());

    Application::viewStack.push_back(view);
}

void Application::onWindowSizeChanged()
{
    Logger::debug("Layout triggered");

    for (View* view : Application::viewStack)
    {
        view->setBoundaries(0, 0, Application::contentWidth, Application::contentHeight);
        view->invalidate();

        view->onWindowSizeChanged();
    }

    if (Application::background)
    {
        Application::background->setBoundaries(
            0,
            0,
            Application::contentWidth,
            Application::contentHeight);

        Application::background->invalidate();
        Application::background->onWindowSizeChanged();
    }

    Application::resizeNotificationManager();
    Application::resizeFramerateCounter();
}

void Application::clear()
{
    for (View* view : Application::viewStack)
    {
        view->willDisappear(true);
        delete view;
    }

    Application::viewStack.clear();
}

Style* Application::getStyle()
{
    return Application::currentStyle;
}

Theme* Application::getTheme()
{
    return Application::currentThemeVariantsWrapper->getTheme(Application::currentThemeVariant);
}

LibraryViewsThemeVariantsWrapper* Application::getThemeVariantsWrapper()
{
    return Application::currentThemeVariantsWrapper;
}

ThemeVariant Application::getThemeVariant()
{
    return Application::currentThemeVariant;
}

int Application::loadFont(const char* fontName, const char* filePath)
{
    return nvgCreateFont(Application::vg, fontName, filePath);
}

int Application::loadFontFromMemory(const char* fontName, void* address, size_t size, bool freeData)
{
    return nvgCreateFontMem(Application::vg, fontName, (unsigned char*)address, size, freeData);
}

int Application::findFont(const char* fontName)
{
    return nvgFindFont(Application::vg, fontName);
}

void Application::crash(std::string text)
{
    CrashFrame* crashFrame = new CrashFrame(text);
    Application::pushView(crashFrame);
}

void Application::blockInputs()
{
    Application::blockInputsTokens += 1;
}

void Application::unblockInputs()
{
    if (Application::blockInputsTokens > 0)
        Application::blockInputsTokens -= 1;
}

NVGcontext* Application::getNVGContext()
{
    return Application::vg;
}

TaskManager* Application::getTaskManager()
{
    return Application::taskManager;
}

void Application::setCommonFooter(std::string footer)
{
    Application::commonFooter = footer;
}

std::string* Application::getCommonFooter()
{
    return &Application::commonFooter;
}

FramerateCounter::FramerateCounter()
    : Label(LabelStyle::LIST_ITEM, "FPS: ---")
{
    this->setColor(nvgRGB(255, 255, 255));
    this->setVerticalAlign(NVG_ALIGN_MIDDLE);
    this->setHorizontalAlign(NVG_ALIGN_RIGHT);
    this->setBackground(ViewBackground::BACKDROP);

    this->lastSecond = cpu_features_get_time_usec() / 1000;
}

void FramerateCounter::frame(FrameContext* ctx)
{
    retro_time_t current = cpu_features_get_time_usec() / 1000;

    if (current - this->lastSecond >= 1000)
    {
        char fps[10];
        snprintf(fps, sizeof(fps), "FPS: %03d", this->frames);
        this->setText(std::string(fps));
        this->invalidate();

        this->frames     = 0;
        this->lastSecond = current;
    }

    this->frames++;

    Label::frame(ctx);
}

void Application::setMaximumFPS(unsigned fps)
{
    if (fps == 0)
        Application::frameTime = 0.0f;
    else
    {
        Application::frameTime = 1000 / (float)fps;
    }

    Logger::info("Maximum FPS set to {} - using a frame time of {:.2f} ms", fps, Application::frameTime);
}

std::string Application::getTitle()
{
    return Application::title;
}

GenericEvent* Application::getGlobalFocusChangeEvent()
{
    return &Application::globalFocusChangeEvent;
}

VoidEvent* Application::getGlobalHintsUpdateEvent()
{
    return &Application::globalHintsUpdateEvent;
}

FontStash* Application::getFontStash()
{
    return &Application::fontStash;
}

void Application::setBackground(Background* background)
{
    if (Application::background)
    {
        Application::background->willDisappear();
        delete Application::background;
    }

    Application::background = background;

    background->setBoundaries(0, 0, Application::contentWidth, Application::contentHeight);
    background->invalidate(true);
    background->willAppear(true);
}

void Application::cleanupNvgGlState()
{
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_STENCIL_TEST);
}

} // namespace brls
