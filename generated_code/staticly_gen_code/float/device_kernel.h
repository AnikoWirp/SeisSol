#ifndef DEVICE_GEN_CODE_DEVICE_KERNEL_H_
#define DEVICE_GEN_CODE_DEVICE_KERNEL_H_
#include <cmath>
#include <limits>
#include "tensor.h"
#include <stdlib.h>
using namespace seissol;
namespace device_gen_code {
  namespace kernel {
    struct volume {
      constexpr static unsigned long const NonZeroFlops = 34839;
      constexpr static unsigned long const HardwareFlops = 125280;

      float const** I{};
      float** Q{};
      tensor::kDivM::Container<float const*> kDivM;
      tensor::star::Container<float const**> star;

      unsigned I_offset{};
      unsigned Q_offset{};
      tensor::kDivM::Container<unsigned> kDivM_offset;
      tensor::star::Container<unsigned> star_offset;

      size_t num_elements{};

      void execute();
    };
  }
  namespace kernel {
    struct rotateGodunovStateLocal {
      constexpr static unsigned long const NonZeroFlops = 1377;
      constexpr static unsigned long const HardwareFlops = 1458;

      float const** QgodLocal{};
      float const** Tinv{};
      float** godunovMatrix{};

      unsigned QgodLocal_offset{};
      unsigned Tinv_offset{};
      unsigned godunovMatrix_offset{};

      size_t num_elements{};

      void execute();
    };
  }
  namespace kernel {
    struct rotateGodunovStateNeighbor {
      constexpr static unsigned long const NonZeroFlops = 1377;
      constexpr static unsigned long const HardwareFlops = 1458;

      float const** QgodNeighbor{};
      float const** Tinv{};
      float** godunovMatrix{};

      unsigned QgodNeighbor_offset{};
      unsigned Tinv_offset{};
      unsigned godunovMatrix_offset{};

      size_t num_elements{};

      void execute();
    };
  }
  namespace kernel {
    struct rotateFluxMatrix {
      constexpr static unsigned long const NonZeroFlops = 432;
      constexpr static unsigned long const HardwareFlops = 1458;

      float fluxScale = std::numeric_limits<float>::signaling_NaN();
      float const** T{};
      float** fluxSolver{};
      tensor::star::Container<float const**> star;

      unsigned T_offset{};
      unsigned fluxSolver_offset{};
      tensor::star::Container<unsigned> star_offset;

      size_t num_elements{};

      void execute();
    };
  }
  namespace kernel {
    struct localFlux {
      constexpr static unsigned long const NonZeroFlops[] = {9936, 10080, 31968, 27216};
      constexpr static unsigned long const HardwareFlops[] = {49248, 49248, 49248, 49248};

      float const** AplusT{};
      float const** I{};
      float** Q{};
      tensor::fMrT::Container<float const*> fMrT;
      tensor::rDivM::Container<float const*> rDivM;

      unsigned AplusT_offset{};
      unsigned I_offset{};
      unsigned Q_offset{};
      tensor::fMrT::Container<unsigned> fMrT_offset;
      tensor::rDivM::Container<unsigned> rDivM_offset;

      size_t num_elements{};

      struct Prefetch {
        float const** I{};
        float const** Q{};
      };
      Prefetch _prefetch;

      void execute0();
      void execute1();
      void execute2();
      void execute3();
      typedef void (localFlux::* const member_function_ptr)(void);
      constexpr static member_function_ptr ExecutePtrs[] = {&localFlux::execute0, &localFlux::execute1, &localFlux::execute2, &localFlux::execute3};
      constexpr static member_function_ptr findExecute(unsigned i0) {
        return ExecutePtrs[1*i0];
      }
      inline void execute(unsigned i0) {
        (this->*findExecute(i0))();
      }
      constexpr static unsigned long nonZeroFlops(unsigned i0) {
        return NonZeroFlops[1*i0];
      }
      constexpr static unsigned long hardwareFlops(unsigned i0) {
        return HardwareFlops[1*i0];
      }
    };
  }
  namespace kernel {
    struct neighboringFlux {
      constexpr static unsigned long const NonZeroFlops[] = {11349, 10125, 11349, 11421, 10197, 11421, 22365, 21141, 22365, 19989, 18765, 19989, 11421, 10197, 11421, 11493, 10269, 11493, 22437, 21213, 22437, 20061, 18837, 20061, 22365, 21141, 22365, 22437, 21213, 22437, 33381, 32157, 33381, 31005, 29781, 31005, 19989, 18765, 19989, 20061, 18837, 20061, 31005, 29781, 31005, 28629, 27405, 28629};
      constexpr static unsigned long const HardwareFlops[] = {58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320, 58320};

