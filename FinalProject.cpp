#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <irrklang/irrKlang.h>


#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/gl.h>
#include <GL/glext.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "InitShader.h"
#include "LoadMesh.h"
#include "LoadTexture.h"
#include "imgui_impl_glut.h"

#include "Triangle.h"
#include "Quad.h"
#include "Cube.h"
#include "Particles.h"

using namespace irrklang;

float w = 1300.0; 
float h = 700.0;
//Texture files and IDs
static const std::string texture_sun = "sun.bmp";
GLuint textureSun_id = -1; //Texture map for Sun
static const std::string texture_earth = "earth.bmp";
GLuint textureEarth_id = -1; //Texture map for Earth
static const std::string texture_moon = "moon.jpg";
GLuint textureMoon_id = -1; //Texture map for Moon

static const std::string cube_name = "cubemap";
GLuint cubemap_id = -1; //Texture id for cubemap

//planet files and IDs
static const std::string sun_vs("sun_vs.glsl");
static const std::string sun_fs("sun_fs.glsl");
static const std::string earth_vs("earth_vs.glsl");
static const std::string earth_fs("earth_fs.glsl");
static const std::string moon_vs("moon_vs.glsl");
static const std::string moon_fs("moon_fs.glsl");
static const std::string sunWithShadow_vs("sunWithShadow_vs.glsl");
static const std::string sunWithShadow_fs("sunWithShadow_fs.glsl");
static const std::string triangle_vs("triangle_vs.glsl");
static const std::string triangle_fs("triangle_fs.glsl");
static const std::string quad_vs("quad_vs.glsl");
static const std::string quad_fs("quad_fs.glsl");

GLuint sun_shader_program = -1;
GLuint earth_shader_program = -1; 
GLuint moon_shader_program = -1;
GLuint sunWithShadow_shader_program = -1;
GLuint triangle_shader_program = -1;
GLuint quad_shader_program = -1;

static const std::string mesh_name = "Sphere.obj";
MeshData mesh_data;
glm::vec3 sunpos(0.0f, 0.0f, 0.0f);
glm::vec3 earthpos(1.1f, 0.0f, 0.0f);
glm::vec3 moonpos(1.2f, 0.0f, 0.0f);
glm::vec3 emDistance(0.05f, 0.0f, 0.0f);

//bool mesh_enabled = false;


bool animation_enabled = false;

//Cube files and IDs
static const std::string cube_vs("cube_vs.glsl");
static const std::string cube_fs("cube_fs.glsl");
GLuint cube_shader_program = -1;
GLuint cube_vao = -1;
bool cube_enabled = false;
GLuint quad_vao = -1;

static const std::string particle_vs("particle_vs.glsl");
static const std::string particle_fs("particle_fs.glsl");
GLuint particle_shader_program = -1;
GLuint particle_vao = -1;
bool particles_enabled = false;

float camangle = 0.0f;
glm::vec3 campos(0.0f, 0.0f, 2.5f);
//second and third viewport
float camangle1 = 20.0f;
glm::vec3 campos1(0.0f, 0.0f, 2.0f);
float moveMoon1 = 170.0;
//Fourth and fifth viewport
float camangle2 = 80.0f;
glm::vec3 campos2(0.0f, 0.0f, 2.0f);
float moveMoon2 = 80.0;

float aspect = 1.0f;

