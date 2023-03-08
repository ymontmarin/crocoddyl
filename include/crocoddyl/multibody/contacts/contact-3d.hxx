///////////////////////////////////////////////////////////////////////////////
// BSD 3-Clause License
//
// Copyright (C) 2019-2023, LAAS-CNRS, University of Edinburgh,
//                          Heriot-Watt University
// Copyright note valid unless otherwise stated in individual files.
// All rights reserved.
///////////////////////////////////////////////////////////////////////////////

namespace crocoddyl {

template <typename Scalar>
ContactModel3DTpl<Scalar>::ContactModel3DTpl(boost::shared_ptr<StateMultibody> state, const pinocchio::FrameIndex id,
                                             const Vector3s& xref, const pinocchio::ReferenceFrame type,
                                             const std::size_t nu, const Vector2s& gains)
    : Base(state, 3, type, nu), xref_(xref), gains_(gains) {
  id_ = id;
}

template <typename Scalar>
ContactModel3DTpl<Scalar>::ContactModel3DTpl(boost::shared_ptr<StateMultibody> state, const pinocchio::FrameIndex id,
                                             const Vector3s& xref, const pinocchio::ReferenceFrame type,
                                             const Vector2s& gains)
    : Base(state, 3, type), xref_(xref), gains_(gains) {
  id_ = id;
}

#pragma GCC diagnostic push  // TODO: Remove once the deprecated FrameXX has been removed in a future release
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

template <typename Scalar>
ContactModel3DTpl<Scalar>::ContactModel3DTpl(boost::shared_ptr<StateMultibody> state, const pinocchio::FrameIndex id,
                                             const Vector3s& xref, const std::size_t nu, const Vector2s& gains)
    : Base(state, 3, pinocchio::ReferenceFrame::LOCAL, nu), xref_(xref), gains_(gains) {
  id_ = id;
  std::cerr << "Deprecated: Use constructor that passes the type of contact, this assumes is pinocchio::LOCAL."
            << std::endl;
}

template <typename Scalar>
ContactModel3DTpl<Scalar>::ContactModel3DTpl(boost::shared_ptr<StateMultibody> state, const pinocchio::FrameIndex id,
                                             const Vector3s& xref, const Vector2s& gains)
    : Base(state, 3, pinocchio::ReferenceFrame::LOCAL), xref_(xref), gains_(gains) {
  id_ = id;
  std::cerr << "Deprecated: Use constructor that passes the type of contact, this assumes is pinocchio::LOCAL."
            << std::endl;
}

#pragma GCC diagnostic pop

template <typename Scalar>
ContactModel3DTpl<Scalar>::~ContactModel3DTpl() {}

template <typename Scalar>
void ContactModel3DTpl<Scalar>::calc(const boost::shared_ptr<ContactDataAbstract>& data,
                                     const Eigen::Ref<const VectorXs>&) {
  Data* d = static_cast<Data*>(data.get());
  pinocchio::updateFramePlacement(*state_->get_pinocchio().get(), *d->pinocchio, id_);
  pinocchio::getFrameJacobian(*state_->get_pinocchio().get(), *d->pinocchio, id_, pinocchio::LOCAL, d->fJf);
  d->v = pinocchio::getFrameVelocity(*state_->get_pinocchio().get(), *d->pinocchio, id_);
  d->a0_local =
      pinocchio::getFrameClassicalAcceleration(*state_->get_pinocchio().get(), *d->pinocchio, id_, pinocchio::LOCAL)
          .linear();

  d->vw = d->v.angular();
  d->vv = d->v.linear();
  pinocchio::SE3::ConstAngularRef oRf = d->pinocchio->oMf[id_].rotation();
  d->dp = d->pinocchio->oMf[id_].translation() - xref_;
  d->dp_local.noalias() = oRf.transpose() * d->dp;

  if (gains_[0] != 0.) {
    d->a0_local += gains_[0] * d->dp_local;
  }
  if (gains_[1] != 0.) {
    d->a0_local += gains_[1] * d->vv;
  }
  switch (type_) {
    case pinocchio::ReferenceFrame::LOCAL:
      d->Jc = d->fJf.template topRows<3>();
      d->a0 = d->a0_local;
      break;
    case pinocchio::ReferenceFrame::WORLD:
    case pinocchio::ReferenceFrame::LOCAL_WORLD_ALIGNED:
      d->Jc.noalias() = oRf * d->fJf.template topRows<3>();
      d->a0.noalias() = oRf * d->a0_local;
      break;
  }
}

template <typename Scalar>
void ContactModel3DTpl<Scalar>::calcDiff(const boost::shared_ptr<ContactDataAbstract>& data,
                                         const Eigen::Ref<const VectorXs>&) {
  Data* d = static_cast<Data*>(data.get());
  const pinocchio::JointIndex joint = state_->get_pinocchio()->frames[d->frame].parent;
  pinocchio::getJointAccelerationDerivatives(*state_->get_pinocchio().get(), *d->pinocchio, joint, pinocchio::LOCAL,
                                             d->v_partial_dq, d->a_partial_dq, d->a_partial_dv, d->a_partial_da);
  const std::size_t nv = state_->get_nv();
  pinocchio::skew(d->vv, d->vv_skew);
  pinocchio::skew(d->vw, d->vw_skew);
  pinocchio::skew(d->dp_local, d->dp_skew);
  d->fXjdv_dq.noalias() = d->fXj * d->v_partial_dq;
  d->fXjda_dq.noalias() = d->fXj * d->a_partial_dq;
  d->fXjda_dv.noalias() = d->fXj * d->a_partial_dv;
  d->da0_local_dx.leftCols(nv) = d->fXjda_dq.template topRows<3>();
  d->da0_local_dx.leftCols(nv).noalias() += d->vw_skew * d->fXjdv_dq.template topRows<3>();
  d->da0_local_dx.leftCols(nv).noalias() -= d->vv_skew * d->fXjdv_dq.template bottomRows<3>();
  d->da0_local_dx.rightCols(nv) = d->fXjda_dv.template topRows<3>();
  d->da0_local_dx.rightCols(nv).noalias() += d->vw_skew * d->fJf.template topRows<3>();
  d->da0_local_dx.rightCols(nv).noalias() -= d->vv_skew * d->fJf.template bottomRows<3>();
  pinocchio::SE3::ConstAngularRef oRf = d->pinocchio->oMf[id_].rotation();

  if (gains_[0] != 0.) {
    d->da0_local_dx.leftCols(nv).noalias() += gains_[0] * d->dp_skew * d->fJf.template bottomRows<3>();
    d->da0_local_dx.leftCols(nv).noalias() += gains_[0] * d->fJf.template topRows<3>();
  }
  if (gains_[1] != 0.) {
    d->da0_local_dx.leftCols(nv).noalias() += gains_[1] * d->fXjdv_dq.template topRows<3>();
    d->da0_local_dx.rightCols(nv).noalias() += gains_[1] * d->fJf.template topRows<3>();
  }
  switch (type_) {
    case pinocchio::ReferenceFrame::LOCAL:
      d->da0_dx = d->da0_local_dx;
      break;
    case pinocchio::ReferenceFrame::WORLD:
    case pinocchio::ReferenceFrame::LOCAL_WORLD_ALIGNED:
      pinocchio::skew(d->a0.template head<3>(), d->a0_skew);
      d->a0_world_skew.noalias() = d->a0_skew * oRf;
      d->da0_dx.noalias() = oRf * d->da0_local_dx;
      d->da0_dx.leftCols(nv).noalias() -= d->a0_world_skew * d->fJf.template bottomRows<3>();
      break;
  }
}

template <typename Scalar>
void ContactModel3DTpl<Scalar>::updateForce(const boost::shared_ptr<ContactDataAbstract>& data,
                                            const VectorXs& force) {
  if (force.size() != 3) {
    throw_pretty("Invalid argument: "
                 << "lambda has wrong dimension (it should be 3)");
  }
  Data* d = static_cast<Data*>(data.get());
  switch (type_) {
    case pinocchio::ReferenceFrame::LOCAL:
      data->f = d->jMf.act(pinocchio::ForceTpl<Scalar>(force, Vector3s::Zero()));
      data->dtau_dq.setZero();
      break;
    case pinocchio::ReferenceFrame::WORLD:
    case pinocchio::ReferenceFrame::LOCAL_WORLD_ALIGNED:
      pinocchio::SE3::ConstAngularRef oRf = d->pinocchio->oMf[id_].rotation();
      d->f_world.noalias() = oRf.transpose() * force;
      data->f = d->jMf.act(pinocchio::ForceTpl<Scalar>(d->f_world, Vector3s::Zero()));
      pinocchio::skew(d->f_world, d->f_skew);
      d->fJf_df.noalias() = d->f_skew * d->fJf.template bottomRows<3>();
      data->dtau_dq.noalias() = -d->fJf.template topRows<3>().transpose() * d->fJf_df;
      break;
  }
}

template <typename Scalar>
boost::shared_ptr<ContactDataAbstractTpl<Scalar> > ContactModel3DTpl<Scalar>::createData(
    pinocchio::DataTpl<Scalar>* const data) {
  return boost::allocate_shared<Data>(Eigen::aligned_allocator<Data>(), this, data);
}

template <typename Scalar>
void ContactModel3DTpl<Scalar>::print(std::ostream& os) const {
  os << "ContactModel3D {frame=" << state_->get_pinocchio()->frames[id_].name << ", type=" << type_ << "}";
}

template <typename Scalar>
const typename MathBaseTpl<Scalar>::Vector3s& ContactModel3DTpl<Scalar>::get_reference() const {
  return xref_;
}

template <typename Scalar>
const typename MathBaseTpl<Scalar>::Vector2s& ContactModel3DTpl<Scalar>::get_gains() const {
  return gains_;
}

template <typename Scalar>
void ContactModel3DTpl<Scalar>::set_reference(const Vector3s& reference) {
  xref_ = reference;
}

}  // namespace crocoddyl
