/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////// INCLUDES & DEFINES////////

#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////// GLOBAL VARS ////////

static glm::vec3* eyeVec;
static glm::vec3* lookVec;

static double xpos; 
static double ypos;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////// FUNCS ////////

class InputHandler {
public:
	static void setEyePos(glm::vec3 *ev)
	{
		eyeVec = ev;
	}

	static void setLookDir(glm::vec3 *lv)
	{
		lookVec = lv;
	}

	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_W && action == GLFW_PRESS)
			callback_W();
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
			callback_S();
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
			callback_A();
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
			callback_D();

		if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
			callback_Space();
		if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS)
			callback_LCtrl();

		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			callback_Esc(window);
	}

	static void cursor_position_callback(GLFWwindow* window, double newxpos, double newypos)
	{
		if (newxpos > xpos)
		{
			//move mouse right
			*lookVec = glm::rotate(*lookVec,
				glm::radians(-1.0f),
				glm::vec3(0.0f, 0.0f, 1.0f));
		}
		else if (newxpos < xpos)
		{
			//moved mouse left
			*lookVec = glm::rotate(*lookVec,
				glm::radians(1.0f),
				glm::vec3(0.0f, 0.0f, 1.0f));
		}

		if (newypos > ypos)
		{
			//moved mouse down
			*lookVec = glm::rotate(*lookVec,
				glm::radians(1.0f),
				glm::vec3(0.0f, 1.0f, 0.0f));
		}
		else if (newypos < ypos)
		{
			//moved mouse up
			*lookVec = glm::rotate(*lookVec,
				glm::radians(-1.0f),
				glm::vec3(0.0f, 1.0f, 0.0f));
		}

		glm::normalize(*lookVec);

		xpos = newxpos;
		ypos = newypos;
	}
private:
	static void callback_W()
	{
		//eyeVec->x += 1.0f;
		*eyeVec += *lookVec * 1.0f;
	}
	static void callback_S()
	{
		//eyeVec->x -= 1.0f;
		*eyeVec -= *lookVec * 1.0f;
	}
	static void callback_A()
	{
		//eyeVec->y += 1.0f;
	}
	static void callback_D()
	{
		//eyeVec->y -= 1.0f;
	}
	static void callback_Space()
	{
		//eyeVec->z += 1.0f;
	}
	static void callback_LCtrl()
	{
		//eyeVec->z -= 1.0f;
	}

	static void callback_Esc(GLFWwindow* window)
	{
		glfwSetWindowShouldClose(window, true); 
	}
};