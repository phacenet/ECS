# Entity Component System (ECS) - C++20

## Design

### Sparse/Dense Set Storage
Each component type is stored in a "SparseSet<T>", which is a two-array structure that provides O(1) insert, remove, and lookup, while keeping component data contiguous in memory.

- The **sparse-array** maps entity IDs to their index in the dense array
- The **dense-array** stores entity IDs in a contiguous order
- A parallel **data array** stores component data values at the same index as the dense array

  Removal uses swap-and-pop to preserve contiguity without shifting elements. Helps to avoid cache-hostile pointer chasing of 'std::unordered_map' based designs. Also helps to ensure that iteration over all entities with a certain Component is a linear scan through contiguous memory.

  A partial specialization of the Component class, **tag components** handles tags (identifiers/markers with no data) separately, removing the data array and no-opping on some of the inherited functions.

   ### Entity ID Management
  Entity IDs are integer values issued sequentially. Destroyed IDs are pushed onto a stack and recycled on the next entity creation call (createEntity()). This helps to prevent unbounded ID growth for heavy usage of IDs.

  ### World Interface
  World is the primary user-facing API, and the source of truth. It owns all component storage and exposes entity/component operations to other classes. A world is non-copyable and non-movable by design, but can be serialized (saved and loaded) to save the world state.

  ### Views and Groups

  **Views** iterate over entities that possess all of a set of component types. At construction, the view calculates the smallest viewed 'SparseSet' and uses it as the iteration source, checking membership in the remaining sets per entity.

  **Groups** reorder component data across owned 'SparseSet's so that entities matching the group's signature are contiguous at the front of every owned array.

  ### Signal/Sink System
  A 'Signal<Ret(Args...)>' class stores callbacks as std::function objects alongside 'CallbackToken's (function ptr + instance ptr pairs) for O(n) disconnect. 'Sink' exposes a restricted interface to the user for connecting and disconnecting listeners.

  ### Observers
  'Observer' and 'GroupObserver' track which entities have had specific component transitions (add/remove) since the last time the observer was cleared. 'SparseSet' notifies registered observers directly on insert and remove. This ensures overhead is proportional to observed transitions instead of the total entity count.

  ### Command Buffer
  'CommandBuffer' defers entity and component mutations to a later flush point. This was important during implementation for potentially "dangerous" operations within a user-created lambda/std::function. This allows for queueing structural changes (creating new entity, add/removing component) during iteration without invalidating the sets being iterated.

  ### Serialization
  Serialization was the hardest architectural challenge during this project for me. 'SparseSet<T>' is a template, and hence component types are user defined, and there is no fixed type registry at compile time.

  My solution was to use __FUNCSIG__ (on MSVC) to extract a stable type name string at compile time, then hash it with a compile-time Fowler-Noll-Vo (FNV) hash to produce a constexpr identifier per component type. On serialization, each SparseSet writes its type hash, entity count, entity IDs, and raw component data to a binary file. On deserialization, hashes are matched against a compile-time-generated dispatch table to route each block to the correct SparseSet<T>::derserialize().

  This approach is a workaround for the (much anticipated!) absence of a reflection system in C++20. The coming C++26 static reflection (std::meta) will hopefully ease the implementation of future systems like this without compiler-specific solutions.

  ## Features
  Tag components, views, groups, signals/sinks, observers, command buffer, serialization/deserialization

  ## Implementation Notes
  - .h files contain class declarations and forward declarations
  - .ipp files contain template implementations and are included at the bottom of their corresponding .h files. This layout/separation resolved my circular dependency issues that arose from the mutual template dependencies (World requires View, View requires World, etc)
  - All component storage is owned by 'World' stored as 'ComponentStorage' (non template base-class) allowing component storage as a std::vector without type erasure overhead at the call site.
  - I created and used a 'DebugFunctions::Access' struct for debugging purposes, but it is also useful for introspection into some of the underlying containers at runtime
 
## Requirements
- C++20, written and tested on MSVC v14.50 (uses __FUNCSIG__ for serialization, but also included a __PRETTY_FUNCTION__ #ifdef)

## Closing
I modeled this loosely after EnTT's ECS. I found EnTT's source code very difficult to understand and follow (because of how amazing they are at taking advantage of template metaprogramming!), and hence have attempted to make my solutions as simple and easy to follow as possible. If you have any questions or suggestions, feel free to reach out and let me know! 
