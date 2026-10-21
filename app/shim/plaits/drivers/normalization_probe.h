#ifndef PLAITS_DRIVERS_NORMALIZATION_PROBE_H_
#define PLAITS_DRIVERS_NORMALIZATION_PROBE_H_
#include "stmlib/stmlib.h"
namespace plaits {
class NormalizationProbe {
 public:
  NormalizationProbe() { }
  ~NormalizationProbe() { }
  static inline void Init() { }
  void Disable() { }
  static inline void High() { }
  static inline void Low() { }
  static inline void Write(bool value) { (void)value; }
 private:
  DISALLOW_COPY_AND_ASSIGN(NormalizationProbe);
};
}  // namespace plaits
#endif
