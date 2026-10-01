#ifndef NUMSIM_CORE_NAMESPACE_H
#define NUMSIM_CORE_NAMESPACE_H

// The library lives in numsim::core, like numsim::materials, numsim::fft and
// numsim::homogenization. numsim_core stays available as an alias, so code
// written against the old name keeps compiling unchanged.
namespace numsim::core {}
namespace numsim_core = numsim::core;

#endif // NUMSIM_CORE_NAMESPACE_H