void draw_gui()
{
   glUseProgram(sun_shader_program);
   static bool first_frame = true;
   ImGui_ImplGlut_NewFrame();
   static bool show_window = true;

   ImGui::SetNextWindowPos(ImVec2(0, 0));
   ImGui::Begin("Overview", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
   ImGui::Checkbox("Animation", &animation_enabled);
   ImGui::PushItemWidth(w/3-100);
   ImGui::SliderFloat("View angle", &camangle, -180.0f, +180.0f);
   ImGui::SliderFloat("Zoom", &campos[2], -10.0f, +10.0f);
   ImGui::End();

  ImGui::SetNextWindowPos(ImVec2((w - 20) / 3 + 10, 0));
  ImGui::Begin("Solar Eclipse", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
  ImGui::PushItemWidth((w - 20) / 3*0.73);
  ImGui::SliderFloat("Zoom", &campos1[2], -10.0f, +10.0f);
  ImGui::SliderFloat("View angle", &camangle1, -180.0f, +180.0f);
  ImGui::SliderFloat("Moon position", &moveMoon1, 170.0f, 190.0f);
   ImGui::End();

   ImGui::SetNextWindowPos(ImVec2((w-20)*2/3+20, 0));
   ImGui::Begin("Lunar Eclipse", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
   ImGui::PushItemWidth((w-20) / 3*0.73);
   ImGui::SliderFloat("Zoom", &campos2[2], -10.0f, +10.0f);
   ImGui::SliderFloat("View angle", &camangle2, -180.0f, +180.0f);
   ImGui::SliderFloat("Moon position", &moveMoon2, -80.0f, 80.0f);

   ImGui::End();
   ImGui::Render();
   first_frame = false;
}


float angSun = 0.0;
void draw_sun(const glm::mat4& P, const glm::mat4& V,float angSun)
{
   glm::mat4 T = glm::translate(sunpos);
   glm::mat4 M = T*glm::rotate(angSun, glm::vec3(0.0f, 1.0f, 1.0f))*glm::scale(glm::vec3(0.5f*mesh_data.mScaleFactor));
   
   glUseProgram(sun_shader_program);
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, textureSun_id);
   int tex_loc = glGetUniformLocation(sun_shader_program, "tex");
   if (tex_loc != -1)
   {
      glUniform1i(tex_loc, 0); // we bound our texture to texture unit 0
   }

   glActiveTexture(GL_TEXTURE1);
   glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_id);
   int cube_loc = glGetUniformLocation(sun_shader_program, "cubemap");
   if (cube_loc != -1)
   {
      glUniform1i(cube_loc, 1); // we bound our texture to texture unit 1
   }

   int PVM_loc = glGetUniformLocation(sun_shader_program, "PVM");
   if (PVM_loc != -1)
   {
      glm::mat4 PVM = P*V*M;
      glUniformMatrix4fv(PVM_loc, 1, false, glm::value_ptr(PVM));
   }

   int M_loc = glGetUniformLocation(sun_shader_program, "M");
   if (M_loc != -1)
   {
      glUniformMatrix4fv(M_loc, 1, false, glm::value_ptr(M));
   }

   int V_loc = glGetUniformLocation(sun_shader_program, "V");
   if (V_loc != -1)
   {
      glUniformMatrix4fv(V_loc, 1, false, glm::value_ptr(V));
   }

   glBindVertexArray(mesh_data.mVao);
   glDrawElements(GL_TRIANGLES, mesh_data.mNumIndices, GL_UNSIGNED_INT, 0);
}

void draw_sunWithShadow(const glm::mat4& P, const glm::mat4& V, float angP, float angS, float angM)
{
	glm::mat4 T = glm::translate(sunpos);
	glm::mat4 M = T*glm::scale(glm::vec3(0.5f*mesh_data.mScaleFactor));
	float angt1 = 5.0;
	float angt = 23.5;
	glm::mat4 Te = glm::translate(earthpos);
	glm::mat4 Me = glm::rotate(angP, glm::vec3(0.0f, 0.0f, 1.0f))*Te*glm::scale(glm::vec3(0.12f*mesh_data.mScaleFactor))*glm::rotate(angS, glm::vec3(0.0f, 0.398f, 0.917f));

	glm::mat4 T0 = glm::translate(moonpos);
	glm::mat4 T1 = glm::translate(moonpos - earthpos);
	glm::mat4 T2 = glm::translate(earthpos - moonpos);
	glm::mat4 Mm = glm::rotate(angP, glm::vec3(0.0f, 0.0f, 1.0f))*T0*T2*glm::rotate(angt1, glm::vec3(1.0f, 0.0f, 0.0f))*glm::rotate(angM, glm::vec3(0.0f, 0.0f, 1.0f))*T1*glm::scale(glm::vec3(0.05f*mesh_data.mScaleFactor));

	glUseProgram(sunWithShadow_shader_program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureSun_id);
	int tex_loc = glGetUniformLocation(sunWithShadow_shader_program, "tex");
	if (tex_loc != -1)
	{
		glUniform1i(tex_loc, 0); // we bound our texture to texture unit 0
	}

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_id);
	int cube_loc = glGetUniformLocation(sunWithShadow_shader_program, "cubemap");
	if (cube_loc != -1)
	{
		glUniform1i(cube_loc, 1); // we bound our texture to texture unit 1
	}

	int PVM_loc = glGetUniformLocation(sunWithShadow_shader_program, "PVM");
	if (PVM_loc != -1)
	{
		glm::mat4 PVM = P*V*M;
		glUniformMatrix4fv(PVM_loc, 1, false, glm::value_ptr(PVM));
	}

	int M_loc = glGetUniformLocation(sunWithShadow_shader_program, "M");
	if (M_loc != -1)
	{
		glUniformMatrix4fv(M_loc, 1, false, glm::value_ptr(M));
	}
	int Me_loc = glGetUniformLocation(sunWithShadow_shader_program, "Me");
	if (Me_loc != -1)
	{
		glUniformMatrix4fv(Me_loc, 1, false, glm::value_ptr(Me));
	}
	int Mm_loc = glGetUniformLocation(sunWithShadow_shader_program, "Mm");
	if (Mm_loc != -1)
	{
		glUniformMatrix4fv(Mm_loc, 1, false, glm::value_ptr(Mm));
	}
	int V_loc = glGetUniformLocation(sunWithShadow_shader_program, "V");
	if (V_loc != -1)
	{
		glUniformMatrix4fv(V_loc, 1, false, glm::value_ptr(V));
	}

	glBindVertexArray(mesh_data.mVao);
	glDrawElements(GL_TRIANGLES, mesh_data.mNumIndices, GL_UNSIGNED_INT, 0);
}