      float const** AminusT{};
      float const** I{};
      float** Q{};
      tensor::fP::Container<float const*> fP;
      tensor::rDivM::Container<float const*> rDivM;
      tensor::rT::Container<float const*> rT;

      unsigned AminusT_offset{};
      unsigned I_offset{};
      unsigned Q_offset{};
      tensor::fP::Container<unsigned> fP_offset;
      tensor::rDivM::Container<unsigned> rDivM_offset;
      tensor::rT::Container<unsigned> rT_offset;

      size_t num_elements{};

      struct Prefetch {
        float const** I{};
      };
      Prefetch _prefetch;

      void execute0();
      void execute1();
      void execute2();
      void execute3();
      void execute4();
      void execute5();
      void execute6();
      void execute7();
      void execute8();
      void execute9();
      void execute10();
      void execute11();
      void execute12();
      void execute13();
      void execute14();
      void execute15();
      void execute16();
      void execute17();
      void execute18();
      void execute19();
      void execute20();
      void execute21();
      void execute22();
      void execute23();
      void execute24();
      void execute25();
      void execute26();
      void execute27();
      void execute28();
      void execute29();
      void execute30();
      void execute31();
      void execute32();
      void execute33();
      void execute34();
      void execute35();
      void execute36();
      void execute37();
      void execute38();
      void execute39();
      void execute40();
      void execute41();
      void execute42();
      void execute43();
      void execute44();
      void execute45();
      void execute46();
      void execute47();
      typedef void (neighboringFlux::* const member_function_ptr)(void);
      constexpr static member_function_ptr ExecutePtrs[] = {&neighboringFlux::execute0, &neighboringFlux::execute1, &neighboringFlux::execute2, &neighboringFlux::execute3, &neighboringFlux::execute4, &neighboringFlux::execute5, &neighboringFlux::execute6, &neighboringFlux::execute7, &neighboringFlux::execute8, &neighboringFlux::execute9, &neighboringFlux::execute10, &neighboringFlux::execute11, &neighboringFlux::execute12, &neighboringFlux::execute13, &neighboringFlux::execute14, &neighboringFlux::execute15, &neighboringFlux::execute16, &neighboringFlux::execute17, &neighboringFlux::execute18, &neighboringFlux::execute19, &neighboringFlux::execute20, &neighboringFlux::execute21, &neighboringFlux::execute22, &neighboringFlux::execute23, &neighboringFlux::execute24, &neighboringFlux::execute25, &neighboringFlux::execute26, &neighboringFlux::execute27, &neighboringFlux::execute28, &neighboringFlux::execute29, &neighboringFlux::execute30, &neighboringFlux::execute31, &neighboringFlux::execute32, &neighboringFlux::execute33, &neighboringFlux::execute34, &neighboringFlux::execute35, &neighboringFlux::execute36, &neighboringFlux::execute37, &neighboringFlux::execute38, &neighboringFlux::execute39, &neighboringFlux::execute40, &neighboringFlux::execute41, &neighboringFlux::execute42, &neighboringFlux::execute43, &neighboringFlux::execute44, &neighboringFlux::execute45, &neighboringFlux::execute46, &neighboringFlux::execute47};
      constexpr static member_function_ptr findExecute(unsigned i0, unsigned i1, unsigned i2) {
        return ExecutePtrs[1*i0 + 3*i1 + 12*i2];
      }
      inline void execute(unsigned i0, unsigned i1, unsigned i2) {
        (this->*findExecute(i0, i1, i2))();
      }
      constexpr static unsigned long nonZeroFlops(unsigned i0, unsigned i1, unsigned i2) {
        return NonZeroFlops[1*i0 + 3*i1 + 12*i2];
      }
      constexpr static unsigned long hardwareFlops(unsigned i0, unsigned i1, unsigned i2) {
        return HardwareFlops[1*i0 + 3*i1 + 12*i2];
      }
    };
  }
  namespace kernel {
    struct derivativeTaylorExpansion {
      constexpr static unsigned long const NonZeroFlops[] = {504, 630, 360, 180, 72, 18};
      constexpr static unsigned long const HardwareFlops[] = {0, 0, 0, 0, 0, 0};

