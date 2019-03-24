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

//= INCLUDES =====================
#include "IComponent.h"
#include <vector>
#include <memory>
#include "../../Math/Vector3.h"
#include "../../Math/Quaternion.h"
#include "../../Math/Matrix.h"
//================================

namespace Directus
{
	class RHI_Device;
	class RHI_ConstantBuffer;

	class ENGINE_CLASS Transform : public IComponent
	{
	public:
		Transform(Context* context, Entity* entity, Transform* transform);
		~Transform() = default;

		//= ICOMPONENT ===============================
		void OnInitialize() override;
		void Serialize(FileStream* stream) override;
		void Deserialize(FileStream* stream) override;
		//============================================

		void UpdateTransform();

		//= POSITION ========================================================================
		Math::Vector3 GetPosition()						{ return m_matrix.GetTranslation(); }
		const Math::Vector3& GetPositionLocal() const	{ return m_positionLocal; }
		void SetPosition(const Math::Vector3& position);
		void SetPositionLocal(const Math::Vector3& position);
		//===================================================================================

		//= ROTATION =========================================================================
		Math::Quaternion GetRotation()						{ return m_matrix.GetRotation(); }
		const Math::Quaternion& GetRotationLocal() const	{ return m_rotationLocal; }
		void SetRotation(const Math::Quaternion& rotation);
		void SetRotationLocal(const Math::Quaternion& rotation);
		//====================================================================================

		//= SCALE =================================================================
		Math::Vector3 GetScale()					{ return m_matrix.GetScale(); }
		const Math::Vector3& GetScaleLocal() const	{ return m_scaleLocal; }
		void SetScale(const Math::Vector3& scale);
		void SetScaleLocal(const Math::Vector3& scale);
		//=========================================================================

		//= TRANSLATION/ROTATION ==================
		void Translate(const Math::Vector3& delta);
		void Rotate(const Math::Quaternion& delta);
		//=========================================

		//= DIRECTIONS ==================
		Math::Vector3 GetUp() const;
		Math::Vector3 GetForward() const;
		Math::Vector3 GetRight() const;
		//===============================

		//= HIERARCHY ==================================================================================
		bool IsRoot() const		{ return !HasParent(); }
		bool HasParent() const	{ return m_parent; }
		void SetParent(Transform* new_parent);
		void BecomeOrphan();
		bool HasChildren() const				{ return GetChildrenCount() > 0 ? true : false; }
		unsigned int GetChildrenCount() const	{ return static_cast<unsigned int>(m_children.size()); }
		void AddChild(Transform* child);
		Transform* GetRoot()			{ return HasParent() ? GetParent()->GetRoot() : this; }
		Transform* GetParent() const	{ return m_parent; }
		Transform* GetChildByIndex(unsigned int index);
		Transform* GetChildByName(const std::string& name);
		const std::vector<Transform*>& GetChildren() const	{ return m_children; }
	
		void AcquireChildren();
		bool IsDescendantOf(Transform* transform) const;
		void GetDescendants(std::vector<Transform*>* descendants);
		//==============================================================================================

		void LookAt(const Math::Vector3& v) { m_lookAt = v; }
		Math::Matrix& GetMatrix()			{ return m_matrix; }
		Math::Matrix& GetLocalMatrix()		{ return m_matrixLocal; }

		// Constant buffer
		void UpdateConstantBuffer(const std::shared_ptr<RHI_Device>& rhi_device, const Math::Matrix& m_view_projection);
		const auto& GetConstantBuffer() { return m_constant_buffer_gpu; }

	private:
		Math::Matrix GetParentTransformMatrix() const;

		// local
		Math::Vector3 m_positionLocal;
		Math::Quaternion m_rotationLocal;
		Math::Vector3 m_scaleLocal;

		Math::Matrix m_matrix;
		Math::Matrix m_matrixLocal;
		Math::Vector3 m_lookAt;

		Transform* m_parent; // the parent of this transform
		std::vector<Transform*> m_children; // the children of this transform

		// Constant buffer
		struct ConstantBufferData
		{
			Math::Matrix model;
			Math::Matrix mvp_current;
			Math::Matrix mvp_previous;
		};
		ConstantBufferData m_constant_buffer_cpu;
		std::shared_ptr<RHI_ConstantBuffer> m_constant_buffer_gpu;
		Math::Matrix m_wvp_previous;
	};
}