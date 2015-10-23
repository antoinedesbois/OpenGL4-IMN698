#include "SSSSTestLevel.h"

#include "Sphere.h"
#include "Light.h"
#include "RenderBuffer.h"
#include "Quad.h"
#include <iostream>
#include <string>
#include "ModelLoader.h"
#include "Camera.h"

#include "GLM/glm/glm.hpp"
#include "GLM/glm/gtc/matrix_transform.hpp"
#include "GLM/glm/gtc/type_ptr.hpp"

SSSSTestLevel::SSSSTestLevel()
	: Scene()
{

}


// LOOP
void SSSSTestLevel::draw()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_hdrFBO);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	for each (Object* obj in m_objects)
	{
		GLuint shaderProgramID = obj->getShaderProgramId();
		glUseProgram(shaderProgramID);

		// Compute/Get information to pass to uniforms
		glm::mat4 modelMatrix = obj->getModelMatrix();
		glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(m_viewMatrix * modelMatrix)));
		const Material* objectMaterial = obj->getMaterial();

		// Get Uniforms location
		// Display matrixes
		GLuint modelLoc = glGetUniformLocation(shaderProgramID, "model");
		GLuint viewLoc = glGetUniformLocation(shaderProgramID, "view");
		GLuint projectionLoc = glGetUniformLocation(shaderProgramID, "projection");
		GLuint normalMatrixLoc = glGetUniformLocation(shaderProgramID, "normalMatrix");
		GLuint nbLightLoc = glGetUniformLocation(shaderProgramID, "nbLights");

		// Object information
		GLuint objectColorLoc = glGetUniformLocation(shaderProgramID, "objectColor");
		GLuint objectAmbientLoc = glGetUniformLocation(shaderProgramID, "objectMaterial.ambientCoefs");
		GLuint objectDiffuseLoc = glGetUniformLocation(shaderProgramID, "objectMaterial.diffuseCoefs");
		GLuint objectSpecularLoc = glGetUniformLocation(shaderProgramID, "objectMaterial.specularCoefs");
		GLuint objectShininessLoc = glGetUniformLocation(shaderProgramID, "objectMaterial.shininess");

		// Display Matrixes 
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(m_viewMatrix));
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(m_projectionMatrix));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		glUniform1f(nbLightLoc, m_lights.size());

		// Object information 
		glUniform3f(objectAmbientLoc, objectMaterial->m_ambientCoefs.x, objectMaterial->m_ambientCoefs.y, objectMaterial->m_ambientCoefs.z);
		glUniform3f(objectDiffuseLoc, objectMaterial->m_diffuseCoefs.x, objectMaterial->m_diffuseCoefs.y, objectMaterial->m_diffuseCoefs.z);
		glUniform3f(objectSpecularLoc, objectMaterial->m_specularCoefs.x, objectMaterial->m_specularCoefs.y, objectMaterial->m_specularCoefs.z);
		glUniform1f(objectShininessLoc, objectMaterial->m_shininess);

		// Lights information
		for (int i = 0; i < m_lights.size(); ++i)
		{
			glm::vec3 lightPosition = m_lights[i]->getPosition();
			const Material* lightMaterial = m_lights[i]->getMaterial();
			Light::AttenuationProperties lightProperties = m_lights[i]->getAttenuationProperties();

			GLuint lightPositionLoc = glGetUniformLocation(shaderProgramID, ("lightsPositions[" + std::to_string(i) + "]").c_str());
			GLuint lightAmbientLoc = glGetUniformLocation(shaderProgramID, ("lightsProperties[" + std::to_string(i) + "].material.ambientCoefs").c_str());
			GLuint lightDiffuseLoc = glGetUniformLocation(shaderProgramID, ("lightsProperties[" + std::to_string(i) + "].material.diffuseCoefs").c_str());
			GLuint lightSpecularLoc = glGetUniformLocation(shaderProgramID, ("lightsProperties[" + std::to_string(i) + "].material.specularCoefs").c_str());
			GLuint lightShininessLoc = glGetUniformLocation(shaderProgramID, ("lightsProperties[" + std::to_string(i) + "].material.shininess").c_str());
			GLuint lightAttConstant = glGetUniformLocation(shaderProgramID, ("lightsProperties[" + std::to_string(i) + "].constant").c_str());
			GLuint lightAttLinear = glGetUniformLocation(shaderProgramID, ("lightsProperties[" + std::to_string(i) + "].linear").c_str());
			GLuint lightAttQuadratic = glGetUniformLocation(shaderProgramID, ("lightsProperties[" + std::to_string(i) + "].quadratic").c_str());

			glUniform3f(lightPositionLoc, lightPosition.x, lightPosition.y, lightPosition.z);
			glUniform3f(lightAmbientLoc, lightMaterial->m_ambientCoefs.x, lightMaterial->m_ambientCoefs.y, lightMaterial->m_ambientCoefs.z);
			glUniform3f(lightDiffuseLoc, lightMaterial->m_diffuseCoefs.x, lightMaterial->m_diffuseCoefs.y, lightMaterial->m_diffuseCoefs.z);
			glUniform3f(lightSpecularLoc, lightMaterial->m_specularCoefs.x, lightMaterial->m_specularCoefs.y, lightMaterial->m_specularCoefs.z);
			glUniform1f(lightShininessLoc, lightMaterial->m_shininess);
			glUniform1f(lightAttConstant, lightProperties.m_constant);
			glUniform1f(lightAttLinear, lightProperties.m_linear);
			glUniform1f(lightAttQuadratic, lightProperties.m_quadratic);
		}

		obj->draw();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(m_renderQuad->getShaderProgramId());
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_renderBuffers[RenderBuffer::BufferType::eColor]->at(0));
	glUniform1i(glGetUniformLocation(m_renderQuad->getShaderProgramId(), "hdr"), true);
	glUniform1f(glGetUniformLocation(m_renderQuad->getShaderProgramId(), "exposure"), 1.0);
	m_renderQuad->draw();
}