float angS = 0.0;
float angP = 0.0;
float angM = 0.0;
	
void draw_earth(const glm::mat4& P, const glm::mat4& V,float angP,float angS,float angM)
{
	float angt = 23.5;
	glm::mat4 T = glm::translate(earthpos);
	glm::mat4 M = glm::rotate(angP, glm::vec3(0.0f, 0.0f, 1.0f))*T*glm::scale(glm::vec3(0.12f*mesh_data.mScaleFactor))*glm::rotate(angS, glm::vec3(0.0f, 0.398f,0.917f));
	float angt1 = 5.0;

	glm::mat4 T0 = glm::translate(moonpos);
	glm::mat4 T1 = glm::translate(moonpos - earthpos);
	glm::mat4 T2 = glm::translate(earthpos - moonpos);
	glm::mat4 Mm = glm::rotate(angP, glm::vec3(0.0f, 0.0f, 1.0f))*T0*T2*glm::rotate(angt1, glm::vec3(1.0f, 0.0f, 0.0f))*glm::rotate(angM, glm::vec3(0.0f, 0.0f, 1.0f))*T1*glm::scale(glm::vec3(0.05f*mesh_data.mScaleFactor))*glm::rotate(90.0f, glm::vec3(0.0f, 0.0f, 1.0f));

	glUseProgram(earth_shader_program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureEarth_id);
	int tex_loc = glGetUniformLocation(earth_shader_program, "tex");
	if (tex_loc != -1)
	{
		glUniform1i(tex_loc, 0); // we bound our texture to texture unit 0
	}

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_id);
	int cube_loc = glGetUniformLocation(earth_shader_program, "cubemap");
	if (cube_loc != -1)
	{
		glUniform1i(cube_loc, 1); // we bound our texture to texture unit 1
	}

	int PVM_loc = glGetUniformLocation(earth_shader_program, "PVM");
	if (PVM_loc != -1)
	{
		glm::mat4 PVM = P*V*M;
		glUniformMatrix4fv(PVM_loc, 1, false, glm::value_ptr(PVM));
	}

	int M_loc = glGetUniformLocation(earth_shader_program, "M");
	if (M_loc != -1)
	{
		glUniformMatrix4fv(M_loc, 1, false, glm::value_ptr(M));
	}
	int Mm_loc = glGetUniformLocation(earth_shader_program, "Mm");
	if (Mm_loc != -1)
	{
		glUniformMatrix4fv(Mm_loc, 1, false, glm::value_ptr(Mm));
	}
	int V_loc = glGetUniformLocation(earth_shader_program, "V");
	if (V_loc != -1)
	{
		glUniformMatrix4fv(V_loc, 1, false, glm::value_ptr(V));
	}

	glBindVertexArray(mesh_data.mVao);
	glDrawElements(GL_TRIANGLES, mesh_data.mNumIndices, GL_UNSIGNED_INT, 0);
}

