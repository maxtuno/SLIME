#pragma once

#include "dezero.hpp"

namespace dz::utils
{

//----------------------------------
// General Utility
//----------------------------------

// ������̑S�u��
inline std::string replace_all(const std::string& target_str, const std::string& old_str, const std::string& new_str)
{
	std::string result_str = target_str;
	std::string::size_type Pos(result_str.find(old_str));
	while (Pos != std::string::npos)
	{
		result_str.replace(Pos, old_str.length(), new_str);
		Pos = result_str.find(old_str, Pos + new_str.length());
	}
	return result_str;
}

// �K��v�Z
inline double factorial(uint32_t x)
{
	// 32bit�����ł̓I�[�o�[�t���[����̂�double�Ƃ���
	double y = 1.0;
	for (; x > 0; x--) {
		y *= static_cast<double>(x);
	}
	return y;
}

// NdArray�p�� broadcast_to
// ��NdArray�͍s��Ɏ����Œ肳��Ă��邽�߁A�����O��Ƃ����ȈՏ����Ƃ���
inline NdArray broadcast_to(const NdArray& in_array, const nc::Shape& shape)
{
	// �u���[�h�L���X�g�\���`�F�b�N
	//assert(in_array.shape().rows == 1 || in_array.shape().rows == shape.rows);
	//assert(in_array.shape().cols == 1 || in_array.shape().cols == shape.cols);

	NdArray out_array;

	// �X�J���[���u���[�h�L���X�g
	if (in_array.shape().rows == 1 && in_array.shape().cols == 1) {
		// �X�J���[�̒l�őS�v�f�𖄂߂�
		out_array = NdArray(shape).fill(0);
		out_array.fill(in_array[0]);
	}
	// �s�����̃u���[�h�L���X�g
	else if (in_array.shape().rows == 1) {
		// ���̓f�[�^�̍s�����̃x�N�g�����擾
		std::vector<data_t> row_vec = in_array.toStlVector();
		// �u���[�h�L���X�g���čs��Ɋg��
		std::vector<std::vector<data_t>> mat;
		for (uint32_t i = 0; i < shape.rows; i++) mat.push_back(row_vec);
		out_array = NdArray(mat);
	}
	// ������̃u���[�h�L���X�g
	else if (in_array.shape().rows == 1) {
		// ���̓f�[�^�̗�����̃x�N�g�����擾
		std::vector<data_t> col_vec = in_array.transpose().toStlVector();
		// �s�����Ƀu���[�h�L���X�g���čs��Ɋg�����Ă���]�n����
		std::vector<std::vector<data_t>> mat;
		for (uint32_t i = 0; i < shape.cols; i++) mat.push_back(col_vec);
		out_array = NdArray(mat).transpose();
	}
	else {
		// �u���[�h�L���X�g�s�i�s�v�j�̏ꍇ�͕ϊ����Ȃ�
		out_array = in_array;
	}

	return out_array;
}

// NdArray�p�� sum_to
// ��NdArray�͍s��Ɏ����Œ肳��Ă��邽�߂��̑O��̏����Ƃ���
inline NdArray sum_to(const NdArray& in_array, const nc::Shape& shape)
{
	// �v�Z�\���`�F�b�N
	assert(shape.rows == 1 || in_array.shape().rows == shape.rows);
	assert(shape.cols == 1 || in_array.shape().cols == shape.cols);

	NdArray out_array;

	// �X�J���[�֍��v
	if (shape.rows == 1 && shape.cols == 1) {
		out_array = in_array.sum();
	}
	// �s�����̍��v
	else if (shape.rows == 1) {
		out_array = NdArray(shape).fill(0);
		// �S�s���[�v
		for (uint32_t r = 0; r < in_array.shape().rows; r++) {
			// 1�s�����Z����
			out_array += in_array(r, in_array.cSlice());
		}
	}
	// ������̍��v
	else if (shape.cols == 1) {
		out_array = NdArray(shape).fill(0);
		// �S�s���[�v
		for (uint32_t c = 0; c < in_array.shape().cols; c++) {
			// 1�񂸂��Z����
			out_array += in_array(in_array.rSlice(), c);
		}
	}
	else {
		out_array = in_array;
	}

	return out_array;
}

// 2�� NdArray �𑊌ݓI�Ƀu���[�h�L���X�g����
// ��NdArray�͎l�����Z�̍ۂȂǂɎ����I�Ƀu���[�h�L���X�g���s���Ȃ����ߖ����I�ɂ��̊֐��𗘗p����
inline void broadcast_mutual(NdArray& a0, NdArray& a1)
{
	auto a0_shape = a0.shape();
	auto a1_shape = a1.shape();
	a0 = broadcast_to(a0, a1_shape);
	a1 = broadcast_to(a1, a0_shape);
}

//----------------------------------
// DOT Language
//----------------------------------

// �I�u�W�F�N�g��ID�i�A�h���X�l�j���擾
template<typename T>
static uintptr_t id(const std::shared_ptr<T>& d)
{
	// VariablePtr, FunctionPtr �̂ǂ��炩��z�肷�邽��
	// std::shared_ptr �^�̂ݎg�p�ł���悤�ɐ���
	return reinterpret_cast<uintptr_t>(&*d);
}

// Variable��DOT�o��
static std::string dot_var(const VariablePtr& v, bool verbose = false)
{
	std::ostringstream osst;

	// �ϐ��̖���
	std::string name = v->name;

	if (verbose && v->data) {
		if (!v->name.empty()) {
			name += ": ";
		}

		// ���̂ɉ����āA�f�[�^�̌`��ƌ^���o��
		osst << name << v->shape() << " " << typeid(data_t).name();
		name = osst.str();
		name = replace_all(name, "\n", "");
		// osst���N���A
		osst.str("");
		osst.clear();
	}

	// �ϐ��m�[�h��dot�`���ŏo��
	osst << id(v) << " [label=\"" << name << "\", color=orange, style=filled]" << std::endl;
	return osst.str();
}

// Function��DOT�o��
static std::string dot_func(const FunctionPtr& f)
{
	std::ostringstream osst;
	std::string txt;

	// �N���X�����擾
	std::string class_name = typeid(*f).name();
	// ���O��ԂȂǂ̏A�E������ꍇ�́A�N���X���݂̂𒊏o
	auto cpos = class_name.find_last_of(":");
	if (cpos != std::string::npos) {
		cpos += 1;
		class_name = class_name.substr(cpos, class_name.size() - cpos);
	}

	// �֐��m�[�h��dot�`���ŏo��
	osst << id(f) << " [label=\"" << class_name << "\", color=lightblue, style=filled, shape=box]" << std::endl;
	txt = osst.str();
	// osst���N���A
	osst.str("");
	osst.clear();

	for (const auto& x : f->inputs) {
		// ���̓f�[�^�Ɗ֐��̃G�b�W��dot�`���ŏo��
		osst << id(x) << " -> " << id(f) << std::endl;
		txt += osst.str();
		// osst���N���A
		osst.str("");
		osst.clear();
	}
	for (const auto& y : f->outputs) {
		// �֐��Əo�̓f�[�^�̃G�b�W��dot�`���ŏo��
		osst << id(f) << " -> " << id(y.lock()) << std::endl;
		txt += osst.str();
		// osst���N���A
		osst.str("");
		osst.clear();
	}

	return txt;
}

// DOT�O���t���o��
inline std::string get_dot_graph(const VariablePtr& output, bool verbose = true)
{
	// dot�`���̕�����
	std::string txt;
	// �֐����X�g
	auto funcs = std::list<FunctionPtr>();
	// �����ς݊֐��Z�b�g
	auto seen_set = std::set<FunctionPtr>();

	// �N���[�W���F�֐����X�g�֒ǉ�
	auto add_func = [&funcs, &seen_set](const FunctionPtr& f) {
		// ���X�g�֖��ǉ��̊֐��Ȃ�
		if (seen_set.find(f) == seen_set.end()) {
			// ���X�g�֒ǉ�
			funcs.push_back(f);
			seen_set.insert(f);
		}
	};

	// �ŏ��̊֐������X�g�ɒǉ�
	add_func(output->creator);
	// �ŏ��̕ϐ���dot�`���ŏo��
	txt += dot_var(output, verbose);

	// �֐����X�g����ɂȂ�܂Ń��[�v
	while (!funcs.empty()) {
		// ���X�g����֐������o��
		auto f = funcs.back();
		funcs.pop_back();
		// �֐���dot�`���ŏo��
		txt += dot_func(f);

		// ���̓f�[�^�ƌ��z�̃y�A�����[�v
		for (const auto& x : f->inputs) {
			// �ϐ���dot�`���ŏo��
			txt += dot_var(x, verbose);

			// �P�O�̊֐������X�g�ɒǉ�
			if (x->creator) {
				add_func(x->creator);
			}
		}
	}

	// dot�`���̑S�̂𐮂��ďo��
	return "digraph g {\n" + txt + "}";
}

// DOT�O���t����w��t�@�C���Ƀ����_�����O
inline void plot_dot_graph(const VariablePtr& output, bool verbose /*=true*/, const std::string& to_file /*="graph.png"*/)
{
	// dot�`���̃f�[�^�𐶐�
	std::string dot_graph = get_dot_graph(output, verbose);

	// �o�̓t�@�C���̃p�X���
	std::filesystem::path p = to_file;

	// dot�t�@�C����ۑ�
	std::string graph_path = p.parent_path().string() + "tmp_graph.dot";	// �摜�Ɠ����ꏊ�ɏo��
	std::ofstream f(graph_path);
	f << dot_graph;
	f.close();

	// dot�R�}���h�����s
	auto ext = p.extension().string();
	ext = ext.substr(1);	// �擪�� '.' ���폜
	std::string cmd = "dot " + graph_path + " -T " + ext + " -o " + to_file;
	std::system(cmd.c_str());
}

}	// namespace dz::utils
