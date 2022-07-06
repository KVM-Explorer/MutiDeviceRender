#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include "MultiRender.hpp"

int main() {

    MultiRender render;

    try {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);

        glm::mat4 matrix;
        glm::vec4 vec;
        auto test = matrix * vec;

       render.init(window);

        auto vert = render.createShaderModule("shaders/Common.vert.spv",0);
        auto frag = render.createShaderModule("shaders/Common.frag.spv",0);
        render.createCommonPipeline(vert, frag, 0);

        //vert = render.createShaderModule("shaders/Common.vert.spv", 1);
        //frag = render.createShaderModule("shaders/Common.frag.spv", 1);
        //render.createCommonPipeline(vert, frag, 1);
        //auto comp = Render::createShaderModule("shaders/RayTrace.comp.spv");
		
        //Render::createComputerPipeline(comp);

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            render.render();
        }
        render.waitIdle();

        render.release();

        glfwDestroyWindow(window);

        glfwTerminate();
    }
    catch (std::exception e)
    {
        std::cout << e.what() << std::endl;
    }
    return 0;
}