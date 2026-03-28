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

//for else branch of invocable_v
template <typename>
inline constexpr bool always_false = false;




template <template <typename> class Pred, typename TUPLE>
struct Filter;

template <template <typename> class Pred, typename ... Ts>
struct Filter<Pred, std::tuple<Ts...>>
{
    using type = decltype(std::tuple_cat(
        std::declval<std::conditional_t<Pred<Ts>::value,
                                        std::tuple<Ts>,
                                         std::tuple<>>>()...));
};


template <typename T>
struct is_not_tag : std::negation<std::is_base_of<TagBase, T>> {};


template <typename F, typename Tuple>
struct is_invocable_with_tuple;

template <typename F, typename... Types>
struct is_invocable_with_tuple<F, std::tuple<Types...>>
: std::is_invocable<F, Types&...> {};


template <typename F, typename ID, typename Tuple>
struct is_invocable_with_tuple_and_id;

template<typename F, typename ID, typename ...Types>
struct is_invocable_with_tuple_and_id<F, ID, std::tuple<Types...>>
    :std::is_invocable<F, ID, Types&...> 
{
    static_assert(std::is_same_v<ID, uint32_t>, "ID must be uint32_t");
};

template <typename T>
struct is_invocable_with_id
    : std::bool_constant <std::is_invocable_v<T, uint32_t>>{};