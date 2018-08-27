/*
 * Copyright 2014 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FRUIT_COMPONENT_STORAGE_ENTRY_H
#define FRUIT_COMPONENT_STORAGE_ENTRY_H

#include <fruit/impl/component_storage/binding_deps.h>
#include <fruit/impl/data_structures/arena_allocator.h>
#include <fruit/impl/data_structures/semistatic_graph.h>
#include <fruit/impl/fruit_internal_forward_decls.h>

namespace fruit {
namespace impl {

/**
 * This represents a generic entry in ComponentStorage.
 * We use a single POD (this struct) to represent any binding so that ComponentStorage can hold a single vector, instead
 * of having to hold multiple vectors (each of which potentially requiring allocation/deallocation when a
 * ComponentStorage is constructed/destroyed).
 * This way each ComponentStorage can hold a single vector and do a single allocation.
 */
struct ComponentStorageEntry {
  enum class Kind {
#if FRUIT_EXTRA_DEBUG
    INVALID,
#endif
    BINDING_FOR_CONSTRUCTED_OBJECT,
    BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION,
    BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION,
    BINDING_FOR_OBJECT_TO_CONSTRUCT_WITH_UNKNOWN_ALLOCATION,
    COMPRESSED_BINDING,
    MULTIBINDING_FOR_CONSTRUCTED_OBJECT,
    MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION,
    MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION,
    // This is not an actual binding, it's an "addendum" to multibinding bindings that specifies how the multibinding
    // vector can be created. Unlike real multibinding entries, this *can* be deduped.
    MULTIBINDING_VECTOR_CREATOR,

    LAZY_COMPONENT_WITH_NO_ARGS,
    LAZY_COMPONENT_WITH_ARGS,

    // Component replacements are stored as a REPLACEMENT_LAZY_COMPONENT_* entry followed by a REPLACED_LAZY_COMPONENT_*
    // entry. Note that the args are independent: e.g. a component with args can be replaced by a component with no
    // args. This also means that the type_id of the two entries can be different (since it's the type_id of the
    // function signature rather than just of the Component<...>).
    REPLACED_LAZY_COMPONENT_WITH_NO_ARGS,
    REPLACED_LAZY_COMPONENT_WITH_ARGS,
    REPLACEMENT_LAZY_COMPONENT_WITH_NO_ARGS,
    REPLACEMENT_LAZY_COMPONENT_WITH_ARGS,

    // These markers are used in expandLazyComponents(), see the comments there for details.
    COMPONENT_WITH_ARGS_END_MARKER,
    COMPONENT_WITHOUT_ARGS_END_MARKER,
  };

#if FRUIT_EXTRA_DEBUG
  mutable
#endif
      Kind kind;

  // This is usually the TypeId for the bound type, except:
  // * when kind==COMPRESSED_BINDING, this is the interface's TypeId
  // * when kind==*LAZY_COMPONENT_*, this is the TypeId of the
  //       Component<...>-returning function.
  TypeId type_id;

  /**
   * This represents an entry in ComponentStorage for a binding (not a multibinding) that holds an already-constructed
   * object.
   */
  struct BindingForConstructedObject {
    using object_ptr_t = const void*;

    // The already-constructed object. We do *not* own this, this object must outlive the injector.
    // This is a const pointer because in some cases it might be a const binding.
    // We can cast this to a non-const pointer when we're sure that the original binding was for a non-const reference.
    object_ptr_t object_ptr;

#if FRUIT_EXTRA_DEBUG
    bool is_nonconst;
#endif
  };

  /**
   * This represents an entry in ComponentStorage for a binding (not a multibinding) that holds an object that needs to
   * be constructed (potentially after injecting any dependencies).
   */
  struct BindingForObjectToConstruct {
    // This is a const pointer because this might be a const binding. If not, we'll cast this back to a non-const
    // pointer when we need to.
    using object_t = const void*;
    using create_t = object_t (*)(InjectorStorage&, SemistaticGraph<TypeId, NormalizedBinding>::node_iterator);

    // The return value of this function is a pointer to the constructed object (guaranteed to be !=nullptr).
    // Once the object is constructed (at injection time), the injector owns that object.
    create_t create;

    // The type IDs that this type depends on.
    const BindingDeps* deps;

#if FRUIT_EXTRA_DEBUG
    bool is_nonconst;
#endif
  };

  /**
   * This represents an entry in ComponentStorage for a multibinding that holds an already-constructed
   * object.
   */
  struct MultibindingForConstructedObject {
    using object_ptr_t = void*;

