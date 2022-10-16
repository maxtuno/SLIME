#pragma once

#include "dezero.hpp"

namespace dz
{

// �N���X�O���錾
class Variable;
class Function;

//----------------------------------
// type
//----------------------------------

// ��{�f�[�^�^
using data_t = double;	// TODO: �ŏI�I�ɂ� float �ɂ���
using NdArray = nc::NdArray<data_t>;

// �X�}�[�g�|�C���^�^
using NdArrayPtr = std::shared_ptr<NdArray>;	// �C���X�^���X�������� std::make_shared<NdArray> �֐����g������
using VariablePtr = std::shared_ptr<Variable>;	// �C���X�^���X�������� std::make_shared<Variable> �֐����g������
using VariableWPtr = std::weak_ptr<Variable>;
using FunctionPtr = std::shared_ptr<Function>;	// �h���N���X�̃C���X�^���X�������� new ���g������
												// �imake_shared ���g���� Function �N���X���C���X�^���X������ăG���[�ƂȂ�j
// ���X�g�^
using NdArrayPtrList = std::vector<NdArrayPtr>;
using VariablePtrList = std::vector<VariablePtr>;
using VariableWPtrList = std::vector<VariableWPtr>;

// NdArrayPtr�����֐�
extern NdArrayPtr as_array(nullptr_t = nullptr);
extern NdArrayPtr as_array(std::initializer_list<NdArray::value_type> list);
extern NdArrayPtr as_array(NdArray::value_type scalar);
extern NdArrayPtr as_array(const NdArray& data);

// VariablePtr�����֐�
extern VariablePtr as_variable(nullptr_t = nullptr);
extern VariablePtr as_variable(const NdArrayPtr& data);
extern VariablePtr as_variable(const Variable& data);

// NdArrayPtr�����֐�
inline NdArrayPtr as_array(nullptr_t /*=nullptr*/)
{
	return NdArrayPtr();	// �����Ȃ��܂��� nullptr �̏ꍇ�� Empty �Ƃ���
}
inline NdArrayPtr as_array(std::initializer_list<NdArray::value_type> list)
{
	return std::make_shared<NdArray>(list);
}
inline NdArrayPtr as_array(NdArray::value_type scalar)
{
	return as_array({ scalar });
}
inline NdArrayPtr as_array(const NdArray& data)
{
	return std::make_shared<NdArray>(data);
}

// VariablePtr�����֐�
inline VariablePtr as_variable(nullptr_t /*=nullptr*/)
{
	return VariablePtr();	// �����Ȃ��܂��� nullptr �̏ꍇ�� Empty �Ƃ���
}
inline VariablePtr as_variable(const NdArrayPtr& data)
{
	return std::make_shared<Variable>(data);
}
inline VariablePtr as_variable(const Variable& data)
{
	return std::make_shared<Variable>(data);
}

//----------------------------------
// class
//----------------------------------

// �ݒ�N���X
class Config
{
private:
	// �R���X�g���N�^
	Config() {
		// �t�`�d��
		param["enable_backprop"] = true;
	}

public:
	// �ݒ�l
	std::unordered_map<std::string, bool> param;

	// �R�s�[/���[�u�s��
	Config(const Config&) = delete;
	Config(Config&&) = delete;
	Config& operator=(const Config&) = delete;
	Config& operator=(Config&&) = delete;

	// �C���X�^���X�擾
	static Config& get_instance() {
		static Config instance;
		return instance;
	}
};

// �ݒ�ꎞ�ύX�N���X
class UsingConfig
{
private:
	// �ύX�O�̒l
	std::string name;
	bool old_value;

public:
	// �R���X�g���N�^
	UsingConfig(std::string name, bool value) :
		name(name)
	{
		// �ݒ�ύX
		old_value = Config::get_instance().param[name];
		Config::get_instance().param[name] = value;
	}
	// �f�X�g���N�^
	virtual ~UsingConfig()
	{
		// �ݒ蕜��
		Config::get_instance().param[name] = old_value;
	}

