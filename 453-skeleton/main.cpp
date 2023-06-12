#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <limits>
#include <functional>
#include <math.h>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"
#include "Camera.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"


// We gave this code in one of the tutorials, so leaving it here too
void updateGPUGeometry(GPU_Geometry &gpuGeom, CPU_Geometry const &cpuGeom) {
	gpuGeom.bind();
	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setCols(cpuGeom.cols);
	gpuGeom.setNormals(cpuGeom.normals);
	gpuGeom.setTexCoords(cpuGeom.texCoords);
}

struct PlanetObject {
	PlanetObject(std::string texturePath, GLenum textureInterpolation) :
		texture(texturePath, textureInterpolation),
		position(0.f, 0.f, 0.f),
		distance(0.f, 0.f, 0.f),
		distance1(0.f, 0.f, 0.f),
		axisRotation(0.f),
		axisTilt(0.f),
		orbitRotation(0.f),
		orbitTilt(0.f),
		orbitRotation1(0.f),
		orbitTilt1(0.f)
	{}

	// some parts taken from A2
	glm::mat4 getTransformationMatrix() const {
		glm::mat4 I(1.0f);
		
		glm::mat4 AR = glm::rotate(I, axisRotation, glm::vec3(0.f, 1.f, 0.f));
		glm::mat4 AT = glm::rotate(I, axisTilt, glm::vec3(0.f, 0.f, 1.f));

		glm::mat4 T = glm::translate(I, distance);
		glm::mat4 OR = glm::rotate(I, orbitRotation, glm::vec3(0.f, 1.f, 0.f));
		glm::mat4 OT = glm::rotate(I, orbitTilt, glm::vec3(0.f, 0.f, 1.f));

		// second orbit
		glm::mat4 T1 = glm::translate(I, distance1);
		glm::mat4 OR1 = glm::rotate(I, orbitRotation1, glm::vec3(0.f, 1.f, 0.f));
		glm::mat4 OT1 = glm::rotate(I, orbitTilt1, glm::vec3(0.f, 0.f, 1.f));
		
		return OT * OR * T * OT1 * OR1 * T1 * AT * AR;
	}

	CPU_Geometry transformedGeometry(bool isSun) {
		CPU_Geometry newGeom = cgeom;
		glm::mat4 T = getTransformationMatrix();
		cgeom = origin;
		position = T * glm::vec4(0.f, 0.f, 0.f, 1.f);

		for (int i = 0; i < newGeom.verts.size(); i++) {
			newGeom.verts[i] = T * glm::vec4(cgeom.verts[i], 1.f);

			// if the planet is the sun, then the normal vector should be facing outwards so light does not get trapped inside the sun
			if (isSun) {
				newGeom.normals[i] = glm::vec3(position - newGeom.verts[i]);
			}
			else {
				newGeom.normals[i] = glm::vec3(newGeom.verts[i] - position);
			}
		}
		return newGeom;
	}

	CPU_Geometry cgeom;
	GPU_Geometry ggeom;
	Texture texture;
	glm::vec3 position;
	glm::vec3 distance; glm::vec3 distance1;
	float axisRotation; float axisTilt;
	float orbitRotation; float orbitTilt; float orbitRotation1; float orbitTilt1;
	CPU_Geometry origin;
};

// EXAMPLE CALLBACKS
class Assignment4 : public CallbackInterface {

public:
	Assignment4() : camera(0.0, 70.0, 50.0), aspect(1.0f), rightDown(false), leftDown(false), pause(false), reset(false) {
	}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
			rightDown = true;
		}
		if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
			leftDown = true;
		}
		if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
			pause = true;
		}
		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			reset = true;
		}
	}
	virtual void mouseButtonCallback(int button, int action, int mods) {
		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (action == GLFW_PRESS) {
				rightMouseDown = true;
			} else if (action == GLFW_RELEASE) {
				rightMouseDown = false;
			}
		}
	}
	virtual void cursorPosCallback(double xpos, double ypos) {
		if (rightMouseDown) {
			double dx = xpos - mouseOldX;
			double dy = ypos - mouseOldY;
			camera.incrementTheta(dy);
			camera.incrementPhi(dx);
		}
		mouseOldX = xpos;
		mouseOldY = ypos;
	}
	virtual void scrollCallback(double xoffset, double yoffset) {
		camera.incrementR(yoffset);
	}
	virtual void windowSizeCallback(int width, int height) {
		// The CallbackInterface::windowSizeCallback will call glViewport for us
		CallbackInterface::windowSizeCallback(width,  height);
		aspect = float(width)/float(height);
	}

	void viewPipeline(ShaderProgram &sp) {
		glm::mat4 M = glm::mat4(1.0);
		glm::mat4 V = camera.getView();
		glm::mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.01f, 1000.f);

		GLint location = glGetUniformLocation(sp, "light");
		//glm::vec3 light = camera.getPos();
		glm::vec3 light = glm::vec3(0.f, 0.f, 0.f);
		glUniform3fv(location, 1, glm::value_ptr(light));

		GLint uniMat = glGetUniformLocation(sp, "M");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(M));
		uniMat = glGetUniformLocation(sp, "V");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(V));
		uniMat = glGetUniformLocation(sp, "P");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(P));
	}

	void refreshStatus() {
		rightDown = false;
		leftDown = false;
		pause = false;
		reset = false;
	}

	bool getRightDown() {
		return rightDown;
	}
	bool getLeftDown() {
		return leftDown;
	}
	bool getPause() {
		return pause;
	}
	bool getReset() {
		return reset;
	}

	Camera camera;

