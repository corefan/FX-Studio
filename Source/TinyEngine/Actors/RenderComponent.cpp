#include "RenderComponent.h"
#include "Actor.h"
#include "TransformComponent.h"
#include "../EventManager/Events.h"
#include "boost/lexical_cast.hpp"

const std::string GridRenderComponent::m_Name = "GridRenderComponent";
const std::string ModelRenderComponent::m_Name = "ModelRenderComponent";
const std::string SkyboxRenderComponent::m_Name = "SkyboxRenderComponent";
const std::string DirectionalLightComponent::m_Name = "DirectionalLightComponent";
const std::string PointLightComponent::m_Name = "PointLightComponent";
const std::string SpotLightComponent::m_Name = "SpotLightComponent";

BaseRenderComponent::BaseRenderComponent() : m_pSceneNode(nullptr)
{
}

BaseRenderComponent::~BaseRenderComponent()
{
}

bool BaseRenderComponent::VInit(tinyxml2::XMLElement* pData)
{
	tinyxml2::XMLElement* pColorNode = pData->FirstChildElement("Color");
	if (pColorNode != nullptr)
	{
		m_Color = LoadColor(pColorNode);
	}

	return VDelegateInit(pData);
}

void BaseRenderComponent::VPostInit()
{
	shared_ptr<SceneNode> pSceneNode(VGetSceneNode());
	shared_ptr<EvtData_New_Render_Component> pEvent(DEBUG_NEW EvtData_New_Render_Component(m_pOwner->GetActorId(), pSceneNode));
	IEventManager::Get()->VTriggerEvent(pEvent);
}

void BaseRenderComponent::VOnChanged()
{
	shared_ptr<EvtData_Modified_Render_Component> pEvent(DEBUG_NEW EvtData_Modified_Render_Component(m_pOwner->GetActorId()));
	IEventManager::Get()->VTriggerEvent(pEvent);
}

tinyxml2::XMLElement* BaseRenderComponent::VGenerateXml(tinyxml2::XMLDocument* pDocument)
{
	tinyxml2::XMLElement* pBaseElement = VCreateBaseElement(pDocument);

	tinyxml2::XMLElement* pColor = pDocument->NewElement("Color");
	pColor->SetAttribute("r", m_Color.x);
	pColor->SetAttribute("g", m_Color.y);
	pColor->SetAttribute("b", m_Color.z);
	pColor->SetAttribute("a", m_Color.w);
	pBaseElement->LinkEndChild(pColor);

	VCreateInheritedXmlElement(pBaseElement, pDocument);
	return pBaseElement;
}

Color BaseRenderComponent::LoadColor(tinyxml2::XMLElement* pData)
{
	Color color;

	if (pData != nullptr)
	{
		color.x = pData->FloatAttribute("r");
		color.y = pData->FloatAttribute("g");
		color.z = pData->FloatAttribute("b");
		color.w = pData->FloatAttribute("a");
	}

	return color;
}

shared_ptr<SceneNode> BaseRenderComponent::VGetSceneNode()
{
	if (m_pSceneNode == nullptr)
	{
		m_pSceneNode = VCreateSceneNode();
	}
	return m_pSceneNode;
}

GridRenderComponent::GridRenderComponent()
{

}

GridRenderComponent::~GridRenderComponent()
{

}

bool GridRenderComponent::VDelegateInit(tinyxml2::XMLElement* pData)
{
	return true;
}

shared_ptr<SceneNode> GridRenderComponent::VCreateSceneNode()
{
// 	shared_ptr<TransformComponent> pTransformComponent =
// 		MakeStrongPtr(m_pOwner->GetComponent<TransformComponent>(TransformComponent::m_Name));
// 	if (pTransformComponent)
// 	{
// 		WeakBaseRenderComponentPtr weakThis(this);
// 
// 		switch (BaseGameApp::GetRendererImpl())
// 		{
// 		case BaseGameApp::Renderer_D3D11:
// 			return shared_ptr<SceneNode>(DEBUG_NEW GridNode(m_pOwner->GetActorId(), weakThis, pTransformComponent->GetTransform()));
// 
// 		default:
// 			DEBUG_ERROR("Unknown Renderer Implementation in GridRenderComponent");
// 		}
// 	}

	return shared_ptr<SceneNode>();
}

