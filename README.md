# Toy Entity Component System (ECS)

After reading about Data Oriented Design and exploring some of the EnTT library's source code, I realized my understanding of C++ metaprogramming wasn't quite where I wanted it to be. The best way to learn is to practice, so I did!

My ECS uses a Sparse and Dense Set to store entity IDs as unsigned 32 bit integers. Entity IDs are created sequentially (0,1,2,...) and are recycled into an unordered set for later re-use if they are destroyed. 
The container class for all of the Component Data is called "SparseSet". The "world" is the primary user interface for interacting with entity IDs and Components. Components (SparseSets) inherit from
a virtual class "ComponentStorage" so I can store the SparseSets in a vector instead of a tuple. I originally started with a tuple but found the vector to be both faster and simpler for storage.

The most difficult portion was serialization and deserialization, specifically for storing user-created types, reading them, and then matching them back to the correct type for loading/storage. I ended up using 
MSVC's __FUNCSIG__ to extract the type name at compile time, then use a compile-time Fowler-Noll-Vo (FNV) hash to map the type names to their correct deserialization functions. Once C++26 is released and supported by compilers,
I imagine that the new Compile-time reflection system will be of great benefit in situations exactly like this.

## Organization
I use .h files to forward declare classes and member functions, and .ipp files to fully implement them. I ran into a lot of circular dependency issues and found this to be the easiest way for me to fix those and keep track of things.
I wrote everything in the same folder, divided by filters in Visual Studio Community but have moved all of the source files into the src folder for the sake of brevity

## Features
Observers, views, signals/sinks, dispatchers, groups, command buffers, and serialization/deserialization

## Learning

This was my first real exposure to a lot of compile-time concepts (if constexpr, type traits, heavy variadic templates). I ran into a lot of new problems and had to do a lot of googling and reading to find solutions. 
Compared to EnTT, this is a very simple toy ECS. I found a lot of EnTT's source code very hard to follow and decipher (because of far they are able to push compile time metaprogramming!), and tried to make my solutions
as readable as possible.

## Requirements: C++20, written and tested on MSVC v.14.50

## Wrap-Up
If you have any questions or suggestions, feel free to reach out and let me know! I had a great time building this ECS and am excited to explore more compile-time metaprogramming techniques in later projects!