    // The already-constructed object. We do *not* own this, this object must outlive the injector.
    object_ptr_t object_ptr;
  };

  /**
   * This represents an entry in ComponentStorage for a multibinding that holds an object that needs to
   * be constructed (potentially after injecting any dependencies).
   */
  struct MultibindingForObjectToConstruct {

    using object_t = void*;
    using create_t = object_t (*)(InjectorStorage&);

    // The return value of this function is a pointer to the constructed object (guaranteed to be !=nullptr).
    // Once the object is constructed (at injection time), the injector owns that object.
    create_t create;

    // The type IDs that this type depends on.
    const BindingDeps* deps;
  };

  /**
   * This is not an actual binding, it's an "addendum" to multibinding bindings that specifies how the multibinding
   * vector can be created. Unlike real multibinding entries, this *can* be deduped.
   */
  struct MultibindingVectorCreator {

    using get_multibindings_vector_t = std::shared_ptr<char> (*)(InjectorStorage&);

    // Returns the std::vector<T*> of instances, or nullptr if none.
    // Caches the result in the `v' member of NormalizedMultibindingData.
    get_multibindings_vector_t get_multibindings_vector;
  };

  // A CompressedBinding with interface_id==getTypeId<I>() and class_id==getTypeId<C>() means that if:
  // * C is not exposed by the component
  // * I is the only node that depends on C
  // * There are no multibindings that directly depend on C
  // The BindingData for C is BindingForObjectToConstruct(
  // Then, taken create1, needs_reallocation such that the ComponentStorageEntry for c_type_id is
  // BindingForObjectToConstruct(createC, deps, needs_allocation), we can remove the binding for I and C and replace
  // them
  // with just a binding for I, with BindingForObjectToConstruct(create, deps, needs_allocation).
  struct CompressedBinding {

    using create_t = BindingForObjectToConstruct::create_t;

    // TypeId for the implementation.
    TypeId c_type_id;

    // The return value of this function is a pointer to the constructed object (guaranteed to be !=nullptr).
    // Once the object is constructed (at injection time), the injector owns that object.
    create_t create;
  };

  /**
   * This represents an entry in ComponentStorage for a lazy component with no arguments.
   */
  struct LazyComponentWithNoArgs {
    // An arbitrary function type, used as type for the field `erased_fun`.
    // Note that we can't use void* here, since data pointers might not have the same size as function pointers.
    using erased_fun_t = void (*)();

    // The function that will be invoked to create the Component.
    // Here we don't know the type, it's only known at construction time.
    erased_fun_t erased_fun;

    using entry_vector_t = std::vector<ComponentStorageEntry, ArenaAllocator<ComponentStorageEntry>>;

    // The function that allows to add this component's bindings to the given ComponentStorage.
    using add_bindings_fun_t = void (*)(erased_fun_t, entry_vector_t&);
    add_bindings_fun_t add_bindings_fun;

    template <typename Component>
    static void addBindings(erased_fun_t erased_fun, entry_vector_t& entries);

    template <typename Component>
    static ComponentStorageEntry create(Component (*fun)());

    template <typename Component>
    static ComponentStorageEntry create(fruit::ComponentFunction<Component> component_function);

    template <typename Component>
    static ComponentStorageEntry createReplacedComponentEntry(Component (*fun)());

    template <typename Component>
    static ComponentStorageEntry createReplacementComponentEntry(Component (*fun)());

    bool operator==(const LazyComponentWithNoArgs&) const;

    void addBindings(entry_vector_t& entries) const;

    std::size_t hashCode() const;

    bool isValid() const;
  };

  /**
   * This represents an entry in ComponentStorage for a lazy component with arguments.
   */
  struct LazyComponentWithArgs {
    class ComponentInterface {
    public:
      // An arbitrary function type, used as type for the field `erased_fun`.
      // Note that we can't use void* here, since data pointers might not have the same size as function pointers.
      using erased_fun_t = void (*)();

      // The function that will be invoked to create the Component.
      // Here we don't know the type, it's only known to the LazyComponent implementation.
      // We store this here instead of in the LazyComponent implementation so that we can do a quick comparison on the
      // pointer without virtual calls (and we can then do the rest of the comparison via virtual call if needed).
      erased_fun_t erased_fun;

      using entry_vector_t = std::vector<ComponentStorageEntry, ArenaAllocator<ComponentStorageEntry>>;

      ComponentInterface(erased_fun_t erased_fun);

      virtual ~ComponentInterface() = default;

