#pragma once

#include "dezero.hpp"

namespace F = dz::functions;

namespace dz::layers
{

// �N���X�O���錾
class Layer;

//----------------------------------
// type
//----------------------------------

// �X�}�[�g�|�C���^�^
using LayerPtr = std::shared_ptr<Layer>;

// �v���p�e�B�̒l
using prop_value_t = std::variant<VariablePtr, LayerPtr>;
// �v���p�e�B�R���N�V����
using props_t = std::unordered_map<std::string, prop_value_t>;
// �p�����[�^�R���N�V����
using params_t = std::set<VariablePtr>;
// �p�����[�^���̃R���N�V����
using param_names_t = std::set<std::string>;

//----------------------------------
// class
//----------------------------------

// �v���p�e�B��set/get�̑㗝�����N���X
class PropProxy
{
private:
	// �v���p�e�B
	props_t& props;
	// �v���p�e�B���̃p�����[�^�ꗗ
	param_names_t& param_names;
	// �L�[
	const std::string& key;

public:
	// �R���X�g���N�^
	PropProxy(props_t& props, param_names_t& param_names, const std::string& key) :
		props(props),
		param_names(param_names),
		key(key)
	{}

	// �R�s�[/���[�u�R���X�g���N�^
	PropProxy(const PropProxy&) = default;
	PropProxy(PropProxy&&) = default;

	// VariablePtr�̃R�s�[������Z�q (set�㗝)
	PropProxy& operator=(const VariablePtr& value)
	{
		return this->set<VariablePtr>(value);
	}

	// LayerPtr�̃R�s�[������Z�q (set�㗝)
	PropProxy& operator=(const LayerPtr& value)
	{
		return this->set<LayerPtr>(value);
	}

	// VariablePtr�ւ̃L���X�g���Z�q (get�㗝)
	operator VariablePtr() const noexcept
	{
		return this->get<VariablePtr>();
	}

	// LayerPtr�ւ̃L���X�g���Z�q (get�㗝)
	operator LayerPtr() const noexcept
	{
		return this->get<LayerPtr>();
	}

	// �A���[���Z�q�őΏۂ� VariablePtr �̃����o�𒼐ڑ���
	VariablePtr operator->() const noexcept
	{
		return static_cast<VariablePtr>(*this);
	}

private:
	// set����
	template<typename T>
	PropProxy& set(const T& value)
	{
		// ParameterPtr, LayerPtr(�h���N���X�܂�) �ł���΁A�p�����[�^�Ƃ��ēo�^����
		if (typeid(*value) == typeid(Parameter)) {
			param_names.insert(key);
		}
		else {
			try {
				// Layer&�ɃL���X�g�\�Ȃ�ALayer�̔h���N���X�^�ł���
				// dynamic_cast<Layer&>(*value);
				param_names.insert(key);
			}
			catch (std::bad_cast&) {}
		}
		// �v���p�e�B�֓o�^
		props[key] = value;
		return *this;
	}

	// get����
	template<typename T>
	T get() const noexcept
	{
		auto p = T();
		if (props.find(key) != props.end()) {
			if (std::holds_alternative<T>(props[key])) {
				p = std::get<T>(props[key]);
			}
			else {
				// �قȂ�^�Ƃ��ĎQ��
				assert(false);
			}
		}
		else {
			// ���݂��Ȃ��v���p�e�B�̎Q��
			assert(false);
		}

		return p;
	}

};

// ���C���N���X
class Layer
{
protected:
	// �v���p�e�B
	props_t props;
	// �v���p�e�B���̃p�����[�^�ꗗ
	param_names_t param_names;

	// ���̓f�[�^
	VariableWPtrList inputs;
	// �o�̓f�[�^
	VariableWPtrList outputs;

public:
	// �f�X�g���N�^
	virtual ~Layer() {}

	// �v���p�e�B��set/get
	// []���Z�q���� this �ƕ��p����Ƃ��ɔώG�ɂȂ�̂ł�������g�p����Ɨǂ�
	PropProxy prop(const std::string& key)
	{
		return (*this)[key];
	}

	// Layer�v���p�e�B��get
	Layer& layer(const std::string& key)
	{
		return *(static_cast<LayerPtr>((*this)[key]));
	}

	//// Variable�v���p�e�B��get
	//Variable& variable(const std::string& key)
	//{
	//	return *(static_cast<VariablePtr>((*this)[key]));
	//}

