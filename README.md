** Toy Entity Component System (ECS) **

I was reading about Data Driven Design and stumbled on the Entt ECS. Reading through some of their code made me realize I do not have as good of a grasp on modern metaprogramming as I had hoped, so I figured the best way to learn something about modern metaprogrammming and get some practice some Data Driven Design was to 
work on a my own ECS! It is certainly nowhere near as impressive or efficient as ECS, but has many of the major features that I would expect an ECS to have (groups, signals/sinks, dispatchers, observers, serialization/deserialization, etc.)

I wrote everything inside of .h files for forward declarations (one or two exceptions that have the full implementation in the .h file as well) and the implementations inside the .ipp files. I ran into a lot of circular dependency issues,
so this was the easiest way for me to fix those.
