#ifndef SHARED_STORAGE_EXPRESSION_MODEL_H
#define SHARED_STORAGE_EXPRESSION_MODEL_H

#include <poincare/context.h>
#include <poincare/expression.h>
#include <poincare/layout.h>
#include <ion/storage.h>

namespace Shared {

class StorageExpressionModel {
  // TODO find better name (once we remove ExpressionModel?)
public:
  StorageExpressionModel(Ion::Storage::Record record);
  void destroy();
  void text(char * buffer, size_t bufferSize) const;
  Poincare::Expression expression() const;
  Poincare::Expression reducedExpression(Poincare::Context * context) const;
  Poincare::Layout layout();
  /* TODO This comment will be true when Sequence inherits from this class
   * Here, isDefined is the exact contrary of isEmpty. However, for Sequence
   * inheriting from ExpressionModel, isEmpty and isDefined have not exactly
   * opposite meaning. For instance, u(n+1)=u(n) & u(0) = ... is not empty and
   * not defined. We thus have to keep both methods. */
  virtual bool isDefined();
  virtual bool isEmpty();
  virtual bool shouldBeClearedBeforeRemove() { return !isEmpty(); }
  virtual void setContent(const char * c);
  virtual void tidy();
private:
  Ion::Storage::Record m_record;
  mutable Poincare::Expression m_expression;
  mutable Poincare::Layout m_layout;
};

}

#endif