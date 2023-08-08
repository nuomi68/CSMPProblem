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
#include "DisplacedProblem.h"

#include "libmesh/system.h"
#include "libmesh/default_coupling.h"
#include "libmesh/dof_map.h"
#include "libmesh/equation_systems.h"
#include "libmesh/coupling_matrix.h"

registerMooseObject("MooseApp", CSMPProblem);

InputParameters
CSMPProblem::validParams()
{
  InputParameters params = FEProblem::validParams();
  params.addClassDescription("A normal (default) Problem object that contains a single "
                             "NonlinearSystem and a single AuxiliarySystem object.");
  params.addParam<Real>(
      "reference_time", -1e10, "By comparing the time, do not change the PC at the same time");

  return params;
}

CSMPProblem::CSMPProblem(const InputParameters & parameters)
  : FEProblem(parameters), _reference_time(getParam<Real>("reference_time"))
{
}

void
CSMPProblem::timestepSetup()
{
  FEProblem::timestepSetup();

  NonlinearSystemBase & nl = getNonlinearSystemBase();
  unsigned int n_vars = nl.nVariables();
  // const auto & libmesh_system = nl.system();
  auto local_cm = std::make_unique<CouplingMatrix>(n_vars);
  if (time() > 999)
  {
    for (unsigned int i = 0; i < n_vars; ++i)
      (*local_cm)(i, i) = 1;
    mooseWarning(time(), " false");
  }
  else
  {
    for (unsigned int i = 0; i < n_vars; ++i)
      for (unsigned int j = 0; j < n_vars; ++j)
        (*local_cm)(i, j) = 1;
    mooseWarning(time(), " true");
  }

  for (const auto i : make_range(numNonlinearSystems()))
  {
    if (i == libMesh::cast_int<unsigned int>(numNonlinearSystems() - 1))
      // This is the last nonlinear system, so it's safe now to move the object
      setCouplingMatrix(std::move(local_cm), i);
    else
      setCouplingMatrix(std::make_unique<CouplingMatrix>(*local_cm), i);

    auto & nl = getNonlinearSystemBase(i);
    // Get the 'permanent' coupling matrix for this nonlinear system
    auto * const cm = couplingMatrix(i);

    auto & dof_map = nl.dofMap();
    dof_map._dof_coupling = const_cast<CouplingMatrix *>(cm);

    // If there are no variables, make sure to pass a nullptr coupling
    // matrix, to avoid warnings about non-nullptr yet empty
    // CouplingMatrices.
    if (n_vars == 0)
      nl.dofMap()._dof_coupling = nullptr;
  }

  // Re-compute sparsity patterns. We can't really do this in a more inexpensive/granular fashion.
  // We need to ensure that the new coupling matrices get propagated to all the coupling functors
  // and this will be done in GhostingFunctor::dofmap_reinit which is called from
  // DofMap::distribute_dofs
  _eq.reinit();

  for (const auto i : make_range(numNonlinearSystems()))
  {
    auto * const cm = couplingMatrix(i);
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
    {
      assembly(tid, i).init(cm);
      if (_displaced_problem)
        _displaced_problem->assembly(tid, i).init(cm);
    }
  }
}
