#include "SceneNode.h"
#include "CameraNode.h"
#include "Scene.h"
#include "Model.h"
#include "Material.h"
#include "../Actors/Actor.h"
#include "../Actors/RenderComponent.h"
#include "../Actors/TransformComponent.h"
#include "../ResourceCache/ResCache.h"
#include "../ResourceCache/TextureResource.h"
#include "../ResourceCache/ShaderResource.h"
#include "../AppFramework/BaseGameApp.h"
#include "../AppFramework/BaseGameLogic.h"
#include <DirectXColors.h>

SceneNodeProperties::SceneNodeProperties()
	: m_ActorId(INVALID_ACTOR_ID),
	m_RenderPass(RenderPass_0),
	m_worldMatrix(Matrix::Identity)
{

}

SceneNode::SceneNode(ActorId actorId, WeakBaseRenderComponentPtr renderComponent, RenderPass renderPass, const Matrix& worldMatrix)
	: m_pParent(nullptr),
	m_pRenderComponent(renderComponent)
{
	m_Properties.m_ActorId = actorId;
	m_Properties.m_ActorName = (renderComponent != nullptr) ? renderComponent->VGetComponentName() : "SceneNode";
	m_Properties.m_RenderPass = renderPass;
	VSetTransform(worldMatrix);
	m_Properties.m_AABox.Center = Vector3(0.0f, 0.0f, 0.0f);
	m_Properties.m_AABox.Extents = Vector3(0.0f, 0.0f, 0.0f);
}

SceneNode::~SceneNode()
{
}

void SceneNode::VSetTransform(const Matrix& worldMatrix)
{
	m_Properties.m_worldMatrix = worldMatrix;

	BaseGameLogic* pGameLogic = g_pApp->GetGameLogic();
	if (pGameLogic == nullptr)
		return;

	StrongActorPtr pActor = MakeStrongPtr(pGameLogic->VGetActor(m_Properties.GetActorId()));
	if (pActor != nullptr)
	{
		shared_ptr<TransformComponent> pTransform = MakeStrongPtr(pActor->GetComponent<TransformComponent>(TransformComponent::m_Name));
		if (pTransform != nullptr)
		{
			pTransform->SetTransform(worldMatrix);
		}
	}
}

HRESULT SceneNode::VOnInitSceneNode(Scene* pScene)
{
	Color color = (m_pRenderComponent) ? m_pRenderComponent->GetColor() : Color(Colors::White.f);

	for (auto& child : m_Children)
	{
		child->VOnInitSceneNode(pScene);
	}
	return S_OK;
}

HRESULT SceneNode::VOnDeleteSceneNode(Scene *pScene)
{
	for (auto& child : m_Children)
	{
		child->VOnDeleteSceneNode(pScene);
	}
	return S_OK;
}

HRESULT SceneNode::VOnUpdate(Scene* pScene, const GameTime& gameTime)
{
	for (auto& child : m_Children)
	{
		child->VOnUpdate(pScene, gameTime);
	}
	return S_OK;
}

HRESULT SceneNode::VPreRender(Scene* pScene)
{
	StrongActorPtr pActor = MakeStrongPtr(g_pApp->GetGameLogic()->VGetActor(m_Properties.GetActorId()));
	if (pActor != nullptr)
	{
		shared_ptr<TransformComponent> pTransform = MakeStrongPtr(pActor->GetComponent<TransformComponent>(TransformComponent::m_Name));
		if (pTransform != nullptr)
		{
			m_Properties.m_worldMatrix = pTransform->GetTransform();
		}
	}

	pScene->PushAndSetMatrix(m_Properties.m_worldMatrix);

	return S_OK;
}

HRESULT SceneNode::VRender(Scene* pScene, const GameTime& gameTime)
{
	return S_OK;
}

HRESULT SceneNode::VRenderChildren(Scene* pScene, const GameTime& gameTime)
{
	for (auto& child : m_Children)
	{
		if (child->VPreRender(pScene) == S_OK)
		{
			if (child->VIsVisible(pScene))
			{
				child->VRender(pScene, gameTime);
				child->VRenderChildren(pScene, gameTime);
			}
		}
		child->VPostRender(pScene);
	}

	return S_OK;
}

HRESULT SceneNode::VPostRender(Scene* pScene)
{
	pScene->PopMatrix();
	return S_OK;
}

bool SceneNode::VIsVisible(Scene* pScene) const
{
	return true;
}

bool SceneNode::VAddChild(shared_ptr<ISceneNode> child)
{
	m_Children.push_back(child);
	shared_ptr<SceneNode> kid = static_pointer_cast<SceneNode>(child);
	kid->m_pParent = this;
	
	return true;
}

bool SceneNode::VRemoveChild(ActorId actorId)
{
	for (SceneNodeList::iterator i = m_Children.begin(); i != m_Children.end(); ++i)
	{
		const SceneNodeProperties* pProps = (*i)->VGet();
		if (pProps->GetActorId() != INVALID_ACTOR_ID && actorId == pProps->GetActorId())
		{
			i = m_Children.erase(i);
			return true;
		}
	}
	return false;
}

void SceneNode::VPick(Scene* pScene, int cursorX, int cursorY)
{
	for (auto& child : m_Children)
	{
		child->VPick(pScene, cursorX, cursorY);
	}
}

void SceneNode::SetBoundingBox(const BoundingBox& aabb)
{
	m_Properties.m_AABox = aabb;
}

void SceneNode::SetBoundingSphere(const BoundingSphere& sphere)
{
	m_Properties.m_Sphere = sphere;
}

