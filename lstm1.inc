//original source https://github.com/byronknoll/lstm-compress

#include <math.h>
#include <algorithm>
#include <numeric> 
#include <valarray>
#include <stdlib.h>
#include <vector>
#include <memory>
#include <cstdlib>
#include <stdio.h>
namespace LSTM {
class Layer {
 public:
  Layer(unsigned int input_size, unsigned int auxiliary_input_size,
      unsigned int output_size, unsigned int num_cells, int horizon,
      float learning_rate);
  const std::valarray<float>& ForwardPass(const std::valarray<float>& input);
  const std::valarray<float>& BackwardPass(const std::valarray<float>& input,
      const std::valarray<float>& hidden_error, int epoch);
  static inline float Rand() {
    return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
  }
  static inline float Logistic(float val) { return 1 / (1 + exp(-val)); }

 private:
  std::valarray<float> state_, hidden_, hidden_error_, output_gate_error_,
      state_error_, input_node_error_, input_gate_error_, forget_gate_error_,
      stored_error_;
  std::valarray<std::valarray<float>> tanh_state_, output_gate_state_,
      input_node_state_, input_gate_state_, forget_gate_state_, last_state_,
      forget_gate_, input_node_, input_gate_, output_gate_, forget_gate_update_,
      input_node_update_, input_gate_update_, output_gate_update_;
  float learning_rate_;
  unsigned int num_cells_, epoch_, horizon_, input_size_, output_size_;
};

Layer::Layer(unsigned int input_size, unsigned int auxiliary_input_size,
    unsigned int output_size, unsigned int num_cells, int horizon,
    float learning_rate) : state_(num_cells), hidden_(num_cells),
    hidden_error_(num_cells), output_gate_error_(num_cells),
    state_error_(num_cells), input_node_error_(num_cells),
    input_gate_error_(num_cells), forget_gate_error_(num_cells),
    stored_error_(num_cells),
    tanh_state_(std::valarray<float>(num_cells), horizon),
    output_gate_state_(std::valarray<float>(num_cells), horizon),
    input_node_state_(std::valarray<float>(num_cells), horizon),
    input_gate_state_(std::valarray<float>(num_cells), horizon),
    forget_gate_state_(std::valarray<float>(num_cells), horizon),
    last_state_(std::valarray<float>(num_cells), horizon),
    forget_gate_(std::valarray<float>(input_size), num_cells),
    input_node_(std::valarray<float>(input_size), num_cells),
    input_gate_(std::valarray<float>(input_size), num_cells),
    output_gate_(std::valarray<float>(input_size), num_cells),
    forget_gate_update_(std::valarray<float>(input_size), num_cells),
    input_node_update_(std::valarray<float>(input_size), num_cells),
    input_gate_update_(std::valarray<float>(input_size), num_cells),
    output_gate_update_(std::valarray<float>(input_size), num_cells),
    learning_rate_(learning_rate), num_cells_(num_cells), epoch_(0),
    horizon_(horizon), input_size_(auxiliary_input_size),
    output_size_(output_size) {
  float low = -0.2;
  float range = 0.4;
  for (unsigned int i = 0; i < forget_gate_.size(); ++i) {
    for (unsigned int j = 0; j < forget_gate_[i].size(); ++j) {
      forget_gate_[i][j] = low + Rand() * range;
      input_node_[i][j] = low + Rand() * range;
      input_gate_[i][j] = low + Rand() * range;
      output_gate_[i][j] = low + Rand() * range;
    }
    forget_gate_[i][forget_gate_[i].size() - 1] = 1;
  }
}

const std::valarray<float>& Layer::ForwardPass(const std::valarray<float>&
    input) {
  last_state_[epoch_] = state_;
  for (unsigned int i = 0; i < state_.size(); ++i) {
    forget_gate_state_[epoch_][i] = Logistic(std::inner_product(&input[0],
        &input[input.size()], &forget_gate_[i][0], 0.0));
    state_[i] *= forget_gate_state_[epoch_][i];
    input_node_state_[epoch_][i] = tanh(std::inner_product(&input[0],
        &input[input.size()], &input_node_[i][0], 0.0));
    input_gate_state_[epoch_][i] = Logistic(std::inner_product(&input[0],
        &input[input.size()], &input_gate_[i][0], 0.0));
    state_[i] += input_node_state_[epoch_][i] * input_gate_state_[epoch_][i];
    tanh_state_[epoch_][i] = tanh(state_[i]);
    output_gate_state_[epoch_][i] = Logistic(std::inner_product(&input[0],
        &input[input.size()], &output_gate_[i][0], 0.0));
    hidden_[i] = output_gate_state_[epoch_][i] * tanh_state_[epoch_][i];
  }
  ++epoch_;
  if (epoch_ == horizon_) epoch_ = 0;
  return hidden_;
}

void ClipGradients(std::valarray<float>* arr) {
  for (unsigned int i = 0; i < arr->size(); ++i) {
    if ((*arr)[i] < -2) (*arr)[i] = -2;
    else if ((*arr)[i] > 2) (*arr)[i] = 2;
  }
}

const std::valarray<float>& Layer::BackwardPass(const std::valarray<float>&
    input, const std::valarray<float>& hidden_error, int epoch) {
  if (epoch == (int)horizon_ - 1) {
    stored_error_ = hidden_error;
    state_error_ = 0;
    for (unsigned int i = 0; i < input_node_.size(); ++i) {
      forget_gate_update_[i] = 0;
      input_node_update_[i] = 0;
      input_gate_update_[i] = 0;
      output_gate_update_[i] = 0;
    }
  } else {
    stored_error_ += hidden_error;
  }

  output_gate_error_ = tanh_state_[epoch] * stored_error_ *
      output_gate_state_[epoch] * (1.0f - output_gate_state_[epoch]);
  state_error_ += stored_error_ * output_gate_state_[epoch] * (1.0f -
      (tanh_state_[epoch] * tanh_state_[epoch]));
  input_node_error_ = state_error_ * input_gate_state_[epoch] * (1.0f -
      (input_node_state_[epoch] * input_node_state_[epoch]));
  input_gate_error_ = state_error_ * input_node_state_[epoch] *
      input_gate_state_[epoch] * (1.0f - input_gate_state_[epoch]);
  forget_gate_error_ = state_error_ * last_state_[epoch] *
      forget_gate_state_[epoch] * (1.0f - forget_gate_state_[epoch]);

  hidden_error_ = 0;
  if (input.size() > output_size_ + 1 + num_cells_ + input_size_) {
    int offset = output_size_ + num_cells_ + input_size_;
    for (unsigned int i = 0; i < input_node_.size(); ++i) {
      for (unsigned int j = offset; j < input.size() - 1; ++j) {
        hidden_error_[j-offset] += input_node_[i][j] * input_node_error_[i];
        hidden_error_[j-offset] += input_gate_[i][j] * input_gate_error_[i];
        hidden_error_[j-offset] += forget_gate_[i][j] * forget_gate_error_[i];
        hidden_error_[j-offset] += output_gate_[i][j] * output_gate_error_[i];
      }
    }
  }

  if (epoch > 0) {
    state_error_ *= forget_gate_state_[epoch];
    stored_error_ = 0;
    for (unsigned int i = 0; i < input_node_.size(); ++i) {
      int offset = output_size_ + input_size_;
      for (unsigned int j = offset; j < offset + num_cells_; ++j) {
        stored_error_[j-offset] += input_node_[i][j] * input_node_error_[i];
        stored_error_[j-offset] += input_gate_[i][j] * input_gate_error_[i];
        stored_error_[j-offset] += forget_gate_[i][j] * forget_gate_error_[i];
        stored_error_[j-offset] += output_gate_[i][j] * output_gate_error_[i];
      }
    }
  }

  ClipGradients(&state_error_);
  ClipGradients(&stored_error_);
  ClipGradients(&hidden_error_);

  for (unsigned int i = 0; i < input_node_.size(); ++i) {
    forget_gate_update_[i] += (learning_rate_ * forget_gate_error_[i]) * input;
    input_node_update_[i] += (learning_rate_ * input_node_error_[i]) * input;
    input_gate_update_[i] += (learning_rate_ * input_gate_error_[i]) * input;
    output_gate_update_[i] += (learning_rate_ * output_gate_error_[i]) * input;
  }
  if (epoch == 0) {
    for (unsigned int i = 0; i < input_node_.size(); ++i) {
        forget_gate_[i] += forget_gate_update_[i];
        input_node_[i] += input_node_update_[i];
        input_gate_[i] += input_gate_update_[i];
        output_gate_[i] += output_gate_update_[i];
    }
  }
  return hidden_error_;
}


class Lstm {
 public:
  Lstm(unsigned int input_size, unsigned int output_size, unsigned int
      num_cells, unsigned int num_layers, int horizon, float learning_rate);
  std::valarray<float>& Perceive(unsigned int input);
  std::valarray<float>& Predict(unsigned int input);
  void SetInput(int index, float val);
int ep();
 private:
  std::vector<std::unique_ptr<Layer>> layers_;
  std::vector<unsigned int> input_history_;
  std::valarray<float> hidden_, hidden_error_;
  std::valarray<std::valarray<std::valarray<float>>> layer_input_,
      output_layer_;
  std::valarray<std::valarray<float>> output_;
  float learning_rate_;
  unsigned int num_cells_, epoch_, horizon_, input_size_, output_size_;
};
Lstm::Lstm(unsigned int input_size, unsigned int output_size, unsigned int
    num_cells, unsigned int num_layers, int horizon, float learning_rate) :
    input_history_(horizon), hidden_(num_cells * num_layers + 1),
    hidden_error_(num_cells),
    layer_input_(std::valarray<std::valarray<float>>(std::valarray<float>
    (input_size + output_size + 1 + num_cells * 2), num_layers), horizon),
    output_layer_(std::valarray<std::valarray<float>>(std::valarray<float>
    (num_cells * num_layers + 1), output_size), horizon),
    output_(std::valarray<float>(1.0 / output_size, output_size), horizon),
    learning_rate_(learning_rate), num_cells_(num_cells), epoch_(0),
    horizon_(horizon), input_size_(input_size), output_size_(output_size) {
  hidden_[hidden_.size() - 1] = 1;
  for (int epoch = 0; epoch < horizon; ++epoch) {
    layer_input_[epoch][0].resize(output_size + 1 + num_cells + input_size);
    for (unsigned int i = 0; i < num_layers; ++i) {
      layer_input_[epoch][i][layer_input_[epoch][i].size() - 1] = 1;
    }
  }
  for (unsigned int i = 0; i < num_layers; ++i) {
    layers_.push_back(std::unique_ptr<Layer>(new Layer(layer_input_[0][i].
        size(), input_size_, output_size_, num_cells, horizon, learning_rate)));
  }
}

void Lstm::SetInput(int index, float val) {
  for (unsigned int i = 0; i < layers_.size(); ++i) {
    layer_input_[epoch_][i][output_size_ + index] = val;
  }
}

std::valarray<float>& Lstm::Perceive(unsigned int input) {
  int last_epoch = epoch_ - 1;
  if (last_epoch == -1) last_epoch = horizon_ - 1;
  input_history_[last_epoch] = input;
  if (epoch_ == 0) {
    for (int epoch = horizon_ - 1; epoch >= 0; --epoch) {
      for (int layer = layers_.size() - 1; layer >= 0; --layer) {
        int offset = layer * num_cells_;
        for (unsigned int i = 0; i < output_size_; ++i) {
          float error = 0;
          if (i == input_history_[epoch]) error = (1 - output_[epoch][i]);
          else error = -output_[epoch][i];
          for (unsigned int j = 0; j < hidden_error_.size(); ++j) {
            hidden_error_[j] += output_layer_[epoch][i][j + offset] * error;
          }
        }
        hidden_error_ = layers_[layer]->BackwardPass(layer_input_[epoch][layer],
            hidden_error_, epoch);
      }
    }
  }

  output_layer_[epoch_] = output_layer_[last_epoch];
  for (unsigned int i = 0; i < output_size_; ++i) {
    float error = 0;
    if (i == input) error = (1 - output_[last_epoch][i]);
    else error = -output_[last_epoch][i];
    output_layer_[epoch_][i] += learning_rate_ * error * hidden_;
  }
  return Predict(input);
}

std::valarray<float>& Lstm::Predict(unsigned int input) {
  for (unsigned int i = 0; i < layers_.size(); ++i) {
    std::fill_n(begin(layer_input_[epoch_][i]), output_size_, 0);
    layer_input_[epoch_][i][input] = 1;
    auto start = begin(hidden_) + i * num_cells_;
    std::copy(start, start + num_cells_, begin(layer_input_[epoch_][i]) +
        output_size_ + input_size_);
    const auto& hidden = layers_[i]->ForwardPass(layer_input_[epoch_][i]);
    std::copy(begin(hidden), end(hidden), start);
    if (i < layers_.size() - 1) {
      start = begin(layer_input_[epoch_][i + 1]) + output_size_ + num_cells_ +
          input_size_;
      std::copy(begin(hidden), end(hidden), start);
    }
  }
  for (unsigned int i = 0; i < output_size_; ++i) {
    output_[epoch_][i] = exp(std::inner_product(&hidden_[0],
        &hidden_[hidden_.size()], &output_layer_[epoch_][i][0], 0.0));
  }
  double sum = 0;
  for (unsigned int i = 0; i < output_size_; ++i) {
    sum += output_[epoch_][i];
  }
  output_[epoch_] /= sum;
  int epoch = epoch_;
  ++epoch_;
  if (epoch_ == horizon_) epoch_ = 0;
  return output_[epoch];
}
int Lstm::ep() {
  return epoch_;
}

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
ByteModel::ByteModel(unsigned int num_cells, unsigned int num_layers,
    int horizon, float learning_rate) : top_(255), mid_(0), bot_(0),
    probs_(1.0 / 256, 256), bit_context_(1),ex(0), lstm_(0, 256, num_cells,
    num_layers, horizon, learning_rate) {}
unsigned int ByteModel::Discretize(float p) {
  return 1 + 4094 * p;
}
unsigned int ByteModel::Predict() {
  float num = 0, denom = 0;
  mid_ = bot_ + ((top_ - bot_) / 2);
  for (int i = bot_; i <= top_; ++i) {
    denom += probs_[i];
    if (i > mid_) num += probs_[i];
  }
  ex = bot_;
    float max_prob_val = probs_[bot_];
    for (int i = bot_ + 1; i <= top_; i++) {
      if (probs_[i] > max_prob_val) {
        max_prob_val = probs_[i];
        ex =  i ;
      }
    }
  if (denom == 0) return Discretize(0.5);
  
  return Discretize(num / denom);
}

void ByteModel::Perceive(int bit) {
  if (bit) {
    bot_ = mid_ + 1;
  } else {
    top_ = mid_;
  }
  bit_context_ += bit_context_ + bit;
  if (bit_context_ >= 256) {
    bit_context_ -= 256;
    probs_ = lstm_.Perceive(bit_context_);
    bit_context_ = 1;
    top_ = 255;
    bot_ = 0;
  }
}
 int ByteModel::epoch() {return lstm_.ep();}
  int ByteModel::expected() {return ex;}
}