	// �R�s�[/���[�u�s��
	UsingConfig(const UsingConfig&) = delete;
	UsingConfig(UsingConfig&&) = delete;
	UsingConfig& operator=(const UsingConfig&) = delete;
	UsingConfig& operator=(UsingConfig&&) = delete;
};

// �t�`�d�ۂ��ꎞ�I��OFF
struct no_grad : UsingConfig
{
	no_grad() : UsingConfig("enable_backprop", false) {}
};

// NdArray�̏o�̓w���p�[�N���X
class NdArrayPrinter
{
public:
	const std::shared_ptr<NdArray> data;
	NdArrayPrinter(const std::shared_ptr<NdArray>& data) :
		data(data)
	{}
	NdArrayPrinter(const NdArray& data) :
		data(std::make_shared<NdArray>(data))
	{}
};

// �ϐ��N���X
class Variable
{
public:
	// �����f�[�^
	NdArrayPtr data;
	// ����
	std::string name;
	// ���z
	NdArrayPtr grad;
	// �������̊֐�
	FunctionPtr creator;
	// ����
	int generation;

	// �R���X�g���N�^
	Variable(const NdArrayPtr& data, const std::string& name = "") :
		data(data),
		name(name),
		generation(0)
	{}

	// �f�X�g���N�^
	virtual ~Variable() {}

	// �������̊֐���ݒ�
	void set_creator(const FunctionPtr& func);

	// �t�`�d(�ċA)
	void backward(bool retain_grad = false);

	// ������������
	void cleargrad() {
		this->grad = nullptr;
	}

	// NdArray�ֈϏ����郁���o
	decltype(auto) shape() { return data->shape(); }
	decltype(auto) size() { return data->size(); }
};

// �֐��N���X
class Function : public std::enable_shared_from_this<Function>
{
public:
	// ���̓f�[�^
	VariablePtrList inputs;
	// �o�̓f�[�^
	VariableWPtrList outputs;
	// ����
	int generation;

	// �f�X�g���N�^
	virtual ~Function() {}

	// ()���Z�q
	VariablePtrList operator()(const NdArrayPtr& input)
	{
		// VariantPtr�ɕϊ����ď���
		return (*this)(as_variable(input));
	}

	// ()���Z�q
	VariablePtrList operator()(const VariablePtr& input)
	{
		// ���X�g�ɕϊ����ď���
		return (*this)(VariablePtrList({ input }));
	}

	// ()���Z�q
	VariablePtrList operator()(const VariablePtrList& inputs)
	{
		// ���̓f�[�^����NdArray�����o��
		auto xs = NdArrayPtrList();
		for (const auto& i : inputs) {
			xs.push_back(i->data);
		}

		// ���`�d
		auto ys = this->forward(xs);

		// �v�Z���ʂ���o�̓f�[�^���쐬
		auto outputs = VariablePtrList();
		for (const auto& y : ys) {
			auto o = as_variable(as_array(*y));
			o->set_creator(shared_from_this());
			outputs.push_back(o);
		}

		// �t�`�d�\�̏ꍇ
		if (Config::get_instance().param["enable_backprop"]) {
			// ���̓f�[�^�̂����ő�l�̐�������g�̐���Ƃ���
			auto max_elem = std::max_element(
				inputs.cbegin(), inputs.cend(),
				[](VariablePtr lhs, VariablePtr rhs) { return lhs->generation < rhs->generation; }
			);
			this->generation = (*max_elem)->generation;

			// ���o�̓f�[�^��ێ�����
			this->inputs = inputs;
			this->outputs = VariableWPtrList();
			for (const auto& o : outputs) {
				VariableWPtr w = o;
				this->outputs.push_back(w);
			}
		}

		return outputs;
	}