RootNode::RootNode()
	: SceneNode(INVALID_ACTOR_ID, WeakBaseRenderComponentPtr(), RenderPass_0)
{
	m_Children.reserve(RenderPass_Last);

	shared_ptr<SceneNode> staticGroup(DEBUG_NEW SceneNode(INVALID_ACTOR_ID, WeakBaseRenderComponentPtr(), RenderPass_Static));
	m_Children.push_back(staticGroup);

	shared_ptr<SceneNode> actorGroup(DEBUG_NEW SceneNode(INVALID_ACTOR_ID, WeakBaseRenderComponentPtr(), RenderPass_Actor));
	m_Children.push_back(actorGroup);

	shared_ptr<SceneNode> skyGroup(DEBUG_NEW SceneNode(INVALID_ACTOR_ID, WeakBaseRenderComponentPtr(), RenderPass_Sky));
	m_Children.push_back(skyGroup);

	shared_ptr<SceneNode> debugGroup(DEBUG_NEW SceneNode(INVALID_ACTOR_ID, WeakBaseRenderComponentPtr(), RenderPass_Debug));
	m_Children.push_back(debugGroup);

	shared_ptr<SceneNode> invisibleGroup(DEBUG_NEW SceneNode(INVALID_ACTOR_ID, WeakBaseRenderComponentPtr(), RenderPass_NotRendered));
	m_Children.push_back(invisibleGroup);
}

RootNode::~RootNode()
{

}

HRESULT RootNode::VRenderChildren(Scene* pScene, const GameTime& gameTime)
{
	for (int pass = RenderPass_0; pass < RenderPass_Last; ++pass)
	{
		switch (pass)
		{
		case RenderPass_Static:
		case RenderPass_Actor:
		{
			m_Children[pass]->VRenderChildren(pScene, gameTime);
			break;
		}

		case RenderPass_Sky:
		{
			shared_ptr<IRenderState> skyPass = pScene->GetRenderder()->VPrepareSkyBoxPass();
			m_Children[pass]->VRenderChildren(pScene, gameTime);
			break;
		}
		case  RenderPass_Debug:
		{
			m_Children[pass]->VRenderChildren(pScene, gameTime);
			break;
		}
		}
	}

	return S_OK;
}

bool RootNode::VAddChild(shared_ptr<ISceneNode> child)
{
	RenderPass pass = child->VGet()->GetRenderPass();
	if ((unsigned)pass >= m_Children.size() || !m_Children[pass])
	{
		DEBUG_ASSERT(0 && _T("There is no such render pass"));
		return false;
	}

	return m_Children[pass]->VAddChild(child);
}

bool RootNode::VRemoveChild(ActorId actorId)
{
	bool anythingRemoved = false;
	for (int i = RenderPass_0; i < RenderPass_Last; ++i)
	{
		if (m_Children[i]->VRemoveChild(actorId))
		{
			anythingRemoved = true;
		}
	}
	return anythingRemoved;
}

void RootNode::VPick(Scene* pScene, int cursorX, int cursorY)
{
	if (m_Children.size() > RenderPass_Actor)
	{
		m_Children[RenderPass_Actor]->VPick(pScene, cursorX, cursorY);
	}
}

GridNode::GridNode(ActorId actorId, WeakBaseRenderComponentPtr renderComponent)
	: SceneNode(actorId, renderComponent, RenderPass_Static),
	m_pEffect(nullptr),
	m_pCurrentPass(nullptr),
	m_pVertexBuffer(nullptr),
	m_pIndexBuffer(nullptr),
	m_VertexCount(0),
	m_IndexCount(0)
{
	GridRenderComponent* pGridRender = static_cast<GridRenderComponent*>(m_pRenderComponent);
	if (pGridRender != nullptr)
	{
		m_TextureName = pGridRender->GetTextureName();
		m_GridSize = pGridRender->GetGridSize();
		m_TicksInterval = pGridRender->GetTicksInterval();
	}

	Resource effectRes("Effects\\Grid.fx");
	shared_ptr<ResHandle> pEffectResHandle = g_pApp->GetResCache()->GetHandle(&effectRes);
	if (pEffectResHandle != nullptr)
	{
		shared_ptr<HlslResourceExtraData> extra = static_pointer_cast<HlslResourceExtraData>(pEffectResHandle->GetExtraData());
		if (extra != nullptr)
		{
			m_pEffect = extra->GetEffect();
		}
	}

	InitGridVertex();
}

GridNode::~GridNode()
{
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pIndexBuffer);
}

