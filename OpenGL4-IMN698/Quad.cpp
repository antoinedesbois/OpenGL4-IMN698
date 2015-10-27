#include "Quad.h"

Quad::Quad(glm::vec3 position, Material* material, GLuint shaderProgram)
: Object(position, material, shaderProgram)
{
	setupObject(); 
}

void Quad::defineVBO()
{
	GLfloat vertices[] = {
		// Vertex position	  //Texture Coordinates 
		-1.0f,  1.0f,  0.0f,     0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f,	 0.0f, 0.0f, 
		 1.0f,  1.0f,  0.0f,     1.0f, 1.0f,
		 1.0f, -1.0f,  0.0f,     1.0f, 0.0f, 
	};

	glGenBuffers(1, &m_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

	m_numIndices = sizeof(vertices) / sizeof(float); 
}
void Quad::defineVAO()
{
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}
void Quad::defineEBO()
{
	
}

void Quad::draw() const
{
	glBindVertexArray(m_VAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, m_numIndices); 
	glBindVertexArray(0); 

}