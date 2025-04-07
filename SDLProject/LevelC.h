#include "Scene.h"

class LevelC : public Scene {
public:
    // Increase enemy count for added difficulty
    int ENEMY_COUNT = 2;
    
    ~LevelC();
    
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};
