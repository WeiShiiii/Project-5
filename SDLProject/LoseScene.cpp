/**
* Author: Wei Shi
* Assignment: A Way Out
* Date due: 04.25, 2:00pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "LoseScene.h"
#include "Utility.h"
#include <SDL.h>
#include "glm/gtc/matrix_transform.hpp"

constexpr char FONT_TEXTURE_PATH[] = "assets/font1.png";

LoseScene::~LoseScene() {
}

void LoseScene::initialise() {
    m_game_state.next_scene_id = -1;
}

void LoseScene::update(float delta_time) {
}

void LoseScene::render(ShaderProgram *program) {
    GLuint font_texture = Utility::load_texture(FONT_TEXTURE_PATH);
    Utility::draw_text(program, font_texture, "You Lose!", 0.5f, -0.05f, glm::vec3(-3.0f, 2.0f, 0.0f));
}
