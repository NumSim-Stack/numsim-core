#ifndef NUMSIM_CORE_PG_INPUT_TYPES_H
#define NUMSIM_CORE_PG_INPUT_TYPES_H

#include <stdexcept>
#include <string>
#include <numsim-core/property_graph/property.h>
#include <numsim-core/property_graph/history_property.h>

namespace numsim_core {

/// Type-erased base for input wiring at finalize() time.
class input_wire_base {
public:
  virtual ~input_wire_base() = default;
  virtual const std::string& source_owner() const noexcept = 0;
  virtual const std::string& source_name() const noexcept = 0;
  virtual EdgeKind edge_kind() const noexcept = 0;
  virtual bool is_wired() const noexcept = 0;
  virtual void wire(const property_base<property_traits>& source) = 0;

  /// Trigger the source property's update callback.
  /// Used by solvers to re-evaluate a consumed property in their inner loop.
  virtual void update_source() const = 0;
};

/// Parsed "material::property" reference for config-driven wiring.
struct connection_source {
  std::string material;
  std::string property;

  static connection_source parse(const std::string& s) {
    auto pos = s.find("::");
    if (pos == std::string::npos)
      throw std::invalid_argument(
          "connection_source::parse: expected 'material::property', got '" + s + "'");
    return {s.substr(0, pos), s.substr(pos + 2)};
  }
};

/// Typed read-only input referencing another material's property.
///
/// Stores a raw pointer to the source property's data (set at wire() time).
/// This is safe because properties are stored as unique_ptr in the registry,
/// which provides pointer stability across unordered_map rehashes.
/// The material_context guarantees that the property registry outlives all
/// materials (see member declaration order in material_context.h).
///
/// After finalize(), get() is noexcept — all inputs are guaranteed wired.
template<typename T, typename Traits = property_traits>
class input_property : public input_wire_base {
public:
  input_property(std::string source_owner, std::string source_name,
                 EdgeKind edge_kind = EdgeKind::Global)
      : m_source_owner(std::move(source_owner)),
        m_source_name(std::move(source_name)),
        m_edge_kind(edge_kind) {}

  const T& get() const noexcept { return *m_ptr; }

  bool is_wired() const noexcept override { return m_ptr != nullptr; }
  const std::string& source_owner() const noexcept override { return m_source_owner; }
  const std::string& source_name() const noexcept override { return m_source_name; }
  EdgeKind edge_kind() const noexcept override { return m_edge_kind; }

  void wire(const property_base<property_traits>& source) override {
    // Try regular property first
    if (auto* typed = dynamic_cast<const property<T, Traits>*>(&source)) {
      m_ptr = &typed->get();
    }
    // Fall back to history_property (read new_value)
    else if (auto* hist = dynamic_cast<const history_property<T, Traits>*>(&source)) {
      m_ptr = &hist->new_value();
    }
    else {
      throw std::runtime_error(
          "input_property::wire(): type mismatch wiring '" +
          m_source_owner + "::" + m_source_name + "'");
    }
    m_source_prop = &source;
  }

  void update_source() const override {
    if (m_source_prop) {
      const auto& cb = m_source_prop->traits().update;
      if (cb) cb();
    }
  }

private:
  std::string m_source_owner;
  std::string m_source_name;
  EdgeKind m_edge_kind;
  const T* m_ptr{nullptr};
  const property_base<property_traits>* m_source_prop{nullptr};
};

/// Typed read-only input referencing another material's history_property.
template<typename T, typename Traits = property_traits>
class input_history : public input_wire_base {
public:
  input_history(std::string source_owner, std::string source_name,
                EdgeKind edge_kind = EdgeKind::Global)
      : m_source_owner(std::move(source_owner)),
        m_source_name(std::move(source_name)),
        m_edge_kind(edge_kind) {}

  const T& old_value() const noexcept { return *m_old_ptr; }
  const T& new_value() const noexcept { return *m_new_ptr; }

  bool is_wired() const noexcept override { return m_new_ptr != nullptr; }
  const std::string& source_owner() const noexcept override { return m_source_owner; }
  const std::string& source_name() const noexcept override { return m_source_name; }
  EdgeKind edge_kind() const noexcept override { return m_edge_kind; }

  void wire(const property_base<property_traits>& source) override {
    auto* hp = dynamic_cast<const history_property<T, Traits>*>(&source);
    if (!hp)
      throw std::runtime_error(
          "input_history::wire(): type mismatch or not a history property for '" +
          m_source_owner + "::" + m_source_name + "'");
    m_old_ptr = &hp->old_value();
    m_new_ptr = &hp->new_value();
    m_source_prop = &source;
  }

  void update_source() const override {
    if (m_source_prop) {
      const auto& cb = m_source_prop->traits().update;
      if (cb) cb();
    }
  }

private:
  std::string m_source_owner;
  std::string m_source_name;
  EdgeKind m_edge_kind;
  const T* m_old_ptr{nullptr};
  const T* m_new_ptr{nullptr};
  const property_base<property_traits>* m_source_prop{nullptr};
};

} // namespace numsim_core

#endif // NUMSIM_CORE_PG_INPUT_TYPES_H
