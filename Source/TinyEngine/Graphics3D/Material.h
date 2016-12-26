#pragma once
#include "../TinyEngineBase.h"
#include "../TinyEngineInterface.h"

class Model;
class Mesh;
class Effect;
class Technique;
class Pass;
class Variable;

class Effect : public boost::noncopyable
{
public:
	Effect(ID3D11Device1* pDevice, ID3DX11Effect* pD3DX11Effect);
	virtual ~Effect();

	const std::vector<Technique*>& GetTechniques() const;
	const std::map<std::string, Technique*>& GetTechniquesByName() const;
	const std::vector<Variable*>& GetVariables() const;
	const std::map<std::string, Variable*>& GetVariablesByName() const;

private:
	std::vector<Technique*> m_Techniques;
	std::map<std::string, Technique*> m_TechniquesByName;
	std::vector<Variable*> m_Variables;
	std::map<std::string, Variable*> m_VariablesByName;
};

class Technique : public boost::noncopyable
{
public:
	Technique(ID3D11Device1* pDevice, ID3DX11EffectTechnique* pD3DX11EffectTechnique);
	~Technique();

	const std::string& GetTechniqueName() const;
	const std::vector<Pass*>& GetPasses() const;
	const std::map<std::string, Pass*>& GetPassesByName() const;

private:
	ID3DX11EffectTechnique* m_pD3DX11EffectTechnique;
	D3DX11_TECHNIQUE_DESC m_D3DX11TechniqueDesc;
	std::string m_TechniqueName;
	std::vector<Pass*> m_Passes;
	std::map<std::string, Pass*> m_PassesByName;
};

class Pass : public boost::noncopyable
{
public:
	Pass(ID3D11Device1* pDevice, ID3DX11EffectPass* pD3DX11EffectPass);

	const std::string& GetPassName() const { return m_PassName; }
	ID3D11InputLayout* GetInputLayout() { return m_pInputLayouts; }
	uint32_t GetVertexSize() { return m_VertexSize; }
	ID3DX11EffectPass* GetEffectPass() { return m_pD3DX11EffectPass; }

	void CreateVertexBuffer(const void* pVertexData, uint32_t size, ID3D11Buffer** ppVertexBuffer) const;
	void CreateIndexBuffer(const void* pIndexData, uint32_t size, ID3D11Buffer** ppIndexBuffer) const;
	void CreateVertexBuffer(const Mesh* mesh, ID3D11Buffer** ppVertexBuffer) const;
	void CreateIndexBuffer(const Mesh* mesh, ID3D11Buffer** ppIndexBuffer) const;

	void Apply(uint32_t flags, ID3D11DeviceContext1* pDeviceContext);

private:
	DXGI_FORMAT GetElementFormat(D3D_REGISTER_COMPONENT_TYPE compoentType, uint8_t mask);

	ID3D11Device1* p_Device;
	ID3DX11EffectPass* m_pD3DX11EffectPass;
	std::string m_PassName;
	ID3D11InputLayout* m_pInputLayouts;
	uint32_t m_VertexSize;
	std::vector<std::string> m_VertexFormat;
};

class Variable : public boost::noncopyable
{
public:
	Variable(ID3D11Device1* pDevice, ID3DX11EffectVariable* pD3DX11EffectVariable);

	const std::string& GetVariableName() const { return m_VariableName; }
	const std::string& GetVariableSemantic() const { return m_VariableSemantic; }
	const std::string& GetVariableType() const { return m_VariableType; }

	void SetMatrix(XMMATRIX value);
	void SetResource(ID3D11ShaderResourceView* value);
	Variable& operator<<(ID3D11UnorderedAccessView* value);
	Variable& operator<<(FXMVECTOR value);
	Variable& operator<<(int value);
	Variable& operator<<(float value);
	Variable& operator<<(const std::vector<float>& values);
	Variable& operator<<(const std::vector<XMFLOAT2>& values);
	Variable& operator<<(const std::vector<XMFLOAT4X4>& values);

private:
	ID3DX11EffectVariable* m_pD3DX11EffectVariable;
	ID3DX11EffectType* m_pD3DX11EffectType;
	std::string m_VariableName;
	std::string m_VariableSemantic;
	std::string m_VariableType;
};
