#pragma once

#include "dezero.hpp"

namespace L = dz::layers;

namespace dz::optimizers
{

//----------------------------------
// class
//----------------------------------

// �œK���N���X
class Optimizer
{
public:
	// �O�����̊֐��^
	using hook_t = void(const L::params_t&);

	// �Ώۃ��C���[
	L::LayerPtr target;
	// �O�����R���N�V����
	std::list<std::function<hook_t>> hooks;

	// �����ݒ�
	Optimizer& setup(L::LayerPtr target)
	{
		this->target = target;
		return *this;
	}
	Optimizer& setup(L::Layer* target)
	{
		return this->setup(L::LayerPtr(target));
	}

	// �X�V����
	void update()
	{
		// ���z���ݒ肳��Ă���p�����[�^���܂Ƃ߂�
		L::params_t params;
		for (auto& p : this->target->params()) {
			if (p->grad) params.insert(p);
		}
		// �O�����i�I�v�V�����j
		for (auto f : this->hooks) {
			f(params);
		}
		// �p�����[�^�̍X�V
		for (auto& param : params) {
			this->update_one(param);
		}
	}

	// �p�����[�^�X�V
	virtual void update_one(const VariablePtr& param) = 0;

	// �O�����o�^
	void add_hook(std::function<hook_t>& f)
	{
		hooks.push_back(f);
	}
};

// ���z�~���@ (SGD: Stochastic Gradient Descent) �N���X
class SGD : public Optimizer
{
public:
	// �w�K�W��
	double lr;

	// �R���X�g���N�^
	SGD(double lr = 0.01) :
		lr(lr)
	{}

	// �p�����[�^�X�V
	void update_one(const VariablePtr& param) override
	{
		// ���z�Ɋw�K�W������Z�����l�Ńp�����[�^���X�V
		*(param->data) -= this->lr * *(param->grad->data);
	}
};

// MomentumSGD�N���X
class MomentumSGD : public Optimizer
{
public:
	double lr;
	double momentum;
	std::unordered_map<uintptr_t, NdArrayPtr> vs;

	// �R���X�g���N�^
	MomentumSGD(double lr = 0.01, double momentum = 0.9) :
		lr(lr),
		momentum(momentum)
	{}

	// �p�����[�^�X�V
	void update_one(const VariablePtr& param) override
	{
		// �p�����[�^�̃��j�[�NID���擾
		auto v_key = utils::id(param);
		// ID�����o�^�Ȃ�p�����[�^�Ɠ��`���0�e���\���ŏ�����
		if (this->vs.find(v_key) == this->vs.end()) {
			this->vs[v_key] = as_array(nc::zeros_like<data_t>(*param->data));
		}

		// �p�����[�^���X�V
		auto v = this->vs[v_key];
		*v *= this->momentum;
		*v -= this->lr * *param->grad->data;
		*param->data += *v;
	}
};

}	// namespace dz::optimizers
