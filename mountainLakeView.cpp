#include <windows.h>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <irrklang/irrKlang.h>
#include <GL/gl.h>
#include <GL/glext.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui_impl_glut.h"

#include <iostream>
#include <string>
#include <vector>
#include "LoadMesh.h"
#include "LoadTexture.h"
#include "InitShader.h"
#include "FpCamera.h"
#include "Cube.h"
#include "mountainLakeView.h"

using namespace irrklang;
#define BUFFER_OFFSET(i)    ((char*)NULL + (i))

//shaders for mountain 4 shaders, including tess. control and tess. evaluation
static const std::string vertex_shader("tessellation_vs.glsl");
static const std::string tess_control_shader("tessellation_tc.glsl");
static const std::string tess_eval_shader("tessellation_te.glsl");
static const std::string fragment_shader("tessellation_fs.glsl");
GLuint shader_program = -1;

//shaders for water 4 shaders, including tess. control and tess. evaluation
static const std::string vertex_shader_w("tessellation_w_vs.glsl");
static const std::string tess_control_shader_w("tessellation_w_tc.glsl");
static const std::string tess_eval_shader_w("tessellation_w_te.glsl");
static const std::string fragment_shader_w("tessellation_w_fs.glsl");
GLuint shader_program_water = -1;

//shaders for tree 2 shaders
static const std::string vertex_shader_t("tessellation_t_vs.glsl");
static const std::string fragment_shader_t("tessellation_t_fs.glsl");
GLuint shader_program_tree = -1;

//Texture map for mountain
GLuint texture_id = -1;
static const std::string texture_name = "terrain_texture8k_unlit.dds";
//Texture map for height
GLuint height_texture_id = -1;
static const std::string height_texture_name = "height.png";
//Texture map for water
GLuint water_texture_id = -1;
static const std::string water_texture_name = "waterDUDV.png";
//Texture map for water
GLuint water_normal_id = -1;
static const std::string water_normal_name = "waternormal.png";
//Skycube
static const std::string cube_name = "cubemap";
GLuint cubemap_id = -1; //Texture id for cubemap
						//Cube files and IDs
static const std::string cube_vs("cube_vs.glsl");
static const std::string cube_fs("cube_fs.glsl");
GLuint cube_shader_program = -1;
GLuint cube_vao = -1;

//VAO and VBO for the tessellated patch mountain
GLuint patch_vao = -1;
GLuint patch_vbo = -1;

//VAO and VBO for the tessellated patch water
GLuint patch_w_vao = -1;
GLuint patch_w_vbo = -1;

bool wireframe = false;
//Number of terrain patches
const float patchX = 50.0f;
const float patchY = 50.0f;
const float patchScale = 10.0f;
const int numPatches = int(patchX*patchY);

int win_width = 1280;
int win_height = 720;
const float aspect_ratio = float(win_width) / float(win_height);

//frame buffer object
GLuint reflection_fbo;
GLuint refraction_fbo;
GLuint depth_fbo;

// Texture which we will render into
GLuint reflection_texture = -1;
GLuint refraction_texture = -1;
GLuint depth_texture = -1;

//tree mesh
static const std::string mesh_name = "tree.obj";
MeshData mesh_data;
//tree texture
GLuint tree_texture_id = -1;
static const std::string tree_texture_name = "wood_low.png";
//tree position
glm::vec3 treepos(0.5f, 0.5f, 0.5f);


