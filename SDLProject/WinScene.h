/**
* Author: Wei Shi
* Assignment: A Way Out
* Date due: 04.25, 2:00pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#pragma once
#include "Scene.h"

class WinScene : public Scene {
public:
    ~WinScene();
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};
