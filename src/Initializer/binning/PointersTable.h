#ifndef INDEX_TABLE_H_ 
#define INDEX_TABLE_H_

#include <unordered_map>
#include <vector>
#include <utility>
#include <string>

#include "specific_types.h"
#include <device_utils.h>

namespace seissol {
  namespace initializers {
    namespace binning {

      class DevicePointers {
      public:
        DevicePointers(std::vector<real *> i_pointers) : m_pointers(i_pointers), m_device_ptrs(nullptr) {
          if (!m_pointers.empty()) {
            m_device_ptrs = (real**)device_malloc(m_pointers.size() * sizeof(real*));
            device_copy_to(m_device_ptrs, m_pointers.data(), m_pointers.size() * sizeof(real*));
          }
        }

        explicit
        DevicePointers(const DevicePointers& other) : m_pointers(other.m_pointers), m_device_ptrs(nullptr) {
          if (!m_pointers.empty()) {
            if (other.m_device_ptrs != nullptr) {
              m_device_ptrs = (real**) device_malloc(other.m_pointers.size() * sizeof(real*));
              device_copy_between(m_device_ptrs, other.m_device_ptrs, other.m_pointers.size() * sizeof(real*));
            }
          }
        }

        DevicePointers& operator=(const DevicePointers& other) = delete;

        virtual ~DevicePointers() {
          if (m_device_ptrs != nullptr) {
            device_free(m_device_ptrs);
            m_device_ptrs = nullptr;
          }
          m_pointers.clear();
        }

        real **get_pointers() { return m_device_ptrs; }
        index_t get_size() { return m_pointers.size(); }

      private:
        std::vector<real*> m_pointers{};
        real **m_device_ptrs{};
      };
    }
  }
}

namespace seissol {
  namespace initializers {
    namespace binning {
      struct PointersTable {

      public:
        PointersTable() : m_is_empty(true) {
          for (auto& pointers: container)
            pointers = nullptr;
        }


        std::array<DevicePointers*, *VariableID::Count> container;

        bool is_empty() { return m_is_empty; };
        void set_not_empty_flag() { m_is_empty = false; }

      private:
        bool m_is_empty;  // TODO: doesn't make sense to keep this flag???
      };
    }
  }
}
#endif 