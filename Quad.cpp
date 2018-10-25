#include "Quad.h"

GLuint create_quad_vao()
{
   GLuint vao, vbo;
   const float quad_verts[] = { -0.5f, -1.5f, 0.0f, 0.5f, -1.5f, 0.0f, -0.5f, 1.5f, 0.0f, 0.5f, 1.5f, 0.0f };

   //generate vao id to hold the mapping from attrib variables in shader to memory locations in vbo
   glGenVertexArrays(1, &vao);
   glGenBuffers(1, &vbo); // Generate vbo to hold vertex attributes for triangle

   //binding vao means that bindbuffer, enablevertexattribarray and vertexattribpointer 
   // state will be remembered by vao
   glBindVertexArray(vao);
   glBindBuffer(GL_ARRAY_BUFFER, vbo); //specify the buffer where vertex attribute data is stored
   //upload from main memory to gpu memory
   glBufferData(GL_ARRAY_BUFFER, sizeof(quad_verts), &quad_verts[0], GL_STATIC_DRAW);

   //get the current shader program
   GLint shader_program = -1;
   glGetIntegerv(GL_CURRENT_PROGRAM, &shader_program);

   //get a reference to an attrib variable name in a shader
   GLint pos_loc = glGetAttribLocation(shader_program, "pos_attrib");

   glEnableVertexAttribArray(pos_loc); //enable this attribute

   //tell opengl how to get the attribute values out of the vbo (stride and offset)
   glVertexAttribPointer(pos_loc, 3, GL_FLOAT, false, 0, 0);
   glBindVertexArray(0); //unbind the vao

   return vao;
}

void draw_quad_vao(GLuint vao)
{
   glBindVertexArray(vao);
   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}