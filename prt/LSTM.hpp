#pragma once
//original source https://github.com/byronknoll/lstm-compress
// v3
#include <valarray>
#include <vector>
#include <memory>

static float lstmerr=0.000021;

namespace LSTM {
 
class Sigmoid {
 public:
  Sigmoid(int logit_size);
  float Logit(float p) const;
  static float Logistic(float p);

 private:
  float SlowLogit(float p);
  int logit_size_;
  std::vector<float> logit_table_;
};



struct NeuronLayer {
  NeuronLayer(unsigned int input_size, unsigned int num_cells, int horizon,
    int offset) : error_(num_cells), ivar_(horizon), gamma_(1.0, num_cells),
    gamma_u_(num_cells), gamma_m_(num_cells), gamma_v_(num_cells),
    beta_(num_cells), beta_u_(num_cells), beta_m_(num_cells),
    beta_v_(num_cells), weights_(std::valarray<float>(input_size), num_cells),
    state_(std::valarray<float>(num_cells), horizon),
    update_(std::valarray<float>(input_size), num_cells),
    m_(std::valarray<float>(input_size), num_cells),
    v_(std::valarray<float>(input_size), num_cells),
    transpose_(std::valarray<float>(num_cells), input_size - offset),
    norm_(std::valarray<float>(num_cells), horizon) {};

  std::valarray<float> error_, ivar_, gamma_, gamma_u_, gamma_m_, gamma_v_,
      beta_, beta_u_, beta_m_, beta_v_;
  std::valarray<std::valarray<float>> weights_, state_, update_, m_, v_,
      transpose_, norm_;
};

class LstmLayer {
 public:
  LstmLayer(unsigned int input_size, unsigned int auxiliary_input_size,
      unsigned int output_size, unsigned int num_cells, int horizon,
      float gradient_clip, float learning_rate);
  void ForwardPass(const std::valarray<float>& input, int input_symbol,
      std::valarray<float>* hidden, int hidden_start);
  void BackwardPass(const std::valarray<float>& input, int epoch,
      int layer, int input_symbol, std::valarray<float>* hidden_error);
  static inline float Rand() {
    return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
  }
  std::vector<std::valarray<std::valarray<float>>*> Weights();

 private:
  std::valarray<float> state_, state_error_, stored_error_;
  std::valarray<std::valarray<float>> tanh_state_, input_gate_state_,
      last_state_;
  float gradient_clip_, learning_rate_;
  unsigned int num_cells_, epoch_, horizon_, input_size_, output_size_;
  unsigned long long update_steps_ = 0;
  NeuronLayer forget_gate_, input_node_, output_gate_;

  void ClipGradients(std::valarray<float>* arr);
  void ForwardPass(NeuronLayer& neurons, const std::valarray<float>& input,
      int input_symbol);
  void BackwardPass(NeuronLayer& neurons, const std::valarray<float>&input,
      int epoch, int layer, int input_symbol,
      std::valarray<float>* hidden_error);
      void Adam(std::valarray<float>* g, std::valarray<float>* m,
    std::valarray<float>* v, std::valarray<float>* w, float learning_rate,
    float t) {
  float beta1 = 0.025, beta2 = 0.9999, alpha = learning_rate * 0.1 /
      sqrt(5e-5 * t + 1), eps = 1e-6;
  (*m) *= beta1;
  (*m) += (1 - beta1) * (*g);
  (*v) *= beta2;
  (*v) += (1 - beta2) * (*g) * (*g);
  (*w) -= alpha * (((*m) / (float)(1 - pow(beta1, t))) /
      (sqrt((*v) / (float)(1 - pow(beta2, t)) + eps)));
}
};

 
 
class Lstm {
 public:
  Lstm(unsigned int input_size, unsigned int output_size, unsigned int
      num_cells, unsigned int num_layers, int horizon, float learning_rate,
      float gradient_clip);
  std::valarray<float>& Perceive(unsigned int input);
  std::valarray<float>& Predict(unsigned int input);
  int ep();

 private:
  std::vector<std::unique_ptr<LstmLayer>> layers_;
  std::vector<unsigned int> input_history_;
  std::valarray<float> hidden_, hidden_error_;
  std::valarray<std::valarray<std::valarray<float>>> layer_input_,
      output_layer_;
  std::valarray<std::valarray<float>> output_;
  float learning_rate_;
  unsigned int num_cells_, epoch_, horizon_, input_size_, output_size_;
};



class ByteModel {
 public:
  ByteModel(unsigned int num_cells, unsigned int num_layers, int horizon,
      float learning_rate);
      unsigned int Discretize(float p) ;
  unsigned int Predict();
  void Perceive(int bit);
int epoch();
int expected();
 protected:
  int top_, mid_, bot_;
  std::valarray<float> probs_;
  unsigned int bit_context_;
  int ex;
  Lstm lstm_;
};

}
