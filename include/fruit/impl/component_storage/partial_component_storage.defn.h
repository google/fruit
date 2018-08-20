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

#ifndef FRUIT_PARTIAL_COMPONENT_STORAGE_DEFN_H
#define FRUIT_PARTIAL_COMPONENT_STORAGE_DEFN_H

#include <fruit/impl/component_storage/partial_component_storage.h>

#include <fruit/impl/bindings.h>
#include <fruit/impl/injector/injector_storage.h>
#include <fruit/impl/util/call_with_tuple.h>
#include <fruit/impl/util/type_info.h>
#include <utility>

namespace fruit {
namespace impl {

template <>
class PartialComponentStorage<> {
public:
  void addBindings(FixedSizeVector<ComponentStorageEntry>& entries) const {
    (void)entries;
  }

  std::size_t numBindings() const {
    return 0;
  }
};

template <typename I, typename C, typename... PreviousBindings>
class PartialComponentStorage<Bind<I, C>, PreviousBindings...> {
private:
  PartialComponentStorage<PreviousBindings...>& previous_storage;

public:
  PartialComponentStorage(PartialComponentStorage<PreviousBindings...>& previous_storage)
      : previous_storage(previous_storage) {}

  void addBindings(FixedSizeVector<ComponentStorageEntry>& entries) const {
    previous_storage.addBindings(entries);
  }

  std::size_t numBindings() const {
    return previous_storage.numBindings();
  }
};

template <typename Signature, typename... PreviousBindings>
class PartialComponentStorage<RegisterConstructor<Signature>, PreviousBindings...> {
private:
  PartialComponentStorage<PreviousBindings...>& previous_storage;

public:
  PartialComponentStorage(PartialComponentStorage<PreviousBindings...>& previous_storage)
      : previous_storage(previous_storage) {}

  void addBindings(FixedSizeVector<ComponentStorageEntry>& entries) const {
    previous_storage.addBindings(entries);
  }

  std::size_t numBindings() const {
    return previous_storage.numBindings();
  }
};

template <typename C, typename C1, typename... PreviousBindings>
class PartialComponentStorage<BindInstance<C, C1>, PreviousBindings...> {
private:
  PartialComponentStorage<PreviousBindings...>& previous_storage;
  C& instance;

public:
  PartialComponentStorage(PartialComponentStorage<PreviousBindings...>& previous_storage, C& instance)
      : previous_storage(previous_storage), instance(instance) {}

  void addBindings(FixedSizeVector<ComponentStorageEntry>& entries) const {
    entries.push_back(InjectorStorage::createComponentStorageEntryForBindInstance<C, C>(instance));
    previous_storage.addBindings(entries);
  }

  std::size_t numBindings() const {
    return previous_storage.numBindings() + 1;
  }
};

template <typename C, typename C1, typename... PreviousBindings>
class PartialComponentStorage<BindConstInstance<C, C1>, PreviousBindings...> {
private:
  PartialComponentStorage<PreviousBindings...>& previous_storage;
  const C& instance;

public:
  PartialComponentStorage(PartialComponentStorage<PreviousBindings...>& previous_storage, const C& instance)
      : previous_storage(previous_storage), instance(instance) {}

  void addBindings(FixedSizeVector<ComponentStorageEntry>& entries) const {
    entries.push_back(InjectorStorage::createComponentStorageEntryForBindConstInstance<C, C>(instance));
    previous_storage.addBindings(entries);
  }

  std::size_t numBindings() const {
    return previous_storage.numBindings() + 1;
  }
};

template <typename C, typename Annotation, typename C1, typename... PreviousBindings>
class PartialComponentStorage<BindInstance<fruit::Annotated<Annotation, C>, C1>, PreviousBindings...> {
private:
  PartialComponentStorage<PreviousBindings...>& previous_storage;
  C& instance;

public:
  PartialComponentStorage(PartialComponentStorage<PreviousBindings...>& previous_storage, C& instance)
      : previous_storage(previous_storage), instance(instance) {}

  void addBindings(FixedSizeVector<ComponentStorageEntry>& entries) const {
    entries.push_back(
        InjectorStorage::createComponentStorageEntryForBindInstance<fruit::Annotated<Annotation, C>, C>(instance));
    previous_storage.addBindings(entries);
  }