void SSSSTestLevel::createShaderPrograms()
{
	// Create Shader programs
	ShaderProgram* BlinnPhongProgram = new ShaderProgram("BlinnPhong.vs", "BlinnPhong.fg");
	ShaderProgram* HDRProgram = new ShaderProgram("hdr.vs", "hdr.fg"); 
	ShaderProgram* SSSProgram = new ShaderProgram("SSS.vs", "SSS.fg");
	ShaderProgram* BumpColorMapsProgram = new ShaderProgram("BumpColorMaps.vs", "BumpColorMaps.fg");

	// Insert ShaderProgram in the list
	m_shaderPrograms.insert(std::pair<std::string, ShaderProgram*>("BlinnPhong", BlinnPhongProgram));
	m_shaderPrograms.insert(std::pair<std::string, ShaderProgram*>("HDR", HDRProgram));
	m_shaderPrograms.insert(std::pair<std::string, ShaderProgram*>("SSS", SSSProgram));
	m_shaderPrograms.insert(std::pair<std::string, ShaderProgram*>("BumpColorMaps", BumpColorMapsProgram));
}

void SSSSTestLevel::createMaterials()
{
	Material* defaultMaterial = new Material(glm::vec3(0.2f, 0.1f, 0.05f),
		glm::vec3(0.8f, 0.4f, 0.31f),
		glm::vec3(0.5f, 0.5f, 0.5f),
		32.0f);

	Material* blueMaterial = new Material(glm::vec3(0.0f, 0.1f, 0.06f),
		glm::vec3(0.0f, 0.5f, 0.5f),
		glm::vec3(0.5f, 0.5f, 0.5f),
		128.0f);

	Material* orangeMaterial = new Material(glm::vec3(0.15f, 0.1f, 0.0f),
		glm::vec3(0.8f, 0.6f, 0.0f),
		glm::vec3(0.6f, 0.6f, 0.5f),
		32);

	Material* defaultLightMaterial = new Material(glm::vec3(1.0f),
		glm::vec3(1.0f),
		glm::vec3(1.0f),
		1.0f);

	m_materials.insert(std::pair<std::string, Material*>("default", defaultMaterial));
	m_materials.insert(std::pair<std::string, Material*>("blue", blueMaterial));
	m_materials.insert(std::pair<std::string, Material*>("orange", orangeMaterial));
	m_materials.insert(std::pair<std::string, Material*>("defaultLight", defaultLightMaterial));
}

