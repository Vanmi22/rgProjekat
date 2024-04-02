#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <common.h>

class Shader {
public:
    unsigned int m_id;

    //konstruktor
    Shader (const char* vertex_path, const char* fragment_path) {

        //char* u string
        std::string vertex_path_string(vertex_path);
        std::string fragment_path_string(fragment_path);

        //dobijanje vertex/fragment koda iz fajlova
        std::string vertex_code;
        std::string fragment_code;
        std::ifstream vertex_shader_file;
        std::ifstream fragment_shader_file;

        //omogucavanje bacanja izuzetaka
        vertex_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fragment_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        try {

            //otvaranje fajlova
            vertex_shader_file.open(vertex_path);
            fragment_shader_file.open(fragment_path);

            std::stringstream vertex_shader_stream;
            std::stringstream fragment_shader_stream;

            //ucitavanje sadrzaja u stream-ove
            vertex_shader_stream << vertex_shader_file.rdbuf();
            fragment_shader_stream << fragment_shader_file.rdbuf();

            vertex_shader_file.close();
            fragment_shader_file.close();

            //konvertovanje stream-ova u string
            vertex_code = vertex_shader_stream.str();
            fragment_code = fragment_shader_stream.str();

        }
        catch (std::ifstream::failure &e) {
            std::cerr << "ERROR::SHADER::NEUSPESNO_UCITAVANJE_FAJLA" << "\n";
        }

        //string u char*
        const char* vertex_shader_code = vertex_code.c_str();
        const char* fragment_shader_code = fragment_code.c_str();

        //kompajliranje shader-a
        unsigned int vertex, fragment;

        //vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vertex_shader_code, NULL);
        glCompileShader(vertex);
        //CHECK ERRORS

        //fragment shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fragment_shader_code, NULL);
        glCompileShader(fragment);
        //CHECK ERRORS

        //shader program
        m_id = glCreateProgram();
        glAttachShader(m_id, vertex);
        glAttachShader(m_id, fragment);

        glLinkProgram(m_id);
        //CHECK ERRORS

        //brisanje shader-a jer vise nisu potrebni
        glDeleteShader(vertex);
        glDeleteShader(fragment);

    }

    //aktiviranje shader-a
    void use() {
        glUseProgram(m_id);
    }

    //korisne uniform funkcije
    void setInt(const std::string &name, int value) const {
        glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
    }

    void setFloat(const std::string &name, float value) const {
        glUniform1f(glGetUniformLocation(m_id, name.c_str()), value);
    }

    void setVec2 (const std::string &name, glm::vec2 &value) const {
        glUniform2fv(glGetUniformLocation(m_id, name.c_str()), 1, &value[0]);
    }

    void setVec2 (const std::string &name, float x, float y) const {
        glUniform2f(glGetUniformLocation(m_id, name.c_str()), x, y);
    }

    void setVec3 (const std::string &name, glm::vec3 &value) const {
        glUniform3fv(glGetUniformLocation(m_id, name.c_str()), 1, &value[0]);
    }

    void setVec3 (const std::string &name, float x, float y, float z) const {
        glUniform3f(glGetUniformLocation(m_id, name.c_str()), z, y, z);
    }

    void setVec4 (const std::string &name, glm::vec4 &value) const {
        glUniform4fv(glGetUniformLocation(m_id, name.c_str()), 1, &value[0]);
    }

    void setVec4 (const std::string &name, float x, float y, float z, float w) const {
        glUniform4f(glGetUniformLocation(m_id, name.c_str()), x, y, z, w);
    }

    void setMat2 (const std::string &name, const glm::mat2 &mat) const {
        glUniformMatrix2fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void setMat3 (const std::string &name, const glm::mat3 &mat) const {
        glUniformMatrix3fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void setMat4 (const std::string &name, const glm::mat4 &mat) const {
        glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

private:
    //funkicja za proveru gresaka pri kompajliranju/povezivanju
    //void check_errors(GLunit shader, std::string type)
};

#endif