      float power = std::numeric_limits<float>::signaling_NaN();
      float** I{};
      tensor::dQ::Container<float const**> dQ;

      unsigned I_offset{};
      tensor::dQ::Container<unsigned> dQ_offset;

      size_t num_elements{};

      void execute0();
      void execute1();
      void execute2();
      void execute3();
      void execute4();
      void execute5();
      typedef void (derivativeTaylorExpansion::* const member_function_ptr)(void);
      constexpr static member_function_ptr ExecutePtrs[] = {&derivativeTaylorExpansion::execute0, &derivativeTaylorExpansion::execute1, &derivativeTaylorExpansion::execute2, &derivativeTaylorExpansion::execute3, &derivativeTaylorExpansion::execute4, &derivativeTaylorExpansion::execute5};
      constexpr static member_function_ptr findExecute(unsigned i0) {
        return ExecutePtrs[1*i0];
      }
      inline void execute(unsigned i0) {
        (this->*findExecute(i0))();
      }
      constexpr static unsigned long nonZeroFlops(unsigned i0) {
        return NonZeroFlops[1*i0];
      }
      constexpr static unsigned long hardwareFlops(unsigned i0) {
        return HardwareFlops[1*i0];
      }
    };
  }
  namespace kernel {
    struct derivative {
      constexpr static unsigned long const NonZeroFlops[] = {0, 34524, 13806, 4716, 1260, 216};
      constexpr static unsigned long const HardwareFlops[] = {0, 136080, 54432, 23328, 7344, 4752};

      tensor::dQ::Container<float**> dQ;
      tensor::kDivMT::Container<float const*> kDivMT;
      tensor::star::Container<float const**> star;

      tensor::dQ::Container<unsigned> dQ_offset;
      tensor::kDivMT::Container<unsigned> kDivMT_offset;
      tensor::star::Container<unsigned> star_offset;

      size_t num_elements{};

      void execute1();
      void execute2();
      void execute3();
      void execute4();
      void execute5();
      typedef void (derivative::* const member_function_ptr)(void);
      constexpr static member_function_ptr ExecutePtrs[] = {nullptr, &derivative::execute1, &derivative::execute2, &derivative::execute3, &derivative::execute4, &derivative::execute5};
      constexpr static member_function_ptr findExecute(unsigned i0) {
        return ExecutePtrs[1*i0];
      }
      inline void execute(unsigned i0) {
        (this->*findExecute(i0))();
      }
      constexpr static unsigned long nonZeroFlops(unsigned i0) {
        return NonZeroFlops[1*i0];
      }
      constexpr static unsigned long hardwareFlops(unsigned i0) {
        return HardwareFlops[1*i0];
      }
    };
  }
  namespace kernel {
    struct godunovState {
      constexpr static unsigned long const NonZeroFlops[] = {53676, 56448, 56448, 56448, 56889, 54117, 56889, 56889, 56889, 56889, 56889, 56889, 54117, 56889, 56889, 56889};
      constexpr static unsigned long const HardwareFlops[] = {65520, 65520, 65520, 65520, 65520, 65520, 65520, 65520, 65520, 65520, 65520, 65520, 65520, 65520, 65520, 65520};

      float const** Q{};
      tensor::V3mTo2n::Container<float const*> V3mTo2n;
      float const** godunovMatrix{};
      float** godunovState{};

      unsigned Q_offset{};
      tensor::V3mTo2n::Container<unsigned> V3mTo2n_offset;
      unsigned godunovMatrix_offset{};
      unsigned godunovState_offset{};