void draw_moon(const glm::mat4& P, const glm::mat4& V,float angP,float angS, float angM)
{
	float angt = 23.5;
	glm::mat4 T = glm::translate(earthpos);
	glm::mat4 Me = glm::rotate(angP, glm::vec3(0.0f, 0.0f, 1.0f))*T*glm::scale(glm::vec3(0.12f*mesh_data.mScaleFactor))*glm::rotate(angS, glm::vec3(0.0f, 0.398f, 0.917f));

	float angt1 = 5.0;
	glm::mat4 T0 = glm::translate(moonpos);
	glm::mat4 T1 = glm::translate(moonpos-earthpos);
	glm::mat4 T2 = glm::translate(earthpos - moonpos);
	glm::mat4 M = glm::rotate(angP, glm::vec3(0.0f, 0.0f, 1.0f))*T0*T2*glm::rotate(angt1, glm::vec3(1.0f, 0.0f, 0.0f))*glm::rotate(angM, glm::vec3(0.0f, 0.0f, 1.0f))*T1*glm::scale(glm::vec3(0.05f*mesh_data.mScaleFactor));
	

	glUseProgram(moon_shader_program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureMoon_id);
	int tex_loc = glGetUniformLocation(moon_shader_program, "tex");
	if (tex_loc != -1)
	{
		glUniform1i(tex_loc, 0); // we bound our texture to texture unit 0
	}

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_id);
	int cube_loc = glGetUniformLocation(moon_shader_program, "cubemap");
	if (cube_loc != -1)
	{
		glUniform1i(cube_loc, 1); // we bound our texture to texture unit 1
	}

	int PVM_loc = glGetUniformLocation(moon_shader_program, "PVM");
	if (PVM_loc != -1)
	{
		glm::mat4 PVM = P*V*M;
		glUniformMatrix4fv(PVM_loc, 1, false, glm::value_ptr(PVM));
	}

	int M_loc = glGetUniformLocation(moon_shader_program, "M");
	if (M_loc != -1)
	{
		glUniformMatrix4fv(M_loc, 1, false, glm::value_ptr(M));
	}
	int Me_loc = glGetUniformLocation(moon_shader_program, "Me");
	if (Me_loc != -1)
	{
		glUniformMatrix4fv(Me_loc, 1, false, glm::value_ptr(Me));
	}

	int V_loc = glGetUniformLocation(moon_shader_program, "V");
	if (V_loc != -1)
	{
		glUniformMatrix4fv(V_loc, 1, false, glm::value_ptr(V));
	}

	glBindVertexArray(mesh_data.mVao);
	glDrawElements(GL_TRIANGLES, mesh_data.mNumIndices, GL_UNSIGNED_INT, 0);
}

