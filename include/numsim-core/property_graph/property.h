#ifndef NUMSIM_CORE_PG_PROPERTY_H
#define NUMSIM_CORE_PG_PROPERTY_H

#include <cstring>
#include <memory>
#include <ostream>
#include <istream>
#include <numsim-core/property_graph/property_traits.h>

namespace numsim_core {

template<typename Traits>
class property_base;
template<typename Traits>
using property_base_ptr = std::unique_ptr<property_base<Traits>>;
template<typename T, typename Traits>
class property;

template<typename T, typename Traits>
auto make_property(T const& value, Traits const& traits) {
  return std::make_unique<property<T, Traits>>(value, traits);
}

template<typename Traits>
class property_base {
public:
  property_base() = delete;
  property_base(Traits&& traits) : m_traits(std::move(traits)) {}
  property_base(Traits const& traits) : m_traits(traits) {}
  virtual ~property_base() = default;
  virtual property_base_ptr<Traits> clone() const = 0;
  constexpr inline const auto& traits() const { return m_traits; }
  constexpr inline auto& traits() { return m_traits; }

  virtual bool is_history() const noexcept { return false; }
  virtual void commit() noexcept {}
  virtual void revert() noexcept {}

  /// Serialize the property's value to a binary stream.
  virtual void serialize(std::ostream&) const {}
  /// Deserialize the property's value from a binary stream.
  virtual void deserialize(std::istream&) {}

protected:
  Traits m_traits;
};

template<typename T, typename Traits>
class property final : public property_base<Traits> {
public:
  using base = property_base<Traits>;
  property(T const& value, Traits const& traits) : base(traits), m_value(value) {}
  property(T&& value, Traits&& traits) : base(std::forward<Traits>(traits)), m_value(std::forward<T>(value)) {}

  virtual property_base_ptr<Traits> clone() const override {
    return make_property<T, Traits>(m_value, base::m_traits);
  }

  const auto& get() const { return m_value; }
  auto& get() { return m_value; }

private:
  T m_value;
};

// Convenience aliases with default property_traits
using property_base_default = property_base<property_traits>;
using property_base_ptr_default = property_base_ptr<property_traits>;
template<typename T>
using property_default = property<T, property_traits>;

} // namespace numsim_core

#endif // NUMSIM_CORE_PG_PROPERTY_H