void draw_gui()
{
	ImGui_ImplGlut_NewFrame();
	ImGui::Begin("Control Panel", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	const int n_sliders = 6;
	static float slider[n_sliders] = { 1.0f,1.0f,1.0f,1.0f,1.0f,1.0f };
	const std::string labels[n_sliders] = { "gl_TessLevelOuter[0]","gl_TessLevelOuter[1]","gl_TessLevelOuter[2]","gl_TessLevelOuter[3]","gl_TessLevelInner[0]", "gl_TessLevelInner[1]" };
	for (int i = 0; i<n_sliders; i++)
	{
		ImGui::SliderFloat(labels[i].c_str(), &slider[i], 1, 64);
	}

	ImGui::Checkbox("Wireframe", &wireframe);
	ImGui::Image((void*)reflection_texture, ImVec2(128.0f, 128.0f), ImVec2(0.0, 1.0), ImVec2(1.0, 0.0));
	ImGui::SameLine();
	ImGui::Image((void*)refraction_texture, ImVec2(128.0f, 128.0f), ImVec2(0.0, 1.0), ImVec2(1.0, 0.0));
	ImGui::SameLine();
	ImGui::Image((void*)depth_texture, ImVec2(128.0f, 128.0f), ImVec2(0.0, 1.0), ImVec2(1.0, 0.0));
	int slider_loc = glGetUniformLocation(shader_program, "slider");
	glUniform1fv(slider_loc, n_sliders, slider);
	glUniform1fv(10, n_sliders, slider);
	ImGui::End();

	ImGui::Render();
}

void drawSky(const glm::mat4 &V)
{
	glUseProgram(cube_shader_program);
	int PVM_loc = glGetUniformLocation(cube_shader_program, "PVM");
	if (PVM_loc != -1)
	{
		
		const float fov = 3.141592f / 2.0f;
		glm::mat4 P = glm::perspective(fov, aspect_ratio, 0.1f, 800.0f);
//		glm::mat4 V = GetViewMatrix()*glm::rotate(ang, glm::vec3(1.0f, 0.0f, 0.0f));
		

		glm::mat4 Msky = glm::scale(glm::vec3(2000.0f));
		glm::mat4 PVM = P*V*Msky;
		PVM[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		glUniformMatrix4fv(PVM_loc, 1, false, glm::value_ptr(PVM));
	}


	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_id);
	int cube_loc = glGetUniformLocation(cube_shader_program, "cubemap");
	if (cube_loc != -1)
	{
		glUniform1i(cube_loc, 1); // we bound our texture to texture unit 1
	}

	draw_cube_vao(cube_vao);
}

void drawMountain(const glm::mat4 &V)
{
	glUseProgram(shader_program);
	const int w = glutGet(GLUT_WINDOW_WIDTH);
	const int h = glutGet(GLUT_WINDOW_HEIGHT);
	const float aspect_ratio = float(w) / float(h);
	const float fov = 3.141592f / 2.0f;
	glm::mat4 P = glm::perspective(fov, aspect_ratio, 0.1f, 1000.0f);
	//glm::mat4 V = GetViewMatrix();
	glm::mat4 M = glm::scale(glm::vec3(patchScale));


	const int P_loc = 0;
	glUniformMatrix4fv(P_loc, 1, false, glm::value_ptr(P));
	const int V_loc = 1;
	glUniformMatrix4fv(V_loc, 1, false, glm::value_ptr(V));
	const int M_loc = 2;
	glUniformMatrix4fv(M_loc, 1, false, glm::value_ptr(M));


	//Mountain texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_id);

	GLuint tex_loc = glGetUniformLocation(shader_program, "tex");
	if (tex_loc != -1)
	{
		glUniform1i(tex_loc, 0);
	}
	//height map
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, height_texture_id);

	GLuint height_tex_loc = glGetUniformLocation(shader_program, "height");
	if (height_tex_loc != -1)
	{
		glUniform1i(height_tex_loc, 1);
	}

	//Draw mountain
	glBindVertexArray(patch_vao);
	glPatchParameteri(GL_PATCH_VERTICES, 4); //number of input verts to the tess. control shader per patch.
	if (wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //Draw wireframe so we can see the edges of generated triangles
	}

	glDrawArrays(GL_PATCHES, 0, numPatches * 4); //Draw patches since we are using a tessellation shader.

	glBindVertexArray(0);
}
void drawTree()
{
	glUseProgram(shader_program_tree);
	const int w = glutGet(GLUT_WINDOW_WIDTH);
	const int h = glutGet(GLUT_WINDOW_HEIGHT);
	const float aspect_ratio = float(w) / float(h);
	const float fov = 3.141592f / 2.0f;
	glm::mat4 P = glm::perspective(fov, aspect_ratio, 0.1f, 1000.0f);
	glm::mat4 V = GetViewMatrix();

	glm::mat4 T = glm::translate(treepos);
	glm::mat4 M = T*glm::scale(glm::vec3(1000.0f*mesh_data.mScaleFactor));

	const int P_loc = 0;
	glUniformMatrix4fv(P_loc, 1, false, glm::value_ptr(P));
	const int V_loc = 1;
	glUniformMatrix4fv(V_loc, 1, false, glm::value_ptr(V));
	const int M_loc = 2;
	glUniformMatrix4fv(M_loc, 1, false, glm::value_ptr(M));
	//tree texture
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, tree_texture_id);

	const int tree_tex_loc = 11;;
	glUniform1i(tree_tex_loc, 7);
	
	glBindVertexArray(mesh_data.mVao);
	glDrawElements(GL_TRIANGLES, mesh_data.mNumIndices, GL_UNSIGNED_INT, 0);
}
void drawWater()
{
	glUseProgram(shader_program_water);
	const int w = glutGet(GLUT_WINDOW_WIDTH);
	const int h = glutGet(GLUT_WINDOW_HEIGHT);
	const float aspect_ratio = float(w) / float(h);
	const float fov = 3.141592f / 2.0f;
	glm::mat4 P = glm::perspective(fov, aspect_ratio, 0.1f, 1000.0f);
	glm::mat4 V = GetViewMatrix();
	glm::mat4 M = glm::scale(glm::vec3(patchScale));

	const int P_loc = 0;
	glUniformMatrix4fv(P_loc, 1, false, glm::value_ptr(P));
	const int V_loc = 1;
	glUniformMatrix4fv(V_loc, 1, false, glm::value_ptr(V));
	const int M_loc = 2;
	glUniformMatrix4fv(M_loc, 1, false, glm::value_ptr(M));
	//water texture
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, water_texture_id);

	
	const int water_tex_loc = 5;;
	glUniform1i(water_tex_loc, 4);
	//water normal texture
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, water_normal_id);

	const int water_normal_loc = 6;;
	glUniform1i(water_normal_loc, 5);

	glBindVertexArray(patch_w_vao);
	glPatchParameteri(GL_PATCH_VERTICES, 4); //number of input verts to the tess. control shader per patch.

											 //	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //Draw wireframe so we can see the edges of generated triangles
	glDrawArrays(GL_PATCHES, 0, numPatches * 4); //Draw patches since we are using a tessellation shader.
	glBindVertexArray(0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
// glut display callback function.
// This function gets called every time the scene gets redisplayed 
void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//glUseProgram(shader_program);
	const int pass_loc = 6;
	glUniform1i(pass_loc, 1);
	glm::mat4 V_mountain = glm::scale(glm::vec3(1.0f, -1.0f, 1.0f))*GetViewMatrix();
	
	glBindFramebuffer(GL_FRAMEBUFFER, reflection_fbo); 
	//Make the viewport match the FBO texture size.
	glViewport(0, 0, win_width, win_height);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	//Clear the FBO attached texture.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Render to reflection FBO.
	float ang = 90.0;
	glm::mat4 V_sky = glm::scale(glm::vec3(1.0f, -1.0f, 1.0f))*GetViewMatrix()*glm::rotate(ang, glm::vec3(1.0f, 0.0f, 0.0f));
	drawSky(V_sky);
	drawMountain(V_mountain);
	
	//refraction-texture
	glEnable(GL_DEPTH_TEST);
	glUniform1i(pass_loc, 2);
	glBindFramebuffer(GL_FRAMEBUFFER, refraction_fbo);
	//glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
	//Make the viewport match the FBO texture size.
	glViewport(0, 0, win_width, win_height);
	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT1,GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(2, drawBuffers);

	// Render to refraction FBO.
	V_mountain = GetViewMatrix();
	//drawMountain(V_mountain);
	
	//Clear the FBO attached texture.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//// Render to reflection FBO.
	drawMountain(V_mountain);

	// Do not render the next pass to FBO.
	glUniform1i(pass_loc, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0); 
	glDrawBuffer(GL_BACK); // Render to back buffer.
	V_sky = GetViewMatrix()*glm::rotate(ang, glm::vec3(1.0f, 0.0f, 0.0f));
	drawSky(V_sky);
	drawTree();
	drawMountain(V_mountain);

	glUseProgram(shader_program_water);
	const int reflectiontex_loc = 4;
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, reflection_texture);
	glUniform1i(reflectiontex_loc, 0);

	const int refractiontex_loc = 7;
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, refraction_texture);
	glUniform1i(refractiontex_loc, 1);

	const int shadow_loc = 8;
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depth_texture);
	glUniform1i(shadow_loc, 3);

	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	drawWater();
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	
	glUseProgram(shader_program);
	