  std::size_t numBindings() const {
    return previous_storage.numBindings() + 1;
  }
};

template <typename C, typename Annotation, typename C1, typename... PreviousBindings>
class PartialComponentStorage<BindConstInstance<fruit::Annotated<Annotation, C>, C1>, PreviousBindings...> {
private:
  PartialComponentStorage<PreviousBindings...>& previous_storage;
  const C& instance;

public:
  PartialComponentStorage(PartialComponentStorage<PreviousBindings...>& previous_storage, const C& instance)
      : previous_storage(previous_storage), instance(instance) {}

  void addBindings(FixedSizeVector<ComponentStorageEntry>& entries) const {
    entries.push_back(
        InjectorStorage::createComponentStorageEntryForBindConstInstance<fruit::Annotated<Annotation, C>, C>(instance));
    previous_storage.addBindings(entries);
  }

  std::size_t numBindings() const {
    return previous_storage.numBindings() + 1;
  }
};

template <typename... Params, typename... PreviousBindings>
class PartialComponentStorage<RegisterProvider<Params...>, PreviousBindings...> {
private:
  PartialComponentStorage<PreviousBindings...>& previous_storage;

public:
  PartialComponentStorage(PartialComponentStorage<PreviousBindings...>& previous_storage)
      : previous_storage(previous_storage) {}

  void addBindings(FixedSizeVector<ComponentStorageEntry>& entries) const {
    previous_storage.addBindings(entries);
  }

  std::size_t numBindings() const {
    return previous_storage.numBindings();
  }
};

template <typename C, typename... PreviousBindings>
class PartialComponentStorage<AddInstanceMultibinding<C>, PreviousBindings...> {
private:
  PartialComponentStorage<PreviousBindings...>& previous_storage;
  C& instance;

public:
  PartialComponentStorage(PartialComponentStorage<PreviousBindings...>& previous_storage, C& instance)
      : previous_storage(previous_storage), instance(instance) {}

  void addBindings(FixedSizeVector<ComponentStorageEntry>& entries) const {
    entries.push_back(InjectorStorage::createComponentStorageEntryForInstanceMultibinding<C, C>(instance));
    entries.push_back(InjectorStorage::createComponentStorageEntryForMultibindingVectorCreator<C>());
    previous_storage.addBindings(entries);
  }

  std::size_t numBindings() const {
    return previous_storage.numBindings() + 2;
  }
};

template <typename C, typename Annotation, typename... PreviousBindings>
class PartialComponentStorage<AddInstanceMultibinding<fruit::Annotated<Annotation, C>>, PreviousBindings...> {
private:
  PartialComponentStorage<PreviousBindings...>& previous_storage;
  C& instance;

public:
  PartialComponentStorage(PartialComponentStorage<PreviousBindings...>& previous_storage, C& instance)
      : previous_storage(previous_storage), instance(instance) {}

  void addBindings(FixedSizeVector<ComponentStorageEntry>& entries) const {
    entries.push_back(
        InjectorStorage::createComponentStorageEntryForInstanceMultibinding<fruit::Annotated<Annotation, C>, C>(
            instance));
    entries.push_back(
        InjectorStorage::createComponentStorageEntryForMultibindingVectorCreator<fruit::Annotated<Annotation, C>>());
    previous_storage.addBindings(entries);
  }

  std::size_t numBindings() const {
    return previous_storage.numBindings() + 2;
  }
};

template <typename C, typename... PreviousBindings>
class PartialComponentStorage<AddInstanceVectorMultibindings<C>, PreviousBindings...> {
private:
  PartialComponentStorage<PreviousBindings...>& previous_storage;
  std::vector<C>& instances;

public:
  PartialComponentStorage(PartialComponentStorage<PreviousBindings...>& previous_storage, std::vector<C>& instances)
      : previous_storage(previous_storage), instances(instances) {}

