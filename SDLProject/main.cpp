/**
* Author: Wei Shi
* Assignment: A Way Out
* Date due: 04.25, 2:00pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 8
#define LEVEL1_LEFT_EDGE 5.0f

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"
#include "Map.h"
#include "Utility.h"
#include "Scene.h"
#include "MenuScene.h"
#include "WinScene.h"
#include "LoseScene.h"
#include "LevelA.h"
#include "LevelB.h"
#include "LevelC.h"
#include "Effects.h"

// ––––– CONSTANTS ––––– //
constexpr int WINDOW_WIDTH  = 640,
          WINDOW_HEIGHT = 480;

constexpr float BG_RED     = 0.1922f,
            BG_BLUE    = 0.549f,
            BG_GREEN   = 0.9059f,
            BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

enum AppStatus { RUNNING, TERMINATED };

// ––––– GLOBAL VARIABLES ––––– //
Scene  *g_current_scene;
MenuScene *g_menu = new MenuScene();
WinScene *g_win = new WinScene();
LoseScene *g_lose = new LoseScene();

LevelA *g_levelA;
LevelB *g_levelB;
LevelC *g_levelC;

Effects *g_effects;
Scene   *g_levels[4];

SDL_Window* g_display_window;


ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

Mix_Chunk* g_rock_break_sfx = nullptr;

float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;

float g_elapsed_time = 0.0f;

bool g_is_colliding_bottom = false;
bool g_game_paused = false;

bool  g_bright_mode  = false;
float g_bright_timer = 0.0f;
int time_bright = 4;

GLuint fontTexture = 0;

int g_lives = 3;
bool wantRockShake = false;

AppStatus g_app_status = RUNNING;

void swtich_to_scene(Scene *scene);
void initialise();
void process_input();
void update();
void render();
void shutdown();

// ––––– GENERAL FUNCTIONS ––––– //
void switch_to_scene(Scene *scene)
{    
    g_current_scene = scene;
    g_current_scene->initialise(); // DON'T FORGET THIS STEP!
}

void initialise()
{

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow("Hello, FIND A WAY OUT!",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    fontTexture = Utility::load_texture("assets/font1.png");
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);
    
    glUseProgram(g_shader_program.get_program_id());
    
    glClearColor(0, 0, 0, BG_OPACITY);
    
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    g_rock_break_sfx = Mix_LoadWAV("assets/rock_breaking.flac");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    g_levelA = new LevelA();
    g_levelB = new LevelB();
    g_levelC = new LevelC();
    
    
    g_levels[0] = g_menu;
    g_levels[1] = g_levelA;
    g_levels[2] = g_levelB;
    g_levels[3] = g_levelC;
    
    
    // Start at level A
    switch_to_scene(g_levels[0]);
    
    g_effects = new Effects(g_projection_matrix, g_view_matrix);
}

void process_input() {

    if (g_current_scene->get_state().player) {
        g_current_scene->get_state().player->set_movement(glm::vec3(0.0f));
    }
    
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_app_status = TERMINATED;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        g_app_status = TERMINATED;
                        break;
                    case SDLK_SPACE:
                        // Only jump if the player exists and is on the ground
                        if (g_current_scene->get_state().player &&
                            g_current_scene->get_state().player->get_collided_bottom()) {
                            g_current_scene->get_state().player->jump();
                            Mix_PlayChannel(-1, g_current_scene->get_state().jump_sfx, 0);
                        }
                        break;
                    case SDLK_p:  // 'P' pauses the game
                        g_game_paused = true;
                        break;
                    case SDLK_r:  // 'R' resumes the game
                        g_game_paused = false;
                        break;

                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
    
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);
    if (g_current_scene->get_state().player) {
        if (key_state[SDL_SCANCODE_LEFT])
            g_current_scene->get_state().player->move_left();
        else if (key_state[SDL_SCANCODE_RIGHT])
            g_current_scene->get_state().player->move_right();
        else if (key_state[SDL_SCANCODE_UP])
            g_current_scene->get_state().player->move_up();
        else if (key_state[SDL_SCANCODE_DOWN])
            g_current_scene->get_state().player->move_down();

        if (glm::length(g_current_scene->get_state().player->get_movement()) > 1.0f)
            g_current_scene->get_state().player->normalise_movement();
    }
    if (key_state[SDL_SCANCODE_C] && !g_bright_mode && time_bright > 0) {
        g_bright_mode  = true;
        g_bright_timer = 2.0f;      // 2 seconds of full brightness
        Mix_PlayChannel(-1, g_rock_break_sfx, 0);
        wantRockShake = true;
        time_bright--;
    }

}


void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    
    g_effects->update(delta_time);
    if (wantRockShake) {
        g_effects->start(SHAKE, 1.0f);
        wantRockShake = false;
    }
    
    if (g_bright_mode) {
        g_bright_timer -= delta_time;
        if (g_bright_timer <= 0.0f) {
            g_bright_mode = false;
        }
    }
    g_elapsed_time += FIXED_TIMESTEP;


    if ((g_current_scene == g_levelA || g_current_scene == g_levelB || g_current_scene == g_levelC)
        && g_elapsed_time >= 300.0f)
    {
        switch_to_scene(g_lose);
    }
    
    
    delta_time += g_accumulator;
    
    if (delta_time < FIXED_TIMESTEP)
    {
        g_accumulator = delta_time;
        return;
    }
    

    if (g_game_paused) return;
    while (delta_time >= FIXED_TIMESTEP) {
        g_current_scene->update(FIXED_TIMESTEP);
        g_effects->update(FIXED_TIMESTEP);
        
        if (g_lives <= 0) {
                switch_to_scene(g_lose);
        }
        
        if (g_is_colliding_bottom == false && g_current_scene->get_state().player &&
            g_current_scene->get_state().player->get_collided_bottom()) {
            g_effects->start(SHAKE, 1.0f);
            Mix_PlayChannel(-1, g_current_scene->get_state().land_sfx, 0);
        }
        if (g_current_scene->get_state().player) {
            g_is_colliding_bottom = g_current_scene->get_state().player->get_collided_bottom();
        }
        
        delta_time -= FIXED_TIMESTEP;
    }
    
    g_accumulator = delta_time;
    
    g_view_matrix = glm::mat4(1.0f);
    
    if (g_current_scene->get_state().player) {
        if (g_current_scene->get_state().player->get_position().x > LEVEL1_LEFT_EDGE) {
            g_view_matrix = glm::translate(glm::mat4(1.0f),
                                           glm::vec3(-g_current_scene->get_state().player->get_position().x, 3.75, 0));
        } else {
            g_view_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(-5, 3.75, 0));
        }
    }
    

    if (g_current_scene->get_state().player) {
        if (g_current_scene == g_levelA && g_current_scene->get_state().player->get_position().y < -20.0f)
            switch_to_scene(g_levelB);
    
        if (g_current_scene == g_levelB && g_current_scene->get_state().player->get_position().y < -20.0f)
            switch_to_scene(g_levelC);
        
        if (g_current_scene == g_levelC && g_current_scene->get_state().player->get_position().y < -20.0f)
            switch_to_scene(g_win);
        
    }
    g_view_matrix = glm::translate(g_view_matrix, g_effects->get_view_offset());


}


void render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    
    glUseProgram(g_shader_program.get_program_id());
        g_shader_program.set_view_matrix(g_view_matrix);

    bool hasPlayer = (g_current_scene->get_state().player != nullptr);

       if (!hasPlayer || g_bright_mode) {
           g_shader_program.set_isC(0);
       } else {
           g_shader_program.set_isC(1);

           glm::vec3 pos = g_current_scene->get_state().player->get_position();
           g_shader_program.set_light_position_matrix(pos);
       }
    g_effects->start(SHAKE, 1.0f);
        g_effects->render();
 
        g_current_scene->render(&g_shader_program);
       
    
    std::string livesText = "Lives: " + std::to_string(g_lives);
    Utility::draw_text(
        &g_shader_program,
        fontTexture,
        livesText,
        0.3f,
        -0.1f,
        glm::vec3(-4.5f, 3.5f, 0.0f)
    );

    float remaining = std::max(0.0f, 300.0f - g_elapsed_time);
    int m = int(remaining) / 60, s = int(remaining) % 60;
    std::ostringstream ss;
    ss << "Time: " << m << ":" << (s < 10 ? "0" : "") << s;
    Utility::draw_text(
        &g_shader_program,
        fontTexture,
        ss.str(),
        0.3f,
        -0.1f,
        glm::vec3(2.5f, 3.5f, 0.0f)
    );
    
    
        SDL_GL_SwapWindow(g_display_window);
       
}

void shutdown()
{    
    SDL_Quit();
    
    delete g_levelA;
    delete g_levelB;
    delete g_effects;
}

// ––––– DRIVER GAME LOOP ––––– //
int main(int argc, char* argv[])
{
    initialise();
    
    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        
        if (g_current_scene->get_state().next_scene_id >= 0) switch_to_scene(g_levels[g_current_scene->get_state().next_scene_id]);
        
        render();
    }
    
    shutdown();
    return 0;
}
