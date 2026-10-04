# Critical review — `history-property-serialize-adl` (fixes #16)

3-lens parallel review (architect-reviewer + cpp-pro + code-reviewer) of the
branch diff. The `if constexpr` rewrite itself was confirmed correct by both
C++ lenses — discarded-branch semantics genuinely remove the #16 instantiation
barrier, and the CMake test is auto-wired via the `tests/` glob. Findings below
are about the *contract* the fix establishes; all three HIGHs are silent-
correctness holes the test suite cannot catch by construction.

## Fixed in the review-hardening commit

| # | Sev | Finding | Fix |
|---|-----|---------|-----|
| A | **HIGH** | **ODR hazard (cpp-pro).** `has_numsim_serialize<T>` is detected by ADL *at the point of instantiation*; if the hook is visible in some TUs but not others, the trait — and the `serialize()` virtual body that branches on it — differ across TUs (silent ODR violation; linker picks one vtable slot). Latent today (real `T`=`double`, no hook). | Documented the contract loudly in the customization-point comment: the hook **MUST** be declared in `T`'s own header (an associated namespace of `T`) so every TU sees it uniformly. Cannot be machine-enforced; the doc is the mitigation. |
| B | **HIGH** | **Asymmetric hooks (architect).** `serialize`/`deserialize` detect hooks independently → a type with `numsim_serialize` but no `numsim_deserialize` writes state it can never read back. | `static_assert(has_numsim_deserialize<T>::value, …)` inside the *taken* hook branch of `serialize()` (and the mirror in `deserialize()`). Sits in an `if constexpr` branch, so the neither-hook case still compiles → **#16 stays fixed**. Sabotage-verified: a serialize-only type now fails to compile with the both-or-neither message. |
| C | **HIGH** | **Dispatch precedence (architect + code-reviewer).** `is_trivially_copyable` is checked before the hook → a trivially-copyable `T` that provides a hook gets it silently ignored (raw bytes win) — the exact POD-portability case a hook exists for. | `static_assert(!has_numsim_serialize<T>::value, …)` inside the trivially-copyable branch ("reject loudly"). For `T`=`double` the trait is `false` → passes; only the ambiguous combination errors. Sabotage-verified. |
| D | MED | **No I/O stream-state check (architect).** `os.write`/`is.read` failures unchecked; a truncated read leaves `m_old/m_new` indeterminate, indistinguishable from success. | `if (!os)` / `if (!is)` after the raw read/write → throw. |
| G | MED | `EXPECT_THROW(std::runtime_error)` too loose (both throws share the base type; a wrong-path throw satisfies it). | Replaced with a `throws_with(fn, needle)` helper matching the message substring (`numsim_serialize` / `numsim_deserialize`); no gmock dependency. |
| H | LOW | Missing `#include <utility>` for `std::declval` (worked via transitive `<type_traits>`; non-conforming). | Added `#include <utility>`. |
| I | LOW | Detection traits checked well-formedness only → false-positive on a non-`void` `numsim_serialize`. | Constrained to `std::enable_if_t<std::is_void_v<decltype(...)>>`. |
| J | LOW | Raw `reinterpret_cast`+`sizeof` is now the documented default — non-portable/un-versioned. | Documented as host-native/same-build only, **not** a portable archive format. |
| K | LOW | `clone()` test asserted only non-null; hook round-trip didn't pin that bytes were produced. | `clone()` test now `dynamic_cast`s, checks values + source-mutation independence; hook test asserts `!os.str().empty()`. |

## Fixed (E) and accepted-by-proof (F)

- **E (MED) — no CI in numsim-core → FIXED.** The #16 regression is clang-specific (eager virtual instantiation per [temp.inst]/11); the repo had no `.github/workflows`, so nothing built it under clang automatically — local g++ gives false-green. **Added `.github/workflows/build.yml`** (mirrors the numsim-codegen matrix): gcc-14 + **clang-19** × {Debug, Release}, configure → build → `ctest`. clang is now a required matrix leg, so the regression is guarded on every push/PR. The cross-repo real-type guard (`history_property<tmech::tensor>`) still belongs downstream — numsim-core cannot depend on tmech.
- **F (MED) — `commit()/revert()` `noexcept` + throwing copy-assignment `T` → ACCEPTED, code-guard proven harmful.** A `T` with throwing copy-assignment (e.g. `std::vector`-backed, under allocation failure) would `std::terminate` in these `noexcept` overrides. **The intended value types are safe**: scalars, and `tmech::tensor` whose `operator=`/copy-ctor are `noexcept` (verified, array-backed). A `static_assert(is_nothrow_copy_assignable_v<T>)` is **not** a valid fix: `commit()/revert()` are virtuals pulled into the vtable, so the assert fires at *class instantiation* — re-blocking `history_property<T>` for throwing-assign `T` exactly as the original #16 static_assert did (proven: `history_property<std::vector>-backed` instantiates today, would not with the guard). The base contract is `noexcept`, which an override cannot widen. So the precondition is documented on the value type near `commit()/revert()` and left to the type; revisit only if a throwing-assign history value type is ever genuinely needed (which would require dropping `noexcept` from the base across all property types).

## Verification

- Suite: **62/62** under g++-14 **and** clang-18 (history_property tests #1–#4 included).
- Sabotage: both new compile-time guards (B, C) fire with the intended messages; reverting them is the only way the ambiguous/asymmetric probes compile.
- Real-type regression: `history_property<tmech::tensor<double,3,2>>` (explicit `template class` forcing all virtuals) compiles **and** runs (commit/revert/clone; serialize throws, no hook) under clang — the exact #16 failure, resolved.
