#pragma once

#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace json {

	// ----------------- is_one_of --------------------------
	template <typename T, typename U, typename... Rest>
	struct is_one_of : std::bool_constant<
		is_one_of<T, U>::value || is_one_of<T, Rest...>::value> {
	};
	template <typename T, typename U>
	struct is_one_of<T, U> : std::is_same<T, U> {};
	template <typename T, typename U, typename... Rest>
	inline constexpr bool is_one_of_v = is_one_of<T, U, Rest...>::value;

	// ----------------- is_instantiation_of -----------------
	template <typename Inst, template <typename...> class Tmpl>
	struct is_instantiation_of : std::false_type {};
	template <template <typename...> class Tmpl, typename... Args>
	struct is_instantiation_of<Tmpl<Args...>, Tmpl> : std::true_type {};
	template <typename Inst, template <typename...> class Tmpl>
	inline constexpr bool is_instantiation_of_v =
		is_instantiation_of<Inst, Tmpl>::value;


	// forward declare begin

	// string, char
	template <typename T>
	std::enable_if_t<is_one_of_v<std::decay_t<T>, std::string, char>, std::string>
		dumps(const T&);

	static inline std::string dumps(const char* s) {
		return json::dumps(std::string(s));
	}

	// void, nullptr
	template <typename T>
	std::
		enable_if_t<is_one_of_v<std::decay_t<T>, void, std::nullptr_t>, std::string>
		dumps(const T&);

	// bool
	template <typename T>
	std::enable_if_t<is_one_of_v<std::decay_t<T>, bool>, std::string>
		dumps(const T&);

	// int, long, long long, unsigned, unsigned long, unsigned long long, float, double, long double
	template <typename T>
	std::enable_if_t<
		is_one_of_v<
		std::decay_t<T>,
		int,
		long,
		long long,
		unsigned,
		unsigned long,
		unsigned long long,
		float,
		double,
		long double>,
		std::string>
		dumps(const T&);

	// vector, list, deque, forward_list, set, multiset, unordered_set, unordered_multiset
	template <template <typename...> class Tmpl, typename... Args>
	std::enable_if_t<
		is_instantiation_of_v<Tmpl<Args...>, std::vector> ||
		is_instantiation_of_v<Tmpl<Args...>, std::list> ||
		is_instantiation_of_v<Tmpl<Args...>, std::deque> ||
		is_instantiation_of_v<Tmpl<Args...>, std::forward_list> ||
		is_instantiation_of_v<Tmpl<Args...>, std::set> ||
		is_instantiation_of_v<Tmpl<Args...>, std::multiset> ||
		is_instantiation_of_v<Tmpl<Args...>, std::unordered_set> ||
		is_instantiation_of_v<Tmpl<Args...>, std::unordered_multiset>,
		std::string>
		dumps(const Tmpl<Args...>&);

	// map, multimap, unordered_map, unordered_multimap
	template <template <typename...> class Tmpl, typename... Args>
	std::enable_if_t<
		is_instantiation_of_v<Tmpl<Args...>, std::map> ||
		is_instantiation_of_v<Tmpl<Args...>, std::multimap> ||
		is_instantiation_of_v<Tmpl<Args...>, std::unordered_map> ||
		is_instantiation_of_v<Tmpl<Args...>, std::unordered_multimap>,
		std::string>
		dumps(const Tmpl<Args...>&);

	// array
	template <typename T>
	std::enable_if_t<std::is_array_v<T>, std::string> dumps(const T&);

	// std::array
	template <typename T, std::size_t N>
	std::string dumps(const std::array<T, N>&);

	// tuple
	template <size_t N, typename... Args>
	std::enable_if_t<N == sizeof...(Args) - 1, std::string>
		dumps(const std::tuple<Args...>&);
	template <size_t N, typename... Args>
	std::enable_if_t<N != 0 && N != sizeof...(Args) - 1, std::string>
		dumps(const std::tuple<Args...>&);
	template <size_t N = 0, typename... Args>
	std::enable_if_t<N == 0, std::string> dumps(const std::tuple<Args...>&);

	// pair
	template <typename T, typename U>
	std::string dumps(const std::pair<T, U>&);

	// pointer
	template <typename T>
	std::string dumps(const T*);

	// shared_ptr, weak_ptr, unique_ptr
	template <typename T>
	std::enable_if_t<
		is_instantiation_of_v<T, std::shared_ptr> ||
		is_instantiation_of_v<T, std::weak_ptr> ||
		is_instantiation_of_v<T, std::unique_ptr>,
		std::string>
		dumps(const std::shared_ptr<T>&);

	// forward declare end

	// impl begin

	// string, char
	template <typename T>
	std::enable_if_t<is_one_of_v<std::decay_t<T>, std::string, char>, std::string>
		dumps(const T& obj) {
		std::stringstream ss;
		ss << '"' << obj << '"';
		return ss.str();
	}

	// void, nullptr
	template <typename T>
	std::
		enable_if_t<is_one_of_v<std::decay_t<T>, void, std::nullptr_t>, std::string>
		dumps(const T&) {
		return "null";
	}

	// bool
	template <typename T>
	std::enable_if_t<is_one_of_v<std::decay_t<T>, bool>, std::string>
		dumps(const T& value) {
		return value ? "true" : "false";
	}

	// int, long, long long, unsigned, unsigned long, unsigned long long, float, double, long double
	template <typename T>
	std::enable_if_t<
		is_one_of_v<
		std::decay_t<T>,
		int,
		long,
		long long,
		unsigned,
		unsigned long,
		unsigned long long,
		float,
		double,
		long double>,
		std::string>
		dumps(const T& value) {
		return std::to_string(value);
	}

	// vector, list, deque, forward_list, set, multiset, unordered_set, unordered_multiset
	template <template <typename...> class Tmpl, typename... Args>
	std::enable_if_t<
		is_instantiation_of_v<Tmpl<Args...>, std::vector> ||
		is_instantiation_of_v<Tmpl<Args...>, std::list> ||
		is_instantiation_of_v<Tmpl<Args...>, std::deque> ||
		is_instantiation_of_v<Tmpl<Args...>, std::forward_list> ||
		is_instantiation_of_v<Tmpl<Args...>, std::set> ||
		is_instantiation_of_v<Tmpl<Args...>, std::multiset> ||
		is_instantiation_of_v<Tmpl<Args...>, std::unordered_set> ||
		is_instantiation_of_v<Tmpl<Args...>, std::unordered_multiset>,
		std::string>
		dumps(const Tmpl<Args...>& obj) {
		std::stringstream ss;
		ss << "[";
		for (auto itr = obj.begin(); itr != obj.end();) {
			ss << dumps(*itr);
			if (++itr != obj.end()) ss << ", ";
		}
		ss << "]";
		return ss.str();
	}

	// map, nultimap, unordered_map, unordered_multimap
	template <template <typename...> class Tmpl, typename... Args>
	std::enable_if_t<
		is_instantiation_of_v<Tmpl<Args...>, std::map> ||
		is_instantiation_of_v<Tmpl<Args...>, std::multimap> ||
		is_instantiation_of_v<Tmpl<Args...>, std::unordered_map> ||
		is_instantiation_of_v<Tmpl<Args...>, std::unordered_multimap>,
		std::string>
		dumps(const Tmpl<Args...>& obj) {
		std::stringstream ss;
		ss << "{";
		for (auto itr = obj.begin(); itr != obj.end();) {
			ss << dumps(itr->first);
			ss << ":";
			ss << dumps(itr->second);
			if (++itr != obj.end()) ss << ", ";
		}
		ss << "}";
		return ss.str();
	}

	// array
	template <typename T>
	std::enable_if_t<std::is_array_v<T>, std::string> dumps(const T& arr) {
		std::stringstream ss;
		ss << "[";
		for (size_t i = 0; i < std::extent_v<T>; ++i) {
			ss << dumps(arr[i]);
			if (i != std::extent_v<T> -1) ss << ", ";
		}
		ss << "]";
		return ss.str();
	}

	// std::array
	template <typename T, std::size_t N>
	std::string dumps(const std::array<T, N>& obj) {
		std::stringstream ss;
		ss << "[";
		for (auto itr = obj.begin(); itr != obj.end();) {
			ss << dumps(*itr);
			if (++itr != obj.end()) ss << ", ";
		}
		ss << "]";
		return ss.str();
	}

	// std::tuple
	template <size_t N, typename... Args>
	std::enable_if_t<N == sizeof...(Args) - 1, std::string>
		dumps(const std::tuple<Args...>& obj) {
		std::stringstream ss;
		ss << dumps(std::get<N>(obj)) << "]";
		return ss.str();
	}
	template <size_t N, typename... Args>
	std::enable_if_t<N != 0 && N != sizeof...(Args) - 1, std::string>
		dumps(const std::tuple<Args...>& obj) {
		std::stringstream ss;
		ss << dumps(std::get<N>(obj)) << ", " << dumps<N + 1, Args...>(obj);
		return ss.str();
	}
	template <size_t N, typename... Args>
	std::enable_if_t<N == 0, std::string> dumps(const std::tuple<Args...>& obj) {
		std::stringstream ss;
		ss << "[" << dumps(std::get<N>(obj)) << ", " << dumps<N + 1, Args...>(obj);
		return ss.str();
	}

	// std::pair
	template <typename T, typename U>
	std::string dumps(const std::pair<T, U>& obj) {
		std::stringstream ss;
		ss << "{" << dumps(obj.first) << ":" << dumps(obj.second) << "}";
		return ss.str();
	}

	// pointer
	template <typename T>
	std::string dumps(const T* p) {
		return dumps(*p);
	}

	// shared_ptr, weak_ptr, unique_ptr
	template <typename T>
	std::enable_if_t<
		is_instantiation_of_v<T, std::shared_ptr> ||
		is_instantiation_of_v<T, std::weak_ptr> ||
		is_instantiation_of_v<T, std::unique_ptr>,
		std::string>
		dumps(const std::shared_ptr<T>& p) {
		return dumps(*p);
	}

	// impl end

}  // namespace json