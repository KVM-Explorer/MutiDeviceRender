#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include "Render.hpp"
//#include "test.hpp"

int main() {

    try {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);

        glm::mat4 matrix;
        glm::vec4 vec;
        auto test = matrix * vec;

        Render::init(window);

        auto vert = Render::createShaderModule("vert.spv");
        auto frag = Render::createShaderModule("frag.spv");
        Render::createPipeline(vert, frag);

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            Render::render();
        }
        Render::waitIdle();

        Render::quit();

        glfwDestroyWindow(window);

        glfwTerminate();
    }catch(std::exception e)
    {
        std::cout << e.what() << std::endl;
    }
    return 0;
}