private:

	bool rightMouseDown = false;
	bool rightDown = false;
	bool leftDown = false;
	bool pause; bool reset;
	float aspect;
	double mouseOldX;
	double mouseOldY;

};

// generates a surface of revolution (basically creates the planet) using a given radius and degree
void generateSurfaceRev(CPU_Geometry& planet, float r, float d) {

	// generate half of the circle first, and keep track of the phi to be used for texture mapping
	std::vector<glm::vec3> halfCirc;
	std::vector<float> phi;
	for (float angle = 90.f; angle >= -90.f; angle -= d) {
		halfCirc.push_back(glm::vec3{ r * cos(glm::radians(angle)), r * sin(glm::radians(angle)), 0.f});
		phi.push_back(90.f - angle);
	}


	// generate the points of the surface revolution and texture mapping points
	std::vector<std::vector<glm::vec3>> surfaceRevList;
	std::vector<glm::vec3> temp;
	std::vector<std::vector<glm::vec2>> textList;
	std::vector<glm::vec2> temp1;

	for (int i = 0; i < halfCirc.size(); i++) {
		temp.clear();
		temp1.clear();

		for (float v = 0.f; v <= glm::radians(360.f); v += glm::radians(10.f)) {
			temp.push_back(glm::vec3(halfCirc[i].x * cos(v), halfCirc[i].y, halfCirc[i].x * sin(v)));
			// formula to calculate texture mapping
			temp1.push_back(glm::vec2(v / glm::radians(360.f), glm::radians(180.f - phi[i]) / glm::radians(180.f)));
		}
		// store the surface points and texture points
		surfaceRevList.push_back(temp);
		textList.push_back(temp1);

	}

	// then make triangles using the surface revolution points to be rendered later
	planet.verts.clear();
	planet.normals.clear();
	planet.cols.clear();
	planet.texCoords.clear();

	for (int i = 0; i < surfaceRevList.size() - 1; i++) {
		for (int j = 0; j < surfaceRevList[i].size() - 1; j++) {
			// VERTS
			// first triangle
			planet.verts.push_back(surfaceRevList[i][j + 1]);
			planet.verts.push_back(surfaceRevList[i][j]);
			planet.verts.push_back(surfaceRevList[i + 1][j]);
			//second triangle
			planet.verts.push_back(surfaceRevList[i][j + 1]);
			planet.verts.push_back(surfaceRevList[i + 1][j]);
			planet.verts.push_back(surfaceRevList[i + 1][j + 1]);

			// TEXTURES
			// first triangle
			planet.texCoords.push_back(textList[i][j + 1]);
			planet.texCoords.push_back(textList[i][j]);
			planet.texCoords.push_back(textList[i + 1][j]);
			//second triangle
			planet.texCoords.push_back(textList[i][j + 1]);
			planet.texCoords.push_back(textList[i + 1][j]);
			planet.texCoords.push_back(textList[i + 1][j + 1]);
		}
	}
	// since the planets are created in (0,0,0), their center is at 0
	// we can set the normal to be equal to verts because (position - 0) = position
	planet.normals = planet.verts;
}

// inverts the normal of a planet (used for the sun and stars to avoid trapping light)
void invertNormal(PlanetObject& planet) {
	for (int i = 0; i < planet.cgeom.normals.size(); i++) {
		planet.cgeom.normals[i] = -planet.cgeom.normals[i];
	}
}