void GridRenderComponent::VCreateInheritedXmlElement(tinyxml2::XMLElement* pBaseElement, tinyxml2::XMLDocument* pDocument)
{

}

ModelRenderComponent::ModelRenderComponent()
	: m_ModelName(),
	m_TextureName()
{

}

ModelRenderComponent::~ModelRenderComponent()
{

}

bool ModelRenderComponent::VDelegateInit(tinyxml2::XMLElement* pData)
{
	tinyxml2::XMLElement* pModel = pData->FirstChildElement("Model");
	if (pModel != nullptr)
	{
		m_ModelName = pModel->FirstChild()->Value();
	}

	tinyxml2::XMLElement* pTexture = pData->FirstChildElement("Texture");
	if (pTexture != nullptr)
	{
		m_TextureName = pTexture->FirstChild()->Value();
	}

	tinyxml2::XMLElement* pEffect = pData->FirstChildElement("Effect");
	if (pEffect != nullptr)
	{
		m_EffectName = pEffect->FirstChild()->Value();
	}

	return true;
}

shared_ptr<SceneNode> ModelRenderComponent::VCreateSceneNode()
{
// 	shared_ptr<TransformComponent> pTransformComponent =
// 		MakeStrongPtr(m_pOwner->GetComponent<TransformComponent>(TransformComponent::m_Name));
// 	if (pTransformComponent)
// 	{
// 		WeakBaseRenderComponentPtr weakThis(this);
// 
// 		switch (BaseGameApp::GetRendererImpl())
// 		{
// 		case BaseGameApp::Renderer_D3D11:
// 		{
// 			std::string extension =  m_ModelName.substr(m_ModelName.find_last_of('.'));
// 			if (!extension.compare("sdkmesh"))
// 			{
// 				return shared_ptr<SceneNode>(DEBUG_NEW D3DShaderMeshNode11(
// 					m_pOwner->GetActorId(), weakThis, RenderPass_Actor, pTransformComponent->GetTransform()));
// 			}
// 			else
// 			{
// 				return shared_ptr<SceneNode>(DEBUG_NEW ModelNode(
// 					m_pOwner->GetActorId(), weakThis, RenderPass_Actor, pTransformComponent->GetTransform()));
// 			}
// 		}
// 
// 		default:
// 			DEBUG_ERROR("Unknown Renderer Implementation in ModelRenderComponent");
// 		}
// 	}

	return shared_ptr<SceneNode>();
}

void ModelRenderComponent::VCreateInheritedXmlElement(tinyxml2::XMLElement* pBaseElement, tinyxml2::XMLDocument* pDocument)
{
	tinyxml2::XMLElement* pSdkMesh = pDocument->NewElement("Model");
	tinyxml2::XMLText* pSdkMeshName = pDocument->NewText(m_ModelName.c_str());
	pSdkMesh->LinkEndChild(pSdkMeshName);
	pBaseElement->LinkEndChild(pSdkMesh);

	tinyxml2::XMLElement* pTexture = pDocument->NewElement("Texture");
	tinyxml2::XMLText* pTextureName = pDocument->NewText(m_TextureName.c_str());
	pTexture->LinkEndChild(pTextureName);
	pBaseElement->LinkEndChild(pTexture);

	tinyxml2::XMLElement* pEffect = pDocument->NewElement("Effect");
	tinyxml2::XMLText* pEffectName = pDocument->NewText(m_EffectName.c_str());
	pEffect->LinkEndChild(pEffectName);
	pBaseElement->LinkEndChild(pEffect);
}

SkyboxRenderComponent::SkyboxRenderComponent()
{

}

SkyboxRenderComponent::~SkyboxRenderComponent()
{

}

bool SkyboxRenderComponent::VDelegateInit(tinyxml2::XMLElement* pData)
{
	tinyxml2::XMLElement* pTexture = pData->FirstChildElement("Texture");
	if (pTexture)
	{
// 		m_textureResource = pTexture->FirstChild()->Value();
	}
	return true;
}

