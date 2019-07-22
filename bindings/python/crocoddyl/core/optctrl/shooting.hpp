///////////////////////////////////////////////////////////////////////////////
// BSD 3-Clause License
//
// Copyright (C) 2018-2019, LAAS-CNRS
// Copyright note valid unless otherwise stated in individual files.
// All rights reserved.
///////////////////////////////////////////////////////////////////////////////

#ifndef PYTHON_CROCODDYL_CORE_OPTCTRL_SHOOTING_HPP_
#define PYTHON_CROCODDYL_CORE_OPTCTRL_SHOOTING_HPP_

#include "crocoddyl/core/optctrl/shooting.hpp"
#include "python/crocoddyl/utils.hpp"

namespace crocoddyl {
namespace python {

namespace bp = boost::python;

class ShootingProblem_wrap : public ShootingProblem, public bp::wrapper<ShootingProblem> {
 public:
  using ShootingProblem::running_datas_;
  using ShootingProblem::running_models_;
  using ShootingProblem::T_;
  using ShootingProblem::terminal_data_;
  using ShootingProblem::x0_;

  ShootingProblem_wrap(const Eigen::VectorXd& x0, std::vector<ActionModelAbstract*> running_models,
                       ActionModelAbstract* terminal_model)
      : ShootingProblem(x0, running_models, terminal_model) {}
};

void exposeShootingProblem() {
  // Register custom converters between std::vector and Python list
  typedef ActionModelAbstract* ActionModelPtr;
  typedef boost::shared_ptr<ActionDataAbstract> ActionDataPtr;
  bp::to_python_converter<std::vector<ActionModelPtr, std::allocator<ActionModelPtr> >,
                          vector_to_list<ActionModelPtr> >();
  bp::to_python_converter<std::vector<ActionDataPtr, std::allocator<ActionDataPtr> >,
                          vector_to_list<ActionDataPtr> >();
  list_to_vector().from_python<std::vector<ActionModelPtr, std::allocator<ActionModelPtr> > >();

  bp::class_<ShootingProblem_wrap, boost::noncopyable>(
      "ShootingProblem",
      "Declare a shooting problem.\n\n"
      "A shooting problem declares the initial state, a set of running action models and a\n"
      "terminal action model. It has two main methods - calc, calcDiff and rollout. The\n"
      "first computes the set of next states and cost values per each action model. calcDiff\n"
      "updates the derivatives of all action models. The last rollouts the stacks of actions\n"
      "models.",
      bp::init<Eigen::VectorXd, std::vector<ActionModelAbstract*>, ActionModelAbstract*>(
          bp::args(" self", " initialState", " runningModels", " terminalModel"),
          "Initialize the shooting problem.\n\n"
          ":param initialState: initial state\n"
          ":param runningModels: running action models\n"
          ":param terminalModel: terminal action model"))
      .def("calc", &ShootingProblem_wrap::calc, bp::args(" self", " xs", " us"),
           "Compute the cost and the next states.\n\n"
           "First, it computes the next state and cost for each action model\n"
           "along a state and control trajectory.\n"
           ":param xs: time-discrete state trajectory\n"
           ":param us: time-discrete control sequence\n"
           ":returns the total cost value")
      .def("calcDiff", &ShootingProblem_wrap::calcDiff, bp::args(" self", " xs", " us"),
           "Compute the cost-and-dynamics derivatives.\n\n"
           "These quantities are computed along a given pair of trajectories xs\n"
           "(states) and us (controls).\n"
           ":param xs: time-discrete state trajectory\n"
           ":param us: time-discrete control sequence")
      .def("rollout", &ShootingProblem_wrap::rollout_us, bp::args(" self", " us"),
           "Integrate the dynamics given a control sequence.\n\n"
           "Rollout the dynamics give a sequence of control commands\n"
           ":param us: time-discrete control sequence")
      .add_property("T", &ShootingProblem_wrap::T_, "number of nodes")
      .add_property("initialState",
                    bp::make_getter(&ShootingProblem_wrap::x0_, bp::return_value_policy<bp::return_by_value>()),
                    "initial state")
      .add_property(
          "runningModels",
          bp::make_getter(&ShootingProblem_wrap::running_models_, bp::return_value_policy<bp::return_by_value>()),
          bp::make_setter(&ShootingProblem_wrap::running_models_, bp::return_value_policy<bp::return_by_value>()),
          "running models")
      .add_property("terminalModel",
                    bp::make_function(&ShootingProblem_wrap::get_terminalModel, bp::return_internal_reference<>()),
                    "terminal model")
      .add_property(
          "runningDatas",
          bp::make_getter(&ShootingProblem_wrap::running_datas_, bp::return_value_policy<bp::return_by_value>()),
          "running datas")
      .add_property(
          "terminalData",
          bp::make_getter(&ShootingProblem_wrap::terminal_data_, bp::return_value_policy<bp::return_by_value>()),
          "terminal data");
}

}  // namespace python
}  // namespace crocoddyl

#endif  // PYTHON_CROCODDYL_CORE_OPTCTRL_SHOOTING_HPP_