      // Checks if *this and other are equal, assuming that this->fun and other.fun are equal.
      virtual bool areParamsEqual(const ComponentInterface& other) const = 0;

      bool operator==(const ComponentInterface& other) const;

      virtual void addBindings(entry_vector_t& component_storage_entries) const = 0;
      virtual std::size_t hashCode() const = 0;
      virtual ComponentInterface* copy() const = 0;

      /**
       * Returns the type ID of the real `fun` object stored by the implementation.
       * We use this instead of the `typeid` operator so that we don't require RTTI.
       */
      virtual TypeId getFunTypeId() const = 0;
    };

    template <typename Component, typename... Args>
    static ComponentStorageEntry create(Component (*fun)(Args...), std::tuple<Args...> args_tuple);

    template <typename Component, typename Arg, typename... Args>
    static ComponentStorageEntry create(fruit::ComponentFunction<Component, Arg, Args...> component_function);

    template <typename Component, typename... Args>
    static ComponentStorageEntry createReplacedComponentEntry(Component (*fun)(Args...),
                                                              std::tuple<Args...> args_tuple);

    template <typename Component, typename... Args>
    static ComponentStorageEntry createReplacementComponentEntry(Component (*fun)(Args...),
                                                                 std::tuple<Args...> args_tuple);

    LazyComponentWithArgs(LazyComponentWithArgs&&) = default;
    LazyComponentWithArgs& operator=(LazyComponentWithArgs&&) = default;

    // Note: we must allow these (and use the default implementations) since this class is used in a union so it must be
    // a POD. However when we need a real object we must call the other constructor above, and when we need a copy we
    // must
    // call copy() explicitly.
    LazyComponentWithArgs() = default; // LCOV_EXCL_LINE
    LazyComponentWithArgs(const LazyComponentWithArgs&) = default;
    LazyComponentWithArgs& operator=(const LazyComponentWithArgs&) = default;

    LazyComponentWithArgs copy() const;
    void destroy() const;

    ComponentInterface* component;
  };

  union {
    // Valid iff kind is BINDING_FOR_CONSTRUCTED_OBJECT.
    BindingForConstructedObject binding_for_constructed_object;

    // Valid iff kind is BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_[NO_]ALLOCATION
    // or BINDING_FOR_OBJECT_TO_CONSTRUCT_WITH_UNKNOWN_ALLOCATION.
    BindingForObjectToConstruct binding_for_object_to_construct;

    // Valid iff kind is MULTIBINDING_FOR_CONSTRUCTED_OBJECT.
    MultibindingForConstructedObject multibinding_for_constructed_object;

    // Valid iff kind is MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_[NO_]ALLOCATION.
    MultibindingForObjectToConstruct multibinding_for_object_to_construct;

    // Valid iff kind is MULTIBINDING_VECTOR_CREATOR.
    MultibindingVectorCreator multibinding_vector_creator;

    // Valid iff kind is COMPRESSED_BINDING.
    CompressedBinding compressed_binding;

    // Valid iff kind is LAZY_COMPONENT_WITH_NO_ARGS, REPLACED_LAZY_COMPONENT_WITH_NO_ARGS or
    // REPLACEMENT_LAZY_COMPONENT_WITH_NO_ARGS.
    LazyComponentWithNoArgs lazy_component_with_no_args;

    // Valid iff kind is LAZY_COMPONENT_WITH_ARGS, REPLACED_LAZY_COMPONENT_WITH_ARGS or
    // REPLACEMENT_LAZY_COMPONENT_WITH_ARGS.
    LazyComponentWithArgs lazy_component_with_args;
  };

  // We use a custom method instead of a real copy constructor so that all copies are explicit (since copying is a
  // fairly expensive operation).
  ComponentStorageEntry copy() const;

  // We use a custom method instead of a real destructor, so that we can hold these in a std::vector but still destroy
  // them when desired.
  void destroy() const;
};

// We can't have this assert in debug mode because we add debug-only fields that increase the size.
#if !FRUIT_EXTRA_DEBUG
// This is not required for correctness, but 4 64-bit words should be enough to hold this object, if not we'd end up
// using more memory/CPU than expected.
static_assert(sizeof(ComponentStorageEntry) <= 4 * sizeof(std::uint64_t),
              "Error: a ComponentStorageEntry doesn't fit in 32 bytes as we expected");
#endif

} // namespace impl
} // namespace fruit

#include <fruit/impl/component_storage/component_storage_entry.defn.h>

#endif // FRUIT_COMPONENT_STORAGE_ENTRY_H