void draw_quad1(const glm::mat4& P, const glm::mat4& V)
{
	glUseProgram(triangle_shader_program);
	glm::mat4 PVM;

	int PVM_loc = glGetUniformLocation(triangle_shader_program, "PVM");
	if (PVM_loc != -1)
	{
		float ang = 90;
		glm::mat4 PVM = glm::translate(glm::vec3(0.0,-0.46,0.8))*glm::scale(glm::vec3(0.5,0.3,0.5))*glm::rotate(ang, glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(PVM_loc, 1, false, glm::value_ptr(PVM));
	
	}

	draw_quad_vao(quad_vao);


}
void draw_quad(const glm::mat4& P, const glm::mat4& V)
{
   glUseProgram(quad_shader_program);
   glm::mat4 PVM;

   int PVM_loc = glGetUniformLocation(quad_shader_program, "PVM");
   if (PVM_loc != -1)
   {
	   float ang = 90;
	   glm::mat4 M = glm::translate(glm::vec3(0.0, 0.0, -0.09))*glm::rotate(ang, glm::vec3(0.0f, 1.0f, 0.0f));
	   glm::mat4 PVM = P*V*M;
	   glUniformMatrix4fv(PVM_loc, 1, false, glm::value_ptr(PVM));
	     }
   int moonposition_loc = glGetUniformLocation(quad_shader_program, "moonposition");
   if (moonposition_loc != -1)
   {
	   glUniform1f(moonposition_loc, moveMoon1);
   }
   draw_quad_vao(quad_vao);

}

void draw_cube(const glm::mat4& P, const glm::mat4& V)
{
   glUseProgram(cube_shader_program);
   int PVM_loc = glGetUniformLocation(cube_shader_program, "PVM");
   if (PVM_loc != -1)
   {
      glm::mat4 Msky = glm::scale(glm::vec3(50.0f));
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

void drawEarthOrbit(const glm::mat4& P, const glm::mat4& V)
{
   glUseProgram(particle_shader_program);
   int PVM_loc = glGetUniformLocation(particle_shader_program, "PVM");
   if (PVM_loc != -1)
   {
      glm::mat4 PVM = P*V;
      glUniformMatrix4fv(PVM_loc, 1, false, glm::value_ptr(PVM));
   }

   draw_particles_vao(particle_vao);
}
void drawMoonOrbit(const glm::mat4& P, const glm::mat4& V, float angP)
{
	
	float angt = 5.0;
	glm::mat4 T = glm::translate(earthpos);
	glm::mat4 M = glm::rotate(angP, glm::vec3(0.0f, 0.0f, 1.0f))*T*glm::rotate(angt, glm::vec3(1.0f, 0.0f, 0.0f))*glm::scale(glm::vec3(0.091f));

	glUseProgram(particle_shader_program);
	int PVM_loc = glGetUniformLocation(particle_shader_program, "PVM");
	if (PVM_loc != -1)
	{
		glm::mat4 PVM = P*V*M;
		glUniformMatrix4fv(PVM_loc, 1, false, glm::value_ptr(PVM));
	}
	
	draw_particles_vao(particle_vao);
}


// glut display callback function.
// This function gets called every time the scene gets redisplayed 
void display()
{
	glViewport(0, 0, (w-20) / 3, h);
	if (animation_enabled)
	{
		angSun = angSun + 0.1;
		angS = angS + 0.365;
		angP = angP + 0.01;
		angM = angM + 0.03;
	}
	float r = 1.1;
	float camrot = -70.0;
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Clear the back buffer
   
   glm::mat4 V = glm::lookAt(campos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f))*glm::rotate(camrot, glm::vec3(1.0f, 0.0f, 0.0f))*glm::rotate(camangle, glm::vec3(1.0f, 0.0f, 0.0f));
   glm::mat4 P = glm::perspective(80.0f, aspect, 0.1f, 100.0f); //not affine
   draw_earth(P, V,angP,angS,angM);
   draw_moon(P, V,angP, angS,angM);
   draw_sun(P, V,angSun);
   draw_cube(P, V);
   drawEarthOrbit(P, V);
   drawMoonOrbit(P, V,angP);

   //Draw the second viewport
	glViewport((w-20) / 3+10, 0.0, (w-20) / 3, h);
	
	glm::mat4 T = glm::inverse(glm::translate(earthpos));
	glm::vec3 camlookat = glm::vec3(0.0,0.0,0.0);
	glm::mat4 V1 = glm::lookAt(campos1, camlookat, glm::vec3(0.0f, 1.0f, 0.0f))*glm::rotate(camrot, glm::vec3(1.0f, 0.0f, 0.0f))*glm::rotate(camangle1, glm::vec3(0.0f, 0.0f, 1.0f))*T;
	glm::mat4 P1 = glm::perspective(80.0f, aspect, 0.1f, 100.0f); //not affine
	draw_earth(P1, V1,0.0,0.0,moveMoon1);
	draw_moon(P1, V1,0.0,0.0,moveMoon1);
	draw_sun(P1, V1,0.0);
	draw_cube(P1, V1);
	drawEarthOrbit(P1, V1);
	drawMoonOrbit(P1, V1,0.0);

// Draw the third viewport
	glViewport((w - 20)/ 3 + 10, h/2, (w - 20) / 3, h);
	glm::mat4 T_sun = glm::inverse(glm::translate(glm::vec3(0.0, 1.45, 0.0)));
	//glm::vec3 camOnEarth = glm::vec3(glm::vec4(0.0, 0.0, 0.0,1.0)*T1);
	float camrot2 = -90.0;
	float camangle_sun = 90.0;

	glm::mat4 V2 = T_sun*glm::lookAt(glm::vec3(0.0f, 0.0f, 4.0f), camlookat, glm::vec3(0.0f, 1.0f, 0.0f))*glm::rotate(camrot2, glm::vec3(1.0f, 0.0f, 0.0f))*glm::rotate(camangle_sun, glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 P2 = glm::perspective(80.0f, aspect, 0.1f, 100.0f); //not affine
//	draw_earth(P2, V2, 0.0, 0.0, moveMoon1);
//	draw_moon(P2, V2, 0.0, 0.0, moveMoon1);
	draw_quad(P2, V2);
	draw_sunWithShadow(P2, V2, 0.0,0.0,moveMoon1*5);
//draw_cube(P2, V2);
	
// Draw the fourth viewport
	glViewport((w - 20)*2/3+20, 0.0, (w - 20) / 3, h);
	float camrot1 = -20.0;
	glm::mat4 V3 = glm::lookAt(campos2, camlookat, glm::vec3(0.0f, 1.0f, 0.0f))*glm::rotate(camrot1, glm::vec3(1.0f, 0.0f, 0.0f))*glm::rotate(camangle2, glm::vec3(0.0f, 0.0f, 1.0f))*T;
	glm::mat4 P3 = glm::perspective(80.0f, aspect, 0.1f, 100.0f); //not affine
    draw_earth(P3, V3,0.0,0.0,moveMoon2);
	draw_moon(P3, V3,0.0,0.0,moveMoon2);
	draw_sun(P3, V3,0.0);
	draw_cube(P3, V3);
	drawEarthOrbit(P3, V3);
	drawMoonOrbit(P3, V3,0.0);
//Draw fifth viewport
	glViewport((w - 20) * 2 / 3 + 20, h/2, (w - 20) / 3, h);
   glm::mat4 T3 = glm::inverse(glm::translate(moonpos));
   glm::mat4 T_moon = glm::inverse(glm::translate(glm::vec3(0.0, 0.19, 0.0)));

	glm::mat4 V4 = T_moon*glm::lookAt(glm::vec3(0.0f, 0.0f, 0.5f), camlookat, glm::vec3(0.0f, 1.0f, 0.0f))*glm::rotate(camrot1, glm::vec3(1.0f, 0.0f, 0.0f))*glm::rotate(camangle_sun,glm::vec3(0.0,0.0,1.0))*glm::rotate(camangle_sun, glm::vec3(0.0f, 1.0f, 0.0f))*T3;
	glm::mat4 P4 = glm::perspective(80.0f, aspect, 0.1f, 100.0f); //not affine
//	draw_earth(P4, V4, 0.0, 0.0, moveMoon2);
	draw_quad1(P4, V4);
	draw_moon(P4, V4, 0.0, 0.0, moveMoon2);
	
//	draw_sun(P4, V4, 0.0);
//	draw_cube(P4, V4);
	
	draw_gui();
	glutSwapBuffers();
}
void idle()
{
   glutPostRedisplay();

   const int time_ms = glutGet(GLUT_ELAPSED_TIME);
   float time_sec = 0.001f*time_ms;

   glUseProgram(earth_shader_program);
   int time_loc = glGetUniformLocation(earth_shader_program, "time");
   if (time_loc != -1)
   {
      //double check that you are using glUniform1f
      glUniform1f(time_loc, time_sec);
   }

   glUseProgram(particle_shader_program);
   time_loc = glGetUniformLocation(particle_shader_program, "time");
   if (time_loc != -1)
   {
      //double check that you are using glUniform1f
      glUniform1f(time_loc, time_sec);
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
   glEnable(GL_POINT_SPRITE);       // allows textured points
   glEnable(GL_PROGRAM_POINT_SIZE); //allows us to set point size in vertex shader
   glClearColor(0.35f, 0.35f, 0.35f, 0.0f);

   textureSun_id = LoadTexture(texture_sun);
   textureEarth_id = LoadTexture(texture_earth);
   textureMoon_id = LoadTexture(texture_moon);
   cubemap_id = LoadCube(cube_name);

   sun_shader_program = InitShader(sun_vs.c_str(), sun_fs.c_str());
   sunWithShadow_shader_program = InitShader(sunWithShadow_vs.c_str(), sunWithShadow_fs.c_str());

   earth_shader_program = InitShader(earth_vs.c_str(), earth_fs.c_str());
   moon_shader_program = InitShader(moon_vs.c_str(), moon_fs.c_str());
   //mesh and texture to be rendered
   mesh_data = LoadMesh(mesh_name);
   
   cube_shader_program = InitShader(cube_vs.c_str(), cube_fs.c_str());
   cube_vao = create_cube_vao();
   triangle_shader_program = InitShader(triangle_vs.c_str(), triangle_fs.c_str());
   quad_shader_program = InitShader(quad_vs.c_str(), quad_fs.c_str());
    quad_vao = create_quad_vao();


   particle_shader_program = InitShader(particle_vs.c_str(), particle_fs.c_str());
   particle_vao = create_particles_vao();
}

// glut callbacks need to send keyboard and mouse events to imgui
void keyboard(unsigned char key, int x, int y)
{
   ImGui_ImplGlut_KeyCallback(key);
   //std::cout << "key : " << key << ", x: " << x << ", y: " << y << std::endl;
}

void keyboard_up(unsigned char key, int x, int y)
{
   ImGui_ImplGlut_KeyUpCallback(key);
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
}

void motion(int x, int y)
{
   ImGui_ImplGlut_MouseMotionCallback(x, y);
}

void mouse(int button, int state, int x, int y)
{
   ImGui_ImplGlut_MouseButtonCallback(button, state);
}

void reshape(int w, int h)
{
   glViewport(0, 0, w, h);
   aspect = (float)w / h/3;
}


int main(int argc, char **argv)
{
   //Configure initial window state
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
   glutInitWindowPosition(5, 5);
   glutInitWindowSize(w, h);
   int win = glutCreateWindow("Solar and Lunar Eclipses");
  
   printGlInfo();

   //Register callback functions with glut. 
   glutDisplayFunc(display);
   glutKeyboardFunc(keyboard);
   glutSpecialFunc(special);
   glutKeyboardUpFunc(keyboard_up);
   glutSpecialUpFunc(special_up);
   glutMouseFunc(mouse);
   glutMotionFunc(motion);
   glutPassiveMotionFunc(motion);
   glutIdleFunc(idle);
 glutReshapeFunc(reshape);

   initOpenGl();
   ImGui_ImplGlut_Init(); // initialize the imgui system

   ISoundEngine* engine = irrklang::createIrrKlangDevice();

   if (!engine)
	   return 0; // error starting up the engine

				 // play some sound stream, looped
   engine->play2D("bensound-scifi.mp3", true);

   //Enter the glut event loop.
   glutMainLoop();
   engine->drop(); // delete engine  

   glutDestroyWindow(win);
  
   return 0;
}