void GridNode::InitGridVertex()
{
	Technique* pCurrentTechnique = m_pEffect->GetTechniquesByName().at("main11");
	if (pCurrentTechnique == nullptr)
	{
		DEBUG_ERROR(std::string("technique is not exist: ") + "main11");
	}
	m_pCurrentPass = pCurrentTechnique->GetPassesByName().at("p0");
	if (m_pCurrentPass == nullptr)
	{
		DEBUG_ERROR(std::string("technique is not exist: ") + "p0");
	}

	Vector2 halfSize = m_GridSize * 0.5f;

	uint16_t rows = static_cast<uint16_t>(m_GridSize.x / m_TicksInterval);
	uint16_t columns = static_cast<uint16_t>(m_GridSize.y / m_TicksInterval);
	m_VertexCount = (rows + 1) * (columns + 1);
	m_IndexCount = rows * columns * 2 * 3;

	std::vector<VertexPositionTexture> vertices;
	vertices.resize(m_VertexCount);

	for (int i = 0; i < (rows + 1); i++)
	{
		for (int j = 0; j < (columns + 1); j++)
		{
			float x = (float)i - halfSize.x;
			float y = halfSize.y - (float)j;

			vertices[i * (columns + 1) + j] = VertexPositionTexture(Vector4(x, 0.0f, y, 1.0f), Vector2(x, y));
		}
	}

	std::vector<uint16_t> indices;
	indices.resize(m_IndexCount);

	uint16_t k = 0;
	for (uint16_t i = 0; i < rows; i++)
	{
		for (uint16_t j = 0; j < columns; j++)
		{
			indices[k] = i * (columns + 1) + j;
			indices[k + 1] = i * (columns + 1) + j + 1;
			indices[k + 2] = (i + 1) * (columns + 1) + j;

			indices[k + 3] = (i + 1) * (columns + 1) + j;
			indices[k + 4] = i * (columns + 1) + j + 1;
			indices[k + 5] = (i + 1) * (columns + 1) + j + 1;

			k += 6;
		}
	}

	m_pCurrentPass->CreateVertexBuffer(&vertices.front(), vertices.size() * sizeof(VertexPositionTexture), &m_pVertexBuffer);
	m_pCurrentPass->CreateIndexBuffer(&indices.front(), indices.size() * sizeof(uint16_t), &m_pIndexBuffer);
}

HRESULT GridNode::VOnInitSceneNode(Scene* pScene)
{
	return S_OK;
}

HRESULT GridNode::VOnDeleteSceneNode(Scene *pScene)
{
	return S_OK;
}

HRESULT GridNode::VRender(Scene* pScene, const GameTime& gameTime)
{
	const std::vector<Variable*>& variables = m_pEffect->GetVariables();
	for (auto variable : variables)
	{
		if (variable->GetVariableSemantic() == "worldviewprojection")
		{
			if (variable->GetVariableType() == "float4x4")
			{
				const XMMATRIX& wvp = pScene->GetCamera()->GetWorldViewProjection(pScene);
				variable->SetMatrix(wvp);
			}
		}
		else if (variable->GetVariableType() == "Texture2D")
		{
			Resource resource(m_TextureName);
			shared_ptr<ResHandle> pTextureRes = g_pApp->GetResCache()->GetHandle(&resource);
			if (pTextureRes != nullptr)
			{
				shared_ptr<D3D11TextureResourceExtraData> extra =
					static_pointer_cast<D3D11TextureResourceExtraData>(pTextureRes->GetExtraData());
				if (extra != nullptr)
				{
					variable->SetResource(extra->GetTexture());
				}
			}
		}
	}

	pScene->GetRenderder()->VInputSetup(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_pCurrentPass->GetInputLayout());
	uint32_t stride = m_pCurrentPass->GetVertexSize();
	uint32_t offset = 0;
	pScene->GetRenderder()->VSetVertexBuffers(m_pVertexBuffer, &stride, &offset);
	pScene->GetRenderder()->VSetIndexBuffer(m_pIndexBuffer, IRenderer::Format_uint16, 0);
	pScene->GetRenderder()->VDrawMesh(m_IndexCount, 0, 0, m_pCurrentPass->GetEffectPass());

	return S_OK;
}

HRESULT GridNode::VOnUpdate(Scene* pScene, const GameTime& gameTime)
{
	return S_OK;
}

const std::string GeometryNode::m_Cube = "Cube";
const std::string GeometryNode::m_Sphere = "Sphere";
const std::string GeometryNode::m_Cylinder = "Cylinder";
const std::string GeometryNode::m_Teapot = "Teapot";
const std::string GeometryNode::m_Plane = "Plane";

GeometryNode::GeometryNode(ActorType actorType, ActorId actorId, WeakBaseRenderComponentPtr renderComponent, RenderPass renderPass, const Matrix& worldMatrix)
	: SceneNode(actorId, renderComponent, renderPass),
	m_pEffect(nullptr),
	m_pCurrentPass(nullptr),
	m_pVertexBuffer(nullptr),
	m_pIndexBuffer(nullptr),
	m_Mesh(nullptr)
{
	if (actorType == m_Cube)
	{
		CreateCube();
	}
	if (actorType == m_Sphere)
	{
		CreateSphere();
	}
	else if (actorType == m_Cylinder)
	{
		CreateCylinder();
	}
	else if (actorType == m_Teapot)
	{
		CreateTeapot();
	}
	else if (actorType == m_Plane)
	{
		CreatePlane();
	}
	else
	{

	}

	SetBoundingBox(m_Mesh->GetBoundingBox());
}

GeometryNode::~GeometryNode()
{
	VOnDeleteSceneNode(nullptr);
}