shared_ptr<SceneNode> SkyboxRenderComponent::VCreateSceneNode()
{
// 	shared_ptr<SkyboxNode> sky;
// 	if (BaseGameApp::GetRendererImpl() == BaseGameApp::Renderer_D3D11)
// 	{
// 		sky = shared_ptr<SkyboxNode>(DEBUG_NEW SkyboxNode(L"Assets\\Textures\\Plains_of_abraham.dds"));
// 	}
// 	else
// 	{
// 		DEBUG_ERROR("Unknown Renderer Implementation in SkyboxRenderComponent");
// 	}
// 	return sky;

	return shared_ptr<SceneNode>();
}

void SkyboxRenderComponent::VCreateInheritedXmlElement(tinyxml2::XMLElement* pBaseElement, tinyxml2::XMLDocument* pDocument)
{
// 	tinyxml2::XMLElement* pTextureNode = pDocument->NewElement("Texture");
// 	tinyxml2::XMLText* pTextureText = pDocument->NewText(m_textureResource.c_str());
// 	pTextureNode->LinkEndChild(pTextureText);
// 	pBaseElement->LinkEndChild(pTextureNode);
}

LightRenderComponent::LightRenderComponent()
{

}

LightRenderComponent::~LightRenderComponent()
{

}

bool LightRenderComponent::VDelegateInit(tinyxml2::XMLElement* pData)
{
	tinyxml2::XMLElement* pModel = pData->FirstChildElement("Model");
	if (pModel != nullptr)
	{
		m_ModelName = pModel->FirstChild()->Value();
	}

	return true;
}

void LightRenderComponent::VCreateInheritedXmlElement(tinyxml2::XMLElement* pBaseElement, tinyxml2::XMLDocument* pDocument)
{
	DEBUG_ASSERT(pBaseElement);
	if (pBaseElement == nullptr)
	{
		return;
	}

	tinyxml2::XMLElement* pModelNode = pDocument->NewElement("Model");
	tinyxml2::XMLText* pModelName = pDocument->NewText(m_ModelName.c_str());
	pModelNode->LinkEndChild(pModelName);
	pBaseElement->LinkEndChild(pModelNode);
}

DirectionalLightComponent::DirectionalLightComponent()
{

}

DirectionalLightComponent::~DirectionalLightComponent()
{

}

bool DirectionalLightComponent::VDelegateInit(tinyxml2::XMLElement* pData)
{
	LightRenderComponent::VDelegateInit(pData);

	float x = 0;
	float y = 0;
	float z = 0;
	tinyxml2::XMLElement* pLight = pData->FirstChildElement("Light");
	tinyxml2::XMLElement* pDirectionNode = pLight->FirstChildElement("Direction");
	if (pDirectionNode != nullptr)
	{
		pDirectionNode->QueryFloatAttribute("x", &x);
		pDirectionNode->QueryFloatAttribute("y", &y);
		pDirectionNode->QueryFloatAttribute("z", &z);
		m_Direction = Vector3(x, y, z);
	}

	tinyxml2::XMLElement* pUpNode = pLight->FirstChildElement("Up");
	if (pUpNode != nullptr)
	{
		pUpNode->QueryFloatAttribute("x", &x);
		pUpNode->QueryFloatAttribute("y", &y);
		pUpNode->QueryFloatAttribute("z", &z);
		m_Up = Vector3(x, y, z);
	}

	tinyxml2::XMLElement* pRightNode = pLight->FirstChildElement("Right");
	if (pRightNode != nullptr)
	{
		pRightNode->QueryFloatAttribute("x", &x);
		pRightNode->QueryFloatAttribute("y", &y);
		pRightNode->QueryFloatAttribute("z", &z);
		m_Right = Vector3(x, y, z);
	}

	return true;
}

shared_ptr<SceneNode> DirectionalLightComponent::VCreateSceneNode()
{
// 	shared_ptr<TransformComponent> pTransformComponent =
// 		MakeStrongPtr(m_pOwner->GetComponent<TransformComponent>(TransformComponent::m_Name));
// 	if (pTransformComponent)
// 	{
// 		WeakBaseRenderComponentPtr weakThis(this);
// 
// 		switch (BaseGameApp::GetRendererImpl())
// 		{
// 		case BaseGameApp::Renderer_D3D11:
// 		{
// 			return shared_ptr<SceneNode>(DEBUG_NEW DirectionalLightNode(
// 				m_pOwner->GetActorId(), weakThis, RenderPass_Actor, pTransformComponent->GetTransform()));
// 		}
// 
// 		default:
// 			DEBUG_ERROR("Unknown Renderer Implementation in LightRenderComponent");
// 		}
// 	}

	return shared_ptr<SceneNode>();
}