//	draw_gui();

	glutSwapBuffers();


}

void idle()
{
	glutPostRedisplay();

	const int time_ms = glutGet(GLUT_ELAPSED_TIME);
	float time_sec = 0.001f*time_ms;

	const int time_loc = 3;
	glUseProgram(shader_program);
	glUniform1f(time_loc, time_sec);

	glUseProgram(shader_program_water);
	glUniform1f(time_loc, time_sec);

	glUseProgram(cube_shader_program);
	glUniform1f(time_loc, time_sec);

}

void reload_shader()
{
	//Use the version of InitShader with 4 parameters. The shader names are in the order the stage are in the pipeline:
	//Vertex shader, Tess. control, Tess. evaluation, fragment shader
	GLuint new_shader = InitShader(vertex_shader.c_str(), tess_control_shader.c_str(), tess_eval_shader.c_str(), fragment_shader.c_str());
	GLuint new_shader_w = InitShader(vertex_shader_w.c_str(), tess_control_shader_w.c_str(), tess_eval_shader_w.c_str(), fragment_shader_w.c_str());

	if (new_shader == -1 || new_shader_w == -1) // loading failed
	{
		glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
	}
	else
	{
		glClearColor(0.35f, 0.35f, 0.35f, 0.0f);

		if (shader_program != -1 || shader_program_water != -1)
		{
			glDeleteProgram(shader_program);
			glDeleteProgram(shader_program_water);
		}
		shader_program = new_shader;
		shader_program_water = new_shader_w;

	}
}