HRESULT GeometryNode::VOnInitSceneNode(Scene* pScene)
{
	VOnDeleteSceneNode(pScene);

	GeometryRenderComponent* pGeometryRender = static_cast<GeometryRenderComponent*>(m_pRenderComponent);
	if (pGeometryRender != nullptr)
	{
		m_EffectName = pGeometryRender->GetEffectName();
	}

	Resource effectRes(m_EffectName);
	shared_ptr<ResHandle> pEffectResHandle = g_pApp->GetResCache()->GetHandle(&effectRes);
	if (pEffectResHandle != nullptr)
	{
		shared_ptr<HlslResourceExtraData> extra = static_pointer_cast<HlslResourceExtraData>(pEffectResHandle->GetExtraData());
		if (extra != nullptr)
		{
			m_pEffect = extra->GetEffect();
		}
	}

	if (m_pEffect == nullptr)
	{
		DEBUG_ERROR("effect is not exist or valid: " + m_EffectName);
	}

	const tinyxml2::XMLDocument* pEffectXmlDoc = m_pEffect->GetEffectXmlDoc();
	if (pEffectXmlDoc == nullptr)
	{
		DEBUG_ERROR("not generate effect xml string");
	}
	const tinyxml2::XMLElement* rootNode = pEffectXmlDoc->RootElement();
	if (rootNode == nullptr)
	{
		DEBUG_ERROR("effect xml is invalid");
	}

	const tinyxml2::XMLElement* pTechniques = rootNode->FirstChildElement("Techniques");
	if (pTechniques == nullptr)
	{
		DEBUG_ERROR("effect xml is invalid");
	}

	for (const tinyxml2::XMLElement* pNode = pTechniques->FirstChildElement(); pNode; pNode = pNode->NextSiblingElement())
	{
		if (pNode->BoolAttribute("checked"))
		{
			std::string techniqueName = pNode->Attribute("name");
			Technique* pCurrentTechnique = m_pEffect->GetTechniquesByName().at(techniqueName);
			if (pCurrentTechnique == nullptr)
			{
				DEBUG_ERROR("technique is not exist: " + techniqueName);
			}

			m_pCurrentPass = pCurrentTechnique->GetPasses().at(0);
			if (m_pCurrentPass == nullptr)
			{
				DEBUG_ERROR("technique is not exist: " + techniqueName);
			}

			m_pCurrentPass->CreateVertexBuffer(m_Mesh.get(), &m_pVertexBuffer);
			m_pCurrentPass->CreateIndexBuffer(m_Mesh.get(), &m_pIndexBuffer);
			return S_OK;
		}
	}

	return S_FALSE;
}

HRESULT GeometryNode::VOnDeleteSceneNode(Scene *pScene)
{
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pIndexBuffer);

	return S_OK;
}

HRESULT GeometryNode::VOnUpdate(Scene* pScene, const GameTime& gameTime)
{
	return S_OK;
}

HRESULT GeometryNode::VRender(Scene* pScene, const GameTime& gameTime)
{
	DEBUG_ASSERT(m_pEffect->GetEffectXmlDoc() != nullptr && m_pEffect->GetEffectXmlDoc()->RootElement() != nullptr);

	const tinyxml2::XMLElement* pVariables = m_pEffect->GetEffectXmlDoc()->RootElement()->FirstChildElement("Variables");
	DEBUG_ASSERT(pVariables != nullptr);

	const std::map<std::string, Variable*>& variables = m_pEffect->GetVariablesByName();
	for (const tinyxml2::XMLElement* pNode = pVariables->FirstChildElement(); pNode; pNode = pNode->NextSiblingElement())
	{
		Variable* variable = variables.at(pNode->Attribute("name"));

		if (variable->GetVariableSemantic() == "worldviewprojection")
		{
			if (variable->GetVariableType() == "float4x4")
			{
				const Matrix& wvp = pScene->GetCamera()->GetWorldViewProjection(pScene);
				variable->SetMatrix(wvp);
			}
		}
		else if (variable->GetVariableSemantic() == "worldinversetranspose")
		{
			if (variable->GetVariableType() == "float4x4")
			{
				Matrix& worldIT = pScene->GetTopMatrix().Invert().Transpose();
				variable->SetMatrix(worldIT);
			}
		}
		else if (variable->GetVariableSemantic() == "world")
		{
			if (variable->GetVariableType() == "float4x4")
			{
				const Matrix& world = pScene->GetTopMatrix();
				variable->SetMatrix(world);
			}
		}
		else if (variable->GetVariableSemantic() == "viewinverse")
		{
			if (variable->GetVariableType() == "float4x4")
			{
				Matrix& viewI = pScene->GetCamera()->GetViewMatrix().Invert();
				variable->SetMatrix(viewI);
			}
		}
		else if (variable->GetVariableType() == "float4")
		{
			std::stringstream ss(pNode->Attribute("value"));
			Vector4 value;
			ss >> value.x >> value.y >> value.z >> value.w;
			variable->SetVector(value);
		}
		else if (variable->GetVariableType() == "float3")
		{
			std::stringstream ss(pNode->Attribute("value"));
			Vector3 value;
			ss >> value.x >> value.y >> value.z;
			variable->SetVector(value);
		}
		else if (variable->GetVariableType() == "float2")
		{
			std::stringstream ss(pNode->Attribute("value"));
			Vector2 value;
			ss >> value.x >> value.y;
			variable->SetVector(value);
		}
		else if (variable->GetVariableType() == "float")
		{
			variable->SetFloat(pNode->FloatAttribute("value"));
		}
		else if (variable->GetVariableType() == "Texture1D" ||
			variable->GetVariableType() == "Texture2D" || variable->GetVariableType() == "TextureCube")
		{
			const tinyxml2::XMLElement* pResourceNode = pNode->FirstChildElement("ResourceName");
			if (pResourceNode != nullptr)
			{
				 std::string textureName = std::string("Textures\\") + pResourceNode->GetText();
				 Resource resource(textureName);
				 shared_ptr<ResHandle> pTextureRes = g_pApp->GetResCache()->GetHandle(&resource);
				 if (pTextureRes != nullptr)
				 {
					 shared_ptr<D3D11TextureResourceExtraData> extra =
						 static_pointer_cast<D3D11TextureResourceExtraData>(pTextureRes->GetExtraData());
					 if (extra != nullptr)
					 {
						 variable->SetResource(extra->GetTexture());
					 }
				 }
			}
		}
	}

	pScene->GetRenderder()->VInputSetup(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_pCurrentPass->GetInputLayout());
	uint32_t stride = m_pCurrentPass->GetVertexSize();
	uint32_t offset = 0;
	pScene->GetRenderder()->VSetVertexBuffers(m_pVertexBuffer, &stride, &offset);
	pScene->GetRenderder()->VSetIndexBuffer(m_pIndexBuffer, IRenderer::Format_uint32, 0);
	pScene->GetRenderder()->VDrawMesh(m_IndexCount, 0, 0, m_pCurrentPass->GetEffectPass());

	return S_OK;
}

