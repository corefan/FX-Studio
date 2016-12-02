#pragma once
#include "../TinyEngineBase.h"
#include "../TinyEngineInterface.h"

class Model;
class Mesh;
class Effect;
class Technique;
class Pass;
class Variable;

class Material : public boost::noncopyable
{
public:
	Material();
	Material(const std::string& defaultTechniqueName);
	virtual ~Material();

	Variable* operator[](const std::string& variableName);
	Effect* GetEffect() const;
	Technique* GetCurrentTechnique() const;
	void SetCurrentTechnique(Technique* pTechnique);
	const std::map<Pass*, ID3D11InputLayout*>& GetInputLayouts() const;

	virtual void Initialize(Effect* pEffect);
	virtual void CreateVertexBuffer(const Model* pModel, std::vector<ID3D11Buffer*>& vertexBuffers) const;
	virtual void CreateVertexBuffer(const Mesh* pMesh, ID3D11Buffer** ppVertexBuffer) const = 0;
	virtual uint32_t VertexSize() const = 0;

protected:
	virtual void CreateInputLayout(
		const std::string& techniqueName, const std::string& passName,
		D3D11_INPUT_ELEMENT_DESC* inputElementDescriptions, uint32_t inputElementDescriptionCount);
	virtual void Material::CreateInputLayout(
		Pass* pPass, D3D11_INPUT_ELEMENT_DESC* inputElementDescriptions, uint32_t inputElementDescriptionCount);

	Effect* m_pEffect;
	Technique* m_pCurrentTechnique;
	std::string m_DefaultTechniqueName;
	std::map<Pass*, ID3D11InputLayout*> m_InputLayouts;
};

class Effect : public boost::noncopyable
{
public:
	Effect(ID3DX11Effect* pD3DX11Effect);
	virtual ~Effect();

	const std::vector<Technique*>& GetTechniques() const;
	const std::map<std::string, Technique*>& GetTechniquesByName() const;
	const std::vector<Variable*>& GetVariables() const;
	const std::map<std::string, Variable*>& GetVariablesByName() const;

private:
	void Initialize();

	ID3DX11Effect* m_pD3DX11Effect;
	D3DX11_EFFECT_DESC m_D3DX11EffectDesc;
	std::vector<Technique*> m_Techniques;
	std::map<std::string, Technique*> m_TechniquesByName;
	std::vector<Variable*> m_Variables;
	std::map<std::string, Variable*> m_VariablesByName;
};

class Technique : public boost::noncopyable
{
public:
	Technique(ID3DX11EffectTechnique* pD3DX11EffectTechnique);
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
	Pass(ID3DX11EffectPass* pD3DX11EffectPass);

	const std::string& GetPassName() const;

	HRESULT CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* inputElementDesc, uint32_t numElements, ID3D11InputLayout **inputLayout);
	void Apply(uint32_t flags);

private:
	void BuildFormatMap();

	ID3DX11EffectPass* m_pD3DX11EffectPass;
	D3DX11_PASS_DESC m_D3DX11PassDesc;
	ID3D11InputLayout* m_InputLayouts;
	std::string m_PassName;
	static std::map<uint8_t, DXGI_FORMAT> m_FormatMap;
};

class Variable : public boost::noncopyable
{
public:
	Variable(ID3DX11EffectVariable* pD3DX11EffectVariable);

	ID3DX11EffectVariable* GetD3DX11EffectVariable() const;
	const D3DX11_EFFECT_VARIABLE_DESC& GetD3DX11EffectVariableDesc() const;
	ID3DX11EffectType* GetD3DX11EffectType() const;
	const D3DX11_EFFECT_TYPE_DESC& GetD3DX11EffectTypeDesc() const;
	const std::string& GetVariableName() const;

	Variable& operator<<(CXMMATRIX value);
	Variable& operator<<(ID3D11ShaderResourceView* value);
	Variable& operator<<(ID3D11UnorderedAccessView* value);
	Variable& operator<<(FXMVECTOR value);
	Variable& operator<<(int value);
	Variable& operator<<(float value);
	Variable& operator<<(const std::vector<float>& values);
	Variable& operator<<(const std::vector<XMFLOAT2>& values);
	Variable& operator<<(const std::vector<XMFLOAT4X4>& values);

private:
	ID3DX11EffectVariable* m_pD3DX11EffectVariable;
	D3DX11_EFFECT_VARIABLE_DESC m_D3DX11EffectVariableDesc;
	ID3DX11EffectType* m_pD3DX11EffectType;
	D3DX11_EFFECT_TYPE_DESC m_D3DX11EffectTypeDesc;
	std::string m_VariableName;
};