  void addBindings(FixedSizeVector<ComponentStorageEntry>& entries) const {
    for (auto i = instances.rbegin(), i_end = instances.rend(); i != i_end; ++i) {
      // TODO: consider optimizing this so that we need just 1 MULTIBINDING_VECTOR_CREATOR entry (removing the
      // assumption that each multibinding entry is always preceded by that).
      entries.push_back(InjectorStorage::createComponentStorageEntryForInstanceMultibinding<C, C>(*i));
      entries.push_back(InjectorStorage::createComponentStorageEntryForMultibindingVectorCreator<C>());
    }
    previous_storage.addBindings(entries);
  }

  std::size_t numBindings() const {
    return previous_storage.numBindings() + instances.size() * 2;
  }
};

template <typename C, typename Annotation, typename... PreviousBindings>
class PartialComponentStorage<AddInstanceVectorMultibindings<fruit::Annotated<Annotation, C>>, PreviousBindings...> {
private:
  PartialComponentStorage<PreviousBindings...>& previous_storage;
  std::vector<C>& instances;

public:
  PartialComponentStorage(PartialComponentStorage<PreviousBindings...>& previous_storage, std::vector<C>& instances)
      : previous_storage(previous_storage), instances(instances) {}

  void addBindings(FixedSizeVector<ComponentStorageEntry>& entries) const {
    for (auto i = instances.rbegin(), i_end = instances.rend(); i != i_end; ++i) {
      // TODO: consider optimizing this so that we need just 1 MULTIBINDING_VECTOR_CREATOR entry (removing the
      // assumption that each multibinding entry is always preceded by that).
      entries.push_back(
          InjectorStorage::createComponentStorageEntryForInstanceMultibinding<fruit::Annotated<Annotation, C>, C>(*i));
      entries.push_back(
          InjectorStorage::createComponentStorageEntryForMultibindingVectorCreator<fruit::Annotated<Annotation, C>>());
    }
    previous_storage.addBindings(entries);
  }

  std::size_t numBindings() const {
    return previous_storage.numBindings() + instances.size() * 2;
  }
};

template <typename I, typename C, typename... PreviousBindings>
class PartialComponentStorage<AddMultibinding<I, C>, PreviousBindings...> {
private:
  PartialComponentStorage<PreviousBindings...>& previous_storage;

public:
  PartialComponentStorage(PartialComponentStorage<PreviousBindings...>& previous_storage)
      : previous_storage(previous_storage) {}

  void addBindings(FixedSizeVector<ComponentStorageEntry>& entries) const {
    previous_storage.addBindings(entries);
  }

  std::size_t numBindings() const {
    return previous_storage.numBindings();
  }
};

template <typename... Params, typename... PreviousBindings>
class PartialComponentStorage<AddMultibindingProvider<Params...>, PreviousBindings...> {
private:
  PartialComponentStorage<PreviousBindings...>& previous_storage;

public:
  PartialComponentStorage(PartialComponentStorage<PreviousBindings...>& previous_storage)
      : previous_storage(previous_storage) {}

  void addBindings(FixedSizeVector<ComponentStorageEntry>& entries) const {
    previous_storage.addBindings(entries);
  }

  std::size_t numBindings() const {
    return previous_storage.numBindings();
  }
};

template <typename DecoratedSignature, typename Lambda, typename... PreviousBindings>
class PartialComponentStorage<RegisterFactory<DecoratedSignature, Lambda>, PreviousBindings...> {
private:
  PartialComponentStorage<PreviousBindings...>& previous_storage;

public:
  PartialComponentStorage(PartialComponentStorage<PreviousBindings...>& previous_storage)
      : previous_storage(previous_storage) {}

  void addBindings(FixedSizeVector<ComponentStorageEntry>& entries) const {
    previous_storage.addBindings(entries);
  }

  std::size_t numBindings() const {
    return previous_storage.numBindings();
  }
};

template <typename OtherComponent, typename... PreviousBindings>
class PartialComponentStorage<InstallComponent<OtherComponent()>, PreviousBindings...> {
private:
  PartialComponentStorage<PreviousBindings...>& previous_storage;
  OtherComponent (*fun)();

public:
  PartialComponentStorage(PartialComponentStorage<PreviousBindings...>& previous_storage, OtherComponent (*fun1)(),
                          std::tuple<>)
      : previous_storage(previous_storage), fun(fun1) {}