void GeometryNode::VPick(Scene* pScene, int cursorX, int cursorY)
{
	const Matrix& projectMat = pScene->GetCamera()->GetProjectMatrix();
	float viewX = (2.0f * cursorX / g_pApp->GetGameConfig().m_ScreenWidth - 1.0f) / projectMat.m[0][0];
	float viewY = (1.0f - 2.0f * cursorY / g_pApp->GetGameConfig().m_ScreenHeight) / projectMat.m[1][1];

	Matrix toLocal = (m_Properties.GetWorldMatrix() * pScene->GetCamera()->GetViewMatrix()).Invert();
	Vector3 rayPos = toLocal.Translation();
	//use right-hand coordinates, z should be -1
	Vector3 rayDir = Vector3::TransformNormal(Vector3(viewX, viewY, -1.0f), toLocal);
	rayDir.Normalize();

	Ray ray(rayPos, rayDir);

	float distance = 0.0f;
	if (ray.Intersects(m_Properties.GetBoundingBox(), distance))
	{
		if (distance < pScene->GetPickDistance())
		{
			pScene->SetPickDistance(distance);
			for (uint32_t i = 0; i < m_IndexCount; i += 3)
			{
				const std::vector<Vector3>& vertices = m_Mesh->GetVertices();
				const std::vector<uint32_t>& indices = m_Mesh->GetIndices();
				Vector3 tri0 = vertices.at(indices[i]);
				Vector3 tri1 = vertices.at(indices[i + 1]);
				Vector3 tri2 = vertices.at(indices[i + 2]);

				float distance = 0.0f;
				if (ray.Intersects(tri0, tri1, tri2, distance))
				{
					pScene->SetPickedActor(m_Properties.GetActorId());
					break;
				}
			}
		}
	}
}

void GeometryNode::CreateCube()
{
	CubeRenderComponent* pMeshRender = static_cast<CubeRenderComponent*>(m_pRenderComponent);
	if (pMeshRender != nullptr)
	{
		float size = pMeshRender->GetSize();
		bool useRHcoords = pMeshRender->UseRHcoords();

		std::vector<VertexPositionNormalTexture> vertices;
		std::vector<uint16_t> indices;
		GeometricPrimitive::CreateCube(vertices, indices, size, useRHcoords);
		m_IndexCount = indices.size();
		m_Mesh = unique_ptr<Mesh>(DEBUG_NEW Mesh(vertices, indices));
	}
}

void GeometryNode::CreateSphere()
{
	SphereRenderComponent* pMeshRender = static_cast<SphereRenderComponent*>(m_pRenderComponent);
	if (pMeshRender != nullptr)
	{
		float diameter = pMeshRender->GetDiameter();
		uint32_t tessellation = pMeshRender->GetTessellation();
		bool useRHcoords = pMeshRender->UseRHcoords();

		std::vector<VertexPositionNormalTexture> vertices;
		std::vector<uint16_t> indices;
		GeometricPrimitive::CreateGeoSphere(vertices, indices, diameter, tessellation, useRHcoords);
		m_IndexCount = indices.size();
		m_Mesh = unique_ptr<Mesh>(DEBUG_NEW Mesh(vertices, indices));
	}
}

void GeometryNode::CreateCylinder()
{
	CylinderRenderComponent* pMeshRender = static_cast<CylinderRenderComponent*>(m_pRenderComponent);
	if (pMeshRender != nullptr)
	{
		float height = pMeshRender->GetHeight();
		float diameter = pMeshRender->GetDiameter();
		uint32_t tessellation = pMeshRender->GetTessellation();
		bool useRHcoords = pMeshRender->UseRHcoords();

		std::vector<VertexPositionNormalTexture> vertices;
		std::vector<uint16_t> indices;
		GeometricPrimitive::CreateCylinder(vertices, indices, height, diameter, tessellation, useRHcoords);
		m_IndexCount = indices.size();
		m_Mesh = unique_ptr<Mesh>(DEBUG_NEW Mesh(vertices, indices));
	}
}

void GeometryNode::CreateTeapot()
{
	TeapotRenderComponent* pMeshRender = static_cast<TeapotRenderComponent*>(m_pRenderComponent);
	if (pMeshRender != nullptr)
	{
		float size = pMeshRender->GetSize();
		uint32_t tessellation = pMeshRender->GetTessellation();
		bool useRHcoords = pMeshRender->UseRHcoords();

		std::vector<VertexPositionNormalTexture> vertices;
		std::vector<uint16_t> indices;
		GeometricPrimitive::CreateTeapot(vertices, indices, size, tessellation, useRHcoords);
		m_IndexCount = indices.size();
		m_Mesh = unique_ptr<Mesh>(DEBUG_NEW Mesh(vertices, indices));
	}
}

void GeometryNode::CreatePlane()
{

}

