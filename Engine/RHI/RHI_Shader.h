/*
Copyright(c) 2016-2019 Panos Karabelas

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is furnished
to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

//= INCLUDES ==============
#include <memory>
#include <string>
#include <map>
#include "RHI_Object.h"
#include "RHI_Definition.h"
//=========================

namespace Directus
{
	// Forward declarations
	class Context;

	enum Shader_Type
	{
		Shader_Vertex,
		Shader_Pixel,
		Shader_VertexPixel
	};

	enum Shader_State
	{
		Shader_Uninitialized,
		Shader_Compiling,
		Shader_Built,
		Shader_Failed
	};

	namespace _RHI_Shader
	{
		static const std::string entry_point_vertex	= "mainVS";
		static const std::string entry_point_pixel	= "mainPS";
		static const std::string shader_model		= "5_0";
	}

	class ENGINE_CLASS RHI_Shader : public RHI_Object
	{
	public:

		RHI_Shader(std::shared_ptr<RHI_Device> rhi_device);
		~RHI_Shader();

		void Compile(Shader_Type type, const std::string& shader, const unsigned long input_layout = 0);
		void Compile_Async(Context* context, Shader_Type type, const std::string& shader, unsigned long input_layout = 0);

		void AddDefine(const std::string& define, const std::string& value = "1");

		template <typename T>
		void AddBuffer()
		{
			m_buffer_size = sizeof(T);
			CreateConstantBuffer(m_buffer_size);
		}
		bool UpdateBuffer(void* data) const;
		void* GetVertexShaderBuffer() const						{ return m_vertex_shader; }
		void* GetPixelShaderBuffer() const						{ return m_pixel_shader; }
		const auto& GetConstantBuffer()							{ return m_constant_buffer; }
		void SetName(const std::string& name)					{ m_name = name; }
		bool HasVertexShader() const							{ return m_has_shader_vertex; }
		bool HasPixelShader() const								{ return m_has_shader_pixel; }
		std::shared_ptr<RHI_InputLayout> GetInputLayout() const	{ return m_input_layout; }
		Shader_State GetState() const							{ return m_compilation_state; }

	protected:
		std::shared_ptr<RHI_Device> m_rhi_device;

	private:
		void CreateConstantBuffer(unsigned int size);
		virtual bool Compile_Vertex(const std::string& shader, unsigned long input_layout);
		virtual bool Compile_Pixel(const std::string& shader);
		void* CompileDXC(Shader_Type type, const std::string& shader);
			
		std::string m_name;
		std::string m_file_path;
		std::string m_entry_point;
		std::string m_profile;
		std::map<std::string, std::string> m_defines;
		std::shared_ptr<RHI_InputLayout> m_input_layout;
		std::shared_ptr<RHI_ConstantBuffer> m_constant_buffer;
		bool m_has_shader_vertex			= false;
		bool m_has_shader_pixel				= false;
		unsigned int m_buffer_size			= 0;
		Shader_State m_compilation_state	= Shader_Uninitialized;

		// D3D11
		void* m_vertex_shader	= nullptr;
		void* m_pixel_shader	= nullptr;
	};
}