	// []���Z�q�F�v���p�e�B��set/get
	PropProxy operator[](const std::string& key)
	{
		return PropProxy(props, param_names, key);
	}

	// ()���Z�q
	VariablePtrList operator()(const NdArrayPtr& input)
	{
		// VariantPtr�ɕϊ����ď���
		return (*this)(as_variable(input));
	}
	VariablePtrList operator()(const VariablePtr& input)
	{
		// ���X�g�ɕϊ����ď���
		return (*this)(VariablePtrList({ input }));
	}
	VariablePtrList operator()(const VariablePtrList& inputs)
	{
		// ���`�d
		auto outputs = this->forward(inputs);

		// ���o�̓f�[�^��ێ�����
		this->inputs = VariableWPtrList();
		for (const auto& i : inputs) {
			VariableWPtr w = i;
			this->inputs.push_back(w);
		}
		this->outputs = VariableWPtrList();
		for (const auto& o : outputs) {
			VariableWPtr w = o;
			this->outputs.push_back(w);
		}

		return outputs;
	}

	// ���`�d
	virtual VariablePtrList forward(const VariablePtrList& xs) = 0;

	// �p�����[�^�̃R���N�V�����𐶐�
	params_t params()
	{
		// �v���p�e�B���p�����[�^�ł��邩���f
		auto is_param = [this](const props_t::value_type& kv)
		{
			return this->param_names.find(kv.first) != this->param_names.end();
		};

		// �v���p�e�B����p�����[�^�݂̂𒊏o
		props_t props_param_only;
		std::copy_if(this->props.begin(), this->props.end(), std::inserter(props_param_only, props_param_only.end()), is_param);

		// �p�����[�^�̒l�̃R���N�V������Ԃ�
		params_t param_values;
		for (auto& kv : props_param_only) {
			// �v���p�e�B��VariablePtr
			if (std::holds_alternative<VariablePtr>(kv.second)) {
				auto v = std::get<VariablePtr>(kv.second);

				// ���Ԃ�ParameterPtr
				if (typeid(*v) == typeid(Parameter)) {
					param_values.insert(v);
				}
			}
			// �v���p�e�B��LayerPtr
			if (std::holds_alternative<LayerPtr>(kv.second)) {
				auto l = std::get<LayerPtr>(kv.second);

				// ���ʃ��C���̃p�����[�^��}��
				auto params_from_layer = l->params();
				param_values.insert(params_from_layer.begin(), params_from_layer.end());
			}
		}
		return param_values;
	}

	// �S�p�����[�^�̌��z��������
	void cleargrads()
	{
		// �p�����[�^�𒊏o���Č��z��������
		for (auto& p : this->params()) {
			p->cleargrad();
		}
	}
};

// ���C���N���X�i���`�ϊ�/�S�����j
class Linear : public Layer
{
public:
	// ���o�̓f�[�^�T�C�Y
	uint32_t in_size;
	uint32_t out_size;

	// �R���X�g���N�^
	Linear(uint32_t out_size, uint32_t in_size = 0, bool nobias = false) :
		in_size(in_size),
		out_size(out_size)
	{
		// �d�݂̏�����
		this->prop("W") = as_parameter(nullptr, "W");
		// in_size ���w�肳��Ă��Ȃ��ꍇ�͌��
		if (this->in_size != 0) {
			this->init_W();
		}

		// �o�C�A�X�̏�����
		if (!nobias) {
			this->prop("b") = as_parameter(as_array(nc::zeros<data_t>({ 1, out_size })), "b");
		}
	}

	// �d�݂̏�����
	void init_W()
	{
		auto I = this->in_size;
		auto O = this->out_size;
		auto W_data = nc::random::randN<data_t>({ I, O }) * nc::sqrt<data_t>(1.0 / I);
		this->prop("W")->data = as_array(W_data);
	}

	// ���`�d
	virtual VariablePtrList forward(const VariablePtrList& xs) override
	{
		auto x = xs[0];

		// �f�[�^�𗬂��^�C�~���O�ŏd�݂�������
		if (!this->prop("W")->data) {
			this->in_size = x->shape().cols;
			this->init_W();
		}
		auto y = F::linear(x, this->prop("W"), this->prop("b"));
		return { y };
	}
};

}	// namespace dz::layers