ModelNode::ModelNode(
	ActorId actorId, WeakBaseRenderComponentPtr renderComponent, RenderPass renderPass, const Matrix& worldMatrix)
	: SceneNode(actorId, renderComponent, renderPass, worldMatrix),
	m_pEffect(nullptr),
	m_pCurrentPass(nullptr),
	m_pModel(nullptr)
{
	ModelRenderComponent* pMeshRender = static_cast<ModelRenderComponent*>(m_pRenderComponent);
	if (pMeshRender != nullptr)
	{
		m_ModelName = pMeshRender->GetModelName();
	}

	Resource modelRes(m_ModelName);
	shared_ptr<ResHandle> pModelResHandle = g_pApp->GetResCache()->GetHandle(&modelRes);
	m_pModel = unique_ptr<Model>(DEBUG_NEW Model(pModelResHandle->Buffer(), pModelResHandle->Size()));
	SetBoundingBox(m_pModel->GetBoundingBox());
}

ModelNode::~ModelNode()
{
	VOnDeleteSceneNode(nullptr);
}

HRESULT ModelNode::VOnInitSceneNode(Scene *pScene)
{
	VOnDeleteSceneNode(pScene);

	ModelRenderComponent* pMeshRender = static_cast<ModelRenderComponent*>(m_pRenderComponent);
	if (pMeshRender != nullptr)
	{
		m_EffectName = pMeshRender->GetEffectName();
	}

	Resource effectRes(m_EffectName);
	shared_ptr<ResHandle> pEffectResHandle = g_pApp->GetResCache()->GetHandle(&effectRes);
	if (pEffectResHandle != nullptr)
	{
		shared_ptr<HlslResourceExtraData> extra = static_pointer_cast<HlslResourceExtraData>(pEffectResHandle->GetExtraData());
		if (extra != nullptr)
		{
			m_pEffect = extra->GetEffect();
		}
	}

	if (m_pEffect == nullptr)
	{
		DEBUG_ERROR("effect is not exist or valid: " + m_EffectName);
	}

	const tinyxml2::XMLDocument* pEffectXmlDoc = m_pEffect->GetEffectXmlDoc();
	if (pEffectXmlDoc == nullptr)
	{
		DEBUG_ERROR("not generate effect xml string");
		return S_FALSE;
	}
	const tinyxml2::XMLElement* rootNode = pEffectXmlDoc->RootElement();
	if (rootNode == nullptr)
	{
		DEBUG_ERROR("effect xml is invalid");
	}

	const tinyxml2::XMLElement* pTechniques = rootNode->FirstChildElement("Techniques");
	if (pTechniques == nullptr)
	{
		DEBUG_ERROR("effect xml is invalid");
	}

	for (const tinyxml2::XMLElement* pNode = pTechniques->FirstChildElement(); pNode; pNode = pNode->NextSiblingElement())
	{
		if (pNode->BoolAttribute("checked"))
		{
			std::string techniqueName = pNode->Attribute("name");
			Technique* pCurrentTechnique = m_pEffect->GetTechniquesByName().at(techniqueName);
			if (pCurrentTechnique == nullptr)
			{
				DEBUG_ERROR("technique is not exist: " + techniqueName);
			}

			m_pCurrentPass = pCurrentTechnique->GetPasses().at(0);
			if (m_pCurrentPass == nullptr)
			{
				DEBUG_ERROR("technique is not exist: " + techniqueName);
			}

			uint32_t meshSize = m_pModel->GetMeshes().size();
			m_pVertexBuffers.resize(meshSize);
			m_pIndexBuffers.resize(meshSize);
			m_IndexCounts.resize(meshSize);
			for (uint32_t i = 0; i < meshSize; i++)
			{
				auto mesh = m_pModel->GetMeshes().at(i);
				ID3D11Buffer* pVertexBuffer = nullptr;
				ID3D11Buffer* pIndexBuffer = nullptr;
				m_pCurrentPass->CreateVertexBuffer(mesh, &pVertexBuffer);
				m_pCurrentPass->CreateIndexBuffer(mesh, &pIndexBuffer);

				m_pVertexBuffers[i] = pVertexBuffer;
				m_pIndexBuffers[i] = pIndexBuffer;
				m_IndexCounts[i] = mesh->GetIndices().size();
			}			

			return S_OK;
		}
	}

	return S_FALSE;
}

HRESULT ModelNode::VOnDeleteSceneNode(Scene* pScene)
{
	for (auto pVertexBuffer : m_pVertexBuffers)
	{
		SAFE_RELEASE(pVertexBuffer);
	}
	m_pVertexBuffers.clear();

	for (auto pIndexBuffer : m_pIndexBuffers)
	{
		SAFE_RELEASE(pIndexBuffer);
	}
	m_pIndexBuffers.clear();
	
	return S_OK;
}

HRESULT ModelNode::VOnUpdate(Scene* pScene, const GameTime& gameTime)
{
	return S_OK;
}

