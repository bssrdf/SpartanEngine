/*
Copyright(c) 2016-2020 Panos Karabelas

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

//= INCLUDES ============
#include "Scripting.h"
#include <memory>
//=======================

class asIScriptObject;
class asIScriptFunction;

namespace Spartan
{
	class Entity;

	// Allows creation of a script instance and execution of it's class functions.
	class ScriptInstance
	{
	public:
		ScriptInstance() = default;
		~ScriptInstance();

		bool Instantiate(const std::string& path, const std::weak_ptr<Entity>& entity, Scripting* scriptEngine);
		bool IsInstantiated() const { return m_isInstantiated; }
		const auto& GetScriptPath() const { return m_scriptPath; }

		void ExecuteStart() const;
		void ExecuteUpdate(float delta_time) const;

	private:
		bool CreateScriptObject();

		std::string m_scriptPath;
		std::string m_className;
		std::string m_constructorDeclaration;
		std::string m_moduleName;
		std::weak_ptr<Entity> m_entity;
		std::shared_ptr<Module> m_module;
		asIScriptObject* m_scriptObject				= nullptr;
		asIScriptFunction* m_constructorFunction	= nullptr;
		asIScriptFunction* m_startFunction			= nullptr;
		asIScriptFunction* m_updateFunction			= nullptr;
        Scripting* m_scripting	                    = nullptr;
		bool m_isInstantiated						= false;
	};
}
