///////////////////////////////////////////////////////////////////////////////
// BSD 3-Clause License
//
// Copyright (C) 2021, LAAS-CNRS, University of Edinburgh, University of Trento
// Copyright note valid unless otherwise stated in individual files.
// All rights reserved.
///////////////////////////////////////////////////////////////////////////////

#include "python/crocoddyl/core/core.hpp"
#include "python/crocoddyl/core/control-base.hpp"
#include "crocoddyl/core/controls/poly-one.hpp"

namespace crocoddyl {
namespace python {

void exposeControlPolyOne() {
  bp::register_ptr_to_python<boost::shared_ptr<ControlPolyOne> >();

  bp::class_<ControlPolyOne, bp::bases<ControlAbstract> >(
      "ControlPolyOne",
      "Constant control.\n\n"
      "This control is a line function of time (normalized in [0,1])."
      "The first half of the parameter vector contains the initial value of u, "
      "whereas the second half contains the value of u at t=0.5.",
      bp::init<int>(bp::args("self", "nu"),
                         "Initialize the control dimensions.\n\n"
                         ":param nu: dimension of control space\n"))
      .def("value", &ControlPolyOne::value_u, bp::args("self", "t", "p"),
           "Compute the control value.\n\n"
           ":param t: normalized time in [0, 1].\n"
           ":param p: control parameters (dim control.np).\n"
           ":return u value (dim control.nu).")
      .def("value_inv", &ControlPolyOne::value_inv_p, bp::args("self", "t", "u"),
           "Compute the control value.\n\n"
           ":param t: normalized time in [0, 1].\n"
           ":param u: control value (dim control.nu).\n"
           ":return p value (dim control.np).")
      .def("convert_bounds", &ControlPolyOne::convert_bounds, bp::args("self", "u_lb", "u_ub"),
           "Convert the bounds on the control to bounds on the control parameters.\n\n"
           ":param u_lb: lower bounds on u (dim control.nu).\n"
           ":param u_ub: upper bounds on u (dim control.nu).\n"
           ":return p_lb, p_ub: lower and upper bounds on the control parameters (dim control.np).")
      .def("dValue", &ControlPolyOne::dValue_J, bp::args("self", "t", "p"),
           "Compute the derivative of the control with respect to the parameters.\n\n"
           ":param t: normalized time in [0, 1].\n"
           ":param p: control parameters (dim control.np).\n"
           ":return Partial derivative of the value function (dim control.nu x control.np).")
      .def("multiplyByDValue", &ControlPolyOne::multiplyByDValue_J, bp::args("self", "t", "p", "A"),
           "Compute the product between the given matrix A and the derivative of the control with respect to the parameters.\n\n"
           ":param t: normalized time in [0, 1].\n"
           ":param p: control parameters (dim control.np).\n"
           ":param A: matrix to multiply (dim na x control.nu).\n"
           ":return Product between A and the partial derivative of the value function (dim na x control.np).")
      .def("multiplyDValueTransposeBy", &ControlPolyOne::multiplyDValueTransposeBy_J, bp::args("self", "t", "p", "A"),
           "Compute the product between the transpose of the derivative of the control with respect to the parameters\n"
           "and a given matrix A.\n\n"
           ":param t: normalized time in [0, 1].\n"
           ":param p: control parameters (dim control.np).\n"
           ":param A: matrix to multiply (dim control.nu x na).\n"
           ":return Product between the partial derivative of the value function (transposed) and A (dim control.np x na).")
      .add_property("nu", bp::make_function(&ControlPolyOne::get_nu), "dimension of control tuple")
      .add_property("np", bp::make_function(&ControlPolyOne::get_np),
                    "dimension of the control parameters");
}

}  // namespace python
}  // namespace crocoddyl