HRESULT ModelNode::VRender(Scene* pScene, const GameTime& gameTime)
{
	DEBUG_ASSERT(m_pEffect->GetEffectXmlDoc() != nullptr && m_pEffect->GetEffectXmlDoc()->RootElement() != nullptr);

	const tinyxml2::XMLElement* pVariables = m_pEffect->GetEffectXmlDoc()->RootElement()->FirstChildElement("Variables");
	DEBUG_ASSERT(pVariables != nullptr);

	const std::map<std::string, Variable*>& variables = m_pEffect->GetVariablesByName();
	for (const tinyxml2::XMLElement* pNode = pVariables->FirstChildElement(); pNode; pNode = pNode->NextSiblingElement())
	{
		Variable* variable = variables.at(pNode->Attribute("name"));

		if (variable->GetVariableSemantic() == "worldviewprojection")
		{
			if (variable->GetVariableType() == "float4x4")
			{
				const Matrix& wvp = pScene->GetCamera()->GetWorldViewProjection(pScene);
				variable->SetMatrix(wvp);
			}
		}
		else if (variable->GetVariableSemantic() == "worldinversetranspose")
		{
			if (variable->GetVariableType() == "float4x4")
			{
				Matrix& worldIT = pScene->GetTopMatrix().Invert().Transpose();
				variable->SetMatrix(worldIT);
			}
		}
		else if (variable->GetVariableSemantic() == "world")
		{
			if (variable->GetVariableType() == "float4x4")
			{
				const Matrix& world = pScene->GetTopMatrix();
				variable->SetMatrix(world);
			}
		}
		else if (variable->GetVariableSemantic() == "viewinverse")
		{
			if (variable->GetVariableType() == "float4x4")
			{
				Matrix& viewI = pScene->GetCamera()->GetViewMatrix().Invert();
				variable->SetMatrix(viewI);
			}
		}
		else if (variable->GetVariableType() == "float4")
		{
			std::stringstream ss(pNode->Attribute("value"));
			Vector4 value;
			ss >> value.x >> value.y >> value.z >> value.w;
			variable->SetVector(value);
		}
		else if (variable->GetVariableType() == "float3")
		{
			std::stringstream ss(pNode->Attribute("value"));
			Vector3 value;
			ss >> value.x >> value.y >> value.z;
			variable->SetVector(value);
		}
		else if (variable->GetVariableType() == "float2")
		{
			std::stringstream ss(pNode->Attribute("value"));
			Vector2 value;
			ss >> value.x >> value.y;
			variable->SetVector(value);
		}
		else if (variable->GetVariableType() == "float")
		{
			variable->SetFloat(pNode->FloatAttribute("value"));
		}
		else if (variable->GetVariableType() == "Texture1D" ||
			variable->GetVariableType() == "Texture2D" || variable->GetVariableType() == "TextureCube")
		{
			const tinyxml2::XMLElement* pResourceNode = pNode->FirstChildElement("ResourceName");
			if (pResourceNode != nullptr)
			{
				std::string textureName = std::string("Textures\\") + pResourceNode->GetText();
				Resource resource(textureName);
				shared_ptr<ResHandle> pTextureRes = g_pApp->GetResCache()->GetHandle(&resource);
				if (pTextureRes != nullptr)
				{
					shared_ptr<D3D11TextureResourceExtraData> extra =
						static_pointer_cast<D3D11TextureResourceExtraData>(pTextureRes->GetExtraData());
					if (extra != nullptr)
					{
						variable->SetResource(extra->GetTexture());
					}
				}
			}
		}
	}

	for (uint32_t i = 0, count = m_pModel->GetMeshes().size(); i < count; i++)
	{
		Mesh* mesh = m_pModel->GetMeshes().at(i);
		switch (mesh->GetPrimitiveType())
		{
		case Mesh::PT_Point:
			pScene->GetRenderder()->VInputSetup(D3D_PRIMITIVE_TOPOLOGY_POINTLIST, m_pCurrentPass->GetInputLayout());
			break;
		case Mesh::PT_Line:
			pScene->GetRenderder()->VInputSetup(D3D_PRIMITIVE_TOPOLOGY_LINELIST, m_pCurrentPass->GetInputLayout());
			break;
		case Mesh::PT_Triangle:
			pScene->GetRenderder()->VInputSetup(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_pCurrentPass->GetInputLayout());
			break;
		default:
			DEBUG_ERROR("Unsupport primitive type!");
			return S_FALSE;
		}

		uint32_t stride = m_pCurrentPass->GetVertexSize();
		uint32_t offset = 0;
		pScene->GetRenderder()->VSetVertexBuffers(m_pVertexBuffers[i], &stride, &offset);
		pScene->GetRenderder()->VSetIndexBuffer(m_pIndexBuffers[i], IRenderer::Format_uint32, 0);
		pScene->GetRenderder()->VDrawMesh(m_IndexCounts[i], 0, 0, m_pCurrentPass->GetEffectPass());
	}

	return S_OK;
}

void ModelNode::VPick(Scene* pScene, int cursorX, int cursorY)
{
	const Matrix& projectMat = pScene->GetCamera()->GetProjectMatrix();
	float viewX = (2.0f * cursorX / g_pApp->GetGameConfig().m_ScreenWidth - 1.0f) / projectMat.m[0][0];
	float viewY = (1.0f - 2.0f * cursorY / g_pApp->GetGameConfig().m_ScreenHeight) / projectMat.m[1][1];

	Matrix toLocal = (m_Properties.GetWorldMatrix() * pScene->GetCamera()->GetViewMatrix()).Invert();
	Vector3 rayPos = toLocal.Translation();
	//use right-hand coordinates, z should be -1
	Vector3 rayDir = Vector3::TransformNormal(Vector3(viewX, viewY, -1.0f), toLocal);
	rayDir.Normalize();

	Ray ray(rayPos, rayDir);

	float distance = 0.0f;
	if (ray.Intersects(m_Properties.GetBoundingBox(), distance))
	{
		if (distance < pScene->GetPickDistance())
		{
			pScene->SetPickDistance(distance);
			for (auto mesh : m_pModel->GetMeshes())
			{
				switch (mesh->GetPrimitiveType())
				{
				case Mesh::PT_Point:
				case Mesh::PT_Line:
				{
					distance = 0.0f;
					if (ray.Intersects(mesh->GetBoundingBox(), distance))
					{
						pScene->SetPickedActor(m_Properties.GetActorId());
						return;
					}
					break;
				}
				case Mesh::PT_Triangle:
				{
					for (uint32_t i = 0, len = mesh->GetIndices().size(); i < len; i += 3)
					{
						const std::vector<Vector3>& vertices = mesh->GetVertices();
						const std::vector<uint32_t>& indices = mesh->GetIndices();
						Vector3 tri0 = vertices.at(indices[i]);
						Vector3 tri1 = vertices.at(indices[i + 1]);
						Vector3 tri2 = vertices.at(indices[i + 2]);

						distance = 0.0f;
						if (ray.Intersects(tri0, tri1, tri2, distance))
						{
							pScene->SetPickedActor(m_Properties.GetActorId());
							return;
						}
					}
					break;
				}
				default:
					break;
				}
			}
		}
	}
}

