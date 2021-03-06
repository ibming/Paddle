/* Copyright (c) 2016 PaddlePaddle Authors. All Rights Reserve.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#include "paddle/operators/auc_op.h"

namespace paddle {
namespace operators {

class AucOp : public framework::OperatorWithKernel {
 public:
  using framework::OperatorWithKernel::OperatorWithKernel;

 protected:
  void InferShape(framework::InferShapeContext *ctx) const override {
    PADDLE_ENFORCE(ctx->HasInput("Out"), "Input of Out must be initialized.");
    PADDLE_ENFORCE(ctx->HasInput("Indices"),
                   "Input of Indices must be initialized.");
    PADDLE_ENFORCE(ctx->HasInput("Label"),
                   "Input of Label must be initialized.");
    auto inference_height = ctx->GetInputDim("Out")[0];
    auto label_height = ctx->GetInputDim("Label")[0];

    PADDLE_ENFORCE_EQ(inference_height, label_height,
                      "Out and Label should have same height.");

    ctx->SetOutputDim("AUC", {1});
    ctx->ShareLoD("Out", /*->*/ "AUC");
  }

 protected:
  // IndicateDataType
  framework::DataType IndicateDataType(
      const framework::ExecutionContext &ctx) const override {
    return framework::ToDataType(ctx.Input<Tensor>("Out")->type());
  }
};

class AucOpMaker : public framework::OpProtoAndCheckerMaker {
 public:
  AucOpMaker(framework::OpProto *proto, framework::OpAttrChecker *op_checker)
      : OpProtoAndCheckerMaker(proto, op_checker) {
    AddInput("Out",
             "A floating point 2D tensor, values are in the range [0, 1]."
             "Each row is descend sorted. This input should be the"
             "output of topk."
             "Typically, this tensor indicates the probability of each label");
    AddInput("Indices",
             "An int 2D tensor, indicating the indices of original"
             "tensor before sort. Typically, this tensor indicates which label"
             "the probability stands for.");
    AddInput("Label",
             "A 2D int tensor indicating the label of the training data."
             "The height is batch size and width is always 1.");
    // TODO(typhoonzero): support weight input
    AddOutput("AUC",
              "A scalar representing the "
              "current area-under-curve.");

    AddAttr<std::string>("curve", "Curve type, can be 'ROC' or 'PR'.")
        .SetDefault("ROC");
    AddAttr<int>("num_thresholds",
                 "The number of thresholds to use when discretizing the"
                 " roc curve.")
        .SetDefault(200);

    AddComment(
        R"DOC(Computes the AUC according forward output and label.
Best to use for binary classification evaluations.

If input label contains values other than 0 and 1, it will be cast
to bool.

You can find the definations here: 
https://en.wikipedia.org/wiki/Receiver_operating_characteristic#Area_under_the_curve

Possible curves are:
- ROC: Receiver operating characteristic
- PR: Precision Recall
)DOC");
  }
};

}  // namespace operators
}  // namespace paddle

namespace ops = paddle::operators;
REGISTER_OP_WITHOUT_GRADIENT(auc, ops::AucOp, ops::AucOpMaker);
REGISTER_OP_CPU_KERNEL(auc, ops::AucKernel<paddle::platform::CPUPlace, float>);
