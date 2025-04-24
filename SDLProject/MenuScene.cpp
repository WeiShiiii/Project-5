/**
* Author: Wei Shi
* Assignment: A Way Out
* Date due: 04.25, 2:00pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "MenuScene.h"
#include "Utility.h"
#include <SDL.h>
#include "glm/gtc/matrix_transform.hpp"

constexpr char FONT_TEXTURE_PATH[] = "assets/font1.png";

MenuScene::~MenuScene() {
}

void MenuScene::initialise() {
    m_game_state.next_scene_id = -1;
}

void MenuScene::update(float delta_time) {
    const Uint8* keys = SDL_GetKeyboardState(NULL);
    if (keys[SDL_SCANCODE_RETURN]) {
        m_game_state.next_scene_id = 1;
        
    }
}

void MenuScene::render(ShaderProgram *program) {
    GLuint font_texture = Utility::load_texture(FONT_TEXTURE_PATH);
    
    Utility::draw_text(program, font_texture, "A Way Out", 0.5f, -0.05f, glm::vec3(-4.0f, 2.0f, 0.0f));
    
    Utility::draw_text(program, font_texture, "Press enter to start", 0.25f, -0.03f, glm::vec3(-3.5f, 0.0f, 0.0f));
}