SkyboxNode::SkyboxNode(ActorId actorId, WeakBaseRenderComponentPtr renderComponent, RenderPass renderPass)
	: SceneNode(actorId, renderComponent, renderPass),
	m_pEffect(nullptr),
	m_pCurrentPass(nullptr),
	m_pVertexBuffer(nullptr),
	m_pIndexBuffer(nullptr),
	m_IndexCount(0),
	m_ScaleMatrix(Matrix::CreateScale(10000.0f))
{
	SkyboxRenderComponent* pSkyboxRender = static_cast<SkyboxRenderComponent*>(m_pRenderComponent);
	if (pSkyboxRender != nullptr)
	{
		m_TextureName = pSkyboxRender->GetTextureName();
	}

	Resource effectRes("Effects\\Skybox.fx");
	shared_ptr<ResHandle> pEffectResHandle = g_pApp->GetResCache()->GetHandle(&effectRes);
	if (pEffectResHandle != nullptr)
	{
		shared_ptr<HlslResourceExtraData> extra = static_pointer_cast<HlslResourceExtraData>(pEffectResHandle->GetExtraData());
		if (extra != nullptr)
		{
			m_pEffect = extra->GetEffect();
		}
	}

	Technique* pCurrentTechnique = m_pEffect->GetTechniquesByName().at("main11");
	if (pCurrentTechnique == nullptr)
	{
		DEBUG_ERROR(std::string("technique is not exist: ") + "main11");
	}
	m_pCurrentPass = pCurrentTechnique->GetPassesByName().at("p0");
	if (m_pCurrentPass == nullptr)
	{
		DEBUG_ERROR(std::string("technique is not exist: ") + "p0");
	}

	std::vector<VertexPositionNormalTexture> vertices;
	std::vector<uint16_t> indices;
	GeometricPrimitive::CreateSphere(vertices, indices);

	std::vector<Vector3> points;
	points.reserve(vertices.size());
	for (auto vertex : vertices)
	{
		points.push_back(vertex.position);
	}

	m_pCurrentPass->CreateVertexBuffer(&points.front(), points.size() * sizeof(Vector3), &m_pVertexBuffer);
	m_pCurrentPass->CreateIndexBuffer(&indices.front(), indices.size() * sizeof(uint16_t), &m_pIndexBuffer);
	m_IndexCount = indices.size();
}

SkyboxNode::~SkyboxNode()
{
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pIndexBuffer);
}

bool SkyboxNode::VIsVisible(Scene* pScene)
{
	return true;
}

HRESULT SkyboxNode::VPreRender(Scene* pScene)
{
	m_ScaleMatrix.Translation(pScene->GetCamera()->GetPosition());
	VSetTransform(m_ScaleMatrix);

	return SceneNode::VPreRender(pScene);
}

HRESULT SkyboxNode::VOnInitSceneNode(Scene* pScene)
{
	return S_OK;
}

HRESULT SkyboxNode::VOnDeleteSceneNode(Scene *pScene)
{
	return S_OK;
}

HRESULT SkyboxNode::VRender(Scene* pScene, const GameTime& gameTime)
{
	const std::vector<Variable*>& variables = m_pEffect->GetVariables();
	for (auto variable : variables)
	{
		if (variable->GetVariableSemantic() == "worldviewprojection")
		{
			if (variable->GetVariableType() == "float4x4")
			{
				const XMMATRIX& wvp = pScene->GetCamera()->GetWorldViewProjection(pScene);
				variable->SetMatrix(wvp);
			}
		}
		else if (variable->GetVariableType() == "TextureCube")
		{
			Resource resource(m_TextureName);
			shared_ptr<ResHandle> pTextureRes = g_pApp->GetResCache()->GetHandle(&resource);
			if (pTextureRes != nullptr)
			{
				shared_ptr<D3D11TextureResourceExtraData> extra =
					static_pointer_cast<D3D11TextureResourceExtraData>(pTextureRes->GetExtraData());
				if (extra != nullptr)
				{
					variable->SetResource(extra->GetTexture());
				}
			}
		}
	}

	pScene->GetRenderder()->VInputSetup(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_pCurrentPass->GetInputLayout());
	uint32_t stride = m_pCurrentPass->GetVertexSize();
	uint32_t offset = 0;
	pScene->GetRenderder()->VSetVertexBuffers(m_pVertexBuffer, &stride, &offset);
	pScene->GetRenderder()->VSetIndexBuffer(m_pIndexBuffer, IRenderer::Format_uint16, 0);
	pScene->GetRenderder()->VDrawMesh(m_IndexCount, 0, 0, m_pCurrentPass->GetEffectPass());

	return S_OK;
}


HRESULT SkyboxNode::VOnUpdate(Scene* pScene, const GameTime& gameTime)
{
	return S_OK;
}
