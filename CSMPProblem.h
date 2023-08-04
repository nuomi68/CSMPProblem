//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FEProblem.h"
#include "MoosePreconditioner.h"

/**
 * Specialization of SubProblem for solving nonlinear equations plus auxiliary equations
 *
 */
class CSMPProblem : public FEProblem,public MoosePreconditioner
{
public:
  static InputParameters validParams();

  CSMPProblem(const InputParameters & parameters);

void computeResidualTags(const std::set<TagID> & tags);
void computeJacobianTags(const std::set<TagID> & tags);

 Real _reference_time;

};
