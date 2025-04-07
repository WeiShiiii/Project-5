/**
* Author: Wei Shi
* Assignment: Rise of the AI
* Date due: 2025-04-05, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "LevelC.h"
#include "Utility.h"

#define LEVEL_WIDTH 20
#define LEVEL_HEIGHT 10
#define ENEMY_COUNT 2

extern int g_lives;

unsigned int LEVELC_DATA[] = {
    3, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,
    3, 0,0,0,0,0, 1,1,1,1, 0,0,0,0,0,0,0,0,0,3,
    3, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,
    3, 0,0,0,0,0,0,0,0,0,0,0, 1,1,1,1, 0,0,0,3,
    3, 0,0,0, 1,1,1,1, 0,0,0,0,0,0,0,0,0,0,0,3,
    3, 1,1,1, 0,0,0,0,0,0,0,0, 1,1,1, 0,0,0,0,3,
    3, 0,0,0,0, 1,1,1,1,1,1,1,1,1,1, 0,0,0,0,3,
    3, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,
    3, 0,0,0,0,0,0,0, 1,1,1,1, 0,0,0,0,0,0,0,3,
};

constexpr char SPRITESHEET_FILEPATH[] = "assets/move.png";
constexpr char ENEMY_FILEPATH[] = "assets/enemy.png";

LevelC::~LevelC()
{
    delete[] m_game_state.enemies;
    delete m_game_state.player;
    delete m_game_state.map;
    Mix_FreeChunk(m_game_state.jump_sfx);
    Mix_FreeChunk(m_game_state.land_sfx);
    Mix_FreeChunk(m_game_state.death_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

void LevelC::initialise()
{
    m_game_state.next_scene_id = -1;

    GLuint map_texture_id = Utility::load_texture("assets/tile.png");
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELC_DATA, map_texture_id, 1.0f, 4, 1);

    int player_anim[4][4] = {
        {4, 5, 6, 7},
        {8, 9, 10, 11},
        {12, 13, 14, 15},
        {0, 1, 2, 3}
    };

    glm::vec3 acceleration = glm::vec3(0.0f, -4.81f, 0.0f);
    GLuint player_texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);

    m_game_state.player = new Entity(
        player_texture_id,
        5.0f,
        acceleration,
        5.0f,
        player_anim,
        0.0f,
        4,
        0,
        4,
        4,
        1.0f,
        1.0f,
        PLAYER
    );
    m_game_state.player->set_position(glm::vec3(3.0f, 0.0f, 0.0f));

    // Set up enemies
    m_game_state.enemies = new Entity[ENEMY_COUNT];

    int enemy_anim[4][4] = {
        {0, 1, 2, 3},
        {0, 1, 2, 3},
        {0, 1, 2, 3},
        {0, 1, 2, 3}
    };

    // GUARD
    m_game_state.enemies[0] = Entity(
        Utility::load_texture(ENEMY_FILEPATH),
        1.0f,
        1.0f,
        1.0f,
        ENEMY, GUARD, IDLE
    );
    m_game_state.enemies[0].set_position(glm::vec3(5.0f, -0.5f, 0.0f));
    m_game_state.enemies[0].set_movement(glm::vec3(0.0f));
    m_game_state.enemies[0].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    m_game_state.enemies[0].set_walking(enemy_anim);

    // FLYER
    m_game_state.enemies[1] = Entity(
        Utility::load_texture(ENEMY_FILEPATH),
        1.0f,
        1.0f,
        1.0f,
        ENEMY, FLYER, IDLE
    );
    m_game_state.enemies[1].set_position(glm::vec3(12.0f, -2.0f, 0.0f));
    m_game_state.enemies[1].set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
    m_game_state.enemies[1].set_walking(enemy_anim);
    
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    m_game_state.bgm = Mix_LoadMUS("assets/bgm.wav");
    Mix_PlayMusic(m_game_state.bgm, -1);
    Mix_VolumeMusic(20);

    m_game_state.jump_sfx = Mix_LoadWAV("assets/jumpland.wav");
    m_game_state.land_sfx = Mix_LoadWAV("assets/land.wav");
    m_game_state.death_sfx = Mix_LoadWAV("assets/blomp.mp3");

}

void LevelC::update(float delta_time)
{
    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, ENEMY_COUNT, m_game_state.map);

    for (int i = 0; i < ENEMY_COUNT; i++) {
        m_game_state.enemies[i].update(delta_time, m_game_state.player, nullptr, 0, m_game_state.map);
    }
    
    if (m_game_state.player->check_collision_with_enemies(m_game_state.enemies, ENEMY_COUNT)) {
        g_lives -= 1;
        Mix_PlayChannel(-1, m_game_state.death_sfx, 0);
        if (g_lives <= 0) {
            m_game_state.next_scene_id = -1;
            return;
        } else {
            m_game_state.player->set_position(glm::vec3(5.0f, 0.0f, 0.0f));
        }
    }
}

void LevelC::render(ShaderProgram* program)
{
    m_game_state.map->render(program);
    m_game_state.player->render(program);

    for (int i = 0; i < ENEMY_COUNT; i++) {
        m_game_state.enemies[i].render(program);
    }
}
