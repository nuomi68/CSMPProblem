//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSMPProblem.h"

#include "Assembly.h"
#include "AuxiliarySystem.h"
#include "MooseEigenSystem.h"
#include "NonlinearSystem.h"
#include "LineSearch.h"
#include "MooseEnum.h"

registerMooseObject("MooseApp", CSMPProblem);

InputParameters
CSMPProblem::validParams()
{
  InputParameters params = FEProblem::validParams();
  params.addClassDescription("A normal (default) Problem object that contains a single "
                             "NonlinearSystem and a single AuxiliarySystem object.");
  params.addParam<Real>("reference_time", -1e10,"By comparing the time, do not change the PC at the same time");

  return params;
}

CSMPProblem::CSMPProblem(const InputParameters & parameters)
  : FEProblem(parameters),
  _reference_time(getParam<Real>("reference_time"))
{
}git push -u origin main

void
CSMPProblem::computeResidualTags(const std::set<TagID> & tags)
{
  FEProblem::computeResidualTags(tags);
  /* <do your postprocessor scaling here> */
  if (_reference_time!=time())
  {
    NonlinearSystemBase & nl = getNonlinearSystemBase();
    unsigned int n_vars = nl.nVariables();
    const auto & libmesh_system = nl.system();
    auto cm = std::make_unique<CouplingMatrix>(n_vars);
    if (time()>999)
    {
      for (unsigned int i = 0; i < n_vars; ++i)
        (*cm)(i, i) =  1;
        mooseWarning(time(),"false");
    }
    else
    {
      for (unsigned int i = 0; i < n_vars; ++i)
        for (unsigned int j = 0; j < n_vars; ++j)
          (*cm)(i, j) = 1;
          mooseWarning(time(),"true");
    }

    for (const auto i : make_range(numNonlinearSystems()))
    {
      if (i == libMesh::cast_int<unsigned int>(numNonlinearSystems() - 1))
        setCouplingMatrix(std::move(cm), i);
      else
        setCouplingMatrix(std::make_unique<CouplingMatrix>(*cm), i);
    }
    _reference_time=time();
  }
}

void
CSMPProblem::computeJacobianTags(const std::set<TagID> & tags)
{
  FEProblem::computeJacobianTags(tags);
  // if (_reference_time!=time())
  // {
  //   NonlinearSystemBase & nl = getNonlinearSystemBase();
  //   unsigned int n_vars = nl.nVariables();
  //   const auto & libmesh_system = nl.system();
  //   auto cm = std::make_unique<CouplingMatrix>(n_vars);
  //   if (time()>999)
  //   {
  //     // put 1s on diagonal
  //     for (unsigned int i = 0; i < n_vars; ++i)
  //       (*cm)(i, i) =  1;
  //       mooseWarning(time(),"false");
  //   }
  //   else
  //   {
  //     for (unsigned int i = 0; i < n_vars; ++i)
  //       for (unsigned int j = 0; j < n_vars; ++j)
  //         (*cm)(i, j) = 1;
  //         mooseWarning(time(),"true");
  //   }

  //   for (const auto i : make_range(numNonlinearSystems()))
  //   {
  //     if (i == libMesh::cast_int<unsigned int>(numNonlinearSystems() - 1))
  //       setCouplingMatrix(std::move(cm), i);
  //     else
  //       setCouplingMatrix(std::make_unique<CouplingMatrix>(*cm), i);
  //   }
  //   _reference_time=time();
  // }

}