void SSSSTestLevel::levelSetup()
{
	Object* sphere1 = new Sphere(glm::vec3(-7, 0, 0), m_materials["default"], 2, 40, 40, m_shaderPrograms["BlinnPhong"]->getId());
	m_objects.push_back(sphere1);
	
	Object* sphere2 = new Sphere(glm::vec3(0, 0, 0), m_materials["orange"], 2, 40, 40, m_shaderPrograms["BlinnPhong"]->getId());
	m_objects.push_back(sphere2);
	
	Object* sphere3 = new Sphere(glm::vec3(7, 0, 0), m_materials["blue"], 2, 40, 40, m_shaderPrograms["SSS"]->getId());
	m_objects.push_back(sphere3);

	//Object* model1 = ModelLoader::loadModel("./HeadModel/head_tri.obj", m_materials["default"], m_shaderPrograms["BumpColorMaps"]->getId());
	//assert(model1, "model Not correctly loaded"); 
	//m_objects.push_back(model1);
	
	m_renderQuad = new Quad(glm::vec3(0.0f), nullptr, m_shaderPrograms["HDR"]->getId()); 

}

void SSSSTestLevel::lightSetup()
{
	Light::AttenuationProperties attenuationProp;
	attenuationProp.m_constant = 1.0f;
	attenuationProp.m_linear = 0.02f;
	attenuationProp.m_quadratic = 0.0005f;

	Light* light1 = new Light(glm::vec3(0, 10, 0), m_materials["defaultLight"], attenuationProp);
	m_lights.push_back(light1);

	Light* light2 = new Light(glm::vec3(-30, 40, 0), m_materials["defaultLight"], attenuationProp);
	m_lights.push_back(light2);
}

void SSSSTestLevel::buffersSetup()
{
	// Create and Bind Frame buffer
	glGenFramebuffers(1, &m_hdrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_hdrFBO);

	// Create RenderBuffers
	RenderBuffer::ParameterMap colorBuffer0Param;
	colorBuffer0Param[GL_TEXTURE_MIN_FILTER] = GL_LINEAR;
	colorBuffer0Param[GL_TEXTURE_MAG_FILTER] = GL_LINEAR;
	//colorBuffer0Param[GL_TEXTURE_WRAP_S] = GL_CLAMP_TO_EDGE;
	//colorBuffer0Param[GL_TEXTURE_WRAP_T] = GL_CLAMP_TO_EDGE;
	RenderBuffer colorBuffer0 = RenderBuffer(RenderBuffer::BufferType::eColor, &colorBuffer0Param);
	RenderBuffer depthBuffer0 = RenderBuffer(RenderBuffer::BufferType::eDepth);
	
	// Create Vectors of renderbuffers for each buffer type
	std::vector<GLuint>* colorBuffers = new std::vector<GLuint>;
	std::vector<GLuint>* depthBuffers = new std::vector<GLuint>;

	// Fill vectors
	colorBuffers->push_back(colorBuffer0.getData()); 
	depthBuffers->push_back(depthBuffer0.getData()); 
	
	// Assign vectors to the buffers map
	m_renderBuffers[RenderBuffer::BufferType::eColor] = colorBuffers; 
	m_renderBuffers[RenderBuffer::BufferType::eDepth] = depthBuffers;
	
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}