      size_t num_elements{};

      struct Prefetch {
        float const** godunovState{};
      };
      Prefetch _prefetch;

      void execute0();
      void execute1();
      void execute2();
      void execute3();
      void execute4();
      void execute5();
      void execute6();
      void execute7();
      void execute8();
      void execute9();
      void execute10();
      void execute11();
      void execute12();
      void execute13();
      void execute14();
      void execute15();
      typedef void (godunovState::* const member_function_ptr)(void);
      constexpr static member_function_ptr ExecutePtrs[] = {&godunovState::execute0, &godunovState::execute1, &godunovState::execute2, &godunovState::execute3, &godunovState::execute4, &godunovState::execute5, &godunovState::execute6, &godunovState::execute7, &godunovState::execute8, &godunovState::execute9, &godunovState::execute10, &godunovState::execute11, &godunovState::execute12, &godunovState::execute13, &godunovState::execute14, &godunovState::execute15};
      constexpr static member_function_ptr findExecute(unsigned i0, unsigned i1) {
        return ExecutePtrs[1*i0 + 4*i1];
      }
      inline void execute(unsigned i0, unsigned i1) {
        (this->*findExecute(i0, i1))();
      }
      constexpr static unsigned long nonZeroFlops(unsigned i0, unsigned i1) {
        return NonZeroFlops[1*i0 + 4*i1];
      }
      constexpr static unsigned long hardwareFlops(unsigned i0, unsigned i1) {
        return HardwareFlops[1*i0 + 4*i1];
      }
    };
  }
  namespace kernel {
    struct nodalFlux {
      constexpr static unsigned long const NonZeroFlops[] = {54117, 56889, 56889, 56889, 56889, 54117, 56889, 56889, 56889, 56889, 56889, 56889, 54117, 56889, 56889, 56889};
      constexpr static unsigned long const HardwareFlops[] = {58464, 58464, 58464, 58464, 58464, 58464, 58464, 58464, 58464, 58464, 58464, 58464, 58464, 58464, 58464, 58464};

      float** Q{};
      tensor::V3mTo2nTWDivM::Container<float const*> V3mTo2nTWDivM;
      float const** fluxSolver{};
      float const** godunovState{};

      unsigned Q_offset{};
      tensor::V3mTo2nTWDivM::Container<unsigned> V3mTo2nTWDivM_offset;
      unsigned fluxSolver_offset{};
      unsigned godunovState_offset{};

      size_t num_elements{};

      struct Prefetch {
        float const** I{};
      };
      Prefetch _prefetch;

      void execute0();
      void execute1();
      void execute2();
      void execute3();
      void execute4();
      void execute5();
      void execute6();
      void execute7();
      void execute8();
      void execute9();
      void execute10();
      void execute11();
      void execute12();
      void execute13();
      void execute14();
      void execute15();
      typedef void (nodalFlux::* const member_function_ptr)(void);
      constexpr static member_function_ptr ExecutePtrs[] = {&nodalFlux::execute0, &nodalFlux::execute1, &nodalFlux::execute2, &nodalFlux::execute3, &nodalFlux::execute4, &nodalFlux::execute5, &nodalFlux::execute6, &nodalFlux::execute7, &nodalFlux::execute8, &nodalFlux::execute9, &nodalFlux::execute10, &nodalFlux::execute11, &nodalFlux::execute12, &nodalFlux::execute13, &nodalFlux::execute14, &nodalFlux::execute15};
      constexpr static member_function_ptr findExecute(unsigned i0, unsigned i1) {
        return ExecutePtrs[1*i0 + 4*i1];
      }
      inline void execute(unsigned i0, unsigned i1) {
        (this->*findExecute(i0, i1))();
      }
      constexpr static unsigned long nonZeroFlops(unsigned i0, unsigned i1) {
        return NonZeroFlops[1*i0 + 4*i1];
      }
      constexpr static unsigned long hardwareFlops(unsigned i0, unsigned i1) {
        return HardwareFlops[1*i0 + 4*i1];
      }
    };
  }
}
#endif