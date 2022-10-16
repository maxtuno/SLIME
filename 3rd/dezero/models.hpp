#pragma once

#include "dezero.hpp"

namespace F = dz::functions;
namespace L = dz::layers;

namespace dz::models
{

//----------------------------------
// class
//----------------------------------

// ���f���N���X
class Model : public layers::Layer
{
public:
	// �v�Z�O���t���v���b�g
	void plot(const VariablePtrList& inputs, const std::string& to_file = "model.png") {
		auto y = this->forward(inputs);
		utils::plot_dot_graph(y[0], true, to_file);
	}
};

// ���w�p�[�Z�v�g���� (MLP: Multi-Layer Perceptron) �N���X
class MLP : public Model
{
private:
	// ���C���[���X�g
	std::vector<L::LayerPtr> layers;
	// �������֐�
	std::function<F::function_t> activation;

public:
	// �R���X�g���N�^
	MLP(std::vector<int> fc_output_sizes, F::function_t* activation = F::sigmoid) :
		activation(activation)
	{
		int i = 0;
		for (auto out_size : fc_output_sizes) {
			auto layer = std::make_shared<L::Linear>(out_size);
			// ���C���[���v���p�e�B�Ƃ��ēo�^
			std::ostringstream osst;
			osst << "l" << i;
			this->prop(osst.str()) = layer;
			// ���g�̃��C���[���X�g�ɂ��o�^
			this->layers.push_back(layer);
			i++;
		}
	}

	// ���`�d
	VariablePtrList forward(const VariablePtrList& xs) override
	{
		// �Ō�̂P�O�̃��C���܂�
		auto xs_tmp = xs;
		for (auto iter = this->layers.begin(); iter != this->layers.end() - 1; iter++) {
			auto& l = **iter;
			xs_tmp = this->activation(l(xs_tmp));
		}

		// �Ō�̃��C��
		auto &l = *this->layers.back();
		return l(xs_tmp);
	}
};

}	// namespace dz::models
