#ifndef REGRESSION_LINEAR_MODEL_H
#define REGRESSION_LINEAR_MODEL_H

#include "model.h"
#include <poincare/expression_layout.h>

namespace Regression {

class LinearModel : public Model {
public:
  using Model::Model;
  Poincare::ExpressionLayout * layout() override;
  I18n::Message formulaMessage() const override { return I18n::Message::LinearRegressionFormula; }
  double evaluate(double * modelCoefficients, double x) const override;
  double levelSet(double * modelCoefficients, double y) const override;
  virtual void fit(Store * store, int series, double * modelCoefficients, Poincare::Context * context) override;
  double partialDerivate(double * modelCoefficients, int derivateCoefficientIndex, double x) const override;
  int numberOfCoefficients() const override { return 2; }
};

}


#endif