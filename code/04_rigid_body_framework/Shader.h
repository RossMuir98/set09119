#pragma once

#include <filesystem>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <GL/glew.h>

#include <glm/glm.hpp>

// Helper function -- reads a text file to a string
inline std::string ReadAllText(const char * filepath)
{
	std::ifstream fs(filepath);
	std::stringstream buffer;
	buffer << fs.rdbuf();
	return buffer.str();
}

// Shader helper class, only stores the OpenGL "program" which is a set of linked vertex and fragment shaders
class Shader
{
public:

	void CreateFromFile(const std::string_view vertexPath, const std::string_view fragmentPath)
	{
		auto vertexSource = ReadAllText(vertexPath.data());
		auto fragmentSource = ReadAllText(fragmentPath.data());
		CreateFromSource(vertexSource, fragmentSource);
	}

	void CreateFromSource(const std::string_view vertexSource, const std::string_view fragmentSource)
	{
		const GLchar* vShaderCode = vertexSource.data();
		const GLchar* fShaderCode = fragmentSource.data();
		// 2. Compile shaders
		GLuint vertex, fragment;
		GLint success;
		GLchar infoLog[512];
		// Vertex Shader
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		// Print compile errors if any
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertex, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		// Fragment Shader
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		// Print compile errors if any
		glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragment, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		// Shader Program
		if (m_program != 0)
			glDeleteProgram(m_program);
		m_program = glCreateProgram();
		glAttachShader(m_program, vertex);
		glAttachShader(m_program, fragment);
		glLinkProgram(m_program);
		// Print linking errors if any
		glGetProgramiv(m_program, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(m_program, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}
		// Delete the shaders as they're linked into our program now and no longer necessery
		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}
	
	// Set current shader as active
	void Use() const
	{
		glUseProgram(m_program);
	}

	// Helper to set a matrix uniform
	void SetUniform(const char* uniformName, const glm::mat4& uniformValue) const
	{
		GLint loc = glGetUniformLocation(m_program, uniformName);
		glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(uniformValue));
	}

	// Helper to set a vec4 uniform
	void SetUniform(const char* uniformName, const glm::vec4& uniformValue) const
	{
		GLint loc = glGetUniformLocation(m_program, uniformName);
		glUniform4fv(loc, 1, glm::value_ptr(uniformValue));
	}

	// Helper to set a float uniform
	void SetUniform(const char* uniformName, const float uniformValue) const
	{
		GLint loc = glGetUniformLocation(m_program, uniformName);
		glUniform1fv(loc, 1, &uniformValue);
	}

private:
	GLuint m_program = 0;
};

// Create a default shader for our objects
// We declare this inline so that we can have the implementation here in the header file
inline Shader CreateDefaultShader()
{
	const char* vertexSource = R"(
		#version 330 core
		layout (location = 0) in vec3 position_in;
		layout (location = 1) in vec3 normal_in;

		uniform mat4 modelViewProjectionMatrix;

		out vec3 normal;

		void main()
		{
			normal = normal_in;
			gl_Position = modelViewProjectionMatrix * vec4(position_in, 1.0f);
		}
	)";

	const char* fragmentSource = R"(
		#version 330 core
		out vec4 fragmentColor;

		uniform vec4 color = vec4(0.5,0.5,0.5,1.0);
		uniform mat4 normalMatrix;

		in vec3 normal;

		void main()
		{
			vec3 n = normalize((normalMatrix * vec4(normalize(normal),0)).xyz);
			float NdotL = abs(dot(n, vec3(0,0,1))); // abs for double-sided lighting
			fragmentColor = vec4( color.xyz * (0.2 + 0.8*NdotL), color.a);
		}
	)";
	Shader shader;
	shader.CreateFromSource(vertexSource, fragmentSource);
	return shader;
}