void DirectionalLightComponent::VCreateInheritedXmlElement(tinyxml2::XMLElement* pBaseElement, tinyxml2::XMLDocument* pDocument)
{
	LightRenderComponent::VCreateInheritedXmlElement(pBaseElement, pDocument);

	tinyxml2::XMLElement* pSceneNode = pDocument->NewElement("Light");

	tinyxml2::XMLElement* pDirectionNode = pDocument->NewElement("Direction");
	pDirectionNode->SetAttribute("x", m_Direction.x);
	pDirectionNode->SetAttribute("y", m_Direction.y);
	pDirectionNode->SetAttribute("z", m_Direction.z);
	pSceneNode->LinkEndChild(pDirectionNode);

	tinyxml2::XMLElement* pUpNode = pDocument->NewElement("Up");
	pUpNode->SetAttribute("x", m_Up.x);
	pUpNode->SetAttribute("y", m_Up.y);
	pUpNode->SetAttribute("z", m_Up.z);
	pSceneNode->LinkEndChild(pUpNode);

	tinyxml2::XMLElement* pRightNode = pDocument->NewElement("Right");
	pRightNode->SetAttribute("x", m_Right.x);
	pRightNode->SetAttribute("y", m_Right.y);
	pRightNode->SetAttribute("z", m_Right.z);
	pSceneNode->LinkEndChild(pRightNode);

	pBaseElement->LinkEndChild(pSceneNode);
}

PointLightComponent::PointLightComponent()
{

}

PointLightComponent::~PointLightComponent()
{

}

bool PointLightComponent::VDelegateInit(tinyxml2::XMLElement* pData)
{
	LightRenderComponent::VDelegateInit(pData);

	tinyxml2::XMLElement* pLight = pData->FirstChildElement("Light");
	tinyxml2::XMLElement* pShapeNode = pLight->FirstChildElement("Shape");
	if (pShapeNode != nullptr)
	{
		m_Radius = pShapeNode->FloatAttribute("Radius");
	}

	return true;
}

shared_ptr<SceneNode> PointLightComponent::VCreateSceneNode()
{
// 	shared_ptr<TransformComponent> pTransformComponent =
// 		MakeStrongPtr(m_pOwner->GetComponent<TransformComponent>(TransformComponent::m_Name));
// 	if (pTransformComponent)
// 	{
// 		WeakBaseRenderComponentPtr weakThis(this);
// 
// 		switch (BaseGameApp::GetRendererImpl())
// 		{
// 		case BaseGameApp::Renderer_D3D11:
// 		{
// 			// 			return shared_ptr<SceneNode>(DEBUG_NEW D3DShaderMeshNode11(
// 			// 				m_pOwner->GetActorId(), weakThis, RenderPass_Actor, pTransformComponent->GetTransform()));
// 		}
// 
// 		default:
// 			DEBUG_ERROR("Unknown Renderer Implementation in LightRenderComponent");
// 		}
// 	}

	return shared_ptr<SceneNode>();
}

void PointLightComponent::VCreateInheritedXmlElement(tinyxml2::XMLElement* pBaseElement, tinyxml2::XMLDocument* pDocument)
{
	LightRenderComponent::VCreateInheritedXmlElement(pBaseElement, pDocument);

	tinyxml2::XMLElement* pSceneNode = pDocument->NewElement("Light");
	tinyxml2::XMLElement* pShapeNode = pDocument->NewElement("Shape");
	pShapeNode->SetAttribute("Radius", m_Radius);
	pSceneNode->LinkEndChild(pShapeNode);

	pBaseElement->LinkEndChild(pSceneNode);
}

SpotLightComponent::SpotLightComponent()
{

}

SpotLightComponent::~SpotLightComponent()
{

}

