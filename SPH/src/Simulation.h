#pragma once
#ifndef SIMULATION_H
#define SIMULATION_H

#include <vector>
#include <string>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <entt/entt.hpp>

#include "GLSL.h"
#include "Program.h"

class Entity;
class MatrixStack;

class Simulation {
    friend class Entity;

    public:
        Simulation();
        Simulation(Simulation const&);
        void operator=(Simulation const&);
        ~Simulation();

        void create_window(const char * window_name);
        void init_programs();
        void init_cameras();
        void set_scene();
        void render_scene();
        void move_camera();
        void fixed_timestep_update();
        void resize_window();
        void swap_buffers();
        bool window_closed();
        void input_capture();
        void look_around();         

        static Simulation& get_instance() {
            static Simulation instance; 
            return instance;
        }

        static void error_callback(int error, const char *description){
            get_instance().error_callback_impl(error, description);
        }

        Entity create_entity(std::string tag);

        
    private:
        void update();        
        void integrate(float h);
        void draw_entities(MatrixStack& MV, MatrixStack& P);
        void error_callback_impl(int error, const char *description);
        
        float   dt = 1.0f/64.0f, 
                current_time, 
                total_time, 
                new_time, 
                frame_time, 
                eps = 0.01f;

        glm::vec3   lightPos = glm::vec3(0.0f, 30.0f, 0.0f),
                    gravity = glm::vec3(0.0f, -9.0f, 0.0f),
                    wind = glm::vec3(1.0f, 0.0f, 1.0f);

        double  o_x= -1.0, 
                o_y= -1.0;                                    
        bool options[256];                 

        GLFWwindow *window;   

        Program pbr_program, fluid_program;

        entt::registry registry;      
};


#endif