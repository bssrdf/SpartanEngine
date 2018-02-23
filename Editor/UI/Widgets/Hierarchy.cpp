/*
Copyright(c) 2016-2018 Panos Karabelas

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

//= INCLUDES ==============================
#include "Hierarchy.h"
#include "../ImGui/imgui.h"
#include "../DragDrop.h"
#include "../EditorHelper.h"
#include "Scene/Scene.h"
#include "Scene/GameObject.h"
#include "Core/Engine.h"
#include "Input/DInput/DInput.h"
#include "Scene/Components/Transform.h"
#include "Scene/Components/Light.h"
#include "Scene/Components/AudioSource.h"
#include "Scene/Components/AudioListener.h"
#include "Scene/Components/RigidBody.h"
#include "Scene/Components/Collider.h"
#include "Scene/Components/Camera.h"
#include "Scene/Components/Constraint.h"
#include "Scene/Components/MeshFilter.h"
#include "Scene/Components/MeshRenderer.h"
#include "Properties.h"
//=========================================

//= NAMESPACES ==========
using namespace std;
using namespace Directus;
//=======================

weak_ptr<GameObject> Hierarchy::m_gameObjectSelected;
weak_ptr<GameObject> g_gameObjectEmpty;

namespace HierarchyStatics
{
	static GameObject* g_hoveredGameObject	= nullptr;
	static Engine* g_engine					= nullptr;
	static Scene* g_scene					= nullptr;
	static Input* g_input					= nullptr;
	static DragDropPayload g_payload;
}
Hierarchy::Hierarchy()
{
	m_title = "Hierarchy";
	m_context = nullptr;
	HierarchyStatics::g_scene = nullptr;
}

void Hierarchy::Initialize(Context* context)
{
	Widget::Initialize(context);

	HierarchyStatics::g_engine = m_context->GetSubsystem<Engine>();
	HierarchyStatics::g_scene = m_context->GetSubsystem<Scene>();
	HierarchyStatics::g_input = m_context->GetSubsystem<Input>();

	m_windowFlags |= ImGuiWindowFlags_HorizontalScrollbar;
}

void Hierarchy::Update()
{
	// If something is being loaded, don't parse the hierarchy
	if (EditorHelper::Get().GetEngineLoading())
		return;
	
	Tree_Show();
}

void Hierarchy::SetSelectedGameObject(weak_ptr<GameObject> gameObject)
{
	m_gameObjectSelected = gameObject;
	Properties::Inspect(m_gameObjectSelected);
}

void Hierarchy::Tree_Show()
{
	OnTreeBegin();

	if (ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_DefaultOpen))
	{
		// Dropping on the scene node should unparent the GameObject
		auto drop = DragDrop::Get().GetPayload(g_dragDrop_Type_GameObject);
		if (drop.type == g_dragDrop_Type_GameObject)
		{
			auto gameObjectID = (unsigned int)drop.data;
			if (auto droppedGameObj = HierarchyStatics::g_scene->GetGameObjectByID(gameObjectID).lock())
			{
				droppedGameObj->GetTransform()->SetParent(nullptr);
			}
		}

		auto rootGameObjects = HierarchyStatics::g_scene->GetRootGameObjects();
		for (const auto& gameObject : rootGameObjects)
		{
			Tree_AddGameObject(gameObject.lock().get());
		}

		ImGui::TreePop();
	}

	OnTreeEnd();
}

void Hierarchy::OnTreeBegin()
{
	HierarchyStatics::g_hoveredGameObject = nullptr;
}

void Hierarchy::OnTreeEnd()
{
	HandleKeyShortcuts();
	HandleClicking();
	ContextMenu();
}

void Hierarchy::Tree_AddGameObject(GameObject* gameObject)
{
	// Node self visibility
	if (!gameObject->IsVisibleInHierarchy())
		return;

	// Node children visibility
	bool hasVisibleChildren = false;
	auto children = gameObject->GetTransform()->GetChildren();
	for (const auto& child : children)
	{
		if (child->GetGameObject()->IsVisibleInHierarchy())
		{
			hasVisibleChildren = true;
			break;
		}
	}

	// Node flags -> Default
	ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_AllowItemOverlap;
	// Node flags -> Expandable?
	node_flags |= hasVisibleChildren ? ImGuiTreeNodeFlags_OpenOnArrow : ImGuiTreeNodeFlags_Leaf;
	// Node flags -> Selected?
	if (!m_gameObjectSelected.expired())
	{
		node_flags |= (m_gameObjectSelected.lock()->GetID() == gameObject->GetID()) ? ImGuiTreeNodeFlags_Selected : 0;
	}

	// Node
	bool isNodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)gameObject->GetID(), node_flags, gameObject->GetName().c_str());
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly))
	{
		HierarchyStatics::g_hoveredGameObject = gameObject;
	}

	HandleDragDrop(gameObject);	

	// Recursively show all child nodes
	if (isNodeOpen)
	{
		if (hasVisibleChildren)
		{
			for (const auto& child : children)
			{
				if (!child->GetGameObject()->IsVisibleInHierarchy())
					continue;

				Tree_AddGameObject(child->GetGameObjectRef().lock().get());
			}
		}
		ImGui::TreePop();
	}
}

void Hierarchy::HandleClicking()
{
	if (ImGui::IsMouseHoveringWindow())
	{		
		// Left click on item
		if (ImGui::IsMouseClicked(0) && HierarchyStatics::g_hoveredGameObject)
		{
			SetSelectedGameObject(HierarchyStatics::g_hoveredGameObject->GetTransform()->GetGameObjectRef());
		}

		// Right click on item
		if (ImGui::IsMouseClicked(1))
		{
			if (HierarchyStatics::g_hoveredGameObject)
			{			
				SetSelectedGameObject(HierarchyStatics::g_hoveredGameObject->GetTransform()->GetGameObjectRef());
			}

			ImGui::OpenPopup("##HierarchyContextMenu");		
		}

		// Clicking (any button) inside the window but not on an item (empty space)
		if ((ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1)) && !ImGui::IsAnyItemHovered())
		{
			SetSelectedGameObject(g_gameObjectEmpty);
		}
	}
}

void Hierarchy::HandleDragDrop(GameObject* gameObjPtr)
{
	// Drag
	if (DragDrop::Get().DragBegin())
	{
		HierarchyStatics::g_payload.data = (char*)gameObjPtr->GetID();
		HierarchyStatics::g_payload.type = g_dragDrop_Type_GameObject;
		DragDrop::Get().DragPayload(HierarchyStatics::g_payload);
		DragDrop::Get().DragEnd();
	}
	// Drop
	auto drop = DragDrop::Get().GetPayload(g_dragDrop_Type_GameObject);
	if (drop.type == g_dragDrop_Type_GameObject)
	{
		auto gameObjectID = (unsigned int)drop.data;
		if (auto droppedGameObj = HierarchyStatics::g_scene->GetGameObjectByID(gameObjectID).lock())
		{
			if (droppedGameObj->GetID() != gameObjPtr->GetID())
			{
				droppedGameObj->GetTransform()->SetParent(gameObjPtr->GetTransform());
			}
		}
	}
}

void Hierarchy::ContextMenu()
{
	if (ImGui::BeginPopup("##HierarchyContextMenu"))
	{
		if (!m_gameObjectSelected.expired())
		{
			ImGui::MenuItem("Rename");
			if (ImGui::MenuItem("Delete", "Delete"))
			{
				Action_GameObject_Delete(m_gameObjectSelected);
			}
			ImGui::Separator();
		}

		// EMPTY
		if (ImGui::MenuItem("Creaty Empty"))
		{
			Action_GameObject_CreateEmpty();
		}

		// 3D OBJECCTS
		if (ImGui::BeginMenu("3D Objects"))
		{
			if (ImGui::MenuItem("Cube"))
			{
				Action_GameObject_CreateCube();
			}
			else if (ImGui::MenuItem("Quad"))
			{
				Action_GameObject_CreateQuad();
			}
			else if (ImGui::MenuItem("Sphere"))
			{
				Action_GameObject_CreateSphere();
			}
			else if (ImGui::MenuItem("Cylinder"))
			{
				Action_GameObject_CreateCylinder();
			}
			else if (ImGui::MenuItem("Cone"))
			{
				Action_GameObject_CreateCone();
			}

			ImGui::EndMenu();
		}

		// CAMERA
		if (ImGui::MenuItem("Camera"))
		{
			Action_GameObject_CreateCamera();
		}

		// LIGHT
		if (ImGui::BeginMenu("Light"))
		{
			if (ImGui::MenuItem("Directional"))
			{
				Action_GameObject_CreateLightDirectional();
			}
			else if (ImGui::MenuItem("Point"))
			{
				Action_GameObject_CreateLightPoint();
			}
			else if (ImGui::MenuItem("Spot"))
			{
				Action_GameObject_CreateLightSpot();
			}

			ImGui::EndMenu();
		}

		// PHYSICS
		if (ImGui::BeginMenu("Physics"))
		{
			if (ImGui::MenuItem("Rigid Body"))
			{
				Action_GameObject_CreateRigidBody();
			}
			else if (ImGui::MenuItem("Collider"))
			{
				Action_GameObject_CreateCollider();
			}
			else if (ImGui::MenuItem("Constraint"))
			{
				Action_GameObject_CreateConstraint();
			}

			ImGui::EndMenu();
		}

		// AUDIO
		if (ImGui::BeginMenu("Audio"))
		{
			if (ImGui::MenuItem("Audio Source"))
			{
				Action_GameObject_CreateAudioSource();
			}
			else if (ImGui::MenuItem("Audio Listener"))
			{
				Action_GameObject_CreateAudioListener();
			}

			ImGui::EndMenu();
		}

		ImGui::EndPopup();
	}
}

void Hierarchy::HandleKeyShortcuts()
{
	if (HierarchyStatics::g_input->GetButtonKeyboard(Delete))
	{
		Action_GameObject_Delete(m_gameObjectSelected);
	}
}

void Hierarchy::Action_GameObject_Delete(weak_ptr<GameObject> gameObject)
{
	HierarchyStatics::g_scene->RemoveGameObject(gameObject);
}

GameObject* Hierarchy::Action_GameObject_CreateEmpty()
{
	auto gameObject = HierarchyStatics::g_scene->CreateGameObject().lock().get();
	if (auto selected = m_gameObjectSelected.lock())
	{
		gameObject->GetTransform()->SetParent(selected->GetTransform());
	}

	return gameObject;
}

void Hierarchy::Action_GameObject_CreateCube()
{
	auto gameObject = Action_GameObject_CreateEmpty();
	gameObject->AddComponent<MeshFilter>().lock()->UseStandardMesh(MeshType_Cube);
	gameObject->AddComponent<MeshRenderer>().lock()->UseStandardMaterial();
	gameObject->SetName("Cube");
}

void Hierarchy::Action_GameObject_CreateQuad()
{
	auto gameObject = Action_GameObject_CreateEmpty();
	gameObject->AddComponent<MeshFilter>().lock()->UseStandardMesh(MeshType_Quad);
	gameObject->AddComponent<MeshRenderer>().lock()->UseStandardMaterial();
	gameObject->SetName("Quad");
}

void Hierarchy::Action_GameObject_CreateSphere()
{
	auto gameObject = Action_GameObject_CreateEmpty();
	gameObject->AddComponent<MeshFilter>().lock()->UseStandardMesh(MeshType_Sphere);
	gameObject->AddComponent<MeshRenderer>().lock()->UseStandardMaterial();
	gameObject->SetName("Sphere");
}

void Hierarchy::Action_GameObject_CreateCylinder()
{
	auto gameObject = Action_GameObject_CreateEmpty();
	gameObject->AddComponent<MeshFilter>().lock()->UseStandardMesh(MeshType_Cylinder);
	gameObject->AddComponent<MeshRenderer>().lock()->UseStandardMaterial();
	gameObject->SetName("Cylinder");
}

void Hierarchy::Action_GameObject_CreateCone()
{
	auto gameObject = Action_GameObject_CreateEmpty();
	gameObject->AddComponent<MeshFilter>().lock()->UseStandardMesh(MeshType_Cone);
	gameObject->AddComponent<MeshRenderer>().lock()->UseStandardMaterial();
	gameObject->SetName("Cone");
}

void Hierarchy::Action_GameObject_CreateCamera()
{
	auto gameObject = Action_GameObject_CreateEmpty();
	gameObject->AddComponent<Camera>();
	gameObject->SetName("Camera");
}

void Hierarchy::Action_GameObject_CreateLightDirectional()
{
	auto gameObject = Action_GameObject_CreateEmpty();
	gameObject->AddComponent<Light>().lock()->SetLightType(LightType_Directional);
	gameObject->SetName("Directional");
}

void Hierarchy::Action_GameObject_CreateLightPoint()
{
	auto gameObject = Action_GameObject_CreateEmpty();
	gameObject->AddComponent<Light>().lock()->SetLightType(LightType_Point);
	gameObject->SetName("Point");
}

void Hierarchy::Action_GameObject_CreateLightSpot()
{
	auto gameObject = Action_GameObject_CreateEmpty();
	gameObject->AddComponent<Light>().lock()->SetLightType(LightType_Spot);
	gameObject->SetName("Spot");
}

void Hierarchy::Action_GameObject_CreateRigidBody()
{
	auto gameObject = Action_GameObject_CreateEmpty();
	gameObject->AddComponent<RigidBody>();
	gameObject->SetName("RigidBody");
}

void Hierarchy::Action_GameObject_CreateCollider()
{
	auto gameObject = Action_GameObject_CreateEmpty();
	gameObject->AddComponent<Collider>();
	gameObject->SetName("Collider");
}

void Hierarchy::Action_GameObject_CreateConstraint()
{
	auto gameObject = Action_GameObject_CreateEmpty();
	gameObject->AddComponent<Constraint>();
	gameObject->SetName("Constraint");
}

void Hierarchy::Action_GameObject_CreateAudioSource()
{
	auto gameObject = Action_GameObject_CreateEmpty();
	gameObject->AddComponent<AudioSource>();
	gameObject->SetName("AudioSource");
}

void Hierarchy::Action_GameObject_CreateAudioListener()
{
	auto gameObject = Action_GameObject_CreateEmpty();
	gameObject->AddComponent<AudioListener>();
	gameObject->SetName("AudioListener");
}
