#include <poincare/uninitialized_evaluation_node.h>

namespace Poincare {

template<typename T>
UninitializedEvaluationNode<T> * UninitializedEvaluationNode<T>::UninitializedEvaluationStaticNode() {
  static UninitializedEvaluationNode<T> exception;
  TreePool::sharedPool()->registerStaticNodeIfRequired(&exception);
  return &exception;
}

template UninitializedEvaluationNode<float> * UninitializedEvaluationNode<float>::UninitializedEvaluationStaticNode();
template UninitializedEvaluationNode<double> * UninitializedEvaluationNode<double>::UninitializedEvaluationStaticNode();

}