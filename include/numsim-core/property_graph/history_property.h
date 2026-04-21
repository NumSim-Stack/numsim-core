#ifndef NUMSIM_CORE_PG_HISTORY_PROPERTY_H
#define NUMSIM_CORE_PG_HISTORY_PROPERTY_H

#include <type_traits>
#include <numsim-core/property_graph/property.h>

namespace numsim_core {

template<typename T, typename Traits>
class history_property;

template<typename T, typename Traits>
auto make_history_property(T&& old_value, T&& new_value, Traits&& traits) {
  using value_t = std::decay_t<T>;
  using traits_t = std::decay_t<Traits>;
  return std::make_unique<history_property<value_t, traits_t>>(
      std::forward<T>(old_value), std::forward<T>(new_value), std::forward<Traits>(traits));
}

template<typename T, typename Traits = property_traits>
class history_property final : public property_base<Traits> {
public:
  using base = property_base<Traits>;

  history_property(T&& old_value, T&& new_value, Traits&& traits)
      : base(std::forward<Traits>(traits)), m_old(std::move(old_value)), m_new(std::move(new_value)) {}
  history_property(const T& old_value, const T& new_value, const Traits& traits)
      : base(traits), m_old(old_value), m_new(new_value) {}

  virtual property_base_ptr<Traits> clone() const override {
    return make_history_property(m_old, m_new, base::m_traits);
  }

  T& old_value() { return m_old; }
  T& new_value() { return m_new; }
  const T& old_value() const { return m_old; }
  const T& new_value() const { return m_new; }

  static_assert(!std::is_reference_v<T>,
                "history_property does not support reference types");

  bool is_history() const noexcept override { return true; }
  void commit() noexcept override { m_old = m_new; }
  void revert() noexcept override { m_new = m_old; }

  /// Serialize old and new values as raw bytes.
  /// Only valid for trivially copyable types (scalars, tensors).
  void serialize(std::ostream& os) const override {
    static_assert(std::is_trivially_copyable_v<T>,
                  "history_property::serialize requires trivially copyable type");
    os.write(reinterpret_cast<const char*>(&m_old), sizeof(T));
    os.write(reinterpret_cast<const char*>(&m_new), sizeof(T));
  }

  void deserialize(std::istream& is) override {
    static_assert(std::is_trivially_copyable_v<T>,
                  "history_property::deserialize requires trivially copyable type");
    is.read(reinterpret_cast<char*>(&m_old), sizeof(T));
    is.read(reinterpret_cast<char*>(&m_new), sizeof(T));
  }

private:
  T m_old;
  T m_new;
};

} // namespace numsim_core

#endif // NUMSIM_CORE_PG_HISTORY_PROPERTY_H
