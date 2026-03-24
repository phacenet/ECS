#pragma once

//Cannot pass two variadic packs to ctor, so wrap in struct for deduction
//Define type template
template <typename ...Types>
struct get_t {};

//Define variable template of get_t type
template <typename ...Types>
inline constexpr get_t<Types...> get{};


/* To check which tuple has the type searched for*/
/* ----------------------------------------------------- */
template <typename T, typename Tuple>
struct has_type;

template <typename T, typename... Us>
struct has_type<T, std::tuple<Us...>> : std::disjunction<std::is_same<T, Us>...> {};
/* ----------------------------------------------------- */