	// ���`�d
	virtual NdArrayPtrList forward(const NdArrayPtrList& xs) = 0;
	// �t�`�d
	virtual NdArrayPtrList backward(const NdArrayPtrList& gy) = 0;
};

// �������̊֐���ݒ�
inline void Variable::set_creator(const FunctionPtr& func)
{
	creator = func;

	// �������̊֐��̐���� +1 ���Ď��g�̐���Ƃ���
	this->generation = func->generation + 1;
}

// �t�`�d
// ������ Function �N���X�̃����o���Q�Ƃ��Ă��邽�߂��̈ʒu�Œ�`����K�v������
inline void Variable::backward(bool retain_grad /*=false*/)
{
	// ���z�����ݒ聁�t�`�d�̊J�n�_
	if (!this->grad) {
		// ���z�̏����l(1)��ݒ�
		auto g = nc::ones_like<data_t>(*this->data);
		this->grad = as_array(g);
	}

	// �֐����X�g
	auto funcs = std::list<FunctionPtr>();
	// �����ς݊֐��Z�b�g
	auto seen_set = std::set<FunctionPtr>();

	// �N���[�W���F�֐����X�g�֒ǉ�
	auto add_func = [&funcs, &seen_set](const FunctionPtr& f) {
		// ���X�g�֖��ǉ��̊֐��Ȃ�
		if (seen_set.find(f) == seen_set.end()) {
			// ���X�g�֒ǉ����Đ���ŏ����\�[�g����
			funcs.push_back(f);
			seen_set.insert(f);
			funcs.sort([](const FunctionPtr& lhs, const FunctionPtr& rhs) { return lhs->generation < rhs->generation; });
		}
	};

	// �ŏ��̊֐������X�g�ɒǉ�
	add_func(this->creator);

	// �֐����X�g����ɂȂ�܂Ń��[�v
	while (!funcs.empty()) {
		// ���X�g����֐������o��
		auto f = funcs.back();
		funcs.pop_back();

		// �o�̓f�[�^������z�����o��
		auto gys = NdArrayPtrList();
		for (const auto& o : f->outputs) {
			gys.push_back(o.lock()->grad);
		}

		// �t�`�d
		auto gxs = f->backward(gys);

		// ���̓f�[�^�ƎZ�o�������z�̗v�f���͈�v����K�v����
		assert(f->inputs.size() == gxs.size());

		// ���̓f�[�^�ƌ��z�̃y�A�����[�v
		for (size_t i = 0; i < gxs.size(); i++) {
			auto x = f->inputs[i];
			auto gx = gxs[i];

			// ���z�����ݒ�Ȃ�������
			if (!x->grad) {
				x->grad = gx;
			}
			// ���z���ݒ�ς݂Ȃ���Z����
			else {
				// �V���� NdArrayPtr �C���X�^���X����邱�Ƃ��d�v
				// �Ⴆ�΁A*x->grad += *gx; �Ƃ��Ă͂����Ȃ��i�t�^A�Q�Ɓj
				x->grad = as_array(*x->grad + *gx);
			}

			// �P�O�̊֐������X�g�ɒǉ�
			if (x->creator) {
				add_func(x->creator);
			}
		}

		// ���z��ێ����Ȃ��ꍇ
		if (!retain_grad) {
			// ���z���폜
			for (const auto& y : f->outputs) {
				y.lock()->grad = nullptr;
			}
		}
	}
}

// �֐��N���X�i���Z�j
class Add : public Function
{
public:
	// ���`�d
	NdArrayPtrList forward(const NdArrayPtrList& xs) override
	{
		auto x0 = *(xs[0]);
		auto x1 = *(xs[1]);
		auto y = x0 + x1;
		return { as_array(y) };
	}
	// �t�`�d
	NdArrayPtrList backward(const NdArrayPtrList& gys) override
	{
		return { gys[0], gys[0] };
	}
};

// �֐��N���X�i���Z�j
class Sub : public Function
{
public:
	// ���`�d
	NdArrayPtrList forward(const NdArrayPtrList& xs) override
	{
		auto x0 = *(xs[0]);
		auto x1 = *(xs[1]);
		auto y = x0 - x1;
		return { as_array(y) };
	}
	// �t�`�d
	NdArrayPtrList backward(const NdArrayPtrList& gys) override
	{
		auto gy = *(gys[0]);
		return { as_array(gy), as_array(-gy) };
	}
};

// �֐��N���X�i��Z�j
class Mul : public Function
{
public:
	// ���`�d
	NdArrayPtrList forward(const NdArrayPtrList& xs) override
	{
		auto x0 = *(xs[0]);
		auto x1 = *(xs[1]);
		auto y = x0 * x1;
		return { as_array(y) };
	}
	// �t�`�d
	NdArrayPtrList backward(const NdArrayPtrList& gys) override
	{
		auto x0 = *(this->inputs[0]->data);
		auto x1 = *(this->inputs[1]->data);
		auto gy = *(gys[0]);
		return { as_array(gy * x1), as_array(gy * x0) };
	}
};

// �֐��N���X�i���Z�j
class Div : public Function
{
public:
	// ���`�d
	NdArrayPtrList forward(const NdArrayPtrList& xs) override
	{
		auto x0 = *(xs[0]);
		auto x1 = *(xs[1]);
		auto y = x0 / x1;
		return { as_array(y) };
	}
	// �t�`�d
	NdArrayPtrList backward(const NdArrayPtrList& gys) override
	{
		auto x0 = *(this->inputs[0]->data);
		auto x1 = *(this->inputs[1]->data);
		auto gy = *(gys[0]);
		auto gx0 = as_array(gy / x1);
		auto gx1 = as_array(gy * (-x0 / nc::power(x1, 2)));
		return { gx0, gx1 };
	}
};

// �֐��N���X�i�����j
class Pos : public Function
{
public:
	// ���`�d
	NdArrayPtrList forward(const NdArrayPtrList& xs) override
	{
		// ���͒l�����̂܂ܕԂ����������A���`�d�ł͐V�����C���X�^���X�ɂ���K�v������
		auto x = *(xs[0]);
		return { as_array(x) };
	}
	// �t�`�d
	NdArrayPtrList backward(const NdArrayPtrList& gys) override
	{
		return gys;
	}
};

// �֐��N���X�i�����j
class Neg : public Function
{
public:
	// ���`�d
	NdArrayPtrList forward(const NdArrayPtrList& xs) override
	{
		auto x = *(xs[0]);
		return { as_array(-x) };
	}
	// �t�`�d
	NdArrayPtrList backward(const NdArrayPtrList& gys) override
	{
		auto gy = *(gys[0]);
		return { as_array(-gy) };
	}
};

// �֐��N���X�i�ݏ�j
class Pow : public Function
{
public:
	uint32_t c;