  void addBindings(FixedSizeVector<ComponentStorageEntry>& entries) const {
    entries.push_back(ComponentStorageEntry::LazyComponentWithNoArgs::create(fun));
    previous_storage.addBindings(entries);
  }

  std::size_t numBindings() const {
    return previous_storage.numBindings() + 1;
  }
};

template <typename OtherComponent, typename... Args, typename... PreviousBindings>
class PartialComponentStorage<InstallComponent<OtherComponent(Args...)>, PreviousBindings...> {
private:
  PartialComponentStorage<PreviousBindings...>& previous_storage;
  OtherComponent (*fun)(Args...);
  std::tuple<Args...> args_tuple;

public:
  PartialComponentStorage(PartialComponentStorage<PreviousBindings...>& previous_storage,
                          OtherComponent (*fun1)(Args...), std::tuple<Args...> args_tuple)
      : previous_storage(previous_storage), fun(fun1), args_tuple(std::move(args_tuple)) {}

  void addBindings(FixedSizeVector<ComponentStorageEntry>& entries) {
    entries.push_back(ComponentStorageEntry::LazyComponentWithArgs::create(fun, std::move(args_tuple)));
    previous_storage.addBindings(entries);
  }

  std::size_t numBindings() const {
    return previous_storage.numBindings() + 1;
  }
};

template <std::size_t i, typename ComponentFunctionsTuple>
struct AddAllComponentStorageEntries {
    inline void operator()(FixedSizeVector<ComponentStorageEntry>& entries,
                           ComponentFunctionsTuple& component_functions_tuple) {
      AddAllComponentStorageEntries<i - 1, ComponentFunctionsTuple>()(entries, component_functions_tuple);
      entries.push_back(createEntry(std::move(std::get<i - 1>(component_functions_tuple))));
    }

    template <typename ComponentType>
    inline ComponentStorageEntry createEntry(
        fruit::ComponentFunction<ComponentType> component_function) {
      return ComponentStorageEntry::LazyComponentWithNoArgs::create(std::move(component_function));
    }

    template <typename ComponentType, typename Arg, typename... Args>
    inline ComponentStorageEntry createEntry(
        fruit::ComponentFunction<ComponentType, Arg, Args...> component_function) {
      return ComponentStorageEntry::LazyComponentWithArgs::create(std::move(component_function));
    }
};

template <typename ComponentFunctionsTuple>
struct AddAllComponentStorageEntries<0, ComponentFunctionsTuple> {
    inline void operator()(FixedSizeVector<ComponentStorageEntry>&,
                           ComponentFunctionsTuple&) {}
};

template <typename ComponentFunctionsTuple>
void addAllComponentStorageEntries(FixedSizeVector<ComponentStorageEntry>& entries,
                                   ComponentFunctionsTuple&& component_functions_tuple) {
  AddAllComponentStorageEntries<std::tuple_size<ComponentFunctionsTuple>::value,
                                ComponentFunctionsTuple>()(
      entries, component_functions_tuple);
}

template <typename... ComponentFunctions, typename... PreviousBindings>
class PartialComponentStorage<InstallComponentFunctions<ComponentFunctions...>, PreviousBindings...> {
private:
  PartialComponentStorage<PreviousBindings...>& previous_storage;
  std::tuple<ComponentFunctions...> component_functions_tuple;

public:
  PartialComponentStorage(PartialComponentStorage<PreviousBindings...>& previous_storage,
                          std::tuple<ComponentFunctions...> component_functions_tuple)
      : previous_storage(previous_storage), component_functions_tuple(std::move(component_functions_tuple)) {}

  void addBindings(FixedSizeVector<ComponentStorageEntry>& entries) {
    addAllComponentStorageEntries(entries, std::move(component_functions_tuple));
    previous_storage.addBindings(entries);
  }

  std::size_t numBindings() const {
    return previous_storage.numBindings() + sizeof...(ComponentFunctions);
  }
};

template <typename OtherComponent, typename... PreviousBindings>
class PartialComponentStorage<PartialReplaceComponent<OtherComponent()>, PreviousBindings...> {
private:
  PartialComponentStorage<PreviousBindings...>& previous_storage;
  OtherComponent (*fun)();

public:
  PartialComponentStorage(PartialComponentStorage<PreviousBindings...>& previous_storage, OtherComponent (*fun1)(),
                          std::tuple<>)
      : previous_storage(previous_storage), fun(fun1) {}