int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453"); // can set callbacks at construction if desired


	//GLDebug::enable();

	// CALLBACKS
	auto a4 = std::make_shared<Assignment4>();
	window.setCallbacks(a4);

	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	//std::vector<glm::vec3> sunHalf = generateHalfCircle(6.f, 6.f);

	PlanetObject sun("textures/sunmap.jpg", GL_LINEAR);
	generateSurfaceRev(sun.cgeom, 2.5f, 6.f);
	invertNormal(sun);
	sun.origin = sun.cgeom;
	updateGPUGeometry(sun.ggeom, sun.cgeom);

	PlanetObject moon("textures/moonmap1.jpg", GL_LINEAR);
	generateSurfaceRev(moon.cgeom, 0.9f, 6.f);
	moon.origin = moon.cgeom;
	updateGPUGeometry(moon.ggeom, moon.cgeom);

	PlanetObject earth("textures/earthmap.jpg", GL_LINEAR);
	generateSurfaceRev(earth.cgeom, 1.5f, 6.f);
	earth.origin = earth.cgeom;
	updateGPUGeometry(earth.ggeom, earth.cgeom);

	PlanetObject stars("textures/stars.jpg", GL_LINEAR);
	generateSurfaceRev(stars.cgeom, 120.f, 10.f);
	invertNormal(stars);
	stars.origin = stars.cgeom;
	updateGPUGeometry(stars.ggeom, stars.cgeom);
	
	float lastTime = glfwGetTime();
	float rotation = 0.f;
	float fps = 60.f;
	float multiplier = 1.f;
	float animPause = false;
	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();

		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_FRAMEBUFFER_SRGB);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);


		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		shader.use();

		// KEYBOARD CONTROLS
		if (a4->getLeftDown() && !animPause) {
			multiplier -= 0.1f;
			if (multiplier <= 0.0f) multiplier = 0.1f;
		}
		if (a4->getRightDown() && !animPause) {
			multiplier += 0.1f;
		}
		if (a4->getPause()) {
			animPause = !animPause;
		}
		if (a4->getReset()) {
			rotation = 0.f;
			multiplier = 1.f;
		}
		a4->refreshStatus();
		// KEYBOARD CONTROLS END

		// ANIMATION CODE, updates depending on the set FPS
		if ((glfwGetTime() - lastTime >= (1.f/fps)) && !animPause) {
			// increase rotation every update
			rotation += 0.005f;
			if (rotation >= 360.f) rotation = 0.f;

			// sun transformation
			sun.axisRotation = (rotation * 2.f * multiplier); sun.axisTilt = .3f;
			sun.cgeom = sun.transformedGeometry(true);
			updateGPUGeometry(sun.ggeom, sun.cgeom);

			// orbit earth around sun
			earth.axisRotation = rotation * 2.f * multiplier; earth.axisTilt = .5f; earth.orbitRotation = rotation * .2 * multiplier; earth.orbitTilt = .3f;
			earth.distance = glm::vec3(0.f, 0.f, -24.f);
			earth.cgeom = earth.transformedGeometry(false);
			updateGPUGeometry(earth.ggeom, earth.cgeom);

			// orbit moon around the sun and earth
			moon.axisRotation = rotation * 2.f * multiplier; moon.axisTilt = .3f; moon.orbitRotation = rotation * .2 * multiplier; moon.orbitTilt = .3f;
			moon.distance = glm::vec3(0.f, 0.f, -24.f);
			moon.orbitRotation1 = rotation * 3.f * multiplier;  moon.orbitTilt1 = .25f; moon.distance1 = glm::vec3(0.f, 0.f, -5.f);
			moon.cgeom = moon.transformedGeometry(false);
			updateGPUGeometry(moon.ggeom, moon.cgeom);

			// update time
			lastTime = glfwGetTime();
		}

		a4->viewPipeline(shader);

		// render
		sun.ggeom.bind();
		sun.texture.bind();
		glDrawArrays(GL_TRIANGLES, 0, GLsizei(sun.cgeom.verts.size()));
		sun.texture.unbind();

		moon.ggeom.bind();
		moon.texture.bind();
		glDrawArrays(GL_TRIANGLES, 0, GLsizei(moon.cgeom.verts.size()));
		moon.texture.unbind();

		earth.ggeom.bind();
		earth.texture.bind();
		glDrawArrays(GL_TRIANGLES, 0, GLsizei(earth.cgeom.verts.size()));
		earth.texture.unbind();

		stars.ggeom.bind();
		stars.texture.bind();
		glDrawArrays(GL_TRIANGLES, 0, GLsizei(stars.cgeom.verts.size()));
		stars.texture.unbind();

		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		window.swapBuffers();
	}

	glfwTerminate();
	return 0;
}