	// �R���X�g���N�^
	Pow(uint32_t c) : c(c) {}

	// ���`�d
	NdArrayPtrList forward(const NdArrayPtrList& xs) override
	{
		auto x = *(xs[0]);
		auto y = nc::power(x, this->c);
		return { as_array(y) };
	}
	// �t�`�d
	NdArrayPtrList backward(const NdArrayPtrList& gys) override
	{
		auto x = *(this->inputs[0]->data);
		auto gy = *(gys[0]);
		auto c = this->c;
		auto gx = static_cast<data_t>(c) * nc::power(x, c - 1)  * gy;
		return { as_array(gx) };
	}
};

//----------------------------------
// function
//----------------------------------

inline std::ostream& operator<<(std::ostream& ost, const NdArrayPrinter& nda)
{
	// nullptr �̏ꍇ
	if (!nda.data) ost << "Null";
	// NdArray���X�J���[�Ȃ璆�g�̃f�[�^��W���o�͂�
	else if (nda.data->shape().rows == 1 && nda.data->shape().cols == 1) ost << (*nda.data)[0];
	// �ʏ펞
	else ost << *(nda.data);
	return ost;
}

extern std::string replace_all(const std::string& target_str, const std::string& old_str, const std::string& new_str);

inline std::ostream& operator<<(std::ostream& ost, const Variable& v)
{
	std::ostringstream osst;
	// �W���o�͂̏����_�ȉ������� 15 �Ƃ���
	osst << std::fixed << std::setprecision(15);
	osst << NdArrayPrinter(v.data);
	auto str = osst.str();

	// �����̉��s���폜
	if (str.back() == '\n') str.pop_back();

	// �r���̉��s�ɃC���f���g��ǉ�
	str = replace_all(str, "\n", "\n          ");

	ost << "variable(" << str << ")";
	return ost;
}

inline std::ostream& operator<<(std::ostream& ost, const VariablePtr& p)
{
	if (!p) ost << "variable(Null)";
	else ost << *p;
	return ost;
}

// ���Z
inline VariablePtr add(const VariablePtr& x0, const VariablePtr& x1)
{
	return (*std::shared_ptr<Function>(new Add()))({ x0, x1 })[0];
}

// ���Z
inline VariablePtr sub(const VariablePtr& x0, const VariablePtr& x1)
{
	return (*std::shared_ptr<Function>(new Sub()))({ x0, x1 })[0];
}

// ��Z
inline VariablePtr mul(const VariablePtr& x0, const VariablePtr& x1)
{
	return (*std::shared_ptr<Function>(new Mul()))({ x0, x1 })[0];
}

// ���Z
inline VariablePtr div(const VariablePtr& x0, const VariablePtr& x1)
{
	return (*std::shared_ptr<Function>(new Div()))({ x0, x1 })[0];
}

// ����
inline VariablePtr pos(const VariablePtr& x)
{
	return (*std::shared_ptr<Function>(new Pos()))({ x })[0];
}

// ����
inline VariablePtr neg(const VariablePtr& x)
{
	return (*std::shared_ptr<Function>(new Neg()))({ x })[0];
}

// �ݏ�
inline VariablePtr power(const VariablePtr& x0, uint32_t c)
{
	return (*std::shared_ptr<Function>(new Pow(c)))(x0)[0];
}
inline VariablePtr power(const NdArrayPtr& x, uint32_t c)
{
	return power(as_variable(x), c);
}
inline VariablePtr power(data_t x, uint32_t c)
{
	return power(as_variable(as_array(x)), c);
}

// VariablePtr�̉��Z�q�I�[�o�[���[�h
// �񍀉��Z�q +
inline VariablePtr operator+(const VariablePtr& lhs, const VariablePtr& rhs) { return add(lhs, rhs); }
inline VariablePtr operator+(const VariablePtr& lhs, const NdArrayPtr& rhs) { return add(lhs, as_variable(rhs)); }
inline VariablePtr operator+(const NdArrayPtr& lhs, const VariablePtr& rhs) { return add(as_variable(lhs), rhs); }
inline VariablePtr operator+(const VariablePtr& lhs, data_t rhs) { return add(lhs, as_variable(as_array(rhs))); }
inline VariablePtr operator+(data_t lhs, const VariablePtr& rhs) { return add(as_variable(as_array(lhs)), rhs); }
// �񍀉��Z�q -
inline VariablePtr operator-(const VariablePtr& lhs, const VariablePtr& rhs) { return sub(lhs, rhs); }
inline VariablePtr operator-(const VariablePtr& lhs, const NdArrayPtr& rhs) { return sub(lhs, as_variable(rhs)); }
inline VariablePtr operator-(const NdArrayPtr& lhs, const VariablePtr& rhs) { return sub(as_variable(lhs), rhs); }
inline VariablePtr operator-(const VariablePtr& lhs, data_t rhs) { return sub(lhs, as_variable(as_array(rhs))); }
inline VariablePtr operator-(data_t lhs, const VariablePtr& rhs) { return sub(as_variable(as_array(lhs)), rhs); }
// �񍀉��Z�q *
inline VariablePtr operator*(const VariablePtr& lhs, const VariablePtr& rhs) { return mul(lhs, rhs); }
inline VariablePtr operator*(const VariablePtr& lhs, const NdArrayPtr& rhs) { return mul(lhs, as_variable(rhs)); }
inline VariablePtr operator*(const NdArrayPtr& lhs, const VariablePtr& rhs) { return mul(as_variable(lhs), rhs); }
inline VariablePtr operator*(const VariablePtr& lhs, data_t rhs) { return mul(lhs, as_variable(as_array(rhs))); }
inline VariablePtr operator*(data_t lhs, const VariablePtr& rhs) { return mul(as_variable(as_array(lhs)), rhs); }
// �񍀉��Z�q /
inline VariablePtr operator/(const VariablePtr& lhs, const VariablePtr& rhs) { return div(lhs, rhs); }
inline VariablePtr operator/(const VariablePtr& lhs, const NdArrayPtr& rhs) { return div(lhs, as_variable(rhs)); }
inline VariablePtr operator/(const NdArrayPtr& lhs, const VariablePtr& rhs) { return div(as_variable(lhs), rhs); }
inline VariablePtr operator/(const VariablePtr& lhs, data_t rhs) { return div(lhs, as_variable(as_array(rhs))); }
inline VariablePtr operator/(data_t lhs, const VariablePtr& rhs) { return div(as_variable(as_array(lhs)), rhs); }
// �P�����Z�q +
inline VariablePtr operator+(const VariablePtr& data) { return pos(data); }
// �P�����Z�q -
inline VariablePtr operator-(const VariablePtr& data) { return neg(data); }

}	// namespace dezerocpp