void reshape(int w, int h)
{
	win_width = w;
	win_height = h;
	glViewport(0, 0, w, h);
}

// glut keyboard callback function.
// This function gets called when an ASCII key is pressed
void keyboard(unsigned char key, int x, int y)
{
	ImGui_ImplGlut_KeyCallback(key);
	//std::cout << "key : " << key << ", x: " << x << ", y: " << y << std::endl;
	CameraKeyboard(key);
	switch (key)
	{
	case 'r':
	case 'R':
		reload_shader();
		break;
	}
}

void printGlInfo()
{
	std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}

void initOpenGl()
{
	glewInit();

	glEnable(GL_DEPTH_TEST);

	reload_shader();
	glUseProgram(shader_program);
	texture_id = LoadTexture(texture_name.c_str());
	height_texture_id = LoadTexture(height_texture_name.c_str());
	

	std::vector<glm::vec3> vertices;

	//make a (patchX x patchY) grid of patches
	for (float x = 0.0f; x < patchX; x += 1.0f)
	{
		for (float y = 0.0f; y < patchY; y += 1.0f)
		{
			vertices.push_back(glm::vec3(x + 0.0f, y + 0.0f, 0.0f));
			vertices.push_back(glm::vec3(x + 1.0f, y + 0.0f, 0.0f));
			vertices.push_back(glm::vec3(x + 1.0f, y + 1.0f, 0.0f));
			vertices.push_back(glm::vec3(x + 0.0f, y + 1.0f, 0.0f));
		}
	}


	//mountain create vertex buffers for vertex coords
	glGenVertexArrays(1, &patch_vao);
	glBindVertexArray(patch_vao);
	glGenBuffers(1, &patch_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, patch_vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
	int pos_loc = glGetAttribLocation(shader_program, "pos_attrib");
	if (pos_loc >= 0)
	{
		glEnableVertexAttribArray(pos_loc);
		glVertexAttribPointer(pos_loc, 3, GL_FLOAT, false, 0, BUFFER_OFFSET(0));
	}

	std::vector<glm::vec3> vertices_w;

	//make a (patchX x patchY) grid of patches
	for (float x = 0.0f; x < patchX; x += 1.0f)
	{
		for (float y = 0.0f; y < patchY; y += 1.0f)
		{
			vertices_w.push_back(glm::vec3(x + 0.0f, y + 0.0f, 0.0f));
			vertices_w.push_back(glm::vec3(x + 1.0f, y + 0.0f, 0.0f));
			vertices_w.push_back(glm::vec3(x + 1.0f, y + 1.0f, 0.0f));
			vertices_w.push_back(glm::vec3(x + 0.0f, y + 1.0f, 0.0f));
		}
	}
	//tree
	shader_program_tree = InitShader(vertex_shader_t.c_str(), fragment_shader_t.c_str());
	glUseProgram(shader_program_tree);
	tree_texture_id = LoadTexture(tree_texture_name.c_str());
	mesh_data = LoadMesh(mesh_name);
	
	


	//water
	glUseProgram(shader_program_water);
	glGenVertexArrays(1, &patch_w_vao);
	glBindVertexArray(patch_w_vao);
	glGenBuffers(1, &patch_w_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, patch_w_vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices_w.size() * sizeof(glm::vec3), vertices_w.data(), GL_STATIC_DRAW);
	int pos_w_loc = glGetAttribLocation(shader_program_water, "pos_attrib");
	if (pos_w_loc >= 0)
	{
		glEnableVertexAttribArray(pos_w_loc);
		glVertexAttribPointer(pos_w_loc, 3, GL_FLOAT, false, 0, BUFFER_OFFSET(0));
	}
	water_texture_id = LoadTexture(water_texture_name.c_str());
	water_normal_id = LoadTexture(water_normal_name.c_str());
	//skycube
	cubemap_id = LoadCube(cube_name);
	cube_shader_program = InitShader(cube_vs.c_str(), cube_fs.c_str());
	glUseProgram(cube_shader_program);
	cube_vao = create_cube_vao();

	const int w = glutGet(GLUT_WINDOW_WIDTH);
	const int h = glutGet(GLUT_WINDOW_HEIGHT);
	//Create a reflection texture object and set initial wrapping and filtering state
	glGenTextures(1, &reflection_texture);
	glBindTexture(GL_TEXTURE_2D, reflection_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	//Create a refraction texture object and set initial wrapping and filtering state
	glGenTextures(1, &refraction_texture);
	glBindTexture(GL_TEXTURE_2D, refraction_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	//create a depth texture for water

	glGenTextures(1, &depth_texture);
	glBindTexture(GL_TEXTURE_2D, depth_texture);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	/*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);*/
	
	glBindTexture(GL_TEXTURE_2D, 0);

	//Create the reflectionframebuffer object
	glGenFramebuffers(1, &reflection_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, reflection_fbo);
	//attach the texture we just created to color attachment 0
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, reflection_texture, 0);
	//unbind the fbo
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//create render buffer object
	GLuint renderbuffer;
	glGenRenderbuffers(1, &renderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
	

	//Create the refractionframebuffer object
	glGenFramebuffers(1, &refraction_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, refraction_fbo);
	//attach the texture we just created to color attachment 1
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, refraction_texture, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);
	//attach the texture we just created to color attachment 2
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, depth_texture, 0);
	//unbind the fbo
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	
	//glGenFramebuffers(1, &depth_fbo);
	//glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0);

	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

	InitCamera(glm::vec3(patchScale*patchX / 2.0f, patchScale*patchY / 2.0f, 1.0f*patchScale), glm::vec3(3.14159265f / 2.0f, 0.0f, 3.14159265f));
}

void keyboard_up(unsigned char key, int x, int y)
{
	ImGui_ImplGlut_KeyUpCallback(key);
	CameraKeyboard(key);
}

void special_up(int key, int x, int y)
{
	ImGui_ImplGlut_SpecialUpCallback(key);
}

void passive(int x, int y)
{
	ImGui_ImplGlut_PassiveMouseMotionCallback(x, y);
}

void special(int key, int x, int y)
{
	ImGui_ImplGlut_SpecialCallback(key);
	CameraSpecialKey(key);
}

void motion(int x, int y)
{
	ImGui_ImplGlut_MouseMotionCallback(x, y);
}

void mouse(int button, int state, int x, int y)
{
	ImGui_ImplGlut_MouseButtonCallback(button, state);
}

int main(int argc, char **argv)
{
	//Configure initial window state
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(5, 5);
	glutInitWindowSize(win_width, win_height);
	int win = glutCreateWindow("Mountain Lake View");

	printGlInfo();

	//Register callback functions with glut. 
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);
	glutSpecialFunc(special);
	glutKeyboardUpFunc(keyboard_up);
	glutSpecialUpFunc(special_up);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutPassiveMotionFunc(motion);
	glutReshapeFunc(reshape);

	initOpenGl();
	ImGui_ImplGlut_Init(); // initialize the imgui system

	ISoundEngine* engine = irrklang::createIrrKlangDevice();

	if (!engine)
		return 0; // error starting up the engine

				  // play some sound stream, looped
	engine->play2D("Guzheng-Songs-of-Fishing-Boats-at-Dusk-yu-zhou-chang-wan.mp3", true);
						   //Enter the glut event loop.
	glutMainLoop();
	engine->drop(); // delete engine  
	glutDestroyWindow(win);
	return 0;
}