  void addBindings(FixedSizeVector<ComponentStorageEntry>& entries) const {
    entries.push_back(ComponentStorageEntry::LazyComponentWithNoArgs::createReplacedComponentEntry(fun));
    previous_storage.addBindings(entries);
  }

  std::size_t numBindings() const {
    return previous_storage.numBindings() + 1;
  }
};

template <typename OtherComponent, typename... ReplacedFunArgs, typename... PreviousBindings>
class PartialComponentStorage<PartialReplaceComponent<OtherComponent(ReplacedFunArgs...)>, PreviousBindings...> {
private:
  PartialComponentStorage<PreviousBindings...>& previous_storage;
  OtherComponent (*fun)(ReplacedFunArgs...);
  std::tuple<ReplacedFunArgs...> args_tuple;

public:
  PartialComponentStorage(PartialComponentStorage<PreviousBindings...>& previous_storage,
                          OtherComponent (*fun1)(ReplacedFunArgs...), std::tuple<ReplacedFunArgs...> args_tuple)
      : previous_storage(previous_storage), fun(fun1), args_tuple(std::move(args_tuple)) {}

  void addBindings(FixedSizeVector<ComponentStorageEntry>& entries) {
    entries.push_back(
        ComponentStorageEntry::LazyComponentWithArgs::createReplacedComponentEntry(fun, std::move(args_tuple)));
    previous_storage.addBindings(entries);
  }

  std::size_t numBindings() const {
    return previous_storage.numBindings() + 1;
  }
};

template <typename OtherComponent, typename... PreviousBindings, typename... ReplacedFunArgs>
class PartialComponentStorage<ReplaceComponent<OtherComponent(ReplacedFunArgs...), OtherComponent()>,
                              PreviousBindings...> {
private:
  using previous_storage_t =
      PartialComponentStorage<PartialReplaceComponent<OtherComponent(ReplacedFunArgs...)>, PreviousBindings...>;

  previous_storage_t& previous_storage;
  OtherComponent (*fun)();

public:
  PartialComponentStorage(previous_storage_t& previous_storage, OtherComponent (*fun1)(), std::tuple<>)
      : previous_storage(previous_storage), fun(fun1) {}

  void addBindings(FixedSizeVector<ComponentStorageEntry>& entries) const {
    entries.push_back(ComponentStorageEntry::LazyComponentWithNoArgs::createReplacementComponentEntry(fun));
    previous_storage.addBindings(entries);
  }

  std::size_t numBindings() const {
    return previous_storage.numBindings() + 1;
  }
};

template <typename OtherComponent, typename... ReplacedFunArgs, typename... ReplacementFunArgs,
          typename... PreviousBindings>
class PartialComponentStorage<
    ReplaceComponent<OtherComponent(ReplacedFunArgs...), OtherComponent(ReplacementFunArgs...)>, PreviousBindings...> {
private:
  using previous_storage_t =
      PartialComponentStorage<PartialReplaceComponent<OtherComponent(ReplacedFunArgs...)>, PreviousBindings...>;

  previous_storage_t& previous_storage;
  OtherComponent (*fun)(ReplacementFunArgs...);
  std::tuple<ReplacementFunArgs...> args_tuple;

public:
  PartialComponentStorage(previous_storage_t& previous_storage, OtherComponent (*fun1)(ReplacementFunArgs...),
                          std::tuple<ReplacementFunArgs...> args_tuple)
      : previous_storage(previous_storage), fun(fun1), args_tuple(std::move(args_tuple)) {}

  void addBindings(FixedSizeVector<ComponentStorageEntry>& entries) {
    entries.push_back(
        ComponentStorageEntry::LazyComponentWithArgs::createReplacementComponentEntry(fun, std::move(args_tuple)));
    previous_storage.addBindings(entries);
  }

  std::size_t numBindings() const {
    return previous_storage.numBindings() + 1;
  }
};

} // namespace impl
} // namespace fruit

#endif // FRUIT_PARTIAL_COMPONENT_STORAGE_DEFN_H
