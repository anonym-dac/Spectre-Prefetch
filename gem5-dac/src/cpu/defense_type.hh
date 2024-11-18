
#ifndef __CPU_POS_HH__
#define __CPU_POS_HH__

namespace gem5
{

/// Pointer to a statically allocated generic "nop" instruction object.
extern bool _isUnsafePrefetch;
extern bool _isIngoreSpecPrefetch;
extern bool _isConfusionPrefetch;
} // namespace gem5

#endif // __CPU_NOP_STATIC_INST_HH__