bool SpotLightComponent::VDelegateInit(tinyxml2::XMLElement* pData)
{
	LightRenderComponent::VDelegateInit(pData);

	float x = 0;
	float y = 0;
	float z = 0;
	tinyxml2::XMLElement* pLight = pData->FirstChildElement("Light");
	tinyxml2::XMLElement* pDirectionNode = pLight->FirstChildElement("Direction");
	if (pDirectionNode != nullptr)
	{
		pDirectionNode->QueryFloatAttribute("x", &x);
		pDirectionNode->QueryFloatAttribute("y", &y);
		pDirectionNode->QueryFloatAttribute("z", &z);
		m_Direction = Vector3(x, y, z);
	}

	tinyxml2::XMLElement* pUpNode = pLight->FirstChildElement("Up");
	if (pUpNode != nullptr)
	{
		pUpNode->QueryFloatAttribute("x", &x);
		pUpNode->QueryFloatAttribute("y", &y);
		pUpNode->QueryFloatAttribute("z", &z);
		m_Up = Vector3(x, y, z);
	}

	tinyxml2::XMLElement* pRightNode = pLight->FirstChildElement("Right");
	if (pRightNode != nullptr)
	{
		pRightNode->QueryFloatAttribute("x", &x);
		pRightNode->QueryFloatAttribute("y", &y);
		pRightNode->QueryFloatAttribute("z", &z);
		m_Right = Vector3(x, y, z);
	}

	tinyxml2::XMLElement* pShapeNode = pLight->FirstChildElement("Shape");
	if (pShapeNode != nullptr)
	{
		m_Radius = pShapeNode->FloatAttribute("Radius");
		m_InnerAngle = pShapeNode->FloatAttribute("InnerAngle");
		m_OuterAngle = pShapeNode->FloatAttribute("OuterAngle");
	}

	return true;
}

shared_ptr<SceneNode> SpotLightComponent::VCreateSceneNode()
{
// 	shared_ptr<TransformComponent> pTransformComponent =
// 		MakeStrongPtr(m_pOwner->GetComponent<TransformComponent>(TransformComponent::m_Name));
// 	if (pTransformComponent)
// 	{
// 		WeakBaseRenderComponentPtr weakThis(this);
// 
// 		switch (BaseGameApp::GetRendererImpl())
// 		{
// 		case BaseGameApp::Renderer_D3D11:
// 		{
// 			// 			return shared_ptr<SceneNode>(DEBUG_NEW D3DShaderMeshNode11(
// 			// 				m_pOwner->GetActorId(), weakThis, RenderPass_Actor, pTransformComponent->GetTransform()));
// 		}
// 
// 		default:
// 			DEBUG_ERROR("Unknown Renderer Implementation in LightRenderComponent");
// 		}
// 	}

	return shared_ptr<SceneNode>();
}

void SpotLightComponent::VCreateInheritedXmlElement(tinyxml2::XMLElement* pBaseElement, tinyxml2::XMLDocument* pDocument)
{
	LightRenderComponent::VCreateInheritedXmlElement(pBaseElement, pDocument);

	tinyxml2::XMLElement* pSceneNode = pDocument->NewElement("Light");

	tinyxml2::XMLElement* pDirectionNode = pDocument->NewElement("Direction");
	pDirectionNode->SetAttribute("x", m_Direction.x);
	pDirectionNode->SetAttribute("y", m_Direction.y);
	pDirectionNode->SetAttribute("z", m_Direction.z);
	pSceneNode->LinkEndChild(pDirectionNode);

	tinyxml2::XMLElement* pUpNode = pDocument->NewElement("Up");
	pUpNode->SetAttribute("x", m_Up.x);
	pUpNode->SetAttribute("y", m_Up.y);
	pUpNode->SetAttribute("z", m_Up.z);
	pSceneNode->LinkEndChild(pUpNode);

	tinyxml2::XMLElement* pRightNode = pDocument->NewElement("Right");
	pRightNode->SetAttribute("x", m_Right.x);
	pRightNode->SetAttribute("y", m_Right.y);
	pRightNode->SetAttribute("z", m_Right.z);
	pSceneNode->LinkEndChild(pRightNode);

	tinyxml2::XMLElement* pShapeNode = pDocument->NewElement("Shape");
	pShapeNode->SetAttribute("Radius", m_Radius);
	pShapeNode->SetAttribute("InnerAngle", m_InnerAngle);
	pShapeNode->SetAttribute("OuterAngle", m_OuterAngle);
	pSceneNode->LinkEndChild(pShapeNode);

	pBaseElement->LinkEndChild(pSceneNode);
}