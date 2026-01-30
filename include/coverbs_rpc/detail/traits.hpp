#pragma once

#include <cppcoro/task.hpp>
#include <glaze/glaze.hpp>
#include <tuple>
#include <type_traits>

namespace coverbs_rpc::detail {

template <typename T>
struct is_task : std::false_type {};
template <typename R>
struct is_task<cppcoro::task<R>> : std::true_type {};
template <typename T>
inline constexpr bool is_task_v = is_task<T>::value;

template <typename T>
struct task_result {
  using type = T;
};
template <typename R>
struct task_result<cppcoro::task<R>> {
  using type = R;
};
template <typename T>
using task_result_t = typename task_result<T>::type;

template <typename F>
struct function_traits;

template <typename R, typename... Args>
struct function_traits_base {
  static constexpr size_t arity = sizeof...(Args);
  static_assert(arity >= 1 && arity <= 2, "RPC function must have 1 or 2 arguments");

  using raw_resp_type = std::decay_t<R>;
  using params = std::tuple<Args...>;

  static constexpr bool is_coro_fn_v = is_task_v<raw_resp_type>;
  using request_type = std::decay_t<std::tuple_element_t<0, params>>;
  using response_type = std::decay_t<task_result_t<raw_resp_type>>;

  static constexpr bool is_with_session() {
    if constexpr (arity == 2) {
      return true;
    }
    return false;
  }
  static constexpr bool call_with_session = is_with_session();
};

template <typename R, typename Class, typename... Args>
struct function_traits<R (Class::*)(Args...)> : function_traits_base<R, Args...> {
  static constexpr bool is_member_fn = true;
};

template <typename R, typename Class, typename... Args>
struct function_traits<R (Class::*)(Args...) const> : function_traits_base<R, Args...> {
  static constexpr bool is_member_fn = true;
};

template <typename R, typename... Args>
struct function_traits<R (*)(Args...)> : function_traits_base<R, Args...> {
  static constexpr bool is_member_fn = false;
};

// For lambdas and functors
template <typename T>
struct function_traits : function_traits<decltype(&T::operator())> {};

template <auto Handler>
using rpc_req_t = typename function_traits<decltype(Handler)>::request_type;

template <auto Handler>
using rpc_resp_t = typename function_traits<decltype(Handler)>::response_type;

template <auto Handler>
inline constexpr bool is_coro_fn_v = function_traits<decltype(Handler)>::is_coro_fn_v;

template <auto Handler>
inline constexpr bool is_with_session_v = function_traits<decltype(Handler)>::call_with_session;

template <auto Handler>
inline constexpr bool is_member_fn_v = function_traits<decltype(Handler)>::is_member_fn;

template <auto Handler, typename Signature = decltype(Handler)>
struct function_id_traits {
  static constexpr std::string_view get_handler_name() { return __PRETTY_FUNCTION__; }

  static constexpr std::string_view name = get_handler_name();

  static constexpr uint32_t hash() {
    constexpr std::string_view str = name;
    uint32_t result = 0x811c9dc5; // FNV offset basis
    for (char c : str) {
      result ^= static_cast<uint32_t>(c);
      result *= 0x01000193; // FNV prime
    }
    return result;
  }

  static constexpr uint32_t id = hash();
};

template <auto Handler>
inline constexpr uint32_t function_id = function_id_traits<Handler>::id;
template <auto Handler>
inline constexpr std::string_view function_name = function_id_traits<Handler>::name;

} // namespace coverbs